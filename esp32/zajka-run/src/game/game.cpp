#include "game.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "gen/assets.h"

// ---------------------------------------------------------------- reglages de la physique (par image a 60 Hz)
namespace {
constexpr float WALK_MAX = 2.0f;
constexpr float RUN_MAX = 3.4f;
constexpr float ACCEL = 0.10f;
constexpr float RUN_ACCEL = 0.13f;
constexpr float AIR_ACCEL = 0.08f;
constexpr float FRICTION = 0.12f;
constexpr float SKID = 0.28f;
constexpr float GRAVITY = 0.32f;
constexpr float GRAVITY_HOLD = 0.18f;  // A tenu en montant : saut plus haut
constexpr float JUMP_V = 5.6f;
constexpr float JUMP_SPEED_BONUS = 0.18f;
constexpr float MAX_FALL = 6.5f;
constexpr float STOMP_V = 4.4f, STOMP_V_HOLD = 6.2f;
constexpr float SPRING_V = 8.0f, SPRING_V_HOLD = 9.4f;
constexpr int COYOTE = 6, JUMP_BUFFER = 7;
constexpr int INVULN = 90;
constexpr int HEARTS_PER_LIFE = 50;
// tram : ligne du toit dans l'image 96x48 (sous le pantographe), mesuree sur art/objects/elektricka_96x48.png
constexpr int TRAM_ROOF = 20;
// renard : x du centre de la tete dans sa case de 48 px (tools/import_ai.py --anchor-x 27)
constexpr int FOX_ANCHOR = 27;
// Andrea : 4 images de course (planche Gemini), seulement sur l'ecran titre
constexpr int RABBIT_RUN_FRAMES = 4;
constexpr float CLIMB_V = 1.6f;
// Andrea plane : A tenu en tombant, chute lente (oreilles en parachute)
constexpr float GLIDE_GRAVITY = 0.05f, GLIDE_FALL = 1.5f;
constexpr float PLATFORM_V = 0.8f, PLATFORM_VV = 0.6f;
constexpr int PLATFORM_W = 48, PLATFORM_H = 8;
constexpr int CRUMBLE_DELAY = 50, CRUMBLE_BACK = 300;   // images avant l'effondrement, avant le retour
constexpr int JET_PERIOD = 200, JET_SPUTTER = 110, JET_FULL = 140, JET_H = 64;
constexpr float JET_V = 6.0f, JET_V_HOLD = 6.6f;   // tenir A pour passer le mur de la vasque
constexpr int WIND_W = 8 * 16, WIND_H = 10 * 16, WIND_PERIOD = 240, WIND_ON = 150;
constexpr int ROCK_PERIOD = 150;

// animations de la planche du renard / d'Andrea (une ligne par animation)
// A_CLIMB : 7e ligne de la planche si elle existe, sinon l'image de saut retournee a chaque pas
// A_GLIDE : Andrea, oreilles en parachute ; 8e ligne de la planche, sinon la pose de chute
enum { A_IDLE, A_RUN, A_JUMP, A_FALL, A_HURT, A_WIN, A_CLIMB, A_GLIDE };
const int ANIM_LEN[] = {4, 6, 1, 1, 1, 2, 2, 2};

const uint16_t WHITE = 0xFFFF, BLACK = 0x0000, SHADOW = 0x18C3;
const uint16_t YELLOW = 0xFF2A, ORANGE = 0xFB00, DARKBLUE = 0x1926;

int ifloor(float v) { return (int)floorf(v); }
bool overlap(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh) {
  return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}
}  // namespace

// ---------------------------------------------------------------- mise en route
void Game::begin(Audio* audio, Storage* storage) {
  audio_ = audio;
  if (storage) storage_ = storage;
  if (storage_) {
    SaveData d;
    if (storage_->load(d) && d.magic == SaveData().magic && d.version == SaveData().version) save_ = d;
  }
  if (save_.unlocked < 1) save_.unlocked = 1;
  if (save_.unlocked > NUM_LEVELS) save_.unlocked = NUM_LEVELS;
  if (sel_ >= save_.unlocked) sel_ = save_.unlocked - 1;
  state_ = S_TITLE;
  stateTime_ = 0;
  audio_->music(ALL_LEVELS[sel_]->music);
  // le niveau choisi sert de decor a l'ecran titre
  startLevel(sel_, true);
  camX_ = 0;
}

void Game::levelCleared() {
  int i = levelIndex_;
  int secs = levelTime_ / 60;
  if (i < 8) {
    if (save_.bestTime[i] == 0 || secs < save_.bestTime[i]) save_.bestTime[i] = (uint16_t)secs;
    if (levelHearts_ > save_.bestHearts[i]) save_.bestHearts[i] = (uint8_t)(levelHearts_ > 255 ? 255 : levelHearts_);
    if (secretsFound_ > save_.bestSecrets[i]) save_.bestSecrets[i] = (uint8_t)secretsFound_;
  }
  if (i + 2 > save_.unlocked) save_.unlocked = (uint8_t)(i + 2 > NUM_LEVELS ? NUM_LEVELS : i + 2);
  if (storage_) storage_->save(save_);
}

int Game::spawn(EType t, float x, float y) {
  int slot = -1;
  for (int i = 0; i < nent_; i++)
    if (ent_[i].type == E_NONE) { slot = i; break; }
  if (slot < 0) {
    if (nent_ >= MAX_ENT) return -1;
    slot = nent_++;
  }
  Entity& e = ent_[slot];
  e = Entity();
  e.type = t;
  e.x = x;
  e.y = y;
  e.baseX = x;
  e.baseY = y;
  return slot;
}

void Game::fx(EType t, float x, float y, const char* text) {
  int i = spawn(t, x, y);
  if (i < 0) return;
  ent_[i].timer = t == E_TEXT ? 60 : (t == E_DUST ? 15 : 14);
  ent_[i].text = text;
}

void Game::startLevel(int index, bool fresh) {
  levelIndex_ = index;
  lvl_.load(ALL_LEVELS[index]);
  nent_ = 0;
  for (int i = 0; i < lvl_.nspawns; i++) {
    const Spawn& s = lvl_.spawns[i];
    float tx = s.tx * TILE, ty = s.ty * TILE;
    int k = -1;
    switch (s.kind) {
      case 'o': k = spawn(E_HEART, tx + 2, ty + 2); ent_[k].w = 12; ent_[k].h = 12; break;
      case 'p': k = spawn(E_PIGEON, tx - 9, ty - 8); ent_[k].w = 34; ent_[k].h = 24; ent_[k].vx = -0.5f; break;
      case 'w': k = spawn(E_WASP, tx - 3, ty); ent_[k].w = 22; ent_[k].h = 16; break;
      case 'P': k = spawn(E_SPRING, tx + 1, ty + 6); ent_[k].w = 14; ent_[k].h = 10; break;
      case 'C': k = spawn(E_CHECK, tx, ty + TILE - 48); ent_[k].w = 16; ent_[k].h = 48; break;
      case 'F': k = spawn(E_GOAL, tx, ty + TILE - 48); ent_[k].w = 52; ent_[k].h = 48; break;
      case 'T': k = spawn(E_TRAM, tx, ty + TILE - 48); ent_[k].w = 96; ent_[k].h = 48; ent_[k].dir = 1; break;
      case 'M':
      case 'V':
        k = spawn(E_PLATFORM, tx, ty);
        ent_[k].w = PLATFORM_W;
        ent_[k].h = PLATFORM_H;
        ent_[k].state = s.kind == 'V';
        ent_[k].dir = 1;   // l'ascenseur ('V') attend en bas qu'on monte dessus
        break;
      case 'b': k = spawn(E_BOTTLE, tx + 3, ty + 2); ent_[k].w = 10; ent_[k].h = 14; break;
      case 's': k = spawn(E_SQUIRREL, tx - 2, ty - 4); ent_[k].w = 20; ent_[k].h = 20; ent_[k].dir = -1; break;
      case 'J':
        k = spawn(E_JET, tx, ty);
        ent_[k].w = 12;
        ent_[k].h = JET_H;
        ent_[k].timer = (s.tx * 37) % JET_PERIOD;   // jets decales les uns des autres
        break;
      case 'z': k = spawn(E_WIND, tx, ty + TILE - WIND_H); ent_[k].w = WIND_W; ent_[k].h = WIND_H; break;
      case 'r': k = spawn(E_ROCKSPAWN, tx, ty); ent_[k].timer = (s.tx * 23) % ROCK_PERIOD; break;
      case 'R': k = spawn(E_HUT, tx, ty + TILE - 64); break;
      case 'l': k = spawn(E_BENCH, tx, ty); break;
      case 'k': k = spawn(E_FLOWERS, tx, ty); break;
      default: break;
    }
  }
  respawn_ = false;
  memset(secrets_, 0, sizeof(secrets_));
  secretsFound_ = 0;
  ncrumbles_ = 0;
  weatherLevel_ = 0;
  flash_ = thunderIn_ = 0;
  if (fresh) {
    checkX_ = lvl_.startX;
    checkY_ = lvl_.startY;
    levelHearts_ = 0;
    levelTime_ = 0;
  }
  p_ = Player();
  p_.x = checkX_ + 1;
  p_.y = checkY_ - PH;
  compHp_ = 3;
  compDown_ = 0;
  switched_ = false;
  resetTrail();
  updateCamera(true);
}

