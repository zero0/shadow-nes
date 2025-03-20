#include "game_data.h"
#include "globals.h"

#define CURRENT_GAME_DATA_VERSION   (uint8_t)1

game_data_t g_current_game_data;

//
// Game Data Migrations
//

typedef void (*game_data_migration_func)(void);

static void __fastcall__ game_data_migration_0_to_1(void)
{
}

static const game_data_migration_func game_data_migrations[] = {
    game_data_migration_0_to_1
};
STATIC_ASSERT(ARRAY_SIZE(game_data_migrations) == CURRENT_GAME_DATA_VERSION);

static void __fastcall__ game_data_clear_current_game_data(void)
{
    g_current_game_data.version = 0;
    g_current_game_data.player_level = 0;
    g_current_game_data.player_type = 0;

    g_current_game_data.current_game_flow = 0;
    g_current_game_data.current_game_flow_last_checkpoint = 0;
    g_current_game_data.current_difficulty = 0;

    g_current_game_data.difficulties_defeated = 0;
    g_current_game_data.bosses_defeated[0] = 0;

    g_current_game_data.currency_0 = 0;
    g_current_game_data.currency_1 = 0;

    g_current_game_data.name_len = 0;
    g_current_game_data.name[0] = 0;
}

//
// Game Data API
//

void __fastcall__ game_data_load_for_new_game(void)
{
    // clear local data
    game_data_clear_current_game_data();

    g_current_game_data.version = CURRENT_GAME_DATA_VERSION;
}

void __fastcall__ game_data_peek_from_save_slot(uint8_t saveSlot)
{
    UNUSED(saveSlot);
    ASSERT(saveSlot < _SAVE_SLOT_COUNT);

    // clear local data
    game_data_clear_current_game_data();

    // TODO: load game data from save ram
}

void __fastcall__ game_data_load_from_save_slot(uint8_t saveSlot)
{
    UNUSED(saveSlot);
    ASSERT(saveSlot < _SAVE_SLOT_COUNT);

    // clear local data
    game_data_clear_current_game_data();

    // TODO: load game data from save ram

    // perform data migration
    ASSERT(g_current_game_data.version <= CURRENT_GAME_DATA_VERSION );
    if( g_current_game_data.version != CURRENT_GAME_DATA_VERSION )
    {
        while( g_current_game_data.version != CURRENT_GAME_DATA_VERSION )
        {
            game_data_migrations[ g_current_game_data.version ]();
            g_current_game_data.version++;
        }

        // resave data when migration complete
        game_data_save_to_save_slot(saveSlot);
    }
}

void __fastcall__ game_data_save_to_save_slot(uint8_t saveSlot)
{
    UNUSED(saveSlot);
    ASSERT(saveSlot < _SAVE_SLOT_COUNT);

    // TODO: save game data to save ram
}
