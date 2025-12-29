#include "game_state_checkpoint.h"
#include "game_flow.h"

// perform checkpointing
void __fastcall__ game_state_checkpoint_enter(void)
{
    game_flow_checkpoint();
}

// advance to the next game flow state after checkpoint
void __fastcall__ game_state_checkpoint_update(void)
{
    game_flow_advance();
}