void Game::debugTeleport(float x, float feetY) {
  checkX_ = x;
  checkY_ = feetY >= 0 ? feetY : lvl_.startY;
  p_.x = x;
  p_.y = checkY_ - PH;
  p_.vx = p_.vy = 0;
  resetTrail();
  updateCamera(true);
}

// ---------------------------------------------------------------- compagnon
void Game::resetTrail() {
  Snap s;
  s.x = p_.x;
  s.y = p_.y;
  s.anim = (uint8_t)p_.anim;
  s.frame = (uint8_t)p_.animFrame;
  s.left = p_.facingLeft;
  for (int i = 0; i < TRAIL_LEN; i++) trail_[i] = s;
  trailHead_ = 0;
}

void Game::recordTrail() {
  Snap& s = trail_[trailHead_];
  s.x = p_.x;
  s.y = p_.y;
  s.anim = (uint8_t)p_.anim;
  s.frame = (uint8_t)p_.animFrame;
  s.left = p_.facingLeft;
  trailHead_ = (trailHead_ + 1) % TRAIL_LEN;
}

// position du personnage actif, 'age' images plus tot
Snap Game::trailAt(int age) const {
  int i = (trailHead_ - 1 - age + 2 * TRAIL_LEN) % TRAIL_LEN;
  return trail_[i];
}

// bascule sur le compagnon, la ou il se trouve sur le trace ; les points de vie suivent chaque personnage
void Game::switchChar(bool onDeath) {
  Snap s = trailAt(TRAIL_DELAY);
  int hp = p_.hp;
  p_.hp = compHp_;
  compHp_ = hp;
  p_.x = s.x;
  p_.y = s.y;
  p_.vx = p_.vy = 0;
  p_.onGround = false;
  p_.coyote = p_.jumpBuf = 0;
  p_.climbing = false;
  p_.riding = -1;
  p_.squash = 0;
  p_.facingLeft = s.left;
  p_.anim = A_IDLE;
  p_.animTime = p_.animFrame = 0;
  p_.invuln = onDeath ? 60 : 0;   // pas de coup immediat sur la meme bete
  rabbit_ = !rabbit_;
  if (onDeath) compDown_ = REVIVE_FRAMES;
  audio_->play(SFX_BUMP);
}

// ---------------------------------------------------------------- boucle logique
void Game::step(const Input& in) {
  frame_++;
  stateTime_++;
  if (volumeShow_ > 0) volumeShow_--;
  if (in.hit(BTN_VOLUME)) {
    audio_->setVolume((audio_->volume() + 1) % 5);
    volumeShow_ = 90;
    audio_->play(SFX_COIN);
  }
  switch (state_) {
    case S_TITLE:
      camX_ += 0.6f;
      if (camX_ > lvl_.pixelW() - SCREEN_W) camX_ = 0;
      if (save_.unlocked > 1 && (in.hit(BTN_LEFT) || in.hit(BTN_RIGHT))) {
        int n = sel_ + (in.hit(BTN_RIGHT) ? 1 : -1);
        if (n >= 0 && n < save_.unlocked && n < NUM_LEVELS) {
          sel_ = n;
          startLevel(sel_, true);
          camX_ = 0;
          audio_->music(lvl_.def->music);
          audio_->play(SFX_BUMP);
        }
      }
      if (in.hit(BTN_START) || in.hit(BTN_A)) {
        if (sel_ == 0) rabbit_ = false;   // au niveau 1 le renard joue seul : Andrea arrive a la fin
        lives_ = 3;
        hearts_ = 0;
        startLevel(sel_, true);
        state_ = S_INTRO;
        stateTime_ = 0;
        audio_->music(-1);
        audio_->play(SFX_PAUSE);
      }
      break;
    case S_INTRO:
      if (stateTime_ > (respawn_ ? 70 : 130) || (stateTime_ > 30 && in.hit(BTN_A | BTN_START))) {
        state_ = S_PLAY;
        stateTime_ = 0;
        audio_->music(lvl_.def->music);
      }
      break;
    case S_PLAY:
      if (in.hit(BTN_START)) {
        state_ = S_PAUSE;
        audio_->music(-1);
        audio_->play(SFX_PAUSE);
        break;
      }
      if (in.hit(BTN_SELECT) && duo() && compDown_ == 0) switchChar(false);
      updatePlay(in);
      break;
    case S_PAUSE:
      if (in.hit(BTN_START)) {
        state_ = S_PLAY;
        audio_->music(lvl_.def->music);
        audio_->play(SFX_PAUSE);
      } else if (in.hit(BTN_SELECT)) {
        begin(audio_);
      }
      break;
    case S_DYING:
      p_.vy += GRAVITY;
      if (stateTime_ > 25) p_.y += p_.vy;
      updateEntities();
      if (stateTime_ > 130) {
        lives_--;
        if (lives_ <= 0) {
          state_ = S_GAMEOVER;
          stateTime_ = 0;
          audio_->play(SFX_GAMEOVER);
        } else {
          p_ = Player();
          p_.x = checkX_ + 1;
          p_.y = checkY_ - PH;
          p_.invuln = INVULN;
          compHp_ = 3;
          compDown_ = 0;
          resetTrail();
          updateCamera(true);
          state_ = S_INTRO;
          stateTime_ = 0;
          respawn_ = true;
        }
      }
      break;
    case S_CLEAR:
      updateEntities();
      p_.animTime++;
      if (stateTime_ > 150 && in.hit(BTN_A | BTN_START)) {
        if (levelIndex_ + 1 < NUM_LEVELS) {
          startLevel(levelIndex_ + 1, true);
          state_ = S_INTRO;
        } else {
          state_ = S_END;
          audio_->music(lvl_.def->music);
        }
        stateTime_ = 0;
      }
      break;
    case S_GAMEOVER:
    case S_END:
      if (stateTime_ > 90 && in.hit(BTN_START | BTN_A)) begin(audio_);
      break;
  }
  if (shake_ > 0) shake_--;
  if (bumpT_ > 0) bumpT_--;
}

void Game::updatePlay(const Input& in) {
  levelTime_++;
  switched_ = false;
  updatePlayer(in);
  recordTrail();
  if (compDown_ > 0 && --compDown_ == 0) {   // le personnage tombe revient, a cote de l'autre
    compHp_ = 3;
    Snap s = trailAt(TRAIL_DELAY);
    fx(E_SPARK, s.x + PW / 2 - 6, s.y + PH - 20);
    audio_->play(SFX_CHECK);
  }
  updateCrumbles();
  updateEntities();
  interact();
  updateWeather();
  if (p_.y > lvl_.pixelH() + 8 && !switched_) killPlayer();
  updateCamera(false);
}

