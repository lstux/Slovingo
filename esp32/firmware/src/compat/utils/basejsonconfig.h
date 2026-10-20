/* Fichier de reglages JSON (sur la carte SD) : version reduite de basejsonconfig.h de la montre. */
#ifndef _BASEJSONCONFIG_H
    #define _BASEJSONCONFIG_H

    #include <stdint.h>
    #include <stddef.h>
    #include <ArduinoJson.h>

    #define MAX_CONFIG_FILE_NAME_LENGTH 64

    class BaseJsonConfig {
        public:
        BaseJsonConfig( const char *configFileName );
        virtual ~BaseJsonConfig() {}
        /** @brief load the settings, the defaults (and a new file) when the file is missing or broken */
        bool load( uint32_t size = 0 );
        /** @brief save the settings, false when the card is missing */
        bool save( uint32_t size = 0 );

        protected:
        virtual bool onSave( JsonDocument &document ) = 0;
        virtual bool onLoad( JsonDocument &document ) = 0;
        virtual bool onDefault( void ) = 0;
        virtual size_t getJsonBufferSize() { return 8192; }

        char fileName[ MAX_CONFIG_FILE_NAME_LENGTH ];
    };

#endif // _BASEJSONCONFIG_H
