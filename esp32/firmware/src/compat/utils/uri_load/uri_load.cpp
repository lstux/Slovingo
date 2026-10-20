#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "utils/alloc.h"
#include "utils/uri_load/uri_load.h"

uri_load_dsc_t *uri_load_to_ram( const char *uri ) {
    if ( !uri || !WiFi.isConnected() )
        return( NULL );

    /*
     * https without checking the server certificate: the courses are public files, nothing
     * secret goes through. (A certificate bundle can replace setInsecure() later.)
     */
    WiFiClientSecure secure;
    WiFiClient plain;
    HTTPClient http;
    bool is_https = strncmp( uri, "https://", 8 ) == 0;

    if ( is_https ) {
        secure.setInsecure();
        http.begin( secure, uri );
    }
    else {
        http.begin( plain, uri );
    }
    http.setFollowRedirects( HTTPC_STRICT_FOLLOW_REDIRECTS );
    http.setRedirectLimit( URI_LOAD_MAX_REDIRECT );
    http.setConnectTimeout( 8000 );
    http.setTimeout( URI_LOAD_STREAM_TIMEOUT );
    http.setUserAgent( "Slovingo-THMI" );

    int code = http.GET();
    if ( code != HTTP_CODE_OK ) {
        log_w("GET %s: http %d", uri, code );
        http.end();
        return( NULL );
    }

    int length = http.getSize();     /* -1 when the length is not announced (chunked answer) */
    uint8_t *data = NULL;
    size_t size = 0;

    if ( length > URI_LOAD_MAX_SIZE ) {
        log_w("%s: %d bytes is too big", uri, length );
        http.end();
        return( NULL );
    }

    if ( length > 0 ) {
        /* known length: read the stream straight into a PSRAM buffer */
        data = (uint8_t*)MALLOC( length + 1 );
        if ( !data ) {
            http.end();
            return( NULL );
        }
        WiFiClient *stream = http.getStreamPtr();
        uint32_t last_data = millis();

        while ( size < (size_t)length && ( http.connected() || stream->available() ) ) {
            int avail = stream->available();
            if ( avail <= 0 ) {
                if ( millis() - last_data > URI_LOAD_STREAM_TIMEOUT )
                    break;
                delay( 5 );
                continue;
            }
            if ( size + avail > (size_t)length )
                avail = length - size;
            int got = stream->readBytes( data + size, avail );
            if ( got > 0 ) {
                size += got;
                last_data = millis();
            }
        }
    }
    else {
        /* chunked answer: getString() decodes the chunks */
        String body = http.getString();
        size = body.length();
        if ( size && size <= URI_LOAD_MAX_SIZE ) {
            data = (uint8_t*)MALLOC( size + 1 );
            if ( data )
                memcpy( data, body.c_str(), size );
        }
    }
    http.end();

    if ( !data || size == 0 || ( length > 0 && size < (size_t)length ) ) {
        log_w("%s: got %d of %d bytes", uri, (int)size, length );
        free( data );
        return( NULL );
    }
    data[ size ] = 0;

    uri_load_dsc_t *dsc = (uri_load_dsc_t*)CALLOC( 1, sizeof( uri_load_dsc_t ) );
    if ( !dsc ) {
        free( data );
        return( NULL );
    }
    dsc->uri = strdup( uri );
    dsc->size = size;
    dsc->timestamp = millis();
    dsc->data = data;
    return( dsc );
}

void uri_load_free_all( uri_load_dsc_t *uri_load_dsc ) {
    if ( !uri_load_dsc )
        return;
    free( uri_load_dsc->data );
    free( uri_load_dsc->uri );
    free( uri_load_dsc->filename );
    free( uri_load_dsc );
}