void Game::updatePlayer(const Input& in) {
  Player& p = p_;
  if (p.climbing || tryStartClimb(in)) {
    if (p.invuln > 0) p.invuln--;
    updateClimb(in);
    return;
  }
  bool left = in.down(BTN_LEFT), right = in.down(BTN_RIGHT);
  bool run = in.down(BTN_B);
  float maxv = run ? RUN_MAX : WALK_MAX;
  if (windT_ > 0) {   // face au vent : on avance moins vite
    windT_--;
    maxv -= 0.9f;
  }
  float acc = p.onGround ? (run ? RUN_ACCEL : ACCEL) : AIR_ACCEL;
  if (p.invuln > INVULN - 20) { left = right = false; }  // recul apres un coup
  if (right && !left) {
    if (p.vx < 0 && p.onGround) p.vx += SKID;
    else if (p.vx < maxv) p.vx += acc;
    p.facingLeft = false;
  } else if (left && !right) {
    if (p.vx > 0 && p.onGround) p.vx -= SKID;
    else if (p.vx > -maxv) p.vx -= acc;
    p.facingLeft = true;
  } else if (p.onGround) {
    if (p.vx > 0) p.vx = p.vx > FRICTION ? p.vx - FRICTION : 0;
    else if (p.vx < 0) p.vx = p.vx < -FRICTION ? p.vx + FRICTION : 0;
  }
  // depasser la vitesse max (apres B relache) : on revient doucement
  if (p.vx > maxv) p.vx -= FRICTION * 0.5f;
  if (p.vx < -maxv) p.vx += FRICTION * 0.5f;
  // poussiere au derapage
  if (p.onGround && ((right && p.vx < -1.2f) || (left && p.vx > 1.2f)) && (frame_ % 5) == 0)
    fx(E_DUST, p.x + PW / 2 - 8, p.y + PH - 14);

  // saut : tampon d'appui + tolerance apres le bord (coyote)
  if (in.hit(BTN_A)) p.jumpBuf = JUMP_BUFFER;
  else if (p.jumpBuf > 0) p.jumpBuf--;
  if (p.onGround) p.coyote = COYOTE;
  else if (p.coyote > 0) p.coyote--;
  if (p.jumpBuf > 0 && p.coyote > 0) {
    p.vy = -(JUMP_V + fabsf(p.vx) * JUMP_SPEED_BONUS);
    p.onGround = false;
    p.coyote = 0;
    p.jumpBuf = 0;
    p.riding = -1;
    audio_->play(SFX_JUMP);
  }
  holdA_ = in.down(BTN_A);
  bool hold = holdA_ && p.vy < 0;
  // Andrea : A tenu en tombant = vol plane, chute lente
  bool glide = rabbit_ && holdA_ && p.vy >= 0 && !p.onGround;
  if (glide) {
    p.vy += GLIDE_GRAVITY;
    if (p.vy > GLIDE_FALL) p.vy = GLIDE_FALL;
  } else {
    p.vy += hold ? GRAVITY_HOLD : GRAVITY;
    if (p.vy > MAX_FALL) p.vy = MAX_FALL;
  }

  bool wasGround = p.onGround;
  float prevBottom = p.y + PH;
  movePlayerX(p.vx);
  p.onGround = false;
  movePlayerY(p.vy, prevBottom);
  if (p.onGround && !wasGround) {
    p.squash = 6;
    if (prevBottom < p.y + PH - 0.5f || p.vy == 0) fx(E_DUST, p.x + PW / 2 - 8, p.y + PH - 14);
  }
  if (p.squash > 0) p.squash--;
  if (p.invuln > 0) p.invuln--;

  // animation
  int a;
  if (p.invuln > INVULN - 20) a = A_HURT;
  else if (!p.onGround) a = glide ? A_GLIDE : (p.vy < 0 ? A_JUMP : A_FALL);
  else if (fabsf(p.vx) > 0.15f) a = A_RUN;
  else a = A_IDLE;
  if (a != p.anim) {
    p.anim = a;
    p.animTime = 0;
  }
  // la course s'accelere avec la vitesse
  int speed = a == A_RUN ? (int)(8 - fabsf(p.vx) * 1.4f) : 12;
  if (speed < 3) speed = 3;
  p.animTime++;
  p.animFrame = (p.animTime / speed) % ANIM_LEN[a];
}

// ---------------------------------------------------------------- echelles
bool Game::tryStartClimb(const Input& in) {
  Player& p = p_;
  if (p.invuln > INVULN - 20) return false;
  int bottom = ifloor(p.y) + PH;
  // colonne de l'echelle : celle du centre du corps, sinon celle d'un des bords
  int cols[3] = {(ifloor(p.x) + PW / 2) >> 4, ifloor(p.x) >> 4, (ifloor(p.x) + PW - 1) >> 4};
  int cx = -1;
  for (int k = 0; k < 3 && cx < 0; k++) {
    int c = cols[k];
    if (in.down(BTN_UP)) {
      bool found = false;
      // une echelle derriere le corps (pas seulement au-dessus de la tete)
      for (int yy = bottom - 4; yy >= ifloor(p.y) + 12; yy -= 8)
        if (lvl_.at(c, yy >> 4) == T_LADDER) found = true;
      // en haut d'une echelle, deja debout dessus : rien a monter
      if (found && p.onGround && lvl_.ladderTop(c, bottom >> 4) && lvl_.at(c, (bottom - 1) >> 4) != T_LADDER)
        found = false;
      if (found) cx = c;
    } else if (in.down(BTN_DOWN) && p.onGround && lvl_.ladderTop(c, bottom >> 4)) {
      cx = c;
      p.y += 4;
    }
  }
  if (cx < 0) return false;
  p.climbing = true;
  p.vx = p.vy = 0;
  p.onGround = false;
  p.riding = -1;
  p.x = cx * TILE + TILE / 2 - PW / 2.0f;
  p.anim = A_CLIMB;
  return true;
}

void Game::updateClimb(const Input& in) {
  Player& p = p_;
  if (in.hit(BTN_A)) {  // on saute de l'echelle
    p.climbing = false;
    p.vy = -4.5f;
    p.vx = in.down(BTN_LEFT) ? -2.0f : (in.down(BTN_RIGHT) ? 2.0f : 0.0f);
    if (p.vx) p.facingLeft = p.vx < 0;
    audio_->play(SFX_JUMP);
    return;
  }
  float dy = in.down(BTN_UP) ? -CLIMB_V : (in.down(BTN_DOWN) ? CLIMB_V : 0.0f);
  int cx = (ifloor(p.x) + PW / 2) >> 4;
  if (dy < 0 && lvl_.kindAt(cx, ifloor(p.y + dy) >> 4) == K_SOLID) dy = 0;  // plafond
  p.y += dy;
  p.climbDist += dy < 0 ? -dy : dy;
  int bottom = ifloor(p.y) + PH;
  if (dy > 0) {  // en bas : on pose le pied
    uint8_t t = lvl_.at(cx, bottom >> 4);
    TileKind k = Level::kind(t);
    if (t != T_LADDER && (k == K_SOLID || k == K_ONEWAY)) {
      p.y = (bottom >> 4) * TILE - PH;
      p.climbing = false;
      p.onGround = true;
      return;
    }
  }
  bool onLadder = lvl_.at(cx, (bottom - 1) >> 4) == T_LADDER || lvl_.at(cx, (ifloor(p.y) + PH / 2) >> 4) == T_LADDER;
  if (!onLadder) {
    if (lvl_.ladderTop(cx, bottom >> 4)) {  // en haut : debout sur l'echelle
      p.y = (bottom >> 4) * TILE - PH;
      p.onGround = true;
    }
    p.climbing = false;
    return;
  }
  p.anim = A_CLIMB;
  p.animFrame = ((int)(p.climbDist / 8)) % 2;
}

bool Game::solidAt(int px, int py) const {
  return lvl_.kindAt(px >> 4, py >> 4) == K_SOLID;
}

void Game::movePlayerX(float dx) {
  Player& p = p_;
  p.x += dx;
  int top = ifloor(p.y) >> 4, bot = (ifloor(p.y) + PH - 1) >> 4;
  if (dx > 0) {
    int tx = (ifloor(p.x) + PW - 1) >> 4;
    for (int ty = top; ty <= bot; ty++)
      if (lvl_.kindAt(tx, ty) == K_SOLID) {
        p.x = tx * TILE - PW;
        p.vx = 0;
        break;
      }
  } else if (dx < 0) {
    int tx = ifloor(p.x) >> 4;
    for (int ty = top; ty <= bot; ty++)
      if (lvl_.kindAt(tx, ty) == K_SOLID) {
        p.x = (tx + 1) * TILE;
        p.vx = 0;
        break;
      }
  }
  if (p.x < 0) p.x = 0;
}

void Game::movePlayerY(float dy, float prevBottom) {
  Player& p = p_;
  p.y += dy;
  int l = ifloor(p.x) >> 4, r = (ifloor(p.x) + PW - 1) >> 4;
  if (dy > 0) {
    float bottom = p.y + PH;
    // la case qui contient le bord bas : on atterrit des qu'on y entre, meme d'une fraction de pixel
    int ty = ifloor(bottom) >> 4;
    for (int tx = l; tx <= r; tx++) {
      TileKind k = lvl_.kindAt(tx, ty);
      bool top = k == K_ONEWAY || (k == K_LADDER && lvl_.ladderTop(tx, ty));
      if (k == K_SOLID || (top && prevBottom <= ty * TILE + 0.01f)) {
        p.y = ty * TILE - PH;
        p.vy = 0;
        p.onGround = true;
        p.riding = -1;
        return;
      }
    }
    // toit du tram, passerelles mobiles
    for (int i = 0; i < nent_; i++) {
      Entity& e = ent_[i];
      if (e.type != E_TRAM && e.type != E_PLATFORM) continue;
      float roof = platformTop(e);
      float tol = e.type == E_PLATFORM ? 2.0f : 0.5f;
      int inset = e.type == E_PLATFORM ? 0 : 4;
      if (prevBottom <= roof + tol && bottom >= roof && p.x + PW > e.x + inset && p.x < e.x + e.w - inset) {
        p.y = roof - PH;
        p.vy = 0;
        p.onGround = true;
        p.riding = i;
        return;
      }
    }
    p.riding = -1;
  } else if (dy < 0) {
    int ty = ifloor(p.y) >> 4;
    // la case la plus proche du centre est celle qu'on cogne
    int cx = (ifloor(p.x) + PW / 2) >> 4;
    int order[3] = {cx, l, r};
    for (int k = 0; k < 3; k++) {
      int tx = order[k];
      if (tx < l || tx > r) continue;
      uint8_t tt = lvl_.at(tx, ty);
      if ((lvl_.kindAt(tx, ty) == K_SOLID || tt == T_HIDDEN) && ty >= 0) {
        p.y = (ty + 1) * TILE;
        p.vy = 0.5f;
        if (tt == T_HIDDEN) {   // un bloc cache : il devient un bloc bonus, avec 5 medovniky
          lvl_.set(tx, ty, T_BONUS);
          secretsFound_++;
          for (int k = 0; k < 4; k++) collectHeart(tx * TILE, ty * TILE - 12, true);
          fx(E_TEXT, tx * TILE - 12, ty * TILE - 26, "Tajné!");
        }
        bumpTile(tx, ty);
        return;
      }
    }
  }
}

