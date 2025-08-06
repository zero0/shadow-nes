
enum
{
    PLAYER_FLASK_ANIMATION_TIME = 40,
    PLAYER_FLASK_ANIMATION_CAN_PERFORM_MOVE_TIME = 20,
    PLAYER_FLASK_ANIMATION_CAN_PERFORM_ACTION_TIME = 20,
};

static timer_t player_flask_cooldown_timer;

// test if flask can be used
// 1. player has enough flasks
// 2. flask cooldown is done
// 3. can actually perform flask action
#define can_perform_flask()                                                             \
(                                                                                       \
    player_flasks > 0 &&                                                                \
    timer_is_done( player_flask_cooldown_timer ) &&                                     \
    flags_is_set( player_can_perform_action_flags, PLAYER_CAN_PERFORM_ACTION_FLASK ) && \
    1   \
)

#define can_queue_flask()       \
( 1 )

static void __fastcall__ perform_flask(void)
{
    player_next_state = PLAYER_STATE_USING_FLASK;
}

static void __fastcall__ player_state_flask_enter(void)
{
    // TODO: implement actual animation
    // play flask animation
    timer_set( player_flask_cooldown_timer, PLAYER_FLASK_ANIMATION_TIME );

    // reset action flags (cannot interupt)
    flags_reset( player_can_perform_action_flags );
}

static void __fastcall__ player_state_flask_update(void)
{
    // leave state and heal when animation is done so if interuptted, heal does not work
    if( timer_is_done( player_flask_cooldown_timer ) )
    {
        // leave state when animation is done
        player_next_state = PLAYER_STATE_IDLE;

        // enquque heal
        ASSERT(player_damage_queue_length < ARRAY_SIZE(player_damage_queue));
        player_damage_queue[player_damage_queue_length].damage_type = MAKE_DAMAGE_TYPE_ATTR( DAMAGE_TYPE_ATTR_HEAL, DAMAGE_TYPE_FLAT );
        player_damage_queue[player_damage_queue_length].damage = player_flask_heal_per_level[g_current_game_data.player_level];
        ++player_damage_queue_length;

        // use flask
        ASSERT(player_flasks > 0);
        --player_flasks;

        // mark flask changed
        flags_mark( player_changed_flags, PLAYER_CHANGED_FLASKS );
    }
    else
    {
        // enable movement flags after a number of frames
        if( player_flask_cooldown_timer == PLAYER_FLASK_ANIMATION_CAN_PERFORM_MOVE_TIME )
        {
            flags_mark( player_can_perform_action_flags, PLAYER_CAN_PERFORM_ACTION_MOVE | PLAYER_CAN_PERFORM_ACTION_DODGE );
        }

        // set action interupts after a number of frames
        if( player_flask_cooldown_timer == PLAYER_FLASK_ANIMATION_CAN_PERFORM_ACTION_TIME )
        {
            flags_mark( player_can_perform_action_flags, PLAYER_CAN_PERFORM_ACTION_ATTACK0 | PLAYER_CAN_PERFORM_ACTION_ATTACK1 );
        }
    }
}

static void __fastcall__ player_state_flask_leave(void)
{
    // TODO: cancel animation

    // reset timer regarless of interupt
    timer_set( player_flask_cooldown_timer, PLAYER_FLASK_COOLDOWN_TIME );

    // probably don't have to since entering a new state will play most likely play an animtaion
    timer_set( player_flask_cooldown_timer, 0 );
}

#define tick_flask_timers()             \
do                                      \
{                                       \
    timer_tick( player_flask_cooldown_timer );   \
}                                       \
while( 0 )