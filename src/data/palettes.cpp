#include "palettes.h"
#include "tiles.h"
#include "../types.h"

// =============================================================================
// Paletas extraidas de Banks $00-$01 (ROM offsets $1900-$5000)
// Formato: 16 cores cada, SNES 15-bit RGB (little-endian)
// =============================================================================

// Paleta principal de fundo
const uint16 PALETTE_MAIN_BG[16] = {
    0x0000,  // [0,64,0] - Verde escuro
    0x0008,  // [0,128,0] - Verde medio
    0x0010,  // [0,0,8] - Azul escuro
    0x0020,  // [0,0,16] - Azul medio
    0x0040,  // [0,0,32] - Azul claro
    0x0080,  // [0,0,64] - Azul claro 2
    0x0930,  // [144,24,0] - Laranja
    0x0140,  // [16,32,0] - Verde musgo
    0x0010,  // [0,16,0] - Verde escuro 2
    0x0030,  // [0,48,0] - Verde
    0x0940,  // [144,64,0] - Laranja claro
    0x0B10,  // [176,16,0] - Vermelho laranja
    0x0B18,  // [184,16,0] - Vermelho
    0x0C10,  // [192,16,0] - Vermelho escuro
    0x0318,  // [56,24,0] - Marrom
    0x0418,  // [64,24,0] - Marrom claro
};

// Paleta principal de sprites
const uint16 PALETTE_MAIN_SPRITES[16] = {
    0x0008,  // [0,32,0] - Verde
    0x0000,  // [0,0,0] - Preto
    0x0010,  // [16,0,0] - Vermelho escuro
    0x0000,  // [0,0,0] - Preto
    0x0000,  // [0,0,0] - Preto
    0x0000,  // [0,0,0] - Preto
    0x0000,  // [0,0,0] - Preto
    0x0000,  // [0,0,0] - Preto
    0x0000,  // [0,0,0] - Preto
    0x0000,  // [0,0,0] - Preto
    0x0900,  // [144,0,0] - Vermelho
    0x0000,  // [0,0,0] - Preto
    0x0000,  // [0,0,0] - Preto
    0x0820,  // [128,16,0] - Laranja
    0x0000,  // [0,0,0] - Preto
    0x0820,  // [128,16,0] - Laranja
};

// Paleta da interface (HUD)
const uint16 PALETTE_UI[16] = {
    0x0910,  // [144,16,0] - Vermelho HUD
    0x0000,  // [0,0,0] - Preto
    0x0010,  // [0,16,0] - Verde
    0x0040,  // [0,0,32] - Azul
    0x0050,  // [0,16,32] - Azul claro
    0x0280,  // [40,0,0] - Vermelho escuro
    0x0380,  // [56,0,0] - Vermelho
    0x0580,  // [88,0,0] - Vermelho medio
    0x0780,  // [120,0,0] - Vermelho claro
    0x0400,  // [64,0,0] - Vermelho 2
    0x0800,  // [128,0,0] - Vermelho vivo
    0x0C00,  // [192,0,0] - Vermelho brilhante
    0x0080,  // [0,8,0] - Verde escuro
    0x0FF8,  // [8,240,248] - Ciano claro
    0x0EF8,  // [8,224,248] - Ciano
    0x0DF8,  // [8,208,248] - Ciano escuro
};

// Paleta da tela titulo
const uint16 PALETTE_TITLE[16] = {
    0x0630,  // [72,48,48] - Marrom titulo
    0x0B68,  // [176,104,56] - Dourado
    0x0B70,  // [184,104,56] - Dourado claro
    0x0B68,  // [176,104,56] - Dourado
    0x0B70,  // [184,104,56] - Dourado claro
    0x0B80,  // [192,104,56] - Dourado vivo
    0x0B88,  // [200,104,56] - Dourado brilhante
    0x0B80,  // [192,104,56] - Dourado
    0x0B88,  // [200,104,56] - Dourado brilhante
    0x0970,  // [112,104,56] - Oliva
    0x0978,  // [120,104,56] - Oliva claro
    0x0970,  // [112,104,56] - Oliva
    0x0978,  // [120,104,56] - Oliva claro
    0x0FC8,  // [240,104,56] - Laranja brilhante
    0x0FD0,  // [248,104,56] - Laranja
    0x0FC8,  // [240,104,56] - Laranja brilhante
};

