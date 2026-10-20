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
#ifndef _SLOVINGO_CONFIG_H
    #define _SLOVINGO_CONFIG_H

    #include "utils/basejsonconfig.h"

    #define SLOVINGO_JSON_CONFIG_FILE   "/slovingo.json"    /** @brief defines json config file name */
    #define SLOVINGO_DEFAULT_URL        "https://lstux.github.io/Slovingo/sk-fr-kids/api/"
    #define SLOVINGO_URL_LEN            160
    #define SLOVINGO_QUESTIONS_DEFAULT  10
    #define SLOVINGO_QUESTIONS_MAX      30
    #define SLOVINGO_NAME_LEN           24
    #define SLOVINGO_DEFAULT_NAME       "L\xC3\xAD\xC5\xA1ka"      /** @brief "Liska" (fox), what [USER_NAME] stands for in the course */

    #define SLOVINGO_LAUNCHER_URL       "https://lstux.github.io/Slovingo/"   /** @brief site of the engine: courses.json lists the courses */

    /**
     * @brief slovingo config structure
     */
    class slovingo_config_t : public BaseJsonConfig {
        public:
        slovingo_config_t();
        /** @brief folder of the course export (output of src/export_api.py), index.json lives in it */
        char url[ SLOVINGO_URL_LEN ] = SLOVINGO_DEFAULT_URL;
        /** @brief true once a course was picked (the first start shows the list) */
        bool chosen = false;
        /** @brief questions per exercise session */
        uint8_t questions = SLOVINGO_QUESTIONS_DEFAULT;
        /** @brief name of the player, shown where the course has [USER_NAME] */
        char name[ SLOVINGO_NAME_LEN ] = SLOVINGO_DEFAULT_NAME;

        protected:
        ////////////// Available for overloading: //////////////
        virtual bool onLoad( JsonDocument& document );
        virtual bool onSave( JsonDocument& document );
        virtual bool onDefault( void );
        virtual size_t getJsonBufferSize() { return 1000; }
    } ;

#endif // _SLOVINGO_CONFIG_H
