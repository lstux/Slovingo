/* LVGL 7.11 sur la T-HMI : ecran (TFT_eSPI) et tactile. */
#ifndef _LVGL_PORT_H
    #define _LVGL_PORT_H

    /** @brief start the TFT: call before touch_init() (the calibration draws on it) */
    void board_display_init( void );
    /** @brief lv_init(), register the display and the touch of the board */
    void lvgl_port_init( void );

#endif // _LVGL_PORT_H
