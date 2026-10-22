/*
 * Boutons de pied de page et etiquettes : version reduite de widget_factory.h de la montre.
 * Les icones (retour, rafraichir, reglages) sont les symboles de LVGL.
 * Tailles pensees pour un tactile resistif (cibles d'au moins 48 px).
 */
#ifndef _WIDGET_FACTORY_H
    #define _WIDGET_FACTORY_H

    #include "lvgl.h"

    #define THEME_ICON_SIZE         40
    #define THEME_ICON_PADDING      6
    #define THEME_PADDING           10
    #define THEME_CONT_HEIGHT       45

    lv_obj_t *wf_add_label( lv_obj_t *parent, char const *text );
    lv_obj_t *wf_add_exit_button( lv_obj_t *parent, lv_event_cb_t event_cb );
    lv_obj_t *wf_add_refresh_button( lv_obj_t *parent, lv_event_cb_t event_cb );
    lv_obj_t *wf_add_setup_button( lv_obj_t *parent, lv_event_cb_t event_cb );

#endif // _WIDGET_FACTORY_H
