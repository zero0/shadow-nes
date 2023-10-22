//static int d[20];
//#define i d[0]
//#define b d[1];
//static unsigned char i, b;

void __fastcall__ ppu_update(void);

void __fastcall__ gamepad_poll( unsigned char pad );

unsigned char __fastcall__ gamepad_state( unsigned char pad );

unsigned char __fastcall__ gamepad_prev_state( unsigned char pad );

//
//
//

static unsigned char gamepad_state0;

void main(void)
{
    while( 1 )
    {
        gamepad_poll( 0 );

        gamepad_state0 = gamepad_state( 0 );

        ppu_update();
    }
}
