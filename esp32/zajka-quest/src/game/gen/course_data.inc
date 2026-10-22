// Genere par tools/build_course.py depuis le cours Slovingo-sk-fr-friends : ne pas modifier.
#include "../course.h"

static const Gloss ep0_dialog_g0[3] = { {"Električka", "le tram"}, {"ide pomaly", "avance lentement"}, {"Je popoludnie", "c'est l'après-midi"} };
static const Gloss ep0_dialog_g1[4] = { {"V električke", "dans le tram"}, {"sedí", "est assis"}, {"Pozerá von oknom", "il regarde par la fenêtre"}, {"a nič nehovorí", "et ne dit rien"} };
static const Gloss ep0_dialog_g2[4] = { {"Oproti", "en face"}, {"sedí", "est assise"}, {"zajačica", "une hase"}, {"a číta knihu", "et lit un livre"} };
static const Gloss ep0_dialog_g3[3] = { {"Počkaj", "attends"}, {"To nie je", "ce n'est pas"}, {"možné", "possible"} };
static const Gloss ep0_dialog_g4[1] = { {"To si ty", "c'est toi"} };
static const Gloss ep0_dialog_g5[3] = { {"To si", "c'est"}, {"naozaj", "vraiment"}, {"ty", "toi"} };
static const Gloss ep0_dialog_g6[2] = { {"Áno", "oui"}, {"To som ja", "c'est moi"} };
static const Gloss ep0_dialog_g7[3] = { {"Ty si tu", "tu es là"}, {"V Košiciach", "à Košice"}, {"V električke", "dans le tram"} };
static const Gloss ep0_dialog_g8[2] = { {"Ako", "comme"}, {"vidíš", "tu vois"} };
static const Line ep0_dialog[9] = {
  {SPK_NARRATOR, "Košice. Električka ide pomaly. Je popoludnie.", "Košice. Le tram avance lentement. C'est l'après-midi.", ep0_dialog_g0, 3, "Le Narrateur raconte la scène en quelques mots. Pour comprendre : pomaly = lentement, popoludnie = l'après-midi."},
  {SPK_NARRATOR, "V električke sedí \001. Pozerá von oknom a nič nehovorí.", "Dans le tram, \001 est assis. Il regarde par la fenêtre et ne dit rien.", ep0_dialog_g1, 4, "Pour comprendre : sedí = il est assis, pozerá = il regarde, nič = rien."},
  {SPK_NARRATOR, "Oproti sedí zajačica a číta knihu.", "En face, une hase est assise et lit un livre.", ep0_dialog_g2, 4, "Pour comprendre : zajačica = la hase, la femelle du lièvre. číta knihu = elle lit un livre."},
  {SPK_FOX, "Počkaj... To nie je možné.", "Attends... C'est pas possible.", ep0_dialog_g3, 3, "počkaj = attends. On le dit à quelqu'un qu'on tutoie… ou à soi-même !"},
  {SPK_FOX, "Andrea? To si ty?", "Andrea ? C'est toi ?", ep0_dialog_g4, 1, "Pour dire « c'est toi », on ajoute si : To si ty. Pour « c'est moi » : To som ja."},
  {SPK_ANDREA, "\001? To si naozaj ty?", "\001 ? C'est vraiment toi ?", ep0_dialog_g5, 3, nullptr},
  {SPK_FOX, "Áno! To som ja!", "Oui ! C'est moi !", ep0_dialog_g6, 2, nullptr},
  {SPK_ANDREA, "Ty si tu? V Košiciach? V električke?", "Tu es là ? À Košice ? Dans le tram ?", ep0_dialog_g7, 3, "v Košiciach = à Košice. Comme dans v električke, le nom change un peu à la fin après v. Retiens-le d'un bloc."},
  {SPK_FOX, "Ako vidíš.", "Comme tu vois.", ep0_dialog_g8, 2, "Pour comprendre : vidíš = tu vois. Tu vas le revoir très vite."},
};
static const Gloss ep0_extra_g0[3] = { {"Kde je", "où est"}, {"zastávka", "l'arrêt"}, {"v meste", "en ville"} };
static const Gloss ep0_extra_g1[3] = { {"To je", "c'est"}, {"môj", "mon"}, {"lístok", "le ticket"} };
static const Line ep0_extra[2] = {
  {SPK_NONE, "Kde je zastávka v meste?", "Où est l'arrêt, en ville ?", ep0_extra_g0, 3, "v meste = en ville. C'est comme v električke : v + un mot qui change un peu à la fin."},
  {SPK_NONE, "To je môj lístok.", "C'est mon ticket.", ep0_extra_g1, 3, nullptr},
};
static const Word ep0_words[10] = {
  {"ja", "moi, je"},
  {"ty", "toi, tu"},
  {"som", "je suis"},
  {"si", "tu es"},
  {"áno", "oui (rappel)"},
  {"tu", "ici, là"},
  {"naozaj", "vraiment"},
  {"počkaj", "attends"},
  {"električka", "le tram"},
  {"v električke", "dans le tram"},
};
static const char* const ep0_x0_csk[3] = { "som", "je", "áno" };
static const char* const ep0_x0_cfr[3] = { "je suis", "il est, elle est", "oui" };
static const char* const ep0_x1_csk[3] = { "počkaj", "tu", "električka" };
static const char* const ep0_x1_cfr[3] = { "attends", "ici, là", "le tram" };
static const char* const ep0_x2_csk[3] = { "naozaj", "tu", "si" };
static const char* const ep0_x2_cfr[3] = { "vraiment", "ici, là", "tu es" };
static const char* const ep0_x3_csk[3] = { "ty", "ja", "si" };
static const char* const ep0_x3_cfr[3] = { "toi, tu", "moi, je", "tu es" };
static const char* const ep0_x4_csk[3] = { "lístok", "mesto", "električka" };
static const char* const ep0_x4_cfr[3] = { "le ticket", "la ville", "le tram" };
static const char* const ep0_x5_cfr[3] = { "Andrea ? C'est moi ?", "Andrea ? Tu es là ?", "Andrea ? C'est vraiment moi ?" };
static const char* const ep0_x6_cfr[3] = { "Oui ! C'est toi !", "Attends ! C'est moi !", "Oui ! C'est vraiment toi !" };
static const char* const ep0_x7_cfr[3] = { "Je suis là ? À Košice ? Dans le tram ?", "Tu es là ? À Košice ? En ville ?", "C'est toi ? À Košice ? Dans le tram ?" };
static const char* const ep0_x8_tsk[3] = { "Počkaj...", "To nie je", "možné." };
static const char* const ep0_x8_tfr[3] = { "Attends...", "C'est pas", "possible." };
static const char* const ep0_x9_tsk[3] = { "Ty si tu?", "V Košiciach?", "V električke?" };
static const char* const ep0_x9_tfr[3] = { "Tu es là ?", "À Košice ?", "Dans le tram ?" };
static const Exercise ep0_exercises[10] = {
  {EX_QCM, "qcm-si", "si", "tu es", ep0_x0_csk, ep0_x0_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-naozaj", "naozaj", "vraiment", ep0_x1_csk, ep0_x1_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-po-kaj", "počkaj", "attends", ep0_x2_csk, ep0_x2_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-tu", "tu", "ici, là", ep0_x3_csk, ep0_x3_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-zast-vka", "zastávka", "l'arrêt", ep0_x4_csk, ep0_x4_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-andrea-to-si-ty", "Andrea? To si ty?", "Andrea ? C'est toi ?", nullptr, ep0_x5_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-no-to-som-ja", "Áno! To som ja!", "Oui ! C'est moi !", nullptr, ep0_x6_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-ty-si-tu-v-ko-iciach-v-elektri-ke", "Ty si tu? V Košiciach? V električke?", "Tu es là ? À Košice ? Dans le tram ?", nullptr, ep0_x7_cfr, 3, nullptr, nullptr, 0},
  {EX_ORDER, "order-po-kaj-to-nie-je-mo-n", "Počkaj... To nie je možné.", "Attends... C'est pas possible.", nullptr, nullptr, 0, ep0_x8_tsk, ep0_x8_tfr, 3},
  {EX_ORDER, "order-ty-si-tu-v-ko-iciach-v-elektri-ke", "Ty si tu? V Košiciach? V električke?", "Tu es là ? À Košice ? Dans le tram ?", nullptr, nullptr, 0, ep0_x9_tsk, ep0_x9_tfr, 3},
};
static const Episode ep0 = { "10_Series_01_Kosice_01_to-si-ty", "Košice", 1, 1, "To si ty?", "Košice",
  ep0_dialog, 9, ep0_extra, 2, ep0_words, 10, ep0_exercises, 10 };

const Episode* const COURSE_EPISODES[] = { &ep0 };
const int COURSE_NUM_EPISODES = 1;
const char* const COURSE_DEFAULT_NAME = "Eric";
const char* const SPEAKER_NAMES[SPK_COUNT] = { "", "", "\001", "Andrea", "Maťo", "Katka", "Babka Zuzana", "Pán Orol", "Pani Ježková", "", "Ján", "Marek", "", "" };