float Game::platformTop(const Entity& e) const {
  return e.type == E_TRAM ? e.y + TRAM_ROOF : e.y;
}

void Game::bumpTile(int tx, int ty) {
  uint8_t t = lvl_.at(tx, ty);
  bumpTx_ = tx;
  bumpTy_ = ty;
  bumpT_ = 8;
  if (t == T_BONUS) {
    lvl_.set(tx, ty, T_BONUS_USED);
    int i = spawn(E_POP, tx * TILE + 2, ty * TILE - 10);
    if (i >= 0) {
      ent_[i].vy = -3.6f;
      ent_[i].timer = 34;
    }
    collectHeart(tx * TILE, ty * TILE - 12, true);
  } else {
    audio_->play(SFX_BUMP);
  }
  // un pigeon pose sur la case cognee est assomme
  for (int i = 0; i < nent_; i++) {
    Entity& e = ent_[i];
    if (e.type == E_PIGEON && e.state == 0 && fabsf(e.y + e.h - ty * TILE) < 2 &&
        e.x + e.w > tx * TILE && e.x < tx * TILE + TILE) {
      e.state = 1;
      e.timer = 30;
      audio_->play(SFX_STOMP);
    }
  }
}

// entre dans un faux mur : tout le passage secret se revele
void Game::revealSecrets() {
  if (lvl_.nsecrets == 0) return;
  int l = ifloor(p_.x) >> 4, r = (ifloor(p_.x) + PW - 1) >> 4;
  int t = ifloor(p_.y) >> 4, b = (ifloor(p_.y) + PH - 1) >> 4;
  for (int ty = t; ty <= b; ty++) {
    for (int tx = l; tx <= r; tx++) {
      if (lvl_.at(tx, ty) != T_SECRET) continue;
      int id = lvl_.regionAt(tx, ty);
      if (id <= 0 || secrets_[id].revealed) continue;
      secrets_[id] = SecretRegion{true, (int)frame_, (int16_t)tx, (int16_t)ty};
      secretsFound_++;
      audio_->play(SFX_LIFE);
      fx(E_TEXT, p_.x - 14, p_.y - 16, "Tajné!");
    }
  }
}

// un medovnik ou une bouteille dans un passage secret encore ferme n'est pas dessine
bool Game::hiddenBySecret(const Entity& e) const {
  if (e.type != E_HEART && e.type != E_BOTTLE) return false;
  int tx = (int)(e.x + e.w / 2) >> 4, ty = (int)(e.y + e.h / 2) >> 4;
  if (lvl_.at(tx, ty) != T_SECRET) return false;
  return !secrets_[lvl_.regionAt(tx, ty)].revealed;
}

void Game::collectHeart(float x, float y, bool popped) {
  hearts_++;
  levelHearts_++;
  audio_->play(SFX_COIN);
  if (!popped) fx(E_SPARK, x, y);
  if (hearts_ % HEARTS_PER_LIFE == 0) {
    lives_++;
    audio_->play(SFX_LIFE);
    fx(E_TEXT, p_.x - 10, p_.y - 14, "+1 život");
  }
}

// ---------------------------------------------------------------- sol qui s'effondre
void Game::updateCrumbles() {
  // le renard debout sur une case fragile la met en route
  if (p_.onGround) {
    int by = (ifloor(p_.y) + PH) >> 4;
    for (int tx = ifloor(p_.x) >> 4; tx <= (ifloor(p_.x) + PW - 1) >> 4; tx++) {
      if (lvl_.at(tx, by) != T_CRUMBLE) continue;
      bool known = false;
      for (int i = 0; i < ncrumbles_; i++)
        if (crumbles_[i].tx == tx && crumbles_[i].ty == by) known = true;
      if (!known && ncrumbles_ < (int)(sizeof(crumbles_) / sizeof(crumbles_[0])))
        crumbles_[ncrumbles_++] = Crumble{(int16_t)tx, (int16_t)by, 0};
    }
  }
  for (int i = 0; i < ncrumbles_; i++) {
    Crumble& c = crumbles_[i];
    c.t++;
    if (c.t == CRUMBLE_DELAY) {
      lvl_.set(c.tx, c.ty, T_EMPTY);
      fx(E_DUST, c.tx * TILE, c.ty * TILE);
      audio_->play(SFX_CRUMBLE);
    }
    if (c.t >= CRUMBLE_DELAY + CRUMBLE_BACK) {
      // revient, sauf si le renard est dedans
      bool inside = overlap(p_.x, p_.y, PW, PH, c.tx * TILE, c.ty * TILE, TILE, TILE);
      if (inside) {
        c.t--;
        continue;
      }
      lvl_.set(c.tx, c.ty, T_CRUMBLE);
      crumbles_[i--] = crumbles_[--ncrumbles_];
    }
  }
}

// ---------------------------------------------------------------- orage (niveau des Tatras)
void Game::updateWeather() {
  if (!lvl_.def->weather) return;
  float k = p_.x / (float)lvl_.pixelW();
  weatherLevel_ = k < 0.25f ? 0 : (k < 0.55f ? 1 : 2);
  if (flash_ > 0) flash_--;
  if (thunderIn_ > 0 && --thunderIn_ == 0) audio_->play(SFX_THUNDER);
  if (weatherLevel_ >= 1 && --nextBolt_ <= 0) {
    // d'abord l'eclair, ensuite le tonnerre (najprv blesk, potom hrom)
    flash_ = 5;
    thunderIn_ = 18 + rnd() % 30;
    shake_ = 4;
    nextBolt_ = (weatherLevel_ == 2 ? 170 : 330) + rnd() % 200;
  }
}

