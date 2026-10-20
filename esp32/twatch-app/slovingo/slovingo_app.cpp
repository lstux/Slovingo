/****************************************************************************
 *   Slovingo app for the T-Watch: language courses from the Slovingo engine
 *   https://github.com/lstux/Slovingo
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "config.h"

#include "slovingo_app.h"
#include "slovingo_app_main.h"

#include "gui/mainbar/mainbar.h"
#include "gui/statusbar.h"
#include "gui/app.h"

uint32_t slovingo_app_main_tile_num;

// app icon
icon_t *slovingo_app = NULL;

// declare the images we need
LV_IMG_DECLARE( slovingo_64px );

static void enter_slovingo_app_event_cb( lv_obj_t * obj, lv_event_t event );

/*
 * automatic register the app setup function, no explicit call in main.cpp needed
 */
static int registed = app_autocall_function( &slovingo_app_setup, APP_PRIO( APP_GROUP_GENERAL, 5 ) );           /** @brief app autocall function */

/*
 * setup routine for the slovingo app
 */
void slovingo_app_setup( void ) {
    /*
     * check if app already registered for autocall
     */
    if( !registed ) {
        return;
    }

    slovingo_app_main_tile_num = mainbar_add_app_tile( 1, 1, "slovingo app" );

    slovingo_app = app_register( "slovingo", &slovingo_64px, enter_slovingo_app_event_cb );

    // init main tile, see slovingo_app_main.cpp
    slovingo_app_main_setup( slovingo_app_main_tile_num );
}

uint32_t slovingo_app_get_app_main_tile_num( void ) {
    return( slovingo_app_main_tile_num );
}

static void enter_slovingo_app_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       app_hide_indicator( slovingo_app );
                                        mainbar_jump_to_tilenumber( slovingo_app_main_tile_num, LV_ANIM_OFF, true );
                                        break;
    }
}
