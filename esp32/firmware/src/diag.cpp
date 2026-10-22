#include "diag.h"

#if SLV_DIAG

#include <Arduino.h>
#include <stdarg.h>
#include <esp_attr.h>
#include <esp_system.h>
#include <TFT_eSPI.h>
#include "board.h"

extern TFT_eSPI tft;

#define DIAG_MAGIC          0x534C5644u     /* "SLVD" */
#define DIAG_STEP_LEN       40
#define DIAG_LINE_HEIGHT    16
#define DIAG_SERIAL_WAIT_MS 2500

/* RTC memory that is not cleared by a crash or a software reset (it is by a power on) */
RTC_NOINIT_ATTR static uint32_t last_magic;
RTC_NOINIT_ATTR static char last_step[ DIAG_STEP_LEN ];

static esp_reset_reason_t reset_reason = ESP_RST_UNKNOWN;
static bool known_last_step = false;
static char previous_step[ DIAG_STEP_LEN ] = "";
static bool screen_ready = false;
static int16_t line_y = 4;
static uint32_t next_heartbeat = 0;

static const char *reset_reason_text( esp_reset_reason_t reason ) {
    switch ( reason ) {
        case ESP_RST_POWERON:   return( "mise sous tension" );
        case ESP_RST_EXT:       return( "reset externe" );
        case ESP_RST_SW:        return( "redemarrage logiciel" );
        case ESP_RST_PANIC:     return( "PLANTAGE (panic)" );
        case ESP_RST_INT_WDT:   return( "PLANTAGE (watchdog interruptions)" );
        case ESP_RST_TASK_WDT:  return( "PLANTAGE (watchdog tache)" );
        case ESP_RST_WDT:       return( "PLANTAGE (watchdog)" );
        case ESP_RST_DEEPSLEEP: return( "sortie de sommeil profond" );
        case ESP_RST_BROWNOUT:  return( "PLANTAGE (tension trop basse)" );
        default:                return( "inconnu" );
    }
}

bool diag_previous_run_crashed( void ) {
    return( reset_reason == ESP_RST_PANIC || reset_reason == ESP_RST_INT_WDT ||
            reset_reason == ESP_RST_TASK_WDT || reset_reason == ESP_RST_WDT ||
            reset_reason == ESP_RST_BROWNOUT );
}

void diag_init( void ) {
    Serial.begin( 115200 );
    /* sends the log_*() messages of the libraries to the USB serial port as well */
    Serial.setDebugOutput( true );

    /* the USB serial port needs a moment to show up on the computer: wait for it, but not for ever */
    uint32_t start = millis();
    while ( !Serial && millis() - start < DIAG_SERIAL_WAIT_MS )
        delay( 10 );

    reset_reason = esp_reset_reason();
    if ( reset_reason != ESP_RST_POWERON && last_magic == DIAG_MAGIC ) {
        last_step[ DIAG_STEP_LEN - 1 ] = 0;
        snprintf( previous_step, sizeof( previous_step ), "%s", last_step );
        known_last_step = true;
    }

    Serial.println();
    Serial.println( "==== Slovingo T-HMI : diagnostic ====" );
    Serial.printf( "puce        : %s rev %d, %d coeurs, %d MHz\n", ESP.getChipModel(), (int)ESP.getChipRevision(), (int)ESP.getChipCores(), (int)ESP.getCpuFreqMHz() );
    Serial.printf( "flash       : %u Mo\n", (unsigned)( ESP.getFlashChipSize() / ( 1024 * 1024 ) ) );
    Serial.printf( "PSRAM       : %u Mo, libre %u Ko\n", (unsigned)( ESP.getPsramSize() / ( 1024 * 1024 ) ), (unsigned)( ESP.getFreePsram() / 1024 ) );
    Serial.printf( "memoire     : libre %u Ko\n", (unsigned)( ESP.getFreeHeap() / 1024 ) );
    Serial.printf( "demarrage   : %s\n", reset_reason_text( reset_reason ) );
    if ( known_last_step )
        Serial.printf( "dernier jalon de l'execution precedente : %s\n", previous_step );
    Serial.println( "=====================================" );

    if ( ESP.getPsramSize() == 0 )
        Serial.println( "ATTENTION : pas de PSRAM detectee (memory_type qio_opi / BOARD_HAS_PSRAM ?)" );
}

void diag_step( const char *name ) {
    last_magic = DIAG_MAGIC;
    snprintf( last_step, sizeof( last_step ), "%s", name );
    Serial.printf( "[%6lu ms] %s\n", (unsigned long)millis(), name );
    if ( screen_ready )
        diag_line( TFT_CYAN, "%s", name );
}

void diag_line( uint16_t color, const char *format, ... ) {
    char text[ 96 ];
    va_list args;

    va_start( args, format );
    vsnprintf( text, sizeof( text ), format, args );
    va_end( args );
    Serial.println( text );

    if ( !screen_ready )
        return;
    if ( line_y > BOARD_HEIGHT - DIAG_LINE_HEIGHT ) {
        tft.fillScreen( TFT_BLACK );
        line_y = 4;
    }
    tft.setTextColor( color, TFT_BLACK );
    tft.drawString( text, 4, line_y, 2 );
    line_y += DIAG_LINE_HEIGHT;
}

void diag_screen_ready( void ) {
    /* the normal firmware shows something only after a crash (the serial port has the rest) */
    if ( diag_previous_run_crashed() )
        diag_screen_start( false );
}

void diag_screen_start( bool color_test ) {
    if ( color_test ) {
        /* a full screen of each colour: shows at once whether the display is driven and the colour order */
        const uint16_t colors[] = { TFT_RED, TFT_GREEN, TFT_BLUE };
        for ( uint8_t i = 0; i < 3; i++ ) {
            tft.fillScreen( colors[ i ] );
            delay( 350 );
        }
    }
    tft.fillScreen( TFT_BLACK );
    screen_ready = true;
    line_y = 4;

    diag_line( TFT_WHITE, "Slovingo T-HMI : diagnostic" );
    diag_line( TFT_WHITE, "demarrage : %s", reset_reason_text( reset_reason ) );
    if ( known_last_step )
        diag_line( diag_previous_run_crashed() ? TFT_RED : TFT_YELLOW, "dernier jalon avant : %s", previous_step );
    if ( ESP.getPsramSize() == 0 )
        diag_line( TFT_RED, "PAS DE PSRAM detectee" );

    /* long enough to read it when the previous run crashed */
    if ( diag_previous_run_crashed() )
        delay( 5000 );
}

void diag_screen_done( void ) {
    screen_ready = false;
}

void diag_heartbeat( void ) {
    uint32_t now = millis();
    if ( (int32_t)( now - next_heartbeat ) < 0 )
        return;
    next_heartbeat = now + 5000;
    last_magic = DIAG_MAGIC;
    snprintf( last_step, sizeof( last_step ), "loop" );
    Serial.printf( "[%6lu ms] loop : memoire libre %u Ko, PSRAM libre %u Ko\n", (unsigned long)now,
                   (unsigned)( ESP.getFreeHeap() / 1024 ), (unsigned)( ESP.getFreePsram() / 1024 ) );
}

#endif // SLV_DIAG
