/* Styles de base (fond sombre, boutons) : version reduite de widget_styles.h de la montre. */
#ifndef _WIDGET_STYLES_H
    #define _WIDGET_STYLES_H

    #include "lvgl.h"

    lv_style_t *ws_get_app_style( void );
    lv_style_t *ws_get_button_style( void );
    lv_style_t *ws_get_label_style( void );

    #define APP_STYLE       ws_get_app_style()

#endif // _WIDGET_STYLES_H
