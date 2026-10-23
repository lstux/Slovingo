/* LilyGo T-HMI : broches, alimentation, carte SD. */
#ifndef _BOARD_H
    #define _BOARD_H

    #include <stdint.h>

    #define PWR_EN_PIN          10      /** @brief a l'etat haut : alimente la carte SD et les peripheriques */
    #define PWR_ON_PIN          14      /** @brief a l'etat haut : garde l'appareil allume sur batterie */
    #define BAT_ADC_PIN         5       /** @brief batterie, pont diviseur par 2 */
    #define BUTTON_BOOT_PIN     0
    #define BUTTON_OK_PIN       21

    #define TOUCH_SCLK_PIN      1
    #define TOUCH_MISO_PIN      4
    #define TOUCH_MOSI_PIN      3
    #define TOUCH_CS_PIN        2
    #define TOUCH_IRQ_PIN       9

    #define SD_CLK_PIN          12
    #define SD_CMD_PIN          11
    #define SD_D0_PIN           13

    /**
     * @brief rotation de l'ecran
     *
     * Portrait (-D SLV_PORTRAIT=1) : 0, ou 2 si l'image est a l'envers.
     * Paysage : 1, ou 3 si l'image est a l'envers.
     * Apres un changement le tactile se recalibre tout seul (la calibration enregistree dit pour quelle rotation elle vaut).
     */
    #ifdef SLV_PORTRAIT
        #define BOARD_ROTATION      0
        #define BOARD_WIDTH         240
        #define BOARD_HEIGHT        320
    #else
        #define BOARD_ROTATION      1
        #define BOARD_WIDTH         320
        #define BOARD_HEIGHT        240
    #endif

    /** @brief power the board and the peripherals, call first in setup() */
    void board_init( void );
    /** @brief mount the SD card (SD_MMC, 1 bit), false when there is none */
    bool board_sd_mount( void );
    /** @brief battery voltage in mV */
    uint32_t board_battery_mv( void );
    /** @brief switch the backlight on or off */
    void board_backlight( bool on );
    /** @brief switch off: on battery the board is cut, on USB it sleeps until the OK button is pressed */
    void board_power_off( void );

#endif // _BOARD_H
