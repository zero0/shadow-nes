#include "game_flow.h"
#include "game_state.h"
#include "macros.h"
#include "boss.h"
#include "types.h"
#include "game_data.h"
#include "flags.h"

enum
{
    GAME_FLOW_TYPE_CUTSCENE,
    GAME_FLOW_TYPE_STORE,
    GAME_FLOW_TYPE_CHECKPOINT,
    GAME_FLOW_TYPE_BOSS,
    GAME_FLOW_TYPE_MAIN_MENU,
    _GAME_FLOW_TYPE_COUNT,
};

//
// Default Game Flow
//

const static uint8_t game_flow_steps[] = {
    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_000_INTRO,

    // boss 0
    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_100_BOSS_0,
    GAME_FLOW_TYPE_BOSS, BOSS_0,
    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_110_BOSS_0,

    GAME_FLOW_TYPE_CHECKPOINT,

    // boss 1
    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_100_BOSS_0,
    GAME_FLOW_TYPE_BOSS, BOSS_1,
    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_110_BOSS_0,

    GAME_FLOW_TYPE_CHECKPOINT,
    GAME_FLOW_TYPE_STORE, STORE_0,

    // boss 2
    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_100_BOSS_0,
    GAME_FLOW_TYPE_BOSS, BOSS_2,
    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_110_BOSS_0,

    GAME_FLOW_TYPE_CHECKPOINT,

    // boss 3
    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_100_BOSS_0,
    GAME_FLOW_TYPE_BOSS, BOSS_3,

    GAME_FLOW_TYPE_CHECKPOINT,

    // boss 4
    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_110_BOSS_0,
    GAME_FLOW_TYPE_BOSS, BOSS_4,
    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_110_BOSS_0,

    GAME_FLOW_TYPE_CHECKPOINT,
    GAME_FLOW_TYPE_STORE, STORE_1,

    // boss 5
    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_100_BOSS_0,
    GAME_FLOW_TYPE_BOSS, BOSS_5,
    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_110_BOSS_0,

    GAME_FLOW_TYPE_CHECKPOINT,

    // boss 6
    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_100_BOSS_0,
    GAME_FLOW_TYPE_BOSS, BOSS_6,
    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_110_BOSS_0,

    GAME_FLOW_TYPE_CHECKPOINT,
    GAME_FLOW_TYPE_STORE, STORE_2,

    // boss 7
    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_100_BOSS_0,
    GAME_FLOW_TYPE_BOSS, BOSS_7,
    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_110_BOSS_0,

    // credits
    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_999_CREDITS,

    GAME_FLOW_TYPE_MAIN_MENU,
};
STATIC_ASSERT( ARRAY_SIZE(game_flow_steps) < 0xFF );

//
//
//

void __fastcall__ game_flow_reset(void)
{
    g_current_game_data.current_game_flow = 0;
    g_current_game_data.current_game_flow_last_checkpoint = 0;
}

void __fastcall__ game_flow_checkpoint(void)
{
    g_current_game_data.current_game_flow_last_checkpoint = g_current_game_data.current_game_flow;
}

void __fastcall__ game_flow_restore_from_checkpoint(void)
{
    // add 1 so it starts from the step after the checkpoint
    g_current_game_data.current_game_flow = g_current_game_data.current_game_flow_last_checkpoint + 1;
}

void __fastcall__ game_flow_advance(void)
{
    // at the end of the game flow, go back to the title screen
    if( g_current_game_data.current_game_flow >= ARRAY_SIZE(game_flow_steps) )
    {
        set_next_game_state( GAME_STATE_TITLE );
        return;
    }

    switch( game_flow_steps[ g_current_game_data.current_game_flow ] )
    {
        case GAME_FLOW_TYPE_CUTSCENE:
        {
            g_current_game_data.current_game_flow++;

            // skip cutscene if needed
            if( !flags_is_set( g_global_settings_data.flags1, GLOBAL_SETTINGS_1_SKIP_CUTSCENES ) )
            {
                // set the next game state with the cutscene id
                set_next_game_state_arg( GAME_STATE_CUTSCENE, game_flow_steps[ g_current_game_data.current_game_flow ] );
            }

            g_current_game_data.current_game_flow++;
        }
            break;

        case GAME_FLOW_TYPE_STORE:
        {
            g_current_game_data.current_game_flow++;

            // skip shop if needed
            if( !flags_is_set( g_global_settings_data.flags1, GLOBAL_SETTINGS_1_SKIP_SHOPS ) )
            {
                // set the next game state with the store level
                set_next_game_state_arg( GAME_STATE_STORE, game_flow_steps[ g_current_game_data.current_game_flow ] );
            }

            g_current_game_data.current_game_flow++;
        }
            break;

        case GAME_FLOW_TYPE_CHECKPOINT:
        {
            g_current_game_data.current_game_flow++;

            // skip checkpoints if needed
            if( !flags_is_set( g_global_settings_data.flags1, GLOBAL_SETTINGS_1_SKIP_SHOPS ) )
            {
                // set the next game state to checkpoint (which saves and then advances)
                set_next_game_state_arg( GAME_STATE_CHECKPOINT, game_flow_steps[ g_current_game_data.current_game_flow ] );
            }
            else
            {
                g_current_game_data.current_game_flow++;
            }
        }
            break;

        case GAME_FLOW_TYPE_BOSS:
        {
            g_current_game_data.current_game_flow++;

            // set the next game state with the boss id
            set_next_game_state_arg( GAME_STATE_PLAYING, game_flow_steps[ g_current_game_data.current_game_flow ] );

            g_current_game_data.current_game_flow++;
        }
            break;

        case GAME_FLOW_TYPE_MAIN_MENU:
        {
            g_current_game_data.current_game_flow = 0;
            g_current_game_data.current_game_flow_last_checkpoint = 0;

            set_next_game_state( GAME_STATE_TITLE );
        }
            break;

        default:
            INVALID_CODE_PATH;
            break;
    }
}