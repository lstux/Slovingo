/*
 * Remplace mainbar.h du firmware de la montre (grille de tuiles). La T-HMI n'a qu'un ecran :
 * l'app y occupe une seule "tuile", un conteneur plein ecran.
 */
#ifndef _MAINBAR_H
    #define _MAINBAR_H

    #include <stdint.h>
    #include "lvgl.h"

    typedef void ( *mainbar_callback_t )( void );

    /** @brief create the tile of an app, returns its number (the x/y position is ignored here) */
    uint32_t mainbar_add_app_tile( uint16_t x, uint16_t y, const char *id );
    /** @brief the lvgl container of a tile */
    lv_obj_t *mainbar_get_tile_obj( uint32_t tile_num );
    /** @brief called when the tile is shown, see mainbar_activate_tile() */
    void mainbar_add_tile_activate_cb( uint32_t tile_num, mainbar_callback_t cb );
    /** @brief called when the tile is left, see mainbar_hibernate_tile() */
    void mainbar_add_tile_hibernate_cb( uint32_t tile_num, mainbar_callback_t cb );
    void mainbar_jump_to_tilenumber( uint32_t tile_num, lv_anim_enable_t anim, bool save_history = true );
    /** @brief back from the first screen of the app: switch the device off, see board_power_off() */
    void mainbar_jump_back( void );

    /** @brief T-HMI only: show the tile and call its activate callback (main.cpp does it once at start) */
    void mainbar_activate_tile( uint32_t tile_num );
    /** @brief T-HMI only: call the hibernate callback of the tile (before going to sleep) */
    void mainbar_hibernate_tile( uint32_t tile_num );

#endif // _MAINBAR_H
