#include "ppu.h"
#include "timer.h"

static timer_handle_t ppu_fade_timer;
static uint8_t ppu_fade_timer_max;
static uint8_t ppu_fade_dest;

void __fastcall__ ppu_fade_to_internal(void)
{
    ppu_fade_dest = PPU_ARGS[0];
    ppu_fade_timer_max = PPU_ARGS[1];
    ppu_fade_timer = request_timer( 1 );

    ppu_update();

    while( !( ppu_fade_dest == PALETTE_TINT_OAM_INDEX && ppu_fade_dest == PALETTE_TINT_BACKGROUND_INDEX ) )
    {
        // skip frame
        ppu_update();

        if( is_timer_done( ppu_fade_timer ) )
        {
            set_timer( ppu_fade_timer, ppu_fade_timer_max );

            // tint oam
            if( ppu_fade_dest < PALETTE_TINT_OAM_INDEX )
            {
                ppu_tint_palellete_oam( PALETTE_TINT_OAM_INDEX - 1 );
            }
            else if( ppu_fade_dest > PALETTE_TINT_OAM_INDEX )
            {
                ppu_tint_palellete_oam( PALETTE_TINT_OAM_INDEX + 1 );
            }

            // tint backgroud
            if( ppu_fade_dest < PALETTE_TINT_BACKGROUND_INDEX )
            {
                ppu_tint_palellete_background( PALETTE_TINT_BACKGROUND_INDEX - 1 );
            }
            else if( ppu_fade_dest > PALETTE_TINT_BACKGROUND_INDEX )
            {
                ppu_tint_palellete_background( PALETTE_TINT_BACKGROUND_INDEX + 1 );
            }
        }
    }

    release_timer( ppu_fade_timer );
}
