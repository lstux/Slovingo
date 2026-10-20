/* Allocation preferant la PSRAM (8 Mo sur la T-HMI), comme alloc.h du firmware de la montre. */
#ifndef _ALLOC_H
    #define _ALLOC_H

    #include <stdlib.h>
    #include <esp_heap_caps.h>

    static inline void *slv_malloc( size_t size ) {
        void *ptr = heap_caps_malloc( size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT );
        return( ptr ? ptr : malloc( size ) );
    }
    static inline void *slv_calloc( size_t nmemb, size_t size ) {
        void *ptr = heap_caps_calloc( nmemb, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT );
        return( ptr ? ptr : calloc( nmemb, size ) );
    }
    static inline void *slv_realloc( void *old, size_t size ) {
        void *ptr = heap_caps_realloc( old, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT );
        return( ptr ? ptr : realloc( old, size ) );
    }

    #define MALLOC      slv_malloc
    #define CALLOC      slv_calloc
    #define REALLOC     slv_realloc

#endif // _ALLOC_H
