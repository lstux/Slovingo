// Boite de dialogue : une suite de repliques du cours, ecrites lettre par lettre en bas de l'ecran.
//
//   A        termine la phrase en cours, puis passe a la suite (tenu : ecrit plus vite)
//   SELECT   slovaque -> traduction francaise -> aide (s'il y en a) -> slovaque
//   croix    choisir dans un choix de replique
//
// Une replique du renard qui porte un Choice est d'abord a reconstituer (morceaux dans l'ordre ou
// bonne phrase) ; une erreur ne coute rien, on rechoisit. Un choix reussi du premier coup, sans
// avoir ouvert la traduction, gagne une replique secrete (Episode::bonus) : elles sont dites a la
// fin de la conversation, une par choix reussi.
// Portable : aucun Arduino.
#pragma once
#include <stdint.h>

#include "audio.h"
#include "course.h"
#include "gfx.h"
#include "input.h"

class Dialog {
 public:
  // lines : les repliques a dire ; bonus / nBonus / bonusUsed : repliques secretes disponibles et
  // combien ont deja ete donnees (avance a chaque bonus gagne).
  void start(const Line* lines, int n, const char* playerName, const Line* bonus = nullptr, int nBonus = 0,
             int* bonusUsed = nullptr);
  bool active() const { return mode_ != M_DONE; }
  void step(const Input& in, Audio* audio);  // 1/60 s
  void render(Band& b);

  // pour le simulateur et les tests
  bool choosing() const { return mode_ == M_CHOICE; }
  int cursor() const { return cursor_; }
  int correctOption() const;  // index de la bonne option a l'ecran
  int bonusEarned() const { return earned_; }
  int mistakes() const { return mistakes_; }
  const Line* current() const { return cur_; }

 private:
  enum Mode : uint8_t { M_TEXT, M_CHOICE, M_DONE };
  enum View : uint8_t { V_SK, V_FR, V_HELP };
  static constexpr int MAX_OPT = 8;
  static constexpr int MAX_ROWS = 24;

  void next();
  void beginLine(const Line* l);
  void beginChoice();
  void layout();
  void cycleView();
  void pick(Audio* audio);
  const char* viewText() const;
  void drawBox(Band& b, const char* name);

  const Line* lines_ = nullptr;
  int n_ = 0, index_ = -1;
  const Line* bonus_ = nullptr;
  int nBonus_ = 0;
  int* bonusUsed_ = nullptr;
  const char* name_ = "";
  const Line* cur_ = nullptr;
  int bonusQueued_ = 0;  // repliques secretes gagnees, a dire a la fin
  Mode mode_ = M_DONE;
  View view_ = V_SK;
  bool lookedUp_ = false;  // traduction ou aide ouverte pendant le choix en cours

  // texte : resolu (nom du joueur), coupe en lignes, par pages de PAGE_ROWS lignes
  char text_[640];
  uint16_t rowStart_[MAX_ROWS], rowEnd_[MAX_ROWS];
  int rows_ = 0, page_ = 0;
  int shown_ = 0;  // octets deja ecrits sur la page
  int blink_ = 0;

  // choix
  uint8_t order_[MAX_OPT];    // ordre d'affichage -> index dans Choice::items
  bool gone_[MAX_OPT];        // option deja prise (CH_ORDER) ou ecartee (erreur en CH_PICK)
  int placed_ = 0;            // CH_ORDER : morceaux deja remis
  int cursor_ = 0;
  int shake_ = 0;
  bool firstTry_ = true;
  int earned_ = 0, mistakes_ = 0;
  uint32_t seed_ = 12345;
};
