/****************************************************************************
 *   Slovingo app for the T-Watch: main tile (courses, sheets, exercises)
 *
 *   One tile, one scrollable page that is rebuilt for every screen:
 *   groups -> sheets -> sheet (read / exercises) -> reader | quiz -> result.
 *   The data comes from the export of src/export_api.py (index.json,
 *   sheets/<id>.json, exercises/<id>.json), downloaded by a worker task
 *   and cached in the SPIFFS, see slovingo_fetch.cpp.
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
#include <stdlib.h>
#include <string.h>

#ifndef NATIVE_64BIT
    #include <Arduino.h>
    #include <esp_system.h>
    #include "hardware/motor.h"
    #include "hardware/display.h"
#endif

#include "slovingo_app.h"
#include "slovingo_app_main.h"
#include "slovingo_config.h"
#include "slovingo_fetch.h"
#include "slovingo_progress.h"
#include "slovingo_streak.h"

#include "gui/mainbar/app_tile/app_tile.h"
#include "gui/mainbar/main_tile/main_tile.h"
#include "gui/mainbar/mainbar.h"
#include "gui/statusbar.h"
#include "gui/keyboard.h"
#include "gui/widget_factory.h"
#include "gui/widget_styles.h"
#include "utils/alloc.h"

LV_FONT_DECLARE( slovingo_16px );
LV_FONT_DECLARE( slovingo_22px );

/* the fox (the player) and the rabbit, see tools/gen_assets.py */
LV_IMG_DECLARE( slv_fox_neutral_40 );
LV_IMG_DECLARE( slv_fox_happy_40 );
LV_IMG_DECLARE( slv_fox_sad_40 );
LV_IMG_DECLARE( slv_fox_happy_64 );
LV_IMG_DECLARE( slv_fox_sad_64 );
LV_IMG_DECLARE( slv_rabbit_happy_64 );
LV_IMG_DECLARE( slv_theme_default_32 );
LV_IMG_DECLARE( slv_theme_basics_32 );
LV_IMG_DECLARE( slv_theme_family_32 );
LV_IMG_DECLARE( slv_theme_house_32 );
LV_IMG_DECLARE( slv_theme_food_32 );
LV_IMG_DECLARE( slv_theme_time_32 );
LV_IMG_DECLARE( slv_theme_shopping_32 );
LV_IMG_DECLARE( slv_theme_city_32 );
LV_IMG_DECLARE( slv_theme_weather_32 );
LV_IMG_DECLARE( slv_theme_mountains_32 );
LV_IMG_DECLARE( slv_theme_easter_32 );
LV_IMG_DECLARE( slv_theme_animals_32 );
LV_IMG_DECLARE( slv_theme_games_32 );
LV_IMG_DECLARE( slv_theme_sea_32 );

/*
 * the themes of the series (the "theme" of the sheets of index.json, the themes of the HTML
 * site): a pictogram and a dark colour for the buttons; the first one is for the others
 */
static const struct {
    const char *key;
    const lv_img_dsc_t *img;
    uint32_t color;
} slv_themes[] = {
    { "default",   &slv_theme_default_32,   0 },
    { "basics",    &slv_theme_basics_32,    0x6F5200 },
    { "family",    &slv_theme_family_32,    0x551C31 },
    { "house",     &slv_theme_house_32,     0x253948 },
    { "food",      &slv_theme_food_32,      0x732C12 },
    { "time",      &slv_theme_time_32,      0x072E43 },
    { "shopping",  &slv_theme_shopping_32,  0x691942 },
    { "city",      &slv_theme_city_32,      0x06367E },
    { "weather",   &slv_theme_weather_32,   0x253973 },
    { "mountains", &slv_theme_mountains_32, 0x16402A },
    { "easter",    &slv_theme_easter_32,    0x402A64 },
    { "animals",   &slv_theme_animals_32,   0x643E09 },
    { "games",     &slv_theme_games_32,     0x2F4900 },
    { "sea",       &slv_theme_sea_32,       0x064946 }
};
#define SLV_THEME_CNT   ( sizeof( slv_themes ) / sizeof( slv_themes[ 0 ] ) )

static uint8_t slv_theme_of( const char *key ) {
    for ( size_t i = 1; i < SLV_THEME_CNT; i++ ) {
        if ( key && !strcmp( slv_themes[ i ].key, key ) )
            return( i );
    }
    return( 0 );
}

#ifdef NATIVE_64BIT
    #define SLV_RAND()      ( (uint32_t)rand() )
#else
    #define SLV_RAND()      ( esp_random() )
#endif

/*
 * margins of the text pages, tighter than the theme ones to give the text more room
 */
#define SLV_SIDE_MARGIN     2       /* px left and right of the text */
#define SLV_GAP             2       /* px above and below the page */
#define SLV_ITEM_GAP        5       /* px between two blocks (buttons, labels) */
#define SLV_BTN_PAD_X       4       /* px inside a button, left and right of its text */
#define SLV_BTN_PAD_Y       2       /* px inside a button, above and below its text */
#define SLV_CHIP_PAD_X      6       /* px inside a word button of the order exercises, left and right */

/*
 * texts of the interface (the course content has its own language)
 */
#define TXT_LOADING         "Chargement..."
#define TXT_READ            "Lire la fiche"
#define TXT_EXERCISES       "Exercices"
#define TXT_NO_EXERCISES    "Pas d'exercice pour cette fiche"
#define SLV_ADVANCE_OK_MS   1000    /** @brief time a right answer stays on screen before the next question */
#define SLV_ADVANCE_KO_MS   2000    /** @brief time a wrong answer stays on screen (the right one is green) */
#define TXT_AGAIN           "Rejouer"
#define TXT_BACK            "Retour"
#define TXT_NEXT_SHEET      "Fiche suivante"
#define TXT_SAVE            "Enregistrer"
#define TXT_DEFAULT         "Valeur par défaut"
#define TXT_URL_HELP        "Adresse du dossier du cours (api) :"
#define TXT_COURSE          "Cours"
#define TXT_CHOOSE_COURSE   "Choisis ton cours"
#define TXT_OTHER_URL       "Autre adresse..."
#define TXT_NO_COURSES      "Liste des cours indisponible (pas de r\xC3\xA9seau ?)"
#define TXT_EMPTY_SHEET     "Fiche vide"
#define TXT_HIDE_TRANSLATIONS "Cacher les traductions"
#define TXT_SHOW_TRANSLATIONS "Afficher les traductions"
#define TXT_ORDER           "Remets les mots dans l'ordre"

#define SLV_MAX_GROUPS      32
#define SLV_MAX_ITEMS       64
#define SLV_MAX_Q           64
#define SLV_MAX_CHOICES     4
#define SLV_MAX_TOKENS      16      /* most words of an order sentence the watch plays */
#define SLV_MAX_DISTRACT    2       /* words that do not belong in the sentence */
#define SLV_MAX_SHOWN       ( SLV_MAX_TOKENS + SLV_MAX_DISTRACT )
#define SLV_MAX_CAND        20      /* distractor candidates looked at */
#define SLV_TOKEN_LEN       40

typedef enum {
    SCR_LOADING,
    SCR_ERROR,
    SCR_GROUPS,
    SCR_SHEETS,
    SCR_SHEET,
    SCR_READER,
    SCR_QUIZ,
    SCR_RESULT,
    SCR_SETUP,
    SCR_COURSES
} slv_screen_t;

typedef enum {
    REQ_COURSES,
    REQ_INDEX,
    REQ_SHEET,
    REQ_EXERCISES
} slv_req_t;

typedef struct {
    char label[ 48 ];
    char cat[ 24 ];
    char sub[ 40 ];
    uint16_t count;
    uint8_t theme;      /* index in slv_themes[] */
} slv_group_t;

typedef enum {
    SLV_KIND_CHOICE,        /* qcm and fill-blank: tap the right answer */
    SLV_KIND_ORDER          /* order: tap the words in the right order */
} slv_kind_t;

typedef struct {
    slv_kind_t kind;
    char prompt[ 200 ];
    char hint[ 160 ];
    char choice[ SLV_MAX_CHOICES ][ 96 ];
    int choice_cnt;
    int correct;
    /* order: the sentence word by word, and the buttons: its words plus the distractors, shuffled */
    char word[ SLV_MAX_TOKENS ][ SLV_TOKEN_LEN ];
    char shown[ SLV_MAX_SHOWN ][ SLV_TOKEN_LEN ];
    uint8_t word_cnt;
    uint8_t shown_cnt;
    uint8_t picked[ SLV_MAX_TOKENS ];   /* buttons tapped so far, in tap order */
    uint8_t picked_cnt;
} slv_question_t;

static slovingo_config_t slovingo_config;

/*
 * lvgl objects
 */
static lv_obj_t *slv_tile = NULL;
static lv_obj_t *slv_page = NULL;
static lv_obj_t *slv_exit_btn = NULL;
static lv_obj_t *slv_refresh_btn = NULL;
static lv_obj_t *slv_progress_lbl = NULL;   /* "x/x  Score: n" in the footer, quiz only */
static lv_obj_t *slv_built_lbl = NULL;      /* order: the sentence built so far, quiz only */
static lv_obj_t *slv_mascot = NULL;         /* the fox in the footer of the quiz, reacts to the answers */
static lv_obj_t *slv_setup_btn = NULL;
static lv_obj_t *slv_prev_btn = NULL;       /* reader: previous and next page */
static lv_obj_t *slv_next_btn = NULL;
static lv_obj_t *slv_url_textarea = NULL;
static lv_task_t *slv_task = NULL;
static lv_style_t slv_style16;
static lv_style_t slv_style22;

static lv_obj_t *slv_item[ SLV_MAX_ITEMS ];         /** @brief clickable buttons of the current screen, in order */
static int slv_item_cnt = 0;

/*
 * screens
 */
static slv_screen_t slv_screen = SCR_LOADING;
static slv_screen_t slv_pending_screen = SCR_LOADING;
static bool slv_pending = false;
static slv_screen_t slv_screen_before_load = SCR_GROUPS;
static char slv_error_text[ 96 ] = "";

/*
 * course data, all in PSRAM
 */
static SpiRamJsonDocument *slv_index = NULL;
static SpiRamJsonDocument *slv_sheet = NULL;
static SpiRamJsonDocument *slv_ex = NULL;

static slv_group_t slv_groups[ SLV_MAX_GROUPS ];
static int slv_group_cnt = 0;
static int slv_cur_group = -1;
static int slv_sheet_idx[ SLV_MAX_ITEMS ];          /** @brief positions in index["sheets"] of the current group */
static int slv_sheet_cnt = 0;
static int slv_cur_sheet = -1;                      /** @brief position in index["sheets"] */

/*
 * worker: one request at a time
 */
static volatile bool slv_busy = false;
static volatile bool slv_done = false;
static slv_req_t slv_req = REQ_INDEX;
static char slv_req_path[ 96 ] = "index.json";
static bool slv_req_force = false;

/*
 * list of the courses published by the Slovingo site (courses.json)
 */
#define SLV_MAX_COURSES     16
typedef struct {
    char id[ 32 ];
    char label[ 48 ];
    char url[ SLOVINGO_URL_LEN ];
} slv_course_t;
static slv_course_t slv_course[ SLV_MAX_COURSES ];
static int slv_course_cnt = 0;
static bool slv_courses_tried = false;
static slovingo_result_t slv_res = SLOVINGO_OK;
static SpiRamJsonDocument *slv_res_doc = NULL;

/*
 * quiz
 */
static uint16_t slv_q_order[ SLV_MAX_Q ];
static int slv_q_cnt = 0;
static int slv_prev_best = -1;      /* best score of the sheet before this quiz */
static bool slv_new_record = false;
static int slv_streak = 0;          /* days in a row, as of the end of the last quiz */
static int slv_q_pos = 0;
static int slv_score = 0;
static bool slv_q_answered = false;
/* end of the series: a little phrase of the course ("cheers" of index.json) drawn once per result */
static char slv_cheer_text[ 80 ];
static char slv_cheer_trans[ 80 ];
/* lv_tick time when the next question is shown after an answer, 0 = nothing scheduled */
static uint32_t slv_advance_at = 0;
/* the watch goes back to the main tile when the screen goes off, unless the display setting "block maintile" is on */
static bool slv_hold = false;
static bool slv_block_saved = false;
static slv_question_t slv_q;

