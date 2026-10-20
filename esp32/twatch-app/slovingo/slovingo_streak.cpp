/****************************************************************************
 *   Slovingo app for the T-Watch: days in a row
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

#ifndef NATIVE_64BIT
    #include <Arduino.h>
    #include <SPIFFS.h>
    #include <FS.h>
#endif

#include <ArduinoJson.h>
#include "slovingo_streak.h"

#ifdef NATIVE_64BIT
/* no SPIFFS on the computer: the file is these two values (tests) */
long slovingo_test_last = -1;
int slovingo_test_streak = 0;
#endif

/* days since 1970-01-01 of a civil date (proleptic gregorian, H. Hinnant's algorithm) */
static long streak_days_from_civil( int y, unsigned m, unsigned d ) {
    y -= m <= 2;
    long era = ( y >= 0 ? y : y - 399 ) / 400;
    unsigned yoe = (unsigned)( y - era * 400 );
    unsigned doy = ( 153 * ( m + ( m > 2 ? -3 : 9 ) ) + 2 ) / 5 + d - 1;
    unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return( era * 146097 + (long)doe - 719468 );
}

long slovingo_day_of( time_t now ) {
    struct tm info;

    localtime_r( &now, &info );
    if ( info.tm_year + 1900 < 2024 )       /* the clock was never set */
        return( -1 );
    return( streak_days_from_civil( info.tm_year + 1900, info.tm_mon + 1, info.tm_mday ) );
}

/* last day played and days in a row up to it */
static bool streak_load( long *last, int *streak ) {
    *last = -1;
    *streak = 0;
#ifndef NATIVE_64BIT
    if ( !SPIFFS.exists( SLOVINGO_STREAK_FILE ) )
        return( false );
    fs::File file = SPIFFS.open( SLOVINGO_STREAK_FILE, FILE_READ );
    if ( !file )
        return( false );
    StaticJsonDocument<128> doc;
    DeserializationError error = deserializeJson( doc, file );
    file.close();
    if ( error )
        return( false );
    *last = doc["last"] | -1L;
    *streak = doc["streak"] | 0;
    return( true );
#else
    *last = slovingo_test_last;
    *streak = slovingo_test_streak;
    return( true );
#endif
}

static void streak_save( long last, int streak ) {
#ifndef NATIVE_64BIT
    fs::File file = SPIFFS.open( SLOVINGO_STREAK_FILE, FILE_WRITE );
    if ( !file )
        return;
    StaticJsonDocument<128> doc;
    doc["last"] = last;
    doc["streak"] = streak;
    serializeJson( doc, file );
    file.close();
#else
    slovingo_test_last = last;
    slovingo_test_streak = streak;
#endif
}

int slovingo_streak_record( long day ) {
    long last;
    int streak;

    if ( day < 0 )
        return( 0 );
    streak_load( &last, &streak );
    if ( last == day && streak > 0 )
        return( streak );
    streak = ( last == day - 1 && streak > 0 ) ? streak + 1 : 1;
    streak_save( day, streak );
    return( streak );
}

int slovingo_streak_current( long day ) {
    long last;
    int streak;

    if ( day < 0 )
        return( 0 );
    streak_load( &last, &streak );
    return( ( last == day || last == day - 1 ) ? streak : 0 );
}
