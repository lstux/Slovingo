/*
 * LVGL 7.11 pour la T-HMI : seuls les reglages qui different des valeurs par defaut
 * sont listes ici, lv_conf_internal.h complete le reste.
 */
#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*
 * Types de base : comme lv_conf.h n'est pas lv_conf_internal.h sans fichier, il faut les declarer ici
 * (memes valeurs que lv_conf_template.h de LVGL 7.11).
 */
typedef int16_t lv_coord_t;
typedef void * lv_anim_user_data_t;
typedef void * lv_group_user_data_t;
typedef void * lv_fs_drv_user_data_t;
typedef void * lv_img_decoder_user_data_t;
typedef void * lv_disp_drv_user_data_t;
typedef void * lv_indev_drv_user_data_t;
typedef void * lv_font_user_data_t;
typedef void * lv_obj_user_data_t;

/* ecran 320x240, couleur 16 bits, les octets sont echanges par pushColors() de TFT_eSPI */
#define LV_HOR_RES_MAX          320
#define LV_VER_RES_MAX          240
#define LV_COLOR_DEPTH          16
#define LV_COLOR_16_SWAP        0

/* la memoire de LVGL vient de la PSRAM (8 Mo), pas du tas interne */
#define LV_MEM_CUSTOM           1
#define LV_MEM_CUSTOM_INCLUDE   <esp32-hal-psram.h>
#define LV_MEM_CUSTOM_ALLOC     ps_malloc
#define LV_MEM_CUSTOM_FREE      free
#define LV_MEM_CUSTOM_REALLOC   ps_realloc

/* l'horloge de LVGL est celle d'Arduino */
#define LV_TICK_CUSTOM          1
#define LV_TICK_CUSTOM_INCLUDE  "Arduino.h"
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())

#define LV_USE_USER_DATA        1

/* tactile resistif : un peu plus de patience que sur un ecran capacitif */
#define LV_INDEV_DEF_READ_PERIOD            30
#define LV_INDEV_DEF_DRAG_LIMIT             10
#define LV_INDEV_DEF_GESTURE_LIMIT          40
#define LV_INDEV_DEF_LONG_PRESS_TIME        400

/* police par defaut : celle des symboles (fleches, roue dentee...) */
#define LV_FONT_MONTSERRAT_14   1
#define LV_FONT_DEFAULT         &lv_font_montserrat_14

#endif /* LV_CONF_H */
