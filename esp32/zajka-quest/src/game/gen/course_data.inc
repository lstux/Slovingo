// Genere par tools/build_course.py depuis le cours Slovingo-sk-fr-friends : ne pas modifier.
#include "../course.h"

static const Gloss ep0_dialog_g0[3] = { {"Električka", "le tram"}, {"ide pomaly", "avance lentement"}, {"Je popoludnie", "c'est l'après-midi"} };
static const Gloss ep0_dialog_g1[4] = { {"V električke", "dans le tram"}, {"sedí", "est assis"}, {"Pozerá von oknom", "il regarde par la fenêtre"}, {"a nič nehovorí", "et ne dit rien"} };
static const Gloss ep0_dialog_g2[4] = { {"Oproti", "en face"}, {"sedí", "est assise"}, {"zajačica", "une hase"}, {"a číta knihu", "et lit un livre"} };
static const Gloss ep0_dialog_g3[3] = { {"Počkaj", "attends"}, {"To nie je", "ce n'est pas"}, {"možné", "possible"} };
static const char* const ep0_dialog_c3_items[3] = { "Počkaj...", "To nie je", "možné." };
static const Choice ep0_dialog_c3 = { CH_ORDER, "order-po-kaj-to-nie-je-mo-n", ep0_dialog_c3_items, 3 };
static const Gloss ep0_dialog_g4[1] = { {"To si ty", "c'est toi"} };
static const char* const ep0_dialog_c4_items[3] = { "Andrea? To si ty?", "Andrea? To som ty?", "Andrea? To je ty?" };
static const Choice ep0_dialog_c4 = { CH_PICK, "qcm-si", ep0_dialog_c4_items, 3 };
static const Gloss ep0_dialog_g5[3] = { {"To si", "c'est"}, {"naozaj", "vraiment"}, {"ty", "toi"} };
static const Gloss ep0_dialog_g6[2] = { {"Áno", "oui"}, {"To som ja", "c'est moi"} };
static const Gloss ep0_dialog_g7[3] = { {"Ty si tu", "tu es là"}, {"V Košiciach", "à Košice"}, {"V električke", "dans le tram"} };
static const Gloss ep0_dialog_g8[2] = { {"Ako", "comme"}, {"vidíš", "tu vois"} };
static const Line ep0_dialog[9] = {
  {SPK_NARRATOR, "Košice. Električka ide pomaly. Je popoludnie.", "Košice. Le tram avance lentement. C'est l'après-midi.", ep0_dialog_g0, 3, "Le Narrateur raconte la scène en quelques mots. Pour comprendre : pomaly = lentement, popoludnie = l'après-midi.", nullptr},
  {SPK_NARRATOR, "V električke sedí \001. Pozerá von oknom a nič nehovorí.", "Dans le tram, \001 est assis. Il regarde par la fenêtre et ne dit rien.", ep0_dialog_g1, 4, "Pour comprendre : sedí = il est assis, pozerá = il regarde, nič = rien.", nullptr},
  {SPK_NARRATOR, "Oproti sedí zajačica a číta knihu.", "En face, une hase est assise et lit un livre.", ep0_dialog_g2, 4, "Pour comprendre : zajačica = la hase, la femelle du lièvre. číta knihu = elle lit un livre.", nullptr},
  {SPK_FOX, "Počkaj... To nie je možné.", "Attends... C'est pas possible.", ep0_dialog_g3, 3, "počkaj = attends. On le dit à quelqu'un qu'on tutoie… ou à soi-même !", &ep0_dialog_c3},
  {SPK_FOX, "Andrea? To si ty?", "Andrea ? C'est toi ?", ep0_dialog_g4, 1, "Pour dire « c'est toi », on ajoute si : To si ty. Pour « c'est moi » : To som ja.", &ep0_dialog_c4},
  {SPK_ANDREA, "\001? To si naozaj ty?", "\001 ? C'est vraiment toi ?", ep0_dialog_g5, 3, nullptr, nullptr},
  {SPK_FOX, "Áno! To som ja!", "Oui ! C'est moi !", ep0_dialog_g6, 2, nullptr, nullptr},
  {SPK_ANDREA, "Ty si tu? V Košiciach? V električke?", "Tu es là ? À Košice ? Dans le tram ?", ep0_dialog_g7, 3, "v Košiciach = à Košice. Comme dans v električke, le nom change un peu à la fin après v. Retiens-le d'un bloc.", nullptr},
  {SPK_FOX, "Ako vidíš.", "Comme tu vois.", ep0_dialog_g8, 2, "Pour comprendre : vidíš = tu vois. Tu vas le revoir très vite.", nullptr},
};
static const Gloss ep0_extra_g0[3] = { {"Kde je", "où est"}, {"zastávka", "l'arrêt"}, {"v meste", "en ville"} };
static const Gloss ep0_extra_g1[3] = { {"To je", "c'est"}, {"môj", "mon"}, {"lístok", "le ticket"} };
static const Line ep0_extra[2] = {
  {SPK_NONE, "Kde je zastávka v meste?", "Où est l'arrêt, en ville ?", ep0_extra_g0, 3, "v meste = en ville. C'est comme v električke : v + un mot qui change un peu à la fin.", nullptr},
  {SPK_NONE, "To je môj lístok.", "C'est mon ticket.", ep0_extra_g1, 3, nullptr, nullptr},
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
static const Line ep0_bonus[2] = {
  {SPK_ANDREA, "Stále vieš po slovensky!", "Tu parles toujours slovaque !", nullptr, 0, nullptr, nullptr},
  {SPK_ANDREA, "Hovoríš ako Slovák!", "Tu parles comme un Slovaque !", nullptr, 0, nullptr, nullptr},
};
static const Episode ep0 = { "10_Series_01_Kosice_01_to-si-ty", "Košice", 1, 1, "To si ty?", "Košice",
  ep0_dialog, 9, ep0_extra, 2, ep0_words, 10, ep0_exercises, 10, ep0_bonus, 2 };

static const Gloss ep1_dialog_g0[2] = { {"Dlho", "longtemps"}, {"sme sa nevideli", "on ne s'est pas vus"} };
static const Gloss ep1_dialog_g1[3] = { {"Veľmi", "très"}, {"dlho", "longtemps"}, {"Päť rokov", "cinq ans"} };
static const char* const ep1_dialog_c1_items[3] = { "Veľmi", "dlho.", "Päť rokov." };
static const Choice ep1_dialog_c1 = { CH_ORDER, "order-ve-mi-dlho-p-rokov", ep1_dialog_c1_items, 3 };
static const Gloss ep1_dialog_g2[2] = { {"Päť rokov", "cinq ans"}, {"Ako sa máš", "comment ça va"} };
static const Gloss ep1_dialog_g3[3] = { {"Mám sa", "je me porte"}, {"super", "super"}, {"A ty", "et toi"} };
static const Gloss ep1_dialog_g4[4] = { {"Aj ja", "moi aussi"}, {"sa mám dobre", "je vais bien"}, {"Rada", "contente"}, {"ťa vidím", "je te vois"} };
static const Gloss ep1_dialog_g5[4] = { {"Aj ja", "moi aussi"}, {"ťa", "te"}, {"rád", "content"}, {"vidím", "je vois"} };
static const char* const ep1_dialog_c5_items[3] = { "Aj ja ťa rád vidím.", "Aj ja ťa rada vidím.", "Aj ja ťa dlho vidím." };
static const Choice ep1_dialog_c5 = { CH_PICK, "qcm-r-d", ep1_dialog_c5_items, 3 };
static const Line ep1_dialog[6] = {
  {SPK_ANDREA, "Dlho sme sa nevideli.", "On ne s'est pas vus depuis longtemps.", ep1_dialog_g0, 2, "À écouter : sme sa nevideli est du passé. Pour l'instant, retiens la phrase d'un bloc.", nullptr},
  {SPK_FOX, "Veľmi dlho. Päť rokov.", "Très longtemps. Cinq ans.", ep1_dialog_g1, 3, "veľmi = très, comme dans Ďakujem veľmi pekne. päť rokov = cinq ans : après päť, rok devient rokov.", &ep1_dialog_c1},
  {SPK_ANDREA, "Päť rokov! Ako sa máš?", "Cinq ans ! Comment ça va ?", ep1_dialog_g2, 2, nullptr, nullptr},
  {SPK_FOX, "Mám sa super! A ty?", "Ça va super ! Et toi ?", ep1_dialog_g3, 3, nullptr, nullptr},
  {SPK_ANDREA, "Aj ja sa mám dobre. Rada ťa vidím.", "Moi aussi, ça va bien. Je suis contente de te voir.", ep1_dialog_g4, 4, "Andrea est une fille : elle dit rada. Toi, tu es un garçon : tu dis rád.", nullptr},
  {SPK_FOX, "Aj ja ťa rád vidím.", "Moi aussi, je suis content de te voir.", ep1_dialog_g5, 4, nullptr, &ep1_dialog_c5},
};
static const Gloss ep1_extra_g0[2] = { {"Čau", "salut"}, {"Ako sa máš", "comment ça va"} };
static const Gloss ep1_extra_g1[2] = { {"Skvelé", "génial"}, {"Mám sa fajn", "ça va bien"} };
static const Line ep1_extra[2] = {
  {SPK_NONE, "Čau! Ako sa máš?", "Salut ! Comment ça va ?", ep1_extra_g0, 2, nullptr, nullptr},
  {SPK_NONE, "Skvelé! Mám sa fajn.", "Génial ! Ça va bien.", ep1_extra_g1, 2, nullptr, nullptr},
};
static const Word ep1_words[10] = {
  {"Ako sa máš?", "Comment vas-tu ? (rappel)"},
  {"Mám sa dobre.", "Je vais bien. (rappel)"},
  {"super", "super, génial"},
  {"A ty?", "Et toi ?"},
  {"aj ja", "moi aussi"},
  {"rád", "content (dit par un garçon)"},
  {"rada", "contente (dit par une fille)"},
  {"ťa", "te"},
  {"vidím", "je vois"},
  {"dlho", "longtemps"},
};
static const char* const ep1_x0_csk[3] = { "rada", "ťa", "dlho" };
static const char* const ep1_x0_cfr[3] = { "contente (dit par une fille)", "te", "longtemps" };
static const char* const ep1_x1_csk[3] = { "rád", "super", "aj ja" };
static const char* const ep1_x1_cfr[3] = { "content (dit par un garçon)", "super, génial", "moi aussi" };
static const char* const ep1_x2_csk[3] = { "A ty?", "ťa", "dlho" };
static const char* const ep1_x2_cfr[3] = { "Et toi ?", "te", "longtemps" };
static const char* const ep1_x3_csk[3] = { "super", "vidím", "ťa" };
static const char* const ep1_x3_cfr[3] = { "super, génial", "je vois", "te" };
static const char* const ep1_x4_csk[3] = { "ťa", "rád", "aj ja" };
static const char* const ep1_x4_cfr[3] = { "te", "content (dit par un garçon)", "moi aussi" };
static const char* const ep1_x5_cfr[3] = { "On ne se voit pas souvent.", "On se connaît depuis longtemps.", "On ne s'est jamais vus." };
static const char* const ep1_x6_cfr[3] = { "Ça va bien ! Et toi ?", "Ça va super ! Moi aussi.", "Je suis content ! Et toi ?" };
static const char* const ep1_x7_cfr[3] = { "Moi aussi, ça va super. Je suis contente de te voir.", "Moi aussi, ça va bien. Je suis content de te voir.", "Moi aussi, ça va bien. Ça fait longtemps." };
static const char* const ep1_x8_tsk[3] = { "Veľmi", "dlho.", "Päť rokov." };
static const char* const ep1_x8_tfr[3] = { "Très", "longtemps.", "Cinq ans." };
static const char* const ep1_x9_tsk[3] = { "Čau!", "Ako", "sa máš?" };
static const char* const ep1_x9_tfr[3] = { "Salut !", "Comment", "ça va ?" };
static const Exercise ep1_exercises[10] = {
  {EX_QCM, "qcm-r-d", "rád", "content (dit par un garçon)", ep1_x0_csk, ep1_x0_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-rada", "rada", "contente (dit par une fille)", ep1_x1_csk, ep1_x1_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-aj-ja", "aj ja", "moi aussi", ep1_x2_csk, ep1_x2_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-dlho", "dlho", "longtemps", ep1_x3_csk, ep1_x3_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-vid-m", "vidím", "je vois", ep1_x4_csk, ep1_x4_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-dlho-sme-sa-nevideli", "Dlho sme sa nevideli.", "On ne s'est pas vus depuis longtemps.", nullptr, ep1_x5_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-m-m-sa-super-a-ty", "Mám sa super! A ty?", "Ça va super ! Et toi ?", nullptr, ep1_x6_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-aj-ja-sa-m-m-dobre-rada-a-vid-m", "Aj ja sa mám dobre. Rada ťa vidím.", "Moi aussi, ça va bien. Je suis contente de te voir.", nullptr, ep1_x7_cfr, 3, nullptr, nullptr, 0},
  {EX_ORDER, "order-ve-mi-dlho-p-rokov", "Veľmi dlho. Päť rokov.", "Très longtemps. Cinq ans.", nullptr, nullptr, 0, ep1_x8_tsk, ep1_x8_tfr, 3},
  {EX_ORDER, "order-au-ako-sa-m", "Čau! Ako sa máš?", "Salut ! Comment ça va ?", nullptr, nullptr, 0, ep1_x9_tsk, ep1_x9_tfr, 3},
};
static const Line ep1_bonus[2] = {
  {SPK_ANDREA, "Tvoja slovenčina je stále dobrá!", "Ton slovaque est toujours bon !", nullptr, 0, nullptr, nullptr},
  {SPK_ANDREA, "Super, všetkému rozumieš!", "Super, tu comprends tout !", nullptr, 0, nullptr, nullptr},
};
static const Episode ep1 = { "10_Series_01_Kosice_02_ako-sa-mas", "Košice", 1, 2, "Ako sa máš?", "Košice",
  ep1_dialog, 6, ep1_extra, 2, ep1_words, 10, ep1_exercises, 10, ep1_bonus, 2 };

static const Gloss ep2_dialog_g0[3] = { {"Čo tu robíš", "qu'est-ce que tu fais ici"}, {"Bývaš", "tu habites"}, {"v Košiciach", "à Košice"} };
static const Gloss ep2_dialog_g1[4] = { {"Nie", "non"}, {"bývam v Lyone", "j'habite à Lyon"}, {"Som tu", "je suis ici"}, {"na prázdninách", "en vacances"} };
static const char* const ep2_dialog_c1_items[4] = { "Nie,", "bývam v Lyone.", "Som tu", "na prázdninách." };
static const Choice ep2_dialog_c1 = { CH_ORDER, "order-nie-b-vam-v-lyone-som-tu-na-pr-zdnin-ch", ep2_dialog_c1_items, 4 };
static const Gloss ep2_dialog_g2[1] = { {"Sám", "seul"} };
static const Gloss ep2_dialog_g3[2] = { {"Áno", "oui"}, {"sám", "seul"} };
static const char* const ep2_dialog_c3_items[3] = { "Áno, sám.", "Áno, asi.", "Áno, vie." };
static const Choice ep2_dialog_c3 = { CH_PICK, "qcm-s-m", ep2_dialog_c3_items, 3 };
static const Gloss ep2_dialog_g4[4] = { {"A", "et"}, {"tvoja mama", "ta maman"}, {"o tom", "de ça"}, {"vie", "sait"} };
static const Gloss ep2_dialog_g5[2] = { {"Vie", "elle sait"}, {"Asi", "je crois, sans doute"} };
static const char* const ep2_dialog_c5_items[3] = { "Vie. Asi.", "Vie. Sám.", "Vie. Čo." };
static const Choice ep2_dialog_c5 = { CH_PICK, "qcm-asi", ep2_dialog_c5_items, 3 };
static const Gloss ep2_dialog_g6[1] = { {"Fajn", "ok, bon"} };
static const Line ep2_dialog[7] = {
  {SPK_ANDREA, "Čo tu robíš? Bývaš v Košiciach?", "Qu'est-ce que tu fais ici ? Tu habites à Košice ?", ep2_dialog_g0, 3, nullptr, nullptr},
  {SPK_FOX, "Nie, bývam v Lyone. Som tu na prázdninách.", "Non, j'habite à Lyon. Je suis ici en vacances.", ep2_dialog_g1, 4, "v + une ville = « à » : v Lyone, v Košiciach. Le nom change un peu à la fin : retiens-le d'un bloc.", &ep2_dialog_c1},
  {SPK_ANDREA, "Sám?", "Tout seul ?", ep2_dialog_g2, 1, "Pour un garçon : sám. Pour une fille : sama.", nullptr},
  {SPK_FOX, "Áno, sám.", "Oui, tout seul.", ep2_dialog_g3, 2, nullptr, &ep2_dialog_c3},
  {SPK_ANDREA, "A tvoja mama o tom vie?", "Et ta maman est au courant ?", ep2_dialog_g4, 4, "Mot à mot : « Et ta maman sait de ça ? ». vie = elle sait, comme viem = je sais.", nullptr},
  {SPK_FOX, "Vie. Asi.", "Oui. Je crois.", ep2_dialog_g5, 2, "Pour répondre « oui » à une question, on répète souvent le verbe : Vie. = « elle sait ».", &ep2_dialog_c5},
  {SPK_ANDREA, "Fajn.", "Bon.", ep2_dialog_g6, 1, "Un mot tout court qui en dit long. Andrea n'ajoutera rien.", nullptr},
};
static const Gloss ep2_extra_g0[2] = { {"Ako", "comment"}, {"cestuješ", "tu voyages"} };
static const Gloss ep2_extra_g1[2] = { {"Cestujem", "je voyage"}, {"vlakom", "en train"} };
static const Line ep2_extra[2] = {
  {SPK_NONE, "Ako cestuješ?", "Comment voyages-tu ?", ep2_extra_g0, 2, nullptr, nullptr},
  {SPK_NONE, "Cestujem vlakom.", "Je voyage en train.", ep2_extra_g1, 2, "vlak = le train. Pour dire « en train », le mot change à la fin : vlakom.", nullptr},
};
static const Word ep2_words[10] = {
  {"čo", "quoi, que"},
  {"robím", "je fais"},
  {"robíš", "tu fais"},
  {"bývam", "j'habite"},
  {"bývaš", "tu habites"},
  {"v Lyone", "à Lyon"},
  {"na prázdninách", "en vacances"},
  {"sám", "seul (dit par un garçon)"},
  {"vie", "il sait, elle sait"},
  {"asi", "peut-être, je crois"},
};
static const char* const ep2_x0_csk[3] = { "robím", "bývaš", "vie" };
static const char* const ep2_x0_cfr[3] = { "je fais", "tu habites", "il sait, elle sait" };
static const char* const ep2_x1_csk[3] = { "bývaš", "robím", "som" };
static const char* const ep2_x1_cfr[3] = { "tu habites", "je fais", "je suis" };
static const char* const ep2_x2_csk[3] = { "asi", "vie", "rád" };
static const char* const ep2_x2_cfr[3] = { "peut-être, je crois", "il sait, elle sait", "content (dit par un garçon)" };
static const char* const ep2_x3_csk[3] = { "v Lyone", "v električke", "v meste" };
static const char* const ep2_x3_cfr[3] = { "à Lyon", "dans le tram", "en ville" };
static const char* const ep2_x4_csk[3] = { "sám", "čo", "naozaj" };
static const char* const ep2_x4_cfr[3] = { "seul (dit par un garçon)", "quoi, que", "vraiment" };
static const char* const ep2_x5_cfr[3] = { "Qu'est-ce que tu fais ici ? Tu habites à Lyon ?", "Où es-tu ? Tu habites à Košice ?", "Qu'est-ce que je fais ici ? J'habite à Košice ?" };
static const char* const ep2_x6_cfr[3] = { "Oui, j'habite à Lyon. Je suis ici en vacances.", "Non, j'habite à Košice. Je suis ici en vacances.", "Non, j'habite à Lyon. Je suis ici avec maman." };
static const char* const ep2_x7_cfr[3] = { "Et ta maman est là ?", "Et ta maman habite ici ?", "Et ta maman va bien ?" };
static const char* const ep2_x8_tsk[4] = { "Nie,", "bývam v Lyone.", "Som tu", "na prázdninách." };
static const char* const ep2_x8_tfr[4] = { "Non,", "j'habite à Lyon.", "Je suis ici", "en vacances." };
static const char* const ep2_x9_tsk[3] = { "A tvoja mama", "o tom", "vie?" };
static const char* const ep2_x9_tfr[3] = { "Et ta maman", "est", "au courant ?" };
static const Exercise ep2_exercises[10] = {
  {EX_QCM, "qcm-rob", "robíš", "tu fais", ep2_x0_csk, ep2_x0_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-b-vam", "bývam", "j'habite", ep2_x1_csk, ep2_x1_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-s-m", "sám", "seul (dit par un garçon)", ep2_x2_csk, ep2_x2_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-na-pr-zdnin-ch", "na prázdninách", "en vacances", ep2_x3_csk, ep2_x3_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-asi", "asi", "peut-être, je crois", ep2_x4_csk, ep2_x4_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-o-tu-rob-b-va-v-ko-iciach", "Čo tu robíš? Bývaš v Košiciach?", "Qu'est-ce que tu fais ici ? Tu habites à Košice ?", nullptr, ep2_x5_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-nie-b-vam-v-lyone-som-tu-na-pr-zdnin-ch", "Nie, bývam v Lyone. Som tu na prázdninách.", "Non, j'habite à Lyon. Je suis ici en vacances.", nullptr, ep2_x6_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-a-tvoja-mama-o-tom-vie", "A tvoja mama o tom vie?", "Et ta maman est au courant ?", nullptr, ep2_x7_cfr, 3, nullptr, nullptr, 0},
  {EX_ORDER, "order-nie-b-vam-v-lyone-som-tu-na-pr-zdnin-ch", "Nie, bývam v Lyone. Som tu na prázdninách.", "Non, j'habite à Lyon. Je suis ici en vacances.", nullptr, nullptr, 0, ep2_x8_tsk, ep2_x8_tfr, 4},
  {EX_ORDER, "order-a-tvoja-mama-o-tom-vie", "A tvoja mama o tom vie?", "Et ta maman est au courant ?", nullptr, nullptr, 0, ep2_x9_tsk, ep2_x9_tfr, 3},
};
static const Line ep2_bonus[1] = {
  {SPK_ANDREA, "Sám na prázdninách? Si odvážny!", "Tout seul en vacances ? Tu es courageux !", nullptr, 0, nullptr, nullptr},
};
static const Episode ep2 = { "10_Series_01_Kosice_03_co-tu-robis", "Košice", 1, 3, "Čo tu robíš?", "Košice",
  ep2_dialog, 7, ep2_extra, 2, ep2_words, 10, ep2_exercises, 10, ep2_bonus, 1 };

static const Gloss ep3_dialog_g0[2] = { {"Pamätáš sa", "tu te souviens"}, {"na Tatry", "des Tatras"} };
static const char* const ep3_dialog_c0_items[3] = { "Pamätáš sa na Tatry?", "Pamätám sa na Tatry?", "Všetko na Tatry?" };
static const Choice ep3_dialog_c0 = { CH_PICK, "qcm-pam-t-sa", ep3_dialog_c0_items, 3 };
static const Gloss ep3_dialog_g1[4] = { {"Pamätám sa", "je me souviens"}, {"na všetko", "de tout"}, {"Aj na to", "même de ça"}, {"ako som sa ťa bála", "comme j'avais peur de toi"} };
static const Gloss ep3_dialog_g2[3] = { {"Veď", "mais enfin"}, {"som bol", "j'étais"}, {"milý", "gentil"} };
static const Gloss ep3_dialog_g3[4] = { {"Bol", "tu l'étais"}, {"To bolo", "c'était"}, {"na tom", "là-dedans"}, {"divné", "bizarre"} };
static const Gloss ep3_dialog_g4[2] = { {"A ako sa majú", "et comment vont"}, {"Katka a Maťo", "Katka et Maťo"} };
static const Gloss ep3_dialog_g5[3] = { {"Dobre", "bien"}, {"Katka má trinásť", "Katka a treize ans"}, {"a Maťo štrnásť", "et Maťo quatorze"} };
static const Gloss ep3_dialog_g6[3] = { {"Katka trinásť", "Katka, treize ans"}, {"Veď", "mais enfin"}, {"mala osem", "elle avait huit ans"} };
static const char* const ep3_dialog_c6_items[3] = { "Katka trinásť? Veď mala osem!", "Katka jedenásť? Veď mala osem!", "Katka štrnásť? Veď mala osem!" };
static const Choice ep3_dialog_c6 = { CH_PICK, "qcm-trin-s", ep3_dialog_c6_items, 3 };
static const Gloss ep3_dialog_g7[4] = { {"Ty si mal desať", "toi, tu avais dix ans"}, {"A ja som mala jedenásť", "et moi, j'avais onze ans"}, {"Teraz", "maintenant"}, {"mám šestnásť", "j'ai seize ans"} };
static const Gloss ep3_dialog_g8[2] = { {"A ja", "et moi"}, {"pätnásť", "quinze"} };
static const char* const ep3_dialog_c8_items[3] = { "A ja pätnásť.", "A ja šestnásť.", "A ja štrnásť." };
static const Choice ep3_dialog_c8 = { CH_PICK, "qcm-p-tn-s", ep3_dialog_c8_items, 3 };
static const Gloss ep3_dialog_g9[3] = { {"Vidíš", "tu vois"}, {"Čas", "le temps"}, {"ide", "passe"} };
static const Line ep3_dialog[10] = {
  {SPK_FOX, "Pamätáš sa na Tatry?", "Tu te souviens des Tatras ?", ep3_dialog_g0, 2, "On dit « se souvenir na quelque chose ». Retiens pamätáš sa na d'un bloc.", &ep3_dialog_c0},
  {SPK_ANDREA, "Pamätám sa na všetko. Aj na to, ako som sa ťa bála.", "Je me souviens de tout. Même de la peur que j'avais de toi.", ep3_dialog_g1, 4, "À écouter : bála som sa = j'avais peur, dit par une fille.", nullptr},
  {SPK_FOX, "Veď som bol milý!", "Mais j'étais gentil !", ep3_dialog_g2, 3, "veď = « mais enfin », « puisque » : on le met en début de phrase quand on proteste.", nullptr},
  {SPK_ANDREA, "Bol. To bolo na tom divné.", "Oui. C'est ça qui était bizarre.", ep3_dialog_g3, 4, "Pour répondre « oui », Andrea répète le verbe : Bol. = « tu l'étais ».", nullptr},
  {SPK_FOX, "A ako sa majú Katka a Maťo?", "Et comment vont Katka et Maťo ?", ep3_dialog_g4, 2, "Ako sa máš? = comment vas-tu ? Pour plusieurs : Ako sa majú? = comment vont-ils ?", nullptr},
  {SPK_ANDREA, "Dobre. Katka má trinásť a Maťo štrnásť.", "Bien. Katka a treize ans et Maťo quatorze.", ep3_dialog_g5, 3, "On peut sauter rokov (ans) quand on parle de l'âge : Katka má trinásť.", nullptr},
  {SPK_FOX, "Katka trinásť? Veď mala osem!", "Katka, treize ans ? Mais elle en avait huit !", ep3_dialog_g6, 3, nullptr, &ep3_dialog_c6},
  {SPK_ANDREA, "Ty si mal desať. A ja som mala jedenásť. Teraz mám šestnásť.", "Toi, tu en avais dix. Et moi, onze. Maintenant j'en ai seize.", ep3_dialog_g7, 4, "mal pour toi (un garçon), mala pour Andrea (une fille). Même logique que rád et rada.", nullptr},
  {SPK_FOX, "A ja pätnásť.", "Et moi quinze.", ep3_dialog_g8, 2, nullptr, &ep3_dialog_c8},
  {SPK_ANDREA, "Vidíš? Čas ide.", "Tu vois ? Le temps passe.", ep3_dialog_g9, 3, "Mot à mot : « le temps va ». čas = le temps qui passe (pas la météo).", nullptr},
};
static const Gloss ep3_extra_g0[2] = { {"Chodím", "je vais"}, {"na gymnázium", "au lycée"} };
static const Gloss ep3_extra_g1[3] = { {"Dóm", "la cathédrale"}, {"je", "est"}, {"veľký kostol", "une grande église"} };
static const Line ep3_extra[2] = {
  {SPK_NONE, "Chodím na gymnázium.", "Je vais au lycée.", ep3_extra_g0, 2, "chodím = je vais, tous les jours ou régulièrement. idem, c'est « je vais » en ce moment.", nullptr},
  {SPK_NONE, "Dóm je veľký kostol.", "Une cathédrale, c'est une grande église.", ep3_extra_g1, 3, nullptr, nullptr},
};
static const Word ep3_words[10] = {
  {"jedenásť", "onze"},
  {"trinásť", "treize"},
  {"štrnásť", "quatorze"},
  {"pätnásť", "quinze"},
  {"šestnásť", "seize"},
  {"pamätám sa", "je me souviens"},
  {"pamätáš sa", "tu te souviens"},
  {"všetko", "tout"},
  {"milý", "gentil (dit d'un garçon)"},
  {"teraz", "maintenant"},
};
static const char* const ep3_x0_csk[3] = { "šestnásť", "štrnásť", "trinásť" };
static const char* const ep3_x0_cfr[3] = { "seize", "quatorze", "treize" };
static const char* const ep3_x1_csk[3] = { "jedenásť", "štrnásť", "šestnásť" };
static const char* const ep3_x1_cfr[3] = { "onze", "quatorze", "seize" };
static const char* const ep3_x2_csk[3] = { "pamätám sa", "všetko", "teraz" };
static const char* const ep3_x2_cfr[3] = { "je me souviens", "tout", "maintenant" };
static const char* const ep3_x3_csk[3] = { "teraz", "milý", "pätnásť" };
static const char* const ep3_x3_cfr[3] = { "maintenant", "gentil (dit d'un garçon)", "quinze" };
static const char* const ep3_x4_csk[3] = { "všetko", "milý", "jedenásť" };
static const char* const ep3_x4_cfr[3] = { "tout", "gentil (dit d'un garçon)", "onze" };
static const char* const ep3_x5_cfr[3] = { "Tu habites dans les Tatras ?", "Je me souviens des Tatras.", "Tu vas dans les Tatras ?" };
static const char* const ep3_x6_cfr[3] = { "Bien. Katka a quatorze ans et Maťo treize.", "Bien. Katka a trois ans et Maťo quatre.", "Bien. Katka a seize ans et Maťo quinze." };
static const char* const ep3_x7_cfr[3] = { "Tu vois ? Le tram passe.", "Tu vois ? Il fait beau.", "Tu sais ? Le temps passe." };
static const char* const ep3_x8_cfr[3] = { "Toi, tu en avais onze. Et moi, dix. Maintenant j'en ai seize.", "Toi, tu en avais dix. Et moi, onze. Maintenant j'en ai quinze.", "Toi, tu en as dix. Et moi, onze. Maintenant j'en ai seize." };
static const char* const ep3_x9_tsk[3] = { "Ty si mal desať.", "A ja som mala jedenásť.", "Teraz mám šestnásť." };
static const char* const ep3_x9_tfr[3] = { "Toi, tu en avais dix.", "Et moi, onze.", "Maintenant j'en ai seize." };
static const char* const ep3_x10_tsk[3] = { "Chodím", "na", "gymnázium." };
static const char* const ep3_x10_tfr[3] = { "Je vais", "au", "lycée." };
static const Exercise ep3_exercises[11] = {
  {EX_QCM, "qcm-p-tn-s", "pätnásť", "quinze", ep3_x0_csk, ep3_x0_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-trin-s", "trinásť", "treize", ep3_x1_csk, ep3_x1_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-pam-t-sa", "pamätáš sa", "tu te souviens", ep3_x2_csk, ep3_x2_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-v-etko", "všetko", "tout", ep3_x3_csk, ep3_x3_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-teraz", "teraz", "maintenant", ep3_x4_csk, ep3_x4_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-pam-t-sa-na-tatry", "Pamätáš sa na Tatry?", "Tu te souviens des Tatras ?", nullptr, ep3_x5_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-dobre-katka-m-trin-s-a-ma-o-trn-s", "Dobre. Katka má trinásť a Maťo štrnásť.", "Bien. Katka a treize ans et Maťo quatorze.", nullptr, ep3_x6_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-vid-as-ide", "Vidíš? Čas ide.", "Tu vois ? Le temps passe.", nullptr, ep3_x7_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-ty-si-mal-desa-a-ja-som-mala-jeden-s-teraz-m-m-estn-s", "Ty si mal desať. A ja som mala jedenásť. Teraz mám šestnásť.", "Toi, tu en avais dix. Et moi, onze. Maintenant j'en ai seize.", nullptr, ep3_x8_cfr, 3, nullptr, nullptr, 0},
  {EX_ORDER, "order-ty-si-mal-desa-a-ja-som-mala-jeden-s-teraz-m-m-estn-s", "Ty si mal desať. A ja som mala jedenásť. Teraz mám šestnásť.", "Toi, tu en avais dix. Et moi, onze. Maintenant j'en ai seize.", nullptr, nullptr, 0, ep3_x9_tsk, ep3_x9_tfr, 3},
  {EX_ORDER, "order-chod-m-na-gymn-zium", "Chodím na gymnázium.", "Je vais au lycée.", nullptr, nullptr, 0, ep3_x10_tsk, ep3_x10_tfr, 3},
};
static const Line ep3_bonus[1] = {
  {SPK_ANDREA, "Pamätáš si to dobre!", "Tu t'en souviens bien !", nullptr, 0, nullptr, nullptr},
};
static const Episode ep3 = { "10_Series_01_Kosice_04_pamatas-sa", "Košice", 1, 4, "Pamätáš sa na Tatry?", "Košice",
  ep3_dialog, 10, ep3_extra, 2, ep3_words, 10, ep3_exercises, 11, ep3_bonus, 1 };

static const Gloss ep4_dialog_g0[5] = { {"začne rozprávať", "se met à parler"}, {"Rozpráva o", "il parle de"}, {"o Lyone", "de Lyon"}, {"o vlaku", "du train"}, {"o svojom lístku", "de son ticket"} };
static const Gloss ep4_dialog_g1[3] = { {"Košice sú super", "Košice, c'est super"}, {"Električka je super", "le tram, c'est super"}, {"môj lístok", "mon ticket"} };
static const Gloss ep4_dialog_g2[4] = { {"viem po slovensky", "je sais le slovaque"}, {"Skoro všetko", "presque tout"}, {"pamätám sa na", "je me souviens de"}, {"každé slovo", "chaque mot"} };
static const Gloss ep4_dialog_g3[5] = { {"Líšky", "les renards"}, {"veľa", "beaucoup"}, {"rozprávajú", "ils parlent"}, {"A sú komplikované", "et ils sont compliqués"}, {"ako mimozemšťania", "comme des extraterrestres"} };
static const Gloss ep4_dialog_g4[3] = { {"Ale", "mais"}, {"ja som", "moi, je suis"}, {"tvoj kamarát", "ton copain"} };
static const char* const ep4_dialog_c4_items[3] = { "Ale ja som tvoj kamarát!", "Ale ja som tvoj kostol!", "Ale ja som tvoj dóm!" };
static const Choice ep4_dialog_c4 = { CH_PICK, "qcm-kamar-t", ep4_dialog_c4_items, 3 };
static const Gloss ep4_dialog_g5[3] = { {"Áno", "oui"}, {"Ale", "mais"}, {"si líška", "tu es un renard"} };
static const Gloss ep4_dialog_g6[3] = { {"Ja", "moi"}, {"Ja som", "moi, je suis"}, {"jednoduchý", "simple"} };
static const char* const ep4_dialog_c6_items[3] = { "Ja? Ja som jednoduchý!", "Ja? Ja som komplikované!", "Ja? Ja som milý!" };
static const Choice ep4_dialog_c6 = { CH_PICK, "qcm-jednoduch", ep4_dialog_c6_items, 3 };
static const Gloss ep4_dialog_g7[1] = { {"Nie", "non"} };
static const Gloss ep4_dialog_g8[1] = { {"Aha", "ah, d'accord"} };
static const Gloss ep4_dialog_g9[1] = { {"Vidíš", "tu vois"} };
static const Gloss ep4_dialog_g10[3] = { {"Električka ide", "le tram passe"}, {"okolo", "devant, à côté de"}, {"veľkého dómu", "une grande cathédrale"} };
static const Gloss ep4_dialog_g11[3] = { {"Pozri", "regarde"}, {"To je", "c'est"}, {"dóm", "la cathédrale"} };
static const Gloss ep4_dialog_g12[4] = { {"Dóm svätej Alžbety", "la cathédrale Sainte-Élisabeth"}, {"Najväčší", "le plus grand"}, {"kostol", "l'église"}, {"na Slovensku", "en Slovaquie"} };
static const Gloss ep4_dialog_g13[2] = { {"A ty", "et toi"}, {"Bývaš tu", "tu habites ici"} };
static const Gloss ep4_dialog_g14[3] = { {"Áno", "oui"}, {"u mojej tety", "chez ma tante"}, {"Chodím na gymnázium", "je vais au lycée"} };
static const Gloss ep4_dialog_g15[5] = { {"Ty", "toi"}, {"Na gymnázium", "au lycée"}, {"Veď", "mais enfin"}, {"si behala", "tu courais"}, {"po lúke", "dans les prés"} };
static const Gloss ep4_dialog_g16[3] = { {"A ty", "et toi"}, {"si spadol", "tu es tombé"}, {"do potoka", "dans le ruisseau"} };
static const Gloss ep4_dialog_g17[2] = { {"To nie je", "ce n'est pas"}, {"pravda", "vrai"} };
static const char* const ep4_dialog_c17_items[3] = { "To nie je pravda.", "To nie je skoro.", "To nie je naozaj." };
static const Choice ep4_dialog_c17 = { CH_PICK, "qcm-pravda", ep4_dialog_c17_items, 3 };
static const Gloss ep4_dialog_g18[1] = { {"Je", "si, c'est vrai"} };
static const Gloss ep4_dialog_g19[2] = { {"Električka", "le tram"}, {"stojí", "s'arrête"} };
static const Gloss ep4_dialog_g20[3] = { {"To je", "c'est"}, {"moja zastávka", "mon arrêt"}, {"Poď", "viens"} };
static const Gloss ep4_dialog_g21[1] = { {"Idem", "j'arrive, je viens"} };
static const Line ep4_dialog[22] = {
  {SPK_NARRATOR, "\001 začne rozprávať. Rozpráva o Lyone, o vlaku, o Košiciach, o električke a o svojom lístku.", "\001 se met à parler. Il parle de Lyon, du train, de Košice, du tram et de son ticket.", ep4_dialog_g0, 5, "Pour comprendre : rozprávať = parler, raconter. Après o (« de, au sujet de »), les noms changent un peu à la fin, comme après v.", nullptr},
  {SPK_FOX, "Košice sú super! Električka je super! A môj lístok je super!", "Košice, c'est super ! Le tram, c'est super ! Et mon ticket, c'est super !", ep4_dialog_g1, 3, "Košice est un pluriel en slovaque : on dit Košice sú, un peu comme « les Košice sont ».", nullptr},
  {SPK_FOX, "A viem po slovensky! Skoro všetko! A pamätám sa na každé slovo!", "Et je parle slovaque ! Presque tout ! Et je me souviens de chaque mot !", ep4_dialog_g2, 4, "Mot nouveau signalé : skoro = presque. Pour comprendre : každé slovo = chaque mot.", nullptr},
  {SPK_ANDREA, "Líšky veľa rozprávajú. A sú komplikované ako mimozemšťania!", "Les renards, ça parle beaucoup. Et c'est compliqué comme des extraterrestres !", ep4_dialog_g3, 5, "Mots nouveaux signalés : veľa = beaucoup, rozprávajú = ils parlent, komplikované = compliqués, mimozemšťania = les extraterrestres. Attention : veľa = beaucoup, veľmi = très. Ce sont deux mots différents !", nullptr},
  {SPK_FOX, "Ale ja som tvoj kamarát!", "Mais moi, je suis ton copain !", ep4_dialog_g4, 3, "Mot nouveau signalé : kamarát = le copain, l'ami.", &ep4_dialog_c4},
  {SPK_ANDREA, "Áno. Ale si líška.", "Oui. Mais tu es un renard.", ep4_dialog_g5, 3, "líška est un mot féminin, même pour un renard garçon : c'est le mot qui compte, pas toi.", nullptr},
  {SPK_FOX, "Ja? Ja som jednoduchý!", "Moi ? Mais je suis simple !", ep4_dialog_g6, 3, "Mot nouveau signalé : jednoduchý = simple, dit d'un garçon. Une fille dirait jednoduchá.", &ep4_dialog_c6},
  {SPK_ANDREA, "Nie.", "Non.", ep4_dialog_g7, 1, "Pas d'explication. Andrea est comme ça.", nullptr},
  {SPK_FOX, "Aha.", "Ah.", ep4_dialog_g8, 1, nullptr, nullptr},
  {SPK_ANDREA, "Vidíš?", "Tu vois ?", ep4_dialog_g9, 1, "vidím (je vois) → vidíš (tu vois), comme robím → robíš.", nullptr},
  {SPK_NARRATOR, "Električka ide okolo veľkého dómu.", "Le tram passe devant une grande cathédrale.", ep4_dialog_g10, 3, nullptr, nullptr},
  {SPK_FOX, "Pozri! To je dóm!", "Regarde ! C'est la cathédrale !", ep4_dialog_g11, 3, nullptr, nullptr},
  {SPK_ANDREA, "Dóm svätej Alžbety. Najväčší kostol na Slovensku.", "La cathédrale Sainte-Élisabeth. La plus grande église de Slovaquie.", ep4_dialog_g12, 4, nullptr, nullptr},
  {SPK_FOX, "A ty? Bývaš tu?", "Et toi ? Tu habites ici ?", ep4_dialog_g13, 2, nullptr, nullptr},
  {SPK_ANDREA, "Áno, u mojej tety. Chodím na gymnázium.", "Oui, chez ma tante. Je vais au lycée.", ep4_dialog_g14, 3, "Mot nouveau signalé : u mojej tety = chez ma tante. teta = la tante ; après u (« chez »), elle devient tety.", nullptr},
  {SPK_FOX, "Ty? Na gymnázium? Veď si behala po lúke!", "Toi ? Au lycée ? Mais tu courais dans les prés !", ep4_dialog_g15, 5, "À écouter : behala si = tu courais, dit à une fille.", nullptr},
  {SPK_ANDREA, "A ty si spadol do potoka.", "Et toi, tu es tombé dans le ruisseau.", ep4_dialog_g16, 3, nullptr, nullptr},
  {SPK_FOX, "To nie je pravda.", "C'est pas vrai.", ep4_dialog_g17, 2, "Mot nouveau signalé : pravda = la vérité. To je pravda = c'est vrai.", &ep4_dialog_c17},
  {SPK_ANDREA, "Je.", "Si.", ep4_dialog_g18, 1, "Pour dire « si », Andrea répète simplement le verbe : Je.", nullptr},
  {SPK_NARRATOR, "Električka stojí.", "Le tram s'arrête.", ep4_dialog_g19, 2, nullptr, nullptr},
  {SPK_ANDREA, "To je moja zastávka. Poď!", "C'est mon arrêt. Viens !", ep4_dialog_g20, 3, nullptr, nullptr},
  {SPK_FOX, "Idem!", "J'arrive !", ep4_dialog_g21, 1, nullptr, nullptr},
};
static const char* const ep4_x0_csk[3] = { "skoro", "všetko", "naozaj" };
static const char* const ep4_x0_cfr[3] = { "presque", "tout", "vraiment" };
static const char* const ep4_x1_csk[3] = { "kostol", "dóm", "lístok" };
static const char* const ep4_x1_cfr[3] = { "l'église", "la cathédrale", "le ticket" };
static const char* const ep4_x2_csk[3] = { "komplikované", "milý", "sám" };
static const char* const ep4_x2_cfr[3] = { "compliqués", "gentil (dit d'un garçon)", "seul (dit par un garçon)" };
static const char* const ep4_x3_csk[3] = { "skoro", "naozaj", "všetko" };
static const char* const ep4_x3_cfr[3] = { "presque", "vraiment", "tout" };
static const char* const ep4_x4_cfr[3] = { "Les renards, ça parle beaucoup. Et c'est gentil comme des copains !", "Les renards, c'est très compliqué. Et ça ne parle pas !", "Les renards, ça parle peu. Et c'est simple comme des extraterrestres !" };
static const char* const ep4_x5_cfr[3] = { "Mais toi, tu es mon copain !", "Mais moi, je suis un renard !", "Mais moi, je suis simple !" };
static const char* const ep4_x6_cfr[3] = { "Oui, chez ma maman. Je vais au lycée.", "Oui, chez ma tante. Je vais à la cathédrale.", "Non, à Lyon. Je vais au lycée." };
static const char* const ep4_x7_cfr[3] = { "C'est vrai.", "C'est pas possible.", "C'est pas moi." };
static const char* const ep4_x8_cfr[3] = { "C'est mon ticket. Viens !", "C'est ma ville. Viens !", "C'est mon arrêt. Attends !" };
static const char* const ep4_x9_tsk[4] = { "Áno,", "u mojej tety.", "Chodím", "na gymnázium." };
static const char* const ep4_x9_tfr[4] = { "Oui,", "chez ma tante.", "Je vais", "au lycée." };
static const char* const ep4_x10_tsk[3] = { "Dóm svätej Alžbety.", "Najväčší kostol", "na Slovensku." };
static const char* const ep4_x10_tfr[3] = { "La cathédrale Sainte-Élisabeth.", "La plus grande église", "de Slovaquie." };
static const char* const ep4_x11_tsk[3] = { "To je", "moja zastávka.", "Poď!" };
static const char* const ep4_x11_tfr[3] = { "C'est", "mon arrêt.", "Viens !" };
static const Exercise ep4_exercises[12] = {
  {EX_QCM, "qcm-ve-a", "veľa", "beaucoup", ep4_x0_csk, ep4_x0_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-kamar-t", "kamarát", "le copain, l'ami", ep4_x1_csk, ep4_x1_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-jednoduch", "jednoduchý", "simple (dit d'un garçon)", ep4_x2_csk, ep4_x2_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-pravda", "pravda", "vrai, la vérité", ep4_x3_csk, ep4_x3_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-l-ky-ve-a-rozpr-vaj-a-s-komplikovan-ako-mimozem-ania", "Líšky veľa rozprávajú. A sú komplikované ako mimozemšťania!", "Les renards, ça parle beaucoup. Et c'est compliqué comme des extraterrestres !", nullptr, ep4_x4_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-ale-ja-som-tvoj-kamar-t", "Ale ja som tvoj kamarát!", "Mais moi, je suis ton copain !", nullptr, ep4_x5_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-no-u-mojej-tety-chod-m-na-gymn-zium", "Áno, u mojej tety. Chodím na gymnázium.", "Oui, chez ma tante. Je vais au lycée.", nullptr, ep4_x6_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-to-nie-je-pravda", "To nie je pravda.", "C'est pas vrai.", nullptr, ep4_x7_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-to-je-moja-zast-vka-po", "To je moja zastávka. Poď!", "C'est mon arrêt. Viens !", nullptr, ep4_x8_cfr, 3, nullptr, nullptr, 0},
  {EX_ORDER, "order-no-u-mojej-tety-chod-m-na-gymn-zium", "Áno, u mojej tety. Chodím na gymnázium.", "Oui, chez ma tante. Je vais au lycée.", nullptr, nullptr, 0, ep4_x9_tsk, ep4_x9_tfr, 4},
  {EX_ORDER, "order-d-m-sv-tej-al-bety-najv-kostol-na-slovensku", "Dóm svätej Alžbety. Najväčší kostol na Slovensku.", "La cathédrale Sainte-Élisabeth. La plus grande église de Slovaquie.", nullptr, nullptr, 0, ep4_x10_tsk, ep4_x10_tfr, 3},
  {EX_ORDER, "order-to-je-moja-zast-vka-po", "To je moja zastávka. Poď!", "C'est mon arrêt. Viens !", nullptr, nullptr, 0, ep4_x11_tsk, ep4_x11_tfr, 3},
};
static const Line ep4_bonus[2] = {
  {SPK_ANDREA, "Dobre, si trochu jednoduchý.", "Bon, tu es un peu simple.", nullptr, 0, nullptr, nullptr},
  {SPK_ANDREA, "Hovoríš ako Košičan!", "Tu parles comme quelqu'un de Košice !", nullptr, 0, nullptr, nullptr},
};
static const Episode ep4 = { "10_Series_01_Kosice_05_v-elektricke", "Košice", 1, 5, "V električke", "Košice",
  ep4_dialog, 22, nullptr, 0, nullptr, 0, ep4_exercises, 12, ep4_bonus, 2 };

static const Gloss ep5_dialog_g0[3] = { {"Je večer", "c'est le soir"}, {"Pred domom", "devant la maison"}, {"je veľká záhrada", "il y a un grand jardin"} };
static const Gloss ep5_dialog_g1[5] = { {"Tu býva", "ici habite"}, {"moja teta", "ma tante"}, {"Je milá", "elle est gentille"}, {"Ale", "mais"}, {"veľká a silná", "grande et forte"} };
static const Gloss ep5_dialog_g2[1] = { {"Bojím sa", "j'ai peur"} };
static const Gloss ep5_dialog_g3[4] = { {"Nemusíš", "tu n'es pas obligé"}, {"Teta", "ma tante"}, {"o tebe", "de toi"}, {"nevie", "ne sait pas"} };
static const Gloss ep5_dialog_g4[1] = { {"Nevie", "elle ne sait pas"} };
static const Gloss ep5_dialog_g5[3] = { {"Budeš spať", "tu vas dormir"}, {"v záhrade", "dans le jardin"}, {"Pod jabloňou", "sous le pommier"} };
static const Gloss ep5_dialog_g6[3] = { {"Ďakujem", "merci"}, {"Som rád", "je suis content"}, {"že ťa vidím", "de te voir"} };
static const char* const ep5_dialog_c6_items[3] = { "Ďakujem, Andrea.", "Som rád,", "že ťa vidím." };
static const Choice ep5_dialog_c6 = { CH_ORDER, "order-akujem-andrea-som-r-d-e-a-vid-m", ep5_dialog_c6_items, 3 };
static const Gloss ep5_dialog_g7[3] = { {"Aj ja", "moi aussi"}, {"Teraz si tu", "maintenant tu es là"}, {"To je skvelé", "c'est super"} };
static const Gloss ep5_dialog_g8[4] = { {"Ale", "mais"}, {"ja som tu", "moi, je suis ici"}, {"len", "seulement"}, {"na prázdninách", "en vacances"} };
static const Gloss ep5_dialog_g9[2] = { {"Uvidíme", "on verra"}, {"Dobrú noc", "bonne nuit"} };
static const Gloss ep5_dialog_g10[1] = { {"Dobrú noc", "bonne nuit"} };
static const Gloss ep5_dialog_g11[5] = { {"leží", "est allongé"}, {"pod jabloňou", "sous le pommier"}, {"pozerá na hviezdy", "regarde les étoiles"}, {"Za chvíľu", "dans un moment"}, {"zaspí", "il s'endort"} };
static const Line ep5_dialog[12] = {
  {SPK_NARRATOR, "Je večer. Pred domom je veľká záhrada.", "C'est le soir. Devant la maison, il y a un grand jardin.", ep5_dialog_g0, 3, "Mots nouveaux signalés : večer = le soir, záhrada = le jardin.", nullptr},
  {SPK_ANDREA, "Tu býva moja teta. Je milá. Ale veľká a silná.", "C'est là qu'habite ma tante. Elle est gentille. Mais grande et forte.", ep5_dialog_g1, 5, "milý pour un garçon, milá pour une fille. Mot nouveau signalé : silná = forte.", nullptr},
  {SPK_FOX, "Bojím sa.", "J'ai peur.", ep5_dialog_g2, 1, "Mot nouveau signalé : bojím sa = j'ai peur.", nullptr},
  {SPK_ANDREA, "Nemusíš. Teta o tebe nevie.", "Tu n'as pas à avoir peur. Ma tante ne sait pas que tu es là.", ep5_dialog_g3, 4, "Mot à mot : « Tu n'es pas obligé. Ma tante ne sait rien de toi. » vie = elle sait, nevie = elle ne sait pas.", nullptr},
  {SPK_FOX, "Nevie?", "Elle ne sait pas ?", ep5_dialog_g4, 1, nullptr, nullptr},
  {SPK_ANDREA, "Budeš spať v záhrade. Pod jabloňou.", "Tu vas dormir dans le jardin. Sous le pommier.", ep5_dialog_g5, 3, "Mots nouveaux signalés : spať = dormir, jabloň = le pommier. v záhrade : après v, le mot change à la fin, comme v električke.", nullptr},
  {SPK_FOX, "Ďakujem, Andrea. Som rád, že ťa vidím.", "Merci, Andrea. Je suis content de te voir.", ep5_dialog_g6, 3, nullptr, &ep5_dialog_c6},
  {SPK_ANDREA, "Aj ja. Teraz si tu. To je skvelé.", "Moi aussi. Maintenant tu es là. C'est super.", ep5_dialog_g7, 3, nullptr, nullptr},
  {SPK_FOX, "Ale ja som tu len na prázdninách...", "Mais moi, je ne suis ici qu'en vacances...", ep5_dialog_g8, 4, nullptr, nullptr},
  {SPK_ANDREA, "Uvidíme. Dobrú noc, \001.", "On verra. Bonne nuit, \001.", ep5_dialog_g9, 2, "Mots nouveaux signalés : uvidíme = on verra, Dobrú noc = bonne nuit.", nullptr},
  {SPK_FOX, "Dobrú noc, Andrea.", "Bonne nuit, Andrea.", ep5_dialog_g10, 1, nullptr, nullptr},
  {SPK_NARRATOR, "\001 leží pod jabloňou a pozerá na hviezdy. Za chvíľu zaspí.", "\001 est allongé sous le pommier et regarde les étoiles. Il va bientôt s'endormir.", ep5_dialog_g11, 5, "Pour comprendre : hviezdy = les étoiles, zaspí = il va s'endormir.", nullptr},
};
static const char* const ep5_x0_cfr[3] = { "C'est là qu'habite ma tante. Elle est petite. Mais gentille et forte.", "C'est là qu'habite ma maman. Elle est gentille. Mais grande et forte.", "C'est là que j'habite. Je suis gentille. Mais grande et forte." };
static const char* const ep5_x1_cfr[3] = { "J'ai froid.", "J'arrive.", "Je dors." };
static const char* const ep5_x2_cfr[3] = { "Tu vas dormir dans le tram. Sous le pommier.", "Tu vas dormir dans la maison. Avec ma tante.", "Tu vas manger dans le jardin. Sous le pommier." };
static const char* const ep5_x3_cfr[3] = { "Moi aussi. Maintenant je suis là. C'est super.", "Toi aussi. Maintenant tu es là. C'est bizarre.", "Moi non. Maintenant tu es là. C'est super." };
static const char* const ep5_x4_cfr[3] = { "Mais moi, j'habite ici...", "Mais moi, je suis ici avec maman...", "Mais toi, tu n'es ici qu'en vacances..." };
static const char* const ep5_x5_cfr[3] = { "C'est le matin. Devant la maison, il y a un grand jardin.", "C'est le soir. Devant la maison, il y a une grande cathédrale.", "C'est le soir. Dans la maison, il y a une grande tante." };
static const char* const ep5_x6_tsk[3] = { "Ďakujem, Andrea.", "Som rád,", "že ťa vidím." };
static const char* const ep5_x6_tfr[3] = { "Merci, Andrea.", "Je suis content", "de te voir." };
static const char* const ep5_x7_tsk[3] = { "Aj ja.", "Teraz si tu.", "To je skvelé." };
static const char* const ep5_x7_tfr[3] = { "Moi aussi.", "Maintenant tu es là.", "C'est super." };
static const char* const ep5_x8_tsk[3] = { "Nemusíš.", "Teta", "o tebe nevie." };
static const char* const ep5_x8_tfr[3] = { "Tu n'as pas à avoir peur.", "Ma tante ne sait pas", "que tu es là." };
static const Exercise ep5_exercises[9] = {
  {EX_LISTEN, "listen-tu-b-va-moja-teta-je-mil-ale-ve-k-a-siln", "Tu býva moja teta. Je milá. Ale veľká a silná.", "C'est là qu'habite ma tante. Elle est gentille. Mais grande et forte.", nullptr, ep5_x0_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-boj-m-sa", "Bojím sa.", "J'ai peur.", nullptr, ep5_x1_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-bude-spa-v-z-hrade-pod-jablo-ou", "Budeš spať v záhrade. Pod jabloňou.", "Tu vas dormir dans le jardin. Sous le pommier.", nullptr, ep5_x2_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-aj-ja-teraz-si-tu-to-je-skvel", "Aj ja. Teraz si tu. To je skvelé.", "Moi aussi. Maintenant tu es là. C'est super.", nullptr, ep5_x3_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-ale-ja-som-tu-len-na-pr-zdnin-ch", "Ale ja som tu len na prázdninách...", "Mais moi, je ne suis ici qu'en vacances...", nullptr, ep5_x4_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-je-ve-er-pred-domom-je-ve-k-z-hrada", "Je večer. Pred domom je veľká záhrada.", "C'est le soir. Devant la maison, il y a un grand jardin.", nullptr, ep5_x5_cfr, 3, nullptr, nullptr, 0},
  {EX_ORDER, "order-akujem-andrea-som-r-d-e-a-vid-m", "Ďakujem, Andrea. Som rád, že ťa vidím.", "Merci, Andrea. Je suis content de te voir.", nullptr, nullptr, 0, ep5_x6_tsk, ep5_x6_tfr, 3},
  {EX_ORDER, "order-aj-ja-teraz-si-tu-to-je-skvel", "Aj ja. Teraz si tu. To je skvelé.", "Moi aussi. Maintenant tu es là. C'est super.", nullptr, nullptr, 0, ep5_x7_tsk, ep5_x7_tfr, 3},
  {EX_ORDER, "order-nemus-teta-o-tebe-nevie", "Nemusíš. Teta o tebe nevie.", "Tu n'as pas à avoir peur. Ma tante ne sait pas que tu es là.", nullptr, nullptr, 0, ep5_x8_tsk, ep5_x8_tfr, 3},
};
static const Line ep5_bonus[1] = {
  {SPK_ANDREA, "Spi dobre, líška.", "Dors bien, renard.", nullptr, 0, nullptr, nullptr},
};
static const Episode ep5 = { "10_Series_01_Kosice_06_extra", "Košice", 1, 6, "Dobrú noc", "Košice",
  ep5_dialog, 12, nullptr, 0, nullptr, 0, ep5_exercises, 9, ep5_bonus, 1 };

static const Gloss ep6_dialog_g0[4] = { {"Ráno", "le matin"}, {"Slnko", "le soleil"}, {"je už vysoko", "est déjà haut"}, {"spí pod jabloňou", "dort sous le pommier"} };
static const Gloss ep6_dialog_g1[1] = { {"Vstávaj", "lève-toi"} };
static const Gloss ep6_dialog_g2[3] = { {"Čo", "quoi"}, {"Kde som", "où je suis"}, {"V záhrade", "dans le jardin"} };
static const char* const ep6_dialog_c2_items[4] = { "Čo...", "Kde som?", "Aha.", "V záhrade." };
static const Choice ep6_dialog_c2 = { CH_ORDER, "order-o-kde-som-aha-v-z-hrade", ep6_dialog_c2_items, 4 };
static const Gloss ep6_dialog_g3[3] = { {"Tu máš vodu", "tiens, de l'eau"}, {"Len trochu", "juste un peu"}, {"Viac nemám", "je n'en ai pas plus"} };
static const Gloss ep6_dialog_g4[4] = { {"Ďakujem", "merci"}, {"Aké je", "comment est"}, {"dnes", "aujourd'hui"}, {"počasie", "le temps (la météo)"} };
static const char* const ep6_dialog_c4_items[3] = { "Ďakujem. Aké je dnes počasie?", "Ďakujem. Aké je každé ráno počasie?", "Ďakujem. Aké je dva mesiace počasie?" };
static const Choice ep6_dialog_c4 = { CH_PICK, "qcm-dnes", ep6_dialog_c4_items, 3 };
static const Gloss ep6_dialog_g5[4] = { {"Horúco", "chaud"}, {"A sucho", "et sec"}, {"Už dva mesiace", "depuis deux mois déjà"}, {"nepršalo", "il n'a pas plu"} };
static const Gloss ep6_dialog_g6[3] = { {"Dva mesiace", "deux mois"}, {"A kde", "et où"}, {"pijete vodu", "vous buvez de l'eau"} };
static const Gloss ep6_dialog_g7[4] = { {"V parku", "dans le parc"}, {"je jazierko", "il y a un petit étang"}, {"Chodím tam", "j'y vais"}, {"každé ráno", "tous les matins"} };
static const Gloss ep6_dialog_g8[2] = { {"Tak", "alors"}, {"poďme", "allons-y"} };
static const Line ep6_dialog[9] = {
  {SPK_NARRATOR, "Ráno. Slnko je už vysoko. \001 spí pod jabloňou.", "Le matin. Le soleil est déjà haut. \001 dort sous le pommier.", ep6_dialog_g0, 4, "Pour comprendre : vysoko = haut. spí = il dort, de spať.", nullptr},
  {SPK_ANDREA, "\001! Vstávaj!", "\001 ! Debout !", ep6_dialog_g1, 1, nullptr, nullptr},
  {SPK_FOX, "Čo... Kde som? Aha. V záhrade.", "Quoi... Où je suis ? Ah. Dans le jardin.", ep6_dialog_g2, 3, nullptr, &ep6_dialog_c2},
  {SPK_ANDREA, "Tu máš vodu. Len trochu. Viac nemám.", "Tiens, de l'eau. Juste un peu. Je n'en ai pas plus.", ep6_dialog_g3, 3, "Tu máš = « voilà pour toi », quand on tend quelque chose. Pour comprendre : trochu = un peu, viac = plus. Après máš, voda devient vodu.", nullptr},
  {SPK_FOX, "Ďakujem. Aké je dnes počasie?", "Merci. Il fait quel temps aujourd'hui ?", ep6_dialog_g4, 4, nullptr, &ep6_dialog_c4},
  {SPK_ANDREA, "Horúco. A sucho. Už dva mesiace nepršalo.", "Chaud. Et sec. Il n'a pas plu depuis deux mois.", ep6_dialog_g5, 4, "À écouter : nepršalo = il n'a pas plu. prší = il pleut.", nullptr},
  {SPK_FOX, "Dva mesiace? A kde pijete vodu?", "Deux mois ? Et où est-ce que vous buvez ?", ep6_dialog_g6, 3, "pijete = vous buvez : Andrea et les autres animaux de la ville. Pour « vous », le verbe finit en -te.", nullptr},
  {SPK_ANDREA, "V parku je jazierko. Chodím tam každé ráno.", "Il y a un petit étang dans le parc. J'y vais tous les matins.", ep6_dialog_g7, 4, nullptr, nullptr},
  {SPK_FOX, "Tak poďme.", "Alors allons-y.", ep6_dialog_g8, 2, "Tu connais poď, « viens ». En ajoutant -me, tu obtiens poďme, « allons-y ».", nullptr},
};
static const Gloss ep6_extra_g0[3] = { {"Ráno je zima", "le matin, il fait froid"}, {"ale", "mais"}, {"dnes je teplo", "aujourd'hui, il fait doux"} };
static const Gloss ep6_extra_g1[3] = { {"Tam je", "là-bas, il y a"}, {"oblak", "un nuage"}, {"Dážď", "de la pluie"} };
static const Line ep6_extra[2] = {
  {SPK_NONE, "Ráno je zima, ale dnes je teplo.", "Le matin, il fait froid, mais aujourd'hui, il fait doux.", ep6_extra_g0, 3, nullptr, nullptr},
  {SPK_NONE, "Tam je oblak. Dážď?", "Là-bas, il y a un nuage. De la pluie ?", ep6_extra_g1, 3, "dážď = la pluie. Il se fait rare, ces temps-ci.", nullptr},
};
static const Word ep6_words[10] = {
  {"Aké je počasie?", "Quel temps fait-il ?"},
  {"dnes", "aujourd'hui"},
  {"horúco", "chaud"},
  {"sucho", "sec"},
  {"slnko", "le soleil"},
  {"vstávaj", "lève-toi, debout"},
  {"jazierko", "le petit étang"},
  {"v parku", "dans le parc"},
  {"dva mesiace", "deux mois"},
  {"každé ráno", "tous les matins"},
};
static const char* const ep6_x0_csk[3] = { "sucho", "slnko", "dnes" };
static const char* const ep6_x0_cfr[3] = { "sec", "le soleil", "aujourd'hui" };
static const char* const ep6_x1_csk[3] = { "každé ráno", "dva mesiace", "horúco" };
static const char* const ep6_x1_cfr[3] = { "tous les matins", "deux mois", "chaud" };
static const char* const ep6_x2_csk[3] = { "počkaj", "sucho", "slnko" };
static const char* const ep6_x2_cfr[3] = { "attends", "sec", "le soleil" };
static const char* const ep6_x3_csk[3] = { "slnko", "v parku", "zima" };
static const char* const ep6_x3_cfr[3] = { "le soleil", "dans le parc", "froid" };
static const char* const ep6_x4_csk[3] = { "oblak", "jazierko", "sucho" };
static const char* const ep6_x4_cfr[3] = { "le nuage", "le petit étang", "sec" };
static const char* const ep6_x5_cfr[3] = { "Chaud. Et sec. Il n'a pas plu depuis deux jours.", "Froid. Et sec. Il n'a pas plu depuis deux mois.", "Chaud. Et sec. Il a plu pendant deux mois." };
static const char* const ep6_x6_cfr[3] = { "Merci. Il fait quel temps ce soir ?", "Merci. Où est l'eau aujourd'hui ?", "Merci. Il fait chaud aujourd'hui ?" };
static const char* const ep6_x7_cfr[3] = { "Il y a un petit étang dans le jardin. J'y vais tous les matins.", "Il y a un petit étang dans le parc. J'y vais tous les soirs.", "Il y a un grand lac dans le parc. J'y vais tous les matins." };
static const char* const ep6_x8_cfr[3] = { "Alors viens.", "Alors on verra.", "Alors attends." };
static const char* const ep6_x9_tsk[3] = { "Tu máš vodu.", "Len trochu.", "Viac nemám." };
static const char* const ep6_x9_tfr[3] = { "Tiens, de l'eau.", "Juste un peu.", "Je n'en ai pas plus." };
static const char* const ep6_x10_tsk[4] = { "Čo...", "Kde som?", "Aha.", "V záhrade." };
static const char* const ep6_x10_tfr[4] = { "Quoi...", "Où je suis ?", "Ah.", "Dans le jardin." };
static const Exercise ep6_exercises[11] = {
  {EX_QCM, "qcm-hor-co", "horúco", "chaud", ep6_x0_csk, ep6_x0_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-dnes", "dnes", "aujourd'hui", ep6_x1_csk, ep6_x1_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-vst-vaj", "vstávaj", "lève-toi, debout", ep6_x2_csk, ep6_x2_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-jazierko", "jazierko", "le petit étang", ep6_x3_csk, ep6_x3_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-slnko", "slnko", "le soleil", ep6_x4_csk, ep6_x4_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-hor-co-a-sucho-u-dva-mesiace-nepr-alo", "Horúco. A sucho. Už dva mesiace nepršalo.", "Chaud. Et sec. Il n'a pas plu depuis deux mois.", nullptr, ep6_x5_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-akujem-ak-je-dnes-po-asie", "Ďakujem. Aké je dnes počasie?", "Merci. Il fait quel temps aujourd'hui ?", nullptr, ep6_x6_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-v-parku-je-jazierko-chod-m-tam-ka-d-r-no", "V parku je jazierko. Chodím tam každé ráno.", "Il y a un petit étang dans le parc. J'y vais tous les matins.", nullptr, ep6_x7_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-tak-po-me", "Tak poďme.", "Alors allons-y.", nullptr, ep6_x8_cfr, 3, nullptr, nullptr, 0},
  {EX_ORDER, "order-tu-m-vodu-len-trochu-viac-nem-m", "Tu máš vodu. Len trochu. Viac nemám.", "Tiens, de l'eau. Juste un peu. Je n'en ai pas plus.", nullptr, nullptr, 0, ep6_x9_tsk, ep6_x9_tfr, 3},
  {EX_ORDER, "order-o-kde-som-aha-v-z-hrade", "Čo... Kde som? Aha. V záhrade.", "Quoi... Où je suis ? Ah. Dans le jardin.", nullptr, nullptr, 0, ep6_x10_tsk, ep6_x10_tfr, 4},
};
static const Line ep6_bonus[2] = {
  {SPK_ANDREA, "Dobré ráno, ospalec!", "Bonjour, la marmotte !", nullptr, 0, nullptr, nullptr},
  {SPK_ANDREA, "Aj ráno hovoríš po slovensky!", "Même le matin, tu parles slovaque !", nullptr, 0, nullptr, nullptr},
};
static const Episode ep6 = { "10_Series_02_Sucho_01_je-horuco", "Sucho", 2, 1, "Je horúco", "Mestský park",
  ep6_dialog, 9, ep6_extra, 2, ep6_words, 10, ep6_exercises, 11, ep6_bonus, 2 };

static const Gloss ep7_dialog_g0[5] = { {"Idú do parku", "ils vont au parc"}, {"rozpráva o všetkom", "parle de tout"}, {"o jabloni", "du pommier"}, {"o tete", "de la tante"}, {"o počasí", "du temps qu'il fait"} };
static const Gloss ep7_dialog_g1[3] = { {"Ty", "toi"}, {"veľa", "beaucoup"}, {"hovoríš", "tu parles"} };
static const Gloss ep7_dialog_g2[3] = { {"Veľmi hovorím", "je parle très"}, {"som líškasky", "je suis « renardesque »"}, {"komplikovaný", "compliqué"} };
static const Gloss ep7_dialog_g3[5] = { {"Rozumiem", "je comprends"}, {"čo chceš povedať", "ce que tu veux dire"}, {"Ale", "mais"}, {"správne je", "le correct, c'est"}, {"veľa hovorím", "je parle beaucoup"} };
static const Gloss ep7_dialog_g4[1] = { {"Veľa hovorím", "je parle beaucoup"} };
static const Gloss ep7_dialog_g5[5] = { {"Veľmi je pre aký", "« veľmi » sert pour « comment »"}, {"veľmi milý", "très gentil"}, {"veľmi ďaleko", "très loin"}, {"Veľa je pre koľko", "« veľa » sert pour « combien »"}, {"veľa vody", "beaucoup d'eau"} };
static const Gloss ep7_dialog_g6[3] = { {"A", "et"}, {"líškasky neexistuje", "« líškasky » n'existe pas"}, {"Si líška", "tu es un renard"} };
static const Gloss ep7_dialog_g7[2] = { {"Aha", "ah"}, {"Som líška", "je suis un renard"} };
static const char* const ep7_dialog_c7_items[3] = { "Aha.", "Som", "líška." };
static const Choice ep7_dialog_c7 = { CH_ORDER, "order-aha-som-l-ka", ep7_dialog_c7_items, 3 };
static const Gloss ep7_dialog_g8[4] = { {"Správne", "exact"}, {"líščí je to", "« líščí », c'est ce"}, {"čo patrí líške", "qui appartient au renard"}, {"líščí chvost", "une queue de renard"} };
static const Gloss ep7_dialog_g9[4] = { {"Takže", "donc"}, {"môj chvost", "ma queue"}, {"je líščí", "est de renard"}, {"A ja som líška", "et moi, je suis un renard"} };
static const char* const ep7_dialog_c9_items[3] = { "Takže môj chvost je líščí. A ja som líška.", "Takže môj hovorím je líščí. A ja som líška.", "Takže môj presne je líščí. A ja som líška." };
static const Choice ep7_dialog_c9 = { CH_PICK, "qcm-chvost", ep7_dialog_c9_items, 3 };
static const Gloss ep7_dialog_g10[1] = { {"Presne", "exactement"} };
static const Gloss ep7_dialog_g11[3] = { {"Teraz", "maintenant"}, {"to chápem", "je comprends"}, {"Skoro", "presque"} };
static const Line ep7_dialog[12] = {
  {SPK_NARRATOR, "Idú do parku. \001 rozpráva o všetkom: o jabloni, o tete, o Lyone, o električke, o počasí...", "Ils vont au parc. \001 parle de tout : du pommier, de la tante, de Lyon, du tram, du temps qu'il fait...", ep7_dialog_g0, 5, nullptr, nullptr},
  {SPK_ANDREA, "Ty veľa hovoríš.", "Tu parles beaucoup.", ep7_dialog_g1, 3, "hovorím (je parle) → hovoríš (tu parles), comme robím → robíš.", nullptr},
  {SPK_FOX, "Veľmi hovorím, som líškasky, komplikovaný...", "Je parle très, je suis renardesque, compliqué...", ep7_dialog_g2, 3, "Cette phrase est fausse, exprès ! Andrea va expliquer pourquoi. Ne la retiens pas telle quelle.", nullptr},
  {SPK_ANDREA, "Rozumiem, čo chceš povedať. Ale správne je: veľa hovorím.", "Je comprends ce que tu veux dire. Mais ce qu'il faut dire, c'est : « je parle beaucoup ».", ep7_dialog_g3, 5, nullptr, nullptr},
  {SPK_FOX, "Veľa hovorím?", "« Je parle beaucoup » ?", ep7_dialog_g4, 1, nullptr, nullptr},
  {SPK_ANDREA, "„Veľmi“ je pre „aký“: veľmi milý, veľmi ďaleko. „Veľa“ je pre „koľko“: veľa vody, veľa hovorím.", "« Veľmi », c'est pour « comment ? » : très gentil, très loin. « Veľa », c'est pour « combien ? » : beaucoup d'eau, je parle beaucoup.", ep7_dialog_g5, 5, "aký = quel, comment ; koľko = combien. Voilà toute la règle en une phrase.", nullptr},
  {SPK_ANDREA, "A „líškasky“ neexistuje. Si líška.", "Et « líškasky » n'existe pas. Tu es un renard.", ep7_dialog_g6, 3, "La terminaison -sky sert pour les langues, comme dans po slovensky. Pour dire ce qu'on est, il faut un nom : si líška.", nullptr},
  {SPK_FOX, "Aha. Som líška.", "Ah. Je suis un renard.", ep7_dialog_g7, 2, nullptr, &ep7_dialog_c7},
  {SPK_ANDREA, "Správne. A „líščí“ je to, čo patrí líške: líščí chvost.", "Exact. Et « líščí », c'est ce qui appartient au renard : la queue de renard.", ep7_dialog_g8, 4, nullptr, nullptr},
  {SPK_FOX, "Takže môj chvost je líščí. A ja som líška.", "Donc ma queue est « de renard ». Et moi, je suis un renard.", ep7_dialog_g9, 4, nullptr, &ep7_dialog_c9},
  {SPK_ANDREA, "Presne.", "Voilà.", ep7_dialog_g10, 1, nullptr, nullptr},
  {SPK_FOX, "Teraz to chápem. Skoro.", "Maintenant, je comprends. Presque.", ep7_dialog_g11, 3, "chápem = je comprends, je pige. C'est un cousin de rozumiem.", nullptr},
};
static const Gloss ep7_extra_g0[3] = { {"Mám", "j'ai"}, {"trochu", "un peu"}, {"vody", "d'eau"} };
static const Gloss ep7_extra_g1[3] = { {"Chcem", "je veux"}, {"viac vody", "plus d'eau"}, {"nie menej", "pas moins"} };
static const Line ep7_extra[2] = {
  {SPK_NONE, "Mám trochu vody.", "J'ai un peu d'eau.", ep7_extra_g0, 3, nullptr, nullptr},
  {SPK_NONE, "Chcem viac vody, nie menej!", "Je veux plus d'eau, pas moins !", ep7_extra_g1, 3, nullptr, nullptr},
};
static const Word ep7_words[10] = {
  {"veľa", "beaucoup (rappel)"},
  {"veľmi", "très (rappel)"},
  {"málo", "peu"},
  {"hovorím", "je parle"},
  {"rozumiem", "je comprends"},
  {"správne", "correct, juste"},
  {"neexistuje", "n'existe pas"},
  {"líščí", "de renard"},
  {"chvost", "la queue"},
  {"presne", "exactement"},
};
static const char* const ep7_x0_csk[3] = { "trochu", "veľmi", "presne" };
static const char* const ep7_x0_cfr[3] = { "un peu", "très", "exactement" };
static const char* const ep7_x1_csk[3] = { "líščí", "hovorím", "presne" };
static const char* const ep7_x1_cfr[3] = { "de renard", "je parle", "exactement" };
static const char* const ep7_x2_csk[3] = { "hovorím", "neexistuje", "správne" };
static const char* const ep7_x2_cfr[3] = { "je parle", "n'existe pas", "correct, juste" };
static const char* const ep7_x3_csk[3] = { "správne", "málo", "neexistuje" };
static const char* const ep7_x3_cfr[3] = { "correct, juste", "peu", "n'existe pas" };
static const char* const ep7_x4_csk[3] = { "chvost", "správne", "málo" };
static const char* const ep7_x4_cfr[3] = { "la queue", "correct, juste", "peu" };
static const char* const ep7_x5_cfr[3] = { "Tu parles très bien.", "Tu parles peu.", "Je parle beaucoup." };
static const char* const ep7_x6_cfr[3] = { "Donc ma queue est rousse. Et moi, je suis un renard.", "Donc ta queue est « de renard ». Et toi, tu es un renard.", "Donc ma queue est « de renard ». Et moi, je suis compliqué." };
static const char* const ep7_x7_cfr[3] = { "Maintenant, je comprends. Exactement.", "Maintenant, je ne comprends pas. Presque.", "Maintenant, je me souviens. Presque." };
static const char* const ep7_x8_cfr[3] = { "J'ai beaucoup d'eau.", "J'ai peu d'eau.", "Je cherche un peu d'eau." };
static const char* const ep7_x9_tsk[3] = { "Aha.", "Som", "líška." };
static const char* const ep7_x9_tfr[3] = { "Ah.", "Je suis", "un renard." };
static const char* const ep7_x10_tsk[4] = { "Rozumiem,", "čo chceš povedať.", "Ale správne je:", "veľa hovorím." };
static const char* const ep7_x10_tfr[4] = { "Je comprends", "ce que tu veux dire.", "Mais ce qu'il faut dire, c'est :", "« je parle beaucoup »." };
static const Exercise ep7_exercises[11] = {
  {EX_QCM, "qcm-m-lo", "málo", "peu", ep7_x0_csk, ep7_x0_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-chvost", "chvost", "la queue", ep7_x1_csk, ep7_x1_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-rozumiem", "rozumiem", "je comprends", ep7_x2_csk, ep7_x2_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-presne", "presne", "exactement", ep7_x3_csk, ep7_x3_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-l", "líščí", "de renard", ep7_x4_csk, ep7_x4_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-ty-ve-a-hovor", "Ty veľa hovoríš.", "Tu parles beaucoup.", nullptr, ep7_x5_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-tak-e-m-j-chvost-je-l-a-ja-som-l-ka", "Takže môj chvost je líščí. A ja som líška.", "Donc ma queue est « de renard ». Et moi, je suis un renard.", nullptr, ep7_x6_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-teraz-to-ch-pem-skoro", "Teraz to chápem. Skoro.", "Maintenant, je comprends. Presque.", nullptr, ep7_x7_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-m-m-trochu-vody", "Mám trochu vody.", "J'ai un peu d'eau.", nullptr, ep7_x8_cfr, 3, nullptr, nullptr, 0},
  {EX_ORDER, "order-aha-som-l-ka", "Aha. Som líška.", "Ah. Je suis un renard.", nullptr, nullptr, 0, ep7_x9_tsk, ep7_x9_tfr, 3},
  {EX_ORDER, "order-rozumiem-o-chce-poveda-ale-spr-vne-je-ve-a-hovor-m", "Rozumiem, čo chceš povedať. Ale správne je: veľa hovorím.", "Je comprends ce que tu veux dire. Mais ce qu'il faut dire, c'est : « je parle beaucoup ».", nullptr, nullptr, 0, ep7_x10_tsk, ep7_x10_tfr, 4},
};
static const Line ep7_bonus[1] = {
  {SPK_ANDREA, "Vidíš? Už hovoríš správne.", "Tu vois ? Tu parles déjà correctement.", nullptr, 0, nullptr, nullptr},
};
static const Episode ep7 = { "10_Series_02_Sucho_02_vela-alebo-velmi", "Sucho", 2, 2, "Veľa alebo veľmi?", "Sucho",
  ep7_dialog, 12, ep7_extra, 2, ep7_words, 10, ep7_exercises, 11, ep7_bonus, 1 };

static const Gloss ep8_dialog_g0[5] = { {"V parku", "dans le parc"}, {"je jazierko", "il y a un petit étang"}, {"Ale", "mais"}, {"vody je v ňom", "il y a de l'eau dedans"}, {"veľmi málo", "très peu"} };
static const Gloss ep8_dialog_g1[5] = { {"Včera", "hier"}, {"tu", "ici"}, {"ešte", "encore"}, {"bola voda", "il y avait de l'eau"}, {"Aspoň trochu", "au moins un peu"} };
static const Gloss ep8_dialog_g2[2] = { {"Chýba ti", "te manque"}, {"voda", "l'eau"} };
static const char* const ep8_dialog_c2_items[3] = { "Chýba", "ti", "voda?" };
static const Choice ep8_dialog_c2 = { CH_ORDER, "order-ch-ba-ti-voda", ep8_dialog_c2_items, 3 };
static const Gloss ep8_dialog_g3[2] = { {"Veľmi", "beaucoup, très fort"}, {"mi chýba", "me manque"} };
static const Gloss ep8_dialog_g4[4] = { {"Pri jazierku", "près de l'étang"}, {"stoja", "se tiennent debout"}, {"ďalšie zvieratá", "d'autres animaux"}, {"Nikto nehovorí", "personne ne parle"} };
static const Gloss ep8_dialog_g5[2] = { {"To je", "c'est"}, {"zlé", "mauvais"} };
static const char* const ep8_dialog_c5_items[3] = { "To je zlé.", "To je včera.", "To je ešte." };
static const Choice ep8_dialog_c5 = { CH_PICK, "qcm-zl", ep8_dialog_c5_items, 3 };
static const Gloss ep8_dialog_g6[2] = { {"To je", "c'est"}, {"veľmi zlé", "très mauvais"} };
static const Line ep8_dialog[7] = {
  {SPK_NARRATOR, "V parku je jazierko. Ale vody je v ňom veľmi málo.", "Dans le parc, il y a un petit étang. Mais il y a très peu d'eau dedans.", ep8_dialog_g0, 5, "veľmi málo : veľmi renforce málo, comme dans veľmi veľa.", nullptr},
  {SPK_ANDREA, "Včera tu ešte bola voda. Aspoň trochu.", "Hier, il y avait encore de l'eau. Un peu, au moins.", ep8_dialog_g1, 5, "À écouter : bola = il y avait. C'est le je d'avant.", nullptr},
  {SPK_FOX, "Chýba ti voda?", "L'eau te manque ?", ep8_dialog_g2, 2, nullptr, &ep8_dialog_c2},
  {SPK_ANDREA, "Veľmi mi chýba.", "Elle me manque beaucoup.", ep8_dialog_g3, 2, "Ici, on utilise veľmi : c'est la force du manque, pas une quantité d'eau. Tu vois la différence ?", nullptr},
  {SPK_NARRATOR, "Pri jazierku stoja ďalšie zvieratá. Nikto nehovorí.", "Près de l'étang, d'autres animaux sont là. Personne ne parle.", ep8_dialog_g4, 4, "Pour dire « personne ne… », le slovaque met deux négations : nikto et ne-. Ce n'est pas une faute, c'est la règle !", nullptr},
  {SPK_FOX, "To je zlé.", "C'est mauvais.", ep8_dialog_g5, 2, nullptr, &ep8_dialog_c5},
  {SPK_ANDREA, "To je veľmi zlé.", "C'est très mauvais.", ep8_dialog_g6, 2, nullptr, nullptr},
};
static const Gloss ep8_extra_g0[3] = { {"Hľadám potok", "je cherche un ruisseau"}, {"Tu je", "voilà"}, {"kvapka vody", "une goutte d'eau"} };
static const Gloss ep8_extra_g1[2] = { {"Jazero je veľké", "un lac, c'est grand"}, {"jazierko je malé", "un petit étang, c'est petit"} };
static const Line ep8_extra[2] = {
  {SPK_NONE, "Hľadám potok. Tu je kvapka vody!", "Je cherche un ruisseau. Voilà une goutte d'eau !", ep8_extra_g0, 3, nullptr, nullptr},
  {SPK_NONE, "Jazero je veľké, jazierko je malé.", "Un lac, c'est grand, un petit étang, c'est petit.", ep8_extra_g1, 2, nullptr, nullptr},
};
static const Word ep8_words[10] = {
  {"chýba mi", "il me manque"},
  {"chýba ti", "il te manque"},
  {"pijem", "je bois (rappel)"},
  {"hľadám", "je cherche"},
  {"včera", "hier"},
  {"ešte", "encore"},
  {"aspoň", "au moins"},
  {"zvieratá", "les animaux"},
  {"nikto", "personne"},
  {"zlé", "mauvais"},
};
static const char* const ep8_x0_csk[3] = { "chýba ti", "hľadám", "aspoň" };
static const char* const ep8_x0_cfr[3] = { "il te manque", "je cherche", "au moins" };
static const char* const ep8_x1_csk[3] = { "ešte", "aspoň", "nikto" };
static const char* const ep8_x1_cfr[3] = { "encore", "au moins", "personne" };
static const char* const ep8_x2_csk[3] = { "zvieratá", "aspoň", "ešte" };
static const char* const ep8_x2_cfr[3] = { "les animaux", "au moins", "encore" };
static const char* const ep8_x3_csk[3] = { "včera", "ešte", "nikto" };
static const char* const ep8_x3_cfr[3] = { "hier", "encore", "personne" };
static const char* const ep8_x4_csk[3] = { "chýba mi", "chýba ti", "včera" };
static const char* const ep8_x4_cfr[3] = { "il me manque", "il te manque", "hier" };
static const char* const ep8_x5_cfr[3] = { "Aujourd'hui, il y a encore de l'eau. Un peu, au moins.", "Hier, il n'y avait plus d'eau. Pas du tout.", "Hier, il y avait beaucoup d'eau. Beaucoup trop." };
static const char* const ep8_x6_cfr[3] = { "L'eau me manque.", "Tu as de l'eau ?", "Tu bois de l'eau ?" };
static const char* const ep8_x7_cfr[3] = { "C'est très bien.", "C'est mauvais.", "C'est très loin." };
static const char* const ep8_x8_cfr[3] = { "Un lac, c'est petit, un petit étang, c'est grand.", "Un ruisseau, c'est grand, un petit étang, c'est petit.", "Un lac, c'est grand, une goutte, c'est petit." };
static const char* const ep8_x9_tsk[3] = { "Chýba", "ti", "voda?" };
static const char* const ep8_x9_tfr[3] = { "L'eau", "te", "manque ?" };
static const char* const ep8_x10_tsk[3] = { "Hľadám potok.", "Tu je", "kvapka vody!" };
static const char* const ep8_x10_tfr[3] = { "Je cherche un ruisseau.", "Voilà", "une goutte d'eau !" };
static const Exercise ep8_exercises[11] = {
  {EX_QCM, "qcm-ch-ba-mi", "chýba mi", "il me manque", ep8_x0_csk, ep8_x0_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-v-era", "včera", "hier", ep8_x1_csk, ep8_x1_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-nikto", "nikto", "personne", ep8_x2_csk, ep8_x2_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-zl", "zlé", "mauvais", ep8_x3_csk, ep8_x3_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-h-ad-m", "hľadám", "je cherche", ep8_x4_csk, ep8_x4_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-v-era-tu-e-te-bola-voda-aspo-trochu", "Včera tu ešte bola voda. Aspoň trochu.", "Hier, il y avait encore de l'eau. Un peu, au moins.", nullptr, ep8_x5_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-ch-ba-ti-voda", "Chýba ti voda?", "L'eau te manque ?", nullptr, ep8_x6_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-to-je-ve-mi-zl", "To je veľmi zlé.", "C'est très mauvais.", nullptr, ep8_x7_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-jazero-je-ve-k-jazierko-je-mal", "Jazero je veľké, jazierko je malé.", "Un lac, c'est grand, un petit étang, c'est petit.", nullptr, ep8_x8_cfr, 3, nullptr, nullptr, 0},
  {EX_ORDER, "order-ch-ba-ti-voda", "Chýba ti voda?", "L'eau te manque ?", nullptr, nullptr, 0, ep8_x9_tsk, ep8_x9_tfr, 3},
  {EX_ORDER, "order-h-ad-m-potok-tu-je-kvapka-vody", "Hľadám potok. Tu je kvapka vody!", "Je cherche un ruisseau. Voilà une goutte d'eau !", nullptr, nullptr, 0, ep8_x10_tsk, ep8_x10_tfr, 3},
};
static const Line ep8_bonus[1] = {
  {SPK_ANDREA, "Dobre povedané.", "Bien dit.", nullptr, 0, nullptr, nullptr},
};
static const Episode ep8 = { "10_Series_02_Sucho_03_chyba-mi-voda", "Sucho", 2, 3, "Chýba mi voda", "Sucho",
  ep8_dialog, 7, ep8_extra, 2, ep8_words, 10, ep8_exercises, 11, ep8_bonus, 1 };

static const Gloss ep9_dialog_g0[2] = { {"Čo", "quoi"}, {"budeme robiť", "nous allons faire"} };
static const char* const ep9_dialog_c0_items[3] = { "Čo budeme robiť?", "Čo ideme robiť?", "Čo spí robiť?" };
static const Choice ep9_dialog_c0 = { CH_PICK, "qcm-budeme", ep9_dialog_c0_items, 3 };
static const Gloss ep9_dialog_g1[2] = { {"Hľadať vodu", "chercher de l'eau"}, {"Inde", "ailleurs"} };
static const Gloss ep9_dialog_g2[1] = { {"Kde", "où"} };
static const Gloss ep9_dialog_g3[4] = { {"Na západe", "à l'ouest"}, {"je Slovenský raj", "il y a le Paradis slovaque"}, {"Tam sú", "là-bas, il y a"}, {"potoky", "des ruisseaux"} };
static const Gloss ep9_dialog_g4[2] = { {"Slovenský raj", "le Paradis slovaque"}, {"To je ďaleko", "c'est loin"} };
static const char* const ep9_dialog_c4_items[3] = { "Slovenský raj? To je ďaleko!", "Slovenský raj? To je inde!", "Slovenský raj? To je len!" };
static const Choice ep9_dialog_c4 = { CH_PICK, "qcm-aleko", ep9_dialog_c4_items, 3 };
static const Gloss ep9_dialog_g5[2] = { {"Veľmi", "très"}, {"ďaleko", "loin"} };
static const Gloss ep9_dialog_g6[2] = { {"A škola", "et l'école"}, {"A tvoja teta", "et ta tante"} };
static const char* const ep9_dialog_c6_items[3] = { "A škola?", "A tvoja", "teta?" };
static const Choice ep9_dialog_c6 = { CH_ORDER, "order-a-kola-a-tvoja-teta", ep9_dialog_c6_items, 3 };
static const Gloss ep9_dialog_g7[3] = { {"Sú prázdniny", "ce sont les vacances"}, {"A teta spí", "et ma tante dort"}, {"do desiatej", "jusqu'à dix heures"} };
static const Gloss ep9_dialog_g8[4] = { {"Ja som tu", "moi, je suis ici"}, {"len", "seulement"}, {"na prázdninách", "en vacances"}, {"a už ideme preč", "et on part déjà"} };
static const char* const ep9_dialog_c8_items[3] = { "Ja som tu len", "na prázdninách...", "a už ideme preč?" };
static const Choice ep9_dialog_c8 = { CH_ORDER, "order-ja-som-tu-len-na-pr-zdnin-ch-a-u-ideme-pre", ep9_dialog_c8_items, 3 };
static const Gloss ep9_dialog_g9[3] = { {"Teraz", "maintenant"}, {"si tu", "tu es là"}, {"Uvidíme", "on verra"} };
static const Line ep9_dialog[10] = {
  {SPK_FOX, "Čo budeme robiť?", "Qu'est-ce qu'on va faire ?", ep9_dialog_g0, 2, nullptr, &ep9_dialog_c0},
  {SPK_ANDREA, "Hľadať vodu. Inde.", "Chercher de l'eau. Ailleurs.", ep9_dialog_g1, 2, nullptr, nullptr},
  {SPK_FOX, "Kde?", "Où ça ?", ep9_dialog_g2, 1, nullptr, nullptr},
  {SPK_ANDREA, "Na západe je Slovenský raj. Tam sú potoky.", "À l'ouest, il y a le Paradis slovaque. Là-bas, il y a des ruisseaux.", ep9_dialog_g3, 4, "potok = un ruisseau, potoky = des ruisseaux. je pour un, sú pour plusieurs.", nullptr},
  {SPK_FOX, "Slovenský raj? To je ďaleko!", "Le Paradis slovaque ? C'est loin !", ep9_dialog_g4, 2, nullptr, &ep9_dialog_c4},
  {SPK_ANDREA, "Veľmi ďaleko.", "Très loin.", ep9_dialog_g5, 2, "Ici, c'est veľmi : très loin. Ça répond à « comment ? », pas à « combien ? ».", nullptr},
  {SPK_FOX, "A škola? A tvoja teta?", "Et l'école ? Et ta tante ?", ep9_dialog_g6, 2, nullptr, &ep9_dialog_c6},
  {SPK_ANDREA, "Sú prázdniny. A teta spí do desiatej.", "Ce sont les vacances. Et ma tante dort jusqu'à dix heures.", ep9_dialog_g7, 3, "prázdniny = les vacances ; na prázdninách = en vacances. do desiatej : retiens-le d'un bloc.", nullptr},
  {SPK_FOX, "Ja som tu len na prázdninách... a už ideme preč?", "Moi, je ne suis ici qu'en vacances... et on repart déjà ?", ep9_dialog_g8, 4, nullptr, &ep9_dialog_c8},
  {SPK_ANDREA, "Teraz si tu. Uvidíme.", "Maintenant tu es là. On verra.", ep9_dialog_g9, 3, nullptr, nullptr},
};
static const Gloss ep9_extra_g0[4] = { {"Mám", "j'ai"}, {"batoh", "un sac à dos"}, {"a", "et"}, {"mapu", "une carte"} };
static const Gloss ep9_extra_g1[2] = { {"Zajtra", "demain"}, {"ideme preč", "on part"} };
static const Line ep9_extra[2] = {
  {SPK_NONE, "Mám batoh a mapu.", "J'ai un sac à dos et une carte.", ep9_extra_g0, 4, "mapa devient mapu après mám, comme voda devient vodu.", nullptr},
  {SPK_NONE, "Zajtra ideme preč.", "Demain, on part.", ep9_extra_g1, 2, nullptr, nullptr},
};
static const Word ep9_words[10] = {
  {"budeme", "nous allons (faire)"},
  {"ideme", "nous allons (quelque part)"},
  {"uvidíme", "on verra (rappel)"},
  {"inde", "ailleurs"},
  {"na západe", "à l'ouest"},
  {"ďaleko", "loin"},
  {"škola", "l'école"},
  {"len", "seulement"},
  {"preč", "parti, loin d'ici"},
  {"spí", "il dort, elle dort"},
};
static const char* const ep9_x0_csk[3] = { "tu", "ďaleko", "len" };
static const char* const ep9_x0_cfr[3] = { "ici, là", "loin", "seulement" };
static const char* const ep9_x1_csk[3] = { "inde", "len", "preč" };
static const char* const ep9_x1_cfr[3] = { "ailleurs", "seulement", "parti, loin d'ici" };
static const char* const ep9_x2_csk[3] = { "gymnázium", "dóm", "mesto" };
static const char* const ep9_x2_cfr[3] = { "le lycée", "la cathédrale", "la ville" };
static const char* const ep9_x3_csk[3] = { "skoro", "aspoň", "ešte" };
static const char* const ep9_x3_cfr[3] = { "presque", "au moins", "encore" };
static const char* const ep9_x4_csk[3] = { "ideme", "spí", "škola" };
static const char* const ep9_x4_cfr[3] = { "nous allons (quelque part)", "il dort, elle dort", "l'école" };
static const char* const ep9_x5_cfr[3] = { "Qu'est-ce que tu fais ?", "Où est-ce qu'on va ?", "Qu'est-ce qu'on fait ici ?" };
static const char* const ep9_x6_cfr[3] = { "À l'est, il y a le Paradis slovaque. Là-bas, il y a des ruisseaux.", "À l'ouest, il y a le Paradis slovaque. Là-bas, il y a des lacs.", "À l'ouest, il y a Košice. Là-bas, il y a des ruisseaux." };
static const char* const ep9_x7_cfr[3] = { "Ce sont les vacances. Et ma tante dort jusqu'à onze heures.", "C'est l'école. Et ma tante dort jusqu'à dix heures.", "Ce sont les vacances. Et ma tante est au lycée jusqu'à dix heures." };
static const char* const ep9_x8_cfr[3] = { "Maintenant je suis là. On verra.", "Maintenant tu es là. Allons-y.", "Avant, tu étais là. On verra." };
static const char* const ep9_x9_tsk[3] = { "Ja som tu len", "na prázdninách...", "a už ideme preč?" };
static const char* const ep9_x9_tfr[3] = { "Moi, je ne suis ici", "qu'en vacances...", "et on repart déjà ?" };
static const char* const ep9_x10_tsk[3] = { "A škola?", "A tvoja", "teta?" };
static const char* const ep9_x10_tfr[3] = { "Et l'école ?", "Et ta", "tante ?" };
static const Exercise ep9_exercises[11] = {
  {EX_QCM, "qcm-inde", "inde", "ailleurs", ep9_x0_csk, ep9_x0_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-aleko", "ďaleko", "loin", ep9_x1_csk, ep9_x1_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-kola", "škola", "l'école", ep9_x2_csk, ep9_x2_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-len", "len", "seulement", ep9_x3_csk, ep9_x3_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-budeme", "budeme", "nous allons (faire)", ep9_x4_csk, ep9_x4_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-o-budeme-robi", "Čo budeme robiť?", "Qu'est-ce qu'on va faire ?", nullptr, ep9_x5_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-na-z-pade-je-slovensk-raj-tam-s-potoky", "Na západe je Slovenský raj. Tam sú potoky.", "À l'ouest, il y a le Paradis slovaque. Là-bas, il y a des ruisseaux.", nullptr, ep9_x6_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-s-pr-zdniny-a-teta-sp-do-desiatej", "Sú prázdniny. A teta spí do desiatej.", "Ce sont les vacances. Et ma tante dort jusqu'à dix heures.", nullptr, ep9_x7_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-teraz-si-tu-uvid-me", "Teraz si tu. Uvidíme.", "Maintenant tu es là. On verra.", nullptr, ep9_x8_cfr, 3, nullptr, nullptr, 0},
  {EX_ORDER, "order-ja-som-tu-len-na-pr-zdnin-ch-a-u-ideme-pre", "Ja som tu len na prázdninách... a už ideme preč?", "Moi, je ne suis ici qu'en vacances... et on repart déjà ?", nullptr, nullptr, 0, ep9_x9_tsk, ep9_x9_tfr, 3},
  {EX_ORDER, "order-a-kola-a-tvoja-teta", "A škola? A tvoja teta?", "Et l'école ? Et ta tante ?", nullptr, nullptr, 0, ep9_x10_tsk, ep9_x10_tfr, 3},
};
static const Line ep9_bonus[2] = {
  {SPK_ANDREA, "Neboj sa, mám mapu.", "Ne t'inquiète pas, j'ai une carte.", nullptr, 0, nullptr, nullptr},
  {SPK_ANDREA, "Si super spoločník.", "Tu es un super compagnon.", nullptr, 0, nullptr, nullptr},
};
static const Episode ep9 = { "10_Series_02_Sucho_04_co-budeme-robit", "Sucho", 2, 4, "Čo budeme robiť?", "Sucho",
  ep9_dialog, 10, ep9_extra, 2, ep9_words, 10, ep9_exercises, 11, ep9_bonus, 2 };

static const Gloss ep10_dialog_g0[4] = { {"V centre mesta", "au centre-ville"}, {"je fontána", "il y a une fontaine"}, {"Spievajúca fontána", "une fontaine chantante"}, {"Ale dnes nespieva", "mais aujourd'hui, elle ne chante pas"} };
static const Gloss ep10_dialog_g1[2] = { {"Pozri", "regarde"}, {"Fontána", "une fontaine"} };
static const char* const ep10_dialog_c1_items[3] = { "Pozri! Fontána!", "Pozri! Záhradník!", "Pozri! V rádiu!" };
static const Choice ep10_dialog_c1 = { CH_PICK, "qcm-font-na", ep10_dialog_c1_items, 3 };
static const Gloss ep10_dialog_g2[2] = { {"Je", "elle est"}, {"prázdna", "vide"} };
static const Gloss ep10_dialog_g3[3] = { {"Pri fontáne", "près de la fontaine"}, {"stojí", "se tient"}, {"záhradník", "un jardinier"} };
static const Gloss ep10_dialog_g4[2] = { {"Dobrý deň", "bonjour"}, {"Hľadáte vodu", "vous cherchez de l'eau"} };
static const Gloss ep10_dialog_g5[3] = { {"Dobrý deň", "bonjour"}, {"Áno", "oui"}, {"hľadáme vodu", "nous cherchons de l'eau"} };
static const char* const ep10_dialog_c5_items[3] = { "Dobrý deň.", "Áno,", "hľadáme vodu." };
static const Choice ep10_dialog_c5 = { CH_ORDER, "order-dobr-de-no-h-ad-me-vodu", ep10_dialog_c5_items, 3 };
static const Gloss ep10_dialog_g6[4] = { {"Prepáčte", "désolé, excusez-moi"}, {"Tu už voda nie je", "ici, il n'y a plus d'eau"}, {"Nikde", "nulle part"}, {"v meste", "en ville"} };
static const Gloss ep10_dialog_g7[1] = { {"A inde", "et ailleurs"} };
static const Gloss ep10_dialog_g8[5] = { {"Na západe", "à l'ouest"}, {"v Slovenskom raji", "dans le Paradis slovaque"}, {"ešte voda je", "il y a encore de l'eau"}, {"Hovorili to", "ils l'ont dit"}, {"v rádiu", "à la radio"} };
static const Gloss ep10_dialog_g9[1] = { {"Ďakujeme", "merci (de notre part à tous les deux)"} };
static const Gloss ep10_dialog_g10[1] = { {"Veľa šťastia", "bonne chance"} };
static const Line ep10_dialog[11] = {
  {SPK_NARRATOR, "V centre mesta je fontána. Spievajúca fontána. Ale dnes nespieva.", "Au centre-ville, il y a une fontaine. Une fontaine chantante. Mais aujourd'hui, elle ne chante pas.", ep10_dialog_g0, 4, "Mot nouveau signalé : fontána = la fontaine. Pour comprendre : spievať = chanter.", nullptr},
  {SPK_FOX, "Pozri! Fontána!", "Regarde ! Une fontaine !", ep10_dialog_g1, 2, nullptr, &ep10_dialog_c1},
  {SPK_ANDREA, "Je prázdna.", "Elle est vide.", ep10_dialog_g2, 2, "Mot nouveau signalé : prázdna = vide. On dit prázdna parce que fontána est un mot féminin.", nullptr},
  {SPK_NARRATOR, "Pri fontáne stojí záhradník.", "Près de la fontaine, un jardinier est là.", ep10_dialog_g3, 3, "Mot nouveau signalé : záhradník = le jardinier, celui qui s'occupe de la záhrada.", nullptr},
  {SPK_HUMAN, "Dobrý deň. Hľadáte vodu?", "Bonjour. Vous cherchez de l'eau ?", ep10_dialog_g4, 2, "Mot nouveau signalé : hľadáte = vous cherchez. On est deux, et c'est un adulte : il dit vy, le verbe finit en -te.", nullptr},
  {SPK_FOX, "Dobrý deň. Áno, hľadáme vodu.", "Bonjour. Oui, nous cherchons de l'eau.", ep10_dialog_g5, 3, "Avec un adulte qu'on ne connaît pas, on commence par Dobrý deň, jamais par ahoj.", &ep10_dialog_c5},
  {SPK_HUMAN, "Prepáčte. Tu už voda nie je. Nikde v meste.", "Désolé. Ici, il n'y a plus d'eau. Nulle part en ville.", ep10_dialog_g6, 4, "Mots nouveaux signalés : prepáčte = excusez-moi (c'est prepáč avec -te), nikde = nulle part.", nullptr},
  {SPK_ANDREA, "A inde?", "Et ailleurs ?", ep10_dialog_g7, 1, nullptr, nullptr},
  {SPK_HUMAN, "Na západe, v Slovenskom raji, ešte voda je. Hovorili to v rádiu.", "À l'ouest, dans le Paradis slovaque, il y en a encore. Ils l'ont dit à la radio.", ep10_dialog_g8, 5, "Mot nouveau signalé : v rádiu = à la radio. À écouter : hovorili = ils ont dit.", nullptr},
  {SPK_FOX, "Ďakujeme.", "Merci.", ep10_dialog_g9, 1, "ďakujem = je remercie, ďakujeme = nous remercions. Encore le -me du « nous ».", nullptr},
  {SPK_HUMAN, "Veľa šťastia.", "Bonne chance.", ep10_dialog_g10, 1, "Mot à mot : « beaucoup de bonheur ». Et c'est bien veľa : c'est une quantité de chance !", nullptr},
};
static const char* const ep10_x0_csk[3] = { "záhradník", "v rádiu", "nikde" };
static const char* const ep10_x0_cfr[3] = { "le jardinier", "à la radio", "nulle part" };
static const char* const ep10_x1_csk[3] = { "nikde", "prepáčte", "fontána" };
static const char* const ep10_x1_cfr[3] = { "nulle part", "excusez-moi (à un adulte)", "la fontaine" };
static const char* const ep10_x2_csk[3] = { "prázdna", "v rádiu", "hľadáte" };
static const char* const ep10_x2_cfr[3] = { "vide (dit d'une fontaine)", "à la radio", "vous cherchez" };
static const char* const ep10_x3_csk[3] = { "hľadáte", "Veľa šťastia!", "nikde" };
static const char* const ep10_x3_cfr[3] = { "vous cherchez", "Bonne chance !", "nulle part" };
static const char* const ep10_x4_csk[3] = { "fontána", "v rádiu", "prázdna" };
static const char* const ep10_x4_cfr[3] = { "la fontaine", "à la radio", "vide (dit d'une fontaine)" };
static const char* const ep10_x5_cfr[3] = { "Elle est vraie.", "Elle est forte.", "Elle est là." };
static const char* const ep10_x6_cfr[3] = { "Désolé. Ici, il y a encore de l'eau. Partout en ville.", "Désolé. Ici, il n'y a plus d'eau. Seulement en ville.", "Désolé. Ici, il n'y a plus de fontaine. Nulle part en ville." };
static const char* const ep10_x7_cfr[3] = { "Et ici ?", "Et maintenant ?", "Et loin ?" };
static const char* const ep10_x8_cfr[3] = { "À l'ouest, dans le Paradis slovaque, il n'y en a plus. Ils l'ont dit à la radio.", "À l'ouest, dans le Paradis slovaque, il y en a encore. Ils l'ont dit à la gare.", "À l'est, dans les Tatras, il y en a encore. Ils l'ont dit à la radio." };
static const char* const ep10_x9_cfr[3] = { "Bon voyage.", "Bonne nuit.", "Beaucoup d'eau." };
static const char* const ep10_x10_tsk[3] = { "Prepáčte.", "Tu už voda nie je.", "Nikde v meste." };
static const char* const ep10_x10_tfr[3] = { "Désolé.", "Ici, il n'y a plus d'eau.", "Nulle part en ville." };
static const char* const ep10_x11_tsk[3] = { "Dobrý deň.", "Áno,", "hľadáme vodu." };
static const char* const ep10_x11_tfr[3] = { "Bonjour.", "Oui,", "nous cherchons de l'eau." };
static const Exercise ep10_exercises[12] = {
  {EX_QCM, "qcm-font-na", "fontána", "la fontaine", ep10_x0_csk, ep10_x0_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-pr-zdna", "prázdna", "vide (dit d'une fontaine)", ep10_x1_csk, ep10_x1_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-nikde", "nikde", "nulle part", ep10_x2_csk, ep10_x2_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-prep-te", "prepáčte", "excusez-moi (à un adulte)", ep10_x3_csk, ep10_x3_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-z-hradn-k", "záhradník", "le jardinier", ep10_x4_csk, ep10_x4_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-je-pr-zdna", "Je prázdna.", "Elle est vide.", nullptr, ep10_x5_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-prep-te-tu-u-voda-nie-je-nikde-v-meste", "Prepáčte. Tu už voda nie je. Nikde v meste.", "Désolé. Ici, il n'y a plus d'eau. Nulle part en ville.", nullptr, ep10_x6_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-a-inde", "A inde?", "Et ailleurs ?", nullptr, ep10_x7_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-na-z-pade-v-slovenskom-raji-e-te-voda-je-hovorili-to-v-r-diu", "Na západe, v Slovenskom raji, ešte voda je. Hovorili to v rádiu.", "À l'ouest, dans le Paradis slovaque, il y en a encore. Ils l'ont dit à la radio.", nullptr, ep10_x8_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-ve-a-astia", "Veľa šťastia.", "Bonne chance.", nullptr, ep10_x9_cfr, 3, nullptr, nullptr, 0},
  {EX_ORDER, "order-prep-te-tu-u-voda-nie-je-nikde-v-meste", "Prepáčte. Tu už voda nie je. Nikde v meste.", "Désolé. Ici, il n'y a plus d'eau. Nulle part en ville.", nullptr, nullptr, 0, ep10_x10_tsk, ep10_x10_tfr, 3},
  {EX_ORDER, "order-dobr-de-no-h-ad-me-vodu", "Dobrý deň. Áno, hľadáme vodu.", "Bonjour. Oui, nous cherchons de l'eau.", nullptr, nullptr, 0, ep10_x11_tsk, ep10_x11_tfr, 3},
};
static const Line ep10_bonus[1] = {
  {SPK_ANDREA, "Hovoríš ako pravý Slovák.", "Tu parles comme un vrai Slovaque.", nullptr, 0, nullptr, nullptr},
};
static const Episode ep10 = { "10_Series_02_Sucho_05_fontana", "Sucho", 2, 5, "Fontána", "Sucho",
  ep10_dialog, 11, nullptr, 0, nullptr, 0, ep10_exercises, 12, ep10_bonus, 1 };

static const Gloss ep11_dialog_g0[4] = { {"\001 a Andrea sú", "\001 et Andrea sont"}, {"na stanici", "à la gare"}, {"Je tam veľa ľudí", "il y a beaucoup de monde"}, {"a veľmi málo tieňa", "et très peu d'ombre"} };
static const Gloss ep11_dialog_g1[5] = { {"Počkaj", "attends"}, {"Musím", "je dois"}, {"napísať", "écrire"}, {"tete", "à ma tante"}, {"lístok", "un petit mot"} };
static const Gloss ep11_dialog_g2[3] = { {"Ja mám", "moi, j'ai"}, {"lístok", "un billet, un ticket"}, {"Je super", "il est super"} };
static const Gloss ep11_dialog_g3[4] = { {"To nie je", "ce n'est pas"}, {"ten lístok", "ce billet-là"}, {"To je", "c'est"}, {"lístok z električky", "un ticket de tram"} };
static const Gloss ep11_dialog_g4[1] = { {"Aha", "ah"} };
static const Gloss ep11_dialog_g5[4] = { {"Andrea píše", "Andrea écrit"}, {"odišla som", "je suis partie"}, {"na pár dní", "pour quelques jours"}, {"Neboj sa", "ne t'inquiète pas"} };
static const Gloss ep11_dialog_g6[3] = { {"Pri okienku", "devant le guichet"}, {"stojí", "se tient"}, {"pani", "une dame"} };
static const Gloss ep11_dialog_g7[4] = { {"Dobrý deň", "bonjour"}, {"Dva lístky", "deux billets"}, {"na vlak", "pour le train"}, {"prosím", "s'il vous plaît"} };
static const Gloss ep11_dialog_g8[2] = { {"Dobrý deň", "bonjour"}, {"Kam idete", "où allez-vous"} };
static const Gloss ep11_dialog_g9[2] = { {"Do", "à, vers"}, {"Spišskej Novej Vsi", "Spišská Nová Ves"} };
static const Gloss ep11_dialog_g10[2] = { {"Dva lístky", "deux billets"}, {"Šťastnú cestu", "bon voyage"} };
static const Gloss ep11_dialog_g11[2] = { {"Vlak ide", "le train va"}, {"na západ", "vers l'ouest"} };
static const Line ep11_dialog[12] = {
  {SPK_NARRATOR, "\001 a Andrea sú na stanici. Je tam veľa ľudí a veľmi málo tieňa.", "\001 et Andrea sont à la gare. Il y a beaucoup de monde et très peu d'ombre.", ep11_dialog_g0, 4, "Mot nouveau signalé : na stanici = à la gare (stanica = la gare). Regarde bien : veľa ľudí (combien ?), mais veľmi málo (comment ?).", nullptr},
  {SPK_ANDREA, "Počkaj. Musím napísať tete lístok.", "Attends. Je dois écrire un mot à ma tante.", ep11_dialog_g1, 5, "Mot nouveau signalé : musím = je dois. Et lístok veut aussi dire « petit mot », comme un petit billet en français !", nullptr},
  {SPK_FOX, "Ja mám lístok! Je super!", "Moi, j'ai un billet ! Il est super !", ep11_dialog_g2, 3, nullptr, nullptr},
  {SPK_ANDREA, "To nie je ten lístok. To je lístok z električky.", "Ce n'est pas ce billet-là. Toi, tu as un ticket de tram.", ep11_dialog_g3, 4, nullptr, nullptr},
  {SPK_FOX, "Aha.", "Ah.", ep11_dialog_g4, 1, nullptr, nullptr},
  {SPK_NARRATOR, "Andrea píše: „Teta, odišla som na pár dní. Neboj sa. Andrea.“", "Andrea écrit : « Ma tante, je suis partie pour quelques jours. Ne t'inquiète pas. Andrea. »", ep11_dialog_g5, 4, "bojím sa = j'ai peur ; Neboj sa! = n'aie pas peur, ne t'inquiète pas. À écouter : odišla som = je suis partie (dit par une fille).", nullptr},
  {SPK_NARRATOR, "Pri okienku stojí pani.", "Devant le guichet, une dame est là.", ep11_dialog_g6, 3, "Pour comprendre : okienko = la petite fenêtre, le guichet. Encore un « petit » mot en -ko, comme jazierko.", nullptr},
  {SPK_ANDREA, "Dobrý deň. Dva lístky na vlak, prosím.", "Bonjour. Deux billets de train, s'il vous plaît.", ep11_dialog_g7, 4, "Mot nouveau signalé : lístky = des billets. Quand ils sont plusieurs, lístok perd son o et finit en -y.", nullptr},
  {SPK_HUMAN, "Dobrý deň. Kam idete?", "Bonjour. Vous allez où ?", ep11_dialog_g8, 2, "kam = où (vers où). Avec un adulte inconnu, on vouvoie : ideš (tu vas) devient idete (vous allez).", nullptr},
  {SPK_ANDREA, "Do Spišskej Novej Vsi.", "À Spišská Nová Ves.", ep11_dialog_g9, 2, "Après do, le nom de la ville change à la fin. Même les Slovaques réfléchissent deux secondes pour celui-là !", nullptr},
  {SPK_HUMAN, "Dva lístky. Šťastnú cestu.", "Deux billets. Bon voyage.", ep11_dialog_g10, 2, "Mot nouveau signalé : Šťastnú cestu! = bon voyage ! Mot à mot : « heureuse route ».", nullptr},
  {SPK_NARRATOR, "Vlak ide na západ.", "Le train part vers l'ouest.", ep11_dialog_g11, 2, "na západe = à l'ouest ; na západ = vers l'ouest.", nullptr},
};
static const char* const ep11_x0_cfr[3] = { "Attends. Je dois acheter un billet pour ma tante.", "Attends. Je dois écrire un mot à ma maman.", "Attends. Je veux dormir chez ma tante." };
static const char* const ep11_x1_cfr[3] = { "C'est ce billet-là. Toi, tu as un ticket de train.", "Ce n'est pas ce billet-là. Toi, tu as un petit mot.", "Ce n'est pas mon billet. Toi, tu as un ticket de tram." };
static const char* const ep11_x2_cfr[3] = { "Bonjour. Deux tickets de tram, s'il vous plaît.", "Bonjour. Un billet de train, s'il vous plaît.", "Bonjour. Deux billets de train, merci." };
static const char* const ep11_x3_cfr[3] = { "Bonjour. Tu vas où ?", "Bonjour. Vous cherchez de l'eau ?", "Bonjour. Vous êtes où ?" };
static const char* const ep11_x4_cfr[3] = { "Deux billets. Bonne chance.", "Un billet. Bon voyage.", "Deux billets. Bonne nuit." };
static const char* const ep11_x5_cfr[3] = { "Moi, j'ai une carte ! Elle est super !", "Toi, tu as un billet ! Il est super !", "Moi, j'ai un billet ! Il est vide !" };
static const char* const ep11_x6_tsk[4] = { "Dobrý deň.", "Dva lístky", "na vlak,", "prosím." };
static const char* const ep11_x6_tfr[4] = { "Bonjour.", "Deux billets", "de train,", "s'il vous plaît." };
static const char* const ep11_x7_tsk[3] = { "Dva", "lístky.", "Šťastnú cestu." };
static const char* const ep11_x7_tfr[3] = { "Deux", "billets.", "Bon voyage." };
static const Exercise ep11_exercises[8] = {
  {EX_LISTEN, "listen-po-kaj-mus-m-nap-sa-tete-l-stok", "Počkaj. Musím napísať tete lístok.", "Attends. Je dois écrire un mot à ma tante.", nullptr, ep11_x0_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-to-nie-je-ten-l-stok-to-je-l-stok-z-elektri-ky", "To nie je ten lístok. To je lístok z električky.", "Ce n'est pas ce billet-là. Toi, tu as un ticket de tram.", nullptr, ep11_x1_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-dobr-de-dva-l-stky-na-vlak-pros-m", "Dobrý deň. Dva lístky na vlak, prosím.", "Bonjour. Deux billets de train, s'il vous plaît.", nullptr, ep11_x2_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-dobr-de-kam-idete", "Dobrý deň. Kam idete?", "Bonjour. Vous allez où ?", nullptr, ep11_x3_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-dva-l-stky-astn-cestu", "Dva lístky. Šťastnú cestu.", "Deux billets. Bon voyage.", nullptr, ep11_x4_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-ja-m-m-l-stok-je-super", "Ja mám lístok! Je super!", "Moi, j'ai un billet ! Il est super !", nullptr, ep11_x5_cfr, 3, nullptr, nullptr, 0},
  {EX_ORDER, "order-dobr-de-dva-l-stky-na-vlak-pros-m", "Dobrý deň. Dva lístky na vlak, prosím.", "Bonjour. Deux billets de train, s'il vous plaît.", nullptr, nullptr, 0, ep11_x6_tsk, ep11_x6_tfr, 4},
  {EX_ORDER, "order-dva-l-stky-astn-cestu", "Dva lístky. Šťastnú cestu.", "Deux billets. Bon voyage.", nullptr, nullptr, 0, ep11_x7_tsk, ep11_x7_tfr, 3},
};
static const Episode ep11 = { "10_Series_02_Sucho_06_extra", "Sucho", 2, 6, "Na stanici", "Sucho",
  ep11_dialog, 12, nullptr, 0, nullptr, 0, ep11_exercises, 8, nullptr, 0 };

static const Gloss ep12_dialog_g0[4] = { {"Vlak stojí", "le train s'arrête"}, {"v Spišskej Novej Vsi", "à Spišská Nová Ves"}, {"vystupujú", "ils descendent"}, {"Je horúco", "il fait chaud"} };
static const Gloss ep12_dialog_g1[2] = { {"Kam", "où (vers où)"}, {"teraz", "maintenant"} };
static const Gloss ep12_dialog_g2[3] = { {"Poď", "viens"}, {"za mnou", "derrière moi"}, {"Mám mapu", "j'ai une carte"} };
static const Gloss ep12_dialog_g3[3] = { {"Ty", "toi"}, {"vždy", "toujours"}, {"máš mapu", "tu as une carte"} };
static const char* const ep12_dialog_c3_items[3] = { "Ty vždy máš mapu.", "Ty tak máš mapu.", "Ty pomaly máš mapu." };
static const Choice ep12_dialog_c3 = { CH_PICK, "qcm-v-dy", ep12_dialog_c3_items, 3 };
static const Gloss ep12_dialog_g4[4] = { {"A ty", "et toi"}, {"vždy", "toujours"}, {"máš otázky", "tu as des questions"}, {"Poď", "viens"} };
static const Gloss ep12_dialog_g5[3] = { {"Počkaj", "attends"}, {"Idem", "je vais"}, {"pomaly", "lentement"} };
static const char* const ep12_dialog_c5_items[3] = { "Počkaj! Idem pomaly!", "Počkaj! Idem rýchlo!", "Počkaj! Idem vždy!" };
static const Choice ep12_dialog_c5 = { CH_PICK, "qcm-pomaly", ep12_dialog_c5_items, 3 };
static const Gloss ep12_dialog_g6[3] = { {"Ja viem", "je sais"}, {"Tak", "alors"}, {"poď rýchlo", "viens vite"} };
static const Line ep12_dialog[7] = {
  {SPK_NARRATOR, "Vlak stojí v Spišskej Novej Vsi. \001 a Andrea vystupujú. Je horúco.", "Le train s'arrête à Spišská Nová Ves. \001 et Andrea descendent. Il fait chaud.", ep12_dialog_g0, 4, "Pour comprendre : vystupujú = ils descendent (d'un train, d'un bus).", nullptr},
  {SPK_FOX, "Kam teraz?", "On va où maintenant ?", ep12_dialog_g1, 2, "Mot à mot : « Vers où, maintenant ? ». Pas besoin de verbe, tout le monde comprend.", nullptr},
  {SPK_ANDREA, "Poď za mnou. Mám mapu.", "Suis-moi. J'ai une carte.", ep12_dialog_g2, 3, "Poď za mnou = « viens derrière moi », donc « suis-moi ».", nullptr},
  {SPK_FOX, "Ty vždy máš mapu.", "Tu as toujours une carte.", ep12_dialog_g3, 3, nullptr, &ep12_dialog_c3},
  {SPK_ANDREA, "A ty vždy máš otázky. Poď.", "Et toi, tu as toujours des questions. Viens.", ep12_dialog_g4, 4, "otázka = une question, otázky = des questions.", nullptr},
  {SPK_FOX, "Počkaj! Idem pomaly!", "Attends ! Je vais lentement !", ep12_dialog_g5, 3, nullptr, &ep12_dialog_c5},
  {SPK_ANDREA, "Ja viem. Tak poď rýchlo.", "Je sais. Alors viens vite.", ep12_dialog_g6, 3, "pomaly = lentement, rýchlo = vite. Deux contraires à apprendre ensemble.", nullptr},
};
static const Gloss ep12_extra_g0[2] = { {"Ideme", "nous allons"}, {"cez les", "à travers la forêt"} };
static const Gloss ep12_extra_g1[2] = { {"V lese", "dans la forêt"}, {"je tieň", "il y a de l'ombre"} };
static const Line ep12_extra[2] = {
  {SPK_NONE, "Ideme cez les.", "On traverse la forêt.", ep12_extra_g0, 2, nullptr, nullptr},
  {SPK_NONE, "V lese je tieň.", "Dans la forêt, il y a de l'ombre.", ep12_extra_g1, 2, "les devient v lese après v, comme mesto devient v meste.", nullptr},
};
static const Word ep12_words[9] = {
  {"Kam teraz?", "On va où maintenant ?"},
  {"Poď za mnou!", "Suis-moi !"},
  {"mapa", "la carte (rappel)"},
  {"vždy", "toujours"},
  {"otázky", "des questions"},
  {"pomaly", "lentement"},
  {"rýchlo", "vite"},
  {"Ja viem.", "Je sais."},
  {"tak", "alors"},
};
static const char* const ep12_x0_csk[3] = { "tak", "pomaly", "rýchlo" };
static const char* const ep12_x0_cfr[3] = { "alors", "lentement", "vite" };
static const char* const ep12_x1_csk[3] = { "rýchlo", "vždy", "tak" };
static const char* const ep12_x1_cfr[3] = { "vite", "toujours", "alors" };
static const char* const ep12_x2_csk[3] = { "pomaly", "vždy", "otázky" };
static const char* const ep12_x2_cfr[3] = { "lentement", "toujours", "des questions" };
static const char* const ep12_x3_csk[3] = { "Kam teraz?", "Ja viem.", "tak" };
static const char* const ep12_x3_cfr[3] = { "On va où maintenant ?", "Je sais.", "alors" };
static const char* const ep12_x4_csk[3] = { "tieň", "cez", "otázky" };
static const char* const ep12_x4_cfr[3] = { "l'ombre", "à travers", "des questions" };
static const char* const ep12_x5_cfr[3] = { "Suis-moi. J'ai une question.", "Attends-moi. J'ai une carte.", "Suis-moi. Tu as une carte." };
static const char* const ep12_x6_cfr[3] = { "Et toi, tu as toujours une carte. Viens.", "Et toi, tu n'as jamais de questions. Viens.", "Et toi, tu as toujours des questions. Attends." };
static const char* const ep12_x7_cfr[3] = { "Attends ! Je vais vite !", "Viens ! Je vais lentement !", "Attends ! Tu vas lentement !" };
static const char* const ep12_x8_cfr[3] = { "Je sais. Alors viens lentement.", "Je ne sais pas. Alors viens vite.", "Je sais. Alors attends ici." };
static const char* const ep12_x9_tsk[3] = { "Ja viem.", "Tak", "poď rýchlo." };
static const char* const ep12_x9_tfr[3] = { "Je sais.", "Alors", "viens vite." };
static const char* const ep12_x10_tsk[3] = { "A ty", "vždy máš otázky.", "Poď." };
static const char* const ep12_x10_tfr[3] = { "Et toi,", "tu as toujours des questions.", "Viens." };
static const Exercise ep12_exercises[11] = {
  {EX_QCM, "qcm-v-dy", "vždy", "toujours", ep12_x0_csk, ep12_x0_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-pomaly", "pomaly", "lentement", ep12_x1_csk, ep12_x1_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-r-chlo", "rýchlo", "vite", ep12_x2_csk, ep12_x2_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-po-za-mnou", "Poď za mnou!", "Suis-moi !", ep12_x3_csk, ep12_x3_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-les", "les", "la forêt", ep12_x4_csk, ep12_x4_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-po-za-mnou-m-m-mapu", "Poď za mnou. Mám mapu.", "Suis-moi. J'ai une carte.", nullptr, ep12_x5_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-a-ty-v-dy-m-ot-zky-po", "A ty vždy máš otázky. Poď.", "Et toi, tu as toujours des questions. Viens.", nullptr, ep12_x6_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-po-kaj-idem-pomaly", "Počkaj! Idem pomaly!", "Attends ! Je vais lentement !", nullptr, ep12_x7_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-ja-viem-tak-po-r-chlo", "Ja viem. Tak poď rýchlo.", "Je sais. Alors viens vite.", nullptr, ep12_x8_cfr, 3, nullptr, nullptr, 0},
  {EX_ORDER, "order-ja-viem-tak-po-r-chlo", "Ja viem. Tak poď rýchlo.", "Je sais. Alors viens vite.", nullptr, nullptr, 0, ep12_x9_tsk, ep12_x9_tfr, 3},
  {EX_ORDER, "order-a-ty-v-dy-m-ot-zky-po", "A ty vždy máš otázky. Poď.", "Et toi, tu as toujours des questions. Viens.", nullptr, nullptr, 0, ep12_x10_tsk, ep12_x10_tfr, 3},
};
static const Line ep12_bonus[1] = {
  {SPK_ANDREA, "Rýchlo sa učíš!", "Tu apprends vite !", nullptr, 0, nullptr, nullptr},
};
static const Episode ep12 = { "10_Series_03_Raj_01_kam-teraz", "Raj", 3, 1, "Kam teraz?", "Spišská Nová Ves",
  ep12_dialog, 7, ep12_extra, 2, ep12_words, 9, ep12_exercises, 11, ep12_bonus, 1 };

static const Gloss ep13_dialog_g0[3] = { {"Idú cez les", "ils traversent la forêt"}, {"Je tu tieň", "ici, il y a de l'ombre"}, {"ale voda nie je", "mais il n'y a pas d'eau"} };
static const Gloss ep13_dialog_g1[3] = { {"Je to", "c'est"}, {"ešte", "encore"}, {"ďaleko", "loin"} };
static const Gloss ep13_dialog_g2[4] = { {"Nie", "non"}, {"Ale", "mais"}, {"ticho", "chut, silence"}, {"Počuješ", "tu entends"} };
static const Gloss ep13_dialog_g3[2] = { {"Nič", "rien"}, {"nepočujem", "je n'entends pas"} };
static const char* const ep13_dialog_c3_items[3] = { "Nič nepočujem.", "Niekde nepočujem.", "Ticho nepočujem." };
static const Choice ep13_dialog_c3 = { CH_PICK, "qcm-ni", ep13_dialog_c3_items, 3 };
static const Gloss ep13_dialog_g4[4] = { {"Presne", "exactement"}, {"Ticho", "le silence"}, {"Aj vtáky", "même les oiseaux"}, {"mlčia", "se taisent"} };
static const Gloss ep13_dialog_g5[2] = { {"To nie je", "ce n'est pas"}, {"dobré", "bon"} };
static const Gloss ep13_dialog_g6[3] = { {"Nie", "non"}, {"Ale počkaj", "mais attends"}, {"Počuješ to", "tu entends ça"} };
static const Gloss ep13_dialog_g7[3] = { {"Niekde", "quelque part"}, {"blízko", "près"}, {"tečie voda", "de l'eau coule"} };
static const Gloss ep13_dialog_g8[2] = { {"Voda", "de l'eau"}, {"Potok", "un ruisseau"} };
static const Line ep13_dialog[9] = {
  {SPK_NARRATOR, "Idú cez les. Je tu tieň, ale voda nie je.", "Ils traversent la forêt. Il y a de l'ombre, mais pas d'eau.", ep13_dialog_g0, 3, nullptr, nullptr},
  {SPK_FOX, "Je to ešte ďaleko?", "C'est encore loin ?", ep13_dialog_g1, 3, nullptr, nullptr},
  {SPK_ANDREA, "Nie. Ale ticho. Počuješ?", "Non. Mais chut. Tu entends ?", ep13_dialog_g2, 4, "ticho veut dire « le silence », mais aussi « chut ! ». Et Ticho! tout seul, c'est « Silence ! ».", nullptr},
  {SPK_FOX, "Nič nepočujem.", "Je n'entends rien.", ep13_dialog_g3, 2, nullptr, &ep13_dialog_c3},
  {SPK_ANDREA, "Presne. Ticho. Aj vtáky mlčia.", "Exactement. Le silence. Même les oiseaux se taisent.", ep13_dialog_g4, 4, "Pour comprendre : mlčia = ils se taisent. vták = un oiseau, vtáky = des oiseaux.", nullptr},
  {SPK_FOX, "To nie je dobré.", "C'est pas bon.", ep13_dialog_g5, 2, nullptr, nullptr},
  {SPK_ANDREA, "Nie. Ale počkaj... Počuješ to?", "Non. Mais attends... Tu entends ça ?", ep13_dialog_g6, 3, nullptr, nullptr},
  {SPK_NARRATOR, "Niekde blízko tečie voda.", "Quelque part tout près, de l'eau coule.", ep13_dialog_g7, 3, "niekde = quelque part, nikde = nulle part. Une seule lettre de différence !", nullptr},
  {SPK_FOX, "Voda! Potok!", "De l'eau ! Un ruisseau !", ep13_dialog_g8, 2, nullptr, nullptr},
};
static const Gloss ep13_extra_g0[3] = { {"Počuješ", "tu entends"}, {"Vták", "un oiseau"}, {"spieva", "chante"} };
static const Gloss ep13_extra_g1[2] = { {"Spieva", "il chante"}, {"veľmi hlasno", "très fort"} };
static const Line ep13_extra[2] = {
  {SPK_NONE, "Počuješ? Vták spieva.", "Tu entends ? Un oiseau chante.", ep13_extra_g0, 3, "spievať = chanter, comme la Spievajúca fontána de Košice.", nullptr},
  {SPK_NONE, "Spieva veľmi hlasno.", "Il chante très fort.", ep13_extra_g1, 2, nullptr, nullptr},
};
static const Word ep13_words[10] = {
  {"počujem", "j'entends"},
  {"počuješ", "tu entends"},
  {"ticho", "le silence, chut"},
  {"nič", "rien"},
  {"ešte", "encore (rappel)"},
  {"vtáky", "les oiseaux"},
  {"dobré", "bon, bien"},
  {"niekde", "quelque part"},
  {"blízko", "près (rappel)"},
  {"tečie", "coule"},
};
static const char* const ep13_x0_csk[3] = { "počujem", "tečie", "spieva" };
static const char* const ep13_x0_cfr[3] = { "j'entends", "coule", "chante" };
static const char* const ep13_x1_csk[3] = { "niekde", "ticho", "ešte" };
static const char* const ep13_x1_cfr[3] = { "quelque part", "le silence, chut", "encore" };
static const char* const ep13_x2_csk[3] = { "nič", "blízko", "ešte" };
static const char* const ep13_x2_cfr[3] = { "rien", "près", "encore" };
static const char* const ep13_x3_csk[3] = { "tečie", "ticho", "dobré" };
static const char* const ep13_x3_cfr[3] = { "coule", "le silence, chut", "bon, bien" };
static const char* const ep13_x4_csk[3] = { "spieva", "počujem", "hlasno" };
static const char* const ep13_x4_cfr[3] = { "chante", "j'entends", "fort (pour un son)" };
static const char* const ep13_x5_cfr[3] = { "C'est encore près ?", "C'est déjà loin ?", "C'est où ?" };
static const char* const ep13_x6_cfr[3] = { "Je ne vois rien.", "J'entends tout.", "Tu n'entends rien." };
static const char* const ep13_x7_cfr[3] = { "Exactement. Le silence. Même les oiseaux chantent.", "Exactement. Le bruit. Même les oiseaux se taisent.", "Presque. Le silence. Même les renards se taisent." };
static const char* const ep13_x8_cfr[3] = { "Tu entends ? Un oiseau se tait.", "Tu vois ? Un oiseau chante.", "Tu entends ? L'eau coule." };
static const char* const ep13_x9_tsk[3] = { "Nie.", "Ale počkaj...", "Počuješ to?" };
static const char* const ep13_x9_tfr[3] = { "Non.", "Mais attends...", "Tu entends ça ?" };
static const char* const ep13_x10_tsk[3] = { "Nie.", "Ale ticho.", "Počuješ?" };
static const char* const ep13_x10_tfr[3] = { "Non.", "Mais chut.", "Tu entends ?" };
static const Exercise ep13_exercises[11] = {
  {EX_QCM, "qcm-po-uje", "počuješ", "tu entends", ep13_x0_csk, ep13_x0_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-ni", "nič", "rien", ep13_x1_csk, ep13_x1_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-niekde", "niekde", "quelque part", ep13_x2_csk, ep13_x2_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-vt-ky", "vtáky", "les oiseaux", ep13_x3_csk, ep13_x3_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-te-ie", "tečie", "coule", ep13_x4_csk, ep13_x4_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-je-to-e-te-aleko", "Je to ešte ďaleko?", "C'est encore loin ?", nullptr, ep13_x5_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-ni-nepo-ujem", "Nič nepočujem.", "Je n'entends rien.", nullptr, ep13_x6_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-presne-ticho-aj-vt-ky-ml-ia", "Presne. Ticho. Aj vtáky mlčia.", "Exactement. Le silence. Même les oiseaux se taisent.", nullptr, ep13_x7_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-po-uje-vt-k-spieva", "Počuješ? Vták spieva.", "Tu entends ? Un oiseau chante.", nullptr, ep13_x8_cfr, 3, nullptr, nullptr, 0},
  {EX_ORDER, "order-nie-ale-po-kaj-po-uje-to", "Nie. Ale počkaj... Počuješ to?", "Non. Mais attends... Tu entends ça ?", nullptr, nullptr, 0, ep13_x9_tsk, ep13_x9_tfr, 3},
  {EX_ORDER, "order-nie-ale-ticho-po-uje", "Nie. Ale ticho. Počuješ?", "Non. Mais chut. Tu entends ?", nullptr, nullptr, 0, ep13_x10_tsk, ep13_x10_tfr, 3},
};
static const Line ep13_bonus[1] = {
  {SPK_ANDREA, "Dobre počuješ!", "Tu entends bien !", nullptr, 0, nullptr, nullptr},
};
static const Episode ep13 = { "10_Series_03_Raj_02_pocujes", "Raj", 3, 2, "Počuješ?", "Raj",
  ep13_dialog, 9, ep13_extra, 2, ep13_words, 10, ep13_exercises, 11, ep13_bonus, 1 };

static const Gloss ep14_dialog_g0[3] = { {"Potok je malý", "le ruisseau est petit"}, {"Vody je v ňom málo", "il y a peu d'eau dedans"}, {"ale je tam", "mais elle est là"} };
static const Gloss ep14_dialog_g1[2] = { {"Je tu voda", "il y a de l'eau"}, {"Je tu veľa vody", "il y a beaucoup d'eau"} };
static const Gloss ep14_dialog_g2[3] = { {"Nie je tu veľa", "il n'y en a pas beaucoup"}, {"Je tu dosť", "il y en a assez"}, {"Pre dvoch", "pour deux"} };
static const Gloss ep14_dialog_g3[2] = { {"To", "ça"}, {"mi stačí", "me suffit"} };
static const Gloss ep14_dialog_g4[4] = { {"Najprv", "d'abord"}, {"sa napi", "bois"}, {"Potom", "ensuite"}, {"rozprávaj", "parle"} };
static const Gloss ep14_dialog_g5[3] = { {"Ty si", "tu es"}, {"zase", "encore"}, {"šéf", "le chef"} };
static const Gloss ep14_dialog_g6[4] = { {"Ja mám mapu", "moi, j'ai la carte"}, {"Ty máš batoh", "toi, tu as le sac à dos"}, {"Takže", "donc"}, {"ja som šéf", "c'est moi le chef"} };
static const Gloss ep14_dialog_g7[2] = { {"To je", "c'est"}, {"nespravodlivé", "injuste"} };
static const Gloss ep14_dialog_g8[1] = { {"Je", "ça l'est"} };
static const Line ep14_dialog[9] = {
  {SPK_NARRATOR, "Potok je malý. Vody je v ňom málo, ale je tam.", "Le ruisseau est petit. Il y a peu d'eau dedans, mais il y en a.", ep14_dialog_g0, 3, nullptr, nullptr},
  {SPK_FOX, "Je tu voda! Je tu veľa vody!", "Il y a de l'eau ! Il y a beaucoup d'eau !", ep14_dialog_g1, 2, nullptr, nullptr},
  {SPK_ANDREA, "Nie je tu veľa. Je tu dosť. Pre dvoch.", "Il n'y en a pas beaucoup. Il y en a assez. Pour deux.", ep14_dialog_g2, 3, "veľa = beaucoup, dosť = assez, málo = peu. Andrea compte précisément.", nullptr},
  {SPK_FOX, "To mi stačí.", "Ça me suffit.", ep14_dialog_g3, 2, "Comme chýba mi (il me manque), stačí mi se construit « à l'envers » : « ça suffit à moi ».", nullptr},
  {SPK_ANDREA, "Najprv sa napi. Potom rozprávaj.", "D'abord, bois. Ensuite, parle.", ep14_dialog_g4, 4, "Napi sa! = bois un coup ! Rozprávaj! = parle, raconte !", nullptr},
  {SPK_FOX, "Ty si zase šéf.", "Tu fais encore la cheffe.", ep14_dialog_g5, 3, "En slovaque familier, on dit šéf même pour une fille. Tu ne vas pas chipoter.", nullptr},
  {SPK_ANDREA, "Ja mám mapu. Ty máš batoh. Takže ja som šéf.", "Moi, j'ai la carte. Toi, tu as le sac à dos. Donc c'est moi la cheffe.", ep14_dialog_g6, 4, nullptr, nullptr},
  {SPK_FOX, "To je nespravodlivé.", "C'est injuste.", ep14_dialog_g7, 2, "Pour comprendre : nespravodlivé = injuste. Un mot long pour une idée courte.", nullptr},
  {SPK_ANDREA, "Je.", "Oui.", ep14_dialog_g8, 1, "Encore un « oui » en répétant le verbe : Je. = « ça l'est ».", nullptr},
};
static const Gloss ep14_extra_g0[2] = { {"Mám", "j'ai"}, {"smäd", "soif"} };
static const Gloss ep14_extra_g1[3] = { {"Tu je", "voilà"}, {"fľaša vody", "une bouteille d'eau"}, {"Môžeš piť", "tu peux boire"} };
static const Line ep14_extra[2] = {
  {SPK_NONE, "Mám smäd.", "J'ai soif.", ep14_extra_g0, 2, nullptr, nullptr},
  {SPK_NONE, "Tu je fľaša vody. Môžeš piť.", "Voilà une bouteille d'eau. Tu peux boire.", ep14_extra_g1, 3, "môžeš = tu peux. On le reverra bientôt.", nullptr},
};
static const Word ep14_words[10] = {
  {"dosť", "assez"},
  {"To mi stačí.", "Ça me suffit."},
  {"pre dvoch", "pour deux"},
  {"najprv", "d'abord"},
  {"potom", "ensuite"},
  {"Napi sa!", "Bois !"},
  {"zase", "encore, de nouveau"},
  {"šéf", "le chef"},
  {"batoh", "le sac à dos (rappel)"},
  {"takže", "donc"},
};
static const char* const ep14_x0_csk[3] = { "zase", "potom", "najprv" };
static const char* const ep14_x0_cfr[3] = { "encore, de nouveau", "ensuite", "d'abord" };
static const char* const ep14_x1_csk[3] = { "potom", "zase", "takže" };
static const char* const ep14_x1_cfr[3] = { "ensuite", "encore, de nouveau", "donc" };
static const char* const ep14_x2_csk[3] = { "najprv", "dosť", "takže" };
static const char* const ep14_x2_cfr[3] = { "d'abord", "assez", "donc" };
static const char* const ep14_x3_csk[3] = { "zase", "dosť", "potom" };
static const char* const ep14_x3_cfr[3] = { "encore, de nouveau", "assez", "ensuite" };
static const char* const ep14_x4_csk[3] = { "fľaša", "šéf", "batoh" };
static const char* const ep14_x4_cfr[3] = { "la bouteille", "le chef", "le sac à dos" };
static const char* const ep14_x5_cfr[3] = { "Il y en a beaucoup. Il y en a trop. Pour deux.", "Il n'y en a pas beaucoup. Il y en a assez. Pour moi.", "Il n'y en a pas assez. Il y en a peu. Pour deux." };
static const char* const ep14_x6_cfr[3] = { "Ça me manque.", "Ça te suffit.", "Ça me plaît." };
static const char* const ep14_x7_cfr[3] = { "D'abord, parle. Ensuite, bois.", "D'abord, bois. Ensuite, dors.", "Bois encore. Ensuite, parle." };
static const char* const ep14_x8_cfr[3] = { "C'est juste.", "C'est inutile.", "C'est mauvais." };
static const char* const ep14_x9_tsk[3] = { "Najprv", "sa napi.", "Potom rozprávaj." };
static const char* const ep14_x9_tfr[3] = { "D'abord,", "bois.", "Ensuite, parle." };
static const char* const ep14_x10_tsk[3] = { "Tu je", "fľaša vody.", "Môžeš piť." };
static const char* const ep14_x10_tfr[3] = { "Voilà", "une bouteille d'eau.", "Tu peux boire." };
static const Exercise ep14_exercises[11] = {
  {EX_QCM, "qcm-dos", "dosť", "assez", ep14_x0_csk, ep14_x0_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-najprv", "najprv", "d'abord", ep14_x1_csk, ep14_x1_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-potom", "potom", "ensuite", ep14_x2_csk, ep14_x2_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-tak-e", "takže", "donc", ep14_x3_csk, ep14_x3_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-sm-d", "smäd", "la soif", ep14_x4_csk, ep14_x4_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-nie-je-tu-ve-a-je-tu-dos-pre-dvoch", "Nie je tu veľa. Je tu dosť. Pre dvoch.", "Il n'y en a pas beaucoup. Il y en a assez. Pour deux.", nullptr, ep14_x5_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-to-mi-sta", "To mi stačí.", "Ça me suffit.", nullptr, ep14_x6_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-najprv-sa-napi-potom-rozpr-vaj", "Najprv sa napi. Potom rozprávaj.", "D'abord, bois. Ensuite, parle.", nullptr, ep14_x7_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-to-je-nespravodliv", "To je nespravodlivé.", "C'est injuste.", nullptr, ep14_x8_cfr, 3, nullptr, nullptr, 0},
  {EX_ORDER, "order-najprv-sa-napi-potom-rozpr-vaj", "Najprv sa napi. Potom rozprávaj.", "D'abord, bois. Ensuite, parle.", nullptr, nullptr, 0, ep14_x9_tsk, ep14_x9_tfr, 3},
  {EX_ORDER, "order-tu-je-f-a-a-vody-m-e-pi", "Tu je fľaša vody. Môžeš piť.", "Voilà une bouteille d'eau. Tu peux boire.", nullptr, nullptr, 0, ep14_x10_tsk, ep14_x10_tfr, 3},
};
static const Line ep14_bonus[1] = {
  {SPK_ANDREA, "Dobre, napi sa ešte.", "Bon, bois encore un peu.", nullptr, 0, nullptr, nullptr},
};
static const Episode ep14 = { "10_Series_03_Raj_03_je-tu-voda", "Raj", 3, 3, "Je tu voda!", "Raj",
  ep14_dialog, 9, ep14_extra, 2, ep14_words, 10, ep14_exercises, 11, ep14_bonus, 1 };

static const Gloss ep15_dialog_g0[5] = { {"Cesta vedie", "le chemin mène"}, {"do rokliny", "dans une gorge"}, {"V skale", "dans la roche"}, {"sú rebríky", "il y a des échelles"}, {"a mostíky nad vodou", "et des petits ponts au-dessus de l'eau"} };
static const Gloss ep15_dialog_g1[2] = { {"To nemyslíš", "tu ne penses pas ça"}, {"vážne", "sérieusement"} };
static const Gloss ep15_dialog_g2[4] = { {"Myslím", "je le pense"}, {"Idem prvá", "je passe la première"}, {"Ty ideš", "toi, tu vas"}, {"za mnou", "derrière moi"} };
static const Gloss ep15_dialog_g3[3] = { {"Po rebríkoch", "par des échelles"}, {"Ja nemám rád", "moi, je n'aime pas"}, {"výšky", "les hauteurs"} };
static const Gloss ep15_dialog_g4[2] = { {"Nepozeraj", "ne regarde pas"}, {"dole", "en bas"} };
static const Gloss ep15_dialog_g5[3] = { {"Ja nepozerám", "je ne regarde pas"}, {"Ja zatváram", "je ferme"}, {"oči", "les yeux"} };
static const Gloss ep15_dialog_g6[3] = { {"Zatvorené oči", "les yeux fermés"}, {"a rebrík", "et une échelle"}, {"To je zlý nápad", "c'est une mauvaise idée"} };
static const Gloss ep15_dialog_g7[3] = { {"Všetky moje nápady", "toutes mes idées"}, {"sú zlé", "sont mauvaises"}, {"Je to môj štýl", "c'est mon style"} };
static const Gloss ep15_dialog_g8[3] = { {"Poď", "viens"}, {"Ak spadneš", "si tu tombes"}, {"chytím ťa", "je te rattrape"} };
static const Gloss ep15_dialog_g9[2] = { {"Ty", "toi"}, {"Ty si zajac", "tu es un lièvre"} };
static const Gloss ep15_dialog_g10[2] = { {"Zajačica", "une hase"}, {"A silná", "et forte"} };
static const Gloss ep15_dialog_g11[2] = { {"Dobre", "d'accord"}, {"Idem", "j'y vais"} };
static const Line ep15_dialog[12] = {
  {SPK_NARRATOR, "Cesta vedie do rokliny. V skale sú rebríky a mostíky nad vodou.", "Le chemin mène dans une gorge. Dans la roche, il y a des échelles et des petits ponts au-dessus de l'eau.", ep15_dialog_g0, 5, "Pour comprendre : skala = la roche, mostík = un petit pont (un « petit » most).", nullptr},
  {SPK_FOX, "To nemyslíš vážne.", "Tu ne parles pas sérieusement.", ep15_dialog_g1, 2, nullptr, nullptr},
  {SPK_ANDREA, "Myslím. Idem prvá. Ty ideš za mnou.", "Si. Je passe la première. Toi, tu me suis.", ep15_dialog_g2, 4, "prvá = la première (une fille). Un garçon dirait prvý.", nullptr},
  {SPK_FOX, "Po rebríkoch? Ja nemám rád výšky!", "Par des échelles ? Moi, je n'aime pas les hauteurs !", ep15_dialog_g3, 3, "mám rád = j'aime (dit par un garçon) ; nemám rád = je n'aime pas.", nullptr},
  {SPK_ANDREA, "Nepozeraj dole.", "Ne regarde pas en bas.", ep15_dialog_g4, 2, nullptr, nullptr},
  {SPK_FOX, "Ja nepozerám. Ja zatváram oči.", "Je ne regarde pas. Je ferme les yeux.", ep15_dialog_g5, 3, nullptr, nullptr},
  {SPK_ANDREA, "Zatvorené oči a rebrík? To je zlý nápad.", "Les yeux fermés et une échelle ? Mauvaise idée.", ep15_dialog_g6, 3, nullptr, nullptr},
  {SPK_FOX, "Všetky moje nápady sú zlé. Je to môj štýl.", "Toutes mes idées sont mauvaises. C'est mon style.", ep15_dialog_g7, 3, nullptr, nullptr},
  {SPK_ANDREA, "Poď. Ak spadneš, chytím ťa.", "Viens. Si tu tombes, je te rattrape.", ep15_dialog_g8, 3, "ak = si. spadneš = tu tombes : tu as déjà entendu spadol si do potoka, « tu es tombé dans le ruisseau ».", nullptr},
  {SPK_FOX, "Ty? Ty si zajac!", "Toi ? Mais tu es un lièvre !", ep15_dialog_g9, 2, nullptr, nullptr},
  {SPK_ANDREA, "Zajačica. A silná.", "Une hase. Et forte.", ep15_dialog_g10, 2, "zajac = le lièvre, zajačica = la hase. Andrea tient beaucoup à la différence.", nullptr},
  {SPK_FOX, "Dobre. Idem.", "D'accord. J'y vais.", ep15_dialog_g11, 2, nullptr, nullptr},
};
static const Gloss ep15_extra_g0[2] = { {"Most", "le pont"}, {"je nad vodou", "est au-dessus de l'eau"} };
static const Gloss ep15_extra_g1[2] = { {"Skala je hore", "la roche est en haut"}, {"voda je dole", "l'eau est en bas"} };
static const Line ep15_extra[2] = {
  {SPK_NONE, "Most je nad vodou.", "Le pont est au-dessus de l'eau.", ep15_extra_g0, 2, nullptr, nullptr},
  {SPK_NONE, "Skala je hore, voda je dole.", "La roche est en haut, l'eau est en bas.", ep15_extra_g1, 2, nullptr, nullptr},
};
static const Word ep15_words[10] = {
  {"roklina", "la gorge (le ravin)"},
  {"rebrík", "l'échelle"},
  {"po rebríkoch", "par les échelles"},
  {"nad vodou", "au-dessus de l'eau"},
  {"dole", "en bas"},
  {"Nemám rád výšky.", "Je n'aime pas les hauteurs."},
  {"oči", "les yeux"},
  {"nápad", "l'idée"},
  {"ak", "si"},
  {"Chytím ťa.", "Je te rattrape."},
};
static const char* const ep15_x0_csk[3] = { "rebrík", "nápad", "oči" };
static const char* const ep15_x0_cfr[3] = { "l'échelle", "l'idée", "les yeux" };
static const char* const ep15_x1_csk[3] = { "roklina", "most", "skala" };
static const char* const ep15_x1_cfr[3] = { "la gorge (le ravin)", "le pont", "la roche, le rocher" };
static const char* const ep15_x2_csk[3] = { "oči", "most", "rebrík" };
static const char* const ep15_x2_cfr[3] = { "les yeux", "le pont", "l'échelle" };
static const char* const ep15_x3_csk[3] = { "dole", "hore", "nad vodou" };
static const char* const ep15_x3_cfr[3] = { "en bas", "en haut", "au-dessus de l'eau" };
static const char* const ep15_x4_csk[3] = { "hore", "nad vodou", "po rebríkoch" };
static const char* const ep15_x4_cfr[3] = { "en haut", "au-dessus de l'eau", "par les échelles" };
static const char* const ep15_x5_cfr[3] = { "Par des échelles ? Moi, j'aime les hauteurs !", "Par des ponts ? Moi, je n'aime pas l'eau !", "Par des échelles ? Toi, tu n'aimes pas les hauteurs !" };
static const char* const ep15_x6_cfr[3] = { "Regarde en bas.", "Ne regarde pas en haut.", "Ne descends pas." };
static const char* const ep15_x7_cfr[3] = { "Toutes mes idées sont bonnes. C'est mon style.", "Toutes tes idées sont mauvaises. C'est ton style.", "Toutes mes idées sont mauvaises. C'est ma faute." };
static const char* const ep15_x8_cfr[3] = { "Viens. Si tu tombes, je rigole.", "Attends. Si tu tombes, je te rattrape.", "Viens. Si je tombe, tu me rattrapes." };
static const char* const ep15_x9_cfr[3] = { "Un lièvre. Et fort.", "Une hase. Et gentille.", "Une renarde. Et forte." };
static const char* const ep15_x10_tsk[3] = { "Myslím.", "Idem prvá.", "Ty ideš za mnou." };
static const char* const ep15_x10_tfr[3] = { "Si.", "Je passe la première.", "Toi, tu me suis." };
static const char* const ep15_x11_tsk[3] = { "Poď.", "Ak spadneš,", "chytím ťa." };
static const char* const ep15_x11_tfr[3] = { "Viens.", "Si tu tombes,", "je te rattrape." };
static const Exercise ep15_exercises[12] = {
  {EX_QCM, "qcm-roklina", "roklina", "la gorge (le ravin)", ep15_x0_csk, ep15_x0_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-rebr-k", "rebrík", "l'échelle", ep15_x1_csk, ep15_x1_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-n-pad", "nápad", "l'idée", ep15_x2_csk, ep15_x2_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-ak", "ak", "si", ep15_x3_csk, ep15_x3_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-dole", "dole", "en bas", ep15_x4_csk, ep15_x4_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-po-rebr-koch-ja-nem-m-r-d-v-ky", "Po rebríkoch? Ja nemám rád výšky!", "Par des échelles ? Moi, je n'aime pas les hauteurs !", nullptr, ep15_x5_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-nepozeraj-dole", "Nepozeraj dole.", "Ne regarde pas en bas.", nullptr, ep15_x6_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-v-etky-moje-n-pady-s-zl-je-to-m-j-t-l", "Všetky moje nápady sú zlé. Je to môj štýl.", "Toutes mes idées sont mauvaises. C'est mon style.", nullptr, ep15_x7_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-po-ak-spadne-chyt-m-a", "Poď. Ak spadneš, chytím ťa.", "Viens. Si tu tombes, je te rattrape.", nullptr, ep15_x8_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-zaja-ica-a-siln", "Zajačica. A silná.", "Une hase. Et forte.", nullptr, ep15_x9_cfr, 3, nullptr, nullptr, 0},
  {EX_ORDER, "order-mysl-m-idem-prv-ty-ide-za-mnou", "Myslím. Idem prvá. Ty ideš za mnou.", "Si. Je passe la première. Toi, tu me suis.", nullptr, nullptr, 0, ep15_x10_tsk, ep15_x10_tfr, 3},
  {EX_ORDER, "order-po-ak-spadne-chyt-m-a", "Poď. Ak spadneš, chytím ťa.", "Viens. Si tu tombes, je te rattrape.", nullptr, nullptr, 0, ep15_x11_tsk, ep15_x11_tfr, 3},
};
static const Line ep15_bonus[1] = {
  {SPK_ANDREA, "Nebojíš sa. Super.", "Tu n'as pas peur. Super.", nullptr, 0, nullptr, nullptr},
};
static const Episode ep15 = { "10_Series_03_Raj_04_rebriky", "Raj", 3, 4, "Rebríky", "Raj",
  ep15_dialog, 12, ep15_extra, 2, ep15_words, 10, ep15_exercises, 12, ep15_bonus, 1 };

static const Gloss ep16_dialog_g0[4] = { {"Na konci rokliny", "au bout de la gorge"}, {"sedí na skale orol", "un aigle est assis sur un rocher"}, {"Je to pán Orol", "c'est monsieur Orol"}, {"strážca parku", "le garde du parc"} };
static const Gloss ep16_dialog_g1[2] = { {"Dobrý deň", "bonjour"}, {"Kam idete", "où allez-vous"} };
static const Gloss ep16_dialog_g2[2] = { {"Dobrý deň", "bonjour"}, {"Hľadáme vodu", "nous cherchons de l'eau"} };
static const Gloss ep16_dialog_g3[4] = { {"Aj tu", "ici aussi"}, {"je vody menej", "il y a moins d'eau"}, {"ako vlani", "que l'an dernier"}, {"Potoky vysychajú", "les ruisseaux s'assèchent"} };
static const Gloss ep16_dialog_g4[2] = { {"Menej ako vlani", "moins que l'an dernier"}, {"Čo sa deje", "qu'est-ce qui se passe"} };
static const Gloss ep16_dialog_g5[5] = { {"Neviem", "je ne sais pas"}, {"V Tatrách", "dans les Tatras"}, {"vraj", "il paraît que"}, {"prší", "il pleut"}, {"Tu nie", "ici, non"} };
static const Gloss ep16_dialog_g6[2] = { {"V Tatrách", "dans les Tatras"}, {"prší", "il pleut"} };
static const Gloss ep16_dialog_g7[5] = { {"Áno", "oui"}, {"V horách", "en montagne"}, {"je počasie iné", "le temps est différent"}, {"Ak chcete vodu", "si vous voulez de l'eau"}, {"choďte tam", "allez là-bas"} };
static const Gloss ep16_dialog_g8[3] = { {"Ďakujeme", "merci"}, {"Pôjdeme", "nous irons"}, {"do Tatier", "dans les Tatras"} };
static const char* const ep16_dialog_c8_items[3] = { "Ďakujeme. Pôjdeme do Tatier.", "Ďakujeme. Opatrne do Tatier.", "Ďakujeme. Vraj do Tatier." };
static const Choice ep16_dialog_c8 = { CH_PICK, "qcm-p-jdeme", ep16_dialog_c8_items, 3 };
static const Gloss ep16_dialog_g9[3] = { {"Opatrne", "prudemment"}, {"A vezmite si", "et prenez"}, {"bundu", "une veste"} };
static const Gloss ep16_dialog_g10[2] = { {"Bundu", "une veste"}, {"V lete", "en été"} };
static const Gloss ep16_dialog_g11[2] = { {"V horách", "en montagne"}, {"je iné leto", "l'été est différent"} };
static const Line ep16_dialog[12] = {
  {SPK_NARRATOR, "Na konci rokliny sedí na skale orol. Je to pán Orol, strážca parku.", "Au bout de la gorge, un aigle est perché sur un rocher. C'est pán Orol, le garde du parc.", ep16_dialog_g0, 4, "Mot nouveau signalé : strážca parku = le garde du parc. orol = l'aigle, et pán Orol, c'est « monsieur Aigle ».", nullptr},
  {SPK_OROL, "Dobrý deň. Kam idete?", "Bonjour. Vous allez où ?", ep16_dialog_g1, 2, nullptr, nullptr},
  {SPK_FOX, "Dobrý deň. Hľadáme vodu.", "Bonjour. Nous cherchons de l'eau.", ep16_dialog_g2, 2, nullptr, nullptr},
  {SPK_OROL, "Aj tu je vody menej ako vlani. Potoky vysychajú.", "Ici aussi, il y a moins d'eau que l'an dernier. Les ruisseaux s'assèchent.", ep16_dialog_g3, 4, "Mots nouveaux signalés : menej ako = moins que, vlani = l'an dernier. Pour comprendre : vysychajú = ils s'assèchent, ils deviennent sucho.", nullptr},
  {SPK_ANDREA, "Menej ako vlani? Čo sa deje?", "Moins que l'an dernier ? Qu'est-ce qui se passe ?", ep16_dialog_g4, 2, "Mot nouveau signalé : Čo sa deje? = qu'est-ce qui se passe ? Une question qui va servir souvent.", nullptr},
  {SPK_OROL, "Neviem. V Tatrách vraj prší. Tu nie.", "Je ne sais pas. Dans les Tatras, il paraît qu'il pleut. Ici, non.", ep16_dialog_g5, 5, "Mot nouveau signalé : vraj = il paraît que, on dit que. prší = il pleut.", nullptr},
  {SPK_ANDREA, "V Tatrách prší?", "Il pleut dans les Tatras ?", ep16_dialog_g6, 2, nullptr, nullptr},
  {SPK_OROL, "Áno. V horách je počasie iné. Ak chcete vodu, choďte tam.", "Oui. En montagne, le temps est différent. Si vous voulez de l'eau, allez là-bas.", ep16_dialog_g7, 5, "Mot nouveau signalé : v horách = en montagne (hory = les montagnes). À écouter : choďte = allez, quand on vouvoie.", nullptr},
  {SPK_FOX, "Ďakujeme. Pôjdeme do Tatier.", "Merci. Nous irons dans les Tatras.", ep16_dialog_g8, 3, "Mot nouveau signalé : pôjdeme = nous irons. C'est le futur de ideme. Et Tatry devient do Tatier : retiens-le d'un bloc.", &ep16_dialog_c8},
  {SPK_OROL, "Opatrne. A vezmite si bundu.", "Soyez prudents. Et prenez une veste.", ep16_dialog_g9, 3, "Mots nouveaux signalés : opatrne = prudemment, bunda = la veste (bundu après « prendre »).", nullptr},
  {SPK_FOX, "Bundu? V lete?", "Une veste ? En plein été ?", ep16_dialog_g10, 2, "Mot nouveau signalé : v lete = en été. leto = l'été.", nullptr},
  {SPK_OROL, "V horách je iné leto.", "En montagne, l'été est différent.", ep16_dialog_g11, 2, nullptr, nullptr},
};
static const char* const ep16_x0_csk[3] = { "v lete", "vraj", "opatrne" };
static const char* const ep16_x0_cfr[3] = { "en été", "il paraît que", "prudemment" };
static const char* const ep16_x1_csk[3] = { "vlani", "opatrne", "menej ako" };
static const char* const ep16_x1_cfr[3] = { "l'an dernier", "prudemment", "moins que" };
static const char* const ep16_x2_csk[3] = { "vraj", "v horách", "vlani" };
static const char* const ep16_x2_cfr[3] = { "il paraît que", "en montagne", "l'an dernier" };
static const char* const ep16_x3_csk[3] = { "strážca parku", "v lete", "pôjdeme" };
static const char* const ep16_x3_cfr[3] = { "le garde du parc", "en été", "nous irons" };
static const char* const ep16_x4_csk[3] = { "opatrne", "vraj", "v horách" };
static const char* const ep16_x4_cfr[3] = { "prudemment", "il paraît que", "en montagne" };
static const char* const ep16_x5_cfr[3] = { "Ici aussi, il y a plus d'eau que l'an dernier. Les ruisseaux débordent.", "Ici aussi, il y a moins d'eau qu'hier. Les ruisseaux s'assèchent.", "Ici, il y a beaucoup d'eau. Les ruisseaux s'assèchent ailleurs." };
static const char* const ep16_x6_cfr[3] = { "Plus que l'an dernier ? Qu'est-ce qui se passe ?", "Moins que l'an dernier ? Qu'est-ce qu'on fait ?", "Moins qu'hier ? Qu'est-ce qui se passe ?" };
static const char* const ep16_x7_cfr[3] = { "Je ne sais pas. Dans les Tatras, il paraît qu'il fait sec. Ici, non.", "Je sais. Dans les Tatras, il pleut. Ici aussi.", "Je ne sais pas. Ici, il paraît qu'il pleut. Dans les Tatras, non." };
static const char* const ep16_x8_cfr[3] = { "Merci. Nous sommes dans les Tatras.", "Merci. Nous irons au village.", "Merci. Vous irez dans les Tatras." };
static const char* const ep16_x9_cfr[3] = { "Une veste ? En hiver ?", "Un sac à dos ? En plein été ?", "Une bouteille ? En plein été ?" };
static const char* const ep16_x10_tsk[3] = { "Opatrne.", "A vezmite si", "bundu." };
static const char* const ep16_x10_tfr[3] = { "Soyez prudents.", "Et prenez", "une veste." };
static const char* const ep16_x11_tsk[3] = { "Neviem.", "V Tatrách vraj prší.", "Tu nie." };
static const char* const ep16_x11_tfr[3] = { "Je ne sais pas.", "Dans les Tatras, il paraît qu'il pleut.", "Ici, non." };
static const Exercise ep16_exercises[12] = {
  {EX_QCM, "qcm-vlani", "vlani", "l'an dernier", ep16_x0_csk, ep16_x0_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-vraj", "vraj", "il paraît que", ep16_x1_csk, ep16_x1_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-opatrne", "opatrne", "prudemment", ep16_x2_csk, ep16_x2_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-bunda", "bunda", "la veste", ep16_x3_csk, ep16_x3_cfr, 3, nullptr, nullptr, 0},
  {EX_QCM, "qcm-p-jdeme", "pôjdeme", "nous irons", ep16_x4_csk, ep16_x4_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-aj-tu-je-vody-menej-ako-vlani-potoky-vysychaj", "Aj tu je vody menej ako vlani. Potoky vysychajú.", "Ici aussi, il y a moins d'eau que l'an dernier. Les ruisseaux s'assèchent.", nullptr, ep16_x5_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-menej-ako-vlani-o-sa-deje", "Menej ako vlani? Čo sa deje?", "Moins que l'an dernier ? Qu'est-ce qui se passe ?", nullptr, ep16_x6_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-neviem-v-tatr-ch-vraj-pr-tu-nie", "Neviem. V Tatrách vraj prší. Tu nie.", "Je ne sais pas. Dans les Tatras, il paraît qu'il pleut. Ici, non.", nullptr, ep16_x7_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-akujeme-p-jdeme-do-tatier", "Ďakujeme. Pôjdeme do Tatier.", "Merci. Nous irons dans les Tatras.", nullptr, ep16_x8_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-bundu-v-lete", "Bundu? V lete?", "Une veste ? En plein été ?", nullptr, ep16_x9_cfr, 3, nullptr, nullptr, 0},
  {EX_ORDER, "order-opatrne-a-vezmite-si-bundu", "Opatrne. A vezmite si bundu.", "Soyez prudents. Et prenez une veste.", nullptr, nullptr, 0, ep16_x10_tsk, ep16_x10_tfr, 3},
  {EX_ORDER, "order-neviem-v-tatr-ch-vraj-pr-tu-nie", "Neviem. V Tatrách vraj prší. Tu nie.", "Je ne sais pas. Dans les Tatras, il paraît qu'il pleut. Ici, non.", nullptr, nullptr, 0, ep16_x11_tsk, ep16_x11_tfr, 3},
};
static const Line ep16_bonus[1] = {
  {SPK_OROL, "Hovoríte veľmi pekne.", "Vous parlez très bien.", nullptr, 0, nullptr, nullptr},
};
static const Episode ep16 = { "10_Series_03_Raj_05_roklina", "Raj", 3, 5, "Na konci rokliny", "Raj",
  ep16_dialog, 12, nullptr, 0, nullptr, 0, ep16_exercises, 12, ep16_bonus, 1 };

static const Gloss ep17_dialog_g0[3] = { {"Je večer", "c'est le soir"}, {"sedia", "sont assis"}, {"pri potoku", "près du ruisseau"} };
static const Gloss ep17_dialog_g1[3] = { {"Nikdy", "jamais"}, {"som nespal", "je n'ai pas dormi"}, {"pod hviezdami", "sous les étoiles"} };
static const Gloss ep17_dialog_g2[2] = { {"Spal si", "tu as dormi"}, {"pod jabloňou", "sous un pommier"} };
static const Gloss ep17_dialog_g3[4] = { {"To nie sú", "ce ne sont pas"}, {"hviezdy", "des étoiles"}, {"To sú", "ce sont"}, {"jablká", "des pommes"} };
static const char* const ep17_dialog_c3_items[3] = { "To nie sú", "hviezdy.", "To sú jablká." };
static const Choice ep17_dialog_c3 = { CH_ORDER, "order-to-nie-s-hviezdy-to-s-jablk", ep17_dialog_c3_items, 3 };
static const Gloss ep17_dialog_g4[1] = { {"Spi", "dors"} };
static const Gloss ep17_dialog_g7[3] = { {"Ty", "toi"}, {"nikdy neveríš", "tu ne crois jamais"}, {"líškam", "aux renards"} };
static const Gloss ep17_dialog_g8[2] = { {"Nikdy", "jamais"}, {"Okrem jednej", "sauf une"} };
static const Gloss ep17_dialog_g9[2] = { {"Okrem jednej", "sauf une"}, {"Ktorej", "laquelle"} };
static const char* const ep17_dialog_c9_items[3] = { "Okrem", "jednej?", "Ktorej?" };
static const Choice ep17_dialog_c9 = { CH_ORDER, "order-okrem-jednej-ktorej", ep17_dialog_c9_items, 3 };
static const Gloss ep17_dialog_g10[1] = { {"Spi", "dors"} };
static const Gloss ep17_dialog_g11[2] = { {"Ja viem", "je sais"}, {"že som to ja", "que c'est moi"} };
static const Gloss ep17_dialog_g12[2] = { {"Nič nevieš", "tu ne sais rien"}, {"Spi", "dors"} };
static const Line ep17_dialog[13] = {
  {SPK_NARRATOR, "Je večer. \001 a Andrea sedia pri potoku.", "C'est le soir. \001 et Andrea sont assis près du ruisseau.", ep17_dialog_g0, 3, nullptr, nullptr},
  {SPK_FOX, "Nikdy som nespal pod hviezdami.", "Je n'ai jamais dormi sous les étoiles.", ep17_dialog_g1, 3, "Mots nouveaux signalés : nikdy = jamais, hviezdy = les étoiles. Encore deux négations : nikdy et ne-.", nullptr},
  {SPK_ANDREA, "Spal si pod jabloňou.", "Tu as dormi sous un pommier.", ep17_dialog_g2, 2, nullptr, nullptr},
  {SPK_FOX, "To nie sú hviezdy. To sú jablká.", "Ce ne sont pas des étoiles. Ce sont des pommes.", ep17_dialog_g3, 4, "Mot nouveau signalé : jablká = des pommes. jablko = une pomme, et jabloň = l'arbre qui les porte.", &ep17_dialog_c3},
  {SPK_ANDREA, "Spi, \001.", "Dors, \001.", ep17_dialog_g4, 1, "spať = dormir → Spi! = dors !", nullptr},
  {SPK_FOX, "Andrea?", "Andrea ?", nullptr, 0, nullptr, nullptr},
  {SPK_ANDREA, "Hm?", "Hm ?", nullptr, 0, nullptr, nullptr},
  {SPK_FOX, "Ty nikdy neveríš líškam?", "Toi, tu ne fais jamais confiance aux renards ?", ep17_dialog_g7, 3, "Mot nouveau signalé : veriť = croire, faire confiance. neveríš = tu ne fais pas confiance.", nullptr},
  {SPK_ANDREA, "Nikdy. Okrem jednej.", "Jamais. Sauf à un.", ep17_dialog_g8, 2, "Mot nouveau signalé : okrem = sauf. jednej est au féminin, parce que líška est un mot féminin.", nullptr},
  {SPK_FOX, "Okrem jednej? Ktorej?", "Sauf à un ? Lequel ?", ep17_dialog_g9, 2, nullptr, &ep17_dialog_c9},
  {SPK_ANDREA, "Spi.", "Dors.", ep17_dialog_g10, 1, nullptr, nullptr},
  {SPK_FOX, "Ja viem, že som to ja.", "Je sais que c'est moi.", ep17_dialog_g11, 2, nullptr, nullptr},
  {SPK_ANDREA, "Nič nevieš. Spi.", "Tu ne sais rien. Dors.", ep17_dialog_g12, 2, nullptr, nullptr},
};
static const char* const ep17_x0_cfr[3] = { "J'ai toujours dormi sous les étoiles.", "Je n'ai jamais dormi sous un pommier.", "Tu n'as jamais dormi sous les étoiles." };
static const char* const ep17_x1_cfr[3] = { "Ce sont des étoiles. Ce ne sont pas des pommes.", "Ce ne sont pas des pommes. Ce sont des étoiles.", "Ce ne sont pas des oiseaux. Ce sont des pommes." };
static const char* const ep17_x2_cfr[3] = { "Toi, tu fais toujours confiance aux renards ?", "Toi, tu n'as jamais peur des renards ?", "Toi, tu ne parles jamais aux renards ?" };
static const char* const ep17_x3_cfr[3] = { "Toujours. Sauf à un.", "Jamais. Même pas à un.", "Jamais. Sauf à deux." };
static const char* const ep17_x4_cfr[3] = { "Je sais que c'est toi.", "Je ne sais pas si c'est moi.", "Tu sais que c'est moi." };
static const char* const ep17_x5_cfr[3] = { "Tu sais tout. Dors.", "Je ne sais rien. Dors.", "Tu ne sais rien. Viens." };
static const char* const ep17_x6_tsk[3] = { "To nie sú", "hviezdy.", "To sú jablká." };
static const char* const ep17_x6_tfr[3] = { "Ce ne sont pas", "des étoiles.", "Ce sont des pommes." };
static const char* const ep17_x7_tsk[3] = { "Okrem", "jednej?", "Ktorej?" };
static const char* const ep17_x7_tfr[3] = { "Sauf", "à un ?", "Lequel ?" };
static const Exercise ep17_exercises[8] = {
  {EX_LISTEN, "listen-nikdy-som-nespal-pod-hviezdami", "Nikdy som nespal pod hviezdami.", "Je n'ai jamais dormi sous les étoiles.", nullptr, ep17_x0_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-to-nie-s-hviezdy-to-s-jablk", "To nie sú hviezdy. To sú jablká.", "Ce ne sont pas des étoiles. Ce sont des pommes.", nullptr, ep17_x1_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-ty-nikdy-never-l-kam", "Ty nikdy neveríš líškam?", "Toi, tu ne fais jamais confiance aux renards ?", nullptr, ep17_x2_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-nikdy-okrem-jednej", "Nikdy. Okrem jednej.", "Jamais. Sauf à un.", nullptr, ep17_x3_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-ja-viem-e-som-to-ja", "Ja viem, že som to ja.", "Je sais que c'est moi.", nullptr, ep17_x4_cfr, 3, nullptr, nullptr, 0},
  {EX_LISTEN, "listen-ni-nevie-spi", "Nič nevieš. Spi.", "Tu ne sais rien. Dors.", nullptr, ep17_x5_cfr, 3, nullptr, nullptr, 0},
  {EX_ORDER, "order-to-nie-s-hviezdy-to-s-jablk", "To nie sú hviezdy. To sú jablká.", "Ce ne sont pas des étoiles. Ce sont des pommes.", nullptr, nullptr, 0, ep17_x6_tsk, ep17_x6_tfr, 3},
  {EX_ORDER, "order-okrem-jednej-ktorej", "Okrem jednej? Ktorej?", "Sauf à un ? Lequel ?", nullptr, nullptr, 0, ep17_x7_tsk, ep17_x7_tfr, 3},
};
static const Line ep17_bonus[1] = {
  {SPK_ANDREA, "Dobrú noc, líška.", "Bonne nuit, renard.", nullptr, 0, nullptr, nullptr},
};
static const Episode ep17 = { "10_Series_03_Raj_06_extra", "Raj", 3, 6, "Pri potoku", "Raj",
  ep17_dialog, 13, nullptr, 0, nullptr, 0, ep17_exercises, 8, ep17_bonus, 1 };

const Episode* const COURSE_EPISODES[] = { &ep0, &ep1, &ep2, &ep3, &ep4, &ep5, &ep6, &ep7, &ep8, &ep9, &ep10, &ep11, &ep12, &ep13, &ep14, &ep15, &ep16, &ep17 };
const Word COURSE_VOCAB[200] = {
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
  {"zastávka", "l'arrêt"},
  {"lístok", "le ticket"},
  {"mesto", "la ville"},
  {"Ako sa máš?", "Comment vas-tu ? (rappel)"},
  {"Mám sa dobre.", "Je vais bien. (rappel)"},
  {"super", "super, génial"},
  {"A ty?", "Et toi ?"},
  {"aj ja", "moi aussi"},
  {"rád", "content (dit par un garçon)"},
  {"rada", "contente (dit par une fille)"},
  {"ťa", "te"},
  {"vidím", "je vois"},
  {"dlho", "longtemps"},
  {"čau", "salut (entre ados)"},
  {"fajn", "ok, bien"},
  {"skvelé", "génial"},
  {"čo", "quoi, que"},
  {"robím", "je fais"},
  {"robíš", "tu fais"},
  {"bývam", "j'habite"},
  {"bývaš", "tu habites"},
  {"v Lyone", "à Lyon"},
  {"na prázdninách", "en vacances"},
  {"sám", "seul (dit par un garçon)"},
  {"vie", "il sait, elle sait"},
  {"asi", "peut-être, je crois"},
  {"cestujem", "je voyage"},
  {"vlak", "le train"},
  {"vlakom", "en train"},
  {"jedenásť", "onze"},
  {"trinásť", "treize"},
  {"štrnásť", "quatorze"},
  {"pätnásť", "quinze"},
  {"šestnásť", "seize"},
  {"pamätám sa", "je me souviens"},
  {"pamätáš sa", "tu te souviens"},
  {"všetko", "tout"},
  {"milý", "gentil (dit d'un garçon)"},
  {"teraz", "maintenant"},
  {"dóm", "la cathédrale"},
  {"kostol", "l'église"},
  {"gymnázium", "le lycée"},
  {"veľa", "beaucoup"},
  {"rozprávajú", "ils parlent"},
  {"komplikované", "compliqués"},
  {"mimozemšťania", "les extraterrestres"},
  {"kamarát", "le copain, l'ami"},
  {"jednoduchý", "simple (dit d'un garçon)"},
  {"skoro", "presque"},
  {"u mojej tety", "chez ma tante"},
  {"pravda", "vrai, la vérité"},
  {"večer", "le soir (nouveau)"},
  {"záhrada", "le jardin (nouveau)"},
  {"teta", "la tante (nouveau)"},
  {"silná", "forte (dit d'une fille) (nouveau)"},
  {"bojím sa", "j'ai peur (nouveau)"},
  {"spať", "dormir (nouveau)"},
  {"jabloň", "le pommier (nouveau)"},
  {"uvidíme", "on verra (nouveau)"},
  {"Dobrú noc", "bonne nuit (nouveau)"},
  {"Aké je počasie?", "Quel temps fait-il ?"},
  {"dnes", "aujourd'hui"},
  {"horúco", "chaud"},
  {"sucho", "sec"},
  {"slnko", "le soleil"},
  {"vstávaj", "lève-toi, debout"},
  {"jazierko", "le petit étang"},
  {"v parku", "dans le parc"},
  {"dva mesiace", "deux mois"},
  {"každé ráno", "tous les matins"},
  {"teplo", "doux, chaud"},
  {"zima", "froid"},
  {"oblak", "le nuage"},
  {"veľmi", "très (rappel)"},
  {"málo", "peu"},
  {"hovorím", "je parle"},
  {"rozumiem", "je comprends"},
  {"správne", "correct, juste"},
  {"neexistuje", "n'existe pas"},
  {"líščí", "de renard"},
  {"chvost", "la queue"},
  {"presne", "exactement"},
  {"trochu", "un peu"},
  {"viac", "plus"},
  {"menej", "moins"},
  {"chýba mi", "il me manque"},
  {"chýba ti", "il te manque"},
  {"pijem", "je bois (rappel)"},
  {"hľadám", "je cherche"},
  {"včera", "hier"},
  {"ešte", "encore"},
  {"aspoň", "au moins"},
  {"zvieratá", "les animaux"},
  {"nikto", "personne"},
  {"zlé", "mauvais"},
  {"potok", "le ruisseau"},
  {"jazero", "le lac"},
  {"kvapka", "la goutte"},
  {"budeme", "nous allons (faire)"},
  {"ideme", "nous allons (quelque part)"},
  {"inde", "ailleurs"},
  {"na západe", "à l'ouest"},
  {"ďaleko", "loin"},
  {"škola", "l'école"},
  {"len", "seulement"},
  {"preč", "parti, loin d'ici"},
  {"spí", "il dort, elle dort"},
  {"batoh", "le sac à dos"},
  {"mapa", "la carte"},
  {"zajtra", "demain"},
  {"fontána", "la fontaine"},
  {"prázdna", "vide (dit d'une fontaine)"},
  {"záhradník", "le jardinier"},
  {"hľadáte", "vous cherchez"},
  {"prepáčte", "excusez-moi (à un adulte)"},
  {"nikde", "nulle part"},
  {"v rádiu", "à la radio"},
  {"Veľa šťastia!", "Bonne chance !"},
  {"stanica", "la gare (nouveau)"},
  {"na stanici", "à la gare (nouveau)"},
  {"musím", "je dois (nouveau)"},
  {"lístky", "des billets (nouveau)"},
  {"Neboj sa!", "Ne t'inquiète pas ! (nouveau)"},
  {"pani", "la dame (nouveau)"},
  {"kam", "où (vers où) (nouveau)"},
  {"na vlak", "pour le train (nouveau)"},
  {"Šťastnú cestu!", "Bon voyage ! (nouveau)"},
  {"Kam teraz?", "On va où maintenant ?"},
  {"Poď za mnou!", "Suis-moi !"},
  {"vždy", "toujours"},
  {"otázky", "des questions"},
  {"pomaly", "lentement"},
  {"rýchlo", "vite"},
  {"Ja viem.", "Je sais."},
  {"tak", "alors"},
  {"les", "la forêt"},
  {"cez", "à travers"},
  {"tieň", "l'ombre"},
  {"počujem", "j'entends"},
  {"počuješ", "tu entends"},
  {"ticho", "le silence, chut"},
  {"nič", "rien"},
  {"vtáky", "les oiseaux"},
  {"dobré", "bon, bien"},
  {"niekde", "quelque part"},
  {"blízko", "près (rappel)"},
  {"tečie", "coule"},
  {"vták", "l'oiseau"},
  {"spieva", "chante"},
  {"hlasno", "fort (pour un son)"},
  {"dosť", "assez"},
  {"To mi stačí.", "Ça me suffit."},
  {"pre dvoch", "pour deux"},
  {"najprv", "d'abord"},
  {"potom", "ensuite"},
  {"Napi sa!", "Bois !"},
  {"zase", "encore, de nouveau"},
  {"šéf", "le chef"},
  {"takže", "donc"},
  {"smäd", "la soif"},
  {"fľaša", "la bouteille"},
  {"piť", "boire"},
  {"roklina", "la gorge (le ravin)"},
  {"rebrík", "l'échelle"},
  {"po rebríkoch", "par les échelles"},
  {"nad vodou", "au-dessus de l'eau"},
  {"dole", "en bas"},
  {"Nemám rád výšky.", "Je n'aime pas les hauteurs."},
  {"oči", "les yeux"},
  {"nápad", "l'idée"},
  {"ak", "si"},
  {"Chytím ťa.", "Je te rattrape."},
  {"most", "le pont"},
  {"skala", "la roche, le rocher"},
  {"hore", "en haut"},
  {"strážca parku", "le garde du parc"},
  {"menej ako", "moins que"},
  {"vlani", "l'an dernier"},
  {"Čo sa deje?", "Qu'est-ce qui se passe ?"},
  {"vraj", "il paraît que"},
  {"v horách", "en montagne"},
  {"pôjdeme", "nous irons"},
  {"opatrne", "prudemment"},
  {"bunda", "la veste"},
  {"v lete", "en été"},
  {"nikdy", "jamais (nouveau)"},
  {"hviezdy", "les étoiles (nouveau)"},
  {"jablká", "des pommes (nouveau)"},
  {"veriť", "croire, faire confiance (nouveau)"},
  {"okrem", "sauf (nouveau)"},
};
const int COURSE_NUM_VOCAB = 200;
static const char* const gl0_c1_items[3] = { "Lístok na električku, prosím.", "Mapu mesta, prosím.", "Vodu, prosím." };
static const Choice gl0_c1 = { CH_PICK, "lines", gl0_c1_items, 3 };
static const Line gl0[3] = {
  {SPK_HUMAN, "Dobrý deň. Čo si želáte?", "Bonjour. Que désirez-vous ?", nullptr, 0, nullptr, nullptr},
  {SPK_FOX, "Lístok na električku, prosím.", "Un ticket de tram, s'il vous plaît.", nullptr, 0, nullptr, &gl0_c1},
  {SPK_HUMAN, "Nech sa páči. Jeden lístok.", "Voilà. Un ticket.", nullptr, 0, nullptr, nullptr},
};
static const Line gl1[1] = {
  {SPK_HUMAN, "Električka stojí na zastávke. Tam, za fontánou.", "Le tram attend à l'arrêt. Là-bas, après la fontaine.", nullptr, 0, nullptr, nullptr},
};
static const Line gl2[3] = {
  {SPK_HUMAN, "Dobrý deň. Máte lístok?", "Bonjour. Vous avez un ticket ?", nullptr, 0, nullptr, nullptr},
  {SPK_FOX, "Nie...", "Non...", nullptr, 0, nullptr, nullptr},
  {SPK_HUMAN, "Lístky predávajú v trafike. Tam, pri lavičke.", "On vend les tickets au kiosque. Là-bas, près du banc.", nullptr, 0, nullptr, nullptr},
};
static const char* const gl3_c1_items[3] = { "Áno. To je môj lístok.", "Áno. To je môj vlak.", "Áno. To je môj kostol." };
static const Choice gl3_c1 = { CH_PICK, "lines", gl3_c1_items, 3 };
static const Line gl3[3] = {
  {SPK_HUMAN, "Dobrý deň. Máte lístok?", "Bonjour. Vous avez un ticket ?", nullptr, 0, nullptr, nullptr},
  {SPK_FOX, "Áno. To je môj lístok.", "Oui. C'est mon ticket.", nullptr, 0, nullptr, &gl3_c1},
  {SPK_HUMAN, "Nastúpte, prosím.", "Montez, s'il vous plaît.", nullptr, 0, nullptr, nullptr},
};
static const Line gl4[1] = {
  {SPK_NARRATOR, "Cvak! Lístok je označený.", "Clac ! Le ticket est composté.", nullptr, 0, nullptr, nullptr},
};
static const Line gl5[1] = {
  {SPK_NARRATOR, "Lístok je už označený.", "Le ticket est déjà composté.", nullptr, 0, nullptr, nullptr},
};
static const Line gl6[1] = {
  {SPK_NARRATOR, "Električka stojí. Nastupuje revízor.", "Le tram s'arrête. Un contrôleur monte.", nullptr, 0, nullptr, nullptr},
};
static const Line gl7[1] = {
  {SPK_ANDREA, "Pozor, revízor! Máš lístok?", "Attention, le contrôleur ! Tu as un ticket ?", nullptr, 0, nullptr, nullptr},
};
static const Line gl8[3] = {
  {SPK_HUMAN, "Dobrý deň, kontrola lístkov.", "Bonjour, contrôle des tickets.", nullptr, 0, nullptr, nullptr},
  {SPK_FOX, "To je môj lístok.", "C'est mon ticket.", nullptr, 0, nullptr, nullptr},
  {SPK_HUMAN, "Lístok nie je označený. Označte si ho, prosím.", "Le ticket n'est pas composté. Compostez-le, s'il vous plaît.", nullptr, 0, "Le composteur jaune est près de la porte du tram.", nullptr},
};
static const char* const gl9_c1_items[3] = { "To je môj lístok.", "To je môj kamarát.", "To je moja zastávka." };
static const Choice gl9_c1 = { CH_PICK, "lines", gl9_c1_items, 3 };
static const Line gl9[3] = {
  {SPK_HUMAN, "Dobrý deň, kontrola lístkov.", "Bonjour, contrôle des tickets.", nullptr, 0, nullptr, nullptr},
  {SPK_FOX, "To je môj lístok.", "C'est mon ticket.", nullptr, 0, nullptr, &gl9_c1},
  {SPK_HUMAN, "Ďakujem, je to v poriadku.", "Merci, c'est en ordre.", nullptr, 0, nullptr, nullptr},
};
static const Line gl10[1] = {
  {SPK_HUMAN, "Príjemnú cestu.", "Bon voyage.", nullptr, 0, nullptr, nullptr},
};
static const Line gl11[1] = {
  {SPK_NONE, "Kúp si lístok v trafike.", "Achète un ticket au kiosque.", nullptr, 0, nullptr, nullptr},
};
static const Line gl12[1] = {
  {SPK_NONE, "Choď na zastávku a nastúp do električky.", "Va à l'arrêt et monte dans le tram.", nullptr, 0, nullptr, nullptr},
};
static const Line gl13[1] = {
  {SPK_NONE, "Porozprávaj sa s Andreou.", "Parle avec Andrea.", nullptr, 0, nullptr, nullptr},
};
static const Line gl14[1] = {
  {SPK_NONE, "Označ si lístok a ukáž ho revízorovi.", "Composte ton ticket et montre-le au contrôleur.", nullptr, 0, nullptr, nullptr},
};
static const Line gl15[1] = {
  {SPK_NONE, "Choď spať pod jabloň.", "Va dormir sous le pommier.", nullptr, 0, nullptr, nullptr},
};
static const Line gl16[1] = {
  {SPK_NONE, "Kapitola je hotová.", "Le chapitre est terminé.", nullptr, 0, nullptr, nullptr},
};
static const Line gl17[1] = {
  {SPK_ANDREA, "Najprv do parku. Tam je jazierko.", "D'abord le parc. Il y a l'étang.", nullptr, 0, nullptr, nullptr},
};
static const Line gl18[1] = {
  {SPK_ANDREA, "Zvieratá sú smädné. Hľadajme kvapky vody.", "Les animaux ont soif. Cherchons des gouttes d'eau.", nullptr, 0, nullptr, nullptr},
};
static const Line gl19[1] = {
  {SPK_MOUSE, "Voda... Máš vodu?", "De l'eau... Tu as de l'eau ?", nullptr, 0, nullptr, nullptr},
};
static const Line gl20[1] = {
  {SPK_MOUSE, "To je málo. Ešte, prosím.", "C'est peu. Encore, s'il te plaît.", nullptr, 0, nullptr, nullptr},
};
static const char* const gl21_c0_items[3] = { "Tu je voda. Pre všetkých.", "Tu je mapa. Pre všetkých.", "Tu je vlak. Pre všetkých." };
static const Choice gl21_c0 = { CH_PICK, "lines", gl21_c0_items, 3 };
static const Line gl21[3] = {
  {SPK_FOX, "Tu je voda. Pre všetkých.", "Voilà de l'eau. Pour tout le monde.", nullptr, 0, nullptr, &gl21_c0},
  {SPK_MOUSE, "Ďakujeme!", "Merci !", nullptr, 0, nullptr, nullptr},
  {SPK_CHICK, "Ďakujeme!", "Merci !", nullptr, 0, nullptr, nullptr},
};
static const Line gl22[1] = {
  {SPK_MOUSE, "Ďakujeme za vodu.", "Merci pour l'eau.", nullptr, 0, nullptr, nullptr},
};
static const Line gl23[1] = {
  {SPK_ANDREA, "Najprv k fontáne. Poď!", "D'abord la fontaine. Viens !", nullptr, 0, nullptr, nullptr},
};
static const Line gl24[1] = {
  {SPK_ANDREA, "Počkaj, najprv lístky!", "Attends, d'abord les billets !", nullptr, 0, nullptr, nullptr},
};
static const char* const gl25_c1_items[4] = { "Teta,", "odišla som", "na pár dní.", "Neboj sa." };
static const Choice gl25_c1 = { CH_ORDER, "lines", gl25_c1_items, 4 };
static const Line gl25[2] = {
  {SPK_ANDREA, "Pomôžeš mi? Píšem tete lístok.", "Tu m'aides ? J'écris un mot à ma tante.", nullptr, 0, nullptr, nullptr},
  {SPK_ANDREA, "Teta, odišla som na pár dní. Neboj sa.", "Tata, je suis partie pour quelques jours. Ne t'inquiète pas.", nullptr, 0, "Le mot qu'Andrea laisse à sa tante : remets les morceaux dans l'ordre.", &gl25_c1},
};
static const Line gl26[1] = {
  {SPK_HUMAN, "Nastupovať, prosím!", "En voiture, s'il vous plaît !", nullptr, 0, nullptr, nullptr},
};
static const Line gl27[1] = {
  {SPK_NONE, "Choď s Andreou do parku.", "Va au parc avec Andrea.", nullptr, 0, nullptr, nullptr},
};
static const Line gl28[1] = {
  {SPK_NONE, "Nájdi päť kvapiek vody pre zvieratá.", "Trouve cinq gouttes d'eau pour les animaux.", nullptr, 0, nullptr, nullptr},
};
static const Line gl29[1] = {
  {SPK_NONE, "Choď k fontáne v centre mesta.", "Va à la fontaine du centre-ville.", nullptr, 0, nullptr, nullptr},
};
static const Line gl30[1] = {
  {SPK_NONE, "Choď na železničnú stanicu.", "Va à la gare.", nullptr, 0, nullptr, nullptr},
};
static const Line gl31[1] = {
  {SPK_NONE, "Kúpte si lístky pri okienku.", "Achetez vos billets au guichet.", nullptr, 0, nullptr, nullptr},
};
static const Line gl32[1] = {
  {SPK_NONE, "Nastúp do vlaku.", "Monte dans le train.", nullptr, 0, nullptr, nullptr},
};
static const Line gl33[1] = {
  {SPK_BIRD, "...", "...", nullptr, 0, nullptr, nullptr},
};
static const Line gl34[1] = {
  {SPK_ANDREA, "Najprv nájdi potok. Počuješ?", "D'abord, trouve le ruisseau. Tu entends ?", nullptr, 0, nullptr, nullptr},
};
static const Line gl35[1] = {
  {SPK_ANDREA, "Najprv sa napi.", "D'abord, bois un coup.", nullptr, 0, nullptr, nullptr},
};
static const Line gl36[1] = {
  {SPK_NARRATOR, "Napil si sa z potoka. Voda je dobrá.", "Tu as bu au ruisseau. L'eau est bonne.", nullptr, 0, nullptr, nullptr},
};
static const Line gl37[1] = {
  {SPK_ANDREA, "Pozor na dosky! Každá vydrží len jeden krok.", "Attention aux planches ! Chacune ne tient qu'un seul pas.", nullptr, 0, "Une planche cède dès que tu la quittes. Prévois ton chemin avant de marcher.", nullptr},
};
static const Line gl38[1] = {
  {SPK_ANDREA, "Nepozeraj dole! Poď zase.", "Ne regarde pas en bas ! Recommence.", nullptr, 0, nullptr, nullptr},
};
static const Line gl39[1] = {
  {SPK_ANDREA, "Poď sem!", "Viens ici !", nullptr, 0, nullptr, nullptr},
};
static const Line gl40[1] = {
  {SPK_ANDREA, "Najprv pán Orol. Poď!", "D'abord monsieur Orol. Viens !", nullptr, 0, nullptr, nullptr},
};
static const Line gl41[1] = {
  {SPK_NONE, "Choď cez les.", "Traverse la forêt.", nullptr, 0, nullptr, nullptr},
};
static const Line gl42[1] = {
  {SPK_NONE, "Nájdi potok. Počuješ?", "Trouve le ruisseau. Tu entends ?", nullptr, 0, nullptr, nullptr},
};
static const Line gl43[1] = {
  {SPK_NONE, "Napi sa z potoka.", "Bois au ruisseau.", nullptr, 0, nullptr, nullptr},
};
static const Line gl44[1] = {
  {SPK_NONE, "Choď do rokliny.", "Va à la gorge.", nullptr, 0, nullptr, nullptr},
};
static const Line gl45[1] = {
  {SPK_NONE, "Prejdi cez dosky. Nevracaj sa.", "Traverse les planches. Ne reviens pas en arrière.", nullptr, 0, nullptr, nullptr},
};
static const Line gl46[1] = {
  {SPK_NONE, "Porozprávaj sa s pánom Orlom.", "Va parler à monsieur Orol.", nullptr, 0, nullptr, nullptr},
};
static const Line gl47[1] = {
  {SPK_NONE, "Choď k potoku. Je večer.", "Va au ruisseau. C'est le soir.", nullptr, 0, nullptr, nullptr},
};
const Script SCRIPTS[48] = { {gl0, 3}, {gl1, 1}, {gl2, 3}, {gl3, 3}, {gl4, 1}, {gl5, 1}, {gl6, 1}, {gl7, 1}, {gl8, 3}, {gl9, 3}, {gl10, 1}, {gl11, 1}, {gl12, 1}, {gl13, 1}, {gl14, 1}, {gl15, 1}, {gl16, 1}, {gl17, 1}, {gl18, 1}, {gl19, 1}, {gl20, 1}, {gl21, 3}, {gl22, 1}, {gl23, 1}, {gl24, 1}, {gl25, 2}, {gl26, 1}, {gl27, 1}, {gl28, 1}, {gl29, 1}, {gl30, 1}, {gl31, 1}, {gl32, 1}, {gl33, 1}, {gl34, 1}, {gl35, 1}, {gl36, 1}, {gl37, 1}, {gl38, 1}, {gl39, 1}, {gl40, 1}, {gl41, 1}, {gl42, 1}, {gl43, 1}, {gl44, 1}, {gl45, 1}, {gl46, 1}, {gl47, 1} };
const int NUM_SCRIPTS = 48;
const int COURSE_NUM_EPISODES = 18;
const char* const COURSE_DEFAULT_NAME = "Eric";
const char* const SPEAKER_NAMES[SPK_COUNT] = { "", "", "\001", "Andrea", "Maťo", "Katka", "Babka Zuzana", "Pán Orol", "Pani Ježková", "", "Ján", "Marek", "", "", "" };
