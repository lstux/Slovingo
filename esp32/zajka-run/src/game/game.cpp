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

// animations de la planche du renard / d'Andrea (une ligne par animation)
enum { A_IDLE, A_RUN, A_JUMP, A_FALL, A_HURT, A_WIN };
const int ANIM_LEN[] = {4, 6, 1, 1, 1, 2};

const uint16_t WHITE = 0xFFFF, BLACK = 0x0000, SHADOW = 0x18C3;
const uint16_t YELLOW = 0xFF2A, ORANGE = 0xFB00, DARKBLUE = 0x1926;

int ifloor(float v) { return (int)floorf(v); }
bool overlap(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh) {
  return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}
}  // namespace

// ---------------------------------------------------------------- mise en route
void Game::begin(Audio* audio) {
  audio_ = audio;
  state_ = S_TITLE;
  stateTime_ = 0;
  audio_->music(0);
  // le niveau 1 sert de decor a l'ecran titre
  startLevel(0, true);
  camX_ = 0;
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
      case 'p': k = spawn(E_PIGEON, tx, ty + 4); ent_[k].w = 18; ent_[k].h = 12; ent_[k].vx = -0.5f; break;
      case 'w': k = spawn(E_WASP, tx + 2, ty + 3); ent_[k].w = 12; ent_[k].h = 10; break;
      case 'P': k = spawn(E_SPRING, tx + 1, ty + 6); ent_[k].w = 14; ent_[k].h = 10; break;
      case 'C': k = spawn(E_CHECK, tx, ty + TILE - 48); ent_[k].w = 16; ent_[k].h = 48; break;
      case 'F': k = spawn(E_GOAL, tx, ty + TILE - 48); ent_[k].w = 52; ent_[k].h = 48; break;
      case 'T': k = spawn(E_TRAM, tx, ty + TILE - 48); ent_[k].w = 96; ent_[k].h = 48; ent_[k].dir = 1; break;
      case 'l': k = spawn(E_BENCH, tx, ty); break;
      case 'k': k = spawn(E_FLOWERS, tx, ty); break;
      default: break;
    }
  }
  respawn_ = false;
  if (fresh) {
    checkX_ = lvl_.startX;
    checkY_ = lvl_.startY;
    levelHearts_ = 0;
    levelTime_ = 0;
  }
  p_ = Player();
  p_.x = checkX_ + 1;
  p_.y = checkY_ - PH;
  updateCamera(true);
}

