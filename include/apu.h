#ifndef APU_H
#define APU_H

#include "macros.h"
#include "types.h"

typedef uint8_t sfx_t;
typedef uint8_t music_t;

void __fastcall__ apu_update_mixer(void);


void __fastcall__ apu_play_music(music_t music);

void __fastcall__ apu_play_sfx(sfx_t sfx);

void __fastcall__ apu_play_sfx_channel(sfx_t sfx, uint8_t channel);


void __fastcall__ apu_pause_all_sfx(void);

void __fastcall__ apu_pause_music(void);


void __fastcall__ apu_stop_sfx(uint8_t channel);

void __fastcall__ apu_stop_all_sfx(void);

void __fastcall__ apu_stop_music(void);

#endif // APU_H
