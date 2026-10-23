/****************************************************************************
 *   Slovingo app for the T-Watch: download, cache and text helpers
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef NATIVE_64BIT
    #include <Arduino.h>
    #include <WiFi.h>
    #include <SPIFFS.h>
    #include <FS.h>
    #include "hardware/wifictl.h"
#endif

#include "slovingo_fetch.h"
#include "utils/alloc.h"
#include "utils/uri_load/uri_load.h"

/*
 * the SPIFFS keeps a file name to 31 characters, so the cache file is named after
 * a hash of the url
 */
#define SLOVINGO_CACHE_NAME_LEN     24
#define SLOVINGO_CACHE_RESERVE      16384   /** @brief free space always left on the SPIFFS */

static uint32_t slovingo_hash( const char *text ) {
    uint32_t hash = 2166136261u;
    while ( *text ) {
        hash ^= (uint8_t)*text++;
        hash *= 16777619u;
    }
    return( hash );
}

/*
 * parse into a PSRAM document, strings are copied so the source buffer can go
 */
static SpiRamJsonDocument *slovingo_parse( const char *data, size_t size ) {
    SpiRamJsonDocument *doc = new SpiRamJsonDocument( size * 4 + 512 );

    if ( !doc )
        return( NULL );
    if ( doc->capacity() == 0 ) {
        delete doc;
        return( NULL );
    }

    DeserializationError error = deserializeJson( *doc, data, size );
    if ( error ) {
        log_e("slovingo json error: %s (%d bytes)", error.c_str(), (int)size );
        delete doc;
        return( NULL );
    }
    return( doc );
}

#ifndef NATIVE_64BIT

static void slovingo_cache_name( const char *url, char *name, size_t len ) {
    snprintf( name, len, "/slv%08x.json", (unsigned int)slovingo_hash( url ) );
}

static void slovingo_cache_store( const char *name, const uint8_t *data, size_t size ) {
    if ( SPIFFS.totalBytes() - SPIFFS.usedBytes() < size + SLOVINGO_CACHE_RESERVE ) {
        log_w("slovingo: not enough SPIFFS space to cache %s", name );
        return;
    }
    fs::File file = SPIFFS.open( name, FILE_WRITE );
    if ( !file )
        return;
    file.write( data, size );
    file.close();
}

static SpiRamJsonDocument *slovingo_cache_load( const char *name ) {
    if ( !SPIFFS.exists( name ) )
        return( NULL );

    fs::File file = SPIFFS.open( name, FILE_READ );
    if ( !file )
        return( NULL );

    size_t size = file.size();
    char *buffer = (char*)MALLOC( size + 1 );
    if ( !buffer ) {
        file.close();
        return( NULL );
    }
    size_t got = file.read( (uint8_t*)buffer, size );
    file.close();

    SpiRamJsonDocument *doc = slovingo_parse( buffer, got );
    free( buffer );
    return( doc );
}

#endif // NATIVE_64BIT

#if defined( SLV_BIG_SCREEN ) && !defined( NATIVE_64BIT )

static int slovingo_cache_hours = 12;
static uint32_t slovingo_wifi_failed_at = 0;        /* millis() of the last wifi that did not connect, 0 none */

void slovingo_set_cache_hours( int hours ) {
    slovingo_cache_hours = hours < 0 ? 0 : hours;
}

/* a cached file younger than the limit; the clock is only right once the wifi was connected, until then nothing is "young" */
static bool slovingo_cache_fresh( const char *name ) {
    time_t now = time( NULL );

    if ( slovingo_cache_hours <= 0 || now < 1700000000 || !SPIFFS.exists( name ) )
        return( false );
    fs::File file = SPIFFS.open( name, FILE_READ );
    if ( !file )
        return( false );
    time_t written = file.getLastWrite();
    file.close();
    return( written >= 1700000000 && now >= written && ( now - written ) < (time_t)slovingo_cache_hours * 3600 );
}

#endif

