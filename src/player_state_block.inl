
static uint8_t player_blocking_state;
static timer_t player_parry_timer;
static timer_t player_block_cooldown_timer;

enum
{
    PLAYER_PARRY_TIMER = 10,
    PLAYER_BLOCK_COOLDOWN = 20,
};

#define can_perform_block()         \
(                                   \
    player_blocking_state == 0  &&  \
    timer_is_done( player_block_cooldown_timer ) && \
    flags_is_set( player_can_perform_action_flags, PLAYER_CAN_PERFORM_ACTION_BLOCK ) && \
    1                               \
)

#define can_queue_block()           \
(                                   \
    player_blocking_state == 0  &&  \
    flags_is_set( player_can_perform_action_flags, PLAYER_CAN_PERFORM_ACTION_BLOCK ) && \
    1                               \
)

#define can_perform_parry()     \
(                               \
    player_parry_timer > 0 &&   \
    1                           \
)

static void __fastcall__ player_state_block_enter(void)
{
    player_blocking_state = 1;
    timer_set( player_parry_timer, PLAYER_PARRY_TIMER );
}

static void __fastcall__ player_state_block_update(void)
{
    timer_tick( player_parry_timer );
}

static void __fastcall__ player_state_block_leave(void)
{
    player_blocking_state = 0;
    timer_set( player_block_cooldown_timer, PLAYER_BLOCK_COOLDOWN );
}

static void __fastcall__ perform_block(void)
{
    player_next_state = PLAYER_STATE_BLOCKING;
}

#define tick_block_timers()             \
do                                      \
{                                       \
    timer_tick( player_block_cooldown_timer );   \
}                                       \
while( 0 )