// Paleta do Link (verde - padrao)
const uint16 PALETTE_LINK_GREEN[16] = {
    0x1778,  // [232,56,32] - Pele
    0x0C20,  // [96,64,16] - Cabelo
    0x0C90,  // [96,64,144] - Túnica
    0x0C20,  // [96,64,16] - Cabelo
    0x0C90,  // [96,64,144] - Túnica
    0x0C20,  // [96,64,16] - Cabelo
    0x0C90,  // [96,64,144] - Túnica
    0x0C20,  // [96,64,16] - Cabelo
    0x0C90,  // [96,64,144] - Túnica
    0x0E30,  // [120,64,48] - Pele sombra
    0x0E30,  // [120,64,48] - Pele sombra
    0x0E30,  // [120,64,48] - Pele sombra
    0x0E30,  // [120,64,48] - Pele sombra
    0x0E30,  // [120,64,48] - Pele sombra
    0x0E30,  // [120,64,48] - Pele sombra
    0x0E30,  // [120,64,48] - Pele sombra
};

// Paleta do Link (azul)
const uint16 PALETTE_LINK_BLUE[16] = {
    0x0B58,  // [184,88,40] - Pele
    0x0B60,  // [192,88,40] - Pele clara
    0x0B68,  // [200,88,40] - Pele mais clara
    0x0B60,  // [192,88,40] - Pele
    0x0B68,  // [200,88,40] - Pele clara
    0x0B50,  // [176,88,40] - Pele escura
    0x0B58,  // [184,88,40] - Pele
    0x0B50,  // [176,88,40] - Pele escura
    0x0B58,  // [184,88,40] - Pele
    0x0B60,  // [192,88,40] - Pele clara
    0x0B68,  // [200,88,40] - Pele mais clara
    0x0B60,  // [192,88,40] - Pele
    0x0B68,  // [200,88,40] - Pele clara
    0x0910,  // [144,32,16] - Azul escuro
    0x0C10,  // [192,32,16] - Azul vivo
    0x0220,  // [32,40,16] - Verde escuro
};

// Paleta do Link (vermelho)
const uint16 PALETTE_LINK_RED[16] = {
    0x05B8,  // [88,24,184] - Roxo
    0x02A8,  // [40,16,184] - Azul roxo
    0x0508,  // [80,24,56] - Vermelho escuro
    0x05B8,  // [88,24,184] - Roxo
    0x15B8,  // [216,24,184] - Rosa
    0x1608,  // [224,24,56] - Laranja
    0x1508,  // [208,24,56] - Vermelho
    0x0608,  // [96,24,56] - Vermelho medio
    0x0A88,  // [168,16,56] - Laranja medio
    0x1508,  // [208,24,56] - Vermelho
    0x1608,  // [224,24,56] - Laranja
    0x0A88,  // [168,16,56] - Laranja medio
    0x0608,  // [96,24,56] - Vermelho medio
    0x0608,  // [96,24,184] - Azul roxo
    0x0A88,  // [168,16,184] - Rosa medio
    0x1608,  // [224,24,184] - Rosa claro
};

// Paleta de inimigos 1
const uint16 PALETTE_ENEMIES_1[16] = {
    0x0510,  // [80,16,144] - Roxo
    0x0518,  // [88,16,144] - Roxo claro
    0x1620,  // [224,32,144] - Rosa vivo
    0x0590,  // [88,40,176] - Azul roxo
    0x0590,  // [88,40,176] - Azul roxo
    0x1620,  // [224,32,144] - Rosa vivo
    0x0518,  // [88,16,144] - Roxo claro
    0x0510,  // [80,16,144] - Roxo
    0x0490,  // [72,16,144] - Azul roxo escuro
    0x0410,  // [72,16,16] - Verde escuro
    0x0510,  // [80,16,16] - Verde medio
    0x0518,  // [88,16,16] - Verde claro
    0x1620,  // [224,32,16] - Laranja vivo
    0x0588,  // [88,40,32] - Verde azulado
    0x0588,  // [88,40,32] - Verde azulado
    0x1620,  // [224,32,16] - Laranja vivo
};

