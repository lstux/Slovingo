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
#ifndef _SLOVINGO_PROGRESS_H
    #define _SLOVINGO_PROGRESS_H

    #include <stddef.h>

    #define SLOVINGO_PROGRESS_FILE      "/slovingo_progress.json"   /** @brief the file on the SPIFFS */
    #define SLOVINGO_PROGRESS_MAX       1500                        /** @brief sheets remembered */

    /** @brief score in percent from which a sheet counts as done */
    #define SLOVINGO_DONE_PCT           50

    /**
     * @brief best score of a sheet of a course, in percent
     *
     * The file is read on the first call. A sheet is found by a hash of the course url and of
     * the sheet id, so the file stays small.
     *
     * @return  0..100, or -1 when the sheet was never played
     */
    int slovingo_progress_best( const char *course_url, const char *sheet_id );

    /**
     * @brief remember the score of a finished quiz, the file is written only when it is a new best
     *
     * @return  true when it is a new best (the first score of a sheet is one)
     */
    bool slovingo_progress_record( const char *course_url, const char *sheet_id, int pct );

#endif // _SLOVINGO_PROGRESS_H
