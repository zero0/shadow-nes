
// test if dodge can be performed
// 1. player has stamina
// 2. dodge cooldown timer is done
// 3. can perform dodge
#define can_perform_dodge()                                                             \
(                                                                                       \
    player_stamina > 0 &&                                                               \
    timer_is_done( player_dodge_cooldown_timer ) &&                                     \
    flags_is_set( player_can_perform_action_flags, PLAYER_CAN_PERFORM_ACTION_DODGE )    \
)

const static subpixel_diff_t PLAYER_DODGE_SPEED_CARDINAL_P = {  2, 192 };    // 2.75
const static subpixel_diff_t PLAYER_DODGE_SPEED_CARDINAL_N = { -2, 192 };    // 2.75
const static subpixel_diff_t PLAYER_DODGE_SPEED_DIAGNAL_P  = {  1, 241 };    // 1.94
const static subpixel_diff_t PLAYER_DODGE_SPEED_DIAGNAL_N  = { -1, 241 };    // 1.94

static void __fastcall__ perform_dodge(uint8_t dir)
{
    player_next_state = PLAYER_STATE_DODGING;

    if( flags_mark( dir, PLAYER_MOVE_DIRECTION_NORTH ) )
    {
        // nw
        if( flags_mark( dir, PLAYER_MOVE_DIRECTION_WEST ) )
        {
            subpixel_inc( player_pos_dx, PLAYER_DODGE_SPEED_DIAGNAL_N );
            subpixel_inc( player_pos_dy, PLAYER_DODGE_SPEED_DIAGNAL_P );
        }
        // ne
        else if( flags_mark( dir, PLAYER_MOVE_DIRECTION_EAST ) )
        {
            subpixel_inc( player_pos_dx, PLAYER_DODGE_SPEED_DIAGNAL_P );
            subpixel_inc( player_pos_dy, PLAYER_DODGE_SPEED_DIAGNAL_P );
        }
        // pure north
        else
        {
            subpixel_inc( player_pos_dy, PLAYER_DODGE_SPEED_CARDINAL_P );
        }
    }
    else if( flags_mark( dir, PLAYER_MOVE_DIRECTION_SOUTH ) )
    {
        // sw
        if( flags_mark( dir, PLAYER_MOVE_DIRECTION_WEST ) )
        {
            subpixel_inc( player_pos_dx, PLAYER_DODGE_SPEED_DIAGNAL_N );
            subpixel_inc( player_pos_dy, PLAYER_DODGE_SPEED_DIAGNAL_N );
        }
        // se
        else if( flags_mark( dir, PLAYER_MOVE_DIRECTION_EAST ) )
        {
            subpixel_inc( player_pos_dx, PLAYER_DODGE_SPEED_DIAGNAL_P );
            subpixel_inc( player_pos_dy, PLAYER_DODGE_SPEED_DIAGNAL_N );
        }
        // pure south
        else
        {
            subpixel_inc( player_pos_dy, PLAYER_DODGE_SPEED_CARDINAL_N );
        }
    }
    // pure east
    else if( flags_mark( dir, PLAYER_MOVE_DIRECTION_EAST ) )
    {
        subpixel_inc( player_pos_dx, PLAYER_DODGE_SPEED_CARDINAL_P );
    }
    // pure west
    else if( flags_mark( dir, PLAYER_MOVE_DIRECTION_WEST ) )
    {
        subpixel_inc( player_pos_dx, PLAYER_DODGE_SPEED_CARDINAL_N );
    }

    // player moved positions
    flags_mark( player_changed_flags, PLAYER_CHANGED_POSITION );
}

static void __fastcall__ player_state_dodge_enter()
{
    // TODO: play dodge animation
    timer_set( player_animation_frame_timer, 50 );

    // set Iframes timer
    timer_set( player_inv_frame_timer, PLAYER_DODGE_INV_TIMER );

    // clear perform flags
    flags_reset( player_can_perform_action_flags );
}

static void __fastcall__ player_state_dodge_update()
{
    if( timer_is_done( player_animation_frame_timer ) )
    {
        player_next_state = PLAYER_STATE_IDLE;
    }
    else
    {
        //
        if( player_animation_frame_timer == 15 )
        {
            flags_mark( player_can_perform_action_flags, PLAYER_CAN_PERFORM_ACTION_ATTACK0 | PLAYER_CAN_PERFORM_ACTION_ATTACK1 );
        }

        if( player_animation_frame_timer == 5 )
        {
            flags_mark( player_can_perform_action_flags, PLAYER_CAN_PERFORM_ACTION_ALL );
        }
    }
}

static void __fastcall__ player_state_dodge_leave()
{
    // reset dodge cooldown on exit, regarless of interupt
    timer_set( player_dodge_cooldown_timer, PLAYER_DODGE_COOLDOWN_TIME );
}
