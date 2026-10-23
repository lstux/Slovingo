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
constexpr uint8_t SAVE_VERSION = 1;

enum SaveFlag : uint8_t {
  SF_INTRO_DONE = 1 << 0,  // le Narrateur a plante le decor du tram
  SF_MET_ANDREA = 1 << 1,  // le dialogue de l'episode 1 a ete joue
};

struct SaveData {
  uint32_t magic = SAVE_MAGIC;
  uint8_t version = SAVE_VERSION;
  uint8_t map = 0;        // index dans ALL_MAPS
  uint8_t tx = 0, ty = 0;
  int8_t dir = 0;
  uint8_t flags = 0;
  uint8_t bonus = 0;      // repliques secretes gagnees
  uint8_t reserved = 0;
  uint32_t check = 0;     // somme de controle des champs precedents

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
  uint8_t buf_[64];
  size_t size_ = 0;
  int writes_ = 0;
};
