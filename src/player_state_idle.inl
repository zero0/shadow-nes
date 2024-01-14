
//
static __fastcall__ player_state_idle_enter()
{
    // TODO: play idle animation

    // can perform any action while in idle state
    flags_set( player_can_perform_action_flags, PLAYER_CAN_PERFORM_ACTION_ALL );
}
