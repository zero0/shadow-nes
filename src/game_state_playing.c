#include "types.h"
#include "macros.h"
#include "gamepad.h"
#include "subpixel.h"
#include "player.h"
#include "boss.h"
#include "ppu.h"
#include "game_state.h"
#include "mapper.h"
#include "globals.h"
#include "chr_rom.h"
#include "hud.png.sprite.h"
#include <gametext.h>

enum
{
    GAME_STATE_PLAYING_INTRO,
    GAME_STATE_PLAYING_PLAYING,
    GAME_STATE_PLAYING_OUTRO,
    GAME_STATE_PLAYING_PAUSED,
    GAME_STATE_PLAYING_SUCCESS,
    GAME_STATE_PLAYING_GAME_OVER,
    _GAME_STATE_PLAYING_COUNT,
};

enum
{
    Boss_Health_Sections = 26,
    Player_Health_Section = 22,
};

enum
{
    Boss_HealthBar_X = ALIGN_SCREEN_WIDTH_LEFT(2),
    Boss_HealthBar_Y = ALIGN_SCREEN_HEIGHT_TOP(1),
    Boss_HealthBar_W = Boss_Health_Sections,
    Boss_HealthBar_H = 1,

    Boss_HealthBar_X_Min = Boss_HealthBar_X,
    Boss_HealthBar_X_Val = Boss_HealthBar_X + 1,
    Boss_HealthBar_X_Max = Boss_HealthBar_W + Boss_HealthBar_X_Val,

    Boss_Status_X = ALIGN_SCREEN_WIDTH_LEFT(4),
    Boss_Status_Y = ALIGN_SCREEN_HEIGHT_TOP(2),
};

enum
{
    Player_HealthBar_X = ALIGN_SCREEN_WIDTH_LEFT(4),
    Player_HealthBar_Y = ALIGN_SCREEN_HEIGHT_BOTTOM(2),
    Player_HealthBar_W = Player_Health_Section,
    Player_HealthBar_H = 1,

    Player_HealthBar_X_Min = Player_HealthBar_X,
    Player_HealthBar_X_Val = Player_HealthBar_X + 1,
    Player_HealthBar_X_Max = Player_HealthBar_W + Player_HealthBar_X_Val,

    Player_Status_X = ALIGN_SCREEN_WIDTH_LEFT(6),
    Player_Status_Y = ALIGN_SCREEN_HEIGHT_BOTTOM(3),
};

enum
{
    Sprite_HealthBar_Min = CHR_SPRITE(CHR_ROM_00_HUD_PNG_SPRITE, SPRITE_PROGRESS_MIN),
    Sprite_HealthBar_Max = CHR_SPRITE(CHR_ROM_00_HUD_PNG_SPRITE, SPRITE_PROGRESS_MAX),
    Sprite_HealthBar_100 = CHR_SPRITE(CHR_ROM_00_HUD_PNG_SPRITE, SPRITE_PROGRESS_8),
    Sprite_HealthBar_000 = CHR_SPRITE(CHR_ROM_00_HUD_PNG_SPRITE, SPRITE_PROGRESS_0),
};

extern ptr_t progress_bar;
extern ptr_t shadow_font;

void __fastcall__ game_state_playing_enter()
{
    // turn off ppu
    ppu_disable();

    // mapper update
    mapper_reset();
    mapper_reset_irq();

    mapper_set_chr_bank_0(CHR_ROM_00_SHADOW_FONT_EN_PNG_FONT_CHR_ROM);
    mapper_set_chr_bank_1(CHR_ROM_02_HUD_PNG_SPRITE_CHR_ROM);

    mapper_set_prg_bank(0);

    {
        ppu_clear_nametable( NAMETABLE_A, 0xFF, 0 );
        ppu_clear_nametable( NAMETABLE_B, 0xFF, 0 );
        ppu_clear_nametable( NAMETABLE_C, 0xFF, 0 );
        ppu_clear_nametable( NAMETABLE_D, 0xFF, 0 );
        //ppu_upload_chr_ram( shadow_font, MAKE_CHR_PTR(0,0,0), 16*4+13 );

        // upload progress bar
        //ppu_upload_chr_ram( progress_bar, MAKE_CHR_PTR(0, 8, 0), 9);

        ppu_set_scroll( 0, 0 );
        ppu_clear_palette();
        ppu_clear_oam();

        ppu_tint_reset();

        ppu_set_palette_background( 0x0F );
        ppu_set_palette( PALETTE_BACKGROUND_0, 0x15, 0x26, 0x37 );
        ppu_set_palette( PALETTE_BACKGROUND_1, 0x05, 0x15, 0x30 ); // red, light red, white
        ppu_set_palette( PALETTE_BACKGROUND_2, 0x1A, 0x2A, 0x30 ); // green, light green, white
        ppu_set_palette( PALETTE_SPRITE_0, 0x0A, 0x1A, 0x2A );

        // top
        //ppu_repeat_tile_batch(ALIGN_SCREEN_WIDTH_LEFT(0), ALIGN_SCREEN_HEIGHT_TOP(0), CHR_SPRITE(CHR_ROM_00_HUD_PNG_SPRITE, SPRITE_BORDER_TOP_CENTER), SCREEN_WIDTH );


        // boss health bar
        ppu_repeat_tile_batch(Boss_HealthBar_X_Min, Boss_HealthBar_Y, Sprite_HealthBar_Min, 1);
        ppu_repeat_tile_batch(Boss_HealthBar_X_Val, Boss_HealthBar_Y, Sprite_HealthBar_100, Boss_HealthBar_W);
        ppu_repeat_tile_batch(Boss_HealthBar_X_Max, Boss_HealthBar_Y, Sprite_HealthBar_Max, 1);

        // player health bar
        ppu_repeat_tile_batch(Player_HealthBar_X_Min, Player_HealthBar_Y, Sprite_HealthBar_Min, 1);
        ppu_repeat_tile_batch(Player_HealthBar_X_Val, Player_HealthBar_Y, Sprite_HealthBar_100, Player_HealthBar_W);
        ppu_repeat_tile_batch(Player_HealthBar_X_Max, Player_HealthBar_Y, Sprite_HealthBar_Max, 1);

#if 0
        ppu_begin_tile_batch(0,2);
        ppu_push_repeat_tile_batch(0, SCREEN_WIDTH * 3);
        ppu_end_tile_batch();

        // bottom
        ppu_begin_tile_batch(0,SCREEN_HEIGH-7);
        ppu_push_repeat_tile_batch(0, SCREEN_WIDTH * 2);
        ppu_end_tile_batch();

        ppu_begin_tile_batch(0,SCREEN_HEIGH-6);
        ppu_push_repeat_tile_batch(0, SCREEN_WIDTH * 3);
        ppu_end_tile_batch();

        ppu_begin_tile_batch(0,SCREEN_HEIGH-3);
        ppu_push_repeat_tile_batch(0, SCREEN_WIDTH * 3);
        ppu_end_tile_batch();
#endif
    }

    ppu_enable();

    game_state_internal = GAME_STATE_PLAYING_INTRO;


    // health and stamina colors
    //ppu_set_nametable_attr( NAMETABLE_A_ATTR,  0, 0,  PALETTE_BACKGROUND_0, PALETTE_BACKGROUND_0, PALETTE_BACKGROUND_2, PALETTE_BACKGROUND_2,  4 );

    // start player
    //player_init();

    // start boss
    //boss_init(next_game_state_arg);
}

