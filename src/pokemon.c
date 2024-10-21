#include "global.h"
#include "malloc.h"
#include "apprentice.h"
#include "battle.h"
#include "battle_ai_switch_items.h"
#include "battle_anim.h"
#include "battle_controllers.h"
#include "battle_message.h"
#include "battle_pike.h"
#include "battle_pyramid.h"
#include "battle_setup.h"
#include "battle_tower.h"
#include "battle_z_move.h"
#include "data.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "evolution_scene.h"
#include "field_specials.h"
#include "field_weather.h"
#include "graphics.h"
#include "item.h"
#include "caps.h"
#include "link.h"
#include "main.h"
#include "overworld.h"
#include "m4a.h"
#include "party_menu.h"
#include "pokedex.h"
#include "pokeblock.h"
#include "pokemon.h"
#include "pokemon_animation.h"
#include "pokemon_icon.h"
#include "pokemon_summary_screen.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "recorded_battle.h"
#include "rtc.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "trainer_hill.h"
#include "util.h"
#include "constants/abilities.h"
#include "constants/battle_frontier.h"
#include "constants/battle_move_effects.h"
#include "constants/battle_script_commands.h"
#include "constants/battle_partner.h"
#include "constants/cries.h"
#include "constants/event_objects.h"
#include "constants/form_change_types.h"
#include "constants/hold_effects.h"
#include "constants/item_effects.h"
#include "constants/items.h"
#include "constants/layouts.h"
#include "constants/moves.h"
#include "constants/songs.h"
#include "constants/species.h"
#include "constants/trainers.h"
#include "constants/weather.h"
#include "wild_encounter.h"

#define FRIENDSHIP_EVO_THRESHOLD ((P_FRIENDSHIP_EVO_THRESHOLD >= GEN_8) ? 160 : 220)

struct SpeciesItem
{
    u16 species;
    u16 item;
};

static void Task_PlayMapChosenOrBattleBGM(u8 taskId);
static bool8 ShouldSkipFriendshipChange(void);
static void RemoveIVIndexFromList(u8 *ivs, u8 selectedIv);
void TrySpecialOverworldEvo();

EWRAM_DATA static u8 sLearningMoveTableID = 0;
EWRAM_DATA u8 gPlayerPartyCount = 0;
EWRAM_DATA u8 gEnemyPartyCount = 0;
EWRAM_DATA struct Pokemon gPlayerParty[PARTY_SIZE] = {0};
EWRAM_DATA struct Pokemon gEnemyParty[PARTY_SIZE] = {0};
EWRAM_DATA struct SpriteTemplate gMultiuseSpriteTemplate = {0};
EWRAM_DATA static struct MonSpritesGfxManager *sMonSpritesGfxManager = NULL;
EWRAM_DATA static u8 sTriedEvolving = 0;
EWRAM_DATA u16 gFollowerSteps = 0;

#include "data/moves_info.h"
#include "data/abilities.h"

// In Battle Palace, moves are chosen based on the pokemons nature rather than by the player
// Moves are grouped into "Attack", "Defense", or "Support" (see PALACE_MOVE_GROUP_*)
// Each nature has a certain percent chance of selecting a move from a particular group
// and a separate percent chance for each group when at or below 50% HP
// The table below doesn't list percentages for Support because you can subtract the other two
// Support percentages are listed in comments off to the side instead
#define PALACE_STYLE(atk, def, atkLow, defLow) {atk, atk + def, atkLow, atkLow + defLow}

const struct NatureInfo gNaturesInfo[NUM_NATURES] =
{
    [NATURE_OFENSIVA] =
    {
        .name = COMPOUND_STRING("Ofensiva"),
        .statUp = STAT_ATK,
        .backAnim = 1,
        .pokeBlockAnim = {ANIM_HARDY, AFFINE_NONE},
        .natureGirlMessage = BattleFrontier_Lounge5_Text_NatureGirlHardy,
        .battlePalacePercents = PALACE_STYLE(61, 7, 61, 7), //32% support >= 50% HP, 32% support < 50% HP
        .battlePalaceFlavorText = B_MSG_EAGER_FOR_MORE,
        .battlePalaceSmokescreen = PALACE_TARGET_STRONGER,
    },
    [NATURE_DEFENSIVA] =
    {
        .name = COMPOUND_STRING("Defensiva"),
        .statUp = STAT_DEF,
        .backAnim = 2,
        .pokeBlockAnim = {ANIM_LONELY, AFFINE_NONE},
        .natureGirlMessage = BattleFrontier_Lounge5_Text_NatureGirlLonely,
        .battlePalacePercents = PALACE_STYLE(20, 25, 84, 8), //55%,  8%
        .battlePalaceFlavorText = B_MSG_GLINT_IN_EYE,
        .battlePalaceSmokescreen = PALACE_TARGET_STRONGER,
    },
    [NATURE_OFENSIVA_ESPECIAL] =
    {
        .name = COMPOUND_STRING("Ofensiva esp."),
        .statUp = STAT_SPATK,
        .backAnim = 1,
        .pokeBlockAnim = {ANIM_BRAVE, AFFINE_TURN_UP},
        .natureGirlMessage = BattleFrontier_Lounge5_Text_NatureGirlBrave,
        .battlePalacePercents = PALACE_STYLE(70, 15, 32, 60), //15%, 8%
        .battlePalaceFlavorText = B_MSG_GETTING_IN_POS,
        .battlePalaceSmokescreen = PALACE_TARGET_WEAKER,
    },
    [NATURE_DEFENSIVA_ESPECIAL] =
    {
        .name = COMPOUND_STRING("Defensiva esp."),
        .statUp = STAT_SPDEF,
        .backAnim = 2,
        .pokeBlockAnim = {ANIM_ADAMANT, AFFINE_NONE},
        .natureGirlMessage = BattleFrontier_Lounge5_Text_NatureGirlAdamant,
        .battlePalacePercents = PALACE_STYLE(38, 31, 70, 15), //31%, 15%
        .battlePalaceFlavorText = B_MSG_GLINT_IN_EYE,
        .battlePalaceSmokescreen = PALACE_TARGET_STRONGER,
    },
    [NATURE_RAPIDA] =
    {
        .name = COMPOUND_STRING("Rápida"),
        .statUp = STAT_SPEED,
        .backAnim = 0,
        .pokeBlockAnim = {ANIM_NAUGHTY, AFFINE_NONE},
        .natureGirlMessage = BattleFrontier_Lounge5_Text_NatureGirlNaughty,
        .battlePalacePercents = PALACE_STYLE(20, 70, 70, 22), //10%, 8%
        .battlePalaceFlavorText = B_MSG_GLINT_IN_EYE,
        .battlePalaceSmokescreen = PALACE_TARGET_WEAKER,
    }
};

#include "data/graphics/pokemon.h"
#include "data/pokemon_graphics/front_pic_anims.h"
#include "data/pokemon/trainer_class_lookups.h"
#include "data/pokemon/experience_tables.h"
#include "data/pokemon/level_up_learnsets.h"
#include "data/pokemon/teachable_learnsets.h"
#include "data/pokemon/egg_moves.h"
#include "data/pokemon/form_species_tables.h"
#include "data/pokemon/form_change_tables.h"
#include "data/object_events/object_event_pic_tables_followers.h"

#include "data/pokemon/species_info.h"

#define PP_UP_SHIFTS(val)           val,        (val) << 2,        (val) << 4,        (val) << 6
#define PP_UP_SHIFTS_INV(val) (u8)~(val), (u8)~((val) << 2), (u8)~((val) << 4), (u8)~((val) << 6)

// PP Up bonuses are stored for a Pokémon as a single byte.
// There are 2 bits (a value 0-3) for each move slot that
// represent how many PP Ups have been applied.
// The following arrays take a move slot id and return:
// gPPUpGetMask - A mask to get the number of PP Ups applied to that move slot
// gPPUpClearMask - A mask to clear the number of PP Ups applied to that move slot
// gPPUpAddValues - A value to add to the PP Bonuses byte to apply 1 PP Up to that move slot
const u8 gPPUpGetMask[MAX_MON_MOVES]   = {PP_UP_SHIFTS(3)};
const u8 gPPUpClearMask[MAX_MON_MOVES] = {PP_UP_SHIFTS_INV(3)};
const u8 gPPUpAddValues[MAX_MON_MOVES] = {PP_UP_SHIFTS(1)};

const u8 gStatStageRatios[MAX_STAT_STAGE + 1][2] =
{
    {10.00, 25.00}, // -6, MIN_STAT_STAGE
    {10.00, 22.50}, // -5
    {10.00, 20.00}, // -4
    {10.00, 17.50}, // -3
    {10.00, 15.00}, // -2
    {10.00, 12.50}, // -1
    {10.00, 10.00}, //  0, DEFAULT_STAT_STAGE
    {12.50, 10.00}, // +1
    {15.00, 10.00}, // +2
    {17.50, 10.00}, // +3
    {20.00, 10.00}, // +4
    {22.50, 10.00}, // +5
    {25.00, 10.00}, // +6, MAX_STAT_STAGE
};

const struct SpriteTemplate gBattlerSpriteTemplates[MAX_BATTLERS_COUNT] =
{
    [B_POSITION_PLAYER_LEFT] = {
        .tileTag = TAG_NONE,
        .paletteTag = 0,
        .oam = &gOamData_BattleSpritePlayerSide,
        .anims = NULL,
        .images = gBattlerPicTable_PlayerLeft,
        .affineAnims = gAffineAnims_BattleSpritePlayerSide,
        .callback = SpriteCB_BattleSpriteStartSlideLeft,
    },
    [B_POSITION_OPPONENT_LEFT] = {
        .tileTag = TAG_NONE,
        .paletteTag = 0,
        .oam = &gOamData_BattleSpriteOpponentSide,
        .anims = NULL,
        .images = gBattlerPicTable_OpponentLeft,
        .affineAnims = gAffineAnims_BattleSpriteOpponentSide,
        .callback = SpriteCB_WildMon,
    },
    [B_POSITION_PLAYER_RIGHT] = {
        .tileTag = TAG_NONE,
        .paletteTag = 0,
        .oam = &gOamData_BattleSpritePlayerSide,
        .anims = NULL,
        .images = gBattlerPicTable_PlayerRight,
        .affineAnims = gAffineAnims_BattleSpritePlayerSide,
        .callback = SpriteCB_BattleSpriteStartSlideLeft,
    },
    [B_POSITION_OPPONENT_RIGHT] = {
        .tileTag = TAG_NONE,
        .paletteTag = 0,
        .oam = &gOamData_BattleSpriteOpponentSide,
        .anims = NULL,
        .images = gBattlerPicTable_OpponentRight,
        .affineAnims = gAffineAnims_BattleSpriteOpponentSide,
        .callback = SpriteCB_WildMon
    },
};

static const struct SpriteTemplate sTrainerBackSpriteTemplates[] =
{
    [TRAINER_BACK_PIC_BRENDAN] = {
        .tileTag = TAG_NONE,
        .paletteTag = 0,
        .oam = &gOamData_BattleSpritePlayerSide,
        .anims = NULL,
        .images = gTrainerBackPicTable_Brendan,
        .affineAnims = gAffineAnims_BattleSpritePlayerSide,
        .callback = SpriteCB_BattleSpriteStartSlideLeft,
    },
    [TRAINER_BACK_PIC_MAY] = {
        .tileTag = TAG_NONE,
        .paletteTag = 0,
        .oam = &gOamData_BattleSpritePlayerSide,
        .anims = NULL,
        .images = gTrainerBackPicTable_May,
        .affineAnims = gAffineAnims_BattleSpritePlayerSide,
        .callback = SpriteCB_BattleSpriteStartSlideLeft,
    },
    [TRAINER_BACK_PIC_RED] = {
        .tileTag = TAG_NONE,
        .paletteTag = 0,
        .oam = &gOamData_BattleSpritePlayerSide,
        .anims = NULL,
        .images = gTrainerBackPicTable_Red,
        .affineAnims = gAffineAnims_BattleSpritePlayerSide,
        .callback = SpriteCB_BattleSpriteStartSlideLeft,
    },
    [TRAINER_BACK_PIC_LEAF] = {
        .tileTag = TAG_NONE,
        .paletteTag = 0,
        .oam = &gOamData_BattleSpritePlayerSide,
        .anims = NULL,
        .images = gTrainerBackPicTable_Leaf,
        .affineAnims = gAffineAnims_BattleSpritePlayerSide,
        .callback = SpriteCB_BattleSpriteStartSlideLeft,
    },
    [TRAINER_BACK_PIC_RUBY_SAPPHIRE_BRENDAN] = {
        .tileTag = TAG_NONE,
        .paletteTag = 0,
        .oam = &gOamData_BattleSpritePlayerSide,
        .anims = NULL,
        .images = gTrainerBackPicTable_RubySapphireBrendan,
        .affineAnims = gAffineAnims_BattleSpritePlayerSide,
        .callback = SpriteCB_BattleSpriteStartSlideLeft,
    },
    [TRAINER_BACK_PIC_RUBY_SAPPHIRE_MAY] = {
        .tileTag = TAG_NONE,
        .paletteTag = 0,
        .oam = &gOamData_BattleSpritePlayerSide,
        .anims = NULL,
        .images = gTrainerBackPicTable_RubySapphireMay,
        .affineAnims = gAffineAnims_BattleSpritePlayerSide,
        .callback = SpriteCB_BattleSpriteStartSlideLeft,
    },
    [TRAINER_BACK_PIC_WALLY] = {
        .tileTag = TAG_NONE,
        .paletteTag = 0,
        .oam = &gOamData_BattleSpritePlayerSide,
        .anims = NULL,
        .images = gTrainerBackPicTable_Wally,
        .affineAnims = gAffineAnims_BattleSpritePlayerSide,
        .callback = SpriteCB_BattleSpriteStartSlideLeft,
    },
    [TRAINER_BACK_PIC_STEVEN] = {
        .tileTag = TAG_NONE,
        .paletteTag = 0,
        .oam = &gOamData_BattleSpritePlayerSide,
        .anims = NULL,
        .images = gTrainerBackPicTable_Steven,
        .affineAnims = gAffineAnims_BattleSpritePlayerSide,
        .callback = SpriteCB_BattleSpriteStartSlideLeft,
    },
};

#define NUM_SECRET_BASE_CLASSES 5
static const u8 sSecretBaseFacilityClasses[GENDER_COUNT][NUM_SECRET_BASE_CLASSES] =
{
    [MALE] = {
        FACILITY_CLASS_YOUNGSTER,
        FACILITY_CLASS_BUG_CATCHER,
        FACILITY_CLASS_RICH_BOY,
        FACILITY_CLASS_CAMPER,
        FACILITY_CLASS_COOLTRAINER_M
    },
    [FEMALE] = {
        FACILITY_CLASS_LASS,
        FACILITY_CLASS_SCHOOL_KID_F,
        FACILITY_CLASS_LADY,
        FACILITY_CLASS_PICNICKER,
        FACILITY_CLASS_COOLTRAINER_F
    }
};

static const u8 sGetMonDataEVConstants[] =
{
    MON_DATA_HP_EV,
    MON_DATA_ATK_EV,
    MON_DATA_DEF_EV,
    MON_DATA_SPEED_EV,
    MON_DATA_SPDEF_EV,
    MON_DATA_SPATK_EV
};

// For stat-raising items
static const u8 sStatsToRaise[] =
{
    STAT_ATK, STAT_ATK, STAT_DEF, STAT_SPEED, STAT_SPATK, STAT_SPDEF, STAT_ACC
};

// 3 modifiers each for how much to change friendship for different ranges
// 0-99, 100-199, 200+
static const s8 sFriendshipEventModifiers[][3] =
{
    [FRIENDSHIP_EVENT_GROW_LEVEL]      = { 5,  3,  2},
    [FRIENDSHIP_EVENT_VITAMIN]         = { 5,  3,  2},
    [FRIENDSHIP_EVENT_BATTLE_ITEM]     = { 1,  1,  0},
    [FRIENDSHIP_EVENT_LEAGUE_BATTLE]   = { 3,  2,  1},
    [FRIENDSHIP_EVENT_LEARN_TMHM]      = { 1,  1,  0},
    [FRIENDSHIP_EVENT_WALKING]         = { 1,  1,  1},
    [FRIENDSHIP_EVENT_FAINT_SMALL]     = {-1, -1, -1},
    [FRIENDSHIP_EVENT_FAINT_FIELD_PSN] = {-5, -5, -10},
    [FRIENDSHIP_EVENT_FAINT_LARGE]     = {-5, -5, -10},
};

#define HM_MOVES_END 0xFFFF

static const u16 sHMMoves[] =
{
    MOVE_CUT, MOVE_FLY, MOVE_SURF, MOVE_STRENGTH, MOVE_FLASH,
    MOVE_ROCK_SMASH, MOVE_WATERFALL, MOVE_DIVE, HM_MOVES_END
};

static const struct SpeciesItem sAlteringCaveWildMonHeldItems[] =
{
    {SPECIES_NONE,      ITEM_NONE},
    {SPECIES_MAREEP,    ITEM_GANLON_BERRY},
    {SPECIES_PINECO,    ITEM_APICOT_BERRY},
    {SPECIES_HOUNDOUR,  ITEM_BIG_MUSHROOM},
    {SPECIES_TEDDIURSA, ITEM_PETAYA_BERRY},
    {SPECIES_AIPOM,     ITEM_BERRY_JUICE},
    {SPECIES_STANTLER,  ITEM_PETAYA_BERRY},
};

static const struct OamData sOamData_64x64 =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(64x64),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(64x64),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0
};

