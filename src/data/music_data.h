#ifndef MUSIC_DATA_H
#define MUSIC_DATA_H

#include "../types.h"

// =============================================================================
// Dados de audio - Zelda: A Link to the Past
// Banks $0A-$1F contem samples BRR e sequencias musicais
// =============================================================================

// =============================================================================
// Constantes de audio SNES
// =============================================================================

// Tamanho de um bloco BRR em bytes
constexpr int BRR_BLOCK_SIZE = 10;

// Samples por bloco BRR
constexpr int BRR_SAMPLES_PER_BLOCK = 16;

// Taxa de amostragem base do SPC700
constexpr int SPC_SAMPLE_RATE = 32000;

// Numero de canais do SPC700
constexpr int SPC_CHANNEL_COUNT = 8;

// Numero de instrumentos
constexpr int INSTRUMENT_COUNT = 64;

// Numero de musicas
constexpr int MUSIC_COUNT = 65;

// Numero de sound effects
constexpr int SFX_COUNT = 96;

// =============================================================================
// Estrutura de um sample BRR
// =============================================================================

// Header de um bloco BRR
struct BRRBlockHeader {
    uint8 range : 4;        // Range (0-12)
    uint8 end : 1;          // Flag de fim
    uint8 loop : 2;         // Flag de loop
    uint8 filter : 3;       // Indice do filtro
};

// Informacoes de um sample BRR
struct BRRSample {
    uint32 rom_offset;      // Offset na ROM
    uint16 block_count;     // Numero de blocos BRR
    uint16 total_size;      // Tamanho total em bytes
    uint16 loop_offset;     // Offset do ponto de loop
    uint16 pitch;           // Pitch base
    uint8  filter;          // Filtro padrao
    uint8  adsr_attack;     // ADSR: Attack
    uint8  adsr_decay;      // ADSR: Decay
    uint8  adsr_sustain;    // ADSR: Sustain
    uint8  adsr_release;    // ADSR: Release
    const char* name;       // Nome do sample
};

// =============================================================================
// Tabela de samples BRR
// =============================================================================

constexpr int BRR_SAMPLE_COUNT = 128;
extern const BRRSample BRR_SAMPLES[BRR_SAMPLE_COUNT];

// =============================================================================
// Estrutura de um instrumento
// =============================================================================

struct Instrument {
    uint8  sample_index;        // Indice do sample BRR
    uint16 pitch_base;          // Pitch base
    uint8  pitch_range;         // Range do pitch
    uint8  adsr_attack;         // ADSR: Attack
    uint8  adsr_decay;          // ADSR: Decay
    uint8  adsr_sustain;        // ADSR: Sustain
    uint8  adsr_release;        // ADSR: Release
    uint8  gain;                // Gain
    uint8  harmony_type;        // Tipo de harmonia (0=normal, 1=octava, etc)
    uint8  effect_type;         // Tipo de efeito
    uint8  effect_depth;        // Profundidade do efeito
    uint8  reserved;            // Reservado
};

// Tabela de instrumentos
extern const Instrument INSTRUMENTS[INSTRUMENT_COUNT];

// =============================================================================
// Estrutura de uma musica
// =============================================================================

// Header de uma musica
struct MusicTrack {
    const char* name;                   // Nome da musica
    uint32 sequence_offset;             // Offset da sequencia na ROM
    uint16 sequence_size;               // Tamanho da sequencia
    uint8  channel_count;               // Numero de canais ativos
    uint8  default_tempo;               // Tempo padrao
    uint8  default_volume;              // Volume padrao
    uint8  instrument_bank;             // Bank dos instrumentos
    uint32 instrument_table_offset;     // Offset da tabela de instrumentos
    uint8  sfx_channel;                 // Canal para sound effects
    uint8  priority;                    // Prioridade da musica
};

// Tabela de musicas
extern const MusicTrack MUSIC_TRACKS[MUSIC_COUNT];

// =============================================================================
// Estrutura de um sound effect
// =============================================================================

struct SFXEntry {
    const char* name;                   // Nome do efeito
    uint32 data_offset;                 // Offset dos dados na ROM
    uint16 data_size;                   // Tamanho dos dados
    uint8  channel;                     // Canal preferido
    uint8  priority;                    // Prioridade
    uint8  repeat_count;                // Numero de repeticoes (0=infinite)
    uint16 pitch;                       // Pitch base
};

// Tabela de sound effects
extern const SFXEntry SFX_TABLE[SFX_COUNT];

// =============================================================================
// Sequencias musicais (formato SPC700)
// =============================================================================

