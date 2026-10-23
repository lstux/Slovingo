#include <Arduino.h>
#include <SPI.h>
#include <SD_MMC.h>
#include <TFT_eSPI.h>
#include <ArduinoJson.h>
#include "board.h"
#include "touch.h"

#define TOUCH_CAL_FILE      "/touch_cal.json"
#define TOUCH_MARGIN        20      /* distance of the crosses from the edges */
#define TOUCH_SAMPLES       12

extern TFT_eSPI tft;

/* calibration: screen x comes from the raw channel (swap ? Y : X), same for y */
typedef struct {
    bool swap;
    int16_t x0, x1;     /* raw values at the left and right crosses */
    int16_t y0, y1;     /* raw values at the top and bottom crosses */
} touch_cal_t;

static touch_cal_t cal;
static bool cal_valid = false;

static int16_t best_two_average( int16_t a, int16_t b, int16_t c ) {
    int16_t ab = abs( a - b ), ac = abs( a - c ), bc = abs( b - c );
    if ( ab <= ac && ab <= bc )
        return( ( a + b ) / 2 );
    if ( ac <= ab && ac <= bc )
        return( ( a + c ) / 2 );
    return( ( b + c ) / 2 );
}

/* raw 12 bit reading, false when the screen is not pressed */
static bool touch_raw( int16_t *rx, int16_t *ry ) {
    if ( digitalRead( TOUCH_IRQ_PIN ) == HIGH )
        return( false );

    int16_t data[ 6 ];
    SPI.beginTransaction( SPISettings( 2000000, MSBFIRST, SPI_MODE0 ) );
    digitalWrite( TOUCH_CS_PIN, LOW );
    SPI.transfer16( 0xB0 );                   /* Z1, thrown away */
    SPI.transfer16( 0xC0 );                   /* Z2, thrown away */
    data[ 0 ] = SPI.transfer16( 0x90 ) >> 4;  /* X */
    data[ 1 ] = SPI.transfer16( 0xD0 ) >> 4;  /* Y */
    data[ 2 ] = SPI.transfer16( 0x90 ) >> 4;
    data[ 3 ] = SPI.transfer16( 0xD0 ) >> 4;
    data[ 4 ] = SPI.transfer16( 0x90 ) >> 4;
    data[ 5 ] = SPI.transfer16( 0xD0 ) >> 4;  /* last read powers the chip down */
    SPI.transfer16( 0 );
    digitalWrite( TOUCH_CS_PIN, HIGH );
    SPI.endTransaction();

    *rx = best_two_average( data[ 0 ], data[ 2 ], data[ 4 ] );
    *ry = best_two_average( data[ 1 ], data[ 3 ], data[ 5 ] );
    return( true );
}

static int16_t scale( int16_t raw, int16_t raw0, int16_t raw1, int16_t pos0, int16_t pos1 ) {
    if ( raw1 == raw0 )
        return( pos0 );
    return( (int16_t)( pos0 + (int32_t)( raw - raw0 ) * ( pos1 - pos0 ) / ( raw1 - raw0 ) ) );
}

bool touch_get( int16_t *x, int16_t *y ) {
    int16_t rx, ry;

    if ( !cal_valid || !touch_raw( &rx, &ry ) )
        return( false );

    int16_t sx = scale( cal.swap ? ry : rx, cal.x0, cal.x1, TOUCH_MARGIN, BOARD_WIDTH - TOUCH_MARGIN );
    int16_t sy = scale( cal.swap ? rx : ry, cal.y0, cal.y1, TOUCH_MARGIN, BOARD_HEIGHT - TOUCH_MARGIN );
    *x = constrain( sx, 0, BOARD_WIDTH - 1 );
    *y = constrain( sy, 0, BOARD_HEIGHT - 1 );
    return( true );
}

static bool cal_load( void ) {
    fs::File file = SD_MMC.open( TOUCH_CAL_FILE, FILE_READ );
    if ( !file )
        return( false );
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson( doc, file );
    file.close();
    if ( error || !doc.containsKey( "x0" ) )
        return( false );
    /* a calibration made for another rotation (or an older file without it: landscape) does not apply */
    if ( (int)( doc["rot"] | 1 ) != BOARD_ROTATION )
        return( false );
    cal.swap = doc["swap"] | false;
    cal.x0 = doc["x0"];
    cal.x1 = doc["x1"];
    cal.y0 = doc["y0"];
    cal.y1 = doc["y1"];
    return( cal.x0 != cal.x1 && cal.y0 != cal.y1 );
}

