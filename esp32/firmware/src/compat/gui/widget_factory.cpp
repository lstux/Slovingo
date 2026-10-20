#include "gui/widget_factory.h"
#include "gui/widget_styles.h"

lv_obj_t *wf_add_label( lv_obj_t *parent, char const *text ) {
    lv_obj_t *label = lv_label_create( parent, NULL );
    lv_label_set_text( label, text );
    lv_obj_add_style( label, LV_OBJ_PART_MAIN, ws_get_label_style() );
    return( label );
}

static lv_obj_t *wf_add_symbol_button( lv_obj_t *parent, const char *symbol, lv_event_cb_t event_cb ) {
    lv_obj_t *button = lv_btn_create( parent, NULL );
    lv_obj_set_size( button, THEME_ICON_SIZE, THEME_ICON_SIZE );
    lv_obj_add_style( button, LV_BTN_PART_MAIN, ws_get_button_style() );
    lv_obj_t *label = lv_label_create( button, NULL );
    lv_label_set_text( label, symbol );
    if ( event_cb )
        lv_obj_set_event_cb( button, event_cb );
    return( button );
}

lv_obj_t *wf_add_exit_button( lv_obj_t *parent, lv_event_cb_t event_cb ) {
    return( wf_add_symbol_button( parent, LV_SYMBOL_LEFT, event_cb ) );
}

lv_obj_t *wf_add_refresh_button( lv_obj_t *parent, lv_event_cb_t event_cb ) {
    return( wf_add_symbol_button( parent, LV_SYMBOL_REFRESH, event_cb ) );
}

lv_obj_t *wf_add_setup_button( lv_obj_t *parent, lv_event_cb_t event_cb ) {
    return( wf_add_symbol_button( parent, LV_SYMBOL_SETTINGS, event_cb ) );
}