static const struct SpriteTemplate sSpriteTemplate_64x64 =
{
    .tileTag = TAG_NONE,
    .paletteTag = TAG_NONE,
    .oam = &sOamData_64x64,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

void ZeroBoxMonData(struct BoxPokemon *boxMon)
{
    u8 *raw = (u8 *)boxMon;
    u32 i;
    for (i = 0; i < sizeof(struct BoxPokemon); i++)
        raw[i] = 0;
}

void ZeroMonData(struct Pokemon *mon)
{
    u32 arg;
    ZeroBoxMonData(&mon->box);
    arg = 0;
    SetMonData(mon, MON_DATA_STATUS, &arg);
    SetMonData(mon, MON_DATA_LEVEL, &arg);
    SetMonData(mon, MON_DATA_HP, &arg);
    SetMonData(mon, MON_DATA_MAX_HP, &arg);
    SetMonData(mon, MON_DATA_ATK, &arg);
    SetMonData(mon, MON_DATA_DEF, &arg);
    SetMonData(mon, MON_DATA_SPEED, &arg);
    SetMonData(mon, MON_DATA_SPATK, &arg);
    SetMonData(mon, MON_DATA_SPDEF, &arg);
    arg = MAIL_NONE;
    SetMonData(mon, MON_DATA_MAIL, &arg);
}

void ZeroPlayerPartyMons(void)
{
    s32 i;
    for (i = 0; i < PARTY_SIZE; i++)
        ZeroMonData(&gPlayerParty[i]);
}

void ZeroEnemyPartyMons(void)
{
    s32 i;
    for (i = 0; i < PARTY_SIZE; i++)
        ZeroMonData(&gEnemyParty[i]);
}

void CreateMon(struct Pokemon *mon, u16 species, u8 level, u8 fixedIV, u8 hasFixedPersonality, u32 fixedPersonality, u8 otIdType, u32 fixedOtId)
{
    u32 mail;
    ZeroMonData(mon);
    CreateBoxMon(&mon->box, species, level, fixedIV, hasFixedPersonality, fixedPersonality, otIdType, fixedOtId);
    SetMonData(mon, MON_DATA_LEVEL, &level);
    mail = MAIL_NONE;
    SetMonData(mon, MON_DATA_MAIL, &mail);
    CalculateMonStats(mon);
}

void CreateBoxMon(struct BoxPokemon *boxMon, u16 species, u8 level, u8 fixedIV, u8 hasFixedPersonality, u32 fixedPersonality, u8 otIdType, u32 fixedOtId)
{
    u8 speciesName[POKEMON_NAME_LENGTH + 1];
    u32 personality;
    u32 value;
    u8 i;
    u8 availableIVs[NUM_STATS];
    u8 selectedIvs[NUM_STATS];
    bool32 isShiny;

    ZeroBoxMonData(boxMon);

    if (hasFixedPersonality)
        personality = fixedPersonality;
    else
        personality = Random32();

    // Determine original trainer ID
    if (otIdType == OT_ID_RANDOM_NO_SHINY)
    {
        value = Random32();
        isShiny = FALSE;
    }
    else if (otIdType == OT_ID_PRESET)
    {
        value = fixedOtId;
        isShiny = GET_SHINY_VALUE(value, personality) < SHINY_ODDS;
    }
    else // Player is the OT
    {
        value = gSaveBlock2Ptr->playerTrainerId[0]
              | (gSaveBlock2Ptr->playerTrainerId[1] << 8)
              | (gSaveBlock2Ptr->playerTrainerId[2] << 16)
              | (gSaveBlock2Ptr->playerTrainerId[3] << 24);

        if (P_FLAG_FORCE_NO_SHINY != 0 && FlagGet(P_FLAG_FORCE_NO_SHINY))
        {
            isShiny = FALSE;
        }
        else if (P_FLAG_FORCE_SHINY != 0 && FlagGet(P_FLAG_FORCE_SHINY))
        {
            isShiny = TRUE;
        }
        else if (P_ONLY_OBTAINABLE_SHINIES && InBattlePyramid())
        {
            isShiny = FALSE;
        }
        else if (P_NO_SHINIES_WITHOUT_POKEBALLS && !HasAtLeastOnePokeBall())
        {
            isShiny = FALSE;
        }
        else
        {
            u32 totalRerolls = 0;
            if (CheckBagHasItem(ITEM_SHINY_CHARM, 1))
                totalRerolls += I_SHINY_CHARM_ADDITIONAL_ROLLS;
            if (LURE_STEP_COUNT != 0)
                totalRerolls += 1;
            if (I_FISHING_CHAIN && gIsFishingEncounter)
                totalRerolls += CalculateChainFishingShinyRolls();

            while (GET_SHINY_VALUE(value, personality) >= SHINY_ODDS && totalRerolls > 0)
            {
                personality = Random32();
                totalRerolls--;
            }

            isShiny = GET_SHINY_VALUE(value, personality) < SHINY_ODDS;
        }
    }

    SetBoxMonData(boxMon, MON_DATA_PERSONALITY, &personality);
    SetBoxMonData(boxMon, MON_DATA_OT_ID, &value);

    SetBoxMonData(boxMon, MON_DATA_IS_SHINY, &isShiny);
    StringCopy(speciesName, GetSpeciesName(species));
    SetBoxMonData(boxMon, MON_DATA_NICKNAME, speciesName);
    SetBoxMonData(boxMon, MON_DATA_OT_NAME, gSaveBlock2Ptr->playerName);
    SetBoxMonData(boxMon, MON_DATA_SPECIES, &species);
    SetBoxMonData(boxMon, MON_DATA_EXP, &gExperienceTables[gSpeciesInfo[species].growthRate][level]);
    SetBoxMonData(boxMon, MON_DATA_FRIENDSHIP, &gSpeciesInfo[species].friendship);
    value = GetCurrentRegionMapSectionId();
    SetBoxMonData(boxMon, MON_DATA_MET_LOCATION, &value);
    SetBoxMonData(boxMon, MON_DATA_MET_LEVEL, &level);
    value = ITEM_POKE_BALL;
    SetBoxMonData(boxMon, MON_DATA_POKEBALL, &value);

    if (fixedIV < USE_RANDOM_IVS)
    {
        SetBoxMonData(boxMon, MON_DATA_HP_IV, &fixedIV);
        SetBoxMonData(boxMon, MON_DATA_ATK_IV, &fixedIV);
        SetBoxMonData(boxMon, MON_DATA_DEF_IV, &fixedIV);
        SetBoxMonData(boxMon, MON_DATA_SPEED_IV, &fixedIV);
        SetBoxMonData(boxMon, MON_DATA_SPATK_IV, &fixedIV);
        SetBoxMonData(boxMon, MON_DATA_SPDEF_IV, &fixedIV);
    }
    else
    {
        u32 iv;
        u32 ivRandom = Random32();
        value = (u16)ivRandom;

        iv = value & MAX_IV_MASK;
        SetBoxMonData(boxMon, MON_DATA_HP_IV, &iv);
        iv = (value & (MAX_IV_MASK << 5)) >> 5;
        SetBoxMonData(boxMon, MON_DATA_ATK_IV, &iv);
        iv = (value & (MAX_IV_MASK << 10)) >> 10;
        SetBoxMonData(boxMon, MON_DATA_DEF_IV, &iv);

        value = (u16)(ivRandom >> 16);

        iv = value & MAX_IV_MASK;
        SetBoxMonData(boxMon, MON_DATA_SPEED_IV, &iv);
        iv = (value & (MAX_IV_MASK << 5)) >> 5;
        SetBoxMonData(boxMon, MON_DATA_SPATK_IV, &iv);
        iv = (value & (MAX_IV_MASK << 10)) >> 10;
        SetBoxMonData(boxMon, MON_DATA_SPDEF_IV, &iv);

        if (gSpeciesInfo[species].perfectIVCount != 0)
        {
            iv = MAX_PER_STAT_IVS;
            // Initialize a list of IV indices.
            for (i = 0; i < NUM_STATS; i++)
            {
                availableIVs[i] = i;
            }

            // Select the IVs that will be perfected.
            for (i = 0; i < NUM_STATS && i < gSpeciesInfo[species].perfectIVCount; i++)
            {
                u8 index = Random() % (NUM_STATS - i);
                selectedIvs[i] = availableIVs[index];
                RemoveIVIndexFromList(availableIVs, index);
            }
            for (i = 0; i < NUM_STATS && i < gSpeciesInfo[species].perfectIVCount; i++)
            {
                switch (selectedIvs[i])
                {
                case STAT_HP:
                    SetBoxMonData(boxMon, MON_DATA_HP_IV, &iv);
                    break;
                case STAT_ATK:
                    SetBoxMonData(boxMon, MON_DATA_ATK_IV, &iv);
                    break;
                case STAT_DEF:
                    SetBoxMonData(boxMon, MON_DATA_DEF_IV, &iv);
                    break;
                case STAT_SPEED:
                    SetBoxMonData(boxMon, MON_DATA_SPEED_IV, &iv);
                    break;
                case STAT_SPATK:
                    SetBoxMonData(boxMon, MON_DATA_SPATK_IV, &iv);
                    break;
                case STAT_SPDEF:
                    SetBoxMonData(boxMon, MON_DATA_SPDEF_IV, &iv);
                    break;
                }
            }
        }
    }

    if (gSpeciesInfo[species].abilities[1])
    {
        value = personality & 1;
        SetBoxMonData(boxMon, MON_DATA_ABILITY_NUM, &value);
    }

    GiveBoxMonInitialMoveset(boxMon);
}

void CreateMonWithNature(struct Pokemon *mon, u16 species, u8 level, u8 fixedIV, u8 nature)
{
    u32 personality;

    do
    {
        personality = Random32();
    }
    while (nature != GetNatureFromPersonality(personality));

    CreateMon(mon, species, level, fixedIV, TRUE, personality, OT_ID_PLAYER_ID, 0);
}

void CreateMonWithGenderNatureLetter(struct Pokemon *mon, u16 species, u8 level, u8 fixedIV, u8 gender, u8 nature)
{
    u32 personality;

    do
    {
        personality = Random32();
    }
    while (nature != GetNatureFromPersonality(personality)
        || gender != GetGenderFromSpeciesAndPersonality(species, personality));

    CreateMon(mon, species, level, fixedIV, TRUE, personality, OT_ID_PLAYER_ID, 0);
}

// This is only used to create Wally's Ralts.
void CreateMaleMon(struct Pokemon *mon, u16 species, u8 level)
{
    u32 personality;
    u32 otId;

    do
    {
        otId = Random32();
        personality = Random32();
    }
    while (GetGenderFromSpeciesAndPersonality(species, personality) != MON_MALE);
    CreateMon(mon, species, level, USE_RANDOM_IVS, TRUE, personality, OT_ID_PRESET, otId);
}

void CreateMonWithIVsPersonality(struct Pokemon *mon, u16 species, u8 level, u32 ivs, u32 personality)
{
    CreateMon(mon, species, level, 0, TRUE, personality, OT_ID_PLAYER_ID, 0);
    SetMonData(mon, MON_DATA_IVS, &ivs);
    CalculateMonStats(mon);
}

void CreateMonWithIVsOTID(struct Pokemon *mon, u16 species, u8 level, u8 *ivs, u32 otId)
{
    CreateMon(mon, species, level, 0, FALSE, 0, OT_ID_PRESET, otId);
    SetMonData(mon, MON_DATA_HP_IV, &ivs[STAT_HP]);
    SetMonData(mon, MON_DATA_ATK_IV, &ivs[STAT_ATK]);
    SetMonData(mon, MON_DATA_DEF_IV, &ivs[STAT_DEF]);
    SetMonData(mon, MON_DATA_SPEED_IV, &ivs[STAT_SPEED]);
    SetMonData(mon, MON_DATA_SPATK_IV, &ivs[STAT_SPATK]);
    SetMonData(mon, MON_DATA_SPDEF_IV, &ivs[STAT_SPDEF]);
    CalculateMonStats(mon);
}

void CreateMonWithEVSpread(struct Pokemon *mon, u16 species, u8 level, u8 fixedIV, u8 evSpread)
{
    s32 i;
    s32 statCount = 0;
    u16 evAmount;
    u8 evsBits;

    CreateMon(mon, species, level, fixedIV, FALSE, 0, OT_ID_PLAYER_ID, 0);

    evsBits = evSpread;

    for (i = 0; i < NUM_STATS; i++)
    {
        if (evsBits & 1)
            statCount++;
        evsBits >>= 1;
    }

    evAmount = MAX_TOTAL_EVS / statCount;

    evsBits = 1;

    for (i = 0; i < NUM_STATS; i++)
    {
        if (evSpread & evsBits)
            SetMonData(mon, MON_DATA_HP_EV + i, &evAmount);
        evsBits <<= 1;
    }

    CalculateMonStats(mon);
}

void CreateBattleTowerMon(struct Pokemon *mon, struct BattleTowerPokemon *src)
{
    s32 i;
    u8 nickname[max(32, POKEMON_NAME_BUFFER_SIZE)];
    u8 value;

    CreateMon(mon, src->species, src->level, 0, TRUE, src->personality, OT_ID_PRESET, src->otId);

    for (i = 0; i < MAX_MON_MOVES; i++)
        SetMonMoveSlot(mon, src->moves[i], i);

    SetMonData(mon, MON_DATA_PP_BONUSES, &src->ppBonuses);
    SetMonData(mon, MON_DATA_HELD_ITEM, &src->heldItem);
    SetMonData(mon, MON_DATA_FRIENDSHIP, &src->friendship);

    StringCopy(nickname, src->nickname);

    SetMonData(mon, MON_DATA_NICKNAME, nickname);
    SetMonData(mon, MON_DATA_HP_EV, &src->hpEV);
    SetMonData(mon, MON_DATA_ATK_EV, &src->attackEV);
    SetMonData(mon, MON_DATA_DEF_EV, &src->defenseEV);
    SetMonData(mon, MON_DATA_SPEED_EV, &src->speedEV);
    SetMonData(mon, MON_DATA_SPATK_EV, &src->spAttackEV);
    SetMonData(mon, MON_DATA_SPDEF_EV, &src->spDefenseEV);
    value = src->abilityNum;
    SetMonData(mon, MON_DATA_ABILITY_NUM, &value);
    value = src->hpIV;
    SetMonData(mon, MON_DATA_HP_IV, &value);
    value = src->attackIV;
    SetMonData(mon, MON_DATA_ATK_IV, &value);
    value = src->defenseIV;
    SetMonData(mon, MON_DATA_DEF_IV, &value);
    value = src->speedIV;
    SetMonData(mon, MON_DATA_SPEED_IV, &value);
    value = src->spAttackIV;
    SetMonData(mon, MON_DATA_SPATK_IV, &value);
    value = src->spDefenseIV;
    SetMonData(mon, MON_DATA_SPDEF_IV, &value);
    MonRestorePP(mon);
    CalculateMonStats(mon);
}

void CreateBattleTowerMon_HandleLevel(struct Pokemon *mon, struct BattleTowerPokemon *src, bool8 lvl50)
{
    s32 i;
    u8 nickname[max(32, POKEMON_NAME_BUFFER_SIZE)];
    u8 level;
    u8 value;

    if (gSaveBlock2Ptr->frontier.lvlMode != FRONTIER_LVL_50)
        level = GetFrontierEnemyMonLevel(gSaveBlock2Ptr->frontier.lvlMode);
    else if (lvl50)
        level = FRONTIER_MAX_LEVEL_50;
    else
        level = src->level;

    CreateMon(mon, src->species, level, 0, TRUE, src->personality, OT_ID_PRESET, src->otId);

    for (i = 0; i < MAX_MON_MOVES; i++)
        SetMonMoveSlot(mon, src->moves[i], i);

    SetMonData(mon, MON_DATA_PP_BONUSES, &src->ppBonuses);
    SetMonData(mon, MON_DATA_HELD_ITEM, &src->heldItem);
    SetMonData(mon, MON_DATA_FRIENDSHIP, &src->friendship);

    StringCopy(nickname, src->nickname);

    SetMonData(mon, MON_DATA_NICKNAME, nickname);
    SetMonData(mon, MON_DATA_HP_EV, &src->hpEV);
    SetMonData(mon, MON_DATA_ATK_EV, &src->attackEV);
    SetMonData(mon, MON_DATA_DEF_EV, &src->defenseEV);
    SetMonData(mon, MON_DATA_SPEED_EV, &src->speedEV);
    SetMonData(mon, MON_DATA_SPATK_EV, &src->spAttackEV);
    SetMonData(mon, MON_DATA_SPDEF_EV, &src->spDefenseEV);
    value = src->abilityNum;
    SetMonData(mon, MON_DATA_ABILITY_NUM, &value);
    value = src->hpIV;
    SetMonData(mon, MON_DATA_HP_IV, &value);
    value = src->attackIV;
    SetMonData(mon, MON_DATA_ATK_IV, &value);
    value = src->defenseIV;
    SetMonData(mon, MON_DATA_DEF_IV, &value);
    value = src->speedIV;
    SetMonData(mon, MON_DATA_SPEED_IV, &value);
    value = src->spAttackIV;
    SetMonData(mon, MON_DATA_SPATK_IV, &value);
    value = src->spDefenseIV;
    SetMonData(mon, MON_DATA_SPDEF_IV, &value);
    MonRestorePP(mon);
    CalculateMonStats(mon);
}

void CreateApprenticeMon(struct Pokemon *mon, const struct Apprentice *src, u8 monId)
{
    s32 i;
    u16 evAmount;
    u32 otId = 0;
    u32 personality = 0;

    CreateMon(mon,
              src->party[monId].species,
              GetFrontierEnemyMonLevel(src->lvlMode - 1),
              MAX_PER_STAT_IVS,
              TRUE,
              personality,
              OT_ID_PRESET,
              otId);

    SetMonData(mon, MON_DATA_HELD_ITEM, &src->party[monId].item);
    for (i = 0; i < MAX_MON_MOVES; i++)
        SetMonMoveSlot(mon, src->party[monId].moves[i], i);

    evAmount = MAX_TOTAL_EVS / NUM_STATS;
    for (i = 0; i < NUM_STATS; i++)
        SetMonData(mon, MON_DATA_HP_EV + i, &evAmount);

    CalculateMonStats(mon);
}

void CreateMonWithEVSpreadNatureOTID(struct Pokemon *mon, u16 species, u8 level, u8 nature, u8 fixedIV, u8 evSpread, u32 otId)
{
    s32 i;
    s32 statCount = 0;
    u8 evsBits;
    u16 evAmount;

    // i is reused as personality value
    do
    {
        i = Random32();
    } while (nature != GetNatureFromPersonality(i));

    CreateMon(mon, species, level, fixedIV, TRUE, i, OT_ID_PRESET, otId);
    evsBits = evSpread;
    for (i = 0; i < NUM_STATS; i++)
    {
        if (evsBits & 1)
            statCount++;
        evsBits >>= 1;
    }

    evAmount = MAX_TOTAL_EVS / statCount;
    evsBits = 1;
    for (i = 0; i < NUM_STATS; i++)
    {
        if (evSpread & evsBits)
            SetMonData(mon, MON_DATA_HP_EV + i, &evAmount);
        evsBits <<= 1;
    }

    CalculateMonStats(mon);
}

void ConvertPokemonToBattleTowerPokemon(struct Pokemon *mon, struct BattleTowerPokemon *dest)
{
    s32 i;
    u16 heldItem;

    dest->species = GetMonData(mon, MON_DATA_SPECIES, NULL);
    heldItem = GetMonData(mon, MON_DATA_HELD_ITEM, NULL);
    dest->heldItem = heldItem;

    for (i = 0; i < MAX_MON_MOVES; i++)
        dest->moves[i] = GetMonData(mon, MON_DATA_MOVE1 + i, NULL);

    dest->level = GetMonData(mon, MON_DATA_LEVEL, NULL);
    dest->ppBonuses = GetMonData(mon, MON_DATA_PP_BONUSES, NULL);
    dest->otId = GetMonData(mon, MON_DATA_OT_ID, NULL);
    dest->hpEV = GetMonData(mon, MON_DATA_HP_EV, NULL);
    dest->attackEV = GetMonData(mon, MON_DATA_ATK_EV, NULL);
    dest->defenseEV = GetMonData(mon, MON_DATA_DEF_EV, NULL);
    dest->speedEV = GetMonData(mon, MON_DATA_SPEED_EV, NULL);
    dest->spAttackEV = GetMonData(mon, MON_DATA_SPATK_EV, NULL);
    dest->spDefenseEV = GetMonData(mon, MON_DATA_SPDEF_EV, NULL);
    dest->friendship = GetMonData(mon, MON_DATA_FRIENDSHIP, NULL);
    dest->hpIV = GetMonData(mon, MON_DATA_HP_IV, NULL);
    dest->attackIV = GetMonData(mon, MON_DATA_ATK_IV, NULL);
    dest->defenseIV = GetMonData(mon, MON_DATA_DEF_IV, NULL);
    dest->speedIV  = GetMonData(mon, MON_DATA_SPEED_IV, NULL);
    dest->spAttackIV  = GetMonData(mon, MON_DATA_SPATK_IV, NULL);
    dest->spDefenseIV  = GetMonData(mon, MON_DATA_SPDEF_IV, NULL);
    dest->abilityNum = GetMonData(mon, MON_DATA_ABILITY_NUM, NULL);
    dest->personality = GetMonData(mon, MON_DATA_PERSONALITY, NULL);
    GetMonData(mon, MON_DATA_NICKNAME10, dest->nickname);
}

static void CreateEventMon(struct Pokemon *mon, u16 species, u8 level, u8 fixedIV, u8 hasFixedPersonality, u32 fixedPersonality, u8 otIdType, u32 fixedOtId)
{
    bool32 isModernFatefulEncounter = TRUE;

    CreateMon(mon, species, level, fixedIV, hasFixedPersonality, fixedPersonality, otIdType, fixedOtId);
    SetMonData(mon, MON_DATA_MODERN_FATEFUL_ENCOUNTER, &isModernFatefulEncounter);
}

// If FALSE, should load this game's Deoxys form. If TRUE, should load normal Deoxys form
bool8 ShouldIgnoreDeoxysForm(u8 caseId, u8 battlerId)
{
    switch (caseId)
    {
    case 0:
    default:
        return FALSE;
    case 1: // Player's side in battle
        if (!(gBattleTypeFlags & BATTLE_TYPE_MULTI))
            return FALSE;
        if (!gMain.inBattle)
            return FALSE;
        if (gLinkPlayers[GetMultiplayerId()].id == battlerId)
            return FALSE;
        break;
    case 2:
        break;
    case 3: // Summary Screen
        if (!(gBattleTypeFlags & BATTLE_TYPE_MULTI))
            return FALSE;
        if (!gMain.inBattle)
            return FALSE;
        if (battlerId == 1 || battlerId == 4 || battlerId == 5)
            return TRUE;
        return FALSE;
    case 4:
        break;
    case 5: // In move animation, e.g. in Role Play or Snatch
        if (gBattleTypeFlags & BATTLE_TYPE_LINK)
        {
            if (!gMain.inBattle)
                return FALSE;
            if (gBattleTypeFlags & BATTLE_TYPE_MULTI)
            {
                if (gLinkPlayers[GetMultiplayerId()].id == battlerId)
                    return FALSE;
            }
            else
            {
                if (GetBattlerSide(battlerId) == B_SIDE_PLAYER)
                    return FALSE;
            }
        }
        else
        {
            if (!gMain.inBattle)
                return FALSE;
            if (GetBattlerSide(battlerId) == B_SIDE_PLAYER)
                return FALSE;
        }
        break;
    }

    return TRUE;
}

void CreateEnemyEventMon(void)
{
    s32 species = gSpecialVar_0x8004;
    s32 level = gSpecialVar_0x8005;
    s32 itemId = gSpecialVar_0x8006;

    ZeroEnemyPartyMons();
    CreateEventMon(&gEnemyParty[0], species, level, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);
    if (itemId)
    {
        u8 heldItem[2];
        heldItem[0] = itemId;
        heldItem[1] = itemId >> 8;
        SetMonData(&gEnemyParty[0], MON_DATA_HELD_ITEM, heldItem);
    }
}

#define CALC_STAT(base, iv, ev, statIndex, field)               \
{                                                               \
    u8 baseStat = gSpeciesInfo[species].base;                   \
    s32 n = (((2 * baseStat + iv + ev / 4) * level) / 100) + 5; \
    n = ModifyStatByNature(nature, n, statIndex);               \
    if (B_FRIENDSHIP_BOOST == TRUE)                             \
        n = n + ((n * 10 * friendship) / (MAX_FRIENDSHIP * 100));\
    SetMonData(mon, field, &n);                                 \
}

void CalculateMonStats(struct Pokemon *mon)
{
    s32 oldMaxHP = GetMonData(mon, MON_DATA_MAX_HP, NULL);
    s32 currentHP = GetMonData(mon, MON_DATA_HP, NULL);
    s32 hpIV = GetMonData(mon, MON_DATA_HYPER_TRAINED_HP) ? MAX_PER_STAT_IVS : GetMonData(mon, MON_DATA_HP_IV, NULL);
    s32 hpEV = GetMonData(mon, MON_DATA_HP_EV, NULL);
    s32 attackIV = GetMonData(mon, MON_DATA_HYPER_TRAINED_ATK) ? MAX_PER_STAT_IVS : GetMonData(mon, MON_DATA_ATK_IV, NULL);
    s32 attackEV = GetMonData(mon, MON_DATA_ATK_EV, NULL);
    s32 defenseIV = GetMonData(mon, MON_DATA_HYPER_TRAINED_DEF) ? MAX_PER_STAT_IVS : GetMonData(mon, MON_DATA_DEF_IV, NULL);
    s32 defenseEV = GetMonData(mon, MON_DATA_DEF_EV, NULL);
    s32 speedIV = GetMonData(mon, MON_DATA_HYPER_TRAINED_SPEED) ? MAX_PER_STAT_IVS : GetMonData(mon, MON_DATA_SPEED_IV, NULL);
    s32 speedEV = GetMonData(mon, MON_DATA_SPEED_EV, NULL);
    s32 spAttackIV = GetMonData(mon, MON_DATA_HYPER_TRAINED_SPATK) ? MAX_PER_STAT_IVS : GetMonData(mon, MON_DATA_SPATK_IV, NULL);
    s32 spAttackEV = GetMonData(mon, MON_DATA_SPATK_EV, NULL);
    s32 spDefenseIV = GetMonData(mon, MON_DATA_HYPER_TRAINED_SPDEF) ? MAX_PER_STAT_IVS : GetMonData(mon, MON_DATA_SPDEF_IV, NULL);
    s32 spDefenseEV = GetMonData(mon, MON_DATA_SPDEF_EV, NULL);
    u16 species = GetMonData(mon, MON_DATA_SPECIES, NULL);
    u8 friendship = GetMonData(mon, MON_DATA_FRIENDSHIP, NULL);
    s32 level = GetLevelFromMonExp(mon);
    s32 newMaxHP;

    u8 nature = GetMonData(mon, MON_DATA_HIDDEN_NATURE, NULL);

    SetMonData(mon, MON_DATA_LEVEL, &level);

    if (species == SPECIES_SHEDINJA)
    {
        newMaxHP = 1;
    }
    else
    {
        s32 n = 2 * gSpeciesInfo[species].baseHP + hpIV;
        newMaxHP = (((n + hpEV / 4) * level) / 100) + level + 10;
    }

    gBattleScripting.levelUpHP = newMaxHP - oldMaxHP;
    if (gBattleScripting.levelUpHP == 0)
        gBattleScripting.levelUpHP = 1;

    SetMonData(mon, MON_DATA_MAX_HP, &newMaxHP);

    CALC_STAT(baseAttack, attackIV, attackEV, STAT_ATK, MON_DATA_ATK)
    CALC_STAT(baseDefense, defenseIV, defenseEV, STAT_DEF, MON_DATA_DEF)
    CALC_STAT(baseSpeed, speedIV, speedEV, STAT_SPEED, MON_DATA_SPEED)
    CALC_STAT(baseSpAttack, spAttackIV, spAttackEV, STAT_SPATK, MON_DATA_SPATK)
    CALC_STAT(baseSpDefense, spDefenseIV, spDefenseEV, STAT_SPDEF, MON_DATA_SPDEF)

    // Since a pokemon's maxHP data could either not have
    // been initialized at this point or this pokemon is
    // just fainted, the check for oldMaxHP is important.
    if (currentHP == 0 && oldMaxHP != 0)
        return;

    // Only add to currentHP if newMaxHP went up.
    if (newMaxHP > oldMaxHP)
        currentHP += newMaxHP - oldMaxHP;

    // Ensure currentHP does not surpass newMaxHP.
    if (currentHP > newMaxHP)
        currentHP = newMaxHP;

    SetMonData(mon, MON_DATA_HP, &currentHP);
}

void BoxMonToMon(const struct BoxPokemon *src, struct Pokemon *dest)
{
    u32 value = 0;
    dest->box = *src;
    dest->status = GetBoxMonData(&dest->box, MON_DATA_STATUS, NULL);
    dest->hp = 0;
    dest->maxHP = 0;
    CalculateMonStats(dest);
    value = GetMonData(dest, MON_DATA_MAX_HP);
    SetMonData(dest, MON_DATA_HP, &value);
}

u8 GetLevelFromMonExp(struct Pokemon *mon)
{
    u16 species = GetMonData(mon, MON_DATA_SPECIES, NULL);
    u32 exp = GetMonData(mon, MON_DATA_EXP, NULL);
    s32 level = 1;

    while (level <= MAX_LEVEL && gExperienceTables[gSpeciesInfo[species].growthRate][level] <= exp)
        level++;

    return level - 1;
}

u8 GetLevelFromBoxMonExp(struct BoxPokemon *boxMon)
{
    u16 species = GetBoxMonData(boxMon, MON_DATA_SPECIES, NULL);
    u32 exp = GetBoxMonData(boxMon, MON_DATA_EXP, NULL);
    s32 level = 1;

    while (level <= MAX_LEVEL && gExperienceTables[gSpeciesInfo[species].growthRate][level] <= exp)
        level++;

    return level - 1;
}

u16 GiveMoveToMon(struct Pokemon *mon, u16 move)
{
    return GiveMoveToBoxMon(&mon->box, move);
}

u16 GiveMoveToBoxMon(struct BoxPokemon *boxMon, u16 move)
{
    s32 i;
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        u16 existingMove = GetBoxMonData(boxMon, MON_DATA_MOVE1 + i, NULL);
        if (existingMove == MOVE_NONE)
        {
            SetBoxMonData(boxMon, MON_DATA_MOVE1 + i, &move);
            SetBoxMonData(boxMon, MON_DATA_PP1 + i, &gMovesInfo[move].pp);
            return move;
        }
        if (existingMove == move)
            return MON_ALREADY_KNOWS_MOVE;
    }
    return MON_HAS_MAX_MOVES;
}

u16 GiveMoveToBattleMon(struct BattlePokemon *mon, u16 move)
{
    s32 i;

    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        if (mon->moves[i] == MOVE_NONE)
        {
            mon->moves[i] = move;
            mon->pp[i] = gMovesInfo[move].pp;
            return move;
        }
    }

    return MON_HAS_MAX_MOVES;
}

