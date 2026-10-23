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

const Episode* const COURSE_EPISODES[] = { &ep0, &ep1, &ep2, &ep3, &ep4, &ep5 };
const int COURSE_NUM_EPISODES = 6;
const char* const COURSE_DEFAULT_NAME = "Eric";
const char* const SPEAKER_NAMES[SPK_COUNT] = { "", "", "\001", "Andrea", "Maťo", "Katka", "Babka Zuzana", "Pán Orol", "Pani Ježková", "", "Ján", "Marek", "", "" };