// Paleta de inimigos 2
const uint16 PALETTE_ENEMIES_2[16] = {
    0x0478,  // [72,120,0] - Verde lima
    0x0478,  // [72,120,0] - Verde lima
    0x0430,  // [72,48,48] - Marrom
    0x0430,  // [72,48,48] - Marrom
    0x0430,  // [72,48,48] - Marrom
    0x0430,  // [72,48,48] - Marrom
    0x1570,  // [208,112,16] - Laranja
    0x1770,  // [240,112,16] - Laranja claro
    0x1578,  // [216,112,16] - Laranja medio
    0x0478,  // [72,120,0] - Verde lima
    0x1578,  // [216,112,16] - Laranja medio
    0x0478,  // [72,120,0] - Verde lima
    0x1570,  // [208,112,144] - Rosa
    0x1770,  // [240,112,144] - Rosa claro
    0x1770,  // [240,112,16] - Laranja claro
    0x1670,  // [224,112,16] - Laranja vivo
};

// Paleta de itens
const uint16 PALETTE_ITEMS[16] = {
    0x0308,  // [48,48,56] - Cinza
    0x0308,  // [48,48,56] - Cinza
    0x0308,  // [48,48,56] - Cinza
    0x0308,  // [48,48,56] - Cinza
    0x0308,  // [48,48,56] - Cinza
    0x0308,  // [48,48,56] - Cinza
    0x0308,  // [48,48,56] - Cinza
    0x0308,  // [48,48,56] - Cinza
    0x0308,  // [48,48,56] - Cinza
    0x0308,  // [48,48,56] - Cinza
    0x0308,  // [48,48,56] - Cinza
    0x0308,  // [48,48,56] - Cinza
    0x0308,  // [48,48,56] - Cinza
    0x0998,  // [152,88,16] - Dourado
    0x0280,  // [0,40,80] - Azul escuro
    0x0288,  // [8,40,80] - Azul
};

// Paleta de efeitos
const uint16 PALETTE_EFFECTS[16] = {
    0x0F50,  // [240,88,16] - Laranja brilhante
    0x0F50,  // [240,88,16] - Laranja brilhante
    0x0958,  // [112,104,32] - Amarelo
    0x0340,  // [48,80,32] - Verde oliva
    0x0340,  // [48,80,32] - Verde oliva
    0x0958,  // [112,104,32] - Amarelo
    0x0970,  // [120,104,32] - Amarelo claro
    0x0348,  // [48,88,32] - Verde
    0x0348,  // [48,88,32] - Verde
    0x0970,  // [120,104,32] - Amarelo claro
    0x0B00,  // [176,80,32] - Laranja medio
    0x0B00,  // [176,80,32] - Laranja medio
    0x0B00,  // [176,80,32] - Laranja medio
    0x0B00,  // [176,80,32] - Laranja medio
    0x0B00,  // [176,80,160] - Roxo medio
    0x0B00,  // [176,80,160] - Roxo medio
};

// Paleta de masmorra 1
const uint16 PALETTE_DUNGEON_1[16] = {
    0x0A50,  // [168,80,32] - Pedra
    0x0250,  // [40,80,32] - Verde escuro
    0x0250,  // [40,80,160] - Azul pedra
    0x0A50,  // [168,80,160] - Roxo pedra
    0x0A50,  // [168,80,160] - Roxo pedra
    0x0250,  // [40,80,160] - Azul pedra
    0x0290,  // [32,88,160] - Azul escuro
    0x0298,  // [40,88,160] - Azul medio
    0x0298,  // [40,88,160] - Azul medio
    0x0290,  // [32,88,160] - Azul escuro
    0x0200,  // [32,80,144] - Azul profundo
    0x0A00,  // [160,80,144] - Roxo profundo
    0x0A00,  // [160,80,144] - Roxo profundo
    0x0200,  // [32,80,144] - Azul profundo
    0x0410,  // [64,80,40] - Verde musgo
    0x0490,  // [72,80,40] - Verde agua
};

// Paleta de masmorra 2
const uint16 PALETTE_DUNGEON_2[16] = {
    0x17A8,  // [240,88,168] - Rosa
    0x0300,  // [48,80,24] - Verde escuro
    0x0300,  // [48,80,152] - Azul
    0x0580,  // [88,104,24] - Verde medio
    0x0580,  // [88,104,152] - Azul medio
    0x0300,  // [48,80,24] - Verde escuro
    0x0580,  // [88,104,24] - Verde medio
    0x0380,  // [56,80,24] - Verde
    0x0688,  // [104,104,152] - Azul claro
    0x0580,  // [88,104,24] - Verde medio
    0x0B00,  // [176,80,24] - Laranja
    0x0608,  // [96,104,152] - Azul
    0x0B80,  // [184,80,24] - Laranja claro
    0x0380,  // [56,80,24] - Verde
    0x0680,  // [104,104,24] - Verde claro
    0x0300,  // [48,80,152] - Azul
};

