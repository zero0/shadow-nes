#ifndef GAME_DATA_H
#define GAME_DATA_H

#include "types.h"
#include "macros.h"
#include "flags.h"
#include "boss.h"

enum
{
    SAVE_SLOT_0,
    SAVE_SLOT_1,
    SAVE_SLOT_2,
    _SAVE_SLOT_COUNT
};

enum
{
    CURRENCY_0,
    CURRENCY_1,
    _CURRENCY_COUNT
};

typedef struct {
    uint8_t version;

    uint8_t player_level;
    uint8_t player_type;

    uint8_t current_game_flow;
    uint8_t current_game_flow_last_checkpoint;
    uint8_t current_difficulty;

    flags8_t difficulties_defeated;
    flags8_t bosses_defeated[_BOSS_COUNT];

    uint8_t currency[_CURRENCY_COUNT];

    char name[16];
} game_data_t;

enum
{
    GLOBAL_SETTINGS_0_MUSIC         = 1 << 0,
    GLOBAL_SETTINGS_0_SFX           = 1 << 1,
    //GLOBAL_SETTINGS_0__UNDEF      = 1 << 2,
    //GLOBAL_SETTINGS_0__UNDEF      = 1 << 3,
    //GLOBAL_SETTINGS_0__UNDEF      = 1 << 4,
    //GLOBAL_SETTINGS_0__UNDEF      = 1 << 5,
    //GLOBAL_SETTINGS_0__UNDEF      = 1 << 6,
#if !DISTRO_BUILD
    GLOBAL_SETTINGS_0_DEBUG_MODE    = 1 << 7,
#endif
};

enum
{
    GLOBAL_SETTINGS_1_SKIP_CUTSCENES        = 1 << 0,
    GLOBAL_SETTINGS_1_SKIP_BOSS_INTRO_OUTRO = 1 << 1,
    GLOBAL_SETTINGS_1_SKIP_CHECKPOINTS      = 1 << 2,
    GLOBAL_SETTINGS_1_SKIP_SHOPS            = 1 << 3,
};

typedef struct {
    flags8_t flags0;
    flags8_t flags1;
} global_settings_data_t;

extern game_data_t g_current_game_data;

extern global_settings_data_t g_global_settings_data;

void game_data_load_for_new_game(void);

void game_data_peek_from_save_slot(uint8_t saveSlot);

void game_data_load_from_save_slot(uint8_t saveSlot);

void game_data_save_to_save_slot(uint8_t saveSlot);

void game_data_load_from_code(uint8_t code);

void game_data_save_to_code(uint16_t code);

#endif // GAME_DATA_H
