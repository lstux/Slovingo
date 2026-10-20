#include "gui/widget_styles.h"

static lv_style_t app_style;
static lv_style_t button_style;
static lv_style_t label_style;
static bool styles_defined = false;

static void define_styles( void ) {
    if ( styles_defined )
        return;
    styles_defined = true;

    /* fond noir, texte blanc */
    lv_style_init( &app_style );
    lv_style_set_bg_color( &app_style, LV_STATE_DEFAULT, LV_COLOR_BLACK );
    lv_style_set_bg_opa( &app_style, LV_STATE_DEFAULT, LV_OPA_COVER );
    lv_style_set_border_width( &app_style, LV_STATE_DEFAULT, 0 );
    lv_style_set_radius( &app_style, LV_STATE_DEFAULT, 0 );
    lv_style_set_text_color( &app_style, LV_STATE_DEFAULT, LV_COLOR_WHITE );
    lv_style_set_pad_all( &app_style, LV_STATE_DEFAULT, 0 );

    /* boutons gris fonce, plus clairs quand on appuie */
    lv_style_init( &button_style );
    lv_style_set_bg_color( &button_style, LV_STATE_DEFAULT, lv_color_hex( 0x303030 ) );
    lv_style_set_bg_color( &button_style, LV_STATE_PRESSED, lv_color_hex( 0x606060 ) );
    lv_style_set_bg_opa( &button_style, LV_STATE_DEFAULT, LV_OPA_COVER );
    lv_style_set_border_width( &button_style, LV_STATE_DEFAULT, 0 );
    lv_style_set_radius( &button_style, LV_STATE_DEFAULT, 8 );
    lv_style_set_text_color( &button_style, LV_STATE_DEFAULT, LV_COLOR_WHITE );
    lv_style_set_shadow_width( &button_style, LV_STATE_DEFAULT, 0 );

    lv_style_init( &label_style );
    lv_style_set_text_color( &label_style, LV_STATE_DEFAULT, LV_COLOR_WHITE );
}

lv_style_t *ws_get_app_style( void ) {
    define_styles();
    return( &app_style );
}

lv_style_t *ws_get_button_style( void ) {
    define_styles();
    return( &button_style );
}

lv_style_t *ws_get_label_style( void ) {
    define_styles();
    return( &label_style );
}