static void slv_show( slv_screen_t screen );
static void slv_goto( slv_screen_t screen );
static void slv_load_course( void );
static void slv_open_courses( void );
static void slv_request( slv_req_t req, const char *path, bool force );
static void slv_app_activate_cb( void );
static void slv_app_hibernate_cb( void );
static void slv_lv_task( lv_task_t * task );
static void slv_back_event_cb( lv_obj_t * obj, lv_event_t event );
static void slv_refresh_event_cb( lv_obj_t * obj, lv_event_t event );
static void slv_setup_event_cb( lv_obj_t * obj, lv_event_t event );
static void slv_nav_event_cb( lv_obj_t * obj, lv_event_t event );
static void slv_gesture_event_cb( lv_obj_t * obj, lv_event_t event );
static void slv_navigate( int dir );
static void slv_read_next_sheet( void );
static void slv_open_data( slv_req_t req );
static void slv_set_nav( bool prev_ok, bool next_ok );
static void slv_item_event_cb( lv_obj_t * obj, lv_event_t event );
static void slv_textarea_event_cb( lv_obj_t * obj, lv_event_t event );

/* ------------------------------------------------------------------------
 * small helpers
 * ---------------------------------------------------------------------- */

static lv_coord_t slv_content_w( void ) {
    return( lv_disp_get_hor_res( NULL ) - 2 * SLV_SIDE_MARGIN );
}

/*
 * label with the slovingo font, text is cleaned up for the font
 */
static lv_obj_t *slv_add_label( const char *text, bool big ) {
    char buffer[ 512 ];

    slovingo_sanitize( text, buffer, sizeof( buffer ) );
    lv_obj_t *label = wf_add_label( slv_page, buffer );
    lv_label_set_long_mode( label, LV_LABEL_LONG_BREAK );
    lv_obj_set_width( label, slv_content_w() );
    lv_obj_add_style( label, LV_LABEL_PART_MAIN, big ? &slv_style22 : &slv_style16 );
    return( label );
}

/*
 * button with a wrapped label, it is registered as item of the screen
 */
static lv_obj_t *slv_add_button( const char *text ) {
    char buffer[ 256 ];

    slovingo_sanitize( text, buffer, sizeof( buffer ) );
    lv_obj_t *btn = lv_btn_create( slv_page, NULL );
    lv_obj_set_width( btn, slv_content_w() );
    lv_btn_set_fit2( btn, LV_FIT_NONE, LV_FIT_TIGHT );
    lv_obj_add_style( btn, LV_BTN_PART_MAIN, ws_get_button_style() );
    lv_obj_set_style_local_pad_left( btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, SLV_BTN_PAD_X );
    lv_obj_set_style_local_pad_right( btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, SLV_BTN_PAD_X );
    lv_obj_set_style_local_pad_top( btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, SLV_BTN_PAD_Y );
    lv_obj_set_style_local_pad_bottom( btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, SLV_BTN_PAD_Y );
    lv_obj_set_event_cb( btn, slv_item_event_cb );

    lv_obj_t *label = lv_label_create( btn, NULL );
    lv_label_set_long_mode( label, LV_LABEL_LONG_BREAK );
    lv_obj_set_width( label, slv_content_w() - 2 * SLV_BTN_PAD_X );
    lv_label_set_align( label, LV_LABEL_ALIGN_CENTER );
    lv_label_set_text( label, buffer );
    lv_obj_add_style( label, LV_LABEL_PART_MAIN, &slv_style16 );

    if ( slv_item_cnt < SLV_MAX_ITEMS )
        slv_item[ slv_item_cnt++ ] = btn;
    return( btn );
}

/* colour of a button, 0 keeps the one of the theme of the watch */
static void slv_tint( lv_obj_t *btn, uint32_t color ) {
    if ( !color )
        return;
    lv_obj_set_style_local_bg_color( btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex( color ) );
    /* the colours are dark: the text is always white, whatever the theme of the watch */
    lv_obj_set_style_local_text_color( btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE );
    lv_obj_set_style_local_text_color( btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_WHITE );
    lv_obj_set_style_local_text_color( btn, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, LV_COLOR_WHITE );
}

/* button with a pictogram at its left, the text beside it */
static lv_obj_t *slv_add_button_icon( const char *text, const lv_img_dsc_t *icon, uint32_t color ) {
    char buffer[ 256 ];

    slovingo_sanitize( text, buffer, sizeof( buffer ) );
    lv_obj_t *btn = lv_btn_create( slv_page, NULL );
    lv_obj_set_width( btn, slv_content_w() );
    lv_btn_set_fit2( btn, LV_FIT_NONE, LV_FIT_TIGHT );
    lv_btn_set_layout( btn, LV_LAYOUT_ROW_MID );
    lv_obj_add_style( btn, LV_BTN_PART_MAIN, ws_get_button_style() );
    lv_obj_set_style_local_pad_left( btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, SLV_BTN_PAD_X );
    lv_obj_set_style_local_pad_right( btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, SLV_BTN_PAD_X );
    lv_obj_set_style_local_pad_top( btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, SLV_BTN_PAD_Y );
    lv_obj_set_style_local_pad_bottom( btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, SLV_BTN_PAD_Y );
    lv_obj_set_style_local_pad_inner( btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 6 );
    lv_obj_set_event_cb( btn, slv_item_event_cb );
    slv_tint( btn, color );

    lv_obj_t *img = lv_img_create( btn, NULL );
    lv_img_set_src( img, icon );
    lv_obj_t *label = lv_label_create( btn, NULL );
    lv_label_set_long_mode( label, LV_LABEL_LONG_BREAK );
    lv_obj_set_width( label, slv_content_w() - 2 * SLV_BTN_PAD_X - icon->header.w - 6 );
    lv_label_set_align( label, LV_LABEL_ALIGN_LEFT );
    lv_label_set_text( label, buffer );
    lv_obj_add_style( label, LV_LABEL_PART_MAIN, &slv_style16 );

    if ( slv_item_cnt < SLV_MAX_ITEMS )
        slv_item[ slv_item_cnt++ ] = btn;
    return( btn );
}

/* a heading: a picture at the left of a text (recolor gives "#RRGGBB text#" its colour), returns the label */
static lv_obj_t *slv_add_head( const lv_img_dsc_t *icon, const char *text, bool big, bool recolor ) {
    lv_obj_t *row = lv_cont_create( slv_page, NULL );

    lv_cont_set_layout( row, LV_LAYOUT_ROW_MID );
    lv_cont_set_fit2( row, LV_FIT_NONE, LV_FIT_TIGHT );
    lv_obj_set_width( row, slv_content_w() );
    lv_obj_set_style_local_bg_opa( row, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP );
    lv_obj_set_style_local_border_width( row, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0 );
    lv_obj_set_style_local_pad_all( row, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0 );
    lv_obj_set_style_local_pad_inner( row, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 8 );
    lv_obj_set_click( row, false );
    lv_obj_set_drag_parent( row, true );

    lv_obj_t *img = lv_img_create( row, NULL );
    lv_img_set_src( img, icon );
    lv_obj_t *label = lv_label_create( row, NULL );
    lv_label_set_long_mode( label, LV_LABEL_LONG_BREAK );
    lv_label_set_recolor( label, recolor );
    lv_obj_set_width( label, slv_content_w() - icon->header.w - 8 );
    lv_obj_add_style( label, LV_LABEL_PART_MAIN, big ? &slv_style22 : &slv_style16 );
    lv_label_set_text( label, text );
    return( label );
}

/* the fox bounces while the course is loading */
static void slv_add_bouncing_fox( void ) {
    lv_obj_t *box = lv_cont_create( slv_page, NULL );

    lv_obj_set_size( box, slv_content_w(), 64 );
    lv_obj_set_style_local_bg_opa( box, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP );
    lv_obj_set_style_local_border_width( box, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0 );
    lv_obj_set_click( box, false );
    lv_obj_set_drag_parent( box, true );

    lv_obj_t *fox = lv_img_create( box, NULL );
    lv_img_set_src( fox, &slv_fox_happy_40 );
    lv_coord_t base = 20;
    lv_obj_set_pos( fox, ( slv_content_w() - slv_fox_happy_40.header.w ) / 2, base );

    lv_anim_t anim;
    lv_anim_init( &anim );
    lv_anim_set_var( &anim, fox );
    lv_anim_set_exec_cb( &anim, (lv_anim_exec_xcb_t)lv_obj_set_y );
    lv_anim_set_values( &anim, base, 2 );
    lv_anim_set_time( &anim, 320 );
    lv_anim_set_playback_time( &anim, 320 );
    lv_anim_set_repeat_count( &anim, LV_ANIM_REPEAT_INFINITE );
    lv_anim_path_t path;
    lv_anim_path_init( &path );
    lv_anim_path_set_cb( &path, lv_anim_path_ease_out );
    lv_anim_set_path( &anim, &path );
    lv_anim_start( &anim );
}

/*
 * word button: as wide as its word so that several fit on a line, registered
 * as item of the screen like the other buttons
 */
static lv_obj_t *slv_add_chip( lv_obj_t *parent, const char *text ) {
    lv_obj_t *btn = lv_btn_create( parent, NULL );
    lv_btn_set_fit( btn, LV_FIT_TIGHT );
    lv_obj_add_style( btn, LV_BTN_PART_MAIN, ws_get_button_style() );
    lv_obj_set_style_local_pad_left( btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, SLV_CHIP_PAD_X );
    lv_obj_set_style_local_pad_right( btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, SLV_CHIP_PAD_X );
    lv_obj_set_style_local_pad_top( btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, SLV_BTN_PAD_Y + 2 );
    lv_obj_set_style_local_pad_bottom( btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, SLV_BTN_PAD_Y + 2 );
    lv_obj_set_event_cb( btn, slv_item_event_cb );

    lv_obj_t *label = lv_label_create( btn, NULL );
    lv_label_set_long_mode( label, LV_LABEL_LONG_EXPAND );
    lv_label_set_text( label, text );
    lv_obj_add_style( label, LV_LABEL_PART_MAIN, &slv_style16 );
    /* a word wider than the screen wraps */
    lv_coord_t max_w = slv_content_w() - 2 * SLV_CHIP_PAD_X;
    if ( lv_obj_get_width( label ) > max_w ) {
        lv_label_set_long_mode( label, LV_LABEL_LONG_BREAK );
        lv_obj_set_width( label, max_w );
    }

    if ( slv_item_cnt < SLV_MAX_ITEMS )
        slv_item[ slv_item_cnt++ ] = btn;
    return( btn );
}

static int slv_item_index( lv_obj_t *obj ) {
    for ( int i = 0; i < slv_item_cnt; i++ ) {
        if ( slv_item[ i ] == obj )
            return( i );
    }
    return( -1 );
}

/* ------------------------------------------------------------------------
 * groups and sheets of the index
 * ---------------------------------------------------------------------- */

static void slv_build_groups( void ) {
    slv_group_cnt = 0;
    slv_cur_group = -1;
    slv_cur_sheet = -1;

    if ( !slv_index )
        return;

    JsonArray sheets = (*slv_index)["sheets"];
    for ( JsonObject sheet : sheets ) {
        const char *cat = sheet["cat"] | "";
        const char *sub = sheet["sub"] | "";
        int found = -1;

        for ( int i = 0; i < slv_group_cnt; i++ ) {
            if ( !strcmp( slv_groups[ i ].cat, cat ) && !strcmp( slv_groups[ i ].sub, sub ) ) {
                found = i;
                break;
            }
        }
        if ( found < 0 ) {
            if ( slv_group_cnt >= SLV_MAX_GROUPS )
                continue;
            found = slv_group_cnt++;
            slv_group_t *group = &slv_groups[ found ];
            snprintf( group->cat, sizeof( group->cat ), "%s", cat );
            snprintf( group->sub, sizeof( group->sub ), "%s", sub );
            const char *label = sheet["sub_label"] | "";
            if ( *label ) {
                snprintf( group->label, sizeof( group->label ), "%s", label );
            }
            else {
                snprintf( group->label, sizeof( group->label ), "%s", cat );
                if ( group->label[ 0 ] >= 'a' && group->label[ 0 ] <= 'z' )
                    group->label[ 0 ] -= 'a' - 'A';
            }
            group->count = 0;
            group->theme = slv_theme_of( sheet["theme"] | "" );
        }
        slv_groups[ found ].count++;
    }
}

static void slv_collect_sheets( int group ) {
    slv_sheet_cnt = 0;
    if ( !slv_index || group < 0 || group >= slv_group_cnt )
        return;

    int pos = 0;
    JsonArray sheets = (*slv_index)["sheets"];
    for ( JsonObject sheet : sheets ) {
        const char *cat = sheet["cat"] | "";
        const char *sub = sheet["sub"] | "";
        if ( !strcmp( slv_groups[ group ].cat, cat ) && !strcmp( slv_groups[ group ].sub, sub ) && slv_sheet_cnt < SLV_MAX_ITEMS )
            slv_sheet_idx[ slv_sheet_cnt++ ] = pos;
        pos++;
    }
}