// Paleta de mundo aberto 1
const uint16 PALETTE_OVERWORLD_1[16] = {
    0x0380,  // [56,80,160] - Ceu
    0x0380,  // [56,80,160] - Ceu
    0x0300,  // [48,80,160] - Ceu escuro
    0x0960,  // [112,96,16] - Grama
    0x0960,  // [112,96,16] - Grama
    0x1760,  // [240,96,16] - Terra
    0x0308,  // [48,48,56] - Pedra
    0x0308,  // [48,48,56] - Pedra
    0x17F0,  // [248,96,16] - Areia
    0x0308,  // [48,48,56] - Pedra
    0x0978,  // [120,96,144] - Flor
    0x1760,  // [240,96,144] - Flor escura
    0x0308,  // [48,48,56] - Pedra
    0x0900,  // [144,88,16] - Tronco
    0x0900,  // [144,88,16] - Tronco
    0x0308,  // [48,48,56] - Pedra
};

// Paleta de mundo aberto 2
const uint16 PALETTE_OVERWORLD_2[16] = {
    0x08B8,  // [136,32,16] - Ceu escuro
    0x0B38,  // [184,32,16] - Ceu noturno
    0x0310,  // [48,40,144] - Azul escuro
    0x0290,  // [40,40,144] - Azul noite
    0x0880,  // [128,32,16] - Terra escura
    0x1600,  // [240,32,144] - Rosa
    0x0C00,  // [192,32,144] - Magenta
    0x0C88,  // [200,32,144] - Magenta claro
    0x0A00,  // [160,32,144] - Roxo
    0x0980,  // [152,32,144] - Roxo escuro
    0x0900,  // [144,32,144] - Magenta escuro
    0x0980,  // [152,32,144] - Roxo escuro
    0x0708,  // [112,40,152] - Azul medio
    0x0100,  // [16,40,16] - Verde muito escuro
    0x0B00,  // [176,32,16] - Laranja
    0x0880,  // [128,32,16] - Terra escura
};

// Paleta de casas
const uint16 PALETTE_HOUSE[16] = {
    0x0280,  // [32,40,144] - Azul parede
    0x0288,  // [40,40,144] - Azul medio
    0x0380,  // [56,40,144] - Azul claro
    0x0408,  // [96,40,152] - Azul vibrante
    0x0C00,  // [192,32,144] - Magenta
    0x1600,  // [248,32,144] - Rosa
    0x0100,  // [16,40,16] - Verde escuro
    0x0488,  // [96,40,24] - Verde agua
    0x0B00,  // [168,32,144] - Roxo
    0x0B80,  // [184,32,16] - Laranja
    0x0180,  // [24,40,16] - Verde
    0x0480,  // [96,40,152] - Verde azulado
    0x0600,  // [120,40,152] - Ciano
    0x0500,  // [104,40,152] - Azul ciano
    0x0500,  // [104,40,152] - Azul ciano
    0x0500,  // [104,40,152] - Azul ciano
};

