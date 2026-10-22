/* Decodage JPEG (baseline) en RGB565 dans la PSRAM, pour les illustrations des fiches. */
#ifndef _JPEG_DECODE_H
    #define _JPEG_DECODE_H

    #include <stddef.h>
    #include <stdint.h>

    /** @brief images above this many pixels are refused */
    #define JPEG_MAX_PIXELS     ( 320 * 240 )

    /**
     * @brief decode a JPEG
     * @param   data, size  the file
     * @param   pixels      on success, w * h RGB565 pixels (lv_color_t with LV_COLOR_16_SWAP 0) in PSRAM, the caller frees it
     * @return  true on success
     */
    bool jpeg_decode_rgb565( const uint8_t *data, size_t size, uint16_t **pixels, uint16_t *w, uint16_t *h );

#endif // _JPEG_DECODE_H
