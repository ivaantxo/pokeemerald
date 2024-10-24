#ifndef GUARD_SCRIPT_MENU_CONSTANTS_H
#define GUARD_SCRIPT_MENU_CONSTANTS_H

enum MultichoiceIDs
{
    MULTI_BRINEY_ON_DEWFORD,
    MULTI_PC,
    MULTI_ENTERINFO,
    MULTI_CONTEST_INFO,
    MULTI_CONTEST_TYPE,
    MULTI_BASE_PC_NO_REGISTRY,
    MULTI_BASE_PC_WITH_REGISTRY,
    MULTI_REGISTER_MENU,
    MULTI_SSTIDAL_LILYCOVE,
    MULTI_FRONTIER_PASS_INFO,
    MULTI_BIKE,
    MULTI_STATUS_INFO,
    MULTI_BRINEY_OFF_DEWFORD,
    MULTI_VIEWED_PAINTINGS,
    MULTI_YESNOINFO,
    MULTI_BATTLE_MODE,
    MULTI_YESNOINFO_2,
    MULTI_CHALLENGEINFO,
    MULTI_LEVEL_MODE,
    MULTI_MECHADOLL1_Q1,
    MULTI_MECHADOLL1_Q2,
    MULTI_MECHADOLL1_Q3,
    MULTI_MECHADOLL2_Q1,
    MULTI_MECHADOLL2_Q2,
    MULTI_MECHADOLL2_Q3,
    MULTI_MECHADOLL3_Q1,
    MULTI_MECHADOLL3_Q2,
    MULTI_MECHADOLL3_Q3,
    MULTI_MECHADOLL4_Q1,
    MULTI_MECHADOLL4_Q2,
    MULTI_MECHADOLL4_Q3,
    MULTI_MECHADOLL5_Q1,
    MULTI_MECHADOLL5_Q2,
    MULTI_MECHADOLL5_Q3,
    MULTI_VENDING_MACHINE,
    MULTI_MACH_BIKE_INFO,
    MULTI_ACRO_BIKE_INFO,
    MULTI_SATISFACTION,
    MULTI_STERN_DEEPSEA,
    MULTI_GAME_CORNER_DOLLS,
    MULTI_GAME_CORNER_COINS,
    MULTI_HOWS_FISHING,
    MULTI_SSTIDAL_SLATEPORT_WITH_BF,
    MULTI_SSTIDAL_BATTLE_FRONTIER,
    MULTI_RIGHTLEFT ,
    MULTI_GAME_CORNER_TMS,
    MULTI_SSTIDAL_SLATEPORT_NO_BF,
    MULTI_FLOORS,
    MULTI_SHARDS_R,
    MULTI_SHARDS_Y,
    MULTI_SHARDS_RY,
    MULTI_SHARDS_B,
    MULTI_SHARDS_RB,
    MULTI_SHARDS_YB,
    MULTI_SHARDS_RYB,
    MULTI_SHARDS_G,
    MULTI_SHARDS_RG,
    MULTI_SHARDS_YG,
    MULTI_SHARDS_RYG,
    MULTI_SHARDS_BG,
    MULTI_SHARDS_RBG,
    MULTI_SHARDS_YBG,
    MULTI_SHARDS_RYBG,
    MULTI_TOURNEY_WITH_RECORD,
    MULTI_WIRELESS_NO_RECORD_BERRY,
    MULTI_WIRELESS_NO_BERRY,
    MULTI_WIRELESS_NO_RECORD,
    MULTI_WIRELESS_ALL_SERVICES,
    MULTI_WIRELESS_MINIGAME,
    MULTI_LINK_LEADER,
    MULTI_CONTEST_RANK,
    MULTI_FRONTIER_ITEM_CHOOSE,
    MULTI_LINK_CONTEST_INFO,
    MULTI_LINK_CONTEST_MODE,
    MULTI_FORCED_START_MENU,
    MULTI_FRONTIER_GAMBLER_BET,
    MULTI_TENT,
    MULTI_FOSSIL,
    MULTI_YESNO,
    MULTI_FRONTIER_RULES,
    MULTI_BATTLE_ARENA_RULES,
    MULTI_BATTLE_TOWER_RULES,
    MULTI_BATTLE_DOME_RULES,
    MULTI_BATTLE_FACTORY_RULES,
    MULTI_BATTLE_PALACE_RULES,
    MULTI_BATTLE_PYRAMID_RULES,
    MULTI_BATTLE_PIKE_RULES,
    MULTI_GO_ON_RECORD_REST_RETIRE,
    MULTI_GO_ON_REST_RETIRE,
    MULTI_GO_ON_RECORD_RETIRE,
    MULTI_GO_ON_RETIRE,
    MULTI_TOURNEY_NO_RECORD,
    MULTI_TV_LATI,
    MULTI_BATTLE_TOWER_FEELINGS,
    MULTI_WHERES_RAYQUAZA,
    MULTI_SLATEPORT_TENT_RULES,
    MULTI_FALLARBOR_TENT_RULES,
    MULTI_TAG_MATCH_TYPE,
    MULTI_BERRY_PLOT,
};

// Eliminar
enum LilycoveSSTidalMultichoice
{
    SSTIDAL_SELECTION_SLATEPORT,
    SSTIDAL_SELECTION_BATTLE_FRONTIER,
    SSTIDAL_SELECTION_SOUTHERN_ISLAND,
    SSTIDAL_SELECTION_NAVEL_ROCK,
    SSTIDAL_SELECTION_BIRTH_ISLAND,
    SSTIDAL_SELECTION_FARAWAY_ISLAND,
    SSTIDAL_SELECTION_EXIT,

    SSTIDAL_SELECTION_COUNT
};

enum StdStringIds
{
    STDSTRING_COOL,
    STDSTRING_BEAUTY,
    STDSTRING_CUTE,
    STDSTRING_SMART,
    STDSTRING_TOUGH,
    STDSTRING_NORMAL,
    STDSTRING_SUPER,
    STDSTRING_HYPER,
    STDSTRING_MASTER,
    STDSTRING_ITEMS,
    STDSTRING_KEYITEMS,
    STDSTRING_POKEBALLS,
    STDSTRING_TMHMS,
    STDSTRING_BERRIES,
    STDSTRING_SINGLE,
    STDSTRING_DOUBLE,
    STDSTRING_MULTI,
    STDSTRING_MULTI_LINK,
    STDSTRING_BATTLE_TOWER,
    STDSTRING_BATTLE_DOME,
    STDSTRING_BATTLE_FACTORY,
    STDSTRING_BATTLE_PALACE,
    STDSTRING_BATTLE_ARENA,
    STDSTRING_BATTLE_PIKE,
    STDSTRING_BATTLE_PYRAMID,
};

#define MULTICHOICE(name) {name, ARRAY_COUNT(name)}

#define MAX_MULTICHOICE_WIDTH 28

#define MULTI_B_PRESSED  127

// Dynamic Multichoice Callbacks

#define DYN_MULTICHOICE_CB_DEBUG      0
#define DYN_MULTICHOICE_CB_SHOW_ITEM  1
#define DYN_MULTICHOICE_CB_NONE       255

#endif //GUARD_SCRIPT_MENU_CONSTANTS_H