static int slv_best_of( JsonObject sheet );

/* number of sheets done (best score from SLOVINGO_DONE_PCT) in each series */
static void slv_count_done( int *done ) {
    for ( int i = 0; i < slv_group_cnt; i++ )
        done[ i ] = 0;
    if ( !slv_index )
        return;
    for ( JsonObject sheet : (*slv_index)["sheets"].as<JsonArray>() ) {
        if ( slv_best_of( sheet ) < SLOVINGO_DONE_PCT )
            continue;
        const char *cat = sheet["cat"] | "";
        const char *sub = sheet["sub"] | "";
        for ( int i = 0; i < slv_group_cnt; i++ ) {
            if ( !strcmp( slv_groups[ i ].cat, cat ) && !strcmp( slv_groups[ i ].sub, sub ) ) {
                done[ i ]++;
                break;
            }
        }
    }
}

static JsonObject slv_cur_sheet_obj( void ) {
    return( (*slv_index)["sheets"][ slv_cur_sheet ] );
}

/* best score of a sheet of the index (-1: never played) */
static int slv_best_of( JsonObject sheet ) {
    return( slovingo_progress_best( slovingo_config.url, sheet["id"] | "" ) );
}

/* colour of a score: green for a perfect one, gold from 70 %, white from 50 %, grey below */
static const char *slv_best_color( int pct ) {
    return( pct >= 100 ? "81C784" : pct >= 70 ? "FFD54F" : pct >= SLOVINGO_DONE_PCT ? "FFFFFF" : "B0B7C0" );
}

/* label of a button: the text may carry #RRGGBB colour# parts */
static void slv_label_recolor( lv_obj_t *btn ) {
    lv_obj_t *label = lv_obj_get_child( btn, NULL );

    if ( label ) {
        lv_label_set_recolor( label, true );
        lv_label_set_text( label, lv_label_get_text( label ) );
    }
}

/* position in index["sheets"] of the sheet before (dir -1) or after (dir 1) the current one, -1 if none.
   The sheets follow the order of the course, so the next one of a series' last is the first of the next series */
static int slv_neighbor_sheet( int dir ) {
    if ( !slv_index || slv_cur_sheet < 0 )
        return( -1 );

    int pos = slv_cur_sheet + dir;
    int total = (*slv_index)["sheets"].size();
    return( pos >= 0 && pos < total ? pos : -1 );
}

/* the sheet at this position becomes the current one, with its series */
static void slv_select_sheet( int pos ) {
    JsonObject sheet = (*slv_index)["sheets"][ pos ];
    const char *cat = sheet["cat"] | "";
    const char *sub = sheet["sub"] | "";

    slv_cur_sheet = pos;
    for ( int i = 0; i < slv_group_cnt; i++ ) {
        if ( !strcmp( slv_groups[ i ].cat, cat ) && !strcmp( slv_groups[ i ].sub, sub ) ) {
            slv_cur_group = i;
            break;
        }
    }
    delete slv_sheet;
    slv_sheet = NULL;
    delete slv_ex;
    slv_ex = NULL;
}

/* exercises the watch can play: qcm and fill-blank (no audio, no drag and drop) */
static int slv_playable_count( JsonObject sheet ) {
    JsonObject ex = sheet["ex"];
    return( ex["qcm"].as<int>() + ex["fill-blank"].as<int>() + ex["order"].as<int>() );
}

/* ------------------------------------------------------------------------
 * reader: the sheet in pages
 *
 * A page is a heading plus text blocks, a few lines of dialogue or a few rows
 * of a vocabulary table, so that it fits the screen. The arrows of the footer
 * turn the pages. A dialogue line shows who speaks, the translation under it,
 * and the word by word glosses when it is tapped. A vocabulary row is a card,
 * a tap shows or hides its translation.
 * ---------------------------------------------------------------------- */

#define SLV_NONE            255
#define SLV_RD_TEXT_CHARS   260     /* characters of text on a page */
#define SLV_RD_DIALOG_CHARS 200     /* characters of dialogue on a page */
#define SLV_RD_DIALOG_LINES 3
#define SLV_RD_TABLE_ROWS   3
#define SLV_MAX_PAGES       160
#define SLV_MAX_SPEAKERS    8
#define SLV_CARD_PAD        3
#define SLV_COLOR_TITLE     0x4FC3F7
#define SLV_COLOR_DIM       0xA0A7B0
#define SLV_COLOR_GLOSS     0x8AB4F8
#define SLV_COLOR_CARD      0x23272E
#define SLV_COLOR_BUBBLE_ME 0x3A3326   /* the player's bubbles in the dialogues */
#define SLV_EMOJI_FOX       "\xF0\x9F\xA6\x8A"

typedef enum {
    RD_TEXT,
    RD_DIALOG,
    RD_TABLE
} slv_rd_kind_t;

typedef struct {
    uint8_t kind;
    uint8_t head;       /* heading block shown on top, SLV_NONE if none */
    uint8_t first;      /* first and last body block, SLV_NONE if the page has no body */
    uint8_t last;
    uint8_t row_from;   /* table: the rows shown, row_to excluded */
    uint8_t row_to;
} slv_rd_page_t;

typedef struct {
    char emoji[ 12 ];
    char name[ 24 ];
} slv_speaker_t;

static slv_rd_page_t slv_rd[ SLV_MAX_PAGES ];
static int slv_rd_cnt = 0;
static int slv_rd_cur = 0;
static bool slv_rd_hide = false;                    /* vocabulary: translations hidden */
static lv_obj_t *slv_rd_more[ SLV_MAX_ITEMS ];      /* object a tap on item i shows or hides */
static slv_speaker_t slv_spk[ SLV_MAX_SPEAKERS ];
static int slv_spk_cnt = 0;

/* characters of a block, to know how much fits on a page */
static size_t slv_block_chars( JsonObject block ) {
    const char *type = block["t"] | "";
    size_t n = 0;

    if ( !strcmp( type, "h" ) || !strcmp( type, "p" ) ) {
        n = strlen( block["x"] | "" );
    }
    else if ( !strcmp( type, "list" ) ) {
        JsonArray items = block["items"];
        for ( JsonVariant item : items ) {
            const char *text = item.as<const char*>();
            n += ( text ? strlen( text ) : 0 ) + 2;
        }
    }
    else if ( !strcmp( type, "card" ) ) {
        n = strlen( block["x"] | "" ) + strlen( block["n"] | "" );
    }
    return( n );
}

static void slv_rd_add( uint8_t kind, uint8_t head, uint8_t first, uint8_t last, uint8_t row_from, uint8_t row_to ) {
    if ( slv_rd_cnt >= SLV_MAX_PAGES )
        return;
    slv_rd_page_t *page = &slv_rd[ slv_rd_cnt++ ];
    page->kind = kind;
    page->head = head;
    page->first = first;
    page->last = last;
    page->row_from = row_from;
    page->row_to = row_to;
}

/*
 * cut the blocks of the sheet into pages: a heading goes with what follows it,
 * text blocks are put together up to a number of characters, a dialogue gets
 * a few lines per page and a table a few rows
 */
static void slv_reader_paginate( void ) {
    uint8_t head = SLV_NONE;
    size_t chars = 0;
    int lines = 0;

    slv_rd_cnt = 0;
    slv_rd_cur = 0;
    slv_rd_hide = true;                 /* translations start hidden: guess first, tap to check */
    if ( !slv_sheet )
        return;

    JsonArray blocks = (*slv_sheet)["blocks"];
    int total = blocks.size();
    if ( total > SLV_NONE - 1 )
        total = SLV_NONE - 1;

    for ( int i = 0; i < total; i++ ) {
        JsonObject block = blocks[ i ];
        const char *type = block["t"] | "";
        size_t len = slv_block_chars( block );
        slv_rd_page_t *last = slv_rd_cnt ? &slv_rd[ slv_rd_cnt - 1 ] : NULL;

        if ( !strcmp( type, "h" ) ) {
            if ( head != SLV_NONE )     /* two headings in a row: the first gets a page of its own */
                slv_rd_add( RD_TEXT, head, SLV_NONE, SLV_NONE, 0, 0 );
            head = i;
        }
        else if ( !strcmp( type, "table" ) ) {
            JsonArray rows = block["rows"];
            int n = rows.size();
            for ( int r = 0; r < n; r += SLV_RD_TABLE_ROWS )
                slv_rd_add( RD_TABLE, r ? SLV_NONE : head, i, i, r, ( r + SLV_RD_TABLE_ROWS < n ) ? r + SLV_RD_TABLE_ROWS : n );
            head = SLV_NONE;
        }
        else if ( !strcmp( type, "card" ) ) {
            if ( last && head == SLV_NONE && last->kind == RD_DIALOG && lines < SLV_RD_DIALOG_LINES && chars + len <= SLV_RD_DIALOG_CHARS ) {
                last->last = i;
                chars += len;
                lines++;
            }
            else {
                slv_rd_add( RD_DIALOG, head, i, i, 0, 0 );
                chars = len;
                lines = 1;
                head = SLV_NONE;
            }
        }
        else if ( !strcmp( type, "p" ) || !strcmp( type, "list" ) ) {
            if ( last && head == SLV_NONE && last->kind == RD_TEXT && last->first != SLV_NONE && chars + len <= SLV_RD_TEXT_CHARS ) {
                last->last = i;
                chars += len;
            }
            else {
                chars = len + ( head != SLV_NONE ? slv_block_chars( blocks[ head ] ) : 0 );
                slv_rd_add( RD_TEXT, head, i, i, 0, 0 );
                head = SLV_NONE;
            }
        }
    }
    if ( head != SLV_NONE )
        slv_rd_add( RD_TEXT, head, SLV_NONE, SLV_NONE, 0, 0 );
}

/*
 * speakers of the dialogue: the course lists its characters ("<emoji> Name, who she is"),
 * a character that is not listed gets a default name
 */
static const char *slv_spk_default( const char *emoji ) {
    static const struct {
        const char *emoji;
        const char *name;
    } known[] = {
        { SLV_EMOJI_FOX, "Toi" },
        { "\xF0\x9F\x90\xB0", "Lapin" },
        { "\xF0\x9F\x90\xB9", "Hamster" },
        { "\xF0\x9F\x90\xBB", "Ours" },
        { "\xF0\x9F\x90\x91", "Mouton" },
        { "\xF0\x9F\xA6\x94", "H\xC3\xA9risson" },
        { "\xF0\x9F\xA6\x85", "Aigle" }
    };

    for ( size_t i = 0; i < sizeof( known ) / sizeof( known[ 0 ] ); i++ ) {
        if ( !strcmp( known[ i ].emoji, emoji ) )
            return( known[ i ].name );
    }
    return( "" );
}

static int slv_spk_find( const char *emoji ) {
    for ( int i = 0; i < slv_spk_cnt; i++ ) {
        if ( !strcmp( slv_spk[ i ].emoji, emoji ) )
            return( i );
    }
    return( -1 );
}

static int slv_spk_add( const char *emoji, const char *name ) {
    int found = slv_spk_find( emoji );

    if ( found >= 0 )
        return( found );
    if ( slv_spk_cnt >= SLV_MAX_SPEAKERS || strlen( emoji ) >= sizeof( slv_spk[ 0 ].emoji ) )
        return( -1 );
    snprintf( slv_spk[ slv_spk_cnt ].emoji, sizeof( slv_spk[ 0 ].emoji ), "%s", emoji );
    slovingo_sanitize( name, slv_spk[ slv_spk_cnt ].name, sizeof( slv_spk[ 0 ].name ) );
    return( slv_spk_cnt++ );
}

static void slv_speakers_build( void ) {
    slv_spk_cnt = 0;
    if ( !slv_sheet )
        return;

    JsonArray blocks = (*slv_sheet)["blocks"];
    for ( JsonObject block : blocks ) {
        if ( strcmp( block["t"] | "", "list" ) )
            continue;
        JsonArray items = block["items"];
        for ( JsonVariant item : items ) {
            const char *text = item.as<const char*>();
            if ( !text || (uint8_t)text[ 0 ] < 0xF0 )       /* an emoji starts the line */
                continue;
            const char *space = strchr( text, ' ' );
            if ( !space || (size_t)( space - text ) >= sizeof( slv_spk[ 0 ].emoji ) )
                continue;

            char emoji[ sizeof( slv_spk[ 0 ].emoji ) ];
            memcpy( emoji, text, space - text );
            emoji[ space - text ] = '\0';

            /* the name goes up to the first comma or parenthesis */
            char name[ 40 ];
            size_t n = 0;
            for ( const char *c = space + 1; *c && *c != ',' && *c != '(' && n < sizeof( name ) - 1; c++ )
                name[ n++ ] = *c;
            while ( n && name[ n - 1 ] == ' ' )
                n--;
            name[ n ] = '\0';
            slv_spk_add( emoji, name );
        }
    }
}

