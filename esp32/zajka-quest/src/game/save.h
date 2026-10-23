// Sauvegarde de la partie : un petit bloc d'octets, ecrit par la plateforme ou elle veut
// (NVS sur la console, memoire ou fichier dans le simulateur). Portable : aucun Arduino.
#pragma once
#include <stddef.h>
#include <stdint.h>

// Ce que la plateforme doit savoir faire.
class Storage {
 public:
  virtual ~Storage() {}
  virtual bool read(void* data, size_t size) = 0;   // false : rien d'enregistre (ou taille differente)
  virtual bool write(const void* data, size_t size) = 0;
  virtual void erase() = 0;
};

constexpr uint32_t SAVE_MAGIC = 0x3153515A;  // « ZQS1 »
constexpr uint8_t SAVE_VERSION = 3;          // 3 : objets, mots, chapitres (une sauvegarde plus ancienne est ignoree)
constexpr int SAVE_EPISODES = 16;
constexpr int SAVE_ITEMS = 16;
constexpr int SAVE_BITS = 8;                 // 64 objets a ramasser, 64 mots de panneaux

struct SaveData {
  uint32_t magic = SAVE_MAGIC;
  uint8_t version = SAVE_VERSION;
  uint8_t map = 0;        // index dans ALL_MAPS
  uint8_t tx = 0, ty = 0;
  int8_t dir = 0;
  uint8_t chapter = 0;    // 0 : Kosice, 1 : Sucho
  uint8_t stage = 0;      // episodes du chapitre joues en entier
  uint8_t reserved0 = 0;
  uint16_t flags = 0;     // drapeaux du chapitre (chapter1.cpp, chapter2.cpp)
  uint16_t reserved1 = 0;
  uint8_t items[SAVE_ITEMS] = {};      // combien de chaque objet (ITEM_...)
  uint8_t picked[SAVE_BITS] = {};      // objets deja ramasses sur les cartes
  uint8_t words[SAVE_BITS] = {};       // mots des panneaux deja lus (Slovnik)
  uint8_t bonus[SAVE_EPISODES] = {};   // repliques secretes gagnees, par episode
  uint32_t check = 0;     // somme de controle des champs precedents (64 octets, sans bourrage)

  uint32_t sum() const {
    const uint8_t* p = (const uint8_t*)this;
    uint32_t h = 2166136261u;  // FNV-1a
    for (size_t i = 0; i < offsetof(SaveData, check); i++) h = (h ^ p[i]) * 16777619u;
    return h;
  }
  bool valid() const { return magic == SAVE_MAGIC && version == SAVE_VERSION && check == sum(); }
};

// Stockage en memoire (simulateur, tests) ; path non vide : garde aussi le bloc dans un fichier.
class MemoryStorage : public Storage {
 public:
  explicit MemoryStorage(const char* path = nullptr) : path_(path) {}
  bool read(void* data, size_t size) override;
  bool write(const void* data, size_t size) override;
  void erase() override;
  int writes() const { return writes_; }

 private:
  const char* path_;
  uint8_t buf_[96];
  size_t size_ = 0;
  int writes_ = 0;
};