void SetMonMoveSlot(struct Pokemon *mon, u16 move, u8 slot)
{
    SetMonData(mon, MON_DATA_MOVE1 + slot, &move);
    SetMonData(mon, MON_DATA_PP1 + slot, &gMovesInfo[move].pp);
}

static void SetMonMoveSlot_KeepPP(struct Pokemon *mon, u16 move, u8 slot)
{
    u8 ppBonuses = GetMonData(mon, MON_DATA_PP_BONUSES, NULL);
    u8 currPP = GetMonData(mon, MON_DATA_PP1 + slot, NULL);
    u8 newPP = CalculatePPWithBonus(move, ppBonuses, slot);
    u16 finalPP = min(currPP, newPP);

    SetMonData(mon, MON_DATA_MOVE1 + slot, &move);
    SetMonData(mon, MON_DATA_PP1 + slot, &finalPP);
}

void SetBattleMonMoveSlot(struct BattlePokemon *mon, u16 move, u8 slot)
{
    mon->moves[slot] = move;
    mon->pp[slot] = gMovesInfo[move].pp;
}

void GiveMonInitialMoveset(struct Pokemon *mon)
{
    GiveBoxMonInitialMoveset(&mon->box);
}

void GiveBoxMonInitialMoveset(struct BoxPokemon *boxMon) //Credit: AsparagusEduardo
{
    u16 species = GetBoxMonData(boxMon, MON_DATA_SPECIES, NULL);
    s32 level = GetLevelFromBoxMonExp(boxMon);
    s32 i;
    u16 moves[MAX_MON_MOVES] = {MOVE_NONE};
    u8 addedMoves = 0;
    const struct LevelUpMove *learnset = GetSpeciesLevelUpLearnset(species);

    for (i = 0; learnset[i].move != LEVEL_UP_MOVE_END; i++)
    {
        s32 j;
        bool32 alreadyKnown = FALSE;

        if (learnset[i].level > level)
            break;
        if (learnset[i].level == 0)
            continue;

        for (j = 0; j < addedMoves; j++)
        {
            if (moves[j] == learnset[i].move)
            {
                alreadyKnown = TRUE;
                break;
            }
        }

        if (!alreadyKnown)
        {
            if (addedMoves < MAX_MON_MOVES)
            {
                moves[addedMoves] = learnset[i].move;
                addedMoves++;
            }
            else
            {
                for (j = 0; j < MAX_MON_MOVES - 1; j++)
                    moves[j] = moves[j + 1];
                moves[MAX_MON_MOVES - 1] = learnset[i].move;
            }
        }
    }
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        SetBoxMonData(boxMon, MON_DATA_MOVE1 + i, &moves[i]);
        SetBoxMonData(boxMon, MON_DATA_PP1 + i, &gMovesInfo[moves[i]].pp);
    }
}

u16 MonTryLearningNewMove(struct Pokemon *mon, bool8 firstMove)
{
    u32 retVal = MOVE_NONE;
    u16 species = GetMonData(mon, MON_DATA_SPECIES, NULL);
    u8 level = GetMonData(mon, MON_DATA_LEVEL, NULL);
    const struct LevelUpMove *learnset = GetSpeciesLevelUpLearnset(species);

    // since you can learn more than one move per level
    // the game needs to know whether you decided to
    // learn it or keep the old set to avoid asking
    // you to learn the same move over and over again
    if (firstMove)
    {
        sLearningMoveTableID = 0;

        while (learnset[sLearningMoveTableID].level != level)
        {
            sLearningMoveTableID++;
            if (learnset[sLearningMoveTableID].move == LEVEL_UP_MOVE_END)
                return MOVE_NONE;
        }
    }

    if (learnset[sLearningMoveTableID].level == level)
    {
        gMoveToLearn = learnset[sLearningMoveTableID].move;
        sLearningMoveTableID++;
        retVal = GiveMoveToMon(mon, gMoveToLearn);
    }

    return retVal;
}

void DeleteFirstMoveAndGiveMoveToMon(struct Pokemon *mon, u16 move)
{
    s32 i;
    u16 moves[MAX_MON_MOVES];
    u8 pp[MAX_MON_MOVES];
    u8 ppBonuses;

    for (i = 0; i < MAX_MON_MOVES - 1; i++)
    {
        moves[i] = GetMonData(mon, MON_DATA_MOVE2 + i, NULL);
        pp[i] = GetMonData(mon, MON_DATA_PP2 + i, NULL);
    }

    ppBonuses = GetMonData(mon, MON_DATA_PP_BONUSES, NULL);
    ppBonuses >>= 2;
    moves[MAX_MON_MOVES - 1] = move;
    pp[MAX_MON_MOVES - 1] = gMovesInfo[move].pp;

    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        SetMonData(mon, MON_DATA_MOVE1 + i, &moves[i]);
        SetMonData(mon, MON_DATA_PP1 + i, &pp[i]);
    }

    SetMonData(mon, MON_DATA_PP_BONUSES, &ppBonuses);
}

void DeleteFirstMoveAndGiveMoveToBoxMon(struct BoxPokemon *boxMon, u16 move)
{
    s32 i;
    u16 moves[MAX_MON_MOVES];
    u8 pp[MAX_MON_MOVES];
    u8 ppBonuses;

    for (i = 0; i < MAX_MON_MOVES - 1; i++)
    {
        moves[i] = GetBoxMonData(boxMon, MON_DATA_MOVE2 + i, NULL);
        pp[i] = GetBoxMonData(boxMon, MON_DATA_PP2 + i, NULL);
    }

    ppBonuses = GetBoxMonData(boxMon, MON_DATA_PP_BONUSES, NULL);
    ppBonuses >>= 2;
    moves[MAX_MON_MOVES - 1] = move;
    pp[MAX_MON_MOVES - 1] = gMovesInfo[move].pp;

    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        SetBoxMonData(boxMon, MON_DATA_MOVE1 + i, &moves[i]);
        SetBoxMonData(boxMon, MON_DATA_PP1 + i, &pp[i]);
    }

    SetBoxMonData(boxMon, MON_DATA_PP_BONUSES, &ppBonuses);
}

u8 CountAliveMonsInBattle(u8 caseId, u32 battler)
{
    s32 i;
    u8 retVal = 0;

    switch (caseId)
    {
    case BATTLE_ALIVE_EXCEPT_BATTLER:
        for (i = 0; i < MAX_BATTLERS_COUNT; i++)
        {
            if (i != battler && !(gAbsentBattlerFlags & (1u << i)))
                retVal++;
        }
        break;
    case BATTLE_ALIVE_SIDE:
        for (i = 0; i < MAX_BATTLERS_COUNT; i++)
        {
            if (GetBattlerSide(i) == GetBattlerSide(battler) && !(gAbsentBattlerFlags & (1u << i)))
                retVal++;
        }
        break;
    }

    return retVal;
}

u8 GetDefaultMoveTarget(u8 battlerId)
{
    u8 opposing = BATTLE_OPPOSITE(GetBattlerSide(battlerId));

    if (!IsDoubleBattle())
        return GetBattlerAtPosition(opposing);
    if (CountAliveMonsInBattle(BATTLE_ALIVE_EXCEPT_BATTLER, battlerId) > 1)
    {
        u8 position;

        if ((Random() & 1) == 0)
            position = BATTLE_PARTNER(opposing);
        else
            position = opposing;

        return GetBattlerAtPosition(position);
    }
    else
    {
        if ((gAbsentBattlerFlags & (1u << opposing)))
            return GetBattlerAtPosition(BATTLE_PARTNER(opposing));
        else
            return GetBattlerAtPosition(opposing);
    }
}

u8 GetMonGender(struct Pokemon *mon)
{
    return GetBoxMonGender(&mon->box);
}

u8 GetBoxMonGender(struct BoxPokemon *boxMon)
{
    u16 species = GetBoxMonData(boxMon, MON_DATA_SPECIES, NULL);
    u32 personality = GetBoxMonData(boxMon, MON_DATA_PERSONALITY, NULL);

    switch (gSpeciesInfo[species].genderRatio)
    {
    case MON_MALE:
    case MON_FEMALE:
    case MON_GENDERLESS:
        return gSpeciesInfo[species].genderRatio;
    }

    if (gSpeciesInfo[species].genderRatio > (personality & 255))
        return MON_FEMALE;
    else
        return MON_MALE;
}

u8 GetGenderFromSpeciesAndPersonality(u16 species, u32 personality)
{
    switch (gSpeciesInfo[species].genderRatio)
    {
    case MON_MALE:
    case MON_FEMALE:
    case MON_GENDERLESS:
        return gSpeciesInfo[species].genderRatio;
    }

    if (gSpeciesInfo[species].genderRatio > (personality & 255))
        return MON_FEMALE;
    else
        return MON_MALE;
}

bool32 IsPersonalityFemale(u16 species, u32 personality)
{
    return GetGenderFromSpeciesAndPersonality(species, personality) == MON_FEMALE;
}

void SetMultiuseSpriteTemplateToPokemon(u16 speciesTag, u8 battlerPosition)
{
    if (gMonSpritesGfxPtr != NULL)
        gMultiuseSpriteTemplate = gMonSpritesGfxPtr->templates[battlerPosition];
    else if (sMonSpritesGfxManager != NULL)
        gMultiuseSpriteTemplate = sMonSpritesGfxManager->templates[battlerPosition];
    else
        gMultiuseSpriteTemplate = gBattlerSpriteTemplates[battlerPosition];

    gMultiuseSpriteTemplate.paletteTag = speciesTag;
    if (battlerPosition == B_POSITION_PLAYER_LEFT || battlerPosition == B_POSITION_PLAYER_RIGHT)
        gMultiuseSpriteTemplate.anims = gAnims_MonPic;
    else
    {
        if (speciesTag > SPECIES_SHINY_TAG)
            speciesTag = speciesTag - SPECIES_SHINY_TAG;

        speciesTag = SanitizeSpeciesId(speciesTag);
        if (gSpeciesInfo[speciesTag].frontAnimFrames != NULL)
            gMultiuseSpriteTemplate.anims = gSpeciesInfo[speciesTag].frontAnimFrames;
        else
            gMultiuseSpriteTemplate.anims = gSpeciesInfo[SPECIES_NONE].frontAnimFrames;
    }
}

void SetMultiuseSpriteTemplateToTrainer(u16 trainerPicId, u8 battlerPosition)
{
    gMultiuseSpriteTemplate.paletteTag = trainerPicId;
    if (battlerPosition == B_POSITION_PLAYER_LEFT || battlerPosition == B_POSITION_PLAYER_RIGHT)
    {
        gMultiuseSpriteTemplate = sTrainerBackSpriteTemplates[trainerPicId];
        gMultiuseSpriteTemplate.anims = gTrainerBacksprites[trainerPicId].animation;
    }
    else
    {
        if (gMonSpritesGfxPtr != NULL)
        {
            gMultiuseSpriteTemplate = gMonSpritesGfxPtr->templates[battlerPosition];
        }
        else
        {
            gMultiuseSpriteTemplate = gBattlerSpriteTemplates[battlerPosition];
        }
        gMultiuseSpriteTemplate.anims = gAnims_Trainer;
    }
}

/* GameFreak called GetMonData with either 2 or 3 arguments, for type
 * safety we have a GetMonData macro (in include/pokemon.h) which
 * dispatches to either GetMonData2 or GetMonData3 based on the number
 * of arguments. */
u32 GetMonData3(struct Pokemon *mon, s32 field, u8 *data)
{
    u32 ret;

    switch (field)
    {
    case MON_DATA_STATUS:
        ret = mon->status;
        break;
    case MON_DATA_LEVEL:
        ret = mon->level;
        break;
    case MON_DATA_HP:
        ret = mon->hp;
        break;
    case MON_DATA_MAX_HP:
        ret = mon->maxHP;
        break;
    case MON_DATA_ATK:
        ret = mon->attack;
        break;
    case MON_DATA_DEF:
        ret = mon->defense;
        break;
    case MON_DATA_SPEED:
        ret = mon->speed;
        break;
    case MON_DATA_SPATK:
        ret = mon->spAttack;
        break;
    case MON_DATA_SPDEF:
        ret = mon->spDefense;
        break;
    case MON_DATA_MAIL:
        ret = 0;
        break;
    default:
        ret = GetBoxMonData(&mon->box, field, data);
        break;
    }
    return ret;
}

u32 GetMonData2(struct Pokemon *mon, s32 field)
{
    return GetMonData3(mon, field, NULL);
}

struct EvolutionTrackerBitfield
{
    u16 a: 5;
    u16 b: 5;
    u16 unused: 6;
};

union EvolutionTracker
{
    u16 value;
    struct EvolutionTrackerBitfield asField;
};

/* GameFreak called GetBoxMonData with either 2 or 3 arguments, for type
 * safety we have a GetBoxMonData macro (in include/pokemon.h) which
 * dispatches to either GetBoxMonData2 or GetBoxMonData3 based on the
 * number of arguments. */
u32 GetBoxMonData3(struct BoxPokemon *boxMon, s32 field, u8 *data)
{
    u32 retVal = 0;

        switch (field)
    {
        case MON_DATA_NICKNAME:
        {
            if (boxMon->isEgg)
            {
                StringCopy(data, gText_EggNickname);
                retVal = StringLength(data);
            }
            else
            {
                for (retVal = 0;
                    retVal < POKEMON_NAME_LENGTH;
                    data[retVal] = boxMon->nickname[retVal], retVal++){}

                data[retVal] = EOS;
            }
            break;
        }
        case MON_DATA_SPECIES:
            retVal = boxMon->species;
            break;
        case MON_DATA_HELD_ITEM:
            retVal = boxMon->heldItem;
            break;
        case MON_DATA_EXP:
            retVal = boxMon->experience;
            break;
        case MON_DATA_PP_BONUSES:
            retVal = boxMon->ppBonuses;
            break;
        case MON_DATA_FRIENDSHIP:
            retVal = boxMon->friendship;
            break;
        case MON_DATA_MOVE1:
            retVal = boxMon->move1;
            break;
        case MON_DATA_MOVE2:
            retVal = boxMon->move2;
            break;
        case MON_DATA_MOVE3:
            retVal = boxMon->move3;
            break;
        case MON_DATA_MOVE4:
            retVal = boxMon->move4;
            break;
        case MON_DATA_PP1:
            retVal = boxMon->pp1;
            break;
        case MON_DATA_PP2:
            retVal = boxMon->pp2;
            break;
        case MON_DATA_PP3:
            retVal = boxMon->pp3;
            break;
        case MON_DATA_PP4:
            retVal = boxMon->pp4;
            break;
        case MON_DATA_HP_EV:
            retVal = boxMon->hpEV;
            break;
        case MON_DATA_ATK_EV:
            retVal = boxMon->attackEV;
            break;
        case MON_DATA_DEF_EV:
            retVal = boxMon->defenseEV;
            break;
        case MON_DATA_SPEED_EV:
            retVal = boxMon->speedEV;
            break;
        case MON_DATA_SPATK_EV:
            retVal = boxMon->spAttackEV;
            break;
        case MON_DATA_SPDEF_EV:
            retVal = boxMon->spDefenseEV;
            break;
        case MON_DATA_COOL:
        case MON_DATA_BEAUTY:
        case MON_DATA_CUTE:
        case MON_DATA_SMART:
        case MON_DATA_TOUGH:
        case MON_DATA_SHEEN:
        case MON_DATA_POKERUS:
            retVal = 0;
            break;
        case MON_DATA_MET_LOCATION:
            retVal = boxMon->metLocation;
            break;
        case MON_DATA_MET_LEVEL:
            retVal = boxMon->metLevel;
            break;
        case MON_DATA_MET_GAME:
            retVal = 0;
            break;
        case MON_DATA_POKEBALL:
            retVal = boxMon->pokeball;
            break;
        case MON_DATA_OT_GENDER:
            retVal = 0;
            break;
        case MON_DATA_HP_IV:
            retVal = boxMon->hpIV;
            break;
        case MON_DATA_ATK_IV:
            retVal = boxMon->attackIV;
            break;
        case MON_DATA_DEF_IV:
            retVal = boxMon->defenseIV;
            break;
        case MON_DATA_SPEED_IV:
            retVal = boxMon->speedIV;
            break;
        case MON_DATA_SPATK_IV:
            retVal = boxMon->spAttackIV;
            break;
        case MON_DATA_SPDEF_IV:
            retVal = boxMon->spDefenseIV;
            break;
        case MON_DATA_IS_EGG:
            retVal = boxMon->isEgg;
            break;
        case MON_DATA_ABILITY_NUM:
            retVal = boxMon->abilityNum;
            break;
        case MON_DATA_COOL_RIBBON:
        case MON_DATA_BEAUTY_RIBBON:
        case MON_DATA_CUTE_RIBBON:
        case MON_DATA_SMART_RIBBON:
        case MON_DATA_TOUGH_RIBBON:
        case MON_DATA_CHAMPION_RIBBON:
        case MON_DATA_WINNING_RIBBON:
        case MON_DATA_VICTORY_RIBBON:
        case MON_DATA_ARTIST_RIBBON:
        case MON_DATA_EFFORT_RIBBON:
        case MON_DATA_MARINE_RIBBON:
        case MON_DATA_LAND_RIBBON:
        case MON_DATA_SKY_RIBBON:
        case MON_DATA_COUNTRY_RIBBON:
        case MON_DATA_NATIONAL_RIBBON:
        case MON_DATA_EARTH_RIBBON:
        case MON_DATA_WORLD_RIBBON:
        case MON_DATA_MODERN_FATEFUL_ENCOUNTER:
            retVal = 0;
            break;
        case MON_DATA_SPECIES_OR_EGG:
            retVal = boxMon->species;
            if (boxMon->species && boxMon->isEgg)
                retVal = SPECIES_EGG;
            break;
        case MON_DATA_IVS:
            retVal = boxMon->hpIV
                    | (boxMon->attackIV << 5)
                    | (boxMon->defenseIV << 10)
                    | (boxMon->speedIV << 15)
                    | (boxMon->spAttackIV << 20)
                    | (boxMon->spDefenseIV << 25);
            break;
        case MON_DATA_KNOWN_MOVES:
            if (boxMon->species && !boxMon->isEgg)
            {
                u16 *moves = (u16 *)data;
                s32 i = 0;

                while (moves[i] != MOVES_COUNT)
                {
                    u16 move = moves[i];
                    if (boxMon->move1 == move
                        || boxMon->move2 == move
                        || boxMon->move3 == move
                        || boxMon->move4 == move)
                        retVal |= (1u << i);
                    i++;
                }
            }
            break;
        case MON_DATA_RIBBON_COUNT:
        case MON_DATA_RIBBONS:
        case MON_DATA_HYPER_TRAINED_HP:
        case MON_DATA_HYPER_TRAINED_ATK:
        case MON_DATA_HYPER_TRAINED_DEF:
        case MON_DATA_HYPER_TRAINED_SPEED:
        case MON_DATA_HYPER_TRAINED_SPATK:
        case MON_DATA_HYPER_TRAINED_SPDEF:
        case MON_DATA_IS_SHADOW:
        case MON_DATA_DYNAMAX_LEVEL:
        case MON_DATA_GIGANTAMAX_FACTOR:
        case MON_DATA_TERA_TYPE:
        case MON_DATA_EVOLUTION_TRACKER:
            retVal = 0;
            break;
        case MON_DATA_STATUS:
            retVal = 0;
            break;
        case MON_DATA_PERSONALITY:
            retVal = boxMon->personality;
            break;
        case MON_DATA_OT_ID:
            retVal = boxMon->otId;
            break;
        case MON_DATA_LANGUAGE:
            retVal = 0;
            break;
        case MON_DATA_SANITY_IS_BAD_EGG:
            retVal = 0;
            break;
        case MON_DATA_SANITY_HAS_SPECIES:
            retVal = 0;
            break;
        case MON_DATA_SANITY_IS_EGG:
            retVal = boxMon->isEgg;
            break;
        case MON_DATA_OT_NAME:
        {
            retVal = 0;

            while (retVal < PLAYER_NAME_LENGTH)
            {
                data[retVal] = boxMon->otName[retVal];
                retVal++;
            }

            data[retVal] = EOS;
            break;
        }
        case MON_DATA_MARKINGS:
            retVal = 0;
            break;
        case MON_DATA_IS_SHINY:
        {
            u32 shinyValue = GET_SHINY_VALUE(boxMon->otId, boxMon->personality);
            retVal = (shinyValue < SHINY_ODDS) ^ boxMon->shinyModifier;
            break;
        }
        case MON_DATA_HIDDEN_NATURE:
        {
            u32 nature = GetNatureFromPersonality(boxMon->personality);
            retVal = nature;
            break;
        }
        default:
            break;
    }

    return retVal;
}

u32 GetBoxMonData2(struct BoxPokemon *boxMon, s32 field)
{
    return GetBoxMonData3(boxMon, field, NULL);
}

#define SET8(lhs) (lhs) = *data
#define SET16(lhs) (lhs) = data[0] + (data[1] << 8)
#define SET32(lhs) (lhs) = data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24)

void SetMonData(struct Pokemon *mon, s32 field, const void *dataArg)
{
    const u8 *data = dataArg;

    switch (field)
    {
    case MON_DATA_STATUS:
        SET32(mon->status);
        break;
    case MON_DATA_LEVEL:
        SET8(mon->level);
        break;
    case MON_DATA_HP:
        SET16(mon->hp);
        break;
    case MON_DATA_MAX_HP:
        SET16(mon->maxHP);
        break;
    case MON_DATA_ATK:
        SET16(mon->attack);
        break;
    case MON_DATA_DEF:
        SET16(mon->defense);
        break;
    case MON_DATA_SPEED:
        SET16(mon->speed);
        break;
    case MON_DATA_SPATK:
        SET16(mon->spAttack);
        break;
    case MON_DATA_SPDEF:
        SET16(mon->spDefense);
        break;
    case MON_DATA_MAIL:
        break;
    case MON_DATA_SPECIES_OR_EGG:
        break;
    default:
        SetBoxMonData(&mon->box, field, data);
        break;
    }
}