static int slv_spk_of( const char *emoji ) {
    if ( !emoji || !*emoji )
        return( -1 );
    int found = slv_spk_find( emoji );
    return( found >= 0 ? found : slv_spk_add( emoji, slv_spk_default( emoji ) ) );
}

/* chat side: the player (the fox) speaks on the right; without it the speakers alternate */
static bool slv_spk_right( int spk ) {
    if ( spk < 0 )
        return( false );
    if ( slv_spk_find( SLV_EMOJI_FOX ) >= 0 )
        return( !strcmp( slv_spk[ spk ].emoji, SLV_EMOJI_FOX ) );
    return( spk % 2 == 1 );
}

static uint32_t slv_spk_color( int spk ) {
    static const uint32_t palette[] = { 0x64B5F6, 0x81C784, 0xBA68C8, 0xF06292, 0x4DD0E1, 0xFFF176, 0xA1887F, 0x90A4AE };

    if ( !strcmp( slv_spk[ spk ].emoji, SLV_EMOJI_FOX ) )
        return( 0xFFB74D );
    return( palette[ spk % 8 ] );
}

/*
 * a card: a rounded box that can be tapped, filled with labels by the caller
 */
static lv_obj_t *slv_add_card_in( lv_obj_t *parent, lv_coord_t width, uint32_t bg, lv_coord_t radius ) {
    lv_obj_t *card = lv_cont_create( parent, NULL );

    lv_cont_set_layout( card, LV_LAYOUT_COLUMN_LEFT );
    lv_cont_set_fit2( card, LV_FIT_NONE, LV_FIT_TIGHT );
    lv_obj_set_width( card, width );
    lv_obj_set_style_local_bg_color( card, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex( bg ) );
    lv_obj_set_style_local_bg_opa( card, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER );
    lv_obj_set_style_local_radius( card, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, radius );
    lv_obj_set_style_local_border_width( card, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0 );
    lv_obj_set_style_local_pad_left( card, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, SLV_CARD_PAD );
    lv_obj_set_style_local_pad_right( card, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, SLV_CARD_PAD );
    lv_obj_set_style_local_pad_top( card, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, SLV_CARD_PAD );
    lv_obj_set_style_local_pad_bottom( card, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, SLV_CARD_PAD );
    lv_obj_set_style_local_pad_inner( card, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 1 );
    lv_obj_set_click( card, true );
    lv_obj_set_event_cb( card, slv_item_event_cb );

    if ( slv_item_cnt < SLV_MAX_ITEMS )
        slv_item[ slv_item_cnt++ ] = card;
    return( card );
}

static lv_obj_t *slv_add_card( void ) {
    return( slv_add_card_in( slv_page, slv_content_w(), SLV_COLOR_CARD, 6 ) );
}

/*
 * a chat bubble: a card of 86 % of the width, at the left or at the right of the page.
 * The page lays its children out in a centred column, so the bubble sits in a row
 * as wide as the page, with an empty spacer on its left when it goes to the right.
 */
static lv_obj_t *slv_add_bubble( bool right, uint32_t bg ) {
    lv_coord_t full = slv_content_w();
    lv_coord_t width = full * 86 / 100;
    lv_obj_t *row = lv_cont_create( slv_page, NULL );

    lv_cont_set_layout( row, LV_LAYOUT_ROW_TOP );
    lv_cont_set_fit2( row, LV_FIT_NONE, LV_FIT_TIGHT );
    lv_obj_set_width( row, full );
    lv_obj_set_style_local_bg_opa( row, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP );
    lv_obj_set_style_local_border_width( row, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0 );
    lv_obj_set_style_local_pad_all( row, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0 );
    lv_obj_set_style_local_pad_inner( row, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0 );
    lv_obj_set_click( row, false );
    lv_obj_set_drag_parent( row, true );

    if ( right ) {
        lv_obj_t *spacer = lv_obj_create( row, NULL );
        lv_obj_set_size( spacer, full - width, 1 );
        lv_obj_set_style_local_bg_opa( spacer, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP );
        lv_obj_set_style_local_border_width( spacer, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0 );
        lv_obj_set_click( spacer, false );
        lv_obj_set_drag_parent( spacer, true );
    }
    return( slv_add_card_in( row, width, bg, 10 ) );
}

/* label in a card, text already cleaned up, recolor gives "#RRGGBB text#" its colour */
static lv_obj_t *slv_card_label( lv_obj_t *card, const char *text, bool big, uint32_t color, bool recolor ) {
    lv_obj_t *label = lv_label_create( card, NULL );

    lv_label_set_long_mode( label, LV_LABEL_LONG_BREAK );
    lv_label_set_recolor( label, recolor );
    lv_obj_set_width( label, lv_obj_get_width( card ) - 2 * SLV_CARD_PAD );
    lv_obj_add_style( label, LV_LABEL_PART_MAIN, big ? &slv_style22 : &slv_style16 );
    lv_obj_set_style_local_text_color( label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex( color ) );
    lv_label_set_text( label, text );
    return( label );
}

/* clean a piece of text for a label with recolor on: no '#' left in it */
static void slv_clean( const char *in, char *out, size_t size ) {
    slovingo_sanitize( in ? in : "", out, size );
    for ( char *c = out; *c; c++ ) {
        if ( *c == '#' )
            *c = ' ';
    }
}

/* append a cleaned up piece of text, what does not fit is dropped */
static void slv_append( char *dst, size_t cap, const char *text ) {
    char piece[ 200 ];
    size_t len = strlen( dst );

    slv_clean( text, piece, sizeof( piece ) );
    size_t n = strlen( piece );
    if ( len + n + 1 > cap )
        return;
    memcpy( dst + len, piece, n + 1 );
}

/* a dialogue line: who speaks and what, the translation, and the glosses and notes on a tap */
static void slv_reader_card( JsonObject block ) {
    char text[ 512 ];
    char piece[ 256 ];
    int spk = slv_spk_of( block["s"] | "" );
    bool right = slv_spk_right( spk );
    lv_obj_t *card = slv_add_bubble( right, right ? SLV_COLOR_BUBBLE_ME : SLV_COLOR_CARD );
    int item = slv_item_cnt - 1;
    bool named = spk >= 0 && *slv_spk[ spk ].name;

    slv_clean( block["x"] | "", piece, sizeof( piece ) );
    if ( named )
        snprintf( text, sizeof( text ), "#%06X %s:# %s", (unsigned)slv_spk_color( spk ), slv_spk[ spk ].name, piece );
    else
        snprintf( text, sizeof( text ), "%s", piece );
    slv_card_label( card, text, false, 0xFFFFFF, named );

    slv_clean( block["n"] | "", piece, sizeof( piece ) );
    slv_card_label( card, piece, false, SLV_COLOR_DIM, false );

    text[ 0 ] = '\0';
    JsonArray gloss = block["lit"];
    for ( JsonVariant line : gloss ) {
        if ( *text )
            slv_append( text, sizeof( text ), "\n" );
        slv_append( text, sizeof( text ), line.as<const char*>() );
    }
    JsonArray notes = block["nt"];
    for ( JsonVariant line : notes ) {
        if ( *text )
            slv_append( text, sizeof( text ), "\n" );
        slv_append( text, sizeof( text ), "* " );
        slv_append( text, sizeof( text ), line.as<const char*>() );
    }
    if ( *text ) {
        lv_obj_t *more = slv_card_label( card, text, false, SLV_COLOR_GLOSS, false );
        lv_obj_set_hidden( more, true );
        slv_rd_more[ item ] = more;
    }
}

/* vocabulary rows as cards: the first column big, the others (sound, meaning) under it */
static void slv_reader_rows( JsonObject block, int from, int to ) {
    JsonArray rows = block["rows"];

    for ( int r = from; r < to; r++ ) {
        JsonArray row = rows[ r ];
        char first[ 128 ] = "";
        char rest[ 256 ] = "";
        int col = 0;

        for ( JsonVariant cell : row ) {
            if ( col++ == 0 ) {
                slv_clean( cell.as<const char*>(), first, sizeof( first ) );
            }
            else {
                char piece[ 128 ];
                slv_clean( cell.as<const char*>(), piece, sizeof( piece ) );
                if ( *piece ) {
                    if ( *rest )
                        slv_append( rest, sizeof( rest ), " - " );
                    slv_append( rest, sizeof( rest ), piece );
                }
            }
        }

        lv_obj_t *card = slv_add_card();
        int item = slv_item_cnt - 1;
        slv_card_label( card, first, true, 0xFFFFFF, false );
        if ( *rest ) {
            lv_obj_t *more = slv_card_label( card, rest, false, SLV_COLOR_DIM, false );
            lv_obj_set_hidden( more, slv_rd_hide );
            slv_rd_more[ item ] = more;
        }
    }
}

static void slv_reader_render( void ) {
    char text[ 512 ];

    if ( !slv_sheet || slv_rd_cnt == 0 ) {
        slv_add_label( TXT_EMPTY_SHEET, false );
        return;
    }

    slv_rd_page_t *page = &slv_rd[ slv_rd_cur ];
    JsonArray blocks = (*slv_sheet)["blocks"];

    if ( slv_rd_cur == 0 )
        slv_add_label( slv_cur_sheet_obj()["title"] | "?", true );
    if ( page->head != SLV_NONE ) {
        lv_obj_t *head = slv_add_label( blocks[ page->head ]["x"] | "", true );
        lv_obj_set_style_local_text_color( head, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex( SLV_COLOR_TITLE ) );
    }

    if ( page->kind == RD_TABLE ) {
        slv_add_button( slv_rd_hide ? TXT_SHOW_TRANSLATIONS : TXT_HIDE_TRANSLATIONS );
        slv_reader_rows( blocks[ page->first ], page->row_from, page->row_to );
    }
    else if ( page->first != SLV_NONE ) {
        for ( int i = page->first; i <= page->last; i++ ) {
            JsonObject block = blocks[ i ];
            const char *type = block["t"] | "";

            if ( !strcmp( type, "p" ) ) {
                slv_add_label( block["x"] | "", false );
            }
            else if ( !strcmp( type, "list" ) ) {
                JsonArray items = block["items"];
                text[ 0 ] = '\0';
                for ( JsonVariant item : items ) {
                    const char *line = item.as<const char*>();
                    char piece[ 160 ];
                    slovingo_sanitize( line ? line : "", piece, sizeof( piece ) );
                    const char *start = piece;
                    while ( *start == ' ' )     /* an emoji that was dropped leaves a space */
                        start++;
                    if ( *text )
                        slv_append( text, sizeof( text ), "\n" );
                    slv_append( text, sizeof( text ), "- " );
                    slv_append( text, sizeof( text ), start );
                }
                slv_add_label( text, false );
            }
            else if ( !strcmp( type, "card" ) ) {
                slv_reader_card( block );
            }
        }
    }

    snprintf( text, sizeof( text ), "%d/%d", slv_rd_cur + 1, slv_rd_cnt );
    lv_label_set_text( slv_progress_lbl, text );
    if ( slv_rd_cur > 0 )
        lv_obj_clear_state( slv_prev_btn, LV_STATE_DISABLED );
    else
        lv_obj_add_state( slv_prev_btn, LV_STATE_DISABLED );
    /* on the last page the next button goes on to the next sheet */
    if ( slv_rd_cur + 1 < slv_rd_cnt || slv_neighbor_sheet( 1 ) >= 0 )
        lv_obj_clear_state( slv_next_btn, LV_STATE_DISABLED );
    else
        lv_obj_add_state( slv_next_btn, LV_STATE_DISABLED );
}

/* a tap on a card shows or hides what goes with it, the first button of a table page hides all the translations */
static void slv_reader_tap( int item ) {
    if ( slv_rd_cnt == 0 )
        return;
    if ( slv_rd[ slv_rd_cur ].kind == RD_TABLE && item == 0 ) {
        slv_rd_hide = !slv_rd_hide;
        slv_goto( SCR_READER );
        return;
    }
    if ( item >= 0 && item < SLV_MAX_ITEMS && slv_rd_more[ item ] )
        lv_obj_set_hidden( slv_rd_more[ item ], !lv_obj_get_hidden( slv_rd_more[ item ] ) );
}

/* ------------------------------------------------------------------------
 * quiz
 * ---------------------------------------------------------------------- */

