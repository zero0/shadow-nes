
enum
{
    BOSS_0_MAX_HEALTH = 1792,
    BOSS_0_HEALTH_PER_BLOCK_LOG2 = 7,
    BOSS_0_MAX_STAMINA = 100,
    BOSS_0_STAMINA_REGEN = 10,
};

#define BOSS_0_HEALTH       {   \
    BOSS_0_MAX_HEALTH,          \
    BOSS_0_MAX_HEALTH / 2,      \
}
#define BOSS_0_STAMINA      {   \
    BOSS_0_MAX_STAMINA,         \
    BOSS_0_STAMINA_REGEN,       \
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