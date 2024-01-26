#include "game_state_checkpoint.h"
#include "game_flow.h"

// perform checkpointing
void __fastcall__ game_state_checkpoint_enter(void)
{
    checkpoint_game_flow();
}

// advance to the next game flow state after checkpoint
void __fastcall__ game_state_checkpoint_update(void)
{
    advance_game_flow();
}
