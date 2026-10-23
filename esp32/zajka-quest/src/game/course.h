// Contenu du cours Zajka Friends, converti par tools/build_course.py (donnees dans gen/course_data.inc).
// Portable : le simulateur PC utilise les memes donnees que le firmware.
//
// Les textes sont en UTF-8. Le nom du joueur est note NAME_MARK (\x01) dans les repliques :
// le jeu le remplace a l'affichage par le nom choisi au depart (COURSE_DEFAULT_NAME par defaut).
#pragma once
#include <stdint.h>

constexpr char NAME_MARK = '\x01';

// Qui parle (l'emoji de la replique dans le cours).
enum Speaker : uint8_t {
  SPK_NONE,      // replique sans avatar (« Encore quelques repliques »)
  SPK_NARRATOR,  // 💬
  SPK_FOX,       // 🦊 le joueur
  SPK_ANDREA,    // 🐰
  SPK_MATO,      // 🐻
  SPK_KATKA,     // 🐹
  SPK_BABKA,     // 🐑 Babka Zuzana
  SPK_OROL,      // 🦅 Pan Orol
  SPK_JEZKOVA,   // 🦔 Pani Jezkova
  SPK_HUMAN,     // 🧑 un adulte humain
  SPK_JAN,       // 🐿️ Jan
  SPK_MAREK,     // 🦡 Marek
  SPK_MOUSE,     // 🐭 (figurant)
  SPK_CHICK,     // 🐥 (figurant)
  SPK_COUNT
};

// Une explication mot a mot : « slovaque = francais ».
struct Gloss {
  const char* sk;
  const char* fr;
};

// Une replique du renard peut devenir un choix, tire d'un exercice du cours :
//  CH_ORDER : remettre les morceaux de la phrase dans l'ordre (exercice « order ») ; items = l'ordre juste
//  CH_PICK  : choisir la bonne phrase (exercice « qcm » : le mot remplace par les mauvaises reponses) ;
//             items[0] = la bonne, le jeu melange
enum ChoiceType : uint8_t { CH_ORDER, CH_PICK };

struct Choice {
  ChoiceType type;
  const char* exercise;  // id de l'exercice d'origine
  const char* const* items;
  uint8_t n;
};

struct Line {
  Speaker speaker;
  const char* sk;      // la replique
  const char* fr;      // sa traduction (SELECT)
  const Gloss* gloss;  // explications mot a mot
  uint8_t nGloss;
  const char* help;    // bulle d'aide (« + » dans le cours), ou nullptr
  const Choice* choice;  // replique du renard a reconstituer, ou nullptr
};

struct Word {
  const char* sk;
  const char* fr;
};

enum ExType : uint8_t { EX_QCM, EX_LISTEN, EX_ORDER };

// Un exercice du cours, repris dans le jeu sous forme de choix de replique ou de question d'un PNJ.
//  EX_QCM    : sk / fr = le mot ; choicesSk / choicesFr = les mauvaises reponses (meme ordre)
//  EX_LISTEN : sk = la phrase entendue, fr = son sens ; choicesFr = les mauvais sens
//  EX_ORDER  : sk / fr = la phrase ; tokensSk / tokensFr = ses morceaux dans le bon ordre
struct Exercise {
  ExType type;
  const char* id;
  const char* sk;
  const char* fr;
  const char* const* choicesSk;
  const char* const* choicesFr;
  uint8_t nChoices;
  const char* const* tokensSk;
  const char* const* tokensFr;
  uint8_t nTokens;
};

struct Episode {
  const char* id;        // nom du fichier du cours, sans .md
  const char* series;    // « Košice »
  uint8_t seriesNum;     // 1
  uint8_t episodeNum;    // 1
  const char* title;     // « To si ty? »
  const char* place;     // premier lieu des lignes « % » (« Košice »)
  const Line* dialog;    // « Le dialogue »
  uint16_t nDialog;
  const Line* extra;     // « Encore quelques répliques » : pour les PNJ de passage
  uint16_t nExtra;
  const Word* words;     // « Les nouveaux mots »
  uint16_t nWords;
  const Exercise* exercises;
  uint16_t nExercises;
  const Line* bonus;     // repliques secretes (story/bonus.txt), une par bonne reponse sans traduction
  uint8_t nBonus;
};

// Une scene propre au jeu (story/lines.txt), SCRIPTS[SC_...] (gen/scripts.h).
struct Script {
  const Line* lines;
  uint8_t n;
};

extern const Episode* const COURSE_EPISODES[];
extern const Word COURSE_VOCAB[];  // tout le vocabulaire des fiches jouees (Slovnik)
extern const int COURSE_NUM_VOCAB;
extern const Script SCRIPTS[];
extern const int NUM_SCRIPTS;
extern const int COURSE_NUM_EPISODES;
extern const char* const COURSE_DEFAULT_NAME;
extern const char* const SPEAKER_NAMES[SPK_COUNT];
