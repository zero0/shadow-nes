#include "game_flow.h"
#include "game_state.h"
#include "macros.h"
#include "boss.h"
#include "types.h"
#include "game_data.h"

#define GAME_DIFFICULTY_MOD_STORY       (uint8_t)6
#define GAME_DIFFICULTY_MOD_EASY        (uint8_t)5
#define GAME_DIFFICULTY_MOD_NORMAL      (uint8_t)4
#define GAME_DIFFICULTY_MOD_HARD        (uint8_t)3
#define GAME_DIFFICULTY_MOD_EXTREME     (uint8_t)2
#define GAME_DIFFICULTY_MOD_BRUTAL      (uint8_t)1
#define GAME_DIFFICULTY_MOD_BOSS_ONLY   (uint8_t)0

#define GAME_FLOW_DIFFICULTY_MOD(x,d)   ( ( 0x03 & (x) ) | ( (d) << 2 ) )

#define GAME_FLOW_DIFFICULTY_STORY(x)   GAME_FLOW_DIFFICULTY_MOD(x, GAME_DIFFICULTY_MOD_STORY)
#define GAME_FLOW_DIFFICULTY_EASY(x)    GAME_FLOW_DIFFICULTY_MOD(x, GAME_DIFFICULTY_MOD_EASY)
#define GAME_FLOW_DIFFICULTY_NORMAL(x)  GAME_FLOW_DIFFICULTY_MOD(x, GAME_DIFFICULTY_MOD_NORMAL)
#define GAME_FLOW_DIFFICULTY_HARD(x)    GAME_FLOW_DIFFICULTY_MOD(x, GAME_DIFFICULTY_MOD_HARD)
#define GAME_FLOW_DIFFICULTY_BRUTAL(x)  GAME_FLOW_DIFFICULTY_MOD(x, GAME_DIFFICULTY_MOD_BURTAL)

enum
{
    GAME_FLOW_TYPE_CUTSCENE,
    GAME_FLOW_TYPE_STORE,
    GAME_FLOW_TYPE_CHECKPOINT,
    GAME_FLOW_TYPE_BOSS,
    GAME_FLOW_TYPE_MAIN_MENU,
    _GAME_FLOW_TYPE_COUNT,
};

#define GAME_FLOW_GET_TYPE(x)           ( 0x03 & (x) )
#define GAME_FLOW_IS_DIFFICULTY(x,d)    ( ( 0x07 & ( (x) >> 2 ) ) <= (d) )

//
// Default Game Flow
//

const static uint8_t game_flow_steps[] = {
    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_000_INTRO,

    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_100_BOSS_0,
    GAME_FLOW_TYPE_BOSS, BOSS_0,
    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_110_BOSS_0,

    GAME_FLOW_TYPE_CHECKPOINT,

    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_100_BOSS_0,
    GAME_FLOW_TYPE_BOSS, BOSS_1,
    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_110_BOSS_0,

    GAME_FLOW_TYPE_CHECKPOINT,
    GAME_FLOW_TYPE_STORE, STORE_0,

    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_100_BOSS_0,
    GAME_FLOW_TYPE_BOSS, BOSS_2,
    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_110_BOSS_0,

    GAME_FLOW_TYPE_CHECKPOINT,

    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_100_BOSS_0,
    GAME_FLOW_TYPE_BOSS, BOSS_3,

    GAME_FLOW_TYPE_CHECKPOINT,

    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_110_BOSS_0,
    GAME_FLOW_TYPE_BOSS, BOSS_4,
    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_110_BOSS_0,

    GAME_FLOW_TYPE_CHECKPOINT,
    GAME_FLOW_TYPE_STORE, STORE_1,

    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_100_BOSS_0,
    GAME_FLOW_TYPE_BOSS, BOSS_5,
    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_110_BOSS_0,

    GAME_FLOW_TYPE_CHECKPOINT,

    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_100_BOSS_0,
    GAME_FLOW_TYPE_BOSS, BOSS_6,
    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_110_BOSS_0,

    GAME_FLOW_TYPE_CHECKPOINT,
    GAME_FLOW_TYPE_STORE, STORE_2,

    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_100_BOSS_0,
    GAME_FLOW_TYPE_BOSS, BOSS_7,
    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_110_BOSS_0,

    GAME_FLOW_TYPE_CUTSCENE, CUTSCENE_999_CREDITS,

    GAME_FLOW_TYPE_MAIN_MENU,
};
STATIC_ASSERT( ARRAY_SIZE(game_flow_steps) < 0xFF );

//
//
//

void __fastcall__ reset_game_flow(void)
{
    g_current_game_data.current_game_flow = 0;
    g_current_game_data.current_game_flow_last_checkpoint = 0;
}

void __fastcall__ checkpoint_game_flow(void)
{
    g_current_game_data.current_game_flow_last_checkpoint = g_current_game_data.current_game_flow;
}

void __fastcall__ restore_game_flow_from_checkpoint(void)
{
    // add 1 so it starts from the step after the checkpoint
    g_current_game_data.current_game_flow = g_current_game_data.current_game_flow_last_checkpoint + 1;
}

void __fastcall__ advance_game_flow(void)
{
    // at the end of the game flow, go back to the title screen
    if( g_current_game_data.current_game_flow >= ARRAY_SIZE(game_flow_steps) )
    {
        set_next_game_state( GAME_STATE_TITLE );
        return;
    }

    switch( GAME_FLOW_GET_TYPE( game_flow_steps[ g_current_game_data.current_game_flow ] ) )
    {
        case GAME_FLOW_TYPE_CUTSCENE:
        {
            g_current_game_data.current_game_flow++;

            // set the next game state with the cutscene id
            set_next_game_state_arg( GAME_STATE_CUTSCENE, game_flow_steps[ g_current_game_data.current_game_flow ] );

            g_current_game_data.current_game_flow++;
        }
            break;

        case GAME_FLOW_TYPE_STORE:
        {
            g_current_game_data.current_game_flow++;

            // set the next game state with the store level
            set_next_game_state_arg( GAME_STATE_STORE, game_flow_steps[ g_current_game_data.current_game_flow ] );

            g_current_game_data.current_game_flow++;
        }
            break;

        case GAME_FLOW_TYPE_CHECKPOINT:
        {
            g_current_game_data.current_game_flow++;

            // set the next game state to checkpoint (which saves and then advances)
            set_next_game_state_arg( GAME_STATE_CHECKPOINT, game_flow_steps[ g_current_game_data.current_game_flow ] );
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