void __fastcall__ game_state_playing_leave()
{

}

// pause internal state
static void __fastcall__ game_state_playing_paused_enter(void)
{
    ppu_disable();

    {
        ppu_clear_nametable( NAMETABLE_C, 0xFF, 0 );

        ppu_set_scroll(0, SCREEN_HEIGH_PIXELS);

        //text_draw_string( ALIGN_SCREEN_WIDTH_CENTER(tr_paused_width), SCREEN_HEIGH + ALIGN_SCREEN_HEIGHT_CENTER(tr_paused_height), PALETTE_BACKGROUND_0, tr_paused );
    }

    ppu_enable();
}

static void __fastcall__ game_state_playing_paused_leave(void)
{
    ppu_set_scroll(0, 0);
}

static void __fastcall__ game_state_playing_paused_update(void)
{
    // close pause menu
    if( GAMEPAD_PRESSED( 0, GAMEPAD_START ) )
    {
        game_state_internal = GAME_STATE_PLAYING_PLAYING;

        game_state_playing_paused_leave();
    }
}

//
static void __fastcall__ game_state_playing_game_over_enter(void)
{

}

static void __fastcall__ game_state_playing_game_over_update(void)
{
    // close pause menu
    if( GAMEPAD_PRESSED( 0, GAMEPAD_START ) )
    {
        set_next_game_state( GAME_STATE_TITLE );

        game_state_playing_paused_leave();
    }
}

static void __fastcall__ game_state_playing_game_over_leave(void)
{

}

//
void __fastcall__ game_state_playing_update()
{
    switch( game_state_internal )
    {
        case GAME_STATE_PLAYING_INTRO:
        {
            // TODO: play intro
            game_state_internal = GAME_STATE_PLAYING_PLAYING;

           // ppu_fade_to( PPU_PALETTE_TINT_DEFAULT, 10 );
        }
            break;

        case GAME_STATE_PLAYING_PLAYING:
        {
            // open pause menu
            if( GAMEPAD_PRESSED( 0, GAMEPAD_START ) )
            {
                game_state_internal = GAME_STATE_PLAYING_PAUSED;

                game_state_playing_paused_enter();
                return;
            }

            // update player and boss
            player_update();

            boss_update();

            // render player and boss
            player_render();

            // check if the player died first
            if( player_is_dead() )
            {
                game_state_internal = GAME_STATE_PLAYING_GAME_OVER;

                game_state_playing_game_over_enter();
            }
            else if( boss_is_dead() )
            {
                game_state_internal = GAME_STATE_PLAYING_OUTRO;
            }
        }
            break;

        case GAME_STATE_PLAYING_OUTRO:
        {
            // TODO: play outro
            game_state_internal = GAME_STATE_PLAYING_SUCCESS;
        }
            break;

        case GAME_STATE_PLAYING_PAUSED:
        {
            game_state_playing_paused_update();
        }
            break;


        case GAME_STATE_PLAYING_SUCCESS:
        {

        }
            break;

        case GAME_STATE_PLAYING_GAME_OVER:
        {
            game_state_playing_game_over_update();
        }
            break;

        default:
            INVALID_CODE_PATH;
            break;
    }
}

void __fastcall__ game_state_playing_set_pause(uint8_t isPaused)
{
    UNUSED(isPaused);
}

uint8_t __fastcall__ game_state_playing_is_pause(void)
{
    return game_state_internal == GAME_STATE_PLAYING_PAUSED;
}
