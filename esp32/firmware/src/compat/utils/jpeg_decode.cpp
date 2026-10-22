#include <Arduino.h>
#include <TJpg_Decoder.h>
#include "utils/alloc.h"
#include "utils/jpeg_decode.h"

static uint16_t *out_pixels = NULL;
static uint16_t out_w = 0;
static uint16_t out_h = 0;

/* the decoder gives the picture block by block */
static bool jpeg_block( int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap ) {
    if ( !out_pixels )
        return( false );
    for ( uint16_t row = 0; row < h; row++ ) {
        int yy = y + row;
        if ( yy < 0 || yy >= out_h )
            continue;
        for ( uint16_t col = 0; col < w; col++ ) {
            int xx = x + col;
            if ( xx >= 0 && xx < out_w )
                out_pixels[ (size_t)yy * out_w + xx ] = bitmap[ (size_t)row * w + col ];
        }
    }
    return( true );
}

bool jpeg_decode_rgb565( const uint8_t *data, size_t size, uint16_t **pixels, uint16_t *w, uint16_t *h ) {
    uint16_t jw = 0;
    uint16_t jh = 0;

    *pixels = NULL;
    if ( !data || !size )
        return( false );

    TJpgDec.setJpgScale( 1 );
    TJpgDec.setSwapBytes( false );
    TJpgDec.setCallback( jpeg_block );

    if ( TJpgDec.getJpgSize( &jw, &jh, data, size ) != 0 || !jw || !jh || (uint32_t)jw * jh > JPEG_MAX_PIXELS ) {
        log_w("jpeg: unreadable or too big (%dx%d)", (int)jw, (int)jh );
        return( false );
    }

    uint16_t *buffer = (uint16_t*)MALLOC( (size_t)jw * jh * sizeof( uint16_t ) );
    if ( !buffer )
        return( false );
    memset( buffer, 0, (size_t)jw * jh * sizeof( uint16_t ) );

    out_pixels = buffer;
    out_w = jw;
    out_h = jh;
    int result = TJpgDec.drawJpg( 0, 0, data, size );
    out_pixels = NULL;

    if ( result != 0 ) {
        log_w("jpeg: decoding failed (%d)", result );
        free( buffer );
        return( false );
    }
    *pixels = buffer;
    *w = jw;
    *h = jh;
    return( true );
}
