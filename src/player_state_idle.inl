
//
static void __fastcall__ player_state_idle_enter(void)
{
    // TODO: play idle animation

    // can perform any action while in idle state
    flags_set( player_can_perform_action_flags, PLAYER_CAN_PERFORM_ACTION_ALL );
}

static void __fastcall__ player_state_idle_update(void)
{

}

static void __fastcall__ player_state_idle_leave(void)
{

}

static void __fastcall__ player_state_idle_process_input(void)
{

}