slovingo_result_t slovingo_fetch_json( const char *base_url, const char *path, bool force_net, SpiRamJsonDocument **doc ) {
    char url[ 256 ];
    size_t base_len = strlen( base_url );

    *doc = NULL;
    snprintf( url, sizeof( url ), "%s%s%s", base_url, ( base_len && base_url[ base_len - 1 ] == '/' ) ? "" : "/", path );
    log_d("slovingo fetch: %s", url );

#ifndef NATIVE_64BIT
    char cache[ SLOVINGO_CACHE_NAME_LEN ];
    slovingo_cache_name( url, cache, sizeof( cache ) );

    bool online = WiFi.isConnected();

#ifdef SLV_BIG_SCREEN
    /* a young cache is enough; an old one gives way to the live version when there is a network */
    if ( !force_net && slovingo_cache_fresh( cache ) ) {
        *doc = slovingo_cache_load( cache );
        if ( *doc )
            return( SLOVINGO_OK );
    }
    if ( !online && slovingo_wifi_failed_at && millis() - slovingo_wifi_failed_at < SLOVINGO_WIFI_RETRY_MS ) {
        *doc = slovingo_cache_load( cache );
        return( *doc ? SLOVINGO_OK : SLOVINGO_ERR_NOWIFI );
    }
#else
    if ( !online && !force_net ) {
        *doc = slovingo_cache_load( cache );
        if ( *doc )
            return( SLOVINGO_OK );
    }
#endif
    if ( !online ) {
        wifictl_on();
        for ( int waited = 0; waited < SLOVINGO_WIFI_WAIT_MS && !WiFi.isConnected(); waited += 250 )
            vTaskDelay( 250 / portTICK_PERIOD_MS );
        online = WiFi.isConnected();
#ifdef SLV_BIG_SCREEN
        slovingo_wifi_failed_at = online ? 0 : ( millis() ? millis() : 1 );
#endif
        if ( !online ) {
            *doc = slovingo_cache_load( cache );
            return( *doc ? SLOVINGO_OK : SLOVINGO_ERR_NOWIFI );
        }
    }
#endif

    /*
     * a download that fails is tried again (the wifi or the tls handshake is
     * often not ready the first time), a document that does not parse is not
     */
    slovingo_result_t result = SLOVINGO_ERR_NET;

    for ( int attempt = 0; attempt < SLOVINGO_TRIES && result == SLOVINGO_ERR_NET; attempt++ ) {
#ifndef NATIVE_64BIT
        if ( attempt ) {
            log_w("slovingo: download failed, try %d of %d", attempt + 1, SLOVINGO_TRIES );
            vTaskDelay( ( attempt * SLOVINGO_RETRY_WAIT_MS ) / portTICK_PERIOD_MS );
            if ( !WiFi.isConnected() )
                continue;
        }
#endif
        uri_load_dsc_t *uri_load_dsc = uri_load_to_ram( url );

        if ( uri_load_dsc ) {
            if ( uri_load_dsc->data && uri_load_dsc->size ) {
                *doc = slovingo_parse( (const char*)uri_load_dsc->data, uri_load_dsc->size );
                if ( *doc ) {
                    result = SLOVINGO_OK;
#ifndef NATIVE_64BIT
                    slovingo_cache_store( cache, uri_load_dsc->data, uri_load_dsc->size );
#endif
                }
                else {
                    result = SLOVINGO_ERR_PARSE;
                }
            }
            uri_load_free_all( uri_load_dsc );
        }
    }

#ifndef NATIVE_64BIT
    if ( !*doc ) {
        *doc = slovingo_cache_load( cache );
        if ( *doc )
            result = SLOVINGO_OK;
    }
#endif
    return( result );
}

#if defined( SLV_BIG_SCREEN ) && !defined( NATIVE_64BIT )

static void slovingo_image_names( const char *base_url, const char *path, char *url, size_t url_len, char *name, size_t name_len ) {
    size_t base_len = strlen( base_url );

    snprintf( url, url_len, "%s%s%s", base_url, ( base_len && base_url[ base_len - 1 ] == '/' ) ? "" : "/", path );
    snprintf( name, name_len, "/slv%08x.jpg", (unsigned int)slovingo_hash( url ) );
}

/* "slv" + 8 hexadecimal digits + ".json" or ".jpg": the files this module writes */
static bool slovingo_is_cache_file( const char *name ) {
    if ( *name == '/' )
        name++;
    if ( strncmp( name, "slv", 3 ) )
        return( false );
    for ( int i = 3; i < 11; i++ ) {
        char c = name[ i ];
        if ( !( ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'f' ) ) )
            return( false );
    }
    return( !strcmp( name + 11, ".json" ) || !strcmp( name + 11, ".jpg" ) );
}

int slovingo_cache_clear( void ) {
    int deleted = 0;

    /* the directory is read again after each batch: removing files while walking it is not safe */
    for ( int round = 0; round < 20; round++ ) {
        char names[ 16 ][ SLOVINGO_CACHE_NAME_LEN ];
        int found = 0;

        fs::File root = SPIFFS.open( "/" );
        if ( !root )
            break;
        for ( fs::File file = root.openNextFile(); file && found < 16; file = root.openNextFile() ) {
            const char *name = file.name();
            if ( !file.isDirectory() && slovingo_is_cache_file( name ) )
                snprintf( names[ found++ ], SLOVINGO_CACHE_NAME_LEN, "%s%s", *name == '/' ? "" : "/", name );
        }
        root.close();
        if ( !found )
            break;
        for ( int i = 0; i < found; i++ ) {
            if ( SPIFFS.remove( names[ i ] ) )
                deleted++;
        }
    }
    return( deleted );
}

bool slovingo_image_prefetch( const char *base_url, const char *path ) {
    char url[ 256 ];
    char name[ SLOVINGO_CACHE_NAME_LEN ];

    slovingo_image_names( base_url, path, url, sizeof( url ), name, sizeof( name ) );
    bool cached = SPIFFS.exists( name );

    if ( cached && slovingo_cache_fresh( name ) )
        return( true );
    if ( !WiFi.isConnected() )
        return( cached );

    for ( int attempt = 0; attempt < 2; attempt++ ) {
        if ( attempt )
            vTaskDelay( SLOVINGO_RETRY_WAIT_MS / portTICK_PERIOD_MS );
        uri_load_dsc_t *dsc = uri_load_to_ram( url );
        if ( !dsc )
            continue;
        bool stored = false;
        if ( dsc->data && dsc->size && dsc->size <= SLOVINGO_IMAGE_MAX_BYTES ) {
            slovingo_cache_store( name, dsc->data, dsc->size );
            stored = SPIFFS.exists( name );
        }
        uri_load_free_all( dsc );
        return( stored || cached );
    }
    return( cached );
}