static void slv_q_add_choice( const char *text ) {
    if ( !text || !*text || slv_q.choice_cnt >= SLV_MAX_CHOICES )
        return;
    for ( int i = 0; i < slv_q.choice_cnt; i++ ) {
        if ( !strcmp( slv_q.choice[ i ], text ) )
            return;
    }
    snprintf( slv_q.choice[ slv_q.choice_cnt++ ], sizeof( slv_q.choice[ 0 ] ), "%s", text );
}

static void slv_q_add_distractors( JsonArray distractors ) {
    for ( JsonVariant value : distractors )
        slv_q_add_choice( value.as<const char*>() );
}

/*
 * order: words of an exercise. Each chunk of tokens_l2 is cut at its spaces to
 * get single words; when that is too many for the screen the chunks are kept
 * as they are. words may be NULL to just count. Returns the number of words,
 * 0 when the exercise cannot be played.
 */
static bool slv_order_add( char words[][ SLV_TOKEN_LEN ], int *n, const char *text, size_t len ) {
    if ( !len )
        return( true );
    if ( *n >= SLV_MAX_TOKENS )
        return( false );
    if ( words ) {
        if ( len >= SLV_TOKEN_LEN )
            len = SLV_TOKEN_LEN - 1;
        memcpy( words[ *n ], text, len );
        words[ *n ][ len ] = '\0';
    }
    ( *n )++;
    return( true );
}

static int slv_order_words( JsonArray tokens, char words[][ SLV_TOKEN_LEN ] ) {
    for ( int split = 1; split >= 0; split-- ) {
        int n = 0;
        bool fits = true;

        for ( JsonVariant token : tokens ) {
            char chunk[ 96 ];
            const char *src = token.as<const char*>();

            slovingo_sanitize( src ? src : "", chunk, sizeof( chunk ) );
            if ( !split ) {
                fits = slv_order_add( words, &n, chunk, strlen( chunk ) );
            }
            else {
                const char *p = chunk;
                while ( fits && *p ) {
                    while ( *p == ' ' )
                        p++;
                    const char *end = p;
                    while ( *end && *end != ' ' )
                        end++;
                    fits = slv_order_add( words, &n, p, end - p );
                    p = end;
                }
            }
            if ( !fits )
                break;
        }
        if ( fits && n >= 2 )
            return( n );
    }
    return( 0 );
}

/*
 * order: the words of the sentence plus one or two words of the other sentences
 * of the sheet that do not belong in it, all in a random order
 */
static void slv_make_order( int ex_idx, JsonObject exercise ) {
    char cand[ SLV_MAX_CAND ][ SLV_TOKEN_LEN ];
    char other_words[ SLV_MAX_TOKENS ][ SLV_TOKEN_LEN ];
    int cand_cnt = 0;
    int n = slv_order_words( exercise["tokens_l2"], slv_q.word );

    slv_q.kind = SLV_KIND_ORDER;
    slv_q.word_cnt = n;
    slovingo_sanitize( exercise["l1"] | "", slv_q.prompt, sizeof( slv_q.prompt ) );
    snprintf( slv_q.hint, sizeof( slv_q.hint ), "%s (%d)", TXT_ORDER, n );

    for ( int k = 0; k < n; k++ )
        memcpy( slv_q.shown[ k ], slv_q.word[ k ], SLV_TOKEN_LEN );
    slv_q.shown_cnt = n;

    /* candidates for the distractors: words of the other sentences, not used in this one */
    int pos = 0;
    JsonArray exercises = (*slv_ex)["exercises"];
    for ( JsonObject other : exercises ) {
        if ( pos++ == ex_idx || cand_cnt >= SLV_MAX_CAND || strcmp( other["type"] | "", "order" ) )
            continue;
        int on = slv_order_words( other["tokens_l2"], other_words );
        for ( int i = 0; i < on && cand_cnt < SLV_MAX_CAND; i++ ) {
            bool known = false;
            for ( int j = 0; j < n; j++ ) {
                if ( !strcmp( slv_q.word[ j ], other_words[ i ] ) )
                    known = true;
            }
            for ( int j = 0; j < cand_cnt; j++ ) {
                if ( !strcmp( cand[ j ], other_words[ i ] ) )
                    known = true;
            }
            if ( !known )
                memcpy( cand[ cand_cnt++ ], other_words[ i ], SLV_TOKEN_LEN );
        }
    }
    int wanted = n >= 5 ? 2 : ( n >= 3 ? 1 : 0 );
    for ( int d = 0; d < wanted && cand_cnt > 0; d++ ) {
        int r = SLV_RAND() % cand_cnt;
        memcpy( slv_q.shown[ slv_q.shown_cnt++ ], cand[ r ], SLV_TOKEN_LEN );
        memcpy( cand[ r ], cand[ --cand_cnt ], SLV_TOKEN_LEN );
    }

    /* shuffle, and try again if the words came out in the right order */
    int total = slv_q.shown_cnt;
    for ( int attempt = 0; attempt < 4; attempt++ ) {
        for ( int i = total - 1; i > 0; i-- ) {
            int j = SLV_RAND() % ( i + 1 );
            char tmp[ SLV_TOKEN_LEN ];
            memcpy( tmp, slv_q.shown[ i ], SLV_TOKEN_LEN );
            memcpy( slv_q.shown[ i ], slv_q.shown[ j ], SLV_TOKEN_LEN );
            memcpy( slv_q.shown[ j ], tmp, SLV_TOKEN_LEN );
        }
        bool same = ( total == n );
        for ( int k = 0; same && k < n; k++ ) {
            if ( strcmp( slv_q.shown[ k ], slv_q.word[ k ] ) )
                same = false;
        }
        if ( !same )
            break;
    }
}

/* exercise the watch can play */
static bool slv_is_playable( JsonObject exercise ) {
    const char *type = exercise["type"] | "";

    if ( !strcmp( type, "qcm" ) || !strcmp( type, "fill-blank" ) )
        return( true );
    if ( !strcmp( type, "order" ) )
        return( slv_order_words( exercise["tokens_l2"], NULL ) > 0 );
    return( false );
}

/*
 * qcm: ask in one language, answer in the other, direction picked at random
 * fill-blank: the target sentence with its missing word hidden
 */
static void slv_make_question( int ex_idx ) {
    JsonObject exercise = (*slv_ex)["exercises"][ ex_idx ];
    const char *type = exercise["type"] | "";

    memset( &slv_q, 0, sizeof( slv_q ) );

    if ( !strcmp( type, "order" ) ) {
        slv_make_order( ex_idx, exercise );
    }
    else if ( !strcmp( type, "fill-blank" ) ) {
        const char *sentence = exercise["l2"] | "";
        const char *missing = exercise["missing_l2"] | "";
        const char *pos = *missing ? strstr( sentence, missing ) : NULL;

        if ( pos )
            snprintf( slv_q.prompt, sizeof( slv_q.prompt ), "%.*s____%s", (int)( pos - sentence ), sentence, pos + strlen( missing ) );
        else
            snprintf( slv_q.prompt, sizeof( slv_q.prompt ), "%s ____", sentence );
        if ( exercise["show_translation"] | true )
            snprintf( slv_q.hint, sizeof( slv_q.hint ), "%s", exercise["l1"] | "" );
        slv_q_add_choice( missing );
        slv_q_add_distractors( exercise["choices_l2"] );
    }
    else if ( !strcmp( type, "qcm" ) ) {
        bool to_target = ( SLV_RAND() & 1 ) != 0;

        snprintf( slv_q.prompt, sizeof( slv_q.prompt ), "%s", exercise[ to_target ? "l1" : "l2" ] | "" );
        slv_q_add_choice( exercise[ to_target ? "l2" : "l1" ] | "" );
        slv_q_add_distractors( exercise[ to_target ? "choices_l2" : "choices_l1" ] );
    }

    /* the right answer was added first, shuffle and follow it */
    slv_q.correct = 0;
    for ( int i = slv_q.choice_cnt - 1; i > 0; i-- ) {
        int j = SLV_RAND() % ( i + 1 );
        if ( i == j )
            continue;
        char tmp[ sizeof( slv_q.choice[ 0 ] ) ];
        memcpy( tmp, slv_q.choice[ i ], sizeof( tmp ) );
        memcpy( slv_q.choice[ i ], slv_q.choice[ j ], sizeof( tmp ) );
        memcpy( slv_q.choice[ j ], tmp, sizeof( tmp ) );
        if ( slv_q.correct == i )
            slv_q.correct = j;
        else if ( slv_q.correct == j )
            slv_q.correct = i;
    }
    slv_q_answered = false;
}

static bool slv_quiz_start( void ) {
    int total = 0;
    int pos = 0;

    if ( !slv_ex )
        return( false );

    JsonArray exercises = (*slv_ex)["exercises"];
    for ( JsonObject exercise : exercises ) {
        if ( slv_is_playable( exercise ) && total < SLV_MAX_Q )
            slv_q_order[ total++ ] = pos;
        pos++;
    }
    if ( total == 0 )
        return( false );

    for ( int i = total - 1; i > 0; i-- ) {
        int j = SLV_RAND() % ( i + 1 );
        uint16_t tmp = slv_q_order[ i ];
        slv_q_order[ i ] = slv_q_order[ j ];
        slv_q_order[ j ] = tmp;
    }

    slv_q_cnt = total < slovingo_config.questions ? total : slovingo_config.questions;
    slv_q_pos = 0;
    slv_score = 0;
    slv_make_question( slv_q_order[ 0 ] );
    return( true );
}

typedef enum {
    MOOD_NEUTRAL,
    MOOD_HAPPY,
    MOOD_SAD
} slv_mood_t;

static void slv_set_mood( slv_mood_t mood ) {
    static const lv_img_dsc_t *const fox[] = { &slv_fox_neutral_40, &slv_fox_happy_40, &slv_fox_sad_40 };

    lv_img_set_src( slv_mascot, fox[ mood ] );
}

/* the fox smiles or cries, and the watch buzzes a little (when the vibe feedback is on) */
static void slv_feedback( bool ok ) {
    slv_set_mood( ok ? MOOD_HAPPY : MOOD_SAD );
#ifndef NATIVE_64BIT
    motor_vibe( ok ? 4 : 20 );
#endif
}

static void slv_answer( int choice ) {
    if ( slv_q_answered || choice < 0 || choice >= slv_q.choice_cnt )
        return;

    slv_q_answered = true;
    if ( choice == slv_q.correct )
        slv_score++;

    lv_obj_set_style_local_bg_color( slv_item[ slv_q.correct ], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex( 0x2E7D32 ) );
    if ( choice != slv_q.correct )
        lv_obj_set_style_local_bg_color( slv_item[ choice ], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex( 0xC62828 ) );

    /* no next button: the next question comes by itself, a wrong answer stays on screen a little longer */
    slv_feedback( choice == slv_q.correct );
    slv_advance_at = lv_tick_get() + ( choice == slv_q.correct ? SLV_ADVANCE_OK_MS : SLV_ADVANCE_KO_MS );
}

/*
 * order: the sentence built so far, or the whole right sentence when full is set
 */
static void slv_order_text( bool full, lv_color_t color ) {
    char text[ 256 ] = "";
    size_t len = 0;
    int n = full ? slv_q.word_cnt : slv_q.picked_cnt;

    for ( int j = 0; j < n; j++ ) {
        const char *word = full ? slv_q.word[ j ] : slv_q.shown[ slv_q.picked[ j ] ];
        size_t wl = strlen( word );
        if ( len + wl + 2 > sizeof( text ) )
            break;
        if ( len )
            text[ len++ ] = ' ';
        memcpy( text + len, word, wl );
        len += wl;
        text[ len ] = '\0';
    }
    lv_label_set_text( slv_built_lbl, text );
    lv_obj_set_style_local_text_color( slv_built_lbl, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color );
}

/*
 * order: a tapped word leaves the list, the answer is checked when all the words are placed
 */
static void slv_order_pick( int k ) {
    if ( slv_q_answered || k < 0 || k >= slv_q.shown_cnt || !slv_built_lbl )
        return;

    slv_q.picked[ slv_q.picked_cnt++ ] = k;
    lv_obj_set_hidden( slv_item[ k ], true );
    slv_order_text( false, lv_color_hex( 0xFFFFFF ) );

    if ( slv_q.picked_cnt < slv_q.word_cnt )
        return;

    slv_q_answered = true;
    bool ok = true;
    for ( int j = 0; j < slv_q.word_cnt; j++ ) {
        if ( strcmp( slv_q.shown[ slv_q.picked[ j ] ], slv_q.word[ j ] ) )
            ok = false;
    }
    if ( ok ) {
        slv_score++;
        slv_order_text( false, lv_color_hex( 0x66BB6A ) );
    }
    else {
        slv_order_text( true, lv_color_hex( 0xEF5350 ) );
    }
    slv_feedback( ok );
    slv_advance_at = lv_tick_get() + ( ok ? SLV_ADVANCE_OK_MS : SLV_ADVANCE_KO_MS );
}

