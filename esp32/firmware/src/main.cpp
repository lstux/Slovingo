/*
 * Slovingo sur la LilyGo T-HMI.
 *
 * L'app est celle de la montre (../twatch-app/slovingo), qui s'appuie sur LVGL 7.11 ; src/compat
 * remplace les quelques fonctions du firmware de la montre qu'elle utilise.
 *
 * Avec -D SLV_DIAG=1 (platformio.ini), chaque etape du demarrage est ecrite sur le port serie
 * et retenue en memoire RTC : apres un plantage, la banniere dit ou ca s'est arrete (voir diag.h).
 */
#include <Arduino.h>
#include "lvgl.h"
#include "board.h"
#include "diag.h"
#include "touch.h"
#include "lvgl_port.h"
#include "gui/mainbar/mainbar.h"
#include "slovingo_app_main.h"

void setup( void ) {
    diag_init();
#if SLV_DIAG
    /* retroeclairage tout de suite : une carte qui tourne mais dont l'ecran n'est pas pilote reste eclairee */
    board_backlight( true );
#endif

    diag_step( "board_init" );
    board_init();

    diag_step( "sd_mount" );
    bool sd = board_sd_mount();
    if ( !sd )
        log_w("without SD card: no cache, no saved settings or progress");

    diag_step( "display_init" );
    board_display_init();
    diag_screen_ready();

    diag_step( sd ? "sd: carte montee" : "sd: PAS DE CARTE" );
    diag_step( "touch_init" );
    /* OK button held at power on: calibrate the touch screen again */
    touch_init( digitalRead( BUTTON_OK_PIN ) == LOW );

    diag_screen_done();
    diag_step( "lvgl_port_init" );
    lvgl_port_init();

    diag_step( "app_setup" );
    uint32_t tile = mainbar_add_app_tile( 1, 1, "slovingo" );
    slovingo_app_main_setup( tile );

    diag_step( "app_activate" );
    mainbar_activate_tile( tile );
    diag_step( "setup_done" );
}

void loop( void ) {
    lv_task_handler();
    diag_heartbeat();
    delay( 5 );
}
