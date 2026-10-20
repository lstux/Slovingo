#include <Arduino.h>
#include <SD_MMC.h>
#include <esp_sleep.h>
#include "board.h"

void board_init( void ) {
    pinMode( PWR_ON_PIN, OUTPUT );
    digitalWrite( PWR_ON_PIN, HIGH );
    pinMode( PWR_EN_PIN, OUTPUT );
    digitalWrite( PWR_EN_PIN, HIGH );
    pinMode( BUTTON_OK_PIN, INPUT_PULLUP );
}

bool board_sd_mount( void ) {
    SD_MMC.setPins( SD_CLK_PIN, SD_CMD_PIN, SD_D0_PIN );
    if ( !SD_MMC.begin( "/sdcard", true ) ) {
        log_w("no SD card");
        return( false );
    }
    log_i("SD card: %.2f GB", SD_MMC.cardSize() / 1024.0 / 1024.0 / 1024.0 );
    return( true );
}

uint32_t board_battery_mv( void ) {
    return( analogReadMilliVolts( BAT_ADC_PIN ) * 2 );
}

void board_backlight( bool on ) {
    pinMode( TFT_BL, OUTPUT );
    digitalWrite( TFT_BL, on ? HIGH : LOW );
}

void board_power_off( void ) {
    board_backlight( false );
    /* sur batterie, ceci coupe l'alimentation */
    digitalWrite( PWR_ON_PIN, LOW );
    /* sur USB, la carte reste alimentee : sommeil profond, le bouton OK reveille */
    delay( 100 );
    esp_sleep_enable_ext0_wakeup( (gpio_num_t)BUTTON_OK_PIN, 0 );
    esp_deep_sleep_start();
}