// Notas musicais
enum MusicNote : uint8 {
    NOTE_REST = 0x80,
    NOTE_C = 0x81,
    NOTE_CSHARP = 0x82,
    NOTE_D = 0x83,
    NOTE_DSHARP = 0x84,
    NOTE_E = 0x85,
    NOTE_F = 0x86,
    NOTE_FSHARP = 0x87,
    NOTE_G = 0x88,
    NOTE_GSHARP = 0x89,
    NOTE_A = 0x8A,
    NOTE_ASHARP = 0x8B,
    NOTE_B = 0x8C,
    NOTE_END = 0xFF,
};

// Comandos de musica
enum MusicCommand : uint8 {
    CMD_INSTRUMENT = 0x00,      // Definir instrumento
    CMD_VOLUME = 0x01,          // Definir volume
    CMD_PAN = 0x02,             // Definir pan
    CMD_PITCH_BEND = 0x03,      // Pitch bend
    CMD_VIBRATO = 0x04,         // Vibrato
    CMD_TREMOLO = 0x05,         // Tremolo
    CMD_TEMPO = 0x06,           // Definir tempo
    CMD_TRANSPOSE = 0x07,       // Transposição
    CMD_LOOP_START = 0x08,      // Inicio de loop
    CMD_LOOP_END = 0x09,        // Fim de loop
    CMD_CALL = 0x0A,            // Chamar sub-rotina
    CMD_RETURN = 0x0B,          // Retornar
    CMD_GOTO = 0x0C,            // Pular para label
    CMD_PROG_BEND = 0x0D,       // Pitch bend progressivo
    CMD_SLUR = 0x0E,            // Ligado
    CMD_STACCATO = 0x0F,        // Staccato
};

// =============================================================================
// Offsets de audio na ROM (Banks $0A-$1F)
// =============================================================================

// Locais conhecidos de dados de audio na ROM
struct AudioROMOffset {
    uint32 samples_offset;          // Offset dos samples BRR
    uint32 instruments_offset;      // Offset dos instrumentos
    uint32 music_table_offset;      // Offset da tabela de musicas
    uint32 sfx_table_offset;        // Offset da tabela de SFX
    uint16 samples_size;            // Tamanho dos samples
    uint16 instruments_size;        // Tamanho dos instrumentos
};

// Offset de audio no bank $0D (conhecido)
constexpr AudioROMOffset AUDIO_OFFSETS = {
    .samples_offset = 0x070000,     // Exemplo: bank $0E
    .instruments_offset = 0x078000,
    .music_table_offset = 0x07C000,
    .sfx_table_offset = 0x07E000,
    .samples_size = 0x8000,
    .instruments_size = 0x4000,
};

// =============================================================================
// Funcoes auxiliares
// =============================================================================

// Obtem informacoes de um sample
inline const BRRSample* music_get_sample(int index) {
    if (index >= 0 && index < BRR_SAMPLE_COUNT) {
        return &BRR_SAMPLES[index];
    }
    return nullptr;
}

// Obtem informacoes de uma musica
inline const MusicTrack* music_get_track(int index) {
    if (index >= 0 && index < MUSIC_COUNT) {
        return &MUSIC_TRACKS[index];
    }
    return nullptr;
}

// Obtem um sound effect
inline const SFXEntry* music_get_sfx(int index) {
    if (index >= 0 && index < SFX_COUNT) {
        return &SFX_TABLE[index];
    }
    return nullptr;
}

// Converte nota musical para frequencia
inline uint16 note_to_frequency(uint8 note, uint8 octave) {
    // Tabela de frequencias baseada no SNES
    // Cada nota = 2^(n/12) * 440
    static const uint16 note_freq_table[12] = {
        0x010B, 0x011B, 0x012C, 0x013E, 0x0151, 0x0165,
        0x017A, 0x0191, 0x01A9, 0x01C2, 0x01DD, 0x01F9
    };

    if (note < 0x81 || note > 0x8C) return 0;

    int note_idx = note - 0x81;
    uint16 freq = note_freq_table[note_idx];

    // Ajustar oitava
    if (octave > 4) {
        freq <<= (octave - 4);
    } else if (octave < 4) {
        freq >>= (4 - octave);
    }

    return freq;
}

// Converte volume (0-127) para register value
inline uint8 volume_to_register(uint8 volume) {
    return (volume * 255) / 127;
}

// Converte pan (-127 a 127) para register value
inline uint8 pan_to_register(int8 pan) {
    return static_cast<uint8>(128 + pan);
}

#endif // MUSIC_DATA_H
