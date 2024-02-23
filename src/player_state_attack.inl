
static uint8_t player_current_attack;
static uint8_t player_current_attack_combo;

static const uint8_t player_attack_0_combo_base_damage[] =
{
    10,
    10,
    12,
    15,
};
static const uint8_t player_attack_0_combo_base_damage_type[] =
{
    DAMAGE_TYPE_PHYSICAL,
    DAMAGE_TYPE_PHYSICAL,
    DAMAGE_TYPE_PHYSICAL,
    DAMAGE_TYPE_PHYSICAL,
};
static const uint8_t player_attack_0_combo_animation[] =
{
    0,
    0,
    0,
    0
};
static const uint8_t player_attack_0_combo_animation_frames[] =
{
    0,
    0,
    0,
    0
};
STATIC_ASSERT( ARRAY_SIZE(player_attack_0_combo_base_damage) == ARRAY_SIZE(player_attack_0_combo_base_damage_type) );
STATIC_ASSERT( ARRAY_SIZE(player_attack_0_combo_base_damage) == ARRAY_SIZE(player_attack_0_combo_animation) );

static const uint8_t player_attack_1_combo_base_damage[] =
{
    14, 16, 20
};

// can perform attack 0
// 1. player has stamina
// 2. attack timer is done
// 3. can perform attack0
#define can_perform_attack0()                                                           \
(                                                                                       \
    player_stamina > 0 &&                                                               \
    timer_is_done( player_attack_cooldown_timer ) &&                                    \
    flags_is_set( player_can_perform_action_flags, PLAYER_CAN_PERFORM_ACTION_ATTACK0 )  \
)

#define can_queue_attack0()     (1)

static void __fastcall__ perform_attack0(void)
{
    player_next_state = PLAYER_STATE_ATTACKING;
    if( player_current_attack != 0 )
    {
        player_current_attack = 0;
        player_current_attack_combo = 0;
    }
    else
    {
        if( timer_is_done( player_attack_combo_timer ) )
        {
            player_current_attack_combo = 0;
        }
        else
        {
            ++player_current_attack_combo;

            if( player_current_attack_combo == ARRAY_SIZE( player_attack_0_combo_base_damage ) )
            {
                player_current_attack_combo = 0;
            }
        }
    }


    ////if( test_attack_hits_boss( DAMAGE_LOCATION_COLUMN_CENTER ) )
    //{
    //    // build damange
    //    _temp_dmg.damage_type = MAKE_DAMAGE_TYPE(0, DAMAGE_TYPE_PHYSICAL);
    //    _temp_dmg.damage = 10;
    //
    //    // modify damage based on stats
    //    MOD_OUTGOING_DAMAGE_FROM_STATUS(_temp_dmg, player_damage_status);
    //
    //    // queue boss damage
    //    queue_damage_boss( _temp_dmg.damage_type, _temp_dmg.damage );
    //}
}

// can perform attack 1
// 1. player has stamina
// 2. attack timer is done
// 3. can perform attack1
#define can_perform_attack1()                                                           \
(                                                                                       \
    player_stamina > 0 &&                                                               \
    timer_is_done( player_attack_cooldown_timer ) &&                                    \
    flags_is_set( player_can_perform_action_flags, PLAYER_CAN_PERFORM_ACTION_ATTACK1 )  \
)

#define can_queue_attack1()     (1)

static void __fastcall__ perform_attack1(void)
{
    player_next_state = PLAYER_STATE_ATTACKING;
}

//
//
//

static void __fastcall__ player_state_attack_enter(void)
{

}

static void __fastcall__ player_state_attack_leave(void)
{

}

static void __fastcall__ player_state_attack_update(void)
{
    player_current_attack = 0xFF;
    player_current_attack_combo = 0;
}
