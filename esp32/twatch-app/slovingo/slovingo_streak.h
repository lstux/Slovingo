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
#ifndef _SLOVINGO_STREAK_H
    #define _SLOVINGO_STREAK_H

    #include <time.h>

    #define SLOVINGO_STREAK_FILE    "/slovingo_streak.json"     /** @brief the file on the SPIFFS */

    /**
     * @brief number of the local day (days since 1970-01-01), -1 when the clock of the watch is not set
     */
    long slovingo_day_of( time_t now );

    /**
     * @brief a quiz was finished on this day: count it in the streak
     *
     * Played again the same day, nothing changes. The day after the last one, the streak grows.
     * After a longer pause it starts again at 1. The file is written only when something changes.
     *
     * @return  days in a row including this one, 0 when day is -1
     */
    int slovingo_streak_record( long day );

    /**
     * @brief days in a row as of this day: 0 once a whole day was missed
     */
    int slovingo_streak_current( long day );

#endif // _SLOVINGO_STREAK_H
