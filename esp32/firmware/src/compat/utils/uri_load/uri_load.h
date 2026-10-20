/* Telechargement HTTP(S) en RAM : version reduite de uri_load.h de la montre. */
#ifndef _URI_LOAD_H
    #define _URI_LOAD_H

    #include <stdint.h>

    #define URI_LOAD_MAX_REDIRECT       5
    #define URI_LOAD_STREAM_TIMEOUT     15000               /** @brief patience for the first byte, in ms */
    #define URI_LOAD_MAX_SIZE           ( 1024 * 1024 )     /** @brief biggest download accepted */

    typedef struct {
        char *filename;             /** @brief unused, NULL */
        char *uri;                  /** @brief copy of the requested uri */
        uint32_t size;              /** @brief size of data in bytes */
        uint64_t timestamp;         /** @brief download time, millis() */
        uint8_t *data;              /** @brief downloaded bytes (one more, zero, after them) */
    } uri_load_dsc_t;

    /** @brief download a file into RAM, NULL when the request fails; free the result with uri_load_free_all() */
    uri_load_dsc_t *uri_load_to_ram( const char *uri );
    void uri_load_free_all( uri_load_dsc_t *uri_load_dsc );

#endif // _URI_LOAD_H
