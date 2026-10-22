/*
 * Diagnostic de demarrage : sortie serie, jalons, test de l'ecran, et memoire du dernier jalon
 * franchi (elle survit a un plantage), pour savoir ou le programme s'arrete quand rien ne s'affiche.
 *
 * Actif avec -D SLV_DIAG=1 (platformio.ini). Sans, tout ceci ne fait rien.
 */
#ifndef _DIAG_H
    #define _DIAG_H

    #include <stdint.h>

    #ifndef SLV_DIAG
        #define SLV_DIAG 0
    #endif

    #if SLV_DIAG

    /** @brief serial port, banner (chip, memory, reset reason) and what the last run stopped on */
    void diag_init( void );
    /** @brief a step of the start: written on the serial port and kept over a crash */
    void diag_step( const char *name );
    /** @brief after tft.begin(): the reset info on the screen, only when the previous run crashed */
    void diag_screen_ready( void );
    /** @brief text on the screen (the diagnostic program calls it with the red, green, blue test) */
    void diag_screen_start( bool color_test );
    /** @brief stop drawing on the screen (call before lvgl takes it over); the serial port stays */
    void diag_screen_done( void );
    /** @brief a line of text on the screen (and the serial port) */
    void diag_line( uint16_t color, const char *format, ... );
    /** @brief call from loop(): every 5 s, uptime and free memory on the serial port */
    void diag_heartbeat( void );
    /** @brief true when the previous run ended in a crash or a watchdog reset */
    bool diag_previous_run_crashed( void );

    #else

    static inline void diag_init( void ) {}
    static inline void diag_step( const char *name ) { (void)name; }
    static inline void diag_screen_ready( void ) {}
    static inline void diag_screen_start( bool color_test ) { (void)color_test; }
    static inline void diag_screen_done( void ) {}
    static inline void diag_heartbeat( void ) {}
    #define diag_line( ... )    ((void)0)

    #endif

#endif // _DIAG_H
