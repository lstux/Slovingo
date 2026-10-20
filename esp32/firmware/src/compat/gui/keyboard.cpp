#include "gui/keyboard.h"

static lv_obj_t *keyboard = NULL;

static void keyboard_event_cb( lv_obj_t *obj, lv_event_t event ) {
    lv_keyboard_def_event_cb( obj, event );
    /* la touche "ok" envoie APPLY, la touche de fermeture envoie CANCEL */
    if ( event == LV_EVENT_APPLY || event == LV_EVENT_CANCEL )
        keyboard_hide();
}

void keyboard_set_textarea( lv_obj_t *textarea ) {
    if ( !keyboard ) {
        keyboard = lv_keyboard_create( lv_scr_act(), NULL );
        lv_obj_set_size( keyboard, lv_disp_get_hor_res( NULL ), lv_disp_get_ver_res( NULL ) / 2 );
        lv_obj_align( keyboard, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0 );
        lv_obj_set_event_cb( keyboard, keyboard_event_cb );
    }
    lv_keyboard_set_textarea( keyboard, textarea );
    lv_obj_set_hidden( keyboard, false );
    lv_obj_move_foreground( keyboard );
}

void keyboard_hide( void ) {
    if ( !keyboard )
        return;
    lv_keyboard_set_textarea( keyboard, NULL );
    lv_obj_set_hidden( keyboard, true );
}
