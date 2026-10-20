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
#ifndef _SLOVINGO_FETCH_H
    #define _SLOVINGO_FETCH_H

    #include <stddef.h>
    #include "utils/json_psram_allocator.h"

    /** @brief how long to wait for the wifi when it has to be switched on */
    #define SLOVINGO_WIFI_WAIT_MS   20000
    #define SLOVINGO_TRIES          3       /** @brief downloads tried before giving up */
    #define SLOVINGO_RETRY_WAIT_MS  1000    /** @brief wait before the 2nd try, doubled before the 3rd */

    typedef enum {
        SLOVINGO_OK = 0,
        SLOVINGO_ERR_NOWIFI,        /** @brief no wifi and nothing in the cache */
        SLOVINGO_ERR_NET,           /** @brief download failed and nothing in the cache */
        SLOVINGO_ERR_PARSE,         /** @brief the answer is not valid json */
        SLOVINGO_ERR_MEM            /** @brief out of memory */
    } slovingo_result_t;

    /**
     * @brief get a json file of the course export, from the network or from the SPIFFS cache
     *
     * With wifi connected (or force_net) the file is downloaded and the cache refreshed.
     * Without wifi the cached copy is used when there is one, otherwise the wifi is switched on.
     * Blocking, call it from a worker task and not from the lvgl task.
     *
     * @param   base_url    folder url of the export, with or without trailing slash
     * @param   path        file in the export, e.g. "index.json" or "sheets/<id>.json"
     * @param   force_net   skip the cache when the network is reachable
     * @param   doc         on success, a new json document in PSRAM, the caller deletes it
     *
     * @return  SLOVINGO_OK or the reason of the failure
     */
    slovingo_result_t slovingo_fetch_json( const char *base_url, const char *path, bool force_net, SpiRamJsonDocument **doc );
    /**
     * @brief short french text for a result code
     */
    const char *slovingo_result_text( slovingo_result_t result );
    /**
     * @brief copy a utf-8 text and keep only what the slovingo fonts can draw
     *
     * The fonts cover U+0020..U+007E and U+00A0..U+017F. Typographic dashes and quotes are
     * replaced by ascii, everything else (emojis, arrows...) is dropped. Newlines are kept.
     *
     * @param   in          zero terminated utf-8 text
     * @param   out         destination
     * @param   out_size    size of the destination, the result is always zero terminated
     */
    void slovingo_sanitize( const char *in, char *out, size_t out_size );

    /**
     * @brief   set the name the course's [USER_NAME] is replaced with by slovingo_sanitize()
     */
    void slovingo_set_user_name( const char *name );

#endif // _SLOVINGO_FETCH_H