void Game::updateEntities() {
  for (int i = 0; i < nent_; i++) {
    Entity& e = ent_[i];
    switch (e.type) {
      case E_PIGEON:
        if (e.state == 0) {
          // demi-tour devant un mur, une butee ou un trou
          float nx = e.x + e.vx;
          int frontX = e.vx < 0 ? ifloor(nx) : ifloor(nx) + e.w - 1;
          int ftx = frontX >> 4, fty = (ifloor(e.y) + e.h - 1) >> 4;
          bool wall = lvl_.kindAt(ftx, fty) == K_SOLID || lvl_.mark(ftx, fty);
          bool hole = lvl_.kindAt(ftx, fty + 1) == K_AIR;
          if (wall || hole) e.vx = -e.vx;
          else e.x = nx;
          e.timer++;
        } else if (--e.timer <= 0) {
          fx(E_SPARK, e.x + 2, e.y - 4);
          e.type = E_NONE;
        }
        break;
      case E_WASP:
        if (e.state == 0) {
          e.timer++;
          float t = e.timer * 0.025f;
          float ox = e.x;
          e.x = e.baseX + sinf(t) * 40;
          e.y = e.baseY + sinf(t * 3.1f) * 6;
          e.dir = e.x >= ox ? 1 : -1;
        } else {
          e.vy += GRAVITY;
          e.y += e.vy;
          if (e.y > lvl_.pixelH() + 20) e.type = E_NONE;
        }
        break;
      case E_SPRING:
        if (e.timer > 0) e.timer--;
        break;
      case E_TRAM: {
        if (e.timer > 0) {   // arret aux butees
          if (--e.timer == 0) audio_->play(SFX_BELL);
          break;
        }
        float dx = 0.9f * e.dir;
        float lead = e.dir > 0 ? e.x + e.w + dx : e.x + dx;
        int ltx = ifloor(lead) >> 4, lty = (ifloor(e.y) + e.h - 1) >> 4;
        if (lvl_.mark(ltx, lty) || lvl_.kindAt(ltx, lty) == K_SOLID) {
          e.dir = -e.dir;
          e.timer = 70;
          break;
        }
        e.x += dx;
        if (p_.riding == i) {
          movePlayerX(dx);
          p_.y = e.y + TRAM_ROOF - PH;
        }
        break;
      }
      case E_PLATFORM: {
        // l'ascenseur reste en haut tant que le renard est dessus
        if (e.state == 1 && e.dir > 0 && e.y < e.baseY && p_.riding == i && e.timer < 30) e.timer = 30;
        if (e.timer > 0) {  // petite pause aux butees
          e.timer--;
          break;
        }
        float dx = 0, dy = 0;
        int tl = ifloor(e.x) >> 4;
        if (e.state == 0) {
          dx = PLATFORM_V * e.dir;
          float lead = e.dir > 0 ? e.x + e.w + dx : e.x + dx;
          int ltx = ifloor(lead) >> 4, lty = ifloor(e.y) >> 4;
          if (lvl_.mark(ltx, lty) || lvl_.kindAt(ltx, lty) == K_SOLID) {
            e.dir = -e.dir;
            e.timer = 30;
            break;
          }
        } else {
          // ascenseur : attend en bas qu'on monte dessus, monte, puis redescend tout seul
          bool atBase = e.dir > 0 && fabsf(e.y - e.baseY) < 0.5f;
          if (atBase && e.y >= e.baseY) {
            e.y = e.baseY;
            if (p_.riding != i) break;
            e.dir = -1;
          }
          if (e.dir > 0 && e.y + PLATFORM_VV >= e.baseY) {  // de retour en bas
            dy = e.baseY - e.y;
            e.y = e.baseY;
            if (p_.riding == i && !p_.climbing) p_.y = e.y - PH;
            break;
          }
          dy = PLATFORM_VV * e.dir;
          float lead = e.dir > 0 ? e.y + e.h + dy : e.y + dy;
          int lty = ifloor(lead) >> 4;
          if (lvl_.mark(tl, lty) || lvl_.kindAt(tl, lty) == K_SOLID || lty < 0 || lty >= lvl_.h) {
            // arret pile sur la grille : le dessus de la passerelle au ras de la roche voisine
            if (e.dir < 0) e.y = (float)((lty + 1) * TILE);
            if (p_.riding == i && !p_.climbing) p_.y = e.y - PH;
            e.dir = -e.dir;
            e.timer = 40;
            break;
          }
        }
        e.x += dx;
        e.y += dy;
        if (p_.riding == i && !p_.climbing) {
          if (dx) movePlayerX(dx);
          p_.y = e.y - PH;
        }
        break;
      }
      case E_SQUIRREL:
        if (e.state == 0) {
          e.vy += GRAVITY;
          if (e.vy > MAX_FALL) e.vy = MAX_FALL;
          // en l'air : avance ; a terre : attend puis bondit vers le renard
          float nx = e.x + e.vx;
          int frontX = e.vx < 0 ? ifloor(nx) : ifloor(nx) + e.w - 1;
          int ftx = frontX >> 4, fty = (ifloor(e.y) + e.h - 1) >> 4;
          if (e.vx && (lvl_.kindAt(ftx, fty) == K_SOLID || lvl_.mark(ftx, fty))) e.vx = -e.vx;
          else e.x = nx;
          float prevB = e.y + e.h;
          e.y += e.vy;
          int bty = (ifloor(e.y) + e.h) >> 4;
          int bl = ifloor(e.x) >> 4, br = (ifloor(e.x) + e.w - 1) >> 4;
          bool landed = false;
          for (int tx = bl; tx <= br && !landed; tx++) {
            TileKind k = lvl_.kindAt(tx, bty);
            bool top = k == K_ONEWAY || (k == K_LADDER && lvl_.ladderTop(tx, bty));
            if (e.vy >= 0 && (k == K_SOLID || (top && prevB <= bty * TILE + 0.01f))) landed = true;
          }
          if (landed) {
            e.y = bty * TILE - e.h;
            e.vy = 0;
            e.vx = 0;
            if (++e.timer > 70 && fabsf(p_.x - e.x) < 150 && fabsf(p_.y - e.y) < 90) {
              e.dir = p_.x < e.x ? -1 : 1;
              e.vx = 1.3f * e.dir;
              e.vy = -4.2f;
              e.timer = 0;
            }
          }
          if (e.y > lvl_.pixelH() || lvl_.kindAt((ifloor(e.x) + 7) >> 4, (ifloor(e.y) + e.h - 2) >> 4) == K_WATER)
            e.type = E_NONE;
        } else if (--e.timer <= 0) {
          fx(E_SPARK, e.x, e.y - 4);
          e.type = E_NONE;
        }
        break;
      case E_BOTTLE:
        e.timer++;
        break;
      case E_JET:
        e.timer = (e.timer + 1) % JET_PERIOD;
        if (e.state > 0) e.state--;   // pas de double propulsion
        break;
      case E_WIND:
        e.timer = (e.timer + 1) % WIND_PERIOD;
        break;
      case E_ROCKSPAWN:
        if (fabsf(p_.x - e.x) < 220 && ++e.timer >= ROCK_PERIOD) {
          e.timer = 0;
          int k = spawn(E_ROCK, e.x + 2, e.y);
          if (k >= 0) {
            ent_[k].w = 12;
            ent_[k].h = 12;
          }
        }
        break;
      case E_ROCK: {
        e.vy += 0.22f;
        if (e.vy > 6) e.vy = 6;
        e.y += e.vy;
        e.timer++;
        int bx = (ifloor(e.x) + 6) >> 4, by = (ifloor(e.y) + e.h) >> 4;
        TileKind k = lvl_.kindAt(bx, by);
        if (e.timer > 8 && (k == K_SOLID || k == K_WATER || e.y > lvl_.pixelH())) {
          fx(E_DUST, e.x - 2, (float)(by * TILE - 14));
          e.type = E_NONE;
        }
        break;
      }
      case E_POP:
        e.vy += 0.3f;
        e.y += e.vy;
        if (--e.timer <= 0) {
          fx(E_SPARK, e.x, e.y);
          e.type = E_NONE;
        }
        break;
      case E_DUST:
      case E_SPARK:
        if (--e.timer <= 0) e.type = E_NONE;
        break;
      case E_TEXT:
        e.y -= 0.5f;
        if (--e.timer <= 0) e.type = E_NONE;
        break;
      case E_GOAL:
      case E_CHECK:
        e.timer++;
        break;
      default:
        break;
    }
  }
}

