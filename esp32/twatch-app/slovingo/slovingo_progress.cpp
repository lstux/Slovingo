/****************************************************************************
 *   Slovingo app for the T-Watch: best score of each sheet
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
#include <string.h>

#ifndef NATIVE_64BIT
    #include <Arduino.h>
    #include <SPIFFS.h>
    #include <FS.h>
#endif

#include "slovingo_progress.h"
#include "utils/json_psram_allocator.h"

/* {"best":{"<8 hex digits>":percent,...}}, about 16 bytes a sheet */
#define PROGRESS_DOC_SIZE   ( SLOVINGO_PROGRESS_MAX * 40 + 512 )

static SpiRamJsonDocument *progress_doc = NULL;
static bool progress_loaded = false;

static uint32_t progress_hash( const char *course_url, const char *sheet_id ) {
    uint32_t hash = 2166136261u;
    const char *parts[ 3 ] = { course_url, "|", sheet_id };

    for ( int i = 0; i < 3; i++ ) {
        for ( const char *p = parts[ i ]; *p; p++ ) {
            hash ^= (uint8_t)*p;
            hash *= 16777619u;
        }
    }
    return( hash );
}

static void progress_key( const char *course_url, const char *sheet_id, char *key, size_t len ) {
    snprintf( key, len, "%08x", (unsigned int)progress_hash( course_url, sheet_id ) );
}

static void progress_load( void ) {
    if ( progress_loaded )
        return;
    progress_loaded = true;

    progress_doc = new SpiRamJsonDocument( PROGRESS_DOC_SIZE );
    if ( !progress_doc || progress_doc->capacity() == 0 ) {
        delete progress_doc;
        progress_doc = NULL;
        return;
    }
#ifndef NATIVE_64BIT
    if ( SPIFFS.exists( SLOVINGO_PROGRESS_FILE ) ) {
        fs::File file = SPIFFS.open( SLOVINGO_PROGRESS_FILE, FILE_READ );
        if ( file ) {
            DeserializationError error = deserializeJson( *progress_doc, file );
            file.close();
            if ( error )
                progress_doc->clear();      /* damaged file: start again */
        }
    }
#endif
    if ( !progress_doc->containsKey( "best" ) )
        progress_doc->createNestedObject( "best" );
}

static void progress_save( void ) {
#ifndef NATIVE_64BIT
    fs::File file = SPIFFS.open( SLOVINGO_PROGRESS_FILE, FILE_WRITE );
    if ( !file )
        return;
    serializeJson( *progress_doc, file );
    file.close();
#endif
}

int slovingo_progress_best( const char *course_url, const char *sheet_id ) {
    char key[ 9 ];

    progress_load();
    if ( !progress_doc || !sheet_id || !*sheet_id )
        return( -1 );
    progress_key( course_url, sheet_id, key, sizeof( key ) );
    JsonVariant value = (*progress_doc)["best"][ key ];
    return( value.isNull() ? -1 : value.as<int>() );
}

bool slovingo_progress_record( const char *course_url, const char *sheet_id, int pct ) {
    char key[ 9 ];

    progress_load();
    if ( !progress_doc || !sheet_id || !*sheet_id )
        return( false );
    if ( pct < 0 )
        pct = 0;
    if ( pct > 100 )
        pct = 100;

    JsonObject best = (*progress_doc)["best"];
    progress_key( course_url, sheet_id, key, sizeof( key ) );
    JsonVariant old = best[ key ];

    if ( !old.isNull() ) {
        if ( pct <= old.as<int>() )
            return( false );
    }
    else if ( best.size() >= SLOVINGO_PROGRESS_MAX ) {
        return( false );
    }
    best[ key ] = pct;
    progress_save();
    return( true );
}