static void cal_save( void ) {
    fs::File file = SD_MMC.open( TOUCH_CAL_FILE, FILE_WRITE );
    if ( !file )
        return;
    StaticJsonDocument<256> doc;
    doc["rot"] = BOARD_ROTATION;
    doc["swap"] = cal.swap;
    doc["x0"] = cal.x0;
    doc["x1"] = cal.x1;
    doc["y0"] = cal.y0;
    doc["y1"] = cal.y1;
    serializeJson( doc, file );
    file.close();
}

static void draw_cross( int16_t x, int16_t y, uint16_t color ) {
    tft.drawLine( x - 10, y, x + 10, y, color );
    tft.drawLine( x, y - 10, x, y + 10, color );
    tft.drawCircle( x, y, 5, color );
}

/* wait for a press on the cross, average a few readings, wait for the release */
static void cal_point( int16_t sx, int16_t sy, int16_t *rx, int16_t *ry ) {
    tft.fillScreen( TFT_BLACK );
    tft.setTextColor( TFT_WHITE, TFT_BLACK );
    tft.drawCentreString( "Touche chaque croix", BOARD_WIDTH / 2, BOARD_HEIGHT / 2 - 8, 2 );
    draw_cross( sx, sy, TFT_GREEN );

    int16_t x, y;
    while ( touch_raw( &x, &y ) )
        delay( 10 );
    delay( 200 );
    while ( !touch_raw( &x, &y ) )
        delay( 10 );

    int32_t sum_x = 0, sum_y = 0;
    int count = 0;
    while ( count < TOUCH_SAMPLES ) {
        if ( touch_raw( &x, &y ) ) {
            sum_x += x;
            sum_y += y;
            count++;
        }
        delay( 15 );
    }
    *rx = sum_x / count;
    *ry = sum_y / count;
    draw_cross( sx, sy, TFT_RED );
    while ( touch_raw( &x, &y ) )
        delay( 10 );
    delay( 200 );
}

static void cal_run( void ) {
    int16_t left = TOUCH_MARGIN, right = BOARD_WIDTH - TOUCH_MARGIN;
    int16_t top = TOUCH_MARGIN, bottom = BOARD_HEIGHT - TOUCH_MARGIN;
    int16_t tl_x, tl_y, tr_x, tr_y, bl_x, bl_y, br_x, br_y;

    cal_point( left, top, &tl_x, &tl_y );
    cal_point( right, top, &tr_x, &tr_y );
    cal_point( left, bottom, &bl_x, &bl_y );
    cal_point( right, bottom, &br_x, &br_y );

    /* the raw channel that changes most from left to right is the screen x */
    int16_t dx_x = ( tr_x + br_x ) / 2 - ( tl_x + bl_x ) / 2;
    int16_t dx_y = ( tr_y + br_y ) / 2 - ( tl_y + bl_y ) / 2;
    cal.swap = abs( dx_y ) > abs( dx_x );

    if ( !cal.swap ) {
        cal.x0 = ( tl_x + bl_x ) / 2;   cal.x1 = ( tr_x + br_x ) / 2;
        cal.y0 = ( tl_y + tr_y ) / 2;   cal.y1 = ( bl_y + br_y ) / 2;
    }
    else {
        cal.x0 = ( tl_y + bl_y ) / 2;   cal.x1 = ( tr_y + br_y ) / 2;
        cal.y0 = ( tl_x + tr_x ) / 2;   cal.y1 = ( bl_x + br_x ) / 2;
    }
    cal_save();
    log_i("touch calibration: swap=%d x %d..%d y %d..%d", cal.swap, cal.x0, cal.x1, cal.y0, cal.y1 );

    tft.fillScreen( TFT_BLACK );
}

void touch_init( bool force_calibration ) {
    pinMode( TOUCH_CS_PIN, OUTPUT );
    digitalWrite( TOUCH_CS_PIN, HIGH );
    pinMode( TOUCH_IRQ_PIN, INPUT_PULLUP );
    SPI.begin( TOUCH_SCLK_PIN, TOUCH_MISO_PIN, TOUCH_MOSI_PIN );

    if ( !force_calibration && cal_load() ) {
        cal_valid = true;
        return;
    }
    cal_run();
    cal_valid = true;
}