void Game::interact() {
  Player& p = p_;
  revealSecrets();
  float px = p.x, py = p.y;
  // pointes
  int l = ifloor(px) >> 4, r = (ifloor(px) + PW - 1) >> 4;
  int t = ifloor(py) >> 4, b = (ifloor(py) + PH - 1) >> 4;
  for (int ty = t; ty <= b; ty++)
    for (int tx = l; tx <= r; tx++)
      if (lvl_.kindAt(tx, ty) == K_HAZARD && overlap(px, py, PW, PH, tx * TILE + 1, ty * TILE + 7, 14, 9))
        hurtPlayer(tx * TILE + 8);
  // l'eau du torrent : plouf
  if (lvl_.kindAt((ifloor(px) + PW / 2) >> 4, (ifloor(py) + PH - 4) >> 4) == K_WATER) {
    fx(E_DUST, px + PW / 2 - 8, (float)(((ifloor(py) + PH - 4) >> 4) * TILE - 12));
    killPlayer();
    return;
  }

  for (int i = 0; i < nent_ && !switched_; i++) {
    Entity& e = ent_[i];
    if (e.type == E_NONE) continue;
    switch (e.type) {
      case E_HEART:
        if (overlap(px, py, PW, PH, e.x, e.y, e.w, e.h)) {
          collectHeart(e.x - 2, e.y - 2, false);
          e.type = E_NONE;
        }
        break;
      case E_JET: {
        // colonne d'eau au-dessus de la buse, seulement quand le jet est plein
        if (e.timer < JET_FULL) break;
        float top = e.y + TILE - JET_H;
        bool in = overlap(px, py, PW, PH, e.x + 2, top, e.w, JET_H);
        if (in && e.state) e.state = 20;   // une seule propulsion tant qu'on reste dans la colonne
        if (in && !e.state) {
          p.vy = holdA_ ? -JET_V_HOLD : -JET_V;
          p.onGround = false;
          p.riding = -1;
          p.climbing = false;
          p.coyote = 0;
          e.state = 20;
          audio_->play(SFX_SPRING);
        }
        break;
      }
      case E_WIND:
        if (e.timer > 30 && e.timer < WIND_ON && !p.climbing && overlap(px, py, PW, PH, e.x, e.y, e.w, e.h)) {
          // rafale de face : on avance encore en marchant, mieux en courant (B)
          p.vx -= p.onGround ? 0.07f : 0.10f;
          if (p.vx < -1.6f) p.vx = -1.6f;
          windT_ = 2;
        }
        break;
      case E_ROCK:
        if (overlap(px + 2, py + 2, PW - 4, PH - 4, e.x, e.y, e.w, e.h)) {
          hurtPlayer(e.x + 6);
          fx(E_DUST, e.x - 2, e.y);
          e.type = E_NONE;
        }
        break;
      case E_BOTTLE:
        if (overlap(px, py, PW, PH, e.x, e.y, e.w, e.h)) {
          if (p.hp < 3) p.hp++;
          audio_->play(SFX_CHECK);
          fx(E_SPARK, e.x - 3, e.y - 2);
          fx(E_TEXT, e.x - 14, e.y - 14, "Voda!");
          e.type = E_NONE;
        }
        break;
      case E_PIGEON:
      case E_WASP:
      case E_SQUIRREL:
        if (e.state != 0) break;
        if (overlap(px + 1, py + 2, PW - 2, PH - 2, e.x, e.y, e.w, e.h)) {
          if (p.vy > 0 && py + PH - p.vy <= e.y + 8) {
            // on lui saute dessus
            e.state = 1;
            e.timer = 30;
            if (e.type == E_WASP) e.vy = -2;
            p.vy = holdA_ ? -STOMP_V_HOLD : -STOMP_V;
            p.coyote = 0;
            audio_->play(SFX_STOMP);
            fx(E_SPARK, e.x + e.w / 2 - 8, e.y - 6);
          } else {
            hurtPlayer(e.x + e.w / 2);
          }
        }
        break;
      case E_SPRING:
        if (p.vy >= 0 && overlap(px, py, PW, PH, e.x, e.y, e.w, e.h) && py + PH - p.vy <= e.y + 5) {
          p.y = e.y - PH;
          p.vy = holdA_ ? -SPRING_V_HOLD : -SPRING_V;
          p.onGround = false;
          p.coyote = 0;
          e.timer = 12;
          audio_->play(SFX_SPRING);
        }
        break;
      case E_CHECK:
        if (e.state == 0 && overlap(px, py, PW, PH, e.x, e.y, e.w, e.h)) {
          e.state = 1;
          checkX_ = e.x;
          checkY_ = e.y + e.h;
          audio_->play(SFX_CHECK);
          fx(E_SPARK, e.x, e.y - 4);
        }
        break;
      case E_TRAM:
        if (p.riding != i && p.invuln == 0 &&
            overlap(px + 2, py + 4, PW - 4, PH - 6, e.x + 4, e.y + TRAM_ROOF + 6, e.w - 8, e.h - TRAM_ROOF - 8))
          hurtPlayer(e.x + e.w / 2);
        break;
      case E_GOAL:
        if (overlap(px, py, PW, PH, e.x, e.y, e.w, e.h)) {
          state_ = S_CLEAR;
          stateTime_ = 0;
          levelCleared();
          p.vx = 0;
          p.climbing = false;
          p.anim = A_WIN;
          p.animTime = 0;
          p.facingLeft = false;
          audio_->music(-1);
          audio_->play(SFX_WIN);
        }
        break;
      default:
        break;
    }
  }
}

void Game::hurtPlayer(float fromX) {
  Player& p = p_;
  if (p.invuln > 0 || state_ != S_PLAY) return;
  p.hp--;
  shake_ = 8;
  if (p.hp <= 0) {
    killPlayer();
    return;
  }
  p.invuln = INVULN;
  p.climbing = false;
  p.vx = (p.x + PW / 2 < fromX) ? -2.5f : 2.5f;
  p.vy = -3.5f;
  p.onGround = false;
  p.riding = -1;
  audio_->play(SFX_HURT);
}

void Game::killPlayer() {
  if (state_ != S_PLAY) return;
  if (duo() && compDown_ == 0) {   // l'autre est debout : on bascule, sans perdre de vie
    switchChar(true);
    switched_ = true;
    fx(E_TEXT, p_.x - 6, p_.y - 16, "Hop!");
    return;
  }
  state_ = S_DYING;
  stateTime_ = 0;
  p_.vy = -6.5f;
  p_.vx = 0;
  p_.anim = A_HURT;
  p_.riding = -1;
  p_.climbing = false;
  audio_->music(-1);
  audio_->play(SFX_DIE);
}

void Game::updateCamera(bool snap) {
  float target = p_.x + PW / 2 - 150;
  lookAhead_ += (p_.vx * 22 - lookAhead_) * 0.04f;
  target += lookAhead_;
  float maxX = lvl_.pixelW() - SCREEN_W;
  float ty = p_.y + PH - 160;
  float maxY = lvl_.pixelH() - SCREEN_H;
  if (snap) {
    camX_ = target;
    camY_ = ty;
  } else {
    camX_ += (target - camX_) * 0.18f;
    camY_ += (ty - camY_) * 0.12f;
  }
  if (camX_ < 0) camX_ = 0;
  if (camX_ > maxX) camX_ = maxX;
  if (camY_ > maxY) camY_ = maxY;
  if (camY_ < 0) camY_ = 0;
}

// ---------------------------------------------------------------- rendu
void Game::renderBand(Band& b) {
  drawWorld(b);
  postProcess(b);
  if (state_ == S_TITLE) {
    drawTitle(b);
    return;
  }
  drawHud(b);
  drawOverlay(b);
}

// pluie, nuit qui tombe et eclairs (niveau des Tatras), sur le decor mais sous le HUD
void Game::postProcess(Band& b) {
  if (!lvl_.def->weather || state_ == S_TITLE) return;
  int n = SCREEN_W * b.h;
  if (flash_ > 0) {
    for (int i = 0; i < n; i++) b.buf[i] |= 0x39E7;   // tout s'eclaire
  } else if (weatherLevel_ == 2) {
    for (int i = 0; i < n; i++) b.buf[i] = (b.buf[i] >> 1) & 0x7BEF;   // moitie moins de lumiere
  } else if (weatherLevel_ == 1) {
    for (int i = 0; i < n; i++) {
      uint16_t c = b.buf[i];
      b.buf[i] = c - ((c >> 2) & 0x39E7);   // un quart de moins
    }
  }
  if (weatherLevel_ >= 1) {
    // pluie : traits obliques qui tombent vite, plus nombreux quand l'orage est la
    int drops = weatherLevel_ == 2 ? 90 : 45;
    uint16_t col = weatherLevel_ == 2 ? 0x8C71 : 0xAD97;
    for (int i = 0; i < drops; i++) {
      int x = (int)((i * 2654435761u >> 7) % 340) - (int)((frame_ * 3) % 340) ;
      int y = (int)(((i * 40503u) + frame_ * 11u) % 260) - 10;
      x = ((x % 340) + 340) % 340 - 10;
      for (int k = 0; k < 6; k++) {
        int yy = y + k, xx = x - k / 2;
        if (yy < b.y0 || yy >= b.y0 + b.h || xx < 0 || xx >= SCREEN_W) continue;
        b.buf[(yy - b.y0) * SCREEN_W + xx] = col;
      }
    }
  }
}

void Game::drawWorld(Band& b) {
  const Theme& th = *lvl_.def->theme;
  int cx = (int)camX_ + (shake_ ? ((frame_ & 2) ? 2 : -2) : 0);
  int cy = (int)camY_;
  b.wrapRow(*th.far, cx / 5, -(cy / 10));
  int ground = lvl_.pixelH() - 2 * TILE - cy;
  b.wrapRow(*th.mid, cx / 2, ground - th.mid->h);
  // sous le niveau de la rue : terre sombre (les trous ne montrent pas le ciel)
  b.fillRect(0, ground, SCREEN_W, SCREEN_H - ground, th.under);
  b.fillRect(0, ground + 20, SCREEN_W, SCREEN_H - ground - 20, th.under2);

  // decor et objets derriere le joueur
  for (int i = 0; i < nent_; i++) {
    const Entity& e = ent_[i];
    if (e.type == E_BENCH || e.type == E_FLOWERS || e.type == E_CHECK || e.type == E_GOAL || e.type == E_TRAM ||
        e.type == E_HUT)
      drawEntity(b, e);
  }

  // tuiles
  const Image8& ts = *th.tiles;
  int ty0 = (b.y0 + cy) >> 4, ty1 = (b.y0 + b.h - 1 + cy) >> 4;
  int tx0 = cx >> 4, tx1 = (cx + SCREEN_W - 1) >> 4;
  for (int ty = ty0; ty <= ty1; ty++) {
    for (int tx = tx0; tx <= tx1; tx++) {
      uint8_t t = lvl_.at(tx, ty);
      if (t == T_EMPTY || t == T_HIDDEN || tx < 0 || tx >= lvl_.w) continue;
      if (t == T_SECRET) {
        const SecretRegion& sr = secrets_[lvl_.regionAt(tx, ty)];
        if (sr.revealed) {
          // le mur s'efface en vague a partir de l'endroit ou l'on est entre
          int age = (int)frame_ - sr.t0, d = abs(tx - sr.ox) + abs(ty - sr.oy);
          if (age >= 5 * d + 10) continue;
          if (age >= 5 * d && (frame_ / 2) % 2) continue;
        }
        t = lvl_.lookAt(tx, ty);
      }
      int dy = ty * TILE - cy;
      if (bumpT_ > 0 && tx == bumpTx_ && ty == bumpTy_) dy -= (bumpT_ > 4 ? 8 - bumpT_ : bumpT_);
      if (t == T_WATER_TOP && ((frame_ / 16 + tx) & 1)) t = T_WATER_TOP2;  // vaguelettes
      int dx = 0;
      if (t == T_CRUMBLE) {
        for (int i = 0; i < ncrumbles_; i++)
          if (crumbles_[i].tx == tx && crumbles_[i].ty == ty && crumbles_[i].t < CRUMBLE_DELAY) {
            t = T_CRUMBLE_HIT;                               // ca se fissure...
            dx = (crumbles_[i].t / 2) % 2 ? 1 : -1;         // ... et ca tremble
          }
      }
      b.blit(ts, (t % 8) * TILE, (t / 8) * TILE, TILE, TILE, tx * TILE - cx + dx, dy);
    }
  }

  for (int i = 0; i < nent_; i++) {
    const Entity& e = ent_[i];
    if (e.type == E_NONE || e.type == E_BENCH || e.type == E_FLOWERS || e.type == E_CHECK ||
        e.type == E_GOAL || e.type == E_TRAM || e.type == E_DUST || e.type == E_SPARK || e.type == E_TEXT ||
        e.type == E_HUT || hiddenBySecret(e))
      continue;
    drawEntity(b, e);
  }
  if (state_ != S_TITLE) {
    if (duo()) drawFollower(b);
    drawPlayer(b);
  }
  for (int i = 0; i < nent_; i++) {
    const Entity& e = ent_[i];
    if (e.type == E_DUST || e.type == E_SPARK || e.type == E_TEXT) drawEntity(b, e);
  }
}

