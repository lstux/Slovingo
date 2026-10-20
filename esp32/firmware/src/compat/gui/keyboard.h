/* Clavier a l'ecran pour saisir l'adresse d'un cours : version reduite de keyboard.h de la montre. */
#ifndef _KEYBOARD_H
    #define _KEYBOARD_H

    #include "lvgl.h"

    /** @brief show the keyboard on the bottom half of the screen and type into this textarea */
    void keyboard_set_textarea( lv_obj_t *textarea );
    /** @brief hide the keyboard */
    void keyboard_hide( void );

#endif // _KEYBOARD_H
