#include "game_flow.h"
#include "game_state.h"
#include "game_state_cutscene.h"
#include "macros.h"
#include "boss.h"
#include "types.h"

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

#define GAME_FLOW_TYPE_CUTSCENE         (uint8_t)0
#define GAME_FLOW_TYPE_SHOP             (uint8_t)1
#define GAME_FLOW_TYPE_CHECKPOINT       (uint8_t)2
#define GAME_FLOW_TYPE_BOSS             (uint8_t)3

#define GAME_FLOW_GET_TYPE(x)           ( 0x03 & (x) )
#define GAME_FLOW_IS_DIFFICULTY(x,d)    ( ( 0x07 & ( (x) >> 2 ) ) <= (d) )

//
// Default Game Flow
//

const static uint8_t game_flow_steps[] = {
    GAME_FLOW_TYPE_CUTSCENE, Cutscene_000_Intro,

    GAME_FLOW_TYPE_CUTSCENE, Cutscene_100_Boss0_Preroll,
    GAME_FLOW_TYPE_BOSS, BOSS_0,
    GAME_FLOW_TYPE_CUTSCENE, Cutscene_110_Boss0_Postroll,

    GAME_FLOW_TYPE_CHECKPOINT,

    GAME_FLOW_TYPE_CUTSCENE, Cutscene_100_Boss0_Preroll,
    GAME_FLOW_TYPE_BOSS, BOSS_1,
    GAME_FLOW_TYPE_CUTSCENE, Cutscene_110_Boss0_Postroll,

    GAME_FLOW_TYPE_CHECKPOINT,
    GAME_FLOW_TYPE_SHOP, 0,

    GAME_FLOW_TYPE_CUTSCENE, Cutscene_100_Boss0_Preroll,
    GAME_FLOW_TYPE_BOSS, BOSS_2,
    GAME_FLOW_TYPE_CUTSCENE, Cutscene_110_Boss0_Postroll,

    GAME_FLOW_TYPE_CHECKPOINT,

    GAME_FLOW_TYPE_CUTSCENE, Cutscene_100_Boss0_Preroll,
    GAME_FLOW_TYPE_BOSS, BOSS_3,

    GAME_FLOW_TYPE_CHECKPOINT,

    GAME_FLOW_TYPE_CUTSCENE, Cutscene_110_Boss0_Postroll,
    GAME_FLOW_TYPE_BOSS, BOSS_4,
    GAME_FLOW_TYPE_CUTSCENE, Cutscene_110_Boss0_Postroll,

    GAME_FLOW_TYPE_CHECKPOINT,
    GAME_FLOW_TYPE_SHOP, 1,

    GAME_FLOW_TYPE_CUTSCENE, Cutscene_100_Boss0_Preroll,
    GAME_FLOW_TYPE_BOSS, BOSS_5,
    GAME_FLOW_TYPE_CUTSCENE, Cutscene_110_Boss0_Postroll,

    GAME_FLOW_TYPE_CHECKPOINT,

    GAME_FLOW_TYPE_CUTSCENE, Cutscene_100_Boss0_Preroll,
    GAME_FLOW_TYPE_BOSS, BOSS_6,
    GAME_FLOW_TYPE_CUTSCENE, Cutscene_110_Boss0_Postroll,

    GAME_FLOW_TYPE_CHECKPOINT,
    GAME_FLOW_TYPE_SHOP, 2,

    GAME_FLOW_TYPE_CUTSCENE, Cutscene_100_Boss0_Preroll,
    GAME_FLOW_TYPE_BOSS, BOSS_7,
    GAME_FLOW_TYPE_CUTSCENE, Cutscene_110_Boss0_Postroll,

    GAME_FLOW_TYPE_CUTSCENE, Cutscene_999_Credits
};
STATIC_ASSERT( ARRAY_SIZE(game_flow_steps) < 0xFF );

//
//
//

static uint8_t game_flow_index;
static uint8_t game_flow_last_checkpoint_index;

void __fastcall__ reset_game_flow(void)
{
    game_flow_index = 0;
    game_flow_last_checkpoint_index = 0;
}

void __fastcall__ checkpoint_game_flow(void)
{
    game_flow_last_checkpoint_index = game_flow_index;
}

void __fastcall__ restore_game_flow_from_checkpoint(void)
{
    // add 1 so it starts from the step after the checkpoint
    game_flow_index = game_flow_last_checkpoint_index + 1;
}

void __fastcall__ advance_game_flow(void)
{
    // at the end of the game flow, go back to the title screen
    if( game_flow_index >= ARRAY_SIZE(game_flow_steps) )
    {
        set_next_game_state( GAME_STATE_TITLE );
        return;
    }

    switch( GAME_FLOW_GET_TYPE( game_flow_steps[ game_flow_index ] ) )
    {
        case GAME_FLOW_TYPE_CUTSCENE:
        {
            game_flow_index++;

            // set the next game state with the cutscene id
            set_next_game_state_arg( GAME_STATE_CUTSCENE, game_flow_steps[ game_flow_index ] );

            game_flow_index++;
        }
            break;

        case GAME_FLOW_TYPE_SHOP:
        {
            game_flow_index++;

            // set the next game state with the store level
            set_next_game_state_arg( GAME_STATE_STORE, game_flow_steps[ game_flow_index ] );

            game_flow_index++;
        }
            break;

        case GAME_FLOW_TYPE_CHECKPOINT:
        {
            game_flow_index++;

            // set the next game state to checkpoint (which saves and then advances)
            set_next_game_state_arg( GAME_STATE_CHECKPOINT, game_flow_steps[ game_flow_index ] );
        }
            break;

        case GAME_FLOW_TYPE_BOSS:
        {
            game_flow_index++;

            // set the next game state with the boss id
            set_next_game_state_arg( GAME_STATE_PLAYING, game_flow_steps[ game_flow_index ] );

            game_flow_index++;
        }
            break;

        default:
            INVALID_CODE_PATH;
            break;
    }
}