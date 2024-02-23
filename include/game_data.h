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

typedef struct {
    uint8_t player_level;
    uint8_t player_type;

    uint8_t current_game_flow;
    uint8_t current_game_flow_last_checkpoint;
    uint8_t current_difficulty;

    flags8_t difficulties_defeated;
    flags8_t bosses_defeated[_BOSS_COUNT];

    uint8_t currency_0;
    uint8_t currency_1;

    uint8_t name_len;
    uint8_t name[15];

    uint8_t version;
} game_data_t;

extern game_data_t g_current_game_data;

void load_game_data_for_new_game(void);

void load_game_data_from_save_slot(uint8_t saveSlot);

void save_game_data_to_save_slot(uint8_t saveSlot);

#endif // GAME_DATA_H