/*
 * one of the phrases the course gives for the score: tier perfect (100 %), great (90),
 * good (70), ok (50) or retry, one picked at random. A course without "cheers" gives nothing.
 */
static void slv_pick_cheer( void ) {
    int pct = slv_q_cnt ? slv_score * 100 / slv_q_cnt : 0;
    const char *tier = pct >= 100 ? "perfect" : ( pct >= 90 ? "great" : ( pct >= 70 ? "good" : ( pct >= 50 ? "ok" : "retry" ) ) );

    slv_cheer_text[ 0 ] = '\0';
    slv_cheer_trans[ 0 ] = '\0';
    if ( !slv_index )
        return;

    JsonArray list = (*slv_index)["cheers"][ tier ];
    size_t cnt = list.size();
    if ( !cnt )
        return;

    JsonObject cheer = list[ SLV_RAND() % cnt ];
    slovingo_sanitize( cheer["t"] | "", slv_cheer_text, sizeof( slv_cheer_text ) );
    slovingo_sanitize( cheer["n"] | "", slv_cheer_trans, sizeof( slv_cheer_trans ) );
}

static void slv_next_question( void ) {
    slv_q_pos++;
    if ( slv_q_pos >= slv_q_cnt ) {
        if ( slv_q_cnt > 0 && slv_cur_sheet >= 0 ) {
            const char *id = slv_cur_sheet_obj()["id"] | "";

            slv_prev_best = slovingo_progress_best( slovingo_config.url, id );
            slv_new_record = slovingo_progress_record( slovingo_config.url, id, slv_score * 100 / slv_q_cnt );
        }
        else {
            slv_prev_best = -1;
            slv_new_record = false;
        }
        slv_streak = slovingo_streak_record( slovingo_day_of( time( NULL ) ) );
        slv_pick_cheer();
        slv_goto( SCR_RESULT );
        return;
    }
    slv_make_question( slv_q_order[ slv_q_pos ] );
    slv_goto( SCR_QUIZ );
}

/* ------------------------------------------------------------------------
 * worker
 * ---------------------------------------------------------------------- */

static bool slv_course_known( const char *id ) {
    for ( int i = 0; i < slv_course_cnt; i++ )
        if ( !strcmp( slv_course[ i ].id, id ) )
            return( true );
    return( false );
}

/* fill the list from courses.json (doc may be NULL: offline, the list stays empty) */
static void slv_build_courses( JsonDocument *doc ) {
    slv_course_cnt = 0;

    if ( doc ) {
        for ( JsonObject entry : (*doc)["courses"].as<JsonArray>() ) {
            const char *id = entry["id"] | "";
            const char *api = entry["api"] | "";
            const char *title = entry["title"] | "";
            size_t len = strlen( api );

            if ( slv_course_cnt >= SLV_MAX_COURSES || !*id || !len || slv_course_known( id ) )
                continue;
            if ( len >= 10 && !strcmp( api + len - 10, "index.json" ) )
                len -= 10;      /* the url of a course is its folder */
            slv_course_t *c = &slv_course[ slv_course_cnt ];
            if ( strlen( SLOVINGO_LAUNCHER_URL ) + len >= sizeof( c->url ) )
                continue;
            snprintf( c->id, sizeof( c->id ), "%s", id );
            slovingo_sanitize( *title ? title : id, c->label, sizeof( c->label ) );
            snprintf( c->url, sizeof( c->url ), "%s%.*s", SLOVINGO_LAUNCHER_URL, (int)len, api );
            slv_course_cnt++;
        }
    }
}

static void slv_worker( void *arg ) {
    SpiRamJsonDocument *doc = NULL;

    slv_res = slovingo_fetch_json( slv_req == REQ_COURSES ? SLOVINGO_LAUNCHER_URL : slovingo_config.url, slv_req_path, slv_req_force, &doc );
    slv_res_doc = doc;
    slv_done = true;
#ifndef NATIVE_64BIT
    vTaskDelete( NULL );
#endif
}

static void slv_request( slv_req_t req, const char *path, bool force ) {
    if ( slv_busy )
        return;

    char copy[ sizeof( slv_req_path ) ];
    snprintf( copy, sizeof( copy ), "%s", path );

    slv_req = req;
    snprintf( slv_req_path, sizeof( slv_req_path ), "%s", copy );
    slv_req_force = force;
    slv_busy = true;
    slv_done = false;
    if ( slv_screen != SCR_LOADING )
        slv_screen_before_load = slv_screen;
    slv_goto( SCR_LOADING );

#ifdef NATIVE_64BIT
    slv_worker( NULL );
#else
    /* the tls handshake needs a big stack */
    if ( xTaskCreatePinnedToCore( slv_worker, "slovingo worker", 12288, NULL, 1, NULL, 1 ) != pdPASS ) {
        slv_busy = false;
        slv_res = SLOVINGO_ERR_MEM;
        snprintf( slv_error_text, sizeof( slv_error_text ), "%s", slovingo_result_text( SLOVINGO_ERR_MEM ) );
        slv_goto( SCR_ERROR );
    }
#endif
}

/*
 * lvgl task: applies the screen changes asked by the callbacks, and takes the
 * result of the worker
 */
static void slv_lv_task( lv_task_t * task ) {
    if ( slv_pending ) {
        slv_pending = false;
        slv_show( slv_pending_screen );
    }

    if ( slv_advance_at && slv_screen == SCR_QUIZ && slv_q_answered && lv_tick_get() >= slv_advance_at ) {
        slv_advance_at = 0;
        slv_next_question();
    }

    if ( !slv_done )
        return;

    slv_done = false;
    slv_busy = false;
    SpiRamJsonDocument *doc = slv_res_doc;
    slv_res_doc = NULL;

    if ( slv_req == REQ_COURSES ) {
        /* no list online: the static one does */
        slv_build_courses( slv_res == SLOVINGO_OK ? doc : NULL );
        delete doc;
        slv_show( SCR_COURSES );
        return;
    }

    if ( slv_res != SLOVINGO_OK || !doc ) {
        delete doc;
        snprintf( slv_error_text, sizeof( slv_error_text ), "%s", slovingo_result_text( slv_res ) );
        slv_show( SCR_ERROR );
        return;
    }

    switch ( slv_req ) {
        case REQ_COURSES:       break;
        case REQ_INDEX:         delete slv_index;
                                slv_index = doc;
                                slv_build_groups();
                                slv_show( SCR_GROUPS );
                                break;
        case REQ_SHEET:         delete slv_sheet;
                                slv_sheet = doc;
                                slv_speakers_build();
                                slv_reader_paginate();
                                slv_show( SCR_READER );
                                break;
        case REQ_EXERCISES:     delete slv_ex;
                                slv_ex = doc;
                                if ( slv_quiz_start() ) {
                                    slv_show( SCR_QUIZ );
                                }
                                else {
                                    snprintf( slv_error_text, sizeof( slv_error_text ), "%s", TXT_NO_EXERCISES );
                                    slv_show( SCR_ERROR );
                                }
                                break;
    }
}

/* ------------------------------------------------------------------------
 * screens
 * ---------------------------------------------------------------------- */

/*
 * confetti: small coloured squares falling over the result screen of a perfect quiz.
 * They are children of the tile (the page scrolls), do not take clicks, and are removed
 * when the screen changes
 */
#define SLV_CONFETTI    28
static lv_obj_t *slv_confetti[ SLV_CONFETTI ];
static int slv_confetti_cnt = 0;

static void slv_confetti_clear( void ) {
    for ( int i = 0; i < slv_confetti_cnt; i++ ) {
        lv_anim_del( slv_confetti[ i ], NULL );
        lv_obj_del( slv_confetti[ i ] );
    }
    slv_confetti_cnt = 0;
}

static void slv_confetti_start( void ) {
    static const uint32_t colors[] = { 0xFFD54F, 0xFF8A65, 0x81C784, 0x4FC3F7, 0xBA68C8, 0xF06292 };
    lv_coord_t width = lv_obj_get_width( slv_tile );
    lv_coord_t height = lv_obj_get_height( slv_tile );

    slv_confetti_clear();
    for ( int i = 0; i < SLV_CONFETTI; i++ ) {
        lv_obj_t *piece = lv_obj_create( slv_tile, NULL );
        int size = 5 + SLV_RAND() % 5;

        lv_obj_set_size( piece, size, size + SLV_RAND() % 4 );
        lv_obj_set_style_local_bg_color( piece, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex( colors[ SLV_RAND() % ( sizeof( colors ) / sizeof( colors[ 0 ] ) ) ] ) );
        lv_obj_set_style_local_bg_opa( piece, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER );
        lv_obj_set_style_local_border_width( piece, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0 );
        lv_obj_set_style_local_radius( piece, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 1 );
        lv_obj_set_click( piece, false );
        lv_obj_set_pos( piece, SLV_RAND() % ( width - size ), -12 );
        slv_confetti[ slv_confetti_cnt++ ] = piece;

        lv_anim_t anim;
        lv_anim_init( &anim );
        lv_anim_set_var( &anim, piece );
        lv_anim_set_exec_cb( &anim, (lv_anim_exec_xcb_t)lv_obj_set_y );
        lv_anim_set_values( &anim, -12, height );
        lv_anim_set_delay( &anim, SLV_RAND() % 1200 );
        lv_anim_set_time( &anim, 1400 + SLV_RAND() % 1400 );
        lv_anim_start( &anim );
    }
}

static void slv_goto( slv_screen_t screen ) {
    slv_pending_screen = screen;
    slv_pending = true;
}

/*
 * result screen: the fox is happy from half of the answers right, the rabbit joins
 * the party from 70 %
 */
static void slv_add_mascots( int pct ) {
    lv_obj_t *row = lv_cont_create( slv_page, NULL );

    lv_cont_set_layout( row, LV_LAYOUT_ROW_MID );
    lv_cont_set_fit( row, LV_FIT_TIGHT );
    lv_obj_set_style_local_bg_opa( row, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP );
    lv_obj_set_style_local_border_width( row, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0 );
    lv_obj_set_style_local_pad_all( row, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0 );
    lv_obj_set_style_local_pad_inner( row, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 6 );

    lv_obj_t *fox = lv_img_create( row, NULL );
    lv_img_set_src( fox, pct >= 50 ? &slv_fox_happy_64 : &slv_fox_sad_64 );
    if ( pct >= 70 ) {
        lv_obj_t *rabbit = lv_img_create( row, NULL );
        lv_img_set_src( rabbit, &slv_rabbit_happy_64 );
    }
}

/*
 * while a quiz or a sheet is on screen the watch stays on it when the screen goes
 * off, like the map app does; the setting of the user comes back when the screen changes
 */
static void slv_hold_screen( bool hold ) {
    if ( hold == slv_hold )
        return;
#ifndef NATIVE_64BIT
    if ( hold ) {
        slv_block_saved = display_get_block_return_maintile();
        display_set_block_return_maintile( true );
    }
    else {
        display_set_block_return_maintile( slv_block_saved );
    }
#endif
    slv_hold = hold;
}

static void slv_set_nav( bool prev_ok, bool next_ok ) {
    if ( prev_ok )
        lv_obj_clear_state( slv_prev_btn, LV_STATE_DISABLED );
    else
        lv_obj_add_state( slv_prev_btn, LV_STATE_DISABLED );
    if ( next_ok )
        lv_obj_clear_state( slv_next_btn, LV_STATE_DISABLED );
    else
        lv_obj_add_state( slv_next_btn, LV_STATE_DISABLED );
}