void SetBoxMonData(struct BoxPokemon *boxMon, s32 field, const void *dataArg)
{
    const u8 *data = dataArg;

        switch (field)
    {
        case MON_DATA_NICKNAME:
        {
            s32 i;
            for (i = 0; i < POKEMON_NAME_LENGTH; i++)
                boxMon->nickname[i] = data[i];
            break;
        }
        case MON_DATA_SPECIES:
            SET16(boxMon->species);
            break;
        case MON_DATA_HELD_ITEM:
            SET16(boxMon->heldItem);
            break;
        case MON_DATA_EXP:
            SET32(boxMon->experience);
            break;
        case MON_DATA_PP_BONUSES:
            SET8(boxMon->ppBonuses);
            break;
        case MON_DATA_FRIENDSHIP:
            SET8(boxMon->friendship);
            break;
        case MON_DATA_MOVE1:
            SET16(boxMon->move1);
            break;
        case MON_DATA_MOVE2:
            SET16(boxMon->move2);
            break;
        case MON_DATA_MOVE3:
            SET16(boxMon->move3);
            break;
        case MON_DATA_MOVE4:
            SET16(boxMon->move4);
            break;
        case MON_DATA_PP1:
            SET8(boxMon->pp1);
            break;
        case MON_DATA_PP2:
            SET8(boxMon->pp2);
            break;
        case MON_DATA_PP3:
            SET8(boxMon->pp3);
            break;
        case MON_DATA_PP4:
            SET8(boxMon->pp4);
            break;
        case MON_DATA_HP_EV:
            SET8(boxMon->hpEV);
            break;
        case MON_DATA_ATK_EV:
            SET8(boxMon->attackEV);
            break;
        case MON_DATA_DEF_EV:
            SET8(boxMon->defenseEV);
            break;
        case MON_DATA_SPEED_EV:
            SET8(boxMon->speedEV);
            break;
        case MON_DATA_SPATK_EV:
            SET8(boxMon->spAttackEV);
            break;
        case MON_DATA_SPDEF_EV:
            SET8(boxMon->spDefenseEV);
            break;
        case MON_DATA_COOL:
        case MON_DATA_BEAUTY:
        case MON_DATA_CUTE:
        case MON_DATA_SMART:
        case MON_DATA_TOUGH:
        case MON_DATA_SHEEN:
        case MON_DATA_POKERUS:
            break;
        case MON_DATA_MET_LOCATION:
            SET8(boxMon->metLocation);
            break;
        case MON_DATA_MET_LEVEL:
        {
            u8 metLevel = *data;
            boxMon->metLevel = metLevel;
            break;
        }
        case MON_DATA_MET_GAME:
            break;
        case MON_DATA_POKEBALL:
        {
            u8 pokeball = *data;
            boxMon->pokeball = pokeball;
            break;
        }
        case MON_DATA_OT_GENDER:
            break;
        case MON_DATA_HP_IV:
            SET8(boxMon->hpIV);
            break;
        case MON_DATA_ATK_IV:
            SET8(boxMon->attackIV);
            break;
        case MON_DATA_DEF_IV:
            SET8(boxMon->defenseIV);
            break;
        case MON_DATA_SPEED_IV:
            SET8(boxMon->speedIV);
            break;
        case MON_DATA_SPATK_IV:
            SET8(boxMon->spAttackIV);
            break;
        case MON_DATA_SPDEF_IV:
            SET8(boxMon->spDefenseIV);
            break;
        case MON_DATA_IS_EGG:
            SET8(boxMon->isEgg);
            break;
        case MON_DATA_ABILITY_NUM:
        case MON_DATA_COOL_RIBBON:
        case MON_DATA_BEAUTY_RIBBON:
        case MON_DATA_CUTE_RIBBON:
        case MON_DATA_SMART_RIBBON:
        case MON_DATA_TOUGH_RIBBON:
        case MON_DATA_CHAMPION_RIBBON:
        case MON_DATA_WINNING_RIBBON:
        case MON_DATA_VICTORY_RIBBON:
        case MON_DATA_ARTIST_RIBBON:
        case MON_DATA_EFFORT_RIBBON:
        case MON_DATA_MARINE_RIBBON:
        case MON_DATA_LAND_RIBBON:
        case MON_DATA_SKY_RIBBON:
        case MON_DATA_COUNTRY_RIBBON:
        case MON_DATA_NATIONAL_RIBBON:
        case MON_DATA_EARTH_RIBBON:
        case MON_DATA_WORLD_RIBBON:
        case MON_DATA_MODERN_FATEFUL_ENCOUNTER:
            break;
        case MON_DATA_IVS:
        {
            u32 ivs = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
            boxMon->hpIV = ivs & MAX_IV_MASK;
            boxMon->attackIV = (ivs >> 5) & MAX_IV_MASK;
            boxMon->defenseIV = (ivs >> 10) & MAX_IV_MASK;
            boxMon->speedIV = (ivs >> 15) & MAX_IV_MASK;
            boxMon->spAttackIV = (ivs >> 20) & MAX_IV_MASK;
            boxMon->spDefenseIV = (ivs >> 25) & MAX_IV_MASK;
            break;
        }
        case MON_DATA_HYPER_TRAINED_HP:
        case MON_DATA_HYPER_TRAINED_ATK:
        case MON_DATA_HYPER_TRAINED_DEF:
        case MON_DATA_HYPER_TRAINED_SPEED:
        case MON_DATA_HYPER_TRAINED_SPATK:
        case MON_DATA_HYPER_TRAINED_SPDEF:
        case MON_DATA_IS_SHADOW:
        case MON_DATA_DYNAMAX_LEVEL:
        case MON_DATA_GIGANTAMAX_FACTOR:
        case MON_DATA_TERA_TYPE:
        case MON_DATA_EVOLUTION_TRACKER:
            break;
        case MON_DATA_STATUS:
            break;
        case MON_DATA_PERSONALITY:
            SET32(boxMon->personality);
            break;
        case MON_DATA_OT_ID:
            SET32(boxMon->otId);
            break;
        case MON_DATA_LANGUAGE:
        case MON_DATA_SANITY_IS_BAD_EGG:
        case MON_DATA_SANITY_HAS_SPECIES:
        case MON_DATA_SANITY_IS_EGG:
            break;
        case MON_DATA_OT_NAME:
        {
            s32 i;
            for (i = 0; i < PLAYER_NAME_LENGTH; i++)
                boxMon->otName[i] = data[i];
            break;
        }
        case MON_DATA_MARKINGS:
            break;
        case MON_DATA_IS_SHINY:
        {
            u32 shinyValue = GET_SHINY_VALUE(boxMon->otId, boxMon->personality);
            bool32 isShiny;
            SET8(isShiny);
            boxMon->shinyModifier = (shinyValue < SHINY_ODDS) ^ isShiny;
            break;
        }
        case MON_DATA_HIDDEN_NATURE:
            break;
    default:
        break;
    }
}

void CopyMon(void *dest, void *src, size_t size)
{
    memcpy(dest, src, size);
}

u8 GiveMonToPlayer(struct Pokemon *mon)
{
    s32 i;

    SetMonData(mon, MON_DATA_OT_NAME, gSaveBlock2Ptr->playerName);
    SetMonData(mon, MON_DATA_OT_ID, gSaveBlock2Ptr->playerTrainerId);

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES, NULL) == SPECIES_NONE)
            break;
    }

    if (i >= PARTY_SIZE)
        return CopyMonToPC(mon);

    CopyMon(&gPlayerParty[i], mon, sizeof(*mon));
    gPlayerPartyCount = i + 1;
    return MON_GIVEN_TO_PARTY;
}

u8 CopyMonToPC(struct Pokemon *mon)
{
    s32 boxNo, boxPos;

    SetPCBoxToSendMon(VarGet(VAR_PC_BOX_TO_SEND_MON));

    boxNo = StorageGetCurrentBox();

    do
    {
        for (boxPos = 0; boxPos < IN_BOX_COUNT; boxPos++)
        {
            struct BoxPokemon* checkingMon = GetBoxedMonPtr(boxNo, boxPos);
            if (GetBoxMonData(checkingMon, MON_DATA_SPECIES, NULL) == SPECIES_NONE)
            {
                MonRestorePP(mon);
                CopyMon(checkingMon, &mon->box, sizeof(mon->box));
                gSpecialVar_MonBoxId = boxNo;
                gSpecialVar_MonBoxPos = boxPos;
                VarSet(VAR_PC_BOX_TO_SEND_MON, boxNo);
                return MON_GIVEN_TO_PC;
            }
        }

        boxNo++;
        if (boxNo == TOTAL_BOXES_COUNT)
            boxNo = 0;
    } while (boxNo != StorageGetCurrentBox());

    return MON_CANT_GIVE;
}

u8 CalculatePartyCount(struct Pokemon *party)
{
    u32 partyCount = 0;

    while (partyCount < PARTY_SIZE
        && GetMonData(&party[partyCount], MON_DATA_SPECIES, NULL) != SPECIES_NONE)
    {
        partyCount++;
    }

    return partyCount;
}

u8 CalculatePlayerPartyCount(void)
{
    gPlayerPartyCount = CalculatePartyCount(gPlayerParty);
    return gPlayerPartyCount;
}

u8 CalculateEnemyPartyCount(void)
{
    gEnemyPartyCount = CalculatePartyCount(gEnemyParty);
    return gEnemyPartyCount;
}

u8 GetMonsStateToDoubles(void)
{
    s32 aliveCount = 0;
    s32 i;
    CalculatePlayerPartyCount();

    if (OW_DOUBLE_APPROACH_WITH_ONE_MON)
        return PLAYER_HAS_TWO_USABLE_MONS;

    if (gPlayerPartyCount == 1)
        return gPlayerPartyCount; // PLAYER_HAS_ONE_MON

    for (i = 0; i < gPlayerPartyCount; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG, NULL) != SPECIES_EGG
         && GetMonData(&gPlayerParty[i], MON_DATA_HP, NULL) != 0
         && GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG, NULL) != SPECIES_NONE)
            aliveCount++;
    }

    return (aliveCount > 1) ? PLAYER_HAS_TWO_USABLE_MONS : PLAYER_HAS_ONE_USABLE_MON;
}

u8 GetMonsStateToDoubles_2(void)
{
    s32 aliveCount = 0;
    s32 i;

    if (OW_DOUBLE_APPROACH_WITH_ONE_MON)
        return PLAYER_HAS_TWO_USABLE_MONS;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        u32 species = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG, NULL);
        if (species != SPECIES_EGG && species != SPECIES_NONE
         && GetMonData(&gPlayerParty[i], MON_DATA_HP, NULL) != 0)
            aliveCount++;
    }

    if (aliveCount == 1)
        return PLAYER_HAS_ONE_MON; // may have more than one, but only one is alive

    return (aliveCount > 1) ? PLAYER_HAS_TWO_USABLE_MONS : PLAYER_HAS_ONE_USABLE_MON;
}

u16 GetAbilityBySpecies(u16 species, u8 abilityNum)
{
    int i;

    if (abilityNum < NUM_ABILITY_SLOTS)
        gLastUsedAbility = gSpeciesInfo[species].abilities[abilityNum];
    else
        gLastUsedAbility = ABILITY_NONE;

    if (abilityNum >= NUM_NORMAL_ABILITY_SLOTS) // if abilityNum is empty hidden ability, look for other hidden abilities
    {
        for (i = NUM_NORMAL_ABILITY_SLOTS; i < NUM_ABILITY_SLOTS && gLastUsedAbility == ABILITY_NONE; i++)
        {
            gLastUsedAbility = gSpeciesInfo[species].abilities[i];
        }
    }

    for (i = 0; i < NUM_ABILITY_SLOTS && gLastUsedAbility == ABILITY_NONE; i++) // look for any non-empty ability
    {
        gLastUsedAbility = gSpeciesInfo[species].abilities[i];
    }

    return gLastUsedAbility;
}

u16 GetMonAbility(struct Pokemon *mon)
{
    u16 species = GetMonData(mon, MON_DATA_SPECIES, NULL);
    u8 abilityNum = GetMonData(mon, MON_DATA_ABILITY_NUM, NULL);
    return GetAbilityBySpecies(species, abilityNum);
}

void CreateSecretBaseEnemyParty(struct SecretBase *secretBaseRecord)
{
    s32 i, j;

    ZeroEnemyPartyMons();
    *gBattleResources->secretBase = *secretBaseRecord;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (gBattleResources->secretBase->party.species[i])
        {
            CreateMon(&gEnemyParty[i],
                gBattleResources->secretBase->party.species[i],
                gBattleResources->secretBase->party.levels[i],
                15,
                TRUE,
                gBattleResources->secretBase->party.personality[i],
                OT_ID_RANDOM_NO_SHINY,
                0);

            SetMonData(&gEnemyParty[i], MON_DATA_HELD_ITEM, &gBattleResources->secretBase->party.heldItems[i]);

            for (j = 0; j < NUM_STATS; j++)
                SetMonData(&gEnemyParty[i], MON_DATA_HP_EV + j, &gBattleResources->secretBase->party.EVs[i]);

            for (j = 0; j < MAX_MON_MOVES; j++)
            {
                SetMonData(&gEnemyParty[i], MON_DATA_MOVE1 + j, &gBattleResources->secretBase->party.moves[i * MAX_MON_MOVES + j]);
                SetMonData(&gEnemyParty[i], MON_DATA_PP1 + j, &gMovesInfo[gBattleResources->secretBase->party.moves[i * MAX_MON_MOVES + j]].pp);
            }
        }
    }
}

u8 GetSecretBaseTrainerPicIndex(void)
{
    u8 facilityClass = sSecretBaseFacilityClasses[gBattleResources->secretBase->gender][gBattleResources->secretBase->trainerId[0] % NUM_SECRET_BASE_CLASSES];
    return gFacilityClassToPicIndex[facilityClass];
}

u8 GetSecretBaseTrainerClass(void)
{
    u8 facilityClass = sSecretBaseFacilityClasses[gBattleResources->secretBase->gender][gBattleResources->secretBase->trainerId[0] % NUM_SECRET_BASE_CLASSES];
    return gFacilityClassToTrainerClass[facilityClass];
}

bool8 IsPlayerPartyAndPokemonStorageFull(void)
{
    s32 i;

    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES, NULL) == SPECIES_NONE)
            return FALSE;

    return IsPokemonStorageFull();
}

bool8 IsPokemonStorageFull(void)
{
    s32 i, j;

    for (i = 0; i < TOTAL_BOXES_COUNT; i++)
        for (j = 0; j < IN_BOX_COUNT; j++)
            if (GetBoxMonDataAt(i, j, MON_DATA_SPECIES) == SPECIES_NONE)
                return FALSE;

    return TRUE;
}

const u8 *GetSpeciesName(u16 species)
{
    species = SanitizeSpeciesId(species);
    if (gSpeciesInfo[species].speciesName[0] == 0)
        return gSpeciesInfo[SPECIES_NONE].speciesName;
    return gSpeciesInfo[species].speciesName;
}

const u8 *GetSpeciesCategory(u16 species)
{
    species = SanitizeSpeciesId(species);
    if (gSpeciesInfo[species].categoryName[0] == 0)
        return gSpeciesInfo[SPECIES_NONE].categoryName;
    return gSpeciesInfo[species].categoryName;
}

const u8 *GetSpeciesPokedexDescription(u16 species)
{
    species = SanitizeSpeciesId(species);
    if (gSpeciesInfo[species].description == NULL)
        return gSpeciesInfo[SPECIES_NONE].description;
    return gSpeciesInfo[species].description;
}

u16 GetSpeciesHeight(u16 species)
{
    return gSpeciesInfo[SanitizeSpeciesId(species)].height;
}

u16 GetSpeciesWeight(u16 species)
{
    return gSpeciesInfo[SanitizeSpeciesId(species)].weight;
}

const struct LevelUpMove *GetSpeciesLevelUpLearnset(u16 species)
{
    const struct LevelUpMove *learnset = gSpeciesInfo[SanitizeSpeciesId(species)].levelUpLearnset;
    if (learnset == NULL)
        return gSpeciesInfo[SPECIES_NONE].levelUpLearnset;
    return learnset;
}

const u16 *GetSpeciesTeachableLearnset(u16 species)
{
    const u16 *learnset = gSpeciesInfo[SanitizeSpeciesId(species)].teachableLearnset;
    if (learnset == NULL)
        return gSpeciesInfo[SPECIES_NONE].teachableLearnset;
    return learnset;
}

const u16 *GetSpeciesEggMoves(u16 species)
{
    const u16 *learnset = gSpeciesInfo[SanitizeSpeciesId(species)].eggMoveLearnset;
    if (learnset == NULL)
        return sNoneEggMoveLearnset;
    return learnset;
}

const struct Evolution *GetSpeciesEvolutions(u16 species)
{
    const struct Evolution *evolutions = gSpeciesInfo[SanitizeSpeciesId(species)].evolutions;
    if (evolutions == NULL)
        return gSpeciesInfo[SPECIES_NONE].evolutions;
    return evolutions;
}

const u16 *GetSpeciesFormTable(u16 species)
{
    const u16 *formTable = gSpeciesInfo[SanitizeSpeciesId(species)].formSpeciesIdTable;
    if (formTable == NULL)
        return gSpeciesInfo[SPECIES_NONE].formSpeciesIdTable;
    return formTable;
}

const struct FormChange *GetSpeciesFormChanges(u16 species)
{
    const struct FormChange *evolutions = gSpeciesInfo[SanitizeSpeciesId(species)].formChangeTable;
    if (evolutions == NULL)
        return gSpeciesInfo[SPECIES_NONE].formChangeTable;
    return evolutions;
}

u8 CalculatePPWithBonus(u16 move, u8 ppBonuses, u8 moveIndex)
{
    u8 basePP = gMovesInfo[move].pp;
    return basePP + ((basePP * 20 * ((gPPUpGetMask[moveIndex] & ppBonuses) >> (2 * moveIndex))) / 100);
}

void RemoveMonPPBonus(struct Pokemon *mon, u8 moveIndex)
{
    u8 ppBonuses = GetMonData(mon, MON_DATA_PP_BONUSES, NULL);
    ppBonuses &= gPPUpClearMask[moveIndex];
    SetMonData(mon, MON_DATA_PP_BONUSES, &ppBonuses);
}

void RemoveBattleMonPPBonus(struct BattlePokemon *mon, u8 moveIndex)
{
    mon->ppBonuses &= gPPUpClearMask[moveIndex];
}

void PokemonToBattleMon(struct Pokemon *src, struct BattlePokemon *dst)
{
    s32 i;
    u8 nickname[POKEMON_NAME_BUFFER_SIZE];

    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        dst->moves[i] = GetMonData(src, MON_DATA_MOVE1 + i, NULL);
        dst->pp[i] = GetMonData(src, MON_DATA_PP1 + i, NULL);
    }

    dst->species = GetMonData(src, MON_DATA_SPECIES, NULL);
    dst->item = GetMonData(src, MON_DATA_HELD_ITEM, NULL);
    dst->ppBonuses = GetMonData(src, MON_DATA_PP_BONUSES, NULL);
    dst->friendship = GetMonData(src, MON_DATA_FRIENDSHIP, NULL);
    dst->experience = GetMonData(src, MON_DATA_EXP, NULL);
    dst->hpIV = GetMonData(src, MON_DATA_HP_IV, NULL);
    dst->attackIV = GetMonData(src, MON_DATA_ATK_IV, NULL);
    dst->defenseIV = GetMonData(src, MON_DATA_DEF_IV, NULL);
    dst->speedIV = GetMonData(src, MON_DATA_SPEED_IV, NULL);
    dst->spAttackIV = GetMonData(src, MON_DATA_SPATK_IV, NULL);
    dst->spDefenseIV = GetMonData(src, MON_DATA_SPDEF_IV, NULL);
    dst->personality = GetMonData(src, MON_DATA_PERSONALITY, NULL);
    dst->status1 = GetMonData(src, MON_DATA_STATUS, NULL);
    dst->level = GetMonData(src, MON_DATA_LEVEL, NULL);
    dst->hp = GetMonData(src, MON_DATA_HP, NULL);
    dst->maxHP = GetMonData(src, MON_DATA_MAX_HP, NULL);
    dst->attack = GetMonData(src, MON_DATA_ATK, NULL);
    dst->defense = GetMonData(src, MON_DATA_DEF, NULL);
    dst->speed = GetMonData(src, MON_DATA_SPEED, NULL);
    dst->spAttack = GetMonData(src, MON_DATA_SPATK, NULL);
    dst->spDefense = GetMonData(src, MON_DATA_SPDEF, NULL);
    dst->abilityNum = GetMonData(src, MON_DATA_ABILITY_NUM, NULL);
    dst->otId = GetMonData(src, MON_DATA_OT_ID, NULL);
    dst->types[0] = gSpeciesInfo[dst->species].types[0];
    dst->types[1] = gSpeciesInfo[dst->species].types[1];
    dst->types[2] = TYPE_MYSTERY;
    dst->isShiny = IsMonShiny(src);
    dst->ability = GetAbilityBySpecies(dst->species, dst->abilityNum);
    GetMonData(src, MON_DATA_NICKNAME, nickname);
    StringCopy_Nickname(dst->nickname, nickname);
    GetMonData(src, MON_DATA_OT_NAME, dst->otName);

    for (i = 0; i < NUM_BATTLE_STATS; i++)
        dst->statStages[i] = DEFAULT_STAT_STAGE;

    dst->status2 = 0;
}

void CopyPlayerPartyMonToBattleData(u8 battlerId, u8 partyIndex)
{
    PokemonToBattleMon(&gPlayerParty[partyIndex], &gBattleMons[battlerId]);
    gBattleStruct->hpOnSwitchout[GetBattlerSide(battlerId)] = gBattleMons[battlerId].hp;
    UpdateSentPokesToOpponentValue(battlerId);
    ClearTemporarySpeciesSpriteData(battlerId, FALSE);
}

bool8 ExecuteTableBasedItemEffect(struct Pokemon *mon, u16 item, u8 partyIndex, u8 moveIndex)
{
    return PokemonUseItemEffects(mon, item, partyIndex, moveIndex, FALSE);
}

#define UPDATE_FRIENDSHIP_FROM_ITEM()                                                                   \
{                                                                                                       \
    if ((retVal == 0 || friendshipOnly) && !ShouldSkipFriendshipChange() && friendshipChange == 0)      \
    {                                                                                                   \
        friendshipChange = itemEffect[itemEffectParam];                                                 \
        friendship = GetMonData(mon, MON_DATA_FRIENDSHIP, NULL);                                        \
        if (friendshipChange > 0 && holdEffect == HOLD_EFFECT_FRIENDSHIP_UP)                            \
            friendship += 150 * friendshipChange / 100;                                                 \
        else                                                                                            \
            friendship += friendshipChange;                                                             \
        if (friendshipChange > 0)                                                                       \
        {                                                                                               \
            if (GetMonData(mon, MON_DATA_POKEBALL, NULL) == ITEM_LUXURY_BALL)                           \
                friendship++;                                                                           \
            if (GetMonData(mon, MON_DATA_MET_LOCATION, NULL) == GetCurrentRegionMapSectionId())         \
                friendship++;                                                                           \
        }                                                                                               \
        if (friendship < 0)                                                                             \
            friendship = 0;                                                                             \
        if (friendship > MAX_FRIENDSHIP)                                                                \
            friendship = MAX_FRIENDSHIP;                                                                \
        SetMonData(mon, MON_DATA_FRIENDSHIP, &friendship);                                              \
        retVal = FALSE;                                                                                 \
    }                                                                                                   \
}

// EXP candies store an index for this table in their holdEffectParam.
const u32 sExpCandyExperienceTable[] = {
    [EXP_100 - 1] = 100,
    [EXP_800 - 1] = 800,
    [EXP_3000 - 1] = 3000,
    [EXP_10000 - 1] = 10000,
    [EXP_30000 - 1] = 30000,
};

