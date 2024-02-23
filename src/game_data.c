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

//
// Game Data API
//

void __fastcall__ load_game_data_for_new_game(void)
{
    g_current_game_data.version = CURRENT_GAME_DATA_VERSION;
}

void __fastcall__ load_game_data_from_save_slot(uint8_t saveSlot)
{
    UNUSED(saveSlot);
    ASSERT(saveSlot < _SaveSlot_Count);

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
        save_game_data_to_save_slot(saveSlot);
    }
}

void __fastcall__ save_game_data_to_save_slot(uint8_t saveSlot)
{
    UNUSED(saveSlot);
    ASSERT(saveSlot < _SaveSlot_Count);

    // TODO: save game data to save ram
}