static void slv_show( slv_screen_t screen ) {
    char text[ 160 ];

    slv_screen = screen;
    slv_pending = false;
    slv_advance_at = 0;
    slv_item_cnt = 0;
    slv_url_textarea = NULL;
    slv_built_lbl = NULL;
    lv_page_clean( slv_page );
    slv_confetti_clear();

    lv_obj_set_hidden( slv_refresh_btn, !( screen == SCR_GROUPS || screen == SCR_ERROR ) );
    lv_obj_set_hidden( slv_setup_btn, screen != SCR_GROUPS );
    lv_obj_set_hidden( slv_progress_lbl, !( screen == SCR_QUIZ || screen == SCR_READER ) );
    slv_hold_screen( screen == SCR_QUIZ || screen == SCR_READER || screen == SCR_RESULT );
    lv_obj_set_hidden( slv_mascot, screen != SCR_QUIZ );
    /* previous and next: page of the reader, sheet of a sheet, series of a series */
    bool nav = screen == SCR_READER || screen == SCR_SHEET || screen == SCR_SHEETS;
    lv_obj_set_hidden( slv_prev_btn, !nav );
    lv_obj_set_hidden( slv_next_btn, !nav );
    if ( screen == SCR_SHEET )
        slv_set_nav( slv_neighbor_sheet( -1 ) >= 0, slv_neighbor_sheet( 1 ) >= 0 );
    else if ( screen == SCR_SHEETS )
        slv_set_nav( slv_cur_group > 0, slv_cur_group + 1 < slv_group_cnt );
    memset( slv_rd_more, 0, sizeof( slv_rd_more ) );

    switch ( screen ) {
        case SCR_LOADING:
            slv_add_bouncing_fox();
            slv_add_label( TXT_LOADING, true );
            break;

        case SCR_ERROR: {
            lv_obj_t *sad = lv_img_create( slv_page, NULL );
            lv_img_set_src( sad, &slv_fox_sad_64 );
            slv_add_label( slv_error_text, false );
            break;
        }

        case SCR_GROUPS: {
            /* the welcome line of the course, when it has one, beside the fox */
            const char *hello = slv_index ? (const char*)( (*slv_index)["greeting"]["t"] | "" ) : "";
            if ( *hello ) {
                char line[ 96 ];
                char trans[ 96 ];

                slv_clean( hello, line, sizeof( line ) );
                slv_clean( slv_index ? (const char*)( (*slv_index)["greeting"]["n"] | "" ) : "", trans, sizeof( trans ) );
                if ( *trans )
                    snprintf( text, sizeof( text ), "%s\n#A0A7B0 %s#", line, trans );
                else
                    snprintf( text, sizeof( text ), "%s", line );
                slv_add_head( &slv_fox_neutral_40, text, false, *trans != '\0' );
            }
            slv_add_label( slv_index ? ( (*slv_index)["title"] | TXT_COURSE ) : TXT_COURSE, true );
            int done[ SLV_MAX_GROUPS ];
            slv_count_done( done );
            for ( int i = 0; i < slv_group_cnt; i++ ) {
                if ( done[ i ] > 0 )
                    snprintf( text, sizeof( text ), "%s (%d/%d)", slv_groups[ i ].label, done[ i ], slv_groups[ i ].count );
                else
                    snprintf( text, sizeof( text ), "%s (%d)", slv_groups[ i ].label, slv_groups[ i ].count );
                slv_add_button_icon( text, slv_themes[ slv_groups[ i ].theme ].img, slv_themes[ slv_groups[ i ].theme ].color );
            }
            break;
        }

        case SCR_SHEETS:
            slv_collect_sheets( slv_cur_group );
            {
                uint8_t theme = slv_cur_group >= 0 ? slv_groups[ slv_cur_group ].theme : 0;
                char head[ 64 ];

                slovingo_sanitize( slv_cur_group >= 0 ? slv_groups[ slv_cur_group ].label : "", head, sizeof( head ) );
                slv_add_head( slv_themes[ theme ].img, head, true, false );
                for ( int i = 0; i < slv_sheet_cnt; i++ ) {
                    JsonObject sheet = (*slv_index)["sheets"][ slv_sheet_idx[ i ] ];
                    int best = slv_best_of( sheet );
                    if ( best >= 0 ) {
                        snprintf( text, sizeof( text ), "%s  #%s %d%%#", (const char*)( sheet["title"] | "?" ), slv_best_color( best ), best );
                        lv_obj_t *btn = slv_add_button( text );
                        slv_label_recolor( btn );
                        slv_tint( btn, slv_themes[ theme ].color );
                    }
                    else {
                        slv_tint( slv_add_button( sheet["title"] | "?" ), slv_themes[ theme ].color );
                    }
                }
            }
            break;

        case SCR_SHEET: {
            JsonObject sheet = slv_cur_sheet_obj();
            int playable = slv_playable_count( sheet );

            slovingo_sanitize( sheet["title"] | "?", text, sizeof( text ) );
            slv_add_head( &slv_fox_neutral_40, text, false, false );
            int best = slv_best_of( sheet );
            if ( best >= 0 ) {
                snprintf( text, sizeof( text ), "Meilleur score : #%s %d %%#", slv_best_color( best ), best );
                lv_label_set_recolor( slv_add_label( text, false ), true );
            }
            slv_tint( slv_add_button( TXT_READ ), 0x1F4E79 );
            if ( playable > 0 ) {
                snprintf( text, sizeof( text ), "%s (%d)", TXT_EXERCISES, playable );
                slv_tint( slv_add_button( text ), 0x8A4B08 );
            }
            else {
                slv_add_label( TXT_NO_EXERCISES, false );
            }
            break;
        }

        case SCR_READER:
            slv_reader_render();
            break;

        case SCR_QUIZ:
            slv_set_mood( MOOD_NEUTRAL );
            snprintf( text, sizeof( text ), "%d/%d  Score: %d", slv_q_pos + 1, slv_q_cnt, slv_score );
            lv_label_set_text( slv_progress_lbl, text );
            slv_add_label( slv_q.prompt, true );
            if ( *slv_q.hint )
                slv_add_label( slv_q.hint, false );
            if ( slv_q.kind == SLV_KIND_ORDER ) {
                slv_built_lbl = slv_add_label( "", false );
                /* the words flow on several lines, each button as wide as its word */
                lv_obj_t *flow = lv_cont_create( slv_page, NULL );
                lv_cont_set_layout( flow, LV_LAYOUT_PRETTY_TOP );
                lv_cont_set_fit2( flow, LV_FIT_NONE, LV_FIT_TIGHT );
                lv_obj_set_width( flow, slv_content_w() );
                lv_obj_set_style_local_bg_opa( flow, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP );
                lv_obj_set_style_local_border_width( flow, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0 );
                lv_obj_set_style_local_pad_left( flow, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0 );
                lv_obj_set_style_local_pad_right( flow, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0 );
                lv_obj_set_style_local_pad_top( flow, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0 );
                lv_obj_set_style_local_pad_bottom( flow, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0 );
                lv_obj_set_style_local_pad_inner( flow, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, SLV_ITEM_GAP );
                for ( int k = 0; k < slv_q.shown_cnt; k++ )
                    slv_add_chip( flow, slv_q.shown[ k ] );
                lv_tileview_add_element( slv_tile, flow );
            }
            else {
                for ( int i = 0; i < slv_q.choice_cnt; i++ )
                    slv_add_button( slv_q.choice[ i ] );
            }
            break;

        case SCR_RESULT: {
            int pct = slv_q_cnt ? slv_score * 100 / slv_q_cnt : 0;

            slv_add_mascots( pct );
            if ( *slv_cheer_text ) {
                lv_obj_t *cheer = slv_add_label( slv_cheer_text, true );
                lv_obj_set_style_local_text_color( cheer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex( pct >= 50 ? 0xFFB74D : SLV_COLOR_DIM ) );
                if ( *slv_cheer_trans )
                    slv_add_label( slv_cheer_trans, false );
            }
            snprintf( text, sizeof( text ), "Score : %d / %d", slv_score, slv_q_cnt );
            slv_add_label( text, false );
            if ( slv_new_record && slv_prev_best >= 0 )
                lv_label_set_recolor( slv_add_label( "#FFD54F Nouveau record !#", false ), true );
            else if ( slv_prev_best >= 0 ) {
                snprintf( text, sizeof( text ), "Record : %d %%", slv_prev_best );
                slv_add_label( text, false );
            }
            if ( slv_streak >= 2 ) {
                snprintf( text, sizeof( text ), "#FFB74D %d jours de suite !#", slv_streak );
                lv_label_set_recolor( slv_add_label( text, false ), true );
            }
            if ( pct >= 100 )
                slv_confetti_start();
            slv_add_button( TXT_AGAIN );
            if ( slv_neighbor_sheet( 1 ) >= 0 )
                slv_add_button( TXT_NEXT_SHEET );
            slv_add_button( TXT_BACK );
            break;
        }

        case SCR_COURSES:
            slv_add_label( TXT_CHOOSE_COURSE, true );
            if ( !slv_course_cnt )
                slv_add_label( TXT_NO_COURSES, false );
            for ( int i = 0; i < slv_course_cnt; i++ )
                slv_add_button( slv_course[ i ].label );
            slv_add_button( TXT_OTHER_URL );
            break;

        case SCR_SETUP:
            slv_add_label( TXT_URL_HELP, false );
            slv_url_textarea = lv_textarea_create( slv_page, NULL );
            lv_textarea_set_text( slv_url_textarea, slovingo_config.url );
            lv_textarea_set_max_length( slv_url_textarea, SLOVINGO_URL_LEN - 1 );
            lv_textarea_set_one_line( slv_url_textarea, true );
            lv_textarea_set_cursor_hidden( slv_url_textarea, true );
            lv_obj_set_width( slv_url_textarea, slv_content_w() );
            lv_obj_set_event_cb( slv_url_textarea, slv_textarea_event_cb );
            lv_tileview_add_element( slv_tile, slv_url_textarea );
            slv_add_button( TXT_SAVE );
            slv_add_button( TXT_DEFAULT );
            break;
    }

    for ( int i = 0; i < slv_item_cnt; i++ )
        lv_tileview_add_element( slv_tile, slv_item[ i ] );
    lv_obj_set_y( lv_page_get_scrollable( slv_page ), 0 );
}

/* ------------------------------------------------------------------------
 * events
 * ---------------------------------------------------------------------- */

static void slv_open_group( int item ) {
    if ( item < 0 || item >= slv_group_cnt )
        return;
    slv_cur_group = item;
    slv_goto( SCR_SHEETS );
}

static void slv_open_sheet( int item ) {
    if ( item < 0 || item >= slv_sheet_cnt )
        return;
    slv_select_sheet( slv_sheet_idx[ item ] );
    slv_goto( SCR_SHEET );
}

static void slv_open_data( slv_req_t req ) {
    char path[ 96 ];
    JsonObject sheet = slv_cur_sheet_obj();

    snprintf( path, sizeof( path ), "%s/%s.json", req == REQ_SHEET ? "sheets" : "exercises", (const char*)( sheet["id"] | "" ) );
    slv_request( req, path, false );
}

/* the list of the courses is downloaded once per start, then the screen is shown */
static void slv_open_courses( void ) {
    if ( ( slv_courses_tried && slv_course_cnt ) || slv_busy ) {
        slv_goto( SCR_COURSES );
        return;
    }
    slv_courses_tried = true;
    slv_request( REQ_COURSES, "courses.json", false );
}

/* the url of the config is the course to load: forget the current one and fetch its index */
static void slv_load_course( void ) {
    slovingo_config.chosen = true;
    slovingo_config.save();
    delete slv_index;
    slv_index = NULL;
    delete slv_sheet;
    slv_sheet = NULL;
    delete slv_ex;
    slv_ex = NULL;
    slv_build_groups();
    slv_request( REQ_INDEX, "index.json", true );
}

static void slv_item_event_cb( lv_obj_t * obj, lv_event_t event ) {
    if ( event != LV_EVENT_CLICKED )
        return;

    int item = slv_item_index( obj );
    if ( item < 0 )
        return;

    switch ( slv_screen ) {
        case SCR_GROUPS:    slv_open_group( item );
                            break;
        case SCR_SHEETS:    slv_open_sheet( item );
                            break;
        case SCR_SHEET:     if ( item == 0 )
                                slv_open_data( REQ_SHEET );
                            else
                                slv_open_data( REQ_EXERCISES );
                            break;
        case SCR_READER:    slv_reader_tap( item );
                            break;
        case SCR_QUIZ:      if ( slv_q.kind == SLV_KIND_ORDER )
                                slv_order_pick( item );
                            else
                                slv_answer( item );
                            break;
        case SCR_RESULT:    if ( item == 0 ) {
                                if ( slv_quiz_start() )
                                    slv_goto( SCR_QUIZ );
                            }
                            else if ( item == 1 && slv_neighbor_sheet( 1 ) >= 0 ) {
                                slv_read_next_sheet();
                            }
                            else {
                                slv_goto( SCR_SHEET );
                            }
                            break;
        case SCR_COURSES:   if ( item >= slv_course_cnt ) {
                                slv_goto( SCR_SETUP );
                                break;
                            }
                            snprintf( slovingo_config.url, sizeof( slovingo_config.url ), "%s", slv_course[ item ].url );
                            slv_load_course();
                            break;
        case SCR_SETUP:     if ( item == 0 ) {
                                if ( slv_url_textarea )
                                    snprintf( slovingo_config.url, sizeof( slovingo_config.url ), "%s", lv_textarea_get_text( slv_url_textarea ) );
                            }
                            else {
                                snprintf( slovingo_config.url, sizeof( slovingo_config.url ), "%s", SLOVINGO_DEFAULT_URL );
                            }
                            keyboard_hide();
                            slv_load_course();
                            break;
        default:            break;
    }
}

