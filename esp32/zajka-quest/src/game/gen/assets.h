// Genere par tools/build_assets.py, ne pas modifier a la main.
#pragma once
#include "../gfx.h"
#include "../world.h"

extern const Image8 img_spr_fox;
extern const Sheet spr_fox;
extern const Image8 img_spr_rabbit;
extern const Sheet spr_rabbit;
extern const Image8 img_walk_andrea;
extern const Sheet walk_andrea;
extern const Image8 img_walk_fox;
extern const Sheet walk_fox;
extern const Image8 img_walk_human;
extern const Sheet walk_human;
extern const Font font_small;
extern const Font font_big;
extern const Font font_title;
extern const Image8 portrait_andrea;
extern const Image8 portrait_fox;
extern const Image8* const PORTRAITS[SPK_COUNT];
constexpr int TL_VIDE = 0;
constexpr int TL_PAVES = 1;
constexpr int TL_PAVES2 = 2;
constexpr int TL_TROTTOIR = 3;
constexpr int TL_HERBE = 4;
constexpr int TL_FLEURS = 5;
constexpr int TL_TOIT = 6;
constexpr int TL_FACADE = 7;
constexpr int TL_PORTE = 8;
constexpr int TL_RAILS = 9;
constexpr int TL_ABRI = 10;
constexpr int TL_BANC = 11;
constexpr int TL_LAMPADAIRE = 12;
constexpr int TL_TRONC = 13;
constexpr int TL_FEUILLAGE = 14;
constexpr int TL_FONTAINE = 15;
constexpr int TL_TRAM = 16;
constexpr int TL_PORTE_TRAM = 17;
constexpr int TL_PLANCHER = 18;
constexpr int TL_SIEGE = 19;
constexpr int TL_PAROI = 20;
constexpr int TL_CAISSE = 21;
constexpr int TL_BARRE = 22;
constexpr int TL_MAISON = 23;
constexpr int TL_PORTE_MAISON = 24;
constexpr int TL_CLOTURE = 25;
constexpr int TL_PORTAIL = 26;
constexpr int TL_ALLEE = 27;
constexpr int TL_POMMIER = 28;
constexpr int TL_FEUILLAGE_POMMIER = 29;
constexpr int TL_BUISSON = 30;
extern const Image8 tiles_kosice;
extern const Tileset tileset_kosice;
constexpr int MAP_KOSICE_HLAVNA = 0;
constexpr int MAP_KOSICE_TETA = 1;
constexpr int MAP_KOSICE_TRAM = 2;