void Game::debugTeleport(float x) {
  checkX_ = x;
  checkY_ = lvl_.startY;
  p_.x = x;
  p_.y = checkY_ - PH;
  p_.vx = p_.vy = 0;
  updateCamera(true);
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
      if (in.hit(BTN_START) || in.hit(BTN_A)) {
        lives_ = 3;
        hearts_ = 0;
        startLevel(0, true);
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
        audio_->music(0);
      }
      break;
    case S_PLAY:
      if (in.hit(BTN_START)) {
        state_ = S_PAUSE;
        audio_->music(-1);
        audio_->play(SFX_PAUSE);
        break;
      }
      updatePlay(in);
      break;
    case S_PAUSE:
      if (in.hit(BTN_START)) {
        state_ = S_PLAY;
        audio_->music(0);
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
          audio_->music(0);
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
  updatePlayer(in);
  updateEntities();
  interact();
  if (p_.y > lvl_.pixelH() + 8) killPlayer();
  updateCamera(false);
}

void Game::updatePlayer(const Input& in) {
  Player& p = p_;
  bool left = in.down(BTN_LEFT), right = in.down(BTN_RIGHT);
  bool run = in.down(BTN_B);
  float maxv = run ? RUN_MAX : WALK_MAX;
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
  p.vy += hold ? GRAVITY_HOLD : GRAVITY;
  if (p.vy > MAX_FALL) p.vy = MAX_FALL;

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
  else if (!p.onGround) a = p.vy < 0 ? A_JUMP : A_FALL;
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
      if (k == K_SOLID || (k == K_ONEWAY && prevBottom <= ty * TILE + 0.01f)) {
        p.y = ty * TILE - PH;
        p.vy = 0;
        p.onGround = true;
        p.riding = -1;
        return;
      }
    }
    // toit du tram (plateforme mobile)
    for (int i = 0; i < nent_; i++) {
      Entity& e = ent_[i];
      if (e.type != E_TRAM) continue;
      float roof = e.y + TRAM_ROOF;
      if (prevBottom <= roof + 0.5f && bottom >= roof && p.x + PW > e.x + 4 && p.x < e.x + e.w - 4) {
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
      if (lvl_.kindAt(tx, ty) == K_SOLID && ty >= 0) {
        p.y = (ty + 1) * TILE;
        p.vy = 0.5f;
        bumpTile(tx, ty);
        return;
      }
    }
  }
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
  float px = p.x, py = p.y;
  // pointes
  int l = ifloor(px) >> 4, r = (ifloor(px) + PW - 1) >> 4;
  int t = ifloor(py) >> 4, b = (ifloor(py) + PH - 1) >> 4;
  for (int ty = t; ty <= b; ty++)
    for (int tx = l; tx <= r; tx++)
      if (lvl_.kindAt(tx, ty) == K_HAZARD && overlap(px, py, PW, PH, tx * TILE + 1, ty * TILE + 7, 14, 9))
        hurtPlayer(tx * TILE + 8);

  for (int i = 0; i < nent_; i++) {
    Entity& e = ent_[i];
    if (e.type == E_NONE) continue;
    switch (e.type) {
      case E_HEART:
        if (overlap(px, py, PW, PH, e.x, e.y, e.w, e.h)) {
          collectHeart(e.x - 2, e.y - 2, false);
          e.type = E_NONE;
        }
        break;
      case E_PIGEON:
      case E_WASP:
        if (e.state != 0) break;
        if (overlap(px + 1, py + 2, PW - 2, PH - 2, e.x, e.y, e.w, e.h)) {
          if (p.vy > 0 && py + PH - p.vy <= e.y + 6) {
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
          p.vx = 0;
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
  p.vx = (p.x + PW / 2 < fromX) ? -2.5f : 2.5f;
  p.vy = -3.5f;
  p.onGround = false;
  p.riding = -1;
  audio_->play(SFX_HURT);
}

void Game::killPlayer() {
  if (state_ != S_PLAY) return;
  state_ = S_DYING;
  stateTime_ = 0;
  p_.vy = -6.5f;
  p_.vx = 0;
  p_.anim = A_HURT;
  p_.riding = -1;
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
  if (state_ == S_TITLE) {
    drawTitle(b);
    return;
  }
  drawHud(b);
  drawOverlay(b);
}

void Game::drawWorld(Band& b) {
  const Theme& th = *lvl_.def->theme;
  int cx = (int)camX_ + (shake_ ? ((frame_ & 2) ? 2 : -2) : 0);
  int cy = (int)camY_;
  b.wrapRow(*th.far, cx / 5, -(cy / 10));
  int ground = lvl_.pixelH() - 2 * TILE - cy;
  b.wrapRow(*th.mid, cx / 2, ground - th.mid->h);
  // sous le niveau de la rue : terre sombre (les trous ne montrent pas le ciel)
  b.fillRect(0, ground, SCREEN_W, SCREEN_H - ground, 0x2945);
  b.fillRect(0, ground + 20, SCREEN_W, SCREEN_H - ground - 20, 0x18C3);

  // decor et objets derriere le joueur
  for (int i = 0; i < nent_; i++) {
    const Entity& e = ent_[i];
    if (e.type == E_BENCH || e.type == E_FLOWERS || e.type == E_CHECK || e.type == E_GOAL || e.type == E_TRAM)
      drawEntity(b, e);
  }

  // tuiles
  const Image8& ts = *th.tiles;
  int ty0 = (b.y0 + cy) >> 4, ty1 = (b.y0 + b.h - 1 + cy) >> 4;
  int tx0 = cx >> 4, tx1 = (cx + SCREEN_W - 1) >> 4;
  for (int ty = ty0; ty <= ty1; ty++) {
    for (int tx = tx0; tx <= tx1; tx++) {
      uint8_t t = lvl_.at(tx, ty);
      if (t == T_EMPTY || tx < 0 || tx >= lvl_.w) continue;
      int dy = ty * TILE - cy;
      if (bumpT_ > 0 && tx == bumpTx_ && ty == bumpTy_) dy -= (bumpT_ > 4 ? 8 - bumpT_ : bumpT_);
      b.blit(ts, (t % 8) * TILE, (t / 8) * TILE, TILE, TILE, tx * TILE - cx, dy);
    }
  }

  for (int i = 0; i < nent_; i++) {
    const Entity& e = ent_[i];
    if (e.type == E_NONE || e.type == E_BENCH || e.type == E_FLOWERS || e.type == E_CHECK ||
        e.type == E_GOAL || e.type == E_TRAM || e.type == E_DUST || e.type == E_SPARK || e.type == E_TEXT)
      continue;
    drawEntity(b, e);
  }
  if (state_ != S_TITLE) drawPlayer(b);
  for (int i = 0; i < nent_; i++) {
    const Entity& e = ent_[i];
    if (e.type == E_DUST || e.type == E_SPARK || e.type == E_TEXT) drawEntity(b, e);
  }
}

void Game::drawEntity(Band& b, const Entity& e) {
  int cx = (int)camX_, cy = (int)camY_;
  int sx = (int)lroundf(e.x) - cx, sy = (int)lroundf(e.y) - cy;
  if (sx < -110 || sx > SCREEN_W + 10) return;
  if (sy > b.y0 + b.h || sy + 64 < b.y0) return;
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
      if (e.state == 0) b.frame(spr_holub, (e.timer / 10) % 2, sx - 3, sy - 4, e.vx < 0);
      else b.frame(spr_holub, 2, sx - 3, sy - 4, e.vx < 0);
      break;
    case E_WASP:
      b.frame(spr_osa, (frame_ / 3) % 2, sx - 2, sy - 3, e.dir < 0);
      break;
    case E_SPRING:
      b.frame(spr_pruzina, e.timer > 0 ? 1 : 0, sx - 1, sy - 6);
      break;
    case E_CHECK:
      b.frame(spr_lampa, e.state ? 1 : 0, sx, sy);
      break;
    case E_GOAL: {
      b.frame(spr_zastavka, 0, sx, sy);
      // Andrea attend a l'arret
      bool cleared = state_ == S_CLEAR;
      int row = cleared ? A_WIN : A_IDLE;
      int f = cleared ? (stateTime_ / 14) % 2 : (e.timer / 14) % 4;
      b.frame(spr_rabbit, row * spr_rabbit.cols + f, sx + 20, sy, true);
      break;
    }
    case E_TRAM:
      b.frame(spr_elektricka, 0, sx, sy, e.dir < 0);
      break;
    case E_BENCH:
      b.frame(spr_lavicka, 0, sx, sy);
      break;
    case E_FLOWERS:
      b.frame(spr_lavicka, 1, sx, sy);
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
  int cx = (int)camX_, cy = (int)camY_;
  int hx = (int)lroundf(p.x + PW / 2.0f) - cx;
  int feet = (int)lroundf(p.y + PH) - cy;
  int row = p.anim, f = p.animFrame;
  if (state_ == S_CLEAR) f = (stateTime_ / 14) % 2;
  if (state_ == S_DYING) row = A_HURT, f = 0;
  int sx = p.facingLeft ? hx - (spr_fox.fw - 1 - FOX_ANCHOR) : hx - FOX_ANCHOR;
  b.frame(spr_fox, row * spr_fox.cols + f, sx, feet - 47, p.facingLeft);
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
  b.frame(spr_rabbit, A_RUN * spr_rabbit.cols + (run + 3) % 6, 168, ground - 47);
  // titre
  b.fillRect(0, 26, SCREEN_W, 58, 0x18E6);
  b.fillRect(0, 26, SCREEN_W, 2, ORANGE);
  b.fillRect(0, 82, SCREEN_W, 2, ORANGE);
  centerText(b, font_title, 30, "ZAJKA RUN", ORANGE, BLACK);
  centerText(b, font_small, 66, "Košice  →  Bratislava", WHITE, BLACK);
  if ((frame_ / 30) % 2 == 0) centerText(b, font_big, 104, "Stlač START", WHITE, BLACK);
  b.fillRect(0, SCREEN_H - 14, SCREEN_W, 14, 0x18E6);
  centerText(b, font_small, SCREEN_H - 13, "Slovingo · Zajka Friends", 0xBDF7, -1);
  if (volumeShow_) drawOverlay(b);
}

void Game::drawOverlay(Band& b) {
  char s[48];
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
      b.fillRect(0, 70, SCREEN_W, 96, DARKBLUE);
      b.fillRect(0, 70, SCREEN_W, 2, ORANGE);
      b.fillRect(0, 164, SCREEN_W, 2, ORANGE);
      snprintf(s, sizeof s, "SVET 1-%d", levelIndex_ + 1);
      centerText(b, font_small, 78, s, YELLOW, BLACK);
      centerText(b, font_title, 92, lvl_.def->name, WHITE, BLACK);
      centerText(b, font_small, 128, lvl_.def->subtitle, 0xBDF7, BLACK);
      b.frame(spr_hud, 3, 140, 148);
      snprintf(s, sizeof s, "×%d", lives_);
      b.text(font_small, 152, 145, s, WHITE, BLACK);
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
          int ax = (int)ent_[i].x - (int)camX_ + 36, ay = (int)ent_[i].y - (int)camY_ - 26;
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
        b.fillRect(60, 30, 200, 74, DARKBLUE);
        b.fillRect(60, 30, 200, 2, ORANGE);
        centerText(b, font_big, 36, "Výborne!", YELLOW, BLACK);
        snprintf(s, sizeof s, "Medovníky : %d", levelHearts_);
        centerText(b, font_small, 62, s, WHITE, -1);
        int secs = levelTime_ / 60;
        snprintf(s, sizeof s, "Čas : %d:%02d", secs / 60, secs % 60);
        centerText(b, font_small, 76, s, WHITE, -1);
        if (stateTime_ > 150 && (frame_ / 30) % 2 == 0) centerText(b, font_small, 90, "Stlač A", 0xBDF7, -1);
      }
      break;
    }
    case S_END:
      b.fillRect(0, 60, SCREEN_W, 110, DARKBLUE);
      centerText(b, font_big, 70, "Pokračovanie nabudúce…", WHITE, BLACK);
      centerText(b, font_small, 100, "Ďalšia zastávka : Slovenský raj", 0xBDF7, -1);
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
