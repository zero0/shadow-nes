#ifndef GAME_FLOW_H
#define GAME_FLOW_H

void game_flow_reset(void);

void game_flow_checkpoint(void);

void game_flow_restore_from_checkpoint(void);

void game_flow_advance(void);

#endif // GAME_FLOW_H
