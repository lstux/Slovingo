/*
 * Programme de diagnostic de la T-HMI (environnement PlatformIO t-hmi-diag) : sans LVGL ni app,
 * juste l'ecran, le port serie, la carte SD, la batterie et les boutons. Sert a savoir si la carte
 * et la config de l'ecran marchent, independamment du reste.
 *
 *   pio run -e t-hmi-diag -t upload
 *   pio device monitor -e t-hmi-diag
 */
#include <Arduino.h>
#include <SD_MMC.h>
#include <TFT_eSPI.h>
#include "board.h"
#include "diag.h"

#if !SLV_DIAG
    #error "t-hmi-diag a besoin de -D SLV_DIAG=1 (platformio.ini)"
#endif

TFT_eSPI tft = TFT_eSPI();

static bool sd_ok = false;
static uint32_t next_refresh = 0;
static uint8_t frame = 0;

void setup( void ) {
    diag_init();
    board_backlight( true );

    diag_step( "board_init" );
    board_init();

    diag_step( "sd_mount" );
    sd_ok = board_sd_mount();

    diag_step( "tft.begin" );
    tft.begin();
    tft.setRotation( BOARD_ROTATION );
    tft.fillScreen( TFT_BLACK );
    diag_screen_start( true );

    diag_step( sd_ok ? "sd: carte montee" : "sd: PAS DE CARTE" );
    diag_line( sd_ok ? TFT_GREEN : TFT_ORANGE, "SD : %s", sd_ok ? "carte montee" : "pas de carte" );
    if ( sd_ok )
        diag_line( TFT_GREEN, "SD : %.1f Go", SD_MMC.cardSize() / 1024.0 / 1024.0 / 1024.0 );

    pinMode( TOUCH_IRQ_PIN, INPUT_PULLUP );
    pinMode( BUTTON_BOOT_PIN, INPUT_PULLUP );
    diag_line( TFT_WHITE, "Touche l'ecran, appuie sur OK / BOOT :" );
    diag_step( "setup_done" );
}

void loop( void ) {
    diag_heartbeat();

    uint32_t now = millis();
    if ( (int32_t)( now - next_refresh ) < 0 )
        return;
    next_refresh = now + 250;

    /* a fixed block at the bottom: touch interrupt (0 = pressed), the two buttons, battery, a moving square */
    int16_t y = BOARD_HEIGHT - 3 * 16 - 4;
    char text[ 64 ];

    tft.setTextColor( TFT_WHITE, TFT_BLACK );
    snprintf( text, sizeof( text ), "tactile IRQ=%d  OK=%d  BOOT=%d   ",
              digitalRead( TOUCH_IRQ_PIN ), digitalRead( BUTTON_OK_PIN ), digitalRead( BUTTON_BOOT_PIN ) );
    tft.drawString( text, 4, y, 2 );
    snprintf( text, sizeof( text ), "batterie %u mV   uptime %lu s   ", (unsigned)board_battery_mv(), (unsigned long)( now / 1000 ) );
    tft.drawString( text, 4, y + 16, 2 );
    snprintf( text, sizeof( text ), "libre %u Ko  PSRAM %u Ko   ", (unsigned)( ESP.getFreeHeap() / 1024 ), (unsigned)( ESP.getFreePsram() / 1024 ) );
    tft.drawString( text, 4, y + 32, 2 );

    tft.fillRect( BOARD_WIDTH - 20, 4, 16, 16, ( frame++ & 1 ) ? TFT_GREEN : TFT_RED );
}