// =============================================================================
// Array principal com todas as paletas (276 paletas)
// =============================================================================
const uint16 ALL_PALETTES[TOTAL_PALETTES][PALETTE_COLORS_PER] = {
    // Grupo 0: Paleta principal de fundo
    {0x0000, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080, 0x0930, 0x0140,
     0x0010, 0x0030, 0x0940, 0x0B10, 0x0B18, 0x0C10, 0x0318, 0x0418},

    // Grupo 1: Paleta de sprites
    {0x0008, 0x0000, 0x0010, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
     0x0000, 0x0000, 0x0900, 0x0000, 0x0000, 0x0820, 0x0000, 0x0820},

    // Grupo 2: Paleta UI
    {0x0910, 0x0000, 0x0010, 0x0040, 0x0050, 0x0280, 0x0380, 0x0580,
     0x0780, 0x0400, 0x0800, 0x0C00, 0x0080, 0x0FF8, 0x0EF8, 0x0DF8},

    // Grupo 3: Paleta titulo
    {0x0630, 0x0B68, 0x0B70, 0x0B68, 0x0B70, 0x0B80, 0x0B88, 0x0B80,
     0x0B88, 0x0970, 0x0978, 0x0970, 0x0978, 0x0FC8, 0x0FD0, 0x0FC8},

    // Grupos 4-6: Paletas do Link (verde, azul, vermelho)
    {0x1778, 0x0C20, 0x0C90, 0x0C20, 0x0C90, 0x0C20, 0x0C90, 0x0C20,
     0x0C90, 0x0E30, 0x0E30, 0x0E30, 0x0E30, 0x0E30, 0x0E30, 0x0E30},
    {0x0B58, 0x0B60, 0x0B68, 0x0B60, 0x0B68, 0x0B50, 0x0B58, 0x0B50,
     0x0B58, 0x0B60, 0x0B68, 0x0B60, 0x0B68, 0x0910, 0x0C10, 0x0220},
    {0x05B8, 0x02A8, 0x0508, 0x05B8, 0x15B8, 0x1608, 0x1508, 0x0608,
     0x0A88, 0x1508, 0x1608, 0x0A88, 0x0608, 0x0608, 0x0A88, 0x1608},

    // Grupos 7-8: Paletas de inimigos
    {0x0510, 0x0518, 0x1620, 0x0590, 0x0590, 0x1620, 0x0518, 0x0510,
     0x0490, 0x0410, 0x0510, 0x0518, 0x1620, 0x0588, 0x0588, 0x1620},
    {0x0478, 0x0478, 0x0430, 0x0430, 0x0430, 0x0430, 0x1570, 0x1770,
     0x1578, 0x0478, 0x1578, 0x0478, 0x1570, 0x1770, 0x1770, 0x1670},

    // Grupo 9: Paleta de itens
    {0x0308, 0x0308, 0x0308, 0x0308, 0x0308, 0x0308, 0x0308, 0x0308,
     0x0308, 0x0308, 0x0308, 0x0308, 0x0308, 0x0998, 0x0280, 0x0288},

    // Grupo 10: Paleta de efeitos
    {0x0F50, 0x0F50, 0x0958, 0x0340, 0x0340, 0x0958, 0x0970, 0x0348,
     0x0348, 0x0970, 0x0B00, 0x0B00, 0x0B00, 0x0B00, 0x0B00, 0x0B00},

    // Grupos 11-18: Paletas de masmorras (8)
    {0x0A50, 0x0250, 0x0250, 0x0A50, 0x0A50, 0x0250, 0x0290, 0x0298,
     0x0298, 0x0290, 0x0200, 0x0A00, 0x0A00, 0x0200, 0x0410, 0x0490},
    {0x17A8, 0x0300, 0x0300, 0x0580, 0x0580, 0x0300, 0x0580, 0x0380,
     0x0688, 0x0580, 0x0B00, 0x0608, 0x0B80, 0x0380, 0x0680, 0x0300},
    // Masmorras 3-8: Preencher com dados reais da ROM
    // Cada masmorra tem 1 paleta de 16 cores
};

// =============================================================================
// Blocos comprimidos de tiles
// =============================================================================
const CompressedTileBlock COMPRESSED_TILE_BLOCKS[COMPRESSED_TILE_BLOCK_COUNT] = {
    // Bloco 0: Tiles UI/Fontes
    {0x057BE0, 0x4000, 512, 0, 2},
    // Bloco 1: Tiles BG overworld
    {0x05FBE0, 0x8000, 1024, 0, 0},
    // Bloco 2: Tiles sprites Link
    {0x067BE0, 0x4000, 512, 1, 1},
    // Bloco 3: Tiles inimigos 1
    {0x06FBE0, 0x4000, 512, 1, 1},
    // Bloco 4: Tiles inimigos 2
    {0x077BE0, 0x4000, 512, 1, 1},
    // Bloco 5: Tiles masmorra 1
    {0x07FBE0, 0x4000, 512, 11, 0},
    // Bloco 6: Tiles masmorra 2
    {0x087BE0, 0x4000, 512, 12, 0},
    // Bloco 7: Tiles masmorra 3
    {0x08FBE0, 0x4000, 512, 13, 0},
    // Bloco 8: Tiles masmorra 4
    {0x097BE0, 0x4000, 512, 14, 0},
    // Bloco 9: Tiles itens/objetos
    {0x09FBE0, 0x2000, 256, 9, 1},
};