void Game::drawEntity(Band& b, const Entity& e) {
  int cx = (int)camX_, cy = (int)camY_;
  int sx = (int)lroundf(e.x) - cx, sy = (int)lroundf(e.y) - cy;
  if (sx < -110 || sx > SCREEN_W + 10) return;
  if (sy > b.y0 + b.h || sy + (e.h > 64 ? e.h : 64) < b.y0) return;
  switch (e.type) {
    case E_HEART: {
      int bob = ((frame_ + (int)e.baseX) / 20) % 2;
      b.frame(spr_medovnik, (frame_ / 8 + (int)e.baseX / 16) % 4, sx - 2, sy - 2 - bob);
      break;
    }
    case E_POP:
      b.frame(spr_medovnik, (frame_ / 4) % 4, sx - 2, sy - 2);
      break;
    case E_PIGEON:
      if (e.state == 0) b.frame(spr_holub, (e.timer / 10) % 2, sx - 7, sy - 8, e.vx < 0);
      else b.frame(spr_holub, 2, sx - 7, sy - 8, e.vx < 0);
      break;
    case E_WASP:
      b.frame(spr_osa, (frame_ / 3) % 2, sx - 5, sy - 8, e.dir < 0);
      break;
    case E_SPRING:
      b.frame(spr_pruzina, e.timer > 0 ? 1 : 0, sx - 1, sy - 6);
      break;
    case E_CHECK:
      b.frame(*lvl_.def->theme->check, e.state ? 1 : 0, sx, sy);
      break;
    case E_GOAL: {
      bool cleared = state_ == S_CLEAR;
      int row = cleared ? A_WIN : A_IDLE;
      int f = cleared ? (stateTime_ / 14) % 2 : (e.timer / 14) % 4;
      if (lvl_.def->npc) {
        // le personnage du niveau (pan Orol, le jardinier, le chatar), Andrea a cote
        b.frame(spr_rabbit, row * spr_rabbit.cols + f, sx, sy, true);
        b.frame(*lvl_.def->npc, cleared && stateTime_ > 40 ? (stateTime_ / 20) % 2 : 0, sx + 26, sy);
      } else {
        // Andrea attend a l'arret de tram
        b.frame(spr_zastavka, 0, sx, sy);
        b.frame(spr_rabbit, row * spr_rabbit.cols + f, sx + 20, sy, true);
      }
      break;
    }
    case E_PLATFORM:
      b.frame(spr_platforma, 0, sx, sy);
      break;
    case E_JET: {
      // buse au sol, colonne d'eau au-dessus (image 0 eteint, 1 crachote, 2-3 plein)
      int f = e.timer < JET_SPUTTER ? 0 : (e.timer < JET_FULL ? 1 : 2 + (frame_ / 4) % 2);
      b.frame(spr_tryska, f, sx, sy + TILE - spr_tryska.fh);
      break;
    }
    case E_WIND:
      if (e.timer > 30 && e.timer < WIND_ON) {
        // rafale : quelques traits blancs qui filent vers la gauche
        for (int i = 0; i < 9; i++) {
          int lx = sx + e.w - (int)((frame_ * 6 + i * 47) % (e.w + 20));
          int ly = sy + 10 + (i * 37) % (e.h - 20);
          b.fillRect(lx, ly, 10 + (i % 3) * 4, 1, 0xE73C);
        }
      }
      break;
    case E_ROCK:
      b.frame(spr_kamen, (e.timer / 6) % 2, sx - 2, sy - 2);
      break;
    case E_HUT:
      b.frame(spr_chata, 0, sx, sy);
      break;
    case E_BOTTLE:
      b.frame(spr_flasa, 0, sx - 3, sy - 2 - ((frame_ / 20) % 2));
      break;
    case E_SQUIRREL:
      b.frame(spr_vevericka, e.state ? 2 : (e.vy < 0 || e.vx ? 1 : 0), sx - 6, sy - 12, e.dir < 0);
      break;
    case E_TRAM:
      if (e.dir < 0) {
        // le tram repart dans l'autre sens : le corps est retourne, mais le pantographe reste a sa place
        const Image8& im = *spr_elektricka.img;
        b.blit(im, 0, TRAM_ROOF, im.w, im.h - TRAM_ROOF, sx, sy + TRAM_ROOF, true);
        b.blit(im, 0, 0, im.w, TRAM_ROOF, sx, sy, false);
      } else {
        b.frame(spr_elektricka, 0, sx, sy);
      }
      break;
    case E_BENCH:
      b.frame(*lvl_.def->theme->decor, 0, sx, sy);
      break;
    case E_FLOWERS:
      b.frame(*lvl_.def->theme->decor, 1, sx, sy);
      break;
    case E_DUST:
      b.frame(spr_fx, 2 - e.timer / 6, sx, sy);
      break;
    case E_SPARK:
      b.frame(spr_fx, 3 + (e.timer / 7) % 2, sx, sy);
      break;
    case E_TEXT:
      if (e.text) b.text(font_small, sx, sy, e.text, YELLOW, SHADOW);
      break;
    default:
      break;
  }
}

void Game::drawPlayer(Band& b) {
  const Player& p = p_;
  if (p.invuln > 0 && p.invuln < INVULN - 20 && (frame_ / 3) % 2 && state_ == S_PLAY) return;  // clignote
  int row = p.anim, f = p.animFrame;
  bool flip = p.facingLeft;
  if (state_ == S_CLEAR) f = (stateTime_ / 14) % 2;
  if (state_ == S_DYING) row = A_HURT, f = 0;
  drawChar(b, rabbit_, row, f, flip, p.x, p.y);
}

// le compagnon : son image est celle du trace, 10 images plus tot (il n'est pas dans la logique)
void Game::drawFollower(Band& b) {
  if (compDown_ > 0) return;
  Snap s = trailAt(TRAIL_DELAY);
  drawChar(b, !rabbit_, s.anim, s.frame, s.left, s.x, s.y);
}

// dessine le renard (rabbit = false) ou Andrea ; (x, y) = coin haut-gauche de la boite de collision
void Game::drawChar(Band& b, bool rabbit, int row, int f, bool flip, float x, float y) {
  int cx = (int)camX_, cy = (int)camY_;
  int hx = (int)lroundf(x + PW / 2.0f) - cx;
  int feet = (int)lroundf(y + PH) - cy;
  const Sheet& sh = rabbit ? spr_rabbit : spr_fox;  // meme format de cases (48x48) et memes lignes
  if (row == A_CLIMB && sh.img->h / sh.fh <= A_CLIMB) {
    // pas encore d'images d'echelle : la pose du saut, retournee a chaque pas
    row = A_JUMP;
    flip = f != 0;
    f = 0;
  }
  if (row == A_GLIDE && sh.img->h / sh.fh <= A_GLIDE) {   // pas encore de planche de vol : la chute
    row = A_FALL;
    f = 0;
  }
  int sx = flip ? hx - (sh.fw - 1 - FOX_ANCHOR) : hx - FOX_ANCHOR;
  b.frame(sh, row * sh.cols + f, sx, feet - 47, flip);
}

