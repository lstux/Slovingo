#include <Arduino.h>
#include <FS.h>
#include <SD_MMC.h>
#include "utils/basejsonconfig.h"
#include "utils/json_psram_allocator.h"

BaseJsonConfig::BaseJsonConfig( const char *configFileName ) {
    snprintf( fileName, sizeof( fileName ), "%s", configFileName );
}

bool BaseJsonConfig::load( uint32_t size ) {
    if ( !SD_MMC.exists( fileName ) ) {
        log_i("%s not found, defaults", fileName );
        onDefault();
        save( size );
        return( false );
    }

    fs::File file = SD_MMC.open( fileName, FILE_READ );
    if ( !file ) {
        onDefault();
        return( false );
    }

    SpiRamJsonDocument doc( size ? size : getJsonBufferSize() );
    DeserializationError error = deserializeJson( doc, file );
    file.close();

    if ( error ) {
        log_e("%s: %s, defaults", fileName, error.c_str() );
        onDefault();
        return( false );
    }
    return( onLoad( doc ) );
}

bool BaseJsonConfig::save( uint32_t size ) {
    SpiRamJsonDocument doc( size ? size : getJsonBufferSize() );

    if ( !onSave( doc ) )
        return( false );

    fs::File file = SD_MMC.open( fileName, FILE_WRITE );
    if ( !file ) {
        log_w("%s: cannot write (no SD card?)", fileName );
        return( false );
    }
    bool ok = serializeJsonPretty( doc, file ) > 0;
    file.close();
    return( ok );
}
