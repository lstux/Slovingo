#include <Arduino.h>
#include <TFT_eSPI.h>
#include <esp_heap_caps.h>
#include "lvgl.h"
#include "board.h"
#include "touch.h"
#include "lvgl_port.h"

#define DRAW_BUFFER_LINES   40      /* two buffers of 40 lines in the internal RAM */

TFT_eSPI tft = TFT_eSPI();

static lv_disp_buf_t disp_buf;
static lv_disp_drv_t disp_drv;
static lv_indev_drv_t indev_drv;

static void flush_cb( lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p ) {
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;

    tft.startWrite();
    tft.setAddrWindow( area->x1, area->y1, w, h );
    tft.pushColors( (uint16_t*)&color_p->full, w * h, true );
    tft.endWrite();
    lv_disp_flush_ready( drv );
}

static bool touch_read_cb( lv_indev_drv_t *drv, lv_indev_data_t *data ) {
    static int16_t last_x = 0, last_y = 0;
    int16_t x, y;

    if ( touch_get( &x, &y ) ) {
        last_x = x;
        last_y = y;
        data->state = LV_INDEV_STATE_PR;
    }
    else {
        data->state = LV_INDEV_STATE_REL;
    }
    data->point.x = last_x;
    data->point.y = last_y;
    return( false );
}

void board_display_init( void ) {
    tft.begin();
    tft.setRotation( BOARD_ROTATION );
    tft.fillScreen( TFT_BLACK );
    board_backlight( true );
}

void lvgl_port_init( void ) {
    lv_init();

    size_t pixels = BOARD_WIDTH * DRAW_BUFFER_LINES;
    lv_color_t *buf1 = (lv_color_t*)heap_caps_malloc( pixels * sizeof( lv_color_t ), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT );
    lv_color_t *buf2 = (lv_color_t*)heap_caps_malloc( pixels * sizeof( lv_color_t ), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT );
    if ( !buf1 ) {
        log_e("no memory for the draw buffer");
        return;
    }
    lv_disp_buf_init( &disp_buf, buf1, buf2, pixels );

    lv_disp_drv_init( &disp_drv );
    disp_drv.hor_res = BOARD_WIDTH;
    disp_drv.ver_res = BOARD_HEIGHT;
    disp_drv.flush_cb = flush_cb;
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register( &disp_drv );

    lv_indev_drv_init( &indev_drv );
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touch_read_cb;
    lv_indev_drv_register( &indev_drv );
}
