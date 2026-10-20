/****************************************************************************
 *   Slovingo app for the T-Watch: config
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

#include "slovingo_config.h"

slovingo_config_t::slovingo_config_t() : BaseJsonConfig( SLOVINGO_JSON_CONFIG_FILE ) {
}

bool slovingo_config_t::onSave( JsonDocument& doc ) {

    doc["url"] = url;
    doc["chosen"] = chosen;
    doc["questions"] = questions;
    doc["name"] = name;

    return true;
}

bool slovingo_config_t::onLoad( JsonDocument& doc ) {

    const char *value = doc["url"] | SLOVINGO_DEFAULT_URL;
    snprintf( url, sizeof( url ), "%s", value );

    chosen = doc["chosen"] | false;

    int count = doc["questions"] | SLOVINGO_QUESTIONS_DEFAULT;
    if ( count < 1 )
        count = 1;
    if ( count > SLOVINGO_QUESTIONS_MAX )
        count = SLOVINGO_QUESTIONS_MAX;
    questions = count;

    const char *player = doc["name"] | SLOVINGO_DEFAULT_NAME;
    snprintf( name, sizeof( name ), "%s", *player ? player : SLOVINGO_DEFAULT_NAME );

    return true;
}

bool slovingo_config_t::onDefault( void ) {
    snprintf( url, sizeof( url ), "%s", SLOVINGO_DEFAULT_URL );
    chosen = false;
    questions = SLOVINGO_QUESTIONS_DEFAULT;
    snprintf( name, sizeof( name ), "%s", SLOVINGO_DEFAULT_NAME );
    return true;
}