void Game::drawHud(Band& b) {
  if (b.y0 > 20) return;
  // fond discret
  for (int i = 0; i < 3; i++) b.frame(spr_hud, i < p_.hp ? 0 : 1, 6 + i * 10, 6);
  b.frame(spr_hud, 2, 44, 6);
  char s[24];
  snprintf(s, sizeof s, "%d", hearts_);
  b.text(font_small, 55, 3, s, WHITE, SHADOW);
  b.frame(spr_hud, 3, 88, 6);
  snprintf(s, sizeof s, "×%d", lives_);
  b.text(font_small, 98, 3, s, WHITE, SHADOW);
  int secs = levelTime_ / 60;
  snprintf(s, sizeof s, "%d:%02d", secs / 60, secs % 60);
  b.text(font_small, SCREEN_W - 8 - textWidth(font_small, s), 3, s, WHITE, SHADOW);
  const char* name = lvl_.def->name;
  b.text(font_small, SCREEN_W - 52 - textWidth(font_small, name), 3, name, 0xC618, SHADOW);
}

void Game::centerText(Band& b, const Font& f, int y, const char* s, uint16_t c, int shadow) {
  b.text(f, (SCREEN_W - textWidth(f, s)) / 2, y, s, c, shadow);
}

void Game::drawTitle(Band& b) {
  // personnages au milieu de l'ecran
  int ground = SCREEN_H - 2 * TILE;
  int run = (frame_ / 6) % 6;
  b.frame(spr_fox, A_RUN * spr_fox.cols + run, 136 - FOX_ANCHOR, ground - 47);
  b.frame(spr_rabbit, A_RUN * spr_rabbit.cols + (frame_ / 8 + 2) % RABBIT_RUN_FRAMES, 168, ground - 47);
  // titre
  b.fillRect(0, 26, SCREEN_W, 58, 0x18E6);
  b.fillRect(0, 26, SCREEN_W, 2, ORANGE);
  b.fillRect(0, 82, SCREEN_W, 2, ORANGE);
  centerText(b, font_title, 30, "ZAJKA RUN", ORANGE, BLACK);
  centerText(b, font_small, 66, "Košice  →  Bratislava", WHITE, BLACK);
  if (save_.unlocked > 1) {
    // choix du niveau : < SVET 1-2 · Slovensky raj >
    char s[64];
    snprintf(s, sizeof s, "%s  SVET 1-%d · %s  %s", sel_ > 0 ? "‹" : " ", sel_ + 1, ALL_LEVELS[sel_]->name,
             sel_ + 1 < save_.unlocked ? "›" : " ");
    b.fillRect(0, 88, SCREEN_W, 16, 0x18E6);
    centerText(b, font_small, 89, s, YELLOW, BLACK);
  }
  if ((frame_ / 30) % 2 == 0) centerText(b, font_big, 108, "Stlač START", WHITE, BLACK);
  b.fillRect(0, SCREEN_H - 14, SCREEN_W, 14, 0x18E6);
  centerText(b, font_small, SCREEN_H - 13, "Slovingo · Zajka Friends", 0xBDF7, -1);
  if (volumeShow_) drawOverlay(b);
}

void Game::drawOverlay(Band& b) {
  char s[64];
  switch (state_) {
    case S_INTRO: {
      if (respawn_) {
        // retour au point de passage : juste les vies
        b.fillRect(110, 100, 100, 36, DARKBLUE);
        b.frame(spr_hud, 3, 140, 113);
        snprintf(s, sizeof s, "×%d", lives_);
        b.text(font_big, 154, 106, s, WHITE, BLACK);
        break;
      }
      bool rec = levelIndex_ < 8 && save_.bestTime[levelIndex_];
      b.fillRect(0, 70, SCREEN_W, rec ? 112 : 96, DARKBLUE);
      b.fillRect(0, 70, SCREEN_W, 2, ORANGE);
      b.fillRect(0, rec ? 180 : 164, SCREEN_W, 2, ORANGE);
      snprintf(s, sizeof s, "SVET 1-%d", levelIndex_ + 1);
      centerText(b, font_small, 78, s, YELLOW, BLACK);
      centerText(b, font_title, 92, lvl_.def->name, WHITE, BLACK);
      centerText(b, font_small, 128, lvl_.def->subtitle, 0xBDF7, BLACK);
      b.frame(spr_hud, 3, 140, 148);
      snprintf(s, sizeof s, "×%d", lives_);
      b.text(font_small, 152, 145, s, WHITE, BLACK);
      if (rec) {
        int bt = save_.bestTime[levelIndex_];
        snprintf(s, sizeof s, "Rekord  %d:%02d  ·  medovníky : %d", bt / 60, bt % 60, save_.bestHearts[levelIndex_]);
        centerText(b, font_small, 163, s, YELLOW, BLACK);
      }
      break;
    }
    case S_PAUSE:
      b.fillRect(80, 80, 160, 80, DARKBLUE);
      b.fillRect(80, 80, 160, 2, ORANGE);
      centerText(b, font_big, 88, "PAUZA", WHITE, BLACK);
      centerText(b, font_small, 118, "START  pokračovať", 0xBDF7, -1);
      centerText(b, font_small, 134, "SELECT  koniec", 0xBDF7, -1);
      break;
    case S_GAMEOVER:
      b.fillRect(0, 90, SCREEN_W, 60, DARKBLUE);
      centerText(b, font_title, 96, "KONIEC HRY", WHITE, BLACK);
      if (stateTime_ > 90) centerText(b, font_small, 132, "Stlač START", 0xBDF7, -1);
      break;
    case S_CLEAR: {
      // bulle d'Andrea
      if (stateTime_ > 40) {
        for (int i = 0; i < nent_; i++) {
          if (ent_[i].type != E_GOAL) continue;
          // la bulle sort de la bouche de pan Orol, ou d'Andrea a l'arret de tram
          int ax = (int)ent_[i].x - (int)camX_ + (lvl_.def->npc ? 44 : 36);
          int ay = (int)ent_[i].y - (int)camY_ - 26;
          const char* hint = lvl_.def->hint;
          int w = textWidth(font_small, hint) + 12;
          int bx = ax - w / 2;
          if (bx + w > SCREEN_W - 4) bx = SCREEN_W - 4 - w;
          b.fillRect(bx, ay, w, 18, WHITE);
          b.fillRect(bx - 1, ay + 1, 1, 16, WHITE);
          b.fillRect(bx + w, ay + 1, 1, 16, WHITE);
          b.fillRect(ax - 3, ay + 18, 7, 2, WHITE);
          b.fillRect(ax - 1, ay + 20, 3, 2, WHITE);
          b.text(font_small, bx + 6, ay + 3, hint, DARKBLUE, -1);
        }
      }
      if (stateTime_ > 90) {
        b.fillRect(60, 30, 200, (lvl_.nsecrets + lvl_.nhidden) > 0 ? 90 : 74, DARKBLUE);
        b.fillRect(60, 30, 200, 2, ORANGE);
        centerText(b, font_big, 36, "Výborne!", YELLOW, BLACK);
        snprintf(s, sizeof s, "Medovníky : %d", levelHearts_);
        centerText(b, font_small, 62, s, WHITE, -1);
        int secs = levelTime_ / 60;
        snprintf(s, sizeof s, "Čas : %d:%02d", secs / 60, secs % 60);
        centerText(b, font_small, 76, s, WHITE, -1);
        int total = lvl_.nsecrets + lvl_.nhidden;
        if (total > 0) {
          snprintf(s, sizeof s, "Tajomstvá : %d/%d", secretsFound_, total);
          centerText(b, font_small, 90, s, secretsFound_ == total ? YELLOW : WHITE, -1);
        }
        if (stateTime_ > 150 && (frame_ / 30) % 2 == 0) centerText(b, font_small, total > 0 ? 104 : 90, "Stlač A", 0xBDF7, -1);
      }
      break;
    }
    case S_END:
      b.fillRect(0, 60, SCREEN_W, 110, DARKBLUE);
      centerText(b, font_big, 70, "Pokračovanie nabudúce…", WHITE, BLACK);
      snprintf(s, sizeof s, "Ďalšia zastávka : %s", lvl_.def->next);
      centerText(b, font_small, 100, s, 0xBDF7, -1);
      snprintf(s, sizeof s, "Medovníky : %d", hearts_);
      centerText(b, font_small, 120, s, YELLOW, -1);
      if (stateTime_ > 90) centerText(b, font_small, 146, "Stlač START", WHITE, -1);
      break;
    default:
      break;
  }
  if (volumeShow_ > 0) {
    b.fillRect(100, 200, 120, 24, DARKBLUE);
    b.text(font_small, 108, 206, "Hlasitosť", WHITE, -1);
    for (int i = 0; i < 4; i++) b.fillRect(170 + i * 10, 208, 7, 8, i < audio_->volume() ? YELLOW : 0x4208);
  }
}