uint8_t *slovingo_image_load( const char *base_url, const char *path, size_t *size ) {
    char url[ 256 ];
    char name[ SLOVINGO_CACHE_NAME_LEN ];

    *size = 0;
    slovingo_image_names( base_url, path, url, sizeof( url ), name, sizeof( name ) );
    if ( !SPIFFS.exists( name ) )
        return( NULL );

    fs::File file = SPIFFS.open( name, FILE_READ );
    if ( !file )
        return( NULL );
    size_t length = file.size();
    uint8_t *buffer = length ? (uint8_t*)MALLOC( length ) : NULL;
    if ( buffer ) {
        size_t got = file.read( buffer, length );
        if ( got != length ) {
            free( buffer );
            buffer = NULL;
            got = 0;
        }
        *size = got;
    }
    file.close();
    return( buffer );
}

#endif

const char *slovingo_result_text( slovingo_result_t result ) {
    switch ( result ) {
        case SLOVINGO_OK:           return( "OK" );
        case SLOVINGO_ERR_NOWIFI:   return( "Pas de Wi-Fi et rien en cache" );
        case SLOVINGO_ERR_NET:      return( "Téléchargement impossible" );
        case SLOVINGO_ERR_PARSE:    return( "Données invalides" );
        case SLOVINGO_ERR_MEM:      return( "Mémoire insuffisante" );
    }
    return( "Erreur" );
}

/* "Liska", until the config says otherwise */
static char slovingo_user_name[ 32 ] = "L\xC3\xAD\xC5\xA1ka";

void slovingo_set_user_name( const char *name ) {
    char clean[ sizeof( slovingo_user_name ) ];

    slovingo_sanitize( name ? name : "", clean, sizeof( clean ) );
    if ( *clean )
        memcpy( slovingo_user_name, clean, sizeof( clean ) );
}

void slovingo_sanitize( const char *in, char *out, size_t out_size ) {
    const uint8_t *p = (const uint8_t*)in;
    size_t o = 0;

    if ( !out_size )
        return;

    while ( *p ) {
        uint32_t cp;
        int len;

        if ( *p == '[' && !strncmp( (const char*)p, "[USER_NAME]", 11 ) ) {
            size_t n = strlen( slovingo_user_name );
            if ( o + n + 1 > out_size )
                break;
            memcpy( out + o, slovingo_user_name, n );
            o += n;
            p += 11;
            continue;
        }

        if ( *p < 0x80 )                    { cp = *p;        len = 1; }
        else if ( ( *p & 0xE0 ) == 0xC0 )   { cp = *p & 0x1F; len = 2; }
        else if ( ( *p & 0xF0 ) == 0xE0 )   { cp = *p & 0x0F; len = 3; }
        else if ( ( *p & 0xF8 ) == 0xF0 )   { cp = *p & 0x07; len = 4; }
        else                                { p++; continue; }

        int i;
        for ( i = 1; i < len; i++ ) {
            if ( ( p[ i ] & 0xC0 ) != 0x80 )
                break;
            cp = ( cp << 6 ) | ( p[ i ] & 0x3F );
        }
        if ( i < len ) {                    /* broken sequence, skip one byte */
            p++;
            continue;
        }

        const uint8_t *start = p;
        p += len;

        if ( cp == '\n' || ( cp >= 0x20 && cp <= 0x7E ) || ( cp >= 0xA0 && cp <= 0x17F ) ) {
            if ( o + len + 1 > out_size )
                break;
            memcpy( out + o, start, len );
            o += len;
            continue;
        }

        const char *replacement = NULL;
        switch ( cp ) {
            case 0x2010: case 0x2011: case 0x2012: case 0x2013: case 0x2014: case 0x2015: case 0x2212:
                            replacement = "-"; break;
            case 0x2018: case 0x2019: case 0x201A: case 0x2032:
                            replacement = "'"; break;
            case 0x201C: case 0x201D: case 0x201E: case 0x2033:
                            replacement = "\""; break;
            case 0x2026:    replacement = "..."; break;
            case 0x2022:    replacement = "-"; break;
            case 0x2192: case 0x25B6: case 0x203A:
                            replacement = ">"; break;
            case 0x2039:    replacement = "<"; break;
            case 0x2190:    replacement = "<"; break;
            case 0x2009: case 0x200A: case 0x202F: case 0x2002: case 0x2003: case '\t':
                            replacement = " "; break;
            default:        break;          /* emojis, variation selectors, ... are dropped */
        }
        if ( replacement ) {
            size_t n = strlen( replacement );
            if ( o + n + 1 > out_size )
                break;
            memcpy( out + o, replacement, n );
            o += n;
        }
    }
    out[ o ] = '\0';
}
