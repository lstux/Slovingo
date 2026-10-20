/* Documents ArduinoJson 6 en PSRAM (SpiRamJsonDocument), comme sur la montre. */
#ifndef _JSON_PSRAM_ALLOCATOR_H
    #define _JSON_PSRAM_ALLOCATOR_H

    #include <ArduinoJson.h>
    #include "utils/alloc.h"

    struct SpiRamAllocator {
        void *allocate( size_t size ) {
            return( MALLOC( size ) );
        }
        void deallocate( void *pointer ) {
            free( pointer );
        }
        void *reallocate( void *pointer, size_t size ) {
            return( REALLOC( pointer, size ) );
        }
    };
    using SpiRamJsonDocument = BasicJsonDocument<SpiRamAllocator>;

#endif // _JSON_PSRAM_ALLOCATOR_H
