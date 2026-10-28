// Boutons de la console (esp32/zajka-*/src/game/input.h) et sources d'entree du navigateur :
// clavier, manette (API Gamepad) et boutons tactiles a l'ecran.
(function (root) {
  'use strict';

  const BTN_UP = 1 << 0, BTN_DOWN = 1 << 1, BTN_LEFT = 1 << 2, BTN_RIGHT = 1 << 3,
    BTN_A = 1 << 4,       // sauter / valider
    BTN_B = 1 << 5,       // courir / annuler
    BTN_START = 1 << 6, BTN_SELECT = 1 << 7, BTN_MENU = 1 << 8, BTN_VOLUME = 1 << 9;

  class Input {
    constructor() { this.held = 0; this.pressed = 0; this.released = 0; }
    update(raw) {
      this.pressed = raw & ~this.held;
      this.released = this.held & ~raw;
      this.held = raw;
    }
    down(b) { return (this.held & b) !== 0; }
    hit(b) { return (this.pressed & b) !== 0; }
  }

  // Touches du clavier -> boutons. On utilise la position physique de la touche (event.code) :
  // « KeyW / KeyA / KeyS / KeyD » correspondent a ZQSD sur un clavier AZERTY et a WASD en QWERTY.
  const KEYS = {
    ArrowUp: BTN_UP, KeyW: BTN_UP,
    ArrowDown: BTN_DOWN, KeyS: BTN_DOWN,
    ArrowLeft: BTN_LEFT, KeyA: BTN_LEFT,
    ArrowRight: BTN_RIGHT, KeyD: BTN_RIGHT,
    Space: BTN_A, KeyK: BTN_A,
    ShiftLeft: BTN_B, ShiftRight: BTN_B, KeyJ: BTN_B, KeyX: BTN_B,
    Enter: BTN_START, KeyP: BTN_START,
    Tab: BTN_SELECT, Backspace: BTN_SELECT, KeyC: BTN_SELECT,
    KeyV: BTN_VOLUME,
  };

  // Sources : on garde l'etat « tenu » de chaque source, et les appuis brefs sont memorises jusqu'au
  // prochain pas de logique (60 Hz) pour qu'un tapotement tactile ne passe pas entre deux pas.
  class InputSources {
    constructor() {
      this.keys = 0;       // clavier
      this.touch = 0;      // boutons tactiles
      this.latch = 0;      // appuis a conserver jusqu'au prochain pas
      this.pad = 0;        // manette (relue a chaque pas)
      this.codes = new Set();
    }

    // renvoie true si la touche est utilisee par le jeu (le navigateur ne doit pas la traiter)
    keyDown(code) {
      const b = KEYS[code];
      if (!b) return false;
      this.codes.add(code);
      this.keys |= b; this.latch |= b;
      return true;
    }
    keyUp(code) {
      const b = KEYS[code];
      if (!b) return false;
      this.codes.delete(code);
      let keys = 0;
      for (const c of this.codes) keys |= KEYS[c];
      this.keys = keys;   // deux touches peuvent porter le meme bouton
      return true;
    }
    releaseAll() { this.codes.clear(); this.keys = 0; this.touch = 0; }

    setTouch(mask) { this.touch = mask; this.latch |= mask; }

    pollGamepad() {
      let b = 0;
      const pads = root.navigator && navigator.getGamepads ? navigator.getGamepads() : [];
      for (const p of pads) {
        if (!p || !p.connected) continue;
        const btn = (i) => !!(p.buttons[i] && p.buttons[i].pressed);
        const ax = p.axes[0] || 0, ay = p.axes[1] || 0;
        if (btn(12) || ay < -0.5) b |= BTN_UP;
        if (btn(13) || ay > 0.5) b |= BTN_DOWN;
        if (btn(14) || ax < -0.5) b |= BTN_LEFT;
        if (btn(15) || ax > 0.5) b |= BTN_RIGHT;
        if (btn(0)) b |= BTN_A;                  // A (bas)
        if (btn(1) || btn(2)) b |= BTN_B;        // B, X
        if (btn(3)) b |= BTN_A;                  // Y
        if (btn(9)) b |= BTN_START;
        if (btn(8)) b |= BTN_SELECT;
        if (btn(4) || btn(5)) b |= BTN_VOLUME;
      }
      this.pad = b;
    }

    // etat a donner a Input.update() pour le prochain pas de logique
    read() {
      this.pollGamepad();
      const raw = this.keys | this.touch | this.pad | this.latch;
      this.latch = 0;
      return raw;
    }
  }

  const ns = root.ZajkaCommon = root.ZajkaCommon || {};
  Object.assign(ns, {
    BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT, BTN_A, BTN_B, BTN_START, BTN_SELECT, BTN_MENU, BTN_VOLUME,
    Input, InputSources, KEYS,
  });
})(globalThis);
