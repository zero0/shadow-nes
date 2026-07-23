#ifndef GAME_DATA_H
#define GAME_DATA_H

#include "types.h"
#include "macros.h"
#include "flags.h"
#include "boss.h"

enum
{
    SAVE_SLOT_0,
    SAVE_SLOT_1,
    SAVE_SLOT_2,
    _SAVE_SLOT_COUNT
};

enum
{
    CURRENCY_0,
    CURRENCY_1,
    _CURRENCY_COUNT
};

enum
{
    SAVE_SLOT_NAME_MAX_LEN = 15,
};

typedef struct {
    uint8_t version;

    uint8_t player_level;
    uint8_t player_type;

    uint8_t current_game_flow;
    uint8_t current_game_flow_last_checkpoint;
    uint8_t current_difficulty;

    flags8_t difficulties_defeated;
    flags8_t bosses_defeated[_BOSS_COUNT];

    uint8_t currency[_CURRENCY_COUNT];

    uint8_t name_len;
    char name[SAVE_SLOT_NAME_MAX_LEN];
} game_data_t;


typedef struct {
    flags8_t flags0;
    flags8_t flags1;
} global_settings_data_t;

extern game_data_t g_current_game_data;


void game_data_load_for_new_game(void);

void game_data_peek_from_save_slot(uint8_t saveSlot);

void game_data_load_from_save_slot(uint8_t saveSlot);

void game_data_save_to_save_slot(uint8_t saveSlot);

void game_data_load_from_code(uint8_t code);

void game_data_save_to_code(uint16_t code);

//
//
//

enum
{
    GLOBAL_SETTINGS_0_MUSIC         = 1 << 0,
    GLOBAL_SETTINGS_0_SFX           = 1 << 1,
    //GLOBAL_SETTINGS_0__UNDEF      = 1 << 2,
    //GLOBAL_SETTINGS_0__UNDEF      = 1 << 3,
    //GLOBAL_SETTINGS_0__UNDEF      = 1 << 4,
    //GLOBAL_SETTINGS_0__UNDEF      = 1 << 5,

#if DEMO_BUILD
    GLOBAL_SETTINGS_0_DEMO_MODE     = 1 << 6,
#endif

#if !DISTRO_BUILD
    GLOBAL_SETTINGS_0_DEBUG_MODE    = 1 << 7,
#endif

    // default settings
    _GLOBAL_SETTINGS_0_DEFAULT      = GLOBAL_SETTINGS_0_MUSIC | GLOBAL_SETTINGS_0_SFX
};

enum
{
    GLOBAL_SETTINGS_1_SKIP_CUTSCENES        = 1 << 0,
    GLOBAL_SETTINGS_1_SKIP_BOSS_INTRO       = 1 << 1,
    GLOBAL_SETTINGS_1_SKIP_BOSS_OUTRO       = 1 << 2,
    GLOBAL_SETTINGS_1_SKIP_CHECKPOINTS      = 1 << 3,
    GLOBAL_SETTINGS_1_SKIP_SHOPS            = 1 << 4,

#if !DISTRO_BUILD
    GLOBAL_SETTINGS_1_DEBUG_ONE_PUNCH       = 1 << 5,
    GLOBAL_SETTINGS_1_DEBUG_FREE_SHOPS      = 1 << 6,
    GLOBAL_SETTINGS_1_DEBUG_INVINCIBLE      = 1 << 7,
#endif

    _GLOBAL_SETTINGS_1_DEFAULT              = 0
};

extern global_settings_data_t g_global_settings_data;

void game_settings_reset(void);

#define game_settings_is_music_enabled()        (flags_is_set(g_global_settings_data.flags0, GLOBAL_SETTINGS_0_MUSIC))
#define game_settings_toggle_music_enabled()    CODE(flags_toggle(g_global_settings_data.flags0, GLOBAL_SETTINGS_0_MUSIC);)

#define game_settings_is_sfx_enabled()          (flags_is_set(g_global_settings_data.flags0, GLOBAL_SETTINGS_0_SFX))
#define game_settings_toggle_sfx_enabled()      CODE(flags_toggle(g_global_settings_data.flags0, GLOBAL_SETTINGS_0_SFX);)

#define CODE(x)    do { x } while(0)

// Demo Build functions
#if DEMO_BUILD
#define game_settings_is_demo_mode_enabled()    (flags_is_set(g_global_settings_data.flags0, GLOBAL_SETTINGS_0_DEMO_MODE))
#define game_settings_enable_demo_mode()        do {                            \
    flags_mark(g_global_settings_data.flags0, GLOBAL_SETTINGS_0_DEMO_MODE);     \
} while(0)
#define game_settings_disable_demo_mode()       do {                            \
    flags_unmark(g_global_settings_data.flags0, GLOBAL_SETTINGS_0_DEMO_MODE);   \
} while(0)
#else
#define game_settings_is_demo_mode_enabled()    (0)
#define game_settings_enable_demo_mode()        (void)0
#define game_settings_disable_demo_mode()       (void)0
#endif // DEMO_BUILD

// Debug Mode functions
#if DISTRO_BUILD
#define game_settings_is_debug_mode_enabled()   (0)
#define game_settings_enable_debug_mode()       (void)0
#define game_settings_disable_debug_mode()      (void)0
#else
#define game_settings_is_debug_mode_enabled()   (flags_is_set(g_global_settings_data.flags0, GLOBAL_SETTINGS_0_DEBUG_MODE))
#define game_settings_enable_debug_mode()       CODE(flags_mark(g_global_settings_data.flags0, GLOBAL_SETTINGS_0_DEBUG_MODE))
#define game_settings_disable_debug_mode()      CODE(                           \
    flags_unmark(g_global_settings_data.flags0, GLOBAL_SETTINGS_0_DEBUG_MODE);  \
)
#endif // DISTRO_BUILD


#endif // GAME_DATA_H
