/*
 * Slovingo sur la LilyGo T-HMI.
 *
 * L'app est celle de la montre (../twatch-app/slovingo), qui s'appuie sur LVGL 7.11 ; src/compat
 * remplace les quelques fonctions du firmware de la montre qu'elle utilise.
 */
#include <Arduino.h>
#include "lvgl.h"
#include "board.h"
#include "touch.h"
#include "lvgl_port.h"
#include "gui/mainbar/mainbar.h"
#include "slovingo_app_main.h"

void setup( void ) {
    Serial.begin( 115200 );
    board_init();

    bool sd = board_sd_mount();
    if ( !sd )
        log_w("without SD card: no cache, no saved settings or progress");

    board_display_init();

    /* OK button held at power on: calibrate the touch screen again */
    touch_init( digitalRead( BUTTON_OK_PIN ) == LOW );
    lvgl_port_init();

    uint32_t tile = mainbar_add_app_tile( 1, 1, "slovingo" );
    slovingo_app_main_setup( tile );
    mainbar_activate_tile( tile );
}

void loop( void ) {
    lv_task_handler();
    delay( 5 );
}
