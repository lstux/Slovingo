#include "gui/mainbar/mainbar.h"
#include "gui/widget_styles.h"
#include "board.h"

#define MAINBAR_MAX_TILES   4

static struct {
    lv_obj_t *obj;
    mainbar_callback_t activate_cb;
    mainbar_callback_t hibernate_cb;
} tiles[ MAINBAR_MAX_TILES ];
static uint32_t tile_count = 0;

uint32_t mainbar_add_app_tile( uint16_t x, uint16_t y, const char *id ) {
    (void)x;
    (void)y;
    (void)id;

    if ( tile_count >= MAINBAR_MAX_TILES )
        return( MAINBAR_MAX_TILES - 1 );

    lv_obj_t *tile = lv_cont_create( lv_scr_act(), NULL );
    lv_obj_set_size( tile, lv_disp_get_hor_res( NULL ), lv_disp_get_ver_res( NULL ) );
    lv_obj_set_pos( tile, 0, 0 );
    lv_obj_add_style( tile, LV_CONT_PART_MAIN, ws_get_app_style() );

    tiles[ tile_count ].obj = tile;
    tiles[ tile_count ].activate_cb = NULL;
    tiles[ tile_count ].hibernate_cb = NULL;
    return( tile_count++ );
}

lv_obj_t *mainbar_get_tile_obj( uint32_t tile_num ) {
    return( tile_num < tile_count ? tiles[ tile_num ].obj : NULL );
}

void mainbar_add_tile_activate_cb( uint32_t tile_num, mainbar_callback_t cb ) {
    if ( tile_num < tile_count )
        tiles[ tile_num ].activate_cb = cb;
}

void mainbar_add_tile_hibernate_cb( uint32_t tile_num, mainbar_callback_t cb ) {
    if ( tile_num < tile_count )
        tiles[ tile_num ].hibernate_cb = cb;
}

void mainbar_activate_tile( uint32_t tile_num ) {
    if ( tile_num >= tile_count )
        return;
    lv_obj_move_foreground( tiles[ tile_num ].obj );
    if ( tiles[ tile_num ].activate_cb )
        tiles[ tile_num ].activate_cb();
}

void mainbar_hibernate_tile( uint32_t tile_num ) {
    if ( tile_num < tile_count && tiles[ tile_num ].hibernate_cb )
        tiles[ tile_num ].hibernate_cb();
}

void mainbar_jump_to_tilenumber( uint32_t tile_num, lv_anim_enable_t anim, bool save_history ) {
    (void)anim;
    (void)save_history;
    mainbar_activate_tile( tile_num );
}

void mainbar_jump_back( void ) {
    /* pas d'ecran precedent : la fleche de retour de l'ecran d'accueil eteint l'appareil */
    for ( uint32_t i = 0; i < tile_count; i++ )
        mainbar_hibernate_tile( i );
    board_power_off();
}
