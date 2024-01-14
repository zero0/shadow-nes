#include "types.h"
#include "macros.h"
#include "gamepad.h"
#include "subpixel.h"
#include "player.h"
#include "boss.h"
#include "ppu.h"
#include "game_state.h"
#include "globals.h"

extern ptr_t knight_sprite_0;
extern ptr_t knight_sprite_1;
extern ptr_t knight_sprite_2;
extern ptr_t progress_bar;

void __fastcall__ game_state_playing_enter()
{
    ppu_clear_nametable( NAMETABLE_A, 0xFF, 0 );
    //ppu_upload_chr_ram( shadow_font, 5, 0x00 );
    //ppu_upload_chr_ram( knight, 2, 0x10 );

    ppu_upload_chr_ram( progress_bar, 1, 0x08);

    ppu_set_scroll( 0, 0 );
    ppu_clear_palette();
    ppu_clear_oam();

    ppu_set_palette_background( 0x0F );
    ppu_set_palette( PALETTE_BACKGROUND_0, 0x15, 0x26, 0x37 );
    ppu_set_palette( PALETTE_BACKGROUND_1, 0x05, 0x15, 0x30 ); // red, light red, white
    ppu_set_palette( PALETTE_BACKGROUND_2, 0x1A, 0x2A, 0x30 ); // green, light green, white
    ppu_set_palette( PALETTE_SPRITE_0, 0x0A, 0x1A, 0x2A );

    // top
    ppu_begin_tile_batch(0,0);
    ppu_repeat_tile_batch(0, SCREEN_WIDTH * 2);
    ppu_end_tile_batch();

    ppu_begin_tile_batch(0,2);
    ppu_repeat_tile_batch(0, SCREEN_WIDTH * 3);
    ppu_end_tile_batch();

    // bottom
    ppu_begin_tile_batch(0,SCREEN_HEIGH-7);
    ppu_repeat_tile_batch(0, SCREEN_WIDTH * 2);
    ppu_end_tile_batch();

    ppu_begin_tile_batch(0,SCREEN_HEIGH-6);
    ppu_repeat_tile_batch(0, SCREEN_WIDTH * 3);
    ppu_end_tile_batch();

    ppu_begin_tile_batch(0,SCREEN_HEIGH-3);
    ppu_repeat_tile_batch(0, SCREEN_WIDTH * 3);
    ppu_end_tile_batch();

    // knite sprite
    ppu_upload_meta_sprite_chr_ram( knight_sprite_0, 0x10 );
    ppu_upload_meta_sprite_chr_ram( knight_sprite_1, 0x12 );
    ppu_upload_meta_sprite_chr_ram( knight_sprite_2, 0x14 );

    // health and stamina colors
    ppu_set_nametable_attr( NAMETABLE_A_ATTR, 0, 0,  0, 0, 2, 2,  4 );

t = 0;
b = 1;
l = 20;

    // start player
    player_init();

    // start boss
    init_boss(BOSS_0);
}

void __fastcall__ game_state_playing_leave()
{

}

#define PLAYER_HEALTH_PER_TILE_LOG2     (uint8_t)3
#define BOSS_HEALTH_PER_TILE_LOG2       (uint8_t)7

void __fastcall__ game_state_playing_update()
{
    gamepad_poll(0);

    // update health and stamina on alternate frames
    if( get_player_changed_flags() & PLAYER_CHANGED_HEALTH )
    {
        x = get_player_current_health();
        y = get_player_max_health();

        // player health bar
        ppu_begin_tile_batch(2,1);

        // full tiles
        for( i = 0, imax = (y >> PLAYER_HEALTH_PER_TILE_LOG2), j = 8; i < imax && x >= j; ++i, j += (1 << PLAYER_HEALTH_PER_TILE_LOG2) )
        {
            ppu_push_tile_batch(0x80 + 8);
        }

        // partial tile
        if( i < imax )
        {
            j -= (1 << PLAYER_HEALTH_PER_TILE_LOG2);
            ppu_push_tile_batch(0x80 + ( x - j ) );
            ++i;
        }

        // empty tiles
        for( ; i < imax; ++i )
        {
            ppu_push_tile_batch( 0x80 );
        }

        ppu_end_tile_batch();
    }

    if( get_player_changed_flags() & PLAYER_CHANGED_STAMINA )
    {
        x = get_player_current_stamina();
        y = get_player_max_stamina();

        // player stamina bar
        ppu_begin_tile_batch(2,2);

        // full tiles
        for( i = 0, imax = (y >> PLAYER_HEALTH_PER_TILE_LOG2), j = (1 << PLAYER_HEALTH_PER_TILE_LOG2); i < imax && x >= j; ++i, j += (1 << PLAYER_HEALTH_PER_TILE_LOG2) )
        {
            ppu_push_tile_batch(0x80 + 8);
        }

        // partial tile
        if( i < imax )
        {
            j -= (1 << PLAYER_HEALTH_PER_TILE_LOG2);
            ppu_push_tile_batch(0x80 + ( x - j ) );
            ++i;
        }

        // empty tiles
        for( ; i < imax; ++i )
        {
            ppu_push_tile_batch( 0x80 );
        }

        ppu_end_tile_batch();
    }

    // boss health changed
    if( get_boss_changed_flags() & BOSS_CHANGED_HEALTH )
    {
        x16 = get_boss_current_health();
        y16 = get_boss_max_health();
        z = get_boss_health_per_block_log2();

        // player stamina bar
        ppu_begin_tile_batch(2,27);

        // full tiles
        for( i16 = 0, imax16 = (y16 >> z), j16 = (1 << z); i16 < imax16 && x16 >= j16; ++i16, j16 += (1 << z) )
        {
            ppu_push_tile_batch(0x80 + 8);
        }

        // partial tile
        if( i16 < imax16 )
        {
            j16 -= (1 << z);
            ppu_push_tile_batch(0x80 + (0x07 & (uint8_t)( x16 - j16 ) ) );
            ++i16;
        }

        // empty tiles
        for( ; i16 < imax16 && i16 < 20; ++i16 )
        {
            ppu_push_tile_batch( 0x80 );
        }

        ppu_end_tile_batch();
    }

    //ppu_begin_tile_batch(2+(x >> 3),1);
    //ppu_repeat_tile_batch(0x80+(x & 0x7), 1);
    //ppu_end_tile_batch();

    switch(t)
    {
        case 0:
            if(b == 0)
            {
                ppu_upload_meta_sprite_chr_ram( knight_sprite_0, 0x10 );
                b = 1;
            }
            ppu_add_meta_sprite( knight_sprite_0, TILE_TO_PIXEL(3), TILE_TO_PIXEL(14), 0x00 );
            break;

        case 1:
          if(b == 0)
            {
                ppu_upload_meta_sprite_chr_ram( knight_sprite_1, 0x10 );
                b = 1;
            }
            ppu_add_meta_sprite( knight_sprite_1, TILE_TO_PIXEL(3), TILE_TO_PIXEL(14), 0x20 );
            break;

        case 2:
          if(b == 0)
            {
                ppu_upload_meta_sprite_chr_ram( knight_sprite_2, 0x10 );
                b = 1;
            }
            ppu_add_meta_sprite( knight_sprite_2, TILE_TO_PIXEL(3), TILE_TO_PIXEL(14), 0x40 );
            break;
    }

--l;
    if(l ==0 || GAMEPAD_HELD(0, GAMEPAD_B))
    {
        l = 10;
        b = 1;
        t++;
        if( t > 2) t = 0;
    }

    player_update();

    update_boss();
}
