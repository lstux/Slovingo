#include <Arduino.h>
#include <WiFi.h>
#include <FS.h>
#include <SD_MMC.h>
#include <time.h>
#include "hardware/wifictl.h"
#include "utils/json_psram_allocator.h"

#define WIFICTL_DEFAULT_TZ      "CET-1CEST,M3.5.0,M10.5.0/3"    /* Europe/Paris */
#define WIFICTL_NTP_SERVER      "pool.ntp.org"

static bool time_configured = false;
static char tz[ 64 ] = WIFICTL_DEFAULT_TZ;

static void wifictl_on_got_ip( arduino_event_id_t event, arduino_event_info_t info ) {
    if ( !time_configured ) {
        time_configured = true;
        configTzTime( tz, WIFICTL_NTP_SERVER );
    }
}

void wifictl_on( void ) {
    static bool hooked = false;

    SpiRamJsonDocument doc( 8192 );
    fs::File file = SD_MMC.open( WIFICTL_CONFIG_FILE, FILE_READ );
    if ( !file ) {
        log_w("no %s on the SD card, no wifi", WIFICTL_CONFIG_FILE );
        return;
    }
    DeserializationError error = deserializeJson( doc, file );
    file.close();
    if ( error ) {
        log_e("%s: %s", WIFICTL_CONFIG_FILE, error.c_str() );
        return;
    }

    snprintf( tz, sizeof( tz ), "%s", (const char*)( doc["tz"] | WIFICTL_DEFAULT_TZ ) );
    if ( !hooked ) {
        hooked = true;
        WiFi.onEvent( wifictl_on_got_ip, ARDUINO_EVENT_WIFI_STA_GOT_IP );
    }

    WiFi.mode( WIFI_STA );
    const char *hostname = doc["hostname"] | "";
    if ( *hostname )
        WiFi.setHostname( hostname );

    /* the strongest network of the list that is in range */
    int found = WiFi.scanNetworks();
    JsonArray list = doc["networklist"];
    const char *best_ssid = NULL;
    const char *best_psk = NULL;
    int best_rssi = -1000;

    for ( int i = 0; i < found; i++ ) {
        for ( JsonObject network : list ) {
            const char *ssid = network["ssid"] | "";
            if ( *ssid && WiFi.SSID( i ) == ssid && WiFi.RSSI( i ) > best_rssi ) {
                best_rssi = WiFi.RSSI( i );
                best_ssid = ssid;
                best_psk = network["psk"] | "";
            }
        }
    }
    if ( !best_ssid ) {
        log_w("no known wifi network in range (%d seen)", found );
        WiFi.scanDelete();
        return;
    }

    log_i("wifi: connecting to %s", best_ssid );
    WiFi.begin( best_ssid, best_psk );
    WiFi.scanDelete();
}
