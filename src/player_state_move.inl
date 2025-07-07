
const static subpixel_diff_t PLAYER_MOVE_SPEED_CARDINAL_P  = subpixel_diff_make(  1, 128 );    // 1.5
const static subpixel_diff_t PLAYER_MOVE_SPEED_CARDINAL_N  = subpixel_diff_make( -1, 128 );    // 1.5
const static subpixel_diff_t PLAYER_MOVE_SPEED_DIAGNAL_P   = subpixel_diff_make(  1,  15 );    // 1.06
const static subpixel_diff_t PLAYER_MOVE_SPEED_DIAGNAL_N   = subpixel_diff_make( -1,  15 );    // 1.06

const static subpixel_diff_t PLAYER_MOVE_SPEED_SLOW_CARDINAL_P  = subpixel_diff_make(  1, 128 );    // 1.5
const static subpixel_diff_t PLAYER_MOVE_SPEED_SLOW_CARDINAL_N  = subpixel_diff_make( -1, 128 );    // 1.5
const static subpixel_diff_t PLAYER_MOVE_SPEED_SLOW_DIAGNAL_P   = subpixel_diff_make(  1,  15 );    // 1.06
const static subpixel_diff_t PLAYER_MOVE_SPEED_SLOW_DIAGNAL_N   = subpixel_diff_make( -1,  15 );    // 1.06


// test if move can be performed
#define can_perform_move()                                                          \
(                                                                                   \
    flags_is_set( player_can_perform_action_flags, PLAYER_CAN_PERFORM_ACTION_MOVE ) \
)

static void __fastcall__ perform_move(uint8_t dir)
{
    if( flags_mark( dir, PLAYER_MOVE_DIRECTION_NORTH ) )
    {
        // nw
        if( flags_mark( dir, PLAYER_MOVE_DIRECTION_WEST ) )
        {
            subpixel_inc( player_pos_dx, PLAYER_MOVE_SPEED_DIAGNAL_N );
            subpixel_inc( player_pos_dy, PLAYER_MOVE_SPEED_DIAGNAL_P );
        }
        // ne
        else if( flags_mark( dir, PLAYER_MOVE_DIRECTION_EAST ) )
        {
            subpixel_inc( player_pos_dx, PLAYER_MOVE_SPEED_DIAGNAL_P );
            subpixel_inc( player_pos_dy, PLAYER_MOVE_SPEED_DIAGNAL_P );
        }
        // pure north
        else
        {
            subpixel_inc( player_pos_dy, PLAYER_MOVE_SPEED_CARDINAL_P );
        }
    }
    else if( flags_mark( dir, PLAYER_MOVE_DIRECTION_SOUTH ) )
    {
        // sw
        if( flags_mark( dir, PLAYER_MOVE_DIRECTION_WEST ) )
        {
            subpixel_inc( player_pos_dx, PLAYER_MOVE_SPEED_DIAGNAL_N );
            subpixel_inc( player_pos_dy, PLAYER_MOVE_SPEED_DIAGNAL_N );
        }
        // se
        else if( flags_mark( dir, PLAYER_MOVE_DIRECTION_EAST ) )
        {
            subpixel_inc( player_pos_dx, PLAYER_MOVE_SPEED_DIAGNAL_P );
            subpixel_inc( player_pos_dy, PLAYER_MOVE_SPEED_DIAGNAL_N );
        }
        // pure south
        else
        {
            subpixel_inc( player_pos_dy, PLAYER_MOVE_SPEED_CARDINAL_N );
        }
    }
    // pure east
    else if( flags_mark( dir, PLAYER_MOVE_DIRECTION_EAST ) )
    {
        subpixel_inc( player_pos_dx, PLAYER_MOVE_SPEED_CARDINAL_P );
    }
    // pure west
    else if( flags_mark( dir, PLAYER_MOVE_DIRECTION_WEST ) )
    {
        subpixel_inc( player_pos_dx, PLAYER_MOVE_SPEED_CARDINAL_N );
    }

    // player moved positions
    flags_mark( player_changed_flags, PLAYER_CHANGED_POSITION );
}

static void __fastcall__ perform_slow_move(uint8_t dir)
{
    if( flags_mark( dir, PLAYER_MOVE_DIRECTION_NORTH ) )
    {
        // nw
        if( flags_mark( dir, PLAYER_MOVE_DIRECTION_WEST ) )
        {
            subpixel_inc( player_pos_dx, PLAYER_MOVE_SPEED_SLOW_DIAGNAL_N );
            subpixel_inc( player_pos_dy, PLAYER_MOVE_SPEED_SLOW_DIAGNAL_P );
        }
        // ne
        else if( flags_mark( dir, PLAYER_MOVE_DIRECTION_EAST ) )
        {
            subpixel_inc( player_pos_dx, PLAYER_MOVE_SPEED_SLOW_DIAGNAL_P );
            subpixel_inc( player_pos_dy, PLAYER_MOVE_SPEED_SLOW_DIAGNAL_P );
        }
        // pure north
        else
        {
            subpixel_inc( player_pos_dy, PLAYER_MOVE_SPEED_SLOW_CARDINAL_P );
        }
    }
    else if( flags_mark( dir, PLAYER_MOVE_DIRECTION_SOUTH ) )
    {
        // sw
        if( flags_mark( dir, PLAYER_MOVE_DIRECTION_WEST ) )
        {
            subpixel_inc( player_pos_dx, PLAYER_MOVE_SPEED_SLOW_DIAGNAL_N );
            subpixel_inc( player_pos_dy, PLAYER_MOVE_SPEED_SLOW_DIAGNAL_N );
        }
        // se
        else if( flags_mark( dir, PLAYER_MOVE_DIRECTION_EAST ) )
        {
            subpixel_inc( player_pos_dx, PLAYER_MOVE_SPEED_SLOW_DIAGNAL_P );
            subpixel_inc( player_pos_dy, PLAYER_MOVE_SPEED_SLOW_DIAGNAL_N );
        }
        // pure south
        else
        {
            subpixel_inc( player_pos_dy, PLAYER_MOVE_SPEED_SLOW_CARDINAL_N );
        }
    }
    // pure east
    else if( flags_mark( dir, PLAYER_MOVE_DIRECTION_EAST ) )
    {
        subpixel_inc( player_pos_dx, PLAYER_MOVE_SPEED_SLOW_CARDINAL_P );
    }
    // pure west
    else if( flags_mark( dir, PLAYER_MOVE_DIRECTION_WEST ) )
    {
        subpixel_inc( player_pos_dx, PLAYER_MOVE_SPEED_SLOW_CARDINAL_N );
    }

    // player moved positions
    flags_mark( player_changed_flags, PLAYER_CHANGED_POSITION );
}