// Returns TRUE if the item has no effect on the Pokémon, FALSE otherwise
bool8 PokemonUseItemEffects(struct Pokemon *mon, u16 item, u8 partyIndex, u8 moveIndex, bool8 usedByAI)
{
    u32 dataUnsigned;
    s32 dataSigned, evCap;
    s32 friendship;
    s32 i;
    bool8 retVal = TRUE;
    const u8 *itemEffect;
    u8 itemEffectParam = ITEM_EFFECT_ARG_START;
    u32 temp1, temp2;
    s8 friendshipChange = 0;
    u8 holdEffect;
    u8 battlerId = MAX_BATTLERS_COUNT;
    u32 friendshipOnly = FALSE;
    u16 heldItem;
    u8 effectFlags;
    s8 evChange;
    u16 evCount;

    // Determine the EV cap to use
    u32 maxAllowedEVs = !B_EV_ITEMS_CAP ? MAX_TOTAL_EVS : GetCurrentEVCap();

    // Get item hold effect
    heldItem = GetMonData(mon, MON_DATA_HELD_ITEM, NULL);
    holdEffect = ItemId_GetHoldEffect(heldItem);

    // Skip using the item if it won't do anything
    if (ItemId_GetEffect(item) == NULL)
        return TRUE;

    // Get item effect
    itemEffect = ItemId_GetEffect(item);

    // Do item effect
    for (i = 0; i < ITEM_EFFECT_ARG_START; i++)
    {
        switch (i)
        {

        // Handle ITEM0 effects (infatuation, Dire Hit, X Attack). ITEM0_SACRED_ASH is handled in party_menu.c
        // Now handled in item battle scripts.
        case 0:
            break;

        // Handle ITEM1 effects (in-battle stat boosting effects)
        // Now handled in item battle scripts.
        case 1:
            break;
        // Formerly used by the item effects of the X Sp. Atk and the X Accuracy
        case 2:
            break;

        // Handle ITEM3 effects (Guard Spec, Rare Candy, cure status)
        case 3:
            // Rare Candy / EXP Candy
            if ((itemEffect[i] & ITEM3_LEVEL_UP)
             && GetMonData(mon, MON_DATA_LEVEL, NULL) != MAX_LEVEL)
            {
                u8 param = ItemId_GetHoldEffectParam(item);
                dataUnsigned = 0;

                if (param == 0) // Rare Candy
                {
                    dataUnsigned = gExperienceTables[gSpeciesInfo[GetMonData(mon, MON_DATA_SPECIES, NULL)].growthRate][GetMonData(mon, MON_DATA_LEVEL, NULL) + 1];
                }
                else if (param - 1 < ARRAY_COUNT(sExpCandyExperienceTable)) // EXP Candies
                {
                    u16 species = GetMonData(mon, MON_DATA_SPECIES, NULL);
                    dataUnsigned = sExpCandyExperienceTable[param - 1] + GetMonData(mon, MON_DATA_EXP, NULL);

                    if (B_RARE_CANDY_CAP && B_EXP_CAP_TYPE == EXP_CAP_HARD)
                    {
                        u32 currentLevelCap = GetCurrentLevelCap();
                        if (dataUnsigned > gExperienceTables[gSpeciesInfo[species].growthRate][currentLevelCap])
                            dataUnsigned = gExperienceTables[gSpeciesInfo[species].growthRate][currentLevelCap];
                    }
                    else if (dataUnsigned > gExperienceTables[gSpeciesInfo[species].growthRate][MAX_LEVEL])
                    {
                        dataUnsigned = gExperienceTables[gSpeciesInfo[species].growthRate][MAX_LEVEL];
                    }
                }

                if (dataUnsigned != 0) // Failsafe
                {
                    SetMonData(mon, MON_DATA_EXP, &dataUnsigned);
                    CalculateMonStats(mon);
                    retVal = FALSE;
                }
            }

            // Cure status
            if ((itemEffect[i] & ITEM3_SLEEP) && HealStatusConditions(mon, STATUS1_SLEEP, battlerId) == 0)
                retVal = FALSE;
            if ((itemEffect[i] & ITEM3_POISON) && HealStatusConditions(mon, STATUS1_PSN_ANY | STATUS1_TOXIC_COUNTER, battlerId) == 0)
                retVal = FALSE;
            if ((itemEffect[i] & ITEM3_BURN) && HealStatusConditions(mon, STATUS1_BURN, battlerId) == 0)
                retVal = FALSE;
            if ((itemEffect[i] & ITEM3_FREEZE) && HealStatusConditions(mon, STATUS1_FREEZE | STATUS1_FROSTBITE, battlerId) == 0)
                retVal = FALSE;
            if ((itemEffect[i] & ITEM3_PARALYSIS) && HealStatusConditions(mon, STATUS1_PARALYSIS, battlerId) == 0)
                retVal = FALSE;
            break;

        // Handle ITEM4 effects (Change HP/Atk EVs, HP heal, PP heal, PP up, Revive, and evolution stones)
        case 4:
            effectFlags = itemEffect[i];

            // PP Up
            if (effectFlags & ITEM4_PP_UP)
            {
                effectFlags &= ~ITEM4_PP_UP;
                dataUnsigned = (GetMonData(mon, MON_DATA_PP_BONUSES, NULL) & gPPUpGetMask[moveIndex]) >> (moveIndex * 2);
                temp1 = CalculatePPWithBonus(GetMonData(mon, MON_DATA_MOVE1 + moveIndex, NULL), GetMonData(mon, MON_DATA_PP_BONUSES, NULL), moveIndex);
                if (dataUnsigned <= 2 && temp1 > 4)
                {
                    dataUnsigned = GetMonData(mon, MON_DATA_PP_BONUSES, NULL) + gPPUpAddValues[moveIndex];
                    SetMonData(mon, MON_DATA_PP_BONUSES, &dataUnsigned);

                    dataUnsigned = CalculatePPWithBonus(GetMonData(mon, MON_DATA_MOVE1 + moveIndex, NULL), dataUnsigned, moveIndex) - temp1;
                    dataUnsigned = GetMonData(mon, MON_DATA_PP1 + moveIndex, NULL) + dataUnsigned;
                    SetMonData(mon, MON_DATA_PP1 + moveIndex, &dataUnsigned);
                    retVal = FALSE;
                }
            }
            temp1 = 0;

            // Loop through and try each of the remaining ITEM4 effects
            while (effectFlags != 0)
            {
                if (effectFlags & 1)
                {
                    switch (temp1)
                    {
                    case 0: // ITEM4_EV_HP
                    case 1: // ITEM4_EV_ATK
                        evCount = GetMonEVCount(mon);
                        temp2 = itemEffect[itemEffectParam];
                        dataSigned = GetMonData(mon, sGetMonDataEVConstants[temp1], NULL);
                        evChange = temp2;

                        if (evChange > 0) // Increasing EV (HP or Atk)
                        {
                            // Check if the total EV limit is reached
                            if (evCount >= maxAllowedEVs)
                                return TRUE;

                            // Ensure the increase does not exceed the max EV per stat (252)
                            evCap = (itemEffect[10] & ITEM10_IS_VITAMIN) ? EV_ITEM_RAISE_LIMIT : MAX_PER_STAT_EVS;

                            // Check if the per-stat limit is reached
                            if (dataSigned >= evCap)
                                return TRUE;  // Prevents item use if the per-stat cap is already reached

                            if (dataSigned + evChange > evCap)
                                temp2 = evCap - dataSigned;
                            else
                                temp2 = evChange;

                            // Ensure the total EVs do not exceed the maximum allowed (510)
                            if (evCount + temp2 > maxAllowedEVs)
                                temp2 = maxAllowedEVs - evCount;

                            // Prevent item use if no EVs can be increased
                            if (temp2 == 0)
                                return TRUE;

                            // Apply the EV increase
                            dataSigned += temp2;
                        }
                        else if (evChange < 0) // Decreasing EV (HP or Atk)
                        {
                            if (dataSigned == 0)
                            {
                                // No EVs to lose, but make sure friendship updates anyway
                                friendshipOnly = TRUE;
                                itemEffectParam++;
                                break;
                            }
                            dataSigned += evChange;
                            if (I_BERRY_EV_JUMP == GEN_4 && dataSigned > 100)
                                dataSigned = 100;
                            if (dataSigned < 0)
                                dataSigned = 0;
                        }
                        else // Reset EV (HP or Atk)
                        {
                            if (dataSigned == 0)
                                break;

                            dataSigned = 0;
                        }

                        // Update EVs and stats
                        SetMonData(mon, sGetMonDataEVConstants[temp1], &dataSigned);
                        CalculateMonStats(mon);
                        itemEffectParam++;
                        retVal = FALSE;
                        break;

                    case 2: // ITEM4_HEAL_HP
                        // Check use validity.
                        if ((effectFlags & (ITEM4_REVIVE >> 2) && GetMonData(mon, MON_DATA_HP, NULL) != 0)
                              || (!(effectFlags & (ITEM4_REVIVE >> 2)) && GetMonData(mon, MON_DATA_HP, NULL) == 0))
                        {
                            itemEffectParam++;
                            break;
                        }

                        // Get amount of HP to restore
                        dataUnsigned = itemEffect[itemEffectParam++];
                        switch (dataUnsigned)
                        {
                        case ITEM6_HEAL_HP_FULL:
                            dataUnsigned = GetMonData(mon, MON_DATA_MAX_HP, NULL) - GetMonData(mon, MON_DATA_HP, NULL);
                            break;
                        case ITEM6_HEAL_HP_HALF:
                            dataUnsigned = GetMonData(mon, MON_DATA_MAX_HP, NULL) / 2;
                            if (dataUnsigned == 0)
                                dataUnsigned = 1;
                            break;
                        case ITEM6_HEAL_HP_LVL_UP:
                            dataUnsigned = gBattleScripting.levelUpHP;
                            break;
                        case ITEM6_HEAL_HP_QUARTER:
                            dataUnsigned = GetMonData(mon, MON_DATA_MAX_HP, NULL) / 4;
                            if (dataUnsigned == 0)
                                dataUnsigned = 1;
                            break;
                        }

                        // Only restore HP if not at max health
                        if (GetMonData(mon, MON_DATA_MAX_HP, NULL) != GetMonData(mon, MON_DATA_HP, NULL))
                        {
                            // Restore HP
                            dataUnsigned = GetMonData(mon, MON_DATA_HP, NULL) + dataUnsigned;
                            if (dataUnsigned > GetMonData(mon, MON_DATA_MAX_HP, NULL))
                                dataUnsigned = GetMonData(mon, MON_DATA_MAX_HP, NULL);
                            SetMonData(mon, MON_DATA_HP, &dataUnsigned);
                            retVal = FALSE;
                        }
                        effectFlags &= ~(ITEM4_REVIVE >> 2);
                        break;

                    case 3: // ITEM4_HEAL_PP
                        if (!(effectFlags & (ITEM4_HEAL_PP_ONE >> 3)))
                        {
                            // Heal PP for all moves
                            for (temp2 = 0; (signed)(temp2) < (signed)(MAX_MON_MOVES); temp2++)
                            {
                                u16 moveId;
                                dataUnsigned = GetMonData(mon, MON_DATA_PP1 + temp2, NULL);
                                moveId = GetMonData(mon, MON_DATA_MOVE1 + temp2, NULL);
                                if (dataUnsigned != CalculatePPWithBonus(moveId, GetMonData(mon, MON_DATA_PP_BONUSES, NULL), temp2))
                                {
                                    dataUnsigned += itemEffect[itemEffectParam];
                                    moveId = GetMonData(mon, MON_DATA_MOVE1 + temp2, NULL); // Redundant
                                    if (dataUnsigned > CalculatePPWithBonus(moveId, GetMonData(mon, MON_DATA_PP_BONUSES, NULL), temp2))
                                    {
                                        moveId = GetMonData(mon, MON_DATA_MOVE1 + temp2, NULL); // Redundant
                                        dataUnsigned = CalculatePPWithBonus(moveId, GetMonData(mon, MON_DATA_PP_BONUSES, NULL), temp2);
                                    }
                                    SetMonData(mon, MON_DATA_PP1 + temp2, &dataUnsigned);
                                    retVal = FALSE;
                                }
                            }
                            itemEffectParam++;
                        }
                        else
                        {
                            // Heal PP for one move
                            u16 moveId;
                            dataUnsigned = GetMonData(mon, MON_DATA_PP1 + moveIndex, NULL);
                            moveId = GetMonData(mon, MON_DATA_MOVE1 + moveIndex, NULL);
                            if (dataUnsigned != CalculatePPWithBonus(moveId, GetMonData(mon, MON_DATA_PP_BONUSES, NULL), moveIndex))
                            {
                                dataUnsigned += itemEffect[itemEffectParam++];
                                moveId = GetMonData(mon, MON_DATA_MOVE1 + moveIndex, NULL); // Redundant
                                if (dataUnsigned > CalculatePPWithBonus(moveId, GetMonData(mon, MON_DATA_PP_BONUSES, NULL), moveIndex))
                                {
                                    moveId = GetMonData(mon, MON_DATA_MOVE1 + moveIndex, NULL); // Redundant
                                    dataUnsigned = CalculatePPWithBonus(moveId, GetMonData(mon, MON_DATA_PP_BONUSES, NULL), moveIndex);
                                }
                                SetMonData(mon, MON_DATA_PP1 + moveIndex, &dataUnsigned);
                                retVal = FALSE;
                            }
                        }
                        break;

                    // cases 4-6 are ITEM4_HEAL_PP_ONE, ITEM4_PP_UP, and ITEM4_REVIVE, which
                    // are already handled above by other cases or before the loop

                    case 7: // ITEM4_EVO_STONE
                        {
                            u16 targetSpecies = GetEvolutionTargetSpecies(mon, EVO_MODE_ITEM_USE, item, NULL);

                            if (targetSpecies != SPECIES_NONE)
                            {
                                BeginEvolutionScene(mon, targetSpecies, FALSE, partyIndex);
                                return FALSE;
                            }
                        }
                        break;
                    }
                }
                temp1++;
                effectFlags >>= 1;
            }
            break;

        // Handle ITEM5 effects (Change Def/SpDef/SpAtk/Speed EVs, PP Max, and friendship changes)
        case 5:
            effectFlags = itemEffect[i];
            temp1 = 0;

            // Loop through and try each of the ITEM5 effects
            while (effectFlags != 0)
            {
                if (effectFlags & 1)
                {
                    switch (temp1)
                    {
                    case 0: // ITEM5_EV_DEF
                    case 1: // ITEM5_EV_SPEED
                    case 2: // ITEM5_EV_SPDEF
                    case 3: // ITEM5_EV_SPATK
                        evCount = GetMonEVCount(mon);
                        temp2 = itemEffect[itemEffectParam];
                        dataSigned = GetMonData(mon, sGetMonDataEVConstants[temp1 + 2], NULL);
                        evChange = temp2;
                        if (evChange > 0) // Increasing EV
                        {
                            // Check if the total EV limit is reached
                            if (evCount >= maxAllowedEVs)
                                return TRUE;

                            // Ensure the increase does not exceed the max EV per stat (252)
                            evCap = (itemEffect[10] & ITEM10_IS_VITAMIN) ? EV_ITEM_RAISE_LIMIT : MAX_PER_STAT_EVS;

                            // Check if the per-stat limit is reached
                            if (dataSigned >= evCap)
                                return TRUE;  // Prevents item use if the per-stat cap is already reached

                            if (dataSigned + evChange > evCap)
                                temp2 = evCap - dataSigned;
                            else
                                temp2 = evChange;

                            // Ensure the total EVs do not exceed the maximum allowed (510)
                            if (evCount + temp2 > maxAllowedEVs)
                                temp2 = maxAllowedEVs - evCount;

                            // Prevent item use if no EVs can be increased
                            if (temp2 == 0)
                                return TRUE;

                            // Apply the EV increase
                            dataSigned += temp2;
                        }
                        else if (evChange < 0) // Decreasing EV
                        {
                            if (dataSigned == 0)
                            {
                                // No EVs to lose, but make sure friendship updates anyway
                                friendshipOnly = TRUE;
                                itemEffectParam++;
                                break;
                            }
                            dataSigned += evChange;
                            if (I_BERRY_EV_JUMP == GEN_4 && dataSigned > 100)
                                dataSigned = 100;
                            if (dataSigned < 0)
                                dataSigned = 0;
                        }
                        else // Reset EV
                        {
                            if (dataSigned == 0)
                                break;

                            dataSigned = 0;
                        }

                        // Update EVs and stats
                        SetMonData(mon, sGetMonDataEVConstants[temp1 + 2], &dataSigned);
                        CalculateMonStats(mon);
                        retVal = FALSE;
                        itemEffectParam++;
                        break;

                    case 4: // ITEM5_PP_MAX
                        dataUnsigned = (GetMonData(mon, MON_DATA_PP_BONUSES, NULL) & gPPUpGetMask[moveIndex]) >> (moveIndex * 2);
                        temp2 = CalculatePPWithBonus(GetMonData(mon, MON_DATA_MOVE1 + moveIndex, NULL), GetMonData(mon, MON_DATA_PP_BONUSES, NULL), moveIndex);

                        // Check if 3 PP Ups have been applied already, and that the move has a total PP of at least 5 (excludes Sketch)
                        if (dataUnsigned < 3 && temp2 >= 5)
                        {
                            dataUnsigned = GetMonData(mon, MON_DATA_PP_BONUSES, NULL);
                            dataUnsigned &= gPPUpClearMask[moveIndex];
                            dataUnsigned += gPPUpAddValues[moveIndex] * 3; // Apply 3 PP Ups (max)

                            SetMonData(mon, MON_DATA_PP_BONUSES, &dataUnsigned);
                            dataUnsigned = CalculatePPWithBonus(GetMonData(mon, MON_DATA_MOVE1 + moveIndex, NULL), dataUnsigned, moveIndex) - temp2;
                            dataUnsigned = GetMonData(mon, MON_DATA_PP1 + moveIndex, NULL) + dataUnsigned;
                            SetMonData(mon, MON_DATA_PP1 + moveIndex, &dataUnsigned);
                            retVal = FALSE;
                        }
                        break;

                    case 5: // ITEM5_FRIENDSHIP_LOW
                        // Changes to friendship are given differently depending on
                        // how much friendship the Pokémon already has.
                        // In general, Pokémon with lower friendship receive more,
                        // and Pokémon with higher friendship receive less.
                        if (GetMonData(mon, MON_DATA_FRIENDSHIP, NULL) < 100)
                            UPDATE_FRIENDSHIP_FROM_ITEM();
                        itemEffectParam++;
                        break;

                    case 6: // ITEM5_FRIENDSHIP_MID
                        if (GetMonData(mon, MON_DATA_FRIENDSHIP, NULL) >= 100 && GetMonData(mon, MON_DATA_FRIENDSHIP, NULL) < 200)
                            UPDATE_FRIENDSHIP_FROM_ITEM();
                        itemEffectParam++;
                        break;

                    case 7: // ITEM5_FRIENDSHIP_HIGH
                        if (GetMonData(mon, MON_DATA_FRIENDSHIP, NULL) >= 200)
                            UPDATE_FRIENDSHIP_FROM_ITEM();
                        itemEffectParam++;
                        break;
                    }
                }
                temp1++;
                effectFlags >>= 1;
            }
            break;
        }
    }
    return retVal;
}

