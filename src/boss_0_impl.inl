
enum
{
    BOSS_0_MAX_HEALTH = 1792,
    BOSS_0_HEALTH_PER_BLOCK_LOG2 = 7,
    BOSS_0_MAX_STAMINA = 100,
    BOSS_0_STAMINA_REGEN = 10,
    BOSS_0_STAMINA_REGEN_TIMER = 10,
    BOSS_0_INV_FRAMES_COOLDONW_TIMER = 12,
};

#define BOSS_0_HEALTH       {   \
    BOSS_0_MAX_HEALTH,          \
    BOSS_0_MAX_HEALTH / 2,      \
}
#define BOSS_0_STAMINA      {   \
    BOSS_0_MAX_STAMINA,         \
    BOSS_0_STAMINA_REGEN,       \
    BOSS_0_STAMINA_REGEN_TIMER, \
}
#define BOSS_0_RESISTANCE   {   \
    0,                          \
    5,                          \
    5,                          \
    5,                          \
    5,                          \
    5,                          \
    5,                          \
    5,                          \
    5,                          \
}
#define BOSS_0_NAME         tr_boss_0_name
#define BOSS_0_LOCATION     tr_boss_0_location
#define BOSS_0_MUSIC        {   \
    0,                          \
    0,                          \
    0,                          \
    0,                          \
    0,                          \
}
#define BOSS_0_COOLDOWN_TIMERS {        \
    BOSS_0_INV_FRAMES_COOLDONW_TIMER,   \
}

//
// Boss Functions
//

static void __fastcall__ boss_0_init(void)
{
    boss_hit_box.ext_x = 10;
    boss_hit_box.ext_y = 10;
}

static void __fastcall__ boss_0_update(void)
{

}
