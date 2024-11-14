#ifndef APU_H
#define APU_H

#include "macros.h"
#include "types.h"

typedef uint8_t sfx_t;
typedef uint8_t music_t;

void __fastcall__ apu_update_mixer(void);

void __fastcall__ apu_play_sfx( sfx_t sfx );

void __fastcall__ apu_play_music( music_t music );

#endif // APU_H
