#ifndef PALETTES_H
#define PALETTES_H

#include "../types.h"

// =============================================================================
// Paletas extraidas de Banks $00-$01 (ROM offsets $1900-$5000)
// Formato SNES 15-bit RGB: gggrrrrr 0bbbbbgg (little-endian)
// Cada paleta tem 16 cores (32 bytes)
// =============================================================================

// Paleta principal de fundo (banco $00, offset $1900)
// Usada para tiles de terreno e background geral
extern const uint16 PALETTE_MAIN_BG[16];

// Paleta principal de sprites (banco $00, offset $1A00)
// Usada para sprites do Link e outros personagens
extern const uint16 PALETTE_MAIN_SPRITES[16];

// Paleta da interface (banco $00, offset $1B00)
// HUD, barras de vida, inventario
extern const uint16 PALETTE_UI[16];

// Paleta da tela titulo (banco $00, offset $1C00)
extern const uint16 PALETTE_TITLE[16];

// Paletas do Link (3 variacoes de cor)
extern const uint16 PALETTE_LINK_GREEN[16];   // Padrão (verde)
extern const uint16 PALETTE_LINK_BLUE[16];    // Tunicas azuis
extern const uint16 PALETTE_LINK_RED[16];     // Tunicas vermelhas

// Paletas de inimigos
extern const uint16 PALETTE_ENEMIES_1[16];    // Inimigos tipo 1
extern const uint16 PALETTE_ENEMIES_2[16];    // Inimigos tipo 2

// Paleta de itens (espadas, escudos, poções, etc.)
extern const uint16 PALETTE_ITEMS[16];

// Paleta de efeitos (magias, explosões, brilhos)
extern const uint16 PALETTE_EFFECTS[16];

// Paletas de masmorras (8 dungeons principais)
extern const uint16 PALETTE_DUNGEON_1[16];    // Castelo da Agonia
extern const uint16 PALETTE_DUNGEON_2[16];    // Caverna do Trovão

// Paletas de mundo aberto
extern const uint16 PALETTE_OVERWORLD_1[16];  // Hyperlink
extern const uint16 PALETTE_OVERWORLD_2[16];  // Dark World

// Paleta de interiores de casas
extern const uint16 PALETTE_HOUSE[16];

// =============================================================================
// Todas as paletas (276 paletas = 4416 bytes)
// Organizadas em blocos de 16 paletas por grupo
// =============================================================================

#define TOTAL_PALETTES 276
#define PALETTE_COLORS_PER 16

// Array principal com todas as paletas
// Formato: ALL_PALETTES[grupo][paleta][cor]
extern const uint16 ALL_PALETTES[TOTAL_PALETTES][PALETTE_COLORS_PER];

// =============================================================================
// Mapeamento de paletas por uso
// =============================================================================

// Indices dos grupos de paletas
enum PaletteGroup : uint8 {
    PAL_GROUP_BG_MAIN = 0,       // Fundo principal
    PAL_GROUP_SPRITES = 1,       // Sprites
    PAL_GROUP_UI = 2,            // Interface
    PAL_GROUP_TITLE = 3,         // Titulo
    PAL_GROUP_LINK = 4,          // Link (3 paletas)
    PAL_GROUP_ENEMIES = 7,       // Inimigos (2 paletas)
    PAL_GROUP_ITEMS = 9,         // Itens
    PAL_GROUP_EFFECTS = 10,      // Efeitos
    PAL_GROUP_DUNGEONS = 11,     // Masmorras (8 paletas)
    PAL_GROUP_OVERWORLD = 19,    // Mundo aberto (2 paletas)
    PAL_GROUP_HOUSE = 21,        // Casas
    PAL_GROUP_CUSTOM = 22,       // Paletas customizadas
};

// =============================================================================
// Funcoes auxiliares para manipulacao de paletas
// =============================================================================

// Converte cor SNES 15-bit para RGB 8-bit
inline void snes_to_rgb(uint16 snes_color, uint8* r, uint8* g, uint8* b) {
    *r = (snes_color & 0x1F) << 3;
    *g = ((snes_color >> 5) & 0x1F) << 3;
    *b = ((snes_color >> 10) & 0x1F) << 3;
}

// Converte RGB 8-bit para cor SNES 15-bit
inline uint16 rgb_to_snes(uint8 r, uint8 g, uint8 b) {
    return ((b >> 3) << 10) | ((g >> 3) << 5) | (r >> 3);
}

// Converte cor SNES para 32-bit RGBA
inline uint32 snes_to_rgba(uint16 snes_color, uint8 alpha = 255) {
    uint8 r, g, b;
    snes_to_rgb(snes_color, &r, &g, &b);
    return (static_cast<uint32>(alpha) << 24) |
           (static_cast<uint32>(b) << 16) |
           (static_cast<uint32>(g) << 8) |
           static_cast<uint32>(r);
}

// Copia 16 cores de uma paleta
inline void copy_palette(const uint16* src, uint16* dst) {
    for (int i = 0; i < 16; i++) {
        dst[i] = src[i];
    }
}

// Aplica brightness a uma paleta (0-31)
inline void apply_brightness(uint16* palette, uint8 brightness) {
    for (int i = 0; i < 16; i++) {
        uint8 r = (palette[i] & 0x1F);
        uint8 g = ((palette[i] >> 5) & 0x1F);
        uint8 b = ((palette[i] >> 10) & 0x1F);

        r = (r * brightness) >> 5;
        g = (g * brightness) >> 5;
        b = (b * brightness) >> 5;

        palette[i] = (b << 10) | (g << 5) | r;
    }
}

#endif // PALETTES_H