bool8 HealStatusConditions(struct Pokemon *mon, u32 healMask, u8 battlerId)
{
    u32 status = GetMonData(mon, MON_DATA_STATUS, 0);

    if (status & healMask)
    {
        status &= ~healMask;
        SetMonData(mon, MON_DATA_STATUS, &status);
        if (gMain.inBattle && battlerId != MAX_BATTLERS_COUNT)
            gBattleMons[battlerId].status1 &= ~healMask;
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

u8 GetItemEffectParamOffset(u32 battler, u16 itemId, u8 effectByte, u8 effectBit)
{
    const u8 *temp;
    const u8 *itemEffect;
    u8 offset;
    int i;
    u8 j;
    u8 effectFlags;

    offset = ITEM_EFFECT_ARG_START;

    temp = ItemId_GetEffect(itemId);

    if (temp != NULL && !temp)
        return 0;

    itemEffect = temp;

    for (i = 0; i < ITEM_EFFECT_ARG_START; i++)
    {
        switch (i)
        {
        case 0:
        case 1:
        case 2:
        case 3:
            if (i == effectByte)
                return 0;
            break;
        case 4:
            effectFlags = itemEffect[4];
            if (effectFlags & ITEM4_PP_UP)
                effectFlags &= ~(ITEM4_PP_UP);
            j = 0;
            while (effectFlags)
            {
                if (effectFlags & 1)
                {
                    switch (j)
                    {
                    case 2: // ITEM4_HEAL_HP
                        if (effectFlags & (ITEM4_REVIVE >> 2))
                            effectFlags &= ~(ITEM4_REVIVE >> 2);
                        // fallthrough
                    case 0: // ITEM4_EV_HP
                        if (i == effectByte && (effectFlags & effectBit))
                            return offset;
                        offset++;
                        break;
                    case 1: // ITEM4_EV_ATK
                        if (i == effectByte && (effectFlags & effectBit))
                            return offset;
                        offset++;
                        break;
                    case 3: // ITEM4_HEAL_PP
                        if (i == effectByte && (effectFlags & effectBit))
                            return offset;
                        offset++;
                        break;
                    case 7: // ITEM4_EVO_STONE
                        if (i == effectByte)
                            return 0;
                        break;
                    }
                }
                j++;
                effectFlags >>= 1;
                if (i == effectByte)
                    effectBit >>= 1;
            }
            break;
        case 5:
            effectFlags = itemEffect[5];
            j = 0;
            while (effectFlags)
            {
                if (effectFlags & 1)
                {
                    switch (j)
                    {
                    case 0: // ITEM5_EV_DEF
                    case 1: // ITEM5_EV_SPEED
                    case 2: // ITEM5_EV_SPDEF
                    case 3: // ITEM5_EV_SPATK
                    case 4: // ITEM5_PP_MAX
                    case 5: // ITEM5_FRIENDSHIP_LOW
                    case 6: // ITEM5_FRIENDSHIP_MID
                        if (i == effectByte && (effectFlags & effectBit))
                            return offset;
                        offset++;
                        break;
                    case 7: // ITEM5_FRIENDSHIP_HIGH
                        if (i == effectByte)
                            return 0;
                        break;
                    }
                }
                j++;
                effectFlags >>= 1;
                if (i == effectByte)
                    effectBit >>= 1;
            }
            break;
        }
    }

    return offset;
}

static void BufferStatRoseMessage(s32 statIdx)
{
    gBattlerTarget = gBattlerInMenuId;
    StringCopy(gBattleTextBuff1, gStatNamesTable[sStatsToRaise[statIdx]]);
    if (B_X_ITEMS_BUFF >= GEN_7)
    {
        StringCopy(gBattleTextBuff2, gText_StatSharply);
        StringAppend(gBattleTextBuff2, gText_StatRose);
    }
    else
    {
        StringCopy(gBattleTextBuff2, gText_StatRose);
    }
    BattleStringExpandPlaceholdersToDisplayedString(gText_DefendersStatRose);
}

u8 *UseStatIncreaseItem(u16 itemId)
{
    const u8 *itemEffect;

    itemEffect = ItemId_GetEffect(itemId);

    gPotentialItemEffectBattler = gBattlerInMenuId;

    if (itemEffect[0] & ITEM0_DIRE_HIT)
    {
        gBattlerAttacker = gBattlerInMenuId;
        BattleStringExpandPlaceholdersToDisplayedString(gText_PkmnGettingPumped);
    }

    switch (itemEffect[1])
    {
        case ITEM1_X_ATTACK:
            BufferStatRoseMessage(STAT_ATK);
            break;
        case ITEM1_X_DEFENSE:
            BufferStatRoseMessage(STAT_DEF);
            break;
        case ITEM1_X_SPEED:
            BufferStatRoseMessage(STAT_SPEED);
            break;
        case ITEM1_X_SPATK:
            BufferStatRoseMessage(STAT_SPATK);
            break;
        case ITEM1_X_SPDEF:
            BufferStatRoseMessage(STAT_SPDEF);
            break;
        case ITEM1_X_ACCURACY:
            BufferStatRoseMessage(STAT_ACC);
            break;
    }

    if (itemEffect[3] & ITEM3_GUARD_SPEC)
    {
        gBattlerAttacker = gBattlerInMenuId;
        BattleStringExpandPlaceholdersToDisplayedString(gText_PkmnShroudedInMist);
    }

    return gDisplayedStringBattle;
}

u8 GetNature(struct Pokemon *mon)
{
    return GetMonData(mon, MON_DATA_PERSONALITY, 0) % NUM_NATURES;
}

u8 GetNatureFromPersonality(u32 personality)
{
    return personality % NUM_NATURES;
}

static u32 GetGMaxTargetSpecies(u32 species)
{
    const struct FormChange *formChanges = GetSpeciesFormChanges(species);
    u32 i;
    for (i = 0; formChanges[i].method != FORM_CHANGE_TERMINATOR; i++)
    {
        if (formChanges[i].method == FORM_CHANGE_BATTLE_GIGANTAMAX)
            return formChanges[i].targetSpecies;
    }
    return SPECIES_NONE;
}

u16 GetEvolutionTargetSpecies(struct Pokemon *mon, enum EvolutionMode mode, u16 evolutionItem, struct Pokemon *tradePartner)
{
    int i, j;
    u16 targetSpecies = SPECIES_NONE;
    u16 species = GetMonData(mon, MON_DATA_SPECIES, 0);
    u16 heldItem = GetMonData(mon, MON_DATA_HELD_ITEM, 0);
    u32 personality = GetMonData(mon, MON_DATA_PERSONALITY, 0);
    u8 level;
    u16 friendship;
    u8 beauty = GetMonData(mon, MON_DATA_BEAUTY, 0);
    u16 upperPersonality = personality >> 16;
    u32 holdEffect, currentMap, partnerSpecies, partnerHeldItem, partnerHoldEffect;
    bool32 consumeItem = FALSE;
    u16 evolutionTracker = GetMonData(mon, MON_DATA_EVOLUTION_TRACKER, 0);
    const struct Evolution *evolutions = GetSpeciesEvolutions(species);

    if (evolutions == NULL)
        return SPECIES_NONE;

    if (tradePartner != NULL)
    {
        partnerSpecies = GetMonData(tradePartner, MON_DATA_SPECIES, 0);
        partnerHeldItem = GetMonData(tradePartner, MON_DATA_HELD_ITEM, 0);

        partnerHoldEffect = ItemId_GetHoldEffect(partnerHeldItem);
    }
    else
    {
        partnerSpecies = SPECIES_NONE;
        partnerHeldItem = ITEM_NONE;
        partnerHoldEffect = HOLD_EFFECT_NONE;
    }

    holdEffect = ItemId_GetHoldEffect(heldItem);

    // Prevent evolution with Everstone, unless we're just viewing the party menu with an evolution item
    if (holdEffect == HOLD_EFFECT_PREVENT_EVOLVE
        && mode != EVO_MODE_ITEM_CHECK
        && (P_KADABRA_EVERSTONE < GEN_4 || species != SPECIES_KADABRA))
        return SPECIES_NONE;

    switch (mode)
    {
    case EVO_MODE_NORMAL:
    case EVO_MODE_BATTLE_ONLY:
        level = GetMonData(mon, MON_DATA_LEVEL, 0);
        friendship = GetMonData(mon, MON_DATA_FRIENDSHIP, 0);

        for (i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
        {
            if (SanitizeSpeciesId(evolutions[i].targetSpecies) == SPECIES_NONE)
                continue;

            switch (evolutions[i].method)
            {
            case EVO_FRIENDSHIP:
                if (friendship >= FRIENDSHIP_EVO_THRESHOLD)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_FRIENDSHIP_DAY:
                if (GetTimeOfDay() != TIEMPO_NOCHE && friendship >= FRIENDSHIP_EVO_THRESHOLD)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_LEVEL_DAY:
                if (GetTimeOfDay() != TIEMPO_NOCHE && evolutions[i].param <= level)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_FRIENDSHIP_NIGHT:
                if (GetTimeOfDay() == TIEMPO_NOCHE && friendship >= FRIENDSHIP_EVO_THRESHOLD)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_LEVEL_NIGHT:
                if (GetTimeOfDay() == TIEMPO_NOCHE && evolutions[i].param <= level)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_ITEM_HOLD_NIGHT:
                if (GetTimeOfDay() == TIEMPO_NOCHE && heldItem == evolutions[i].param)
                {
                    targetSpecies = evolutions[i].targetSpecies;
                    consumeItem = TRUE;
                }
                break;
            case EVO_ITEM_HOLD_DAY:
                if (GetTimeOfDay() != TIEMPO_NOCHE && heldItem == evolutions[i].param)
                {
                    targetSpecies = evolutions[i].targetSpecies;
                    consumeItem = TRUE;
                }
                break;
            case EVO_LEVEL_DUSK:
                if (GetTimeOfDay() == TIEMPO_TARDE && evolutions[i].param <= level)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_LEVEL:
                if (evolutions[i].param <= level)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_LEVEL_FEMALE:
                if (evolutions[i].param <= level && GetMonGender(mon) == MON_FEMALE)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_LEVEL_MALE:
                if (evolutions[i].param <= level && GetMonGender(mon) == MON_MALE)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_LEVEL_ATK_GT_DEF:
                if (evolutions[i].param <= level)
                    if (GetMonData(mon, MON_DATA_ATK, 0) > GetMonData(mon, MON_DATA_DEF, 0))
                        targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_LEVEL_ATK_EQ_DEF:
                if (evolutions[i].param <= level)
                    if (GetMonData(mon, MON_DATA_ATK, 0) == GetMonData(mon, MON_DATA_DEF, 0))
                        targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_LEVEL_ATK_LT_DEF:
                if (evolutions[i].param <= level)
                    if (GetMonData(mon, MON_DATA_ATK, 0) < GetMonData(mon, MON_DATA_DEF, 0))
                        targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_LEVEL_SILCOON:
                if (evolutions[i].param <= level && (upperPersonality % 10) <= 4)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_LEVEL_CASCOON:
                if (evolutions[i].param <= level && (upperPersonality % 10) > 4)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_LEVEL_NINJASK:
                if (evolutions[i].param <= level)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_LEVEL_FAMILY_OF_FOUR:
                if (mode == EVO_MODE_BATTLE_ONLY && evolutions[i].param <= level && (personality % 100) != 0)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_LEVEL_FAMILY_OF_THREE:
                if (mode == EVO_MODE_BATTLE_ONLY && evolutions[i].param <= level && (personality % 100) == 0)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_BEAUTY:
                if (evolutions[i].param <= beauty)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_MOVE:
                if (MonKnowsMove(mon, evolutions[i].param))
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_MOVE_TWO_SEGMENT:
                if (MonKnowsMove(mon, evolutions[i].param) && (personality % 100) != 0)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_MOVE_THREE_SEGMENT:
                if (MonKnowsMove(mon, evolutions[i].param) && (personality % 100) == 0)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_FRIENDSHIP_MOVE_TYPE:
                if (friendship >= FRIENDSHIP_EVO_THRESHOLD)
                {
                    for (j = 0; j < MAX_MON_MOVES; j++)
                    {
                        if (gMovesInfo[GetMonData(mon, MON_DATA_MOVE1 + j, NULL)].type == evolutions[i].param)
                        {
                            targetSpecies = evolutions[i].targetSpecies;
                            break;
                        }
                    }
                }
                break;
            case EVO_SPECIFIC_MON_IN_PARTY:
                for (j = 0; j < PARTY_SIZE; j++)
                {
                    if (GetMonData(&gPlayerParty[j], MON_DATA_SPECIES, NULL) == evolutions[i].param)
                    {
                        targetSpecies = evolutions[i].targetSpecies;
                        break;
                    }
                }
                break;
            case EVO_LEVEL_DARK_TYPE_MON_IN_PARTY:
                if (evolutions[i].param <= level)
                {
                    for (j = 0; j < PARTY_SIZE; j++)
                    {
                        u16 currSpecies = GetMonData(&gPlayerParty[j], MON_DATA_SPECIES, NULL);
                        if (gSpeciesInfo[currSpecies].types[0] == TYPE_DARK
                         || gSpeciesInfo[currSpecies].types[1] == TYPE_DARK)
                        {
                            targetSpecies = evolutions[i].targetSpecies;
                            break;
                        }
                    }
                }
                break;
            case EVO_LEVEL_RAIN:
                j = GetCurrentWeather();
                if (evolutions[i].param <= level
                 && (j == WEATHER_RAIN || j == WEATHER_RAIN_THUNDERSTORM || j == WEATHER_DOWNPOUR))
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_LEVEL_FOG:
                j = GetCurrentWeather();
                if (evolutions[i].param <= level
                 && (j == WEATHER_FOG_HORIZONTAL || j == WEATHER_FOG_DIAGONAL))
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_MAPSEC:
                if (gMapHeader.regionMapSectionId == evolutions[i].param)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_SPECIFIC_MAP:
                currentMap = ((gSaveBlock1Ptr->location.mapGroup) << 8 | gSaveBlock1Ptr->location.mapNum);
                if (currentMap == evolutions[i].param)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_LEVEL_NATURE_AMPED:
                if (evolutions[i].param <= level)
                {
                    u8 nature = GetNature(mon);
                    switch (nature)
                    {
                    case NATURE_OFENSIVA:
                    case NATURE_OFENSIVA_ESPECIAL:
                    case NATURE_RAPIDA:
                        targetSpecies = evolutions[i].targetSpecies;
                        break;
                    }
                }
                break;
            case EVO_LEVEL_NATURE_LOW_KEY:
                if (evolutions[i].param <= level)
                {
                    u8 nature = GetNature(mon);
                    switch (nature)
                    {
                    case NATURE_DEFENSIVA:
                    case NATURE_DEFENSIVA_ESPECIAL:
                        targetSpecies = evolutions[i].targetSpecies;
                        break;
                    }
                }
                break;
            case EVO_ITEM_HOLD:
                if (heldItem == evolutions[i].param)
                {
                    targetSpecies = evolutions[i].targetSpecies;
                    consumeItem = TRUE;
                }
                break;
            case EVO_USE_MOVE_TWENTY_TIMES:
                if (evolutionTracker >= 20)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_RECOIL_DAMAGE_MALE:
                if (evolutionTracker >= evolutions[i].param && GetMonGender(mon) == MON_MALE)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_RECOIL_DAMAGE_FEMALE:
                if (evolutionTracker >= evolutions[i].param && GetMonGender(mon) == MON_FEMALE)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_DEFEAT_THREE_WITH_ITEM:
                if (evolutionTracker >= 3)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_OVERWORLD_STEPS:
                if (mon == GetFirstLiveMon() && gFollowerSteps >= evolutions[i].param)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            }
        }
        break;
    case EVO_MODE_CANT_STOP:
        level = GetMonData(mon, MON_DATA_LEVEL, 0);
        friendship = GetMonData(mon, MON_DATA_FRIENDSHIP, 0);

        for (i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
        {
            if (SanitizeSpeciesId(evolutions[i].targetSpecies) == SPECIES_NONE)
                continue;

            switch (evolutions[i].method)
            {
            case EVO_ITEM_COUNT_999:
                if (CheckBagHasItem(evolutions[i].param, 999))
                {
                    targetSpecies = evolutions[i].targetSpecies;
                    RemoveBagItem(evolutions[i].param, 999);
                }
                break;
            }
        }
        break;
    case EVO_MODE_TRADE:
        for (i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
        {
            if (SanitizeSpeciesId(evolutions[i].targetSpecies) == SPECIES_NONE)
                continue;

            switch (evolutions[i].method)
            {
            case EVO_TRADE:
                targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_TRADE_ITEM:
                if (evolutions[i].param == heldItem)
                {
                    targetSpecies = evolutions[i].targetSpecies;
                    consumeItem = TRUE;
                }
                break;
            case EVO_TRADE_SPECIFIC_MON:
                if (evolutions[i].param == partnerSpecies && partnerHoldEffect != HOLD_EFFECT_PREVENT_EVOLVE)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            }
        }
        break;
    case EVO_MODE_ITEM_USE:
    case EVO_MODE_ITEM_CHECK:
        for (i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
        {
            if (SanitizeSpeciesId(evolutions[i].targetSpecies) == SPECIES_NONE)
                continue;

            switch (evolutions[i].method)
            {
            case EVO_ITEM:
                if (evolutions[i].param == evolutionItem)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_ITEM_FEMALE:
                if (GetMonGender(mon) == MON_FEMALE && evolutions[i].param == evolutionItem)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_ITEM_MALE:
                if (GetMonGender(mon) == MON_MALE && evolutions[i].param == evolutionItem)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_ITEM_NIGHT:
                if (GetTimeOfDay() == TIEMPO_NOCHE && evolutions[i].param == evolutionItem)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_ITEM_DAY:
                if (GetTimeOfDay() != TIEMPO_NOCHE && evolutions[i].param == evolutionItem)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            }
        }
        break;
    // Battle evolution without leveling; party slot is being passed into the evolutionItem arg.
    case EVO_MODE_BATTLE_SPECIAL:
        for (i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
        {
            if (SanitizeSpeciesId(evolutions[i].targetSpecies) == SPECIES_NONE)
                continue;

            switch (evolutions[i].method)
            {
            case EVO_CRITICAL_HITS:
                if (gPartyCriticalHits[evolutionItem] >= evolutions[i].param)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            }
        }
        break;
    // Overworld evolution without leveling; evolution method is being passed into the evolutionItem arg.
    case EVO_MODE_OVERWORLD_SPECIAL:
        for (i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
        {
            if (SanitizeSpeciesId(evolutions[i].targetSpecies) == SPECIES_NONE)
                continue;

            switch (evolutions[i].method)
            {
            case EVO_SCRIPT_TRIGGER_DMG:
            {
                u16 currentHp = GetMonData(mon, MON_DATA_HP, NULL);
                if (evolutionItem == EVO_SCRIPT_TRIGGER_DMG
                    && currentHp != 0
                    && (GetMonData(mon, MON_DATA_MAX_HP, NULL) - currentHp >= evolutions[i].param))
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            }
            case EVO_DARK_SCROLL:
                if (evolutionItem == EVO_DARK_SCROLL)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            case EVO_WATER_SCROLL:
                if (evolutionItem == EVO_WATER_SCROLL)
                    targetSpecies = evolutions[i].targetSpecies;
                break;
            }
        }
        break;
    }

    // Pikachu, Meowth, and Eevee cannot evolve if they have the
    // Gigantamax Factor. We assume that is because their evolutions
    // do not have a Gigantamax Form.
    if (GetMonData(mon, MON_DATA_GIGANTAMAX_FACTOR, NULL)
     && GetGMaxTargetSpecies(species) != SPECIES_NONE
     && GetGMaxTargetSpecies(targetSpecies) == SPECIES_NONE)
    {
        return SPECIES_NONE;
    }

    if (consumeItem)
    {
        heldItem = ITEM_NONE;
        SetMonData(mon, MON_DATA_HELD_ITEM, &heldItem);
    }

    return targetSpecies;
}

bool8 IsMonPastEvolutionLevel(struct Pokemon *mon)
{
    int i;
    u16 species = GetMonData(mon, MON_DATA_SPECIES, 0);
    u8 level = GetMonData(mon, MON_DATA_LEVEL, 0);
    const struct Evolution *evolutions = GetSpeciesEvolutions(species);

    if (evolutions == NULL)
        return FALSE;

    for (i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
    {
        if (SanitizeSpeciesId(evolutions[i].targetSpecies) == SPECIES_NONE)
            continue;

        switch (evolutions[i].method)
        {
        case EVO_LEVEL:
            if (evolutions[i].param <= level)
                return TRUE;
            break;
        }
    }

    return FALSE;
}

u16 NationalPokedexNumToSpecies(u16 nationalNum)
{
    u16 species;

    if (!nationalNum)
        return 0;

    species = 1;

    while (species < (NUM_SPECIES) && gSpeciesInfo[species].natDexNum != nationalNum)
        species++;

    if (species == NUM_SPECIES)
        return DEX_NONE;

    return GET_BASE_SPECIES_ID(species);
}

u16 SpeciesToNationalPokedexNum(u16 species)
{
    species = SanitizeSpeciesId(species);
    if (!species)
        return DEX_NONE;

    return gSpeciesInfo[species].natDexNum;
}

void EvolutionRenameMon(struct Pokemon *mon, u16 oldSpecies, u16 newSpecies)
{

}

// The below two functions determine which side of a multi battle the trainer battles on
// 0 is the left (top in  party menu), 1 is right (bottom in party menu)
u8 GetPlayerFlankId(void)
{
    u8 flankId = 0;
    switch (gLinkPlayers[GetMultiplayerId()].id)
    {
    case 0:
    case 3:
        flankId = 0;
        break;
    case 1:
    case 2:
        flankId = 1;
        break;
    }
    return flankId;
}

u16 GetLinkTrainerFlankId(u8 linkPlayerId)
{
    u16 flankId = 0;
    switch (gLinkPlayers[linkPlayerId].id)
    {
    case 0:
    case 3:
        flankId = 0;
        break;
    case 1:
    case 2:
        flankId = 1;
        break;
    }
    return flankId;
}

s32 GetBattlerMultiplayerId(u16 id)
{
    s32 multiplayerId;
    for (multiplayerId = 0; multiplayerId < MAX_LINK_PLAYERS; multiplayerId++)
        if (gLinkPlayers[multiplayerId].id == id)
            break;
    return multiplayerId;
}

u8 GetTrainerEncounterMusicId(u16 trainerOpponentId)
{
    if (InBattlePyramid())
        return GetTrainerEncounterMusicIdInBattlePyramid(trainerOpponentId);
    else if (InTrainerHillChallenge())
        return GetTrainerEncounterMusicIdInTrainerHill(trainerOpponentId);
    else
        return gTrainers[SanitizeTrainerId(trainerOpponentId)].encounterMusic_gender & (F_TRAINER_FEMALE - 1);
}

u16 ModifyStatByNature(u8 nature, u16 stat, u8 statIndex)
{
    // Don't modify HP, Accuracy, or Evasion by nature
    if (statIndex <= STAT_HP || statIndex > NUM_NATURE_STATS)
        return stat;
    else if (statIndex == gNaturesInfo[nature].statUp)
        return stat * 110 / 100;
    else
        return stat;
}

void AdjustFriendship(struct Pokemon *mon, u8 event)
{
    u16 species, heldItem;
    u8 holdEffect;
    s8 mod;

    if (ShouldSkipFriendshipChange())
        return;

    species = GetMonData(mon, MON_DATA_SPECIES_OR_EGG, 0);
    heldItem = GetMonData(mon, MON_DATA_HELD_ITEM, 0);

    holdEffect = ItemId_GetHoldEffect(heldItem);

    if (species && species != SPECIES_EGG)
    {
        u8 friendshipLevel = 0;
        s16 friendship = GetMonData(mon, MON_DATA_FRIENDSHIP, 0);
        u32 opponentTrainerClass = GetTrainerClassFromId(gTrainerBattleOpponent_A);

        if (friendship > 99)
            friendshipLevel++;
        if (friendship > 199)
            friendshipLevel++;

        if (event == FRIENDSHIP_EVENT_WALKING)
        {
            // 50% chance every 128 steps
            if (Random() & 1)
                return;
        }
        if (event == FRIENDSHIP_EVENT_LEAGUE_BATTLE)
        {
            // Only if it's a trainer battle with league progression significance
            if (!(gBattleTypeFlags & BATTLE_TYPE_TRAINER))
                return;
            if (!(opponentTrainerClass == TRAINER_CLASS_LEADER
                || opponentTrainerClass == TRAINER_CLASS_ELITE_FOUR
                || opponentTrainerClass == TRAINER_CLASS_CHAMPION))
                return;
        }

        mod = sFriendshipEventModifiers[event][friendshipLevel];
        if (mod > 0 && holdEffect == HOLD_EFFECT_FRIENDSHIP_UP)
            // 50% increase, rounding down
            mod = (150 * mod) / 100;

        friendship += mod;
        if (mod > 0)
        {
            if (GetMonData(mon, MON_DATA_POKEBALL, NULL) == ITEM_LUXURY_BALL)
                friendship++;
            if (GetMonData(mon, MON_DATA_MET_LOCATION, NULL) == GetCurrentRegionMapSectionId())
                friendship++;
        }

        if (friendship < 0)
            friendship = 0;
        if (friendship > MAX_FRIENDSHIP)
            friendship = MAX_FRIENDSHIP;

        SetMonData(mon, MON_DATA_FRIENDSHIP, &friendship);
    }
}

void MonGainEVs(struct Pokemon *mon, u16 defeatedSpecies)
{
    u8 evs[NUM_STATS];
    u16 evIncrease = 0;
    u16 totalEVs = 0;
    u16 heldItem;
    u8 holdEffect;
    int i, multiplier;
    u8 stat;
    u8 bonus;
    u32 currentEVCap = GetCurrentEVCap();

    heldItem = GetMonData(mon, MON_DATA_HELD_ITEM, 0);
    holdEffect = ItemId_GetHoldEffect(heldItem);

    stat = ItemId_GetSecondaryId(heldItem);
    bonus = ItemId_GetHoldEffectParam(heldItem);

    for (i = 0; i < NUM_STATS; i++)
    {
        evs[i] = GetMonData(mon, MON_DATA_HP_EV + i, 0);
        totalEVs += evs[i];
    }

    for (i = 0; i < NUM_STATS; i++)
    {
        if (totalEVs >= currentEVCap)
            break;

        if (CheckPartyHasHadPokerus(mon, 0))
            multiplier = 2;
        else
            multiplier = 1;

        switch (i)
        {
        case STAT_HP:
            if (holdEffect == HOLD_EFFECT_POWER_ITEM && stat == STAT_HP)
                evIncrease = (gSpeciesInfo[defeatedSpecies].evYield_HP + bonus) * multiplier;
            else
                evIncrease = gSpeciesInfo[defeatedSpecies].evYield_HP * multiplier;
            break;
        case STAT_ATK:
            if (holdEffect == HOLD_EFFECT_POWER_ITEM && stat == STAT_ATK)
                evIncrease = (gSpeciesInfo[defeatedSpecies].evYield_Attack + bonus) * multiplier;
            else
                evIncrease = gSpeciesInfo[defeatedSpecies].evYield_Attack * multiplier;
            break;
        case STAT_DEF:
            if (holdEffect == HOLD_EFFECT_POWER_ITEM && stat == STAT_DEF)
                evIncrease = (gSpeciesInfo[defeatedSpecies].evYield_Defense + bonus) * multiplier;
            else
                evIncrease = gSpeciesInfo[defeatedSpecies].evYield_Defense * multiplier;
            break;
        case STAT_SPEED:
            if (holdEffect == HOLD_EFFECT_POWER_ITEM && stat == STAT_SPEED)
                evIncrease = (gSpeciesInfo[defeatedSpecies].evYield_Speed + bonus) * multiplier;
            else
                evIncrease = gSpeciesInfo[defeatedSpecies].evYield_Speed * multiplier;
            break;
        case STAT_SPATK:
            if (holdEffect == HOLD_EFFECT_POWER_ITEM && stat == STAT_SPATK)
                evIncrease = (gSpeciesInfo[defeatedSpecies].evYield_SpAttack + bonus) * multiplier;
            else
                evIncrease = gSpeciesInfo[defeatedSpecies].evYield_SpAttack * multiplier;
            break;
        case STAT_SPDEF:
            if (holdEffect == HOLD_EFFECT_POWER_ITEM && stat == STAT_SPDEF)
                evIncrease = (gSpeciesInfo[defeatedSpecies].evYield_SpDefense + bonus) * multiplier;
            else
                evIncrease = gSpeciesInfo[defeatedSpecies].evYield_SpDefense * multiplier;
            break;
        }

        if (holdEffect == HOLD_EFFECT_MACHO_BRACE)
            evIncrease *= 2;

        if (totalEVs + (s16)evIncrease > currentEVCap)
            evIncrease = ((s16)evIncrease + currentEVCap) - (totalEVs + evIncrease);

        if (evs[i] + (s16)evIncrease > MAX_PER_STAT_EVS)
        {
            int val1 = (s16)evIncrease + MAX_PER_STAT_EVS;
            int val2 = evs[i] + evIncrease;
            evIncrease = val1 - val2;
        }

        evs[i] += evIncrease;
        totalEVs += evIncrease;
        SetMonData(mon, MON_DATA_HP_EV + i, &evs[i]);
    }
}

u16 GetMonEVCount(struct Pokemon *mon)
{
    int i;
    u16 count = 0;

    for (i = 0; i < NUM_STATS; i++)
        count += GetMonData(mon, MON_DATA_HP_EV + i, 0);

    return count;
}

void RandomlyGivePartyPokerus(struct Pokemon *party)
{
    u16 rnd = Random();
    if (rnd == 0x4000 || rnd == 0x8000 || rnd == 0xC000)
    {
        struct Pokemon *mon;

        do
        {
            rnd = Random() % PARTY_SIZE;
            mon = &party[rnd];
        }
        while (!GetMonData(mon, MON_DATA_SPECIES, 0) || GetMonData(mon, MON_DATA_IS_EGG, 0));

        if (!(CheckPartyHasHadPokerus(party, 1u << rnd)))
        {
            u8 rnd2;

            do
            {
                rnd2 = Random();
            }
            while ((rnd2 & 0x7) == 0);

            if (rnd2 & 0xF0)
                rnd2 &= 0x7;

            rnd2 |= (rnd2 << 4);
            rnd2 &= 0xF3;
            rnd2++;

            SetMonData(&party[rnd], MON_DATA_POKERUS, &rnd2);
        }
    }
}

u8 CheckPartyPokerus(struct Pokemon *party, u8 selection)
{
    u8 retVal;

    int partyIndex = 0;
    unsigned curBit = 1;
    retVal = 0;

    if (selection)
    {
        do
        {
            if ((selection & 1) && (GetMonData(&party[partyIndex], MON_DATA_POKERUS, 0) & 0xF))
                retVal |= curBit;
            partyIndex++;
            curBit <<= 1;
            selection >>= 1;
        }
        while (selection);
    }
    else if (GetMonData(&party[0], MON_DATA_POKERUS, 0) & 0xF)
    {
        retVal = 1;
    }

    return retVal;
}

u8 CheckPartyHasHadPokerus(struct Pokemon *party, u8 selection)
{
    u8 retVal;

    int partyIndex = 0;
    unsigned curBit = 1;
    retVal = 0;

    if (selection)
    {
        do
        {
            if ((selection & 1) && GetMonData(&party[partyIndex], MON_DATA_POKERUS, 0))
                retVal |= curBit;
            partyIndex++;
            curBit <<= 1;
            selection >>= 1;
        }
        while (selection);
    }
    else if (GetMonData(&party[0], MON_DATA_POKERUS, 0))
    {
        retVal = 1;
    }

    return retVal;
}

void UpdatePartyPokerusTime(u16 days)
{
    int i;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES, 0))
        {
            u8 pokerus = GetMonData(&gPlayerParty[i], MON_DATA_POKERUS, 0);
            if (pokerus & 0xF)
            {
                if ((pokerus & 0xF) < days || days > 4)
                    pokerus &= 0xF0;
                else
                    pokerus -= days;

                if (pokerus == 0)
                    pokerus = 0x10;

                SetMonData(&gPlayerParty[i], MON_DATA_POKERUS, &pokerus);
            }
        }
    }
}

void PartySpreadPokerus(struct Pokemon *party)
{
    if ((Random() % 3) == 0)
    {
        int i;
        for (i = 0; i < PARTY_SIZE; i++)
        {
            if (GetMonData(&party[i], MON_DATA_SPECIES, 0))
            {
                u8 pokerus = GetMonData(&party[i], MON_DATA_POKERUS, 0);
                u8 curPokerus = pokerus;
                if (pokerus)
                {
                    if (pokerus & 0xF)
                    {
                        // Spread to adjacent party members.
                        if (i != 0 && !(GetMonData(&party[i - 1], MON_DATA_POKERUS, 0) & 0xF0))
                            SetMonData(&party[i - 1], MON_DATA_POKERUS, &curPokerus);
                        if (i != (PARTY_SIZE - 1) && !(GetMonData(&party[i + 1], MON_DATA_POKERUS, 0) & 0xF0))
                        {
                            SetMonData(&party[i + 1], MON_DATA_POKERUS, &curPokerus);
                            i++;
                        }
                    }
                }
            }
        }
    }
}

bool8 TryIncrementMonLevel(struct Pokemon *mon)
{
    u16 species = GetMonData(mon, MON_DATA_SPECIES, 0);
    u8 nextLevel = GetMonData(mon, MON_DATA_LEVEL, 0) + 1;
    u32 expPoints = GetMonData(mon, MON_DATA_EXP, 0);
    if (expPoints > gExperienceTables[gSpeciesInfo[species].growthRate][MAX_LEVEL])
    {
        expPoints = gExperienceTables[gSpeciesInfo[species].growthRate][MAX_LEVEL];
        SetMonData(mon, MON_DATA_EXP, &expPoints);
    }
    if (nextLevel > GetCurrentLevelCap() || expPoints < gExperienceTables[gSpeciesInfo[species].growthRate][nextLevel])
    {
        return FALSE;
    }
    else
    {
        SetMonData(mon, MON_DATA_LEVEL, &nextLevel);
        return TRUE;
    }
}

static const u16 sUniversalMoves[] =
{
    MOVE_BIDE,
    MOVE_FRUSTRATION,
    MOVE_HIDDEN_POWER,
    MOVE_MIMIC,
    MOVE_NATURAL_GIFT,
    MOVE_RAGE,
    MOVE_RETURN,
    MOVE_SECRET_POWER,
    MOVE_SUBSTITUTE,
    MOVE_TERA_BLAST,
};

u8 CanLearnTeachableMove(u16 species, u16 move)
{
    if (species == SPECIES_EGG)
    {
        return FALSE;
    }
    else if (species == SPECIES_MEW)
    {
        switch (move)
        {
        case MOVE_BADDY_BAD:
        case MOVE_BOUNCY_BUBBLE:
        case MOVE_BUZZY_BUZZ:
        case MOVE_DRAGON_ASCENT:
        case MOVE_FLOATY_FALL:
        case MOVE_FREEZY_FROST:
        case MOVE_GLITZY_GLOW:
        case MOVE_RELIC_SONG:
        case MOVE_SAPPY_SEED:
        case MOVE_SECRET_SWORD:
        case MOVE_SIZZLY_SLIDE:
        case MOVE_SPARKLY_SWIRL:
        case MOVE_SPLISHY_SPLASH:
        case MOVE_VOLT_TACKLE:
        case MOVE_ZIPPY_ZAP:
            return FALSE;
        default:
            return TRUE;
        }
    }
    else
    {
        u32 i, j;
        const u16 *teachableLearnset = GetSpeciesTeachableLearnset(species);
        for (i = 0; i < ARRAY_COUNT(sUniversalMoves); i++)
        {
            if (sUniversalMoves[i] == move)
            {
                const struct LevelUpMove *learnset = GetSpeciesLevelUpLearnset(species);

                if (P_TM_LITERACY < GEN_6)
                    return FALSE;

                for (j = 0; j < MAX_LEVEL_UP_MOVES && learnset[j].move != LEVEL_UP_MOVE_END; j++)
                {
                    if (learnset[j].move == move)
                        return TRUE;
                }
                return FALSE;
            }
        }
        for (i = 0; teachableLearnset[i] != MOVE_UNAVAILABLE; i++)
        {
            if (teachableLearnset[i] == move)
                return TRUE;
        }
        return FALSE;
    }
}

u8 GetMoveRelearnerMoves(struct Pokemon *mon, u16 *moves)
{
    u16 learnedMoves[4];
    u8 numMoves = 0;
    u16 species = GetMonData(mon, MON_DATA_SPECIES, 0);
    u8 level = GetMonData(mon, MON_DATA_LEVEL, 0);
    const struct LevelUpMove *learnset = GetSpeciesLevelUpLearnset(species);
    int i, j, k;

    for (i = 0; i < MAX_MON_MOVES; i++)
        learnedMoves[i] = GetMonData(mon, MON_DATA_MOVE1 + i, 0);

    for (i = 0; i < MAX_LEVEL_UP_MOVES; i++)
    {
        u16 moveLevel;

        if (learnset[i].move == LEVEL_UP_MOVE_END)
            break;

        moveLevel = learnset[i].level;

        if (moveLevel <= level)
        {
            for (j = 0; j < MAX_MON_MOVES && learnedMoves[j] != learnset[i].move; j++)
                ;

            if (j == MAX_MON_MOVES)
            {
                for (k = 0; k < numMoves && moves[k] != learnset[i].move; k++)
                    ;

                if (k == numMoves)
                    moves[numMoves++] = learnset[i].move;
            }
        }
    }

    return numMoves;
}

u8 GetLevelUpMovesBySpecies(u16 species, u16 *moves)
{
    u8 numMoves = 0;
    int i;
    const struct LevelUpMove *learnset = GetSpeciesLevelUpLearnset(species);

    for (i = 0; i < MAX_LEVEL_UP_MOVES && learnset[i].move != LEVEL_UP_MOVE_END; i++)
         moves[numMoves++] = learnset[i].move;

     return numMoves;
}

u8 GetNumberOfRelearnableMoves(struct Pokemon *mon)
{
    u16 learnedMoves[MAX_MON_MOVES];
    u16 moves[MAX_LEVEL_UP_MOVES];
    u8 numMoves = 0;
    u16 species = GetMonData(mon, MON_DATA_SPECIES_OR_EGG, 0);
    u8 level = GetMonData(mon, MON_DATA_LEVEL, 0);
    const struct LevelUpMove *learnset = GetSpeciesLevelUpLearnset(species);
    int i, j, k;

    if (species == SPECIES_EGG)
        return 0;

    for (i = 0; i < MAX_MON_MOVES; i++)
        learnedMoves[i] = GetMonData(mon, MON_DATA_MOVE1 + i, 0);

    for (i = 0; i < MAX_LEVEL_UP_MOVES; i++)
    {
        u16 moveLevel;

        if (learnset[i].move == LEVEL_UP_MOVE_END)
            break;

        moveLevel = learnset[i].level;

        if (moveLevel <= level)
        {
            for (j = 0; j < MAX_MON_MOVES && learnedMoves[j] != learnset[i].move; j++)
                ;

            if (j == MAX_MON_MOVES)
            {
                for (k = 0; k < numMoves && moves[k] != learnset[i].move; k++)
                    ;

                if (k == numMoves)
                    moves[numMoves++] = learnset[i].move;
            }
        }
    }

    return numMoves;
}

u16 SpeciesToPokedexNum(u16 species)
{
    return SpeciesToNationalPokedexNum(species);
}

u16 GetBattleBGM(void)
{
    if (gBattleTypeFlags & BATTLE_TYPE_LEGENDARY)
    {
        switch (GetMonData(&gEnemyParty[0], MON_DATA_SPECIES, NULL))
        {
        case SPECIES_RAYQUAZA:
            return MUS_VS_RAYQUAZA;
        case SPECIES_KYOGRE:
        case SPECIES_GROUDON:
            return MUS_VS_KYOGRE_GROUDON;
        case SPECIES_REGIROCK:
        case SPECIES_REGICE:
        case SPECIES_REGISTEEL:
        case SPECIES_REGIGIGAS:
        case SPECIES_REGIELEKI:
        case SPECIES_REGIDRAGO:
            return MUS_VS_REGI;
        default:
            return MUS_RG_VS_LEGEND;
        }
    }
    else if (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED_LINK))
        return MUS_VS_TRAINER;
    else if (gBattleTypeFlags & BATTLE_TYPE_TRAINER)
    {
        u8 trainerClass;

        if (gBattleTypeFlags & BATTLE_TYPE_FRONTIER)
            trainerClass = GetFrontierOpponentClass(gTrainerBattleOpponent_A);
        else if (gBattleTypeFlags & BATTLE_TYPE_TRAINER_HILL)
            trainerClass = TRAINER_CLASS_EXPERT;
        else
            trainerClass = GetTrainerClassFromId(gTrainerBattleOpponent_A);

        switch (trainerClass)
        {
        case TRAINER_CLASS_AQUA_LEADER:
        case TRAINER_CLASS_MAGMA_LEADER:
            return MUS_VS_AQUA_MAGMA_LEADER;
        case TRAINER_CLASS_TEAM_AQUA:
        case TRAINER_CLASS_TEAM_MAGMA:
        case TRAINER_CLASS_AQUA_ADMIN:
        case TRAINER_CLASS_MAGMA_ADMIN:
            return MUS_VS_AQUA_MAGMA;
        case TRAINER_CLASS_LEADER:
            return MUS_VS_GYM_LEADER;
        case TRAINER_CLASS_CHAMPION:
            return MUS_VS_CHAMPION;
        case TRAINER_CLASS_RIVAL:
            if (gBattleTypeFlags & BATTLE_TYPE_FRONTIER)
                return MUS_VS_RIVAL;
            if (!StringCompare(GetTrainerNameFromId(gTrainerBattleOpponent_A), gText_BattleWallyName))
                return MUS_VS_TRAINER;
            return MUS_VS_RIVAL;
        case TRAINER_CLASS_ELITE_FOUR:
            return MUS_VS_ELITE_FOUR;
        case TRAINER_CLASS_SALON_MAIDEN:
        case TRAINER_CLASS_DOME_ACE:
        case TRAINER_CLASS_PALACE_MAVEN:
        case TRAINER_CLASS_ARENA_TYCOON:
        case TRAINER_CLASS_FACTORY_HEAD:
        case TRAINER_CLASS_PIKE_QUEEN:
        case TRAINER_CLASS_PYRAMID_KING:
            return MUS_VS_FRONTIER_BRAIN;
        default:
            return MUS_VS_TRAINER;
        }
    }
    else
    {
        switch (GetMonData(&gEnemyParty[0], MON_DATA_SPECIES, NULL))
        {
        case SPECIES_ARTICUNO:
        case SPECIES_ZAPDOS:
        case SPECIES_MOLTRES:
            return MUS_RG_VS_LEGEND;
        case SPECIES_MEWTWO:
            return MUS_RG_VS_MEWTWO;
        case SPECIES_MEW:
            return MUS_VS_MEW;
        case SPECIES_RAIKOU:
            return MUS_HG_VS_RAIKOU;
        case SPECIES_ENTEI:
            return MUS_HG_VS_ENTEI;
        case SPECIES_SUICUNE:
            return MUS_HG_VS_SUICUNE;
        case SPECIES_LUGIA:
            return MUS_HG_VS_LUGIA;
        case SPECIES_HO_OH:
            return MUS_HG_VS_HO_OH;
        case SPECIES_CELEBI:
            return MUS_HG_VS_WILD;
        case SPECIES_REGIROCK:
        case SPECIES_REGICE:
        case SPECIES_REGISTEEL:
        case SPECIES_REGIGIGAS:
        case SPECIES_REGIELEKI:
        case SPECIES_REGIDRAGO:
            return MUS_VS_REGI;
        case SPECIES_LATIAS:
        case SPECIES_LATIOS:
            return MUS_VS_WILD;
        case SPECIES_GROUDON:
        case SPECIES_KYOGRE:
        case SPECIES_RAYQUAZA:
            return MUS_VS_KYOGRE_GROUDON;
        case SPECIES_JIRACHI:
            return MUS_VS_WILD;
        case SPECIES_DEOXYS_NORMAL:
        case SPECIES_DEOXYS_ATTACK:
        case SPECIES_DEOXYS_DEFENSE:
        case SPECIES_DEOXYS_SPEED:
            return MUS_RG_VS_DEOXYS;
        case SPECIES_ROTOM:
        case SPECIES_ROTOM_HEAT:
        case SPECIES_ROTOM_WASH:
        case SPECIES_ROTOM_FROST:
        case SPECIES_ROTOM_FAN:
        case SPECIES_ROTOM_MOW:
        case SPECIES_HEATRAN:
            return MUS_DP_VS_LEGEND;
        default:
            return MUS_VS_WILD;
        }
    }
}

void PlayBattleBGM(void)
{
    ResetMapMusic();
    m4aMPlayAllStop();
    PlayBGM(GetBattleBGM());
}

void PlayMapChosenOrBattleBGM(u16 songId)
{
    ResetMapMusic();
    m4aMPlayAllStop();
    if (songId)
        PlayNewMapMusic(songId);
    else
        PlayNewMapMusic(GetBattleBGM());
}

// Identical to PlayMapChosenOrBattleBGM, but uses a task instead
// Only used by Battle Dome
#define tSongId data[0]
void CreateTask_PlayMapChosenOrBattleBGM(u16 songId)
{
    u8 taskId;

    ResetMapMusic();
    m4aMPlayAllStop();

    taskId = CreateTask(Task_PlayMapChosenOrBattleBGM, 0);
    gTasks[taskId].tSongId = songId;
}

static void Task_PlayMapChosenOrBattleBGM(u8 taskId)
{
    if (gTasks[taskId].tSongId)
        PlayNewMapMusic(gTasks[taskId].tSongId);
    else
        PlayNewMapMusic(GetBattleBGM());
    DestroyTask(taskId);
}

#undef tSongId

const u32 *GetMonFrontSpritePal(struct Pokemon *mon)
{
    u16 species = GetMonData(mon, MON_DATA_SPECIES, NULL);
    bool32 isShiny = GetMonData(mon, MON_DATA_IS_SHINY, NULL);
    u32 personality = GetMonData(mon, MON_DATA_PERSONALITY, NULL);
    return GetMonSpritePalFromSpeciesAndPersonality(species, isShiny, personality);
}

const u32 *GetMonSpritePalFromSpeciesAndPersonality(u16 species, bool32 isShiny, u32 personality)
{
    return GetMonSpritePalFromSpecies(species, isShiny, IsPersonalityFemale(species, personality));
}

const u32 *GetMonSpritePalFromSpecies(u16 species, bool32 isShiny, bool32 isFemale)
{
    species = SanitizeSpeciesId(species);

    if (isShiny)
    {
        if (gSpeciesInfo[species].shinyPaletteFemale != NULL && isFemale)
            return gSpeciesInfo[species].shinyPaletteFemale;
        else if (gSpeciesInfo[species].shinyPalette != NULL)
            return gSpeciesInfo[species].shinyPalette;
        else
            return gSpeciesInfo[SPECIES_NONE].shinyPalette;
    }
    else
    {
        if (gSpeciesInfo[species].paletteFemale != NULL && isFemale)
            return gSpeciesInfo[species].paletteFemale;
        else if (gSpeciesInfo[species].palette != NULL)
            return gSpeciesInfo[species].palette;
        else
            return gSpeciesInfo[SPECIES_NONE].palette;
    }
}

bool8 IsMonSpriteNotFlipped(u16 species)
{
    return gSpeciesInfo[species].noFlip;
}

s8 GetMonFlavorRelation(struct Pokemon *mon, u8 flavor)
{
    u8 nature = GetNature(mon);
    return gPokeblockFlavorCompatibilityTable[nature * FLAVOR_COUNT + flavor];
}

s8 GetFlavorRelationByPersonality(u32 personality, u8 flavor)
{
    u8 nature = GetNatureFromPersonality(personality);
    return gPokeblockFlavorCompatibilityTable[nature * FLAVOR_COUNT + flavor];
}

bool8 IsTradedMon(struct Pokemon *mon)
{
    u8 otName[PLAYER_NAME_LENGTH + 1];
    u32 otId;
    GetMonData(mon, MON_DATA_OT_NAME, otName);
    otId = GetMonData(mon, MON_DATA_OT_ID, 0);
    return IsOtherTrainer(otId, otName);
}

bool8 IsOtherTrainer(u32 otId, u8 *otName)
{
    if (otId ==
        (gSaveBlock2Ptr->playerTrainerId[0]
      | (gSaveBlock2Ptr->playerTrainerId[1] << 8)
      | (gSaveBlock2Ptr->playerTrainerId[2] << 16)
      | (gSaveBlock2Ptr->playerTrainerId[3] << 24)))
    {
        int i;
        for (i = 0; otName[i] != EOS; i++)
            if (otName[i] != gSaveBlock2Ptr->playerName[i])
                return TRUE;
        return FALSE;
    }

    return TRUE;
}

void MonRestorePP(struct Pokemon *mon)
{
    BoxMonRestorePP(&mon->box);
}

void BoxMonRestorePP(struct BoxPokemon *boxMon)
{
    int i;

    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        if (GetBoxMonData(boxMon, MON_DATA_MOVE1 + i, 0))
        {
            u16 move = GetBoxMonData(boxMon, MON_DATA_MOVE1 + i, 0);
            u16 bonus = GetBoxMonData(boxMon, MON_DATA_PP_BONUSES, 0);
            u8 pp = CalculatePPWithBonus(move, bonus, i);
            SetBoxMonData(boxMon, MON_DATA_PP1 + i, &pp);
        }
    }
}

void SetMonPreventsSwitchingString(void)
{
    gLastUsedAbility = gBattleStruct->abilityPreventingSwitchout;

    gBattleTextBuff1[0] = B_BUFF_PLACEHOLDER_BEGIN;
    gBattleTextBuff1[1] = B_BUFF_MON_NICK_WITH_PREFIX;
    gBattleTextBuff1[2] = gBattleStruct->battlerPreventingSwitchout;
    gBattleTextBuff1[4] = B_BUFF_EOS;

    if (GetBattlerSide(gBattleStruct->battlerPreventingSwitchout) == B_SIDE_PLAYER)
        gBattleTextBuff1[3] = GetPartyIdFromBattlePartyId(gBattlerPartyIndexes[gBattleStruct->battlerPreventingSwitchout]);
    else
        gBattleTextBuff1[3] = gBattlerPartyIndexes[gBattleStruct->battlerPreventingSwitchout];

    PREPARE_MON_NICK_WITH_PREFIX_BUFFER(gBattleTextBuff2, gBattlerInMenuId, GetPartyIdFromBattlePartyId(gBattlerPartyIndexes[gBattlerInMenuId]))

    BattleStringExpandPlaceholders(gText_PkmnsXPreventsSwitching, gStringVar4);
}

static s32 GetWildMonTableIdInAlteringCave(u16 species)
{
    s32 i;
    for (i = 0; i < (s32) ARRAY_COUNT(sAlteringCaveWildMonHeldItems); i++)
        if (sAlteringCaveWildMonHeldItems[i].species == species)
            return i;
    return 0;
}

static inline bool32 CanFirstMonBoostHeldItemRarity(void)
{
    u32 ability;
    if (GetMonData(&gPlayerParty[0], MON_DATA_SANITY_IS_EGG))
        return FALSE;

    ability = GetMonAbility(&gPlayerParty[0]);
    if (ability == ABILITY_COMPOUND_EYES)
        return TRUE;
    else if ((OW_SUPER_LUCK >= GEN_8) && ability == ABILITY_SUPER_LUCK)
        return TRUE;
    return FALSE;
}

void SetWildMonHeldItem(void)
{
    if (!(gBattleTypeFlags & (BATTLE_TYPE_LEGENDARY | BATTLE_TYPE_TRAINER | BATTLE_TYPE_PYRAMID | BATTLE_TYPE_PIKE)))
    {
        u16 rnd;
        u16 species;
        u16 count = (WILD_DOUBLE_BATTLE) ? 2 : 1;
        u16 i;
        bool32 itemHeldBoost = CanFirstMonBoostHeldItemRarity();
        u16 chanceNoItem = itemHeldBoost ? 20 : 45;
        u16 chanceNotRare = itemHeldBoost ? 80 : 95;

        for (i = 0; i < count; i++)
        {
            if (GetMonData(&gEnemyParty[i], MON_DATA_HELD_ITEM, NULL) != ITEM_NONE)
                continue; // prevent overwriting previously set item

            rnd = Random() % 100;
            species = GetMonData(&gEnemyParty[i], MON_DATA_SPECIES, 0);
            if (gMapHeader.mapLayoutId == LAYOUT_ALTERING_CAVE)
            {
                s32 alteringCaveId = GetWildMonTableIdInAlteringCave(species);
                if (alteringCaveId != 0)
                {
                    // In active Altering Cave, use special item list
                    if (rnd < chanceNotRare)
                        continue;
                    SetMonData(&gEnemyParty[i], MON_DATA_HELD_ITEM, &sAlteringCaveWildMonHeldItems[alteringCaveId].item);
                }
                else
                {
                    // In inactive Altering Cave, use normal items
                    if (rnd < chanceNoItem)
                        continue;
                    if (rnd < chanceNotRare)
                        SetMonData(&gEnemyParty[i], MON_DATA_HELD_ITEM, &gSpeciesInfo[species].itemCommon);
                    else
                        SetMonData(&gEnemyParty[i], MON_DATA_HELD_ITEM, &gSpeciesInfo[species].itemRare);
                }
            }
            else
            {
                if (gSpeciesInfo[species].itemCommon == gSpeciesInfo[species].itemRare && gSpeciesInfo[species].itemCommon != ITEM_NONE)
                {
                    // Both held items are the same, 100% chance to hold item
                    SetMonData(&gEnemyParty[i], MON_DATA_HELD_ITEM, &gSpeciesInfo[species].itemCommon);
                }
                else
                {
                    if (rnd < chanceNoItem)
                        continue;
                    if (rnd < chanceNotRare)
                        SetMonData(&gEnemyParty[i], MON_DATA_HELD_ITEM, &gSpeciesInfo[species].itemCommon);
                    else
                        SetMonData(&gEnemyParty[i], MON_DATA_HELD_ITEM, &gSpeciesInfo[species].itemRare);
                }
            }
        }
    }
}

bool8 IsMonShiny(struct Pokemon *mon)
{
    return GetMonData(mon, MON_DATA_IS_SHINY, NULL);
}

const u8 *GetTrainerPartnerName(void)
{
    if (gBattleTypeFlags & BATTLE_TYPE_INGAME_PARTNER)
    {
        if (gPartnerTrainerId == TRAINER_PARTNER(PARTNER_STEVEN))
        {
            return GetTrainerNameFromId(TRAINER_STEVEN);
        }
        else
        {
            GetFrontierTrainerName(gStringVar1, gPartnerTrainerId);
            return gStringVar1;
        }
    }
    else
    {
        u8 id = GetMultiplayerId();
        return gLinkPlayers[GetBattlerMultiplayerId(gLinkPlayers[id].id ^ 2)].name;
    }
}

#define READ_PTR_FROM_TASK(taskId, dataId)                      \
    (void *)(                                                   \
    ((u16)(gTasks[taskId].data[dataId]) |                       \
    ((u16)(gTasks[taskId].data[dataId + 1]) << 16)))

#define STORE_PTR_IN_TASK(ptr, taskId, dataId)                 \
{                                                              \
    gTasks[taskId].data[dataId] = (u32)(ptr);                  \
    gTasks[taskId].data[dataId + 1] = (u32)(ptr) >> 16;        \
}

#define sAnimId    data[2]
#define sAnimDelay data[3]

static void Task_AnimateAfterDelay(u8 taskId)
{
    if (--gTasks[taskId].sAnimDelay == 0)
    {
        LaunchAnimationTaskForFrontSprite(READ_PTR_FROM_TASK(taskId, 0), gTasks[taskId].sAnimId);
        DestroyTask(taskId);
    }
}

static void Task_PokemonSummaryAnimateAfterDelay(u8 taskId)
{
    if (--gTasks[taskId].sAnimDelay == 0)
    {
        StartMonSummaryAnimation(READ_PTR_FROM_TASK(taskId, 0), gTasks[taskId].sAnimId);
        SummaryScreen_SetAnimDelayTaskId(TASK_NONE);
        DestroyTask(taskId);
    }
}

void BattleAnimateFrontSprite(struct Sprite *sprite, u16 species, bool8 noCry, u8 panMode)
{
    if (gHitMarker & HITMARKER_NO_ANIMATIONS && !(gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED_LINK)))
        DoMonFrontSpriteAnimation(sprite, species, noCry, panMode | SKIP_FRONT_ANIM);
    else
        DoMonFrontSpriteAnimation(sprite, species, noCry, panMode);
}

void DoMonFrontSpriteAnimation(struct Sprite *sprite, u16 species, bool8 noCry, u8 panModeAnimFlag)
{
    s8 pan;
    switch (panModeAnimFlag & (u8)~SKIP_FRONT_ANIM) // Exclude anim flag to get pan mode
    {
    case 0:
        pan = -25;
        break;
    case 1:
        pan = 25;
        break;
    default:
        pan = 0;
        break;
    }
    if (panModeAnimFlag & SKIP_FRONT_ANIM)
    {
        // No animation, only check if cry needs to be played
        if (!noCry)
            PlayCry_Normal(species, pan);
        sprite->callback = SpriteCallbackDummy;
    }
    else
    {
        if (!noCry)
        {
            PlayCry_Normal(species, pan);
            if (HasTwoFramesAnimation(species))
                StartSpriteAnim(sprite, 1);
        }
        if (gSpeciesInfo[species].frontAnimDelay != 0)
        {
            // Animation has delay, start delay task
            u8 taskId = CreateTask(Task_AnimateAfterDelay, 0);
            STORE_PTR_IN_TASK(sprite, taskId, 0);
            gTasks[taskId].sAnimId = gSpeciesInfo[species].frontAnimId;
            gTasks[taskId].sAnimDelay = gSpeciesInfo[species].frontAnimDelay;
        }
        else
        {
            // No delay, start animation
            LaunchAnimationTaskForFrontSprite(sprite, gSpeciesInfo[species].frontAnimId);
        }
        sprite->callback = SpriteCallbackDummy;
    }
}

void PokemonSummaryDoMonAnimation(struct Sprite *sprite, u16 species, bool8 oneFrame)
{
    if (!oneFrame && HasTwoFramesAnimation(species))
        StartSpriteAnim(sprite, 1);
    if (gSpeciesInfo[species].frontAnimDelay != 0)
    {
        // Animation has delay, start delay task
        u8 taskId = CreateTask(Task_PokemonSummaryAnimateAfterDelay, 0);
        STORE_PTR_IN_TASK(sprite, taskId, 0);
        gTasks[taskId].sAnimId = gSpeciesInfo[species].frontAnimId;
        gTasks[taskId].sAnimDelay = gSpeciesInfo[species].frontAnimDelay;
        SummaryScreen_SetAnimDelayTaskId(taskId);
        SetSpriteCB_MonAnimDummy(sprite);
    }
    else
    {
        // No delay, start animation
        StartMonSummaryAnimation(sprite, gSpeciesInfo[species].frontAnimId);
    }
}

void StopPokemonAnimationDelayTask(void)
{
    u8 delayTaskId = FindTaskIdByFunc(Task_PokemonSummaryAnimateAfterDelay);
    if (delayTaskId != TASK_NONE)
        DestroyTask(delayTaskId);
}

void BattleAnimateBackSprite(struct Sprite *sprite, u16 species)
{
    if (gHitMarker & HITMARKER_NO_ANIMATIONS && !(gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED_LINK)))
    {
        sprite->callback = SpriteCallbackDummy;
    }
    else
    {
        LaunchAnimationTaskForBackSprite(sprite, GetSpeciesBackAnimSet(species));
        sprite->callback = SpriteCallbackDummy;
    }
}

u8 GetOpposingLinkMultiBattlerId(bool8 rightSide, u8 multiplayerId)
{
    s32 i;
    s32 battlerId = 0;
    switch (gLinkPlayers[multiplayerId].id)
    {
    case 0:
    case 2:
        battlerId = rightSide ? 1 : 3;
        break;
    case 1:
    case 3:
        battlerId = rightSide ? 2 : 0;
        break;
    }
    for (i = 0; i < MAX_LINK_PLAYERS; i++)
    {
        if (gLinkPlayers[i].id == (s16)battlerId)
            break;
    }
    return i;
}

u16 FacilityClassToPicIndex(u16 facilityClass)
{
    return gFacilityClassToPicIndex[facilityClass];
}

u16 PlayerGenderToFrontTrainerPicId(u8 playerGender)
{
    if (playerGender != MALE)
        return FacilityClassToPicIndex(FACILITY_CLASS_MAY);
    else
        return FacilityClassToPicIndex(FACILITY_CLASS_BRENDAN);
}

void HandleSetPokedexFlag(u16 nationalNum, u8 caseId)
{
    u8 getFlagCaseId = (caseId == FLAG_SET_SEEN) ? FLAG_GET_SEEN : FLAG_GET_CAUGHT;
    if (!GetSetPokedexFlag(nationalNum, getFlagCaseId)) // don't set if it's already set
    {
        GetSetPokedexFlag(nationalNum, caseId);
    }
}

bool8 HasTwoFramesAnimation(u16 species)
{
    return P_TWO_FRAME_FRONT_SPRITES;
}

static bool8 ShouldSkipFriendshipChange(void)
{
    if (gMain.inBattle && gBattleTypeFlags & (BATTLE_TYPE_FRONTIER))
        return TRUE;
    if (!gMain.inBattle && (InBattlePike() || InBattlePyramid()))
        return TRUE;
    return FALSE;
}

#define ALLOC_FAIL_BUFFER (1 << 0)
#define ALLOC_FAIL_STRUCT (1 << 1)

struct MonSpritesGfxManager *CreateMonSpritesGfxManager(void)
{
    u32 i, j;
    u8 failureFlags;
    struct MonSpritesGfxManager *gfx;

    failureFlags = 0;
    gfx = AllocZeroed(sizeof(*gfx));
    if (gfx == NULL)
        return NULL;

    // Set up sprite / sprite pointer buffers
    gfx->spriteBuffer = AllocZeroed(MON_PIC_SIZE * MAX_MON_PIC_FRAMES * MAX_BATTLERS_COUNT);
    gfx->spritePointers = AllocZeroed(MAX_BATTLERS_COUNT * 32);
    if (gfx->spriteBuffer == NULL || gfx->spritePointers == NULL)
    {
        failureFlags |= ALLOC_FAIL_BUFFER;
    }
    else
    {
        for (i = 0; i < MAX_BATTLERS_COUNT; i++)
            gfx->spritePointers[i] = gfx->spriteBuffer + (MON_PIC_SIZE * MAX_MON_PIC_FRAMES * i);
    }

    // Set up sprite structs
    gfx->templates = AllocZeroed(sizeof(struct SpriteTemplate) * MAX_BATTLERS_COUNT);
    gfx->frameImages = AllocZeroed(sizeof(struct SpriteFrameImage) * MAX_BATTLERS_COUNT * MAX_MON_PIC_FRAMES);
    if (gfx->templates == NULL || gfx->frameImages == NULL)
    {
        failureFlags |= ALLOC_FAIL_STRUCT;
    }
    else
    {
        for (i = 0; i < MAX_MON_PIC_FRAMES * MAX_BATTLERS_COUNT; i++)
            gfx->frameImages[i].size = MON_PIC_SIZE;

        for (i = 0; i < MAX_BATTLERS_COUNT; i++)
        {
            gfx->templates[i] = gBattlerSpriteTemplates[i];
            for (j = 0; j < MAX_MON_PIC_FRAMES; j++)
                gfx->frameImages[i * MAX_MON_PIC_FRAMES + j].data = &gfx->spritePointers[i][j * MON_PIC_SIZE];

            gfx->templates[i].images = &gfx->frameImages[i * MAX_MON_PIC_FRAMES];
        }
    }

    // If either of the allocations failed free their respective members
    if (failureFlags & ALLOC_FAIL_STRUCT)
    {
        TRY_FREE_AND_SET_NULL(gfx->frameImages);
        TRY_FREE_AND_SET_NULL(gfx->templates);
    }
    if (failureFlags & ALLOC_FAIL_BUFFER)
    {
        TRY_FREE_AND_SET_NULL(gfx->spritePointers);
        TRY_FREE_AND_SET_NULL(gfx->spriteBuffer);
    }

    if (failureFlags)
    {
        // Clear, something failed to allocate
        memset(gfx, 0, sizeof(*gfx));
        Free(gfx);
    }
    else
    {
        gfx->active = TRUE;
        sMonSpritesGfxManager = gfx;
    }

    return sMonSpritesGfxManager;
}

void DestroyMonSpritesGfxManager(void)
{
    struct MonSpritesGfxManager *gfx;

    gfx = sMonSpritesGfxManager;
    if (gfx == NULL)
        return;

    if (gfx->active == FALSE)
    {
        memset(gfx, 0, sizeof(*gfx));
    }
    else
    {
        TRY_FREE_AND_SET_NULL(gfx->frameImages);
        TRY_FREE_AND_SET_NULL(gfx->templates);
        TRY_FREE_AND_SET_NULL(gfx->spritePointers);
        TRY_FREE_AND_SET_NULL(gfx->spriteBuffer);
        memset(gfx, 0, sizeof(*gfx));
        Free(gfx);
    }
}

u8 *MonSpritesGfxManager_GetSpritePtr(void)
{
    struct MonSpritesGfxManager *gfx = sMonSpritesGfxManager;
    if (gfx->active == FALSE)
    {
        return NULL;
    }
    else
    {
        return gfx->spritePointers[B_POSITION_OPPONENT_LEFT];
    }
}

u16 GetFormSpeciesId(u16 speciesId, u8 formId)
{
    if (GetSpeciesFormTable(speciesId) != NULL)
        return GetSpeciesFormTable(speciesId)[formId];
    else
        return speciesId;
}

u8 GetFormIdFromFormSpeciesId(u16 formSpeciesId)
{
    u8 targetFormId = 0;

    if (GetSpeciesFormTable(formSpeciesId) != NULL)
    {
        for (targetFormId = 0; GetSpeciesFormTable(formSpeciesId)[targetFormId] != FORM_SPECIES_END; targetFormId++)
        {
            if (formSpeciesId == GetSpeciesFormTable(formSpeciesId)[targetFormId])
                break;
        }
    }
    return targetFormId;
}

u16 GetFormChangeTargetSpecies(struct Pokemon *mon, u16 method, u32 arg)
{
    return GetFormChangeTargetSpeciesBoxMon(&mon->box, method, arg);
}

// Returns SPECIES_NONE if no form change is possible
u16 GetFormChangeTargetSpeciesBoxMon(struct BoxPokemon *boxMon, u16 method, u32 arg)
{
    u32 i;
    u16 targetSpecies = SPECIES_NONE;
    u16 species = GetBoxMonData(boxMon, MON_DATA_SPECIES, NULL);
    const struct FormChange *formChanges = GetSpeciesFormChanges(species);
    u16 heldItem;
    u32 ability;

    if (formChanges != NULL)
    {
        heldItem = GetBoxMonData(boxMon, MON_DATA_HELD_ITEM, NULL);
        ability = GetAbilityBySpecies(species, GetBoxMonData(boxMon, MON_DATA_ABILITY_NUM, NULL));

        for (i = 0; formChanges[i].method != FORM_CHANGE_TERMINATOR; i++)
        {
            if (method == formChanges[i].method && species != formChanges[i].targetSpecies)
            {
                switch (method)
                {
                case FORM_CHANGE_ITEM_HOLD:
                    if ((heldItem == formChanges[i].param1 || formChanges[i].param1 == ITEM_NONE)
                     && (ability == formChanges[i].param2 || formChanges[i].param2 == ABILITY_NONE))
                        targetSpecies = formChanges[i].targetSpecies;
                    break;
                case FORM_CHANGE_ITEM_USE:
                    if (arg == formChanges[i].param1)
                    {
                        bool32 pass = TRUE;
                        switch (formChanges[i].param2)
                        {
                        case DAY:
                            if (GetTimeOfDay() == TIEMPO_NOCHE)
                                pass = FALSE;
                            break;
                        case NIGHT:
                            if (GetTimeOfDay() != TIEMPO_NOCHE)
                                pass = FALSE;
                            break;
                        }

                        if (formChanges[i].param3 != STATUS1_NONE && GetBoxMonData(boxMon, MON_DATA_STATUS, NULL) & formChanges[i].param3)
                            pass = FALSE;

                        if (pass)
                            targetSpecies = formChanges[i].targetSpecies;
                    }
                    break;
                case FORM_CHANGE_ITEM_USE_MULTICHOICE:
                    if (arg == formChanges[i].param1)
                    {
                        if (formChanges[i].param2 == gSpecialVar_Result)
                            targetSpecies = formChanges[i].targetSpecies;
                    }
                    break;
                case FORM_CHANGE_MOVE:
                    if (BoxMonKnowsMove(boxMon, formChanges[i].param1) != formChanges[i].param2)
                        targetSpecies = formChanges[i].targetSpecies;
                    break;
                case FORM_CHANGE_BEGIN_BATTLE:
                case FORM_CHANGE_END_BATTLE:
                    if (heldItem == formChanges[i].param1 || formChanges[i].param1 == ITEM_NONE)
                        targetSpecies = formChanges[i].targetSpecies;
                    break;
                case FORM_CHANGE_END_BATTLE_TERRAIN:
                    if (gBattleTerrain == formChanges[i].param1)
                        targetSpecies = formChanges[i].targetSpecies;
                    break;
                case FORM_CHANGE_WITHDRAW:
                case FORM_CHANGE_FAINT:
                case FORM_CHANGE_STATUS:
                    if (GetBoxMonData(boxMon, MON_DATA_STATUS, NULL) & formChanges[i].param1)
                        targetSpecies = formChanges[i].targetSpecies;
                    break;
                case FORM_CHANGE_TIME_OF_DAY:
                    switch (formChanges[i].param1)
                    {
                    case DAY:
                        if (GetTimeOfDay() != TIEMPO_NOCHE)
                            targetSpecies = formChanges[i].targetSpecies;
                        break;
                    case NIGHT:
                        if (GetTimeOfDay() == TIEMPO_NOCHE)
                            targetSpecies = formChanges[i].targetSpecies;
                        break;
                    }
                    break;
                }
            }
        }
    }

    return targetSpecies;
}

bool32 DoesSpeciesHaveFormChangeMethod(u16 species, u16 method)
{
    u32 i;
    const struct FormChange *formChanges = GetSpeciesFormChanges(species);

    if (formChanges != NULL)
    {
        for (i = 0; formChanges[i].method != FORM_CHANGE_TERMINATOR; i++)
        {
            if (method == formChanges[i].method && species != formChanges[i].targetSpecies)
                return TRUE;
        }
    }

    return FALSE;
}

u16 MonTryLearningNewMoveEvolution(struct Pokemon *mon, bool8 firstMove)
{
    u16 species = GetMonData(mon, MON_DATA_SPECIES, NULL);
    u8 level = GetMonData(mon, MON_DATA_LEVEL, NULL);
    const struct LevelUpMove *learnset = GetSpeciesLevelUpLearnset(species);

    // Since you can learn more than one move per level,
    // the game needs to know whether you decided to
    // learn it or keep the old set to avoid asking
    // you to learn the same move over and over again.
    if (firstMove)
    {
        sLearningMoveTableID = 0;
    }
    while(learnset[sLearningMoveTableID].move != LEVEL_UP_MOVE_END)
    {
        while (learnset[sLearningMoveTableID].level == 0 || learnset[sLearningMoveTableID].level == level)
        {
            gMoveToLearn = learnset[sLearningMoveTableID].move;
            sLearningMoveTableID++;
            return GiveMoveToMon(mon, gMoveToLearn);
        }
        sLearningMoveTableID++;
    }
    return 0;
}

static void RemoveIVIndexFromList(u8 *ivs, u8 selectedIv)
{
    s32 i, j;
    u8 temp[NUM_STATS];

    ivs[selectedIv] = 0xFF;
    for (i = 0; i < NUM_STATS; i++)
    {
        temp[i] = ivs[i];
    }

    j = 0;
    for (i = 0; i < NUM_STATS; i++)
    {
        if (temp[i] != 0xFF)
            ivs[j++] = temp[i];
    }
}

// Attempts to perform non-level/item related overworld evolutions; called by tryspecialevo command.
void TrySpecialOverworldEvo(void)
{
    u8 i;
    u8 evoMethod = gSpecialVar_0x8000;
    u16 canStopEvo = gSpecialVar_0x8001;
    u16 tryMultiple = gSpecialVar_0x8002;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        u16 targetSpecies = GetEvolutionTargetSpecies(&gPlayerParty[i], EVO_MODE_OVERWORLD_SPECIAL, evoMethod, SPECIES_NONE);
        if (targetSpecies != SPECIES_NONE && !(sTriedEvolving & (1u << i)))
        {
            sTriedEvolving |= 1u << i;
            if(gMain.callback2 == TrySpecialOverworldEvo) // This fixes small graphics glitches.
                EvolutionScene(&gPlayerParty[i], targetSpecies, canStopEvo, i);
            else
                BeginEvolutionScene(&gPlayerParty[i], targetSpecies, canStopEvo, i);
            if (tryMultiple)
                gCB2_AfterEvolution = TrySpecialOverworldEvo;
            else
                gCB2_AfterEvolution = CB2_ReturnToField;
            return;
        }
    }

    sTriedEvolving = 0;
    SetMainCallback2(CB2_ReturnToField);
}

bool32 SpeciesHasGenderDifferences(u16 species)
{
    if (gSpeciesInfo[species].frontPicFemale != NULL
     || gSpeciesInfo[species].backPicFemale != NULL
     || gSpeciesInfo[species].paletteFemale != NULL
     || gSpeciesInfo[species].shinyPaletteFemale != NULL
     || gSpeciesInfo[species].iconSpriteFemale != NULL)
        return TRUE;

    return FALSE;
}

bool32 TryFormChange(u32 monId, u32 side, u16 method)
{
    struct Pokemon *party = (side == B_SIDE_PLAYER) ? gPlayerParty : gEnemyParty;
    u16 targetSpecies;

    if (GetMonData(&party[monId], MON_DATA_SPECIES_OR_EGG, 0) == SPECIES_NONE
     || GetMonData(&party[monId], MON_DATA_SPECIES_OR_EGG, 0) == SPECIES_EGG)
        return FALSE;

    targetSpecies = GetFormChangeTargetSpecies(&party[monId], method, 0);

    if (targetSpecies == SPECIES_NONE && gBattleStruct != NULL)
        targetSpecies = gBattleStruct->changedSpecies[side][monId];

    if (targetSpecies != SPECIES_NONE)
    {
        TryToSetBattleFormChangeMoves(&party[monId], method);
        SetMonData(&party[monId], MON_DATA_SPECIES, &targetSpecies);
        CalculateMonStats(&party[monId]);
        return TRUE;
    }

    return FALSE;
}

u16 SanitizeSpeciesId(u16 species)
{
    if (species > NUM_SPECIES || !IsSpeciesEnabled(species))
        return SPECIES_NONE;
    else
        return species;
}

bool32 IsSpeciesEnabled(u16 species)
{
    return gSpeciesInfo[species].baseHP > 0 || species == SPECIES_EGG;
}

void TryToSetBattleFormChangeMoves(struct Pokemon *mon, u16 method)
{
    int i, j;
    u16 species = GetMonData(mon, MON_DATA_SPECIES, NULL);
    const struct FormChange *formChanges = GetSpeciesFormChanges(species);

    if (formChanges == NULL
        || (method != FORM_CHANGE_BEGIN_BATTLE && method != FORM_CHANGE_END_BATTLE))
        return;

    for (i = 0; formChanges[i].method != FORM_CHANGE_TERMINATOR; i++)
    {
        if (formChanges[i].method == method
            && formChanges[i].param2
            && formChanges[i].param3
            && formChanges[i].targetSpecies != species)
        {
            u16 originalMove = formChanges[i].param2;
            u16 newMove = formChanges[i].param3;

            for (j = 0; j < MAX_MON_MOVES; j++)
            {
                u16 currMove = GetMonData(mon, MON_DATA_MOVE1 + j, NULL);
                if (currMove == originalMove)
                    SetMonMoveSlot_KeepPP(mon, newMove, j);
            }
            break;
        }
    }
}

u32 GetMonFriendshipScore(struct Pokemon *pokemon)
{
    u32 friendshipScore = GetMonData(pokemon, MON_DATA_FRIENDSHIP, NULL);

    if (friendshipScore == MAX_FRIENDSHIP)
        return FRIENDSHIP_MAX;
    if (friendshipScore >= 200)
        return FRIENDSHIP_200_TO_254;
    if (friendshipScore >= 150)
        return FRIENDSHIP_150_TO_199;
    if (friendshipScore >= 100)
        return FRIENDSHIP_100_TO_149;
    if (friendshipScore >= 50)
        return FRIENDSHIP_50_TO_99;
    if (friendshipScore >= 1)
        return FRIENDSHIP_1_TO_49;

    return FRIENDSHIP_NONE;
}

u32 GetMonAffectionHearts(struct Pokemon *pokemon)
{
    u32 friendship = GetMonData(pokemon, MON_DATA_FRIENDSHIP, NULL);

    if (friendship == MAX_FRIENDSHIP)
        return AFFECTION_FIVE_HEARTS;
    if (friendship >= 220)
        return AFFECTION_FOUR_HEARTS;
    if (friendship >= 180)
        return AFFECTION_THREE_HEARTS;
    if (friendship >= 130)
        return AFFECTION_TWO_HEARTS;
    if (friendship >= 80)
        return AFFECTION_ONE_HEART;

    return AFFECTION_NO_HEARTS;
}

void UpdateMonPersonality(struct BoxPokemon *boxMon, u32 personality)
{
    bool32 isShiny = GetBoxMonData(boxMon, MON_DATA_IS_SHINY, NULL);
    u32 hiddenNature = GetBoxMonData(boxMon, MON_DATA_HIDDEN_NATURE, NULL);
    u32 teraType = GetBoxMonData(boxMon, MON_DATA_TERA_TYPE, NULL);

    SetBoxMonData(boxMon, MON_DATA_IS_SHINY, &isShiny);
    SetBoxMonData(boxMon, MON_DATA_HIDDEN_NATURE, &hiddenNature);
    SetBoxMonData(boxMon, MON_DATA_TERA_TYPE, &teraType);
}

void HealPokemon(struct Pokemon *mon)
{
    u32 data;

    data = GetMonData(mon, MON_DATA_MAX_HP);
    SetMonData(mon, MON_DATA_HP, &data);

    data = STATUS1_NONE;
    SetMonData(mon, MON_DATA_STATUS, &data);

    MonRestorePP(mon);
}

void HealBoxPokemon(struct BoxPokemon *boxMon)
{
    u32 data;

    data = STATUS1_NONE;
    SetBoxMonData(boxMon, MON_DATA_STATUS, &data);

    BoxMonRestorePP(boxMon);
}

u16 GetCryIdBySpecies(u16 species)
{
    species = SanitizeSpeciesId(species);
    if (gSpeciesInfo[species].cryId >= CRY_COUNT)
        return CRY_NONE;
    return gSpeciesInfo[species].cryId;
}

u16 GetSpeciesPreEvolution(u16 species)
{
    int i, j;

    for (i = SPECIES_BULBASAUR; i < NUM_SPECIES; i++)
    {
        const struct Evolution *evolutions = GetSpeciesEvolutions(i);
        if (evolutions == NULL)
            continue;
        for (j = 0; evolutions[j].method != EVOLUTIONS_END; j++)
        {
            if (SanitizeSpeciesId(evolutions[j].targetSpecies) == species)
                return i;
        }
    }

    return SPECIES_NONE;
}

const u8 *GetMoveName(u16 moveId)
{
    return gMovesInfo[moveId].name;
}

const u8 *GetMoveAnimationScript(u16 moveId)
{
    if (gMovesInfo[moveId].battleAnimScript == NULL)
    {
        DebugPrintfLevel(MGBA_LOG_WARN, "No animation for moveId=%u", moveId);
        return gMovesInfo[MOVE_NONE].battleAnimScript;
    }
    return gMovesInfo[moveId].battleAnimScript;
}

u32 CheckDynamicMoveType(struct Pokemon *mon, u32 move, u32 battler)
{
    u32 moveType = GetDynamicMoveType(mon, move, battler, NULL);
    if (moveType != TYPE_NONE)
        return moveType;
    return gMovesInfo[move].type;
}
