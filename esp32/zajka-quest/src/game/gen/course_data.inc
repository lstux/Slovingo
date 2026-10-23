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

const Episode* const COURSE_EPISODES[] = { &ep0, &ep1, &ep2, &ep3, &ep4, &ep5, &ep6, &ep7, &ep8, &ep9, &ep10, &ep11 };
const Word COURSE_VOCAB[137] = {
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
};
const int COURSE_NUM_VOCAB = 137;
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
const Script SCRIPTS[33] = { {gl0, 3}, {gl1, 1}, {gl2, 3}, {gl3, 3}, {gl4, 1}, {gl5, 1}, {gl6, 1}, {gl7, 1}, {gl8, 3}, {gl9, 3}, {gl10, 1}, {gl11, 1}, {gl12, 1}, {gl13, 1}, {gl14, 1}, {gl15, 1}, {gl16, 1}, {gl17, 1}, {gl18, 1}, {gl19, 1}, {gl20, 1}, {gl21, 3}, {gl22, 1}, {gl23, 1}, {gl24, 1}, {gl25, 2}, {gl26, 1}, {gl27, 1}, {gl28, 1}, {gl29, 1}, {gl30, 1}, {gl31, 1}, {gl32, 1} };
const int NUM_SCRIPTS = 33;
const int COURSE_NUM_EPISODES = 12;
const char* const COURSE_DEFAULT_NAME = "Eric";
const char* const SPEAKER_NAMES[SPK_COUNT] = { "", "", "\001", "Andrea", "Maťo", "Katka", "Babka Zuzana", "Pán Orol", "Pani Ježková", "", "Ján", "Marek", "", "" };
