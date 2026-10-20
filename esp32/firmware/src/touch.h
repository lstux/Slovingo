/* Tactile resistif XPT2046 de la T-HMI, avec calibration sur la carte SD. */
#ifndef _TOUCH_H
    #define _TOUCH_H

    #include <stdint.h>

    /**
     * @brief start the touch controller and load the calibration of the SD card (/touch_cal.json).
     * Without calibration, or with force_calibration, a screen with four crosses is shown first
     * (draws with TFT_eSPI, call it before lvgl starts, after the screen is initialized).
     */
    void touch_init( bool force_calibration );
    /** @brief true and the screen position while the screen is pressed */
    bool touch_get( int16_t *x, int16_t *y );

#endif // _TOUCH_H