static void slv_back_event_cb( lv_obj_t * obj, lv_event_t event ) {
    if ( event != LV_EVENT_CLICKED )
        return;

    keyboard_hide();

    switch ( slv_screen ) {
        case SCR_LOADING:   /* the worker cannot be stopped, wait for its answer */
                            break;
        case SCR_GROUPS:    mainbar_jump_back();
                            break;
        case SCR_SHEETS:    slv_goto( SCR_GROUPS );
                            break;
        case SCR_SHEET:     slv_goto( SCR_SHEETS );
                            break;
        case SCR_READER:
        case SCR_QUIZ:
        case SCR_RESULT:    slv_goto( SCR_SHEET );
                            break;
        case SCR_SETUP:     slv_open_courses();
                            break;
        case SCR_COURSES:   if ( slv_index )
                                slv_goto( SCR_GROUPS );
                            else
                                mainbar_jump_back();
                            break;
        case SCR_ERROR:     if ( !slv_index )
                                slv_open_courses();
                            else
                                slv_goto( slv_screen_before_load == SCR_LOADING ? SCR_GROUPS : slv_screen_before_load );
                            break;
    }
}

static void slv_refresh_event_cb( lv_obj_t * obj, lv_event_t event ) {
    if ( event != LV_EVENT_CLICKED )
        return;

    if ( slv_screen == SCR_ERROR )
        slv_request( slv_req, slv_req_path, true );
    else if ( slv_screen == SCR_GROUPS )
        slv_request( REQ_INDEX, "index.json", true );
}

static void slv_setup_event_cb( lv_obj_t * obj, lv_event_t event ) {
    if ( event != LV_EVENT_CLICKED )
        return;
    if ( slv_screen == SCR_GROUPS )
        slv_open_courses();
}

/* open the reader on the next sheet; if the download fails the way back is the menu of that sheet */
static void slv_read_next_sheet( void ) {
    slv_select_sheet( slv_neighbor_sheet( 1 ) );
    slv_screen = SCR_SHEET;
    slv_open_data( REQ_SHEET );
}

/* dir -1 or 1: previous or next page of the reader (the next sheet after the last page),
   sheet of the sheet screen, series of the sheets screen */
static void slv_navigate( int dir ) {
    switch ( slv_screen ) {
        case SCR_READER:
            if ( slv_rd_cur + dir >= 0 && slv_rd_cur + dir < slv_rd_cnt ) {
                slv_rd_cur += dir;
                slv_goto( SCR_READER );
            }
            else if ( dir > 0 && slv_neighbor_sheet( 1 ) >= 0 ) {
                slv_read_next_sheet();
            }
            break;
        case SCR_SHEET:
            if ( slv_neighbor_sheet( dir ) >= 0 ) {
                slv_select_sheet( slv_neighbor_sheet( dir ) );
                slv_goto( SCR_SHEET );
            }
            break;
        case SCR_SHEETS:
            if ( slv_cur_group + dir >= 0 && slv_cur_group + dir < slv_group_cnt ) {
                slv_cur_group += dir;
                slv_goto( SCR_SHEETS );
            }
            break;
        default:
            break;
    }
}

static void slv_nav_event_cb( lv_obj_t * obj, lv_event_t event ) {
    if ( event != LV_EVENT_CLICKED || slv_busy )
        return;
    slv_navigate( obj == slv_prev_btn ? -1 : 1 );
}

/* swipe: only the pages of the reader, left for the next, right for the previous */
static void slv_gesture_event_cb( lv_obj_t * obj, lv_event_t event ) {
    if ( event != LV_EVENT_GESTURE || slv_screen != SCR_READER || slv_busy )
        return;

    switch ( lv_indev_get_gesture_dir( lv_indev_get_act() ) ) {
        case LV_GESTURE_DIR_LEFT:   slv_navigate( 1 );
                                    break;
        case LV_GESTURE_DIR_RIGHT:  slv_navigate( -1 );
                                    break;
        default:                    break;
    }
}

static void slv_textarea_event_cb( lv_obj_t * obj, lv_event_t event ) {
    if ( event == LV_EVENT_CLICKED )
        keyboard_set_textarea( obj );
}

static void slv_app_activate_cb( void ) {
    lv_task_set_prio( slv_task, LV_TASK_PRIO_MID );

    if ( !slv_index && !slv_busy && !slovingo_config.chosen ) {
        slv_open_courses();
    }
    else if ( !slv_index && !slv_busy && slv_screen != SCR_COURSES && slv_screen != SCR_SETUP ) {
        slv_screen = SCR_GROUPS;
        slv_request( REQ_INDEX, "index.json", false );
    }
    else if ( slv_screen == SCR_SHEET && !slv_pending ) {
        slv_goto( SCR_SHEET );
    }
}

static void slv_app_hibernate_cb( void ) {
    keyboard_hide();
    slv_confetti_clear();
    slv_advance_at = 0;
    slv_hold_screen( false );

    /* a series in progress keeps its exercises and comes back where it was: when the
       screen was off the answer given is counted and the next question is ready */
    bool series = ( slv_screen == SCR_QUIZ || slv_screen == SCR_RESULT ) && slv_ex;

    /* the big documents go, the index stays, the screens rebuild from it */
    delete slv_sheet;
    slv_sheet = NULL;
    if ( !series ) {
        delete slv_ex;
        slv_ex = NULL;
    }

    if ( series ) {
        if ( slv_screen == SCR_RESULT ) {
            slv_goto( SCR_RESULT );
        }
        else if ( slv_q_answered ) {
            slv_next_question();
        }
        else {
            slv_q.picked_cnt = 0;
            slv_goto( SCR_QUIZ );
        }
    }
    else if ( slv_screen == SCR_READER || slv_screen == SCR_QUIZ || slv_screen == SCR_RESULT )
        slv_goto( SCR_SHEET );
    else if ( slv_screen == SCR_SETUP )
        slv_open_courses();
}

void slovingo_app_main_setup( uint32_t tile_num ) {

    slovingo_config.load();
    slovingo_set_user_name( slovingo_config.name );

    mainbar_add_tile_activate_cb( tile_num, slv_app_activate_cb );
    mainbar_add_tile_hibernate_cb( tile_num, slv_app_hibernate_cb );

    slv_tile = mainbar_get_tile_obj( tile_num );

    lv_style_init( &slv_style16 );
    lv_style_set_text_font( &slv_style16, LV_STATE_DEFAULT, &slovingo_16px );
    lv_style_init( &slv_style22 );
    lv_style_set_text_font( &slv_style22, LV_STATE_DEFAULT, &slovingo_22px );

    lv_coord_t hor_res = lv_disp_get_hor_res( NULL );
    lv_coord_t ver_res = lv_disp_get_ver_res( NULL );

    /*
     * the page fills the tile down to the footer, its content is rebuilt by slv_show()
     */
    slv_page = lv_page_create( slv_tile, NULL );
    lv_obj_set_size( slv_page, hor_res, ver_res - THEME_ICON_SIZE );
    lv_obj_align( slv_page, slv_tile, LV_ALIGN_IN_TOP_MID, 0, 0 );
    lv_obj_add_style( slv_page, LV_OBJ_PART_MAIN, APP_STYLE );
    lv_page_set_scrlbar_mode( slv_page, LV_SCRLBAR_MODE_DRAG );
    lv_page_set_scrl_layout( slv_page, LV_LAYOUT_COLUMN_MID );
    lv_obj_set_style_local_pad_left( slv_page, LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, 0 );
    lv_obj_set_style_local_pad_right( slv_page, LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, 0 );
    lv_obj_set_style_local_pad_top( slv_page, LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, SLV_GAP );
    lv_obj_set_style_local_pad_bottom( slv_page, LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, SLV_GAP );
    lv_obj_set_style_local_pad_inner( slv_page, LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, SLV_ITEM_GAP );

    slv_exit_btn = wf_add_exit_button( slv_tile, slv_back_event_cb );
    lv_obj_align( slv_exit_btn, slv_tile, LV_ALIGN_IN_BOTTOM_LEFT, THEME_ICON_PADDING, -THEME_ICON_PADDING );

    /* quiz progress, right of the exit arrow (same row, the refresh button is hidden in the quiz) */
    slv_progress_lbl = lv_label_create( slv_tile, NULL );
    lv_label_set_long_mode( slv_progress_lbl, LV_LABEL_LONG_CROP );
    lv_obj_set_width( slv_progress_lbl, hor_res - 2 * THEME_ICON_SIZE - 3 * THEME_ICON_PADDING );
    lv_obj_add_style( slv_progress_lbl, LV_LABEL_PART_MAIN, &slv_style16 );
    lv_obj_align( slv_progress_lbl, slv_exit_btn, LV_ALIGN_OUT_RIGHT_MID, THEME_PADDING, 0 );
    lv_obj_set_hidden( slv_progress_lbl, true );

    /* the fox of the quiz, right of the footer where the other buttons are hidden */
    slv_mascot = lv_img_create( slv_tile, NULL );
    lv_img_set_src( slv_mascot, &slv_fox_neutral_40 );
    lv_obj_align( slv_mascot, slv_tile, LV_ALIGN_IN_BOTTOM_RIGHT, -THEME_ICON_PADDING, -THEME_ICON_PADDING );
    lv_obj_set_hidden( slv_mascot, true );

    /* reader: next page at the right of the footer, previous page left of it, the page counter sits right of the exit arrow */
    slv_next_btn = lv_btn_create( slv_tile, NULL );
    lv_obj_set_size( slv_next_btn, THEME_ICON_SIZE, THEME_ICON_SIZE );
    lv_obj_add_style( slv_next_btn, LV_BTN_PART_MAIN, ws_get_button_style() );
    lv_obj_set_event_cb( slv_next_btn, slv_nav_event_cb );
    lv_obj_align( slv_next_btn, slv_tile, LV_ALIGN_IN_BOTTOM_RIGHT, -THEME_ICON_PADDING, -THEME_ICON_PADDING );
    lv_label_set_text( lv_label_create( slv_next_btn, NULL ), LV_SYMBOL_RIGHT );
    lv_obj_set_hidden( slv_next_btn, true );

    slv_prev_btn = lv_btn_create( slv_tile, NULL );
    lv_obj_set_size( slv_prev_btn, THEME_ICON_SIZE, THEME_ICON_SIZE );
    lv_obj_add_style( slv_prev_btn, LV_BTN_PART_MAIN, ws_get_button_style() );
    lv_obj_set_event_cb( slv_prev_btn, slv_nav_event_cb );
    lv_obj_align( slv_prev_btn, slv_next_btn, LV_ALIGN_OUT_LEFT_MID, -THEME_PADDING, 0 );
    lv_label_set_text( lv_label_create( slv_prev_btn, NULL ), LV_SYMBOL_LEFT );
    lv_obj_set_hidden( slv_prev_btn, true );

    slv_refresh_btn = wf_add_refresh_button( slv_tile, slv_refresh_event_cb );
    lv_obj_align( slv_refresh_btn, slv_tile, LV_ALIGN_IN_BOTTOM_MID, 0, -THEME_ICON_PADDING );

    slv_setup_btn = wf_add_setup_button( slv_tile, slv_setup_event_cb );
    lv_obj_align( slv_setup_btn, slv_tile, LV_ALIGN_IN_BOTTOM_RIGHT, -THEME_ICON_PADDING, -THEME_ICON_PADDING );

    /* the swipes of the pages stop at the tile: the reader turns its pages with them */
    lv_obj_set_gesture_parent( slv_tile, false );
    lv_obj_set_event_cb( slv_tile, slv_gesture_event_cb );

    lv_tileview_add_element( slv_tile, slv_page );
    lv_tileview_add_element( slv_tile, slv_exit_btn );
    lv_tileview_add_element( slv_tile, slv_refresh_btn );
    lv_tileview_add_element( slv_tile, slv_setup_btn );
    lv_tileview_add_element( slv_tile, slv_progress_lbl );
    lv_tileview_add_element( slv_tile, slv_prev_btn );
    lv_tileview_add_element( slv_tile, slv_next_btn );

    slv_task = lv_task_create( slv_lv_task, 100, LV_TASK_PRIO_OFF, NULL );

    slv_show( SCR_LOADING );
}
