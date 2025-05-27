#include "global.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_ai_main.h"
#include "battle_ai_util.h"
#include "battle_arena.h"
#include "battle_controllers.h"
#include "battle_interface.h"
#include "battle_main.h"
#include "battle_message.h"
#include "battle_pyramid.h"
#include "battle_scripts.h"
#include "battle_setup.h"
#include "battle_tower.h"
#include "battle_util.h"
#include "berry.h"
#include "bg.h"
#include "data.h"
#include "debug.h"
#include "decompress.h"
#include "dma3.h"
#include "event_data.h"
#include "evolution_scene.h"
#include "field_weather.h"
#include "graphics.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "item.h"
#include "link.h"
#include "load_save.h"
#include "main.h"
#include "malloc.h"
#include "m4a.h"
#include "palette.h"
#include "party_menu.h"
#include "pokeball.h"
#include "pokedex.h"
#include "pokemon.h"
#include "random.h"
#include "roamer.h"
#include "safari_zone.h"
#include "scanline_effect.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "trig.h"
#include "util.h"
#include "wild_encounter.h"
#include "window.h"
#include "constants/abilities.h"
#include "constants/battle_ai.h"
#include "constants/battle_move_effects.h"
#include "constants/battle_string_ids.h"
#include "constants/battle_partner.h"
#include "constants/hold_effects.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/party_menu.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "constants/trainers.h"
#include "constants/weather.h"

extern const struct BgTemplate gBattleBgTemplates[];
extern const struct WindowTemplate *const gBattleWindowTemplates[];

static void CB2_InitBattleInternal(void);
static void CB2_PreInitIngamePlayerPartnerBattle(void);
static void CB2_HandleStartBattle(void);
static u8 CreateNPCTrainerParty(struct Pokemon *party, u16 trainerNum, bool8 firstTrainer);
static void BattleMainCB1(void);
static void SpriteCB_MoveWildMonToRight(struct Sprite *sprite);
static void SpriteCB_WildMonShowHealthbox(struct Sprite *sprite);
static void SpriteCB_WildMonAnimate(struct Sprite *sprite);
static void SpriteCB_AnimFaintOpponent(struct Sprite *sprite);
static void SpriteCB_BlinkVisible(struct Sprite *sprite);
static void SpriteCB_Idle(struct Sprite *sprite);
static void SpriteCB_BattleSpriteSlideLeft(struct Sprite *sprite);
static void TurnValuesCleanUp(bool8 var0);
static void SpriteCB_BounceEffect(struct Sprite *sprite);
static void BattleStartClearSetData(void);
static void DoBattleIntro(void);
static void TryDoEventsBeforeFirstTurn(void);
static void HandleTurnActionSelectionState(void);
static void RunTurnActionsFunctions(void);
static void SetActionsAndBattlersTurnOrder(void);
static void UpdateBattlerPartyOrdersOnSwitch(u32 battler);
static bool8 AllAtActionConfirmed(void);
static void TryChangingTurnOrderEffects(u32 battler1, u32 battler2);
static void CheckChangingTurnOrderEffects(void);
static void FreeResetData_ReturnToOvOrDoEvolutions(void);
static void ReturnFromBattleToOverworld(void);
static void TryEvolvePokemon(void);
static void WaitForEvoSceneToFinish(void);
static void HandleEndTurn_ContinueBattle(void);
static void HandleEndTurn_BattleWon(void);
static void HandleEndTurn_BattleLost(void);
static void HandleEndTurn_RanFromBattle(void);
static void HandleEndTurn_MonFled(void);
static void HandleEndTurn_FinishBattle(void);
static u32 Crc32B (const u8 *data, u32 size);
static u32 GeneratePartyHash(const struct Trainer *trainer, u32 i);
static s32 Factorial(s32);

EWRAM_DATA u16 gBattle_BG0_X = 0;
EWRAM_DATA u16 gBattle_BG0_Y = 0;
EWRAM_DATA u16 gBattle_BG1_X = 0;
EWRAM_DATA u16 gBattle_BG1_Y = 0;
EWRAM_DATA u16 gBattle_BG2_X = 0;
EWRAM_DATA u16 gBattle_BG2_Y = 0;
EWRAM_DATA u16 gBattle_BG3_X = 0;
EWRAM_DATA u16 gBattle_BG3_Y = 0;
EWRAM_DATA u16 gBattle_WIN0H = 0;
EWRAM_DATA u16 gBattle_WIN0V = 0;
EWRAM_DATA u16 gBattle_WIN1H = 0;
EWRAM_DATA u16 gBattle_WIN1V = 0;
EWRAM_DATA u8 gDisplayedStringBattle[425] = {0}; // Increased in size to fit Juan's defeat text (SootopolisCity_Gym_1F_Text_JuanDefeat)
EWRAM_DATA u8 gBattleTextBuff1[TEXT_BUFF_ARRAY_COUNT] = {0};
EWRAM_DATA u8 gBattleTextBuff2[TEXT_BUFF_ARRAY_COUNT] = {0};
EWRAM_DATA u8 gBattleTextBuff3[TEXT_BUFF_ARRAY_COUNT] = {0};
EWRAM_DATA u32 gBattleTypeFlags = 0;
EWRAM_DATA u8 gBattleTerrain = 0;
EWRAM_DATA struct MultiPartnerMenuPokemon gMultiPartnerParty[MULTI_PARTY_SIZE + 1] = {0};
EWRAM_DATA static struct MultiPartnerMenuPokemon* sMultiPartnerPartyBuffer = NULL;
EWRAM_DATA u8 *gBattleAnimBgTileBuffer = NULL;
EWRAM_DATA u8 *gBattleAnimBgTilemapBuffer = NULL;
EWRAM_DATA u32 gBattleControllerExecFlags = 0;
EWRAM_DATA u8 gBattlersCount = 0;
EWRAM_DATA u16 gBattlerPartyIndexes[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u8 gBattlerPositions[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u8 gActionsByTurnOrder[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u8 gBattlerByTurnOrder[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u8 gCurrentTurnActionNumber = 0;
EWRAM_DATA u8 gCurrentActionFuncId = 0;
EWRAM_DATA struct BattlePokemon gBattleMons[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u8 gBattlerSpriteIds[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u8 gCurrMovePos = 0;
EWRAM_DATA u8 gChosenMovePos = 0;
EWRAM_DATA u16 gCurrentMove = 0;
EWRAM_DATA u16 gChosenMove = 0;
EWRAM_DATA u16 gCalledMove = 0;
EWRAM_DATA s32 gBattleMoveDamage = 0;
EWRAM_DATA s32 gHpDealt = 0;
EWRAM_DATA s32 gBideDmg[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u16 gLastUsedItem = 0;
EWRAM_DATA u16 gLastUsedAbility = 0;
EWRAM_DATA u8 gBattlerAttacker = 0;
EWRAM_DATA u8 gBattlerTarget = 0;
EWRAM_DATA u8 gBattlerFainted = 0;
EWRAM_DATA u8 gEffectBattler = 0;
EWRAM_DATA u8 gPotentialItemEffectBattler = 0;
EWRAM_DATA u8 gAbsentBattlerFlags = 0;
EWRAM_DATA u8 gIsCriticalHit = FALSE;
EWRAM_DATA u8 gMultiHitCounter = 0;
EWRAM_DATA const u8 *gBattlescriptCurrInstr = NULL;
EWRAM_DATA u8 gChosenActionByBattler[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA const u8 *gSelectionBattleScripts[MAX_BATTLERS_COUNT] = {NULL};
EWRAM_DATA const u8 *gPalaceSelectionBattleScripts[MAX_BATTLERS_COUNT] = {NULL};
EWRAM_DATA u16 gLastPrintedMoves[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u16 gLastMoves[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u16 gLastLandedMoves[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u16 gLastHitByType[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u16 gLastUsedMoveType[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u16 gLastResultingMoves[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u16 gLockedMoves[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u16 gLastUsedMove = 0;
EWRAM_DATA u8 gLastHitBy[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u16 gChosenMoveByBattler[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u16 gMoveResultFlags = 0;
EWRAM_DATA u32 gHitMarker = 0;
EWRAM_DATA u8 gBideTarget[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u32 gSideStatuses[NUM_BATTLE_SIDES] = {0};
EWRAM_DATA struct SideTimer gSideTimers[NUM_BATTLE_SIDES] = {0};
EWRAM_DATA u32 gStatuses3[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u32 gStatuses4[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA struct DisableStruct gDisableStructs[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u16 gPauseCounterBattle = 0;
EWRAM_DATA u16 gPaydayMoney = 0;
EWRAM_DATA u8 gBattleCommunication[BATTLE_COMMUNICATION_ENTRIES_COUNT] = {0};
EWRAM_DATA u8 gBattleOutcome = 0;
EWRAM_DATA struct ProtectStruct gProtectStructs[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA struct SpecialStatus gSpecialStatuses[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u16 gBattleWeather = 0;
EWRAM_DATA struct WishFutureKnock gWishFutureKnock = {0};
EWRAM_DATA u16 gIntroSlideFlags = 0;
EWRAM_DATA u8 gSentPokesToOpponent[2] = {0};
EWRAM_DATA struct BattleScripting gBattleScripting = {0};
EWRAM_DATA struct BattleStruct *gBattleStruct = NULL;
EWRAM_DATA struct BattleResources *gBattleResources = NULL;
EWRAM_DATA u8 gActionSelectionCursor[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u8 gMoveSelectionCursor[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u8 gBattlerStatusSummaryTaskId[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u8 gBattlerInMenuId = 0;
EWRAM_DATA bool8 gDoingBattleAnim = FALSE;
EWRAM_DATA u32 gTransformedPersonalities[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA bool8 gTransformedShininess[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u8 gPlayerDpadHoldFrames = 0;
EWRAM_DATA struct BattleSpriteData *gBattleSpritesDataPtr = NULL;
EWRAM_DATA struct MonSpritesGfx *gMonSpritesGfxPtr = NULL;
EWRAM_DATA u16 gBattleMovePower = 0;
EWRAM_DATA u16 gMoveToLearn = 0;
EWRAM_DATA u32 gFieldStatuses = 0;
EWRAM_DATA struct FieldTimer gFieldTimers = {0};
EWRAM_DATA u8 gBattlerAbility = 0;
EWRAM_DATA struct QueuedStatBoost gQueuedStatBoosts[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u8 gLastUsedBall = 0;
EWRAM_DATA u16 gLastThrownBall = 0;
EWRAM_DATA u16 gBallToDisplay = 0;
EWRAM_DATA bool8 gLastUsedBallMenuPresent = FALSE;
EWRAM_DATA static u8 sTriedEvolving = 0;
EWRAM_DATA u8 gCategoryIconSpriteId = 0;

COMMON_DATA void (*gPreBattleCallback1)(void) = NULL;
COMMON_DATA void (*gBattleMainFunc)(void) = NULL;
COMMON_DATA struct BattleResults gBattleResults = {0};
COMMON_DATA u8 gLeveledUpInBattle = 0;
COMMON_DATA u8 gHealthboxSpriteIds[MAX_BATTLERS_COUNT] = {0};
COMMON_DATA u8 gMultiUsePlayerCursor = 0;
COMMON_DATA u8 gNumberOfMovesToChoose = 0;

static const struct ScanlineEffectParams sIntroScanlineParams16Bit =
{
    &REG_BG3HOFS, SCANLINE_EFFECT_DMACNT_16BIT, 1
};

const struct OamData gOamData_BattleSpriteOpponentSide =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_NORMAL,
    .objMode = ST_OAM_OBJ_NORMAL,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(64x64),
    .x = 0,
    .size = SPRITE_SIZE(64x64),
    .tileNum = 0,
    .priority = 2,
    .paletteNum = 0,
    .affineParam = 0,
};

const struct OamData gOamData_BattleSpritePlayerSide =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_NORMAL,
    .objMode = ST_OAM_OBJ_NORMAL,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(64x64),
    .x = 0,
    .size = SPRITE_SIZE(64x64),
    .tileNum = 0,
    .priority = 2,
    .paletteNum = 2,
    .affineParam = 0,
};

static const s8 sCenterToCornerVecXs[8] ={-32, -16, -16, -32, -32};

#include "data/types_info.h"

// extra args are money and ball
#define TRAINER_CLASS(trainerClass, trainerName, ...)   \
    [TRAINER_CLASS_##trainerClass] =                    \
    {                                                   \
        .name = _(trainerName),                         \
        .money = DEFAULT(5, __VA_ARGS__),               \
        .ball = DEFAULT_2(ITEM_POKE_BALL, __VA_ARGS__), \
    }

const struct TrainerClass gTrainerClasses[TRAINER_CLASS_COUNT] =
{
    TRAINER_CLASS(PKMN_TRAINER_1, "Entrenador", 6, ITEM_POKE_BALL),
    TRAINER_CLASS(PKMN_TRAINER_2, "Entrenador", 6, ITEM_POKE_BALL),
    TRAINER_CLASS(HIKER, "Montañero", 10, ITEM_ULTRA_BALL),
    TRAINER_CLASS(TEAM_AQUA, "Equipo Aqua", 8, ITEM_DIVE_BALL),
    TRAINER_CLASS(PKMN_BREEDER, "Criador {PKMN}", 10, ITEM_HEAL_BALL),
    TRAINER_CLASS(COOLTRAINER, "Molón", 12, ITEM_ULTRA_BALL),
    TRAINER_CLASS(BIRD_KEEPER, "Ornitólogo", 8, ITEM_NEST_BALL),
    TRAINER_CLASS(COLLECTOR, "Colector", 15, ITEM_PREMIER_BALL),
    TRAINER_CLASS(SWIMMER_M, "Nadador", 2, ITEM_DIVE_BALL),
    TRAINER_CLASS(TEAM_MAGMA, "Equipo Magma", 8, ITEM_DUSK_BALL),
    TRAINER_CLASS(EXPERT, "Pokéxperto", 10, ITEM_FRIEND_BALL),
    TRAINER_CLASS(AQUA_ADMIN, "Admin. Aqua", 10, ITEM_DIVE_BALL),
    TRAINER_CLASS(BLACK_BELT, "Karateka", 8, ITEM_ULTRA_BALL),
    TRAINER_CLASS(AQUA_LEADER, "Líder Aqua", 20, ITEM_MASTER_BALL),
    TRAINER_CLASS(HEX_MANIAC, "Bruja", 6, ITEM_HEAVY_BALL),
    TRAINER_CLASS(AROMA_LADY, "Señorita", 10, ITEM_LOVE_BALL),
    TRAINER_CLASS(RUIN_MANIAC, "Ruinamaníaco", 15, ITEM_DUSK_BALL),
    TRAINER_CLASS(INTERVIEWER, "Reportera", 12, ITEM_FRIEND_BALL),
    TRAINER_CLASS(TUBER_F, "Playera", 1, ITEM_NET_BALL),
    TRAINER_CLASS(TUBER_M, "Playero", 1, ITEM_NET_BALL),
    TRAINER_CLASS(LADY, "Damisela", 50, ITEM_LUXURY_BALL),
    TRAINER_CLASS(BEAUTY, "Bella", 20, ITEM_LOVE_BALL),
    TRAINER_CLASS(RICH_BOY, "Niño bien", 50, ITEM_LUXURY_BALL),
    TRAINER_CLASS(POKEMANIAC, "Pokémaniaco", 15, ITEM_NEST_BALL),
    TRAINER_CLASS(GUITARIST, "Guitarrista", 8, ITEM_QUICK_BALL),
    TRAINER_CLASS(KINDLER, "Fogonero", 8, ITEM_QUICK_BALL),
    TRAINER_CLASS(CAMPER, "Campista", 4, ITEM_LEVEL_BALL),
    TRAINER_CLASS(PICNICKER, "Dominguera", 4, ITEM_NEST_BALL),
    TRAINER_CLASS(BUG_MANIAC, "Bichomaníaco", 15, ITEM_NET_BALL),
    TRAINER_CLASS(PSYCHIC, "Médium", 6, ITEM_DUSK_BALL),
    TRAINER_CLASS(GENTLEMAN, "Caballero", 20, ITEM_LUXURY_BALL),
    TRAINER_CLASS(ELITE_FOUR, "Alto Mando", 25, ITEM_ULTRA_BALL),
    TRAINER_CLASS(LEADER, "Líder", 25, ITEM_ULTRA_BALL),
    TRAINER_CLASS(SCHOOL_KID, "Colegial"),
    TRAINER_CLASS(SR_AND_JR, "Estudiantes", 4, ITEM_REPEAT_BALL),
    TRAINER_CLASS(WINSTRATE, "Estratega", 10, ITEM_REPEAT_BALL),
    TRAINER_CLASS(POKEFAN, "Pokéfan", 20, ITEM_PREMIER_BALL),
    TRAINER_CLASS(YOUNGSTER, "Joven", 4, ITEM_NEST_BALL),
    TRAINER_CLASS(CHAMPION, "Campeón", 50, ITEM_ULTRA_BALL),
    TRAINER_CLASS(FISHERMAN, "Pescador", 10, B_TRAINER_CLASS_POKE_BALLS >= GEN_8 ? ITEM_DIVE_BALL : ITEM_LURE_BALL),
    TRAINER_CLASS(TRIATHLETE, "Triatleta", 10, ITEM_QUICK_BALL),
    TRAINER_CLASS(DRAGON_TAMER, "Domadragones", 12, ITEM_CHERISH_BALL),
    TRAINER_CLASS(NINJA_BOY, "Ninja", 3, ITEM_MOON_BALL),
    TRAINER_CLASS(BATTLE_GIRL, "Luchadora", 6, ITEM_GREAT_BALL),
    TRAINER_CLASS(PARASOL_LADY, "Dama parasol", 10, ITEM_LUXURY_BALL),
    TRAINER_CLASS(SWIMMER_F, "Nadadora", 2, ITEM_DIVE_BALL),
    TRAINER_CLASS(TWINS, "Gemelas", 3, ITEM_FRIEND_BALL),
    TRAINER_CLASS(SAILOR, "Marinero", 8, ITEM_DIVE_BALL),
    TRAINER_CLASS(COOLTRAINER_2, "Molón", 10, ITEM_ULTRA_BALL),
    TRAINER_CLASS(MAGMA_ADMIN, "Admin. Magma", 10, ITEM_DUSK_BALL),
    TRAINER_CLASS(RIVAL, "Entrenador", 15, ITEM_ULTRA_BALL),
    TRAINER_CLASS(BUG_CATCHER, "Cazabichos", 4, ITEM_NET_BALL),
    TRAINER_CLASS(PKMN_RANGER, "{PKMN} Ranger", 12, ITEM_ULTRA_BALL),
    TRAINER_CLASS(MAGMA_LEADER, "Líder Magma", 20, ITEM_MASTER_BALL),
    TRAINER_CLASS(LASS, "Señorita", 4, ITEM_LUXURY_BALL),
    TRAINER_CLASS(YOUNG_COUPLE, "Joven pareja", 8, ITEM_LOVE_BALL),
    TRAINER_CLASS(OLD_COUPLE, "Pareja mayor", 10, ITEM_LOVE_BALL),
    TRAINER_CLASS(SIS_AND_BRO, "Hermanos", 3, ITEM_FRIEND_BALL),
    TRAINER_CLASS(SALON_MAIDEN, "Dama Torre"),
    TRAINER_CLASS(DOME_ACE, "Astro Cúpula"),
    TRAINER_CLASS(PALACE_MAVEN, "Amo Palacio"),
    TRAINER_CLASS(ARENA_TYCOON, "Maestra Dojo"),
    TRAINER_CLASS(FACTORY_HEAD, "Jefe Fábrica"),
    TRAINER_CLASS(PIKE_QUEEN, "Reina Sierpe"),
    TRAINER_CLASS(PYRAMID_KING, "Rey Pirámide"),
    TRAINER_CLASS(RS_PROTAG, "Entrenador"),
};

static void (* const sTurnActionsFuncsTable[])(void) =
{
    [B_ACTION_USE_MOVE]               = HandleAction_UseMove,
    [B_ACTION_USE_ITEM]               = HandleAction_UseItem,
    [B_ACTION_SWITCH]                 = HandleAction_Switch,
    [B_ACTION_RUN]                    = HandleAction_Run,
    [B_ACTION_SAFARI_WATCH_CAREFULLY] = HandleAction_WatchesCarefully,
    [B_ACTION_SAFARI_BALL]            = HandleAction_SafariZoneBallThrow,
    [B_ACTION_SAFARI_POKEBLOCK]       = HandleAction_ThrowPokeblock,
    [B_ACTION_SAFARI_GO_NEAR]         = HandleAction_GoNear,
    [B_ACTION_SAFARI_RUN]             = HandleAction_SafariZoneRun,
    [B_ACTION_WALLY_THROW]            = HandleAction_WallyBallThrow,
    [B_ACTION_EXEC_SCRIPT]            = HandleAction_RunBattleScript,
    [B_ACTION_TRY_FINISH]             = HandleAction_TryFinish,
    [B_ACTION_FINISHED]               = HandleAction_ActionFinished,
    [B_ACTION_NOTHING_FAINTED]        = HandleAction_NothingIsFainted,
    [B_ACTION_THROW_BALL]             = HandleAction_ThrowBall,
};

static void (* const sEndTurnFuncsTable[])(void) =
{
    [0]                           = HandleEndTurn_ContinueBattle,
    [B_OUTCOME_WON]               = HandleEndTurn_BattleWon,
    [B_OUTCOME_LOST]              = HandleEndTurn_BattleLost,
    [B_OUTCOME_DREW]              = HandleEndTurn_BattleLost,
    [B_OUTCOME_RAN]               = HandleEndTurn_RanFromBattle,
    [B_OUTCOME_PLAYER_TELEPORTED] = HandleEndTurn_FinishBattle,
    [B_OUTCOME_MON_FLED]          = HandleEndTurn_MonFled,
    [B_OUTCOME_CAUGHT]            = HandleEndTurn_FinishBattle,
    [B_OUTCOME_NO_SAFARI_BALLS]   = HandleEndTurn_FinishBattle,
    [B_OUTCOME_FORFEITED]         = HandleEndTurn_FinishBattle,
    [B_OUTCOME_MON_TELEPORTED]    = HandleEndTurn_FinishBattle,
};

const u8 *const gStatusConditionStringsTable[7] =
{
    gText_Poison,
    gText_Sleep,
    gText_Paralysis,
    gText_Burn,
    gText_Ice,
    gText_Confusion,
    gText_Love
};

void CB2_InitBattle(void)
{
    ResetHeap();
    AllocateBattleResources();
    AllocateBattleSpritesData();
    AllocateMonSpritesGfx();
    CB2_InitBattleInternal();
}

static void CB2_InitBattleInternal(void)
{
    s32 i;

    SetHBlankCallback(NULL);
    SetVBlankCallback(NULL);

    CpuFill32(0, (void *)(VRAM), VRAM_SIZE);

    SetGpuReg(REG_OFFSET_MOSAIC, 0);
    SetGpuReg(REG_OFFSET_WIN0H, DISPLAY_WIDTH);
    SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(DISPLAY_HEIGHT / 2, DISPLAY_HEIGHT / 2 + 1));
    SetGpuReg(REG_OFFSET_WININ, 0);
    SetGpuReg(REG_OFFSET_WINOUT, 0);

    gBattle_WIN0H = DISPLAY_WIDTH;

    gBattle_WIN0V = WIN_RANGE(DISPLAY_HEIGHT / 2, DISPLAY_HEIGHT / 2 + 1);
    ScanlineEffect_Clear();

    for (i = 0; i < DISPLAY_HEIGHT / 2; i++)
    {
        gScanlineEffectRegBuffers[0][i] = 0xF0;
        gScanlineEffectRegBuffers[1][i] = 0xF0;
    }

    for (; i < DISPLAY_HEIGHT; i++)
    {
        gScanlineEffectRegBuffers[0][i] = 0xFF10;
        gScanlineEffectRegBuffers[1][i] = 0xFF10;
    }

    ScanlineEffect_SetParams(sIntroScanlineParams16Bit);

    ResetPaletteFade();
    gBattle_BG0_X = 0;
    gBattle_BG0_Y = 0;
    gBattle_BG1_X = 0;
    gBattle_BG1_Y = 0;
    gBattle_BG2_X = 0;
    gBattle_BG2_Y = 0;
    gBattle_BG3_X = 0;
    gBattle_BG3_Y = 0;

    if (!DEBUG_OVERWORLD_MENU || (DEBUG_OVERWORLD_MENU && !gIsDebugBattle))
    {
        gBattleTerrain = BattleSetup_GetTerrainId();
    }

    InitBattleBgsVideo();
    LoadBattleTextboxAndBackground();
    ResetSpriteData();
    ResetTasks();
    DrawBattleEntryBackground();
    FreeAllSpritePalettes();
    gReservedSpritePaletteCount = MAX_BATTLERS_COUNT;
    SetVBlankCallback(VBlankCB_Battle);
    SetUpBattleVarsAndBirchZigzagoon();
    SetMainCallback2(CB2_HandleStartBattle);

    if (!DEBUG_OVERWORLD_MENU || (DEBUG_OVERWORLD_MENU && !gIsDebugBattle))
    {
        CreateNPCTrainerParty(&gEnemyParty[0], gTrainerBattleOpponent_A, TRUE);
        SetWildMonHeldItem();
        CalculateEnemyPartyCount();
    }

    gMain.inBattle = TRUE;
    gSaveBlockPtr->frontier.disableRecordBattle = FALSE;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        AdjustFriendship(&gPlayerParty[i], FRIENDSHIP_EVENT_LEAGUE_BATTLE);
    }

    gBattleCommunication[MULTIUSE_STATE] = 0;
}

#define BUFFER_PARTY_VS_SCREEN_STATUS(party, flags, i)                      \
    for ((i) = 0; (i) < PARTY_SIZE; (i)++)                                  \
    {                                                                       \
        u16 species = GetMonData(&(party)[(i)], MON_DATA_SPECIES_OR_EGG);   \
        u16 hp = GetMonData(&(party)[(i)], MON_DATA_HP);                    \
        u32 status = GetMonData(&(party)[(i)], MON_DATA_STATUS);            \
                                                                            \
        if (species == SPECIES_NONE)                                        \
            continue;                                                       \
                                                                            \
        /* Is healthy mon? */                                               \
        if (species != SPECIES_EGG && hp != 0 && status == 0)               \
            (flags) |= 1 << (i) * 2;                                        \
                                                                            \
        /* Is Egg or statused? */                                           \
        if (hp != 0 && (species == SPECIES_EGG || status != 0))             \
            (flags) |= 2 << (i) * 2;                                        \
                                                                            \
        /* Is fainted? */                                                   \
        if (species != SPECIES_EGG && hp == 0)                              \
            (flags) |= 3 << (i) * 2;                                        \
    }

static void CB2_HandleStartBattle(void)
{
    u8 playerMultiplayerId;

    RunTasks();
    AnimateSprites();
    BuildOamBuffer();

    playerMultiplayerId = GetMultiplayerId();
    gBattleScripting.multiplayerId = playerMultiplayerId;

    switch (gBattleCommunication[MULTIUSE_STATE])
    {
    case 0:
        if (!IsDma3ManagerBusyWithBgCopy())
        {
            ShowBg(0);
            ShowBg(1);
            ShowBg(2);
            ShowBg(3);
            FillAroundBattleWindows();
            gBattleCommunication[MULTIUSE_STATE] = 1;
        }
        break;
    case 1:
        gBattleCommunication[MULTIUSE_STATE] = 15;
        break;
    case 2:
        break;
    case 3:
        // Link battle, send/receive party Pokémon 2 at a time
        if (IsLinkTaskFinished())
        {
            // Send Pokémon 1-2
            SendBlock(BitmaskAllOtherLinkPlayers(), gPlayerParty, sizeof(struct Pokemon) * 2);
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 4:
        if ((GetBlockReceivedStatus() & 3) == 3)
        {
            // Recv Pokémon 1-2
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 7:
        if (IsLinkTaskFinished())
        {
            // Send Pokémon 3-4
            SendBlock(BitmaskAllOtherLinkPlayers(), &gPlayerParty[2], sizeof(struct Pokemon) * 2);
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 8:
        if ((GetBlockReceivedStatus() & 3) == 3)
        {
            // Recv Pokémon 3-4
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 11:
        if (IsLinkTaskFinished())
        {
            // Send Pokémon 5-6
            SendBlock(BitmaskAllOtherLinkPlayers(), &gPlayerParty[4], sizeof(struct Pokemon) * 2);
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 12:
        if ((GetBlockReceivedStatus() & 3) == 3)
        {
            // Recv Pokémon 5-6
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 15:
        InitBattleControllers();
        gBattleCommunication[SPRITES_INIT_STATE1] = 0;
        gBattleCommunication[SPRITES_INIT_STATE2] = 0;
        gBattleCommunication[MULTIUSE_STATE] = 18;
        break;
    case 16:
        break;
    case 17:
        // Receive rng seed for recorded battle (only read it if partner is the link master)
        if ((GetBlockReceivedStatus() & 3) == 3)
        {
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 18:
        // Finish, start battle
        if (BattleInitAllSprites(&gBattleCommunication[SPRITES_INIT_STATE1], &gBattleCommunication[SPRITES_INIT_STATE2]))
        {
            gPreBattleCallback1 = gMain.callback1;
            gMain.callback1 = BattleMainCB1;
            SetMainCallback2(BattleMainCB2);
        }
        break;
    // Introduce short delays between sending party Pokémon for link
    case 5:
    case 9:
    case 13:
        gBattleCommunication[MULTIUSE_STATE]++;
        gBattleCommunication[1] = 1;
    case 6:
    case 10:
    case 14:
        if (--gBattleCommunication[1] == 0)
            gBattleCommunication[MULTIUSE_STATE]++;
        break;
    }
}

static void SetMultiPartnerMenuParty(u8 offset)
{
    s32 i;

    for (i = 0; i < MULTI_PARTY_SIZE; i++)
    {
        gMultiPartnerParty[i].species     = GetMonData(&gPlayerParty[offset + i], MON_DATA_SPECIES);
        gMultiPartnerParty[i].heldItem    = GetMonData(&gPlayerParty[offset + i], MON_DATA_HELD_ITEM);
        GetMonData(&gPlayerParty[offset + i], MON_DATA_NICKNAME, gMultiPartnerParty[i].nickname);
        gMultiPartnerParty[i].level       = GetMonData(&gPlayerParty[offset + i], MON_DATA_LEVEL);
        gMultiPartnerParty[i].hp          = GetMonData(&gPlayerParty[offset + i], MON_DATA_HP);
        gMultiPartnerParty[i].maxhp       = GetMonData(&gPlayerParty[offset + i], MON_DATA_MAX_HP);
        gMultiPartnerParty[i].status      = GetMonData(&gPlayerParty[offset + i], MON_DATA_STATUS);
        gMultiPartnerParty[i].personality = GetMonData(&gPlayerParty[offset + i], MON_DATA_PERSONALITY);
        gMultiPartnerParty[i].gender      = GetMonGender(&gPlayerParty[offset + i]);
        StripExtCtrlCodes(gMultiPartnerParty[i].nickname);
        PadNameString(gMultiPartnerParty[i].nickname, CHAR_SPACE);
    }
    memcpy(sMultiPartnerPartyBuffer, gMultiPartnerParty, sizeof(gMultiPartnerParty));
}

static void CB2_PreInitIngamePlayerPartnerBattle(void)
{
    u32 *savedBattleTypeFlags;
    void (**savedCallback)(void);

    savedCallback = &gBattleStruct->savedCallback;
    savedBattleTypeFlags = &gBattleStruct->savedBattleTypeFlags;

    RunTasks();
    AnimateSprites();
    BuildOamBuffer();

    switch (gBattleCommunication[MULTIUSE_STATE])
    {
    case 0:
        sMultiPartnerPartyBuffer = Alloc(sizeof(gMultiPartnerParty));
        SetMultiPartnerMenuParty(MULTI_PARTY_SIZE);
        gBattleCommunication[MULTIUSE_STATE]++;
        *savedCallback = gMain.savedCallback;
        *savedBattleTypeFlags = gBattleTypeFlags;
        gMain.savedCallback = CB2_PreInitIngamePlayerPartnerBattle;
        ShowPartyMenuToShowcaseMultiBattleParty();
        break;
    case 1:
        if (!gFundidoPaletas.activo)
        {
            gBattleCommunication[MULTIUSE_STATE] = 2;
            gBattleTypeFlags = *savedBattleTypeFlags;
            gMain.savedCallback = *savedCallback;
            SetMainCallback2(CB2_InitBattleInternal);
            FREE_AND_SET_NULL(sMultiPartnerPartyBuffer);
        }
        break;
    }
}

void BattleMainCB2(void)
{
    AnimateSprites();
    BuildOamBuffer();
    RunTextPrinters();
    UpdatePaletteFade();
    RunTasks();
}

#define sState data[0]
#define sDelay data[4]

static u32 Crc32B (const u8 *data, u32 size)
{
   s32 i, j;
   u32 byte, crc, mask;

   i = 0;
   crc = 0xFFFFFFFF;
   for (i = 0; i < size; ++i)
   {
        byte = data[i];
        crc = crc ^ byte;
        for (j = 7; j >= 0; --j)
        {
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
   }
   return ~crc;
}

static u32 GeneratePartyHash(const struct Trainer *trainer, u32 i)
{
    const u8 *buffer = (const u8 *) &trainer->party[i];
    u32 n = sizeof(*trainer->party);
    return Crc32B(buffer, n);
}

void ModifyPersonalityForNature(u32 *personality, u32 newNature)
{
    u32 nature = GetNatureFromPersonality(*personality);
    s32 diff = abs((s32)nature - (s32)newNature);
    s32 sign = (nature > newNature) ? 1 : -1;
    if (diff > NUMERO_NATURALEZAS / 2)
    {
        diff = NUMERO_NATURALEZAS - diff;
        sign *= -1;
    }
    *personality -= (diff * sign);
}

u32 GeneratePersonalityForGender(u32 gender, u32 species)
{
    const struct SpeciesInfo *speciesInfo = &gSpeciesInfo[species];
    if (gender == MON_GENDERLESS)
        return 0;
    else if (gender == MON_MALE)
        return ((255 - speciesInfo->genderRatio) / 2) + speciesInfo->genderRatio;
    else
        return speciesInfo->genderRatio / 2;
}

void CustomTrainerPartyAssignMoves(struct Pokemon *mon, const struct TrainerMon *partyEntry)
{
    bool32 noMoveSet = TRUE;
    u32 j;

    for (j = 0; j < MAX_MON_MOVES; ++j)
    {
        if (partyEntry->moves[j] != MOVE_NONE)
            noMoveSet = FALSE;
    }
    if (noMoveSet)
    {
        // TODO: Figure out a default strategy when moves are not set, to generate a good moveset
        return;
    }

    for (j = 0; j < MAX_MON_MOVES; ++j)
    {
        SetMonData(mon, MON_DATA_MOVE1 + j, &partyEntry->moves[j]);
        SetMonData(mon, MON_DATA_PP1 + j, &gMovesInfo[partyEntry->moves[j]].pp);
    }
}

u8 CreateNPCTrainerPartyFromTrainer(struct Pokemon *party, const struct Trainer *trainer, bool32 firstTrainer, u32 battleTypeFlags)
{
    s32 i;
    if (EsContraEntrenador())
    {
        for (i = 0; i < trainer->partySize; i++)
        {
            s32 ball = -1;
            u32 personalityHash = GeneratePartyHash(trainer, i);
            const struct TrainerMon *partyData = trainer->party;
            u32 otIdType = OT_ID_RANDOM_NO_SHINY;
            u32 fixedOtId = 0;
            u32 ability = 0;
            u32 personalityValue = 128;

            personalityValue += personalityHash << 8;
            if (partyData[i].gender == TRAINER_MON_MALE)
                personalityValue = (personalityValue & 0xFFFFFF00) | GeneratePersonalityForGender(MON_MALE, partyData[i].species);
            else if (partyData[i].gender == TRAINER_MON_FEMALE)
                personalityValue = (personalityValue & 0xFFFFFF00) | GeneratePersonalityForGender(MON_FEMALE, partyData[i].species);
            else if (partyData[i].gender == TRAINER_MON_RANDOM_GENDER)
                personalityValue = (personalityValue & 0xFFFFFF00) | GeneratePersonalityForGender(Random() & 1 ? MON_MALE : MON_FEMALE, partyData[i].species);
            ModifyPersonalityForNature(&personalityValue, partyData[i].nature);
            if (partyData[i].isShiny)
            {
                otIdType = OT_ID_PRESET;
                fixedOtId = HIHALF(personalityValue) ^ LOHALF(personalityValue);
            }
            CreateMon(&party[i], partyData[i].species, partyData[i].lvl, 0, TRUE, personalityValue, otIdType, fixedOtId);
            SetMonData(&party[i], MON_DATA_HELD_ITEM, &partyData[i].heldItem);

            CustomTrainerPartyAssignMoves(&party[i], &partyData[i]);
            SetMonData(&party[i], MON_DATA_IVS, &(partyData[i].iv));
            if (partyData[i].ev != NULL)
            {
                SetMonData(&party[i], MON_DATA_HP_EV, &(partyData[i].ev[0]));
                SetMonData(&party[i], MON_DATA_ATK_EV, &(partyData[i].ev[1]));
                SetMonData(&party[i], MON_DATA_DEF_EV, &(partyData[i].ev[2]));
                SetMonData(&party[i], MON_DATA_SPATK_EV, &(partyData[i].ev[3]));
                SetMonData(&party[i], MON_DATA_SPDEF_EV, &(partyData[i].ev[4]));
                SetMonData(&party[i], MON_DATA_SPEED_EV, &(partyData[i].ev[5]));
            }
            if (partyData[i].ability != ABILITY_NONE)
            {
                const struct SpeciesInfo *speciesInfo = &gSpeciesInfo[partyData[i].species];
                u32 maxAbilities = ARRAY_COUNT(speciesInfo->abilities);
                for (ability = 0; ability < maxAbilities; ++ability)
                {
                    if (speciesInfo->abilities[ability] == partyData[i].ability)
                        break;
                }
                if (ability >= maxAbilities)
                    ability = 0;
            }
            else if (B_TRAINER_MON_RANDOM_ABILITY)
            {
                const struct SpeciesInfo *speciesInfo = &gSpeciesInfo[partyData[i].species];
                ability = personalityHash % 3;
                while (speciesInfo->abilities[ability] == ABILITY_NONE)
                {
                    ability--;
                }
            }
            SetMonData(&party[i], MON_DATA_ABILITY_NUM, &ability);
            SetMonData(&party[i], MON_DATA_FRIENDSHIP, &(partyData[i].friendship));
            if (partyData[i].ball != ITEM_NONE)
            {
                ball = partyData[i].ball;
                SetMonData(&party[i], MON_DATA_POKEBALL, &ball);
            }
            if (partyData[i].nickname != NULL)
            {
                SetMonData(&party[i], MON_DATA_NICKNAME, partyData[i].nickname);
            }
            if (partyData[i].isShiny)
            {
                u32 data = TRUE;
                SetMonData(&party[i], MON_DATA_IS_SHINY, &data);
            }
            CalculateMonStats(&party[i]);

            if (B_TRAINER_CLASS_POKE_BALLS >= GEN_7 && ball == -1)
            {
                ball = gTrainerClasses[trainer->trainerClass].ball ?: ITEM_POKE_BALL;
                SetMonData(&party[i], MON_DATA_POKEBALL, &ball);
            }
        }
    }

    return trainer->partySize;
}

static u8 CreateNPCTrainerParty(struct Pokemon *party, u16 trainerNum, bool8 firstTrainer)
{
    u8 retVal;
    retVal = CreateNPCTrainerPartyFromTrainer(party, GetTrainerStructFromId(trainerNum), firstTrainer, gBattleTypeFlags);
    return retVal;
}

void CreateTrainerPartyForPlayer(void)
{
    ZeroPlayerPartyMons();
    gPartnerTrainerId = gSpecialVar_0x8004;
    CreateNPCTrainerPartyFromTrainer(gPlayerParty, GetTrainerStructFromId(gSpecialVar_0x8004), TRUE, TIPO_BATALLA_ENTRENADOR);
}

void VBlankCB_Battle(void)
{
    AdvanceRandom();
    SetGpuReg(REG_OFFSET_BG0HOFS, gBattle_BG0_X);
    SetGpuReg(REG_OFFSET_BG0VOFS, gBattle_BG0_Y);
    SetGpuReg(REG_OFFSET_BG1HOFS, gBattle_BG1_X);
    SetGpuReg(REG_OFFSET_BG1VOFS, gBattle_BG1_Y);
    SetGpuReg(REG_OFFSET_BG2HOFS, gBattle_BG2_X);
    SetGpuReg(REG_OFFSET_BG2VOFS, gBattle_BG2_Y);
    SetGpuReg(REG_OFFSET_BG3HOFS, gBattle_BG3_X);
    SetGpuReg(REG_OFFSET_BG3VOFS, gBattle_BG3_Y);
    SetGpuReg(REG_OFFSET_WIN0H, gBattle_WIN0H);
    SetGpuReg(REG_OFFSET_WIN0V, gBattle_WIN0V);
    SetGpuReg(REG_OFFSET_WIN1H, gBattle_WIN1H);
    SetGpuReg(REG_OFFSET_WIN1V, gBattle_WIN1V);
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
    ScanlineEffect_InitHBlankDmaTransfer();
}

void SpriteCB_VsLetterDummy(struct Sprite *sprite)
{

}

static void SpriteCB_VsLetter(struct Sprite *sprite)
{
    if (sprite->data[0] != 0)
        sprite->x = sprite->data[1] + ((sprite->data[2] & 0xFF00) >> 8);
    else
        sprite->x = sprite->data[1] - ((sprite->data[2] & 0xFF00) >> 8);

    sprite->data[2] += 0x180;

    if (sprite->affineAnimEnded)
    {
        FreeSpriteTilesByTag(ANIM_SPRITES_START);
        FreeSpritePaletteByTag(ANIM_SPRITES_START);
        FreeSpriteOamMatrix(sprite);
        DestroySprite(sprite);
    }
}

void SpriteCB_VsLetterInit(struct Sprite *sprite)
{
    StartSpriteAffineAnim(sprite, 1);
    sprite->callback = SpriteCB_VsLetter;
    PlaySE(SE_MUGSHOT);
}

u32 GetBattleBgTemplateData(u8 arrayId, u8 caseId)
{
    u32 ret = 0;

    switch (caseId)
    {
    case 0:
        ret = gBattleBgTemplates[arrayId].bg;
        break;
    case 1:
        ret = gBattleBgTemplates[arrayId].charBaseIndex;
        break;
    case 2:
        ret = gBattleBgTemplates[arrayId].mapBaseIndex;
        break;
    case 3:
        ret = gBattleBgTemplates[arrayId].screenSize;
        break;
    case 4:
        ret = gBattleBgTemplates[arrayId].paletteMode;
        break;
    case 5: // Only this case is used
        ret = gBattleBgTemplates[arrayId].priority;
        break;
    case 6:
        ret = gBattleBgTemplates[arrayId].baseTile;
        break;
    }

    return ret;
}

u32 GetBattleWindowTemplatePixelWidth(u32 windowsType, u32 tableId)
{
    return gBattleWindowTemplates[windowsType][tableId].width * 8;
}

#define sBattler            data[0]
#define sSpeciesId          data[2]

void SpriteCB_WildMon(struct Sprite *sprite)
{
    sprite->callback = SpriteCB_MoveWildMonToRight;
    StartSpriteAnimIfDifferent(sprite, 0);
    BeginNormalPaletteFade((0x10000 << sprite->sBattler), 0, 10, 10, RGB(8, 8, 8));
}

static void SpriteCB_MoveWildMonToRight(struct Sprite *sprite)
{
    if ((gIntroSlideFlags & 1) == 0)
    {
        sprite->x2 += 2;
        if (sprite->x2 == 0)
        {
            sprite->callback = SpriteCB_WildMonShowHealthbox;
        }
    }
}

static void SpriteCB_WildMonShowHealthbox(struct Sprite *sprite)
{
    if (sprite->animEnded)
    {
        StartHealthboxSlideIn(sprite->sBattler);
        SetHealthboxSpriteVisible(gHealthboxSpriteIds[sprite->sBattler]);
        sprite->callback = SpriteCB_WildMonAnimate;
        StartSpriteAnimIfDifferent(sprite, 0);
        BeginNormalPaletteFade((0x10000 << sprite->sBattler), 0, 10, 0, RGB(8, 8, 8));
    }
}

static void SpriteCB_WildMonAnimate(struct Sprite *sprite)
{
    if (!gFundidoPaletas.activo)
    {
        BattleAnimateFrontSprite(sprite, sprite->sSpeciesId, FALSE, 1);
    }
}

void SpriteCB_FaintOpponentMon(struct Sprite *sprite)
{
    u8 battler = sprite->sBattler;
    u16 species;
    u8 yOffset;

    if (gBattleSpritesDataPtr->battlerData[battler].transformSpecies != 0)
        species = gBattleSpritesDataPtr->battlerData[battler].transformSpecies;
    else
        species = sprite->sSpeciesId;

    species = SanitizeSpeciesId(species);
    yOffset = gSpeciesInfo[species].frontPicYOffset;

    sprite->data[3] = 8 - yOffset / 8;
    sprite->data[4] = 1;
    sprite->callback = SpriteCB_AnimFaintOpponent;
}

static void SpriteCB_AnimFaintOpponent(struct Sprite *sprite)
{
    s32 i;

    if (--sprite->data[4] == 0)
    {
        sprite->data[4] = 2;
        sprite->y2 += 8; // Move the sprite down.
        if (--sprite->data[3] < 0)
        {
            FreeSpriteOamMatrix(sprite);
            DestroySprite(sprite);
        }
        else // Erase bottom part of the sprite to create a smooth illusion of mon falling down.
        {
            u8 *dst = &gMonSpritesGfxPtr->spritesGfx[GetBattlerPosition(sprite->sBattler)][(sprite->data[3] << 8)];

            for (i = 0; i < 0x100; i++)
                *(dst++) = 0;

            StartSpriteAnim(sprite, 0);
        }
    }
}

// Used when selecting a move, which can hit multiple targets, in double battles.
void SpriteCB_ShowAsMoveTarget(struct Sprite *sprite)
{
    sprite->data[3] = 8;
    sprite->data[4] = sprite->invisible;
    sprite->callback = SpriteCB_BlinkVisible;
}

static void SpriteCB_BlinkVisible(struct Sprite *sprite)
{
    if (--sprite->data[3] == 0)
    {
        sprite->invisible ^= 1;
        sprite->data[3] = 8;
    }
}

void SpriteCB_HideAsMoveTarget(struct Sprite *sprite)
{
    sprite->invisible = sprite->data[4];
    sprite->data[4] = FALSE;
    sprite->callback = SpriteCallbackDummy;
}

void SpriteCB_OpponentMonFromBall(struct Sprite *sprite)
{
    if (sprite->affineAnimEnded)
    {
        if (!(gHitMarker & HITMARKER_NO_ANIMATIONS))
            StartSpriteAnim(sprite, 1);
        BattleAnimateFrontSprite(sprite, sprite->sSpeciesId, TRUE, 1);
    }
}

// This callback is frequently overwritten by SpriteCB_TrainerSlideIn
void SpriteCB_BattleSpriteStartSlideLeft(struct Sprite *sprite)
{
    sprite->callback = SpriteCB_BattleSpriteSlideLeft;
}

static void SpriteCB_BattleSpriteSlideLeft(struct Sprite *sprite)
{
    if (!(gIntroSlideFlags & 1))
    {
        sprite->x2 -= 2;
        if (sprite->x2 == 0)
        {
            sprite->callback = SpriteCB_Idle;
            sprite->data[1] = 0;
        }
    }
}

static void SpriteCB_Idle(struct Sprite *sprite)
{
}

#define sSpeedX data[1]
#define sSpeedY data[2]

void SpriteCB_FaintSlideAnim(struct Sprite *sprite)
{
    if (!(gIntroSlideFlags & 1))
    {
        sprite->x2 += sprite->sSpeedX;
        sprite->y2 += sprite->sSpeedY;
    }
}

#undef sSpeedX
#undef sSpeedY

#define sSinIndex           data[3]
#define sDelta              data[4]
#define sAmplitude          data[5]
#define sBouncerSpriteId    data[6]
#define sWhich              data[7]

void DoBounceEffect(u8 battler, u8 which, s8 delta, s8 amplitude)
{
    u8 invisibleSpriteId;
    u8 bouncerSpriteId;

    switch (which)
    {
    case BOUNCE_HEALTHBOX:
    default:
        if (gBattleSpritesDataPtr->healthBoxesData[battler].healthboxIsBouncing)
            return;
        break;
    case BOUNCE_MON:
        if (gBattleSpritesDataPtr->healthBoxesData[battler].battlerIsBouncing)
            return;
        break;
    }

    invisibleSpriteId = CreateInvisibleSpriteWithCallback(SpriteCB_BounceEffect);
    if (which == BOUNCE_HEALTHBOX)
    {
        bouncerSpriteId = gHealthboxSpriteIds[battler];
        gBattleSpritesDataPtr->healthBoxesData[battler].healthboxBounceSpriteId = invisibleSpriteId;
        gBattleSpritesDataPtr->healthBoxesData[battler].healthboxIsBouncing = 1;
        gSprites[invisibleSpriteId].sSinIndex = 128; // 0
    }
    else
    {
        bouncerSpriteId = gBattlerSpriteIds[battler];
        gBattleSpritesDataPtr->healthBoxesData[battler].battlerBounceSpriteId = invisibleSpriteId;
        gBattleSpritesDataPtr->healthBoxesData[battler].battlerIsBouncing = 1;
        gSprites[invisibleSpriteId].sSinIndex = 192; // -1
    }
    gSprites[invisibleSpriteId].sDelta = delta;
    gSprites[invisibleSpriteId].sAmplitude = amplitude;
    gSprites[invisibleSpriteId].sBouncerSpriteId = bouncerSpriteId;
    gSprites[invisibleSpriteId].sWhich = which;
    gSprites[invisibleSpriteId].sBattler = battler;
    gSprites[bouncerSpriteId].x2 = 0;
    gSprites[bouncerSpriteId].y2 = 0;
}

void EndBounceEffect(u8 battler, u8 which)
{
    u8 bouncerSpriteId;

    if (which == BOUNCE_HEALTHBOX)
    {
        if (!gBattleSpritesDataPtr->healthBoxesData[battler].healthboxIsBouncing)
            return;

        bouncerSpriteId = gSprites[gBattleSpritesDataPtr->healthBoxesData[battler].healthboxBounceSpriteId].sBouncerSpriteId;
        DestroySprite(&gSprites[gBattleSpritesDataPtr->healthBoxesData[battler].healthboxBounceSpriteId]);
        gBattleSpritesDataPtr->healthBoxesData[battler].healthboxIsBouncing = 0;
    }
    else
    {
        if (!gBattleSpritesDataPtr->healthBoxesData[battler].battlerIsBouncing)
            return;

        bouncerSpriteId = gSprites[gBattleSpritesDataPtr->healthBoxesData[battler].battlerBounceSpriteId].sBouncerSpriteId;
        DestroySprite(&gSprites[gBattleSpritesDataPtr->healthBoxesData[battler].battlerBounceSpriteId]);
        gBattleSpritesDataPtr->healthBoxesData[battler].battlerIsBouncing = 0;
    }

    gSprites[bouncerSpriteId].x2 = 0;
    gSprites[bouncerSpriteId].y2 = 0;
}

static void SpriteCB_BounceEffect(struct Sprite *sprite)
{
    u8 bouncerSpriteId = sprite->sBouncerSpriteId;
    s32 index = sprite->sSinIndex;
    s32 y = Sin(index, sprite->sAmplitude) + sprite->sAmplitude;

    gSprites[bouncerSpriteId].y2 = y;
    sprite->sSinIndex = (sprite->sSinIndex + sprite->sDelta) & 0xFF;
}

#undef sSinIndex
#undef sDelta
#undef sAmplitude
#undef sBouncerSpriteId
#undef sWhich

void SpriteCB_PlayerMonFromBall(struct Sprite *sprite)
{
    if (sprite->affineAnimEnded)
        BattleAnimateBackSprite(sprite, sprite->sSpeciesId);
}

void SpriteCB_PlayerMonSlideIn(struct Sprite *sprite)
{
    if (sprite->data[3] == 0)
    {
        PlaySE(SE_BALL_TRAY_ENTER);
        sprite->data[3]++;
    }
    else if (sprite->data[3] == 1)
    {
        if (sprite->animEnded)
            return;
        sprite->data[4] = sprite->x;
        sprite->x = -33;
        sprite->invisible = FALSE;
        sprite->data[3]++;
    }
    else if (sprite->data[3] < 27)
    {
        sprite->x += 4;
        sprite->data[3]++;
    }
    else
    {
        sprite->data[3] = 0;
        sprite->x = sprite->data[4];
        sprite->data[4] = 0;
        sprite->callback = SpriteCB_PlayerMonFromBall;
        PlayCry_ByMode(sprite->sSpeciesId, -25, CRY_MODE_NORMAL);
    }
}

static void SpriteCB_TrainerThrowObject_Main(struct Sprite *sprite)
{
    AnimSetCenterToCornerVecX(sprite);
    if (sprite->animEnded)
        sprite->callback = SpriteCB_Idle;
}

// Sprite callback for a trainer back pic to throw an object
// (Wally throwing a ball, throwing Pokéblocks/balls in the Safari Zone)
void SpriteCB_TrainerThrowObject(struct Sprite *sprite)
{
    StartSpriteAnim(sprite, 1);
    sprite->callback = SpriteCB_TrainerThrowObject_Main;
}

void AnimSetCenterToCornerVecX(struct Sprite *sprite)
{
    if (sprite->animDelayCounter == 0)
        sprite->centerToCornerVecX = sCenterToCornerVecXs[sprite->animCmdIndex];
}

void BeginBattleIntroDummy(void)
{

}

void BeginBattleIntro(void)
{
    BattleStartClearSetData();
    gBattleCommunication[1] = 0;
    gBattleStruct->estadoIntro = 0;
    gBattleMainFunc = DoBattleIntro;
}

static void BattleMainCB1(void)
{
    u32 battler;

    gBattleMainFunc();
    for (battler = 0; battler < gBattlersCount; battler++)
        gBattlerControllerFuncs[battler](battler);
}

static void ClearSetBScriptingStruct(void)
{
    // windowsType is set up earlier in BattleInitBgsAndWindows, so we need to save the value
    u32 temp = gBattleScripting.windowsType;
    memset(&gBattleScripting, 0, sizeof(gBattleScripting));

    gBattleScripting.windowsType                = temp;
}

static void BattleStartClearSetData(void)
{
    s32 i;

    TurnValuesCleanUp(FALSE);
    SpecialStatusesClear();

    memset(&gDisableStructs, 0, sizeof(gDisableStructs));
    memset(&gFieldTimers, 0, sizeof(gFieldTimers));
    memset(&gSideStatuses, 0, sizeof(gSideStatuses));
    memset(&gSideTimers, 0, sizeof(gSideTimers));
    memset(&gWishFutureKnock, 0, sizeof(gWishFutureKnock));
    memset(&gBattleResults, 0, sizeof(gBattleResults));
    ClearSetBScriptingStruct();

    for (i = 0; i < MAX_BATTLERS_COUNT; i++)
    {
        gStatuses3[i] = 0;
        gStatuses4[i] = 0;
        gDisableStructs[i].isFirstTurn = 2;
        gLastMoves[i] = MOVE_NONE;
        gLastLandedMoves[i] = MOVE_NONE;
        gLastHitByType[i] = 0;
        gLastUsedMoveType[i] = 0;
        gLastResultingMoves[i] = MOVE_NONE;
        gLastHitBy[i] = 0xFF;
        gLockedMoves[i] = MOVE_NONE;
        gLastPrintedMoves[i] = MOVE_NONE;
        gBattleResources->flags->flags[i] = 0;
        gPalaceSelectionBattleScripts[i] = 0;
        gBattleStruct->lastTakenMove[i] = MOVE_NONE;
        gBattleStruct->choicedMove[i] = MOVE_NONE;
        gBattleStruct->changedItems[i] = 0;
        gBattleStruct->lastTakenMoveFrom[i][0] = MOVE_NONE;
        gBattleStruct->lastTakenMoveFrom[i][1] = MOVE_NONE;
        gBattleStruct->lastTakenMoveFrom[i][2] = MOVE_NONE;
        gBattleStruct->lastTakenMoveFrom[i][3] = MOVE_NONE;
        gBattleStruct->AI_monToSwitchIntoId[i] = PARTY_SIZE;
        gBattleStruct->skyDropTargets[i] = 0xFF;
        gBattleStruct->overwrittenAbilities[i] = ABILITY_NONE;
    }

    gLastUsedMove = 0;
    gFieldStatuses = 0;

    gLastUsedBall = 0;

    gBattlerAttacker = 0;
    gBattlerTarget = 0;
    gEffectBattler = 0;
    gBattlerAbility = 0;
    gBattleWeather = 0;
    gHitMarker = 0;

    if (gSaveBlockPtr->optionsBattleSceneOff == TRUE)
        gHitMarker |= HITMARKER_NO_ANIMATIONS;

    gMultiHitCounter = 0;
    gBattleOutcome = 0;
    gBattleControllerExecFlags = 0;
    gPaydayMoney = 0;
    gBattleResources->battleScriptsStack->size = 0;
    gBattleResources->battleCallbackStack->size = 0;

    for (i = 0; i < BATTLE_COMMUNICATION_ENTRIES_COUNT; i++)
        gBattleCommunication[i] = 0;

    gPauseCounterBattle = 0;
    gBattleMoveDamage = 0;
    gIntroSlideFlags = 0;
    gLeveledUpInBattle = 0;
    gAbsentBattlerFlags = 0;
    gBattleStruct->runTries = 0;
    gBattleStruct->safariGoNearCounter = 0;
    gBattleStruct->safariPkblThrowCounter = 0;
    gBattleStruct->safariCatchFactor = gSpeciesInfo[GetMonData(&gEnemyParty[0], MON_DATA_SPECIES)].catchRate * 100 / 1275;
    gBattleStruct->safariEscapeFactor = 3;
    gBattleStruct->wildVictorySong = 0;
    gBattleStruct->moneyMultiplier = 1;

    gBattleStruct->givenExpMons = 0;
    gBattleStruct->palaceFlags = 0;

    gBattleResults.shinyWildMon = IsMonShiny(&gEnemyParty[0]);

    gBattleStruct->arenaLostPlayerMons = 0;
    gBattleStruct->arenaLostOpponentMons = 0;

    for (i = 0; i < ARRAY_COUNT(gSideTimers); i++)
    {
        gSideTimers[i].stickyWebBattlerId = 0xFF;
    }
    gBattleStruct->appearedInBattle = 0;
    gBattleStruct->beatUpSlot = 0;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        gBattleStruct->usedHeldItems[i][B_SIDE_PLAYER] = 0;
        gBattleStruct->usedHeldItems[i][B_SIDE_OPPONENT] = 0;
        gBattleStruct->itemLost[B_SIDE_PLAYER][i].originalItem = GetMonData(&gPlayerParty[i], MON_DATA_HELD_ITEM);
        gBattleStruct->itemLost[B_SIDE_OPPONENT][i].originalItem = GetMonData(&gEnemyParty[i], MON_DATA_HELD_ITEM);
    }

    gBattleStruct->swapDamageCategory = FALSE; // Photon Geyser, Shell Side Arm, Light That Burns the Sky
    gBattleStruct->categoryOverride = FALSE; // used for Z-Moves and Max Moves

    gSelectedMonPartyId = PARTY_SIZE; // Revival Blessing
    gCategoryIconSpriteId = 0xFF;
}

void SwitchInClearSetData(u32 battler)
{
    s32 i;
    struct DisableStruct disableStructCopy = gDisableStructs[battler];

    ClearIllusionMon(battler);
    if (gMovesInfo[gCurrentMove].effect != EFFECT_BATON_PASS)
    {
        for (i = 0; i < NUMERO_ESTADISTICAS_BATALLA; i++)
            gBattleMons[battler].statStages[i] = ESTADISTICA_NEUTRA;
        for (i = 0; i < gBattlersCount; i++)
        {
            if ((gBattleMons[i].status2 & STATUS2_ESCAPE_PREVENTION) && gDisableStructs[i].battlerPreventingEscape == battler)
                gBattleMons[i].status2 &= ~STATUS2_ESCAPE_PREVENTION;
            if ((gStatuses3[i] & STATUS3_ALWAYS_HITS) && gDisableStructs[i].battlerWithSureHit == battler)
            {
                gStatuses3[i] &= ~STATUS3_ALWAYS_HITS;
                gDisableStructs[i].battlerWithSureHit = 0;
            }
        }
    }
    if (gMovesInfo[gCurrentMove].effect == EFFECT_BATON_PASS)
    {
        gBattleMons[battler].status2 &= (STATUS2_CONFUSION | STATUS2_FOCUS_ENERGY_ANY | STATUS2_SUBSTITUTE | STATUS2_ESCAPE_PREVENTION | STATUS2_CURSED);
        gStatuses3[battler] &= (STATUS3_LEECHSEED_BATTLER | STATUS3_LEECHSEED | STATUS3_ALWAYS_HITS | STATUS3_PERISH_SONG | STATUS3_ROOTED
                                       | STATUS3_GASTRO_ACID | STATUS3_EMBARGO | STATUS3_TELEKINESIS | STATUS3_MAGNET_RISE | STATUS3_HEAL_BLOCK
                                       | STATUS3_AQUA_RING | STATUS3_POWER_TRICK);
        gStatuses4[battler] &= (STATUS4_MUD_SPORT | STATUS4_WATER_SPORT | STATUS4_INFINITE_CONFUSION);
        for (i = 0; i < gBattlersCount; i++)
        {
            if (GetBattlerSide(battler) != GetBattlerSide(i)
             && (gStatuses3[i] & STATUS3_ALWAYS_HITS) != 0
             && (gDisableStructs[i].battlerWithSureHit == battler))
            {
                gStatuses3[i] &= ~STATUS3_ALWAYS_HITS;
                gStatuses3[i] |= STATUS3_ALWAYS_HITS_TURN(2);
            }
        }
        if (gStatuses3[battler] & STATUS3_POWER_TRICK)
            SWAP(gBattleMons[battler].attack, gBattleMons[battler].defense, i);
    }
    else
    {
        gBattleMons[battler].status2 = 0;
        gStatuses3[battler] = 0;
        gStatuses4[battler] = 0;
    }

    for (i = 0; i < gBattlersCount; i++)
    {
        if (gBattleMons[i].status2 & STATUS2_INFATUATED_WITH(battler))
            gBattleMons[i].status2 &= ~STATUS2_INFATUATED_WITH(battler);
        if ((gBattleMons[i].status2 & STATUS2_WRAPPED) && *(gBattleStruct->wrappedBy + i) == battler)
            gBattleMons[i].status2 &= ~STATUS2_WRAPPED;
        if ((gStatuses4[i] & STATUS4_SYRUP_BOMB) && *(gBattleStruct->stickySyrupdBy + i) == battler)
            gStatuses4[i] &= ~STATUS4_SYRUP_BOMB;
    }

    gActionSelectionCursor[battler] = 0;
    gMoveSelectionCursor[battler] = 0;

    memset(&gDisableStructs[battler], 0, sizeof(struct DisableStruct));

    if (gMovesInfo[gCurrentMove].effect == EFFECT_BATON_PASS)
    {
        gDisableStructs[battler].substituteHP = disableStructCopy.substituteHP;
        gDisableStructs[battler].battlerWithSureHit = disableStructCopy.battlerWithSureHit;
        gDisableStructs[battler].perishSongTimer = disableStructCopy.perishSongTimer;
        gDisableStructs[battler].battlerPreventingEscape = disableStructCopy.battlerPreventingEscape;
        gDisableStructs[battler].embargoTimer = disableStructCopy.embargoTimer;
    }
    else if (gMovesInfo[gCurrentMove].effect == EFFECT_SHED_TAIL)
    {
        gBattleMons[battler].status2 |= STATUS2_SUBSTITUTE;
        gDisableStructs[battler].substituteHP = disableStructCopy.substituteHP;
    }

    gMoveResultFlags = 0;
    gDisableStructs[battler].isFirstTurn = 2;
    gDisableStructs[battler].truantSwitchInHack = disableStructCopy.truantSwitchInHack;
    gLastMoves[battler] = MOVE_NONE;
    gLastLandedMoves[battler] = MOVE_NONE;
    gLastHitByType[battler] = 0;
    gLastUsedMoveType[battler] = 0;
    gLastResultingMoves[battler] = MOVE_NONE;
    gLastPrintedMoves[battler] = MOVE_NONE;
    gLastHitBy[battler] = 0xFF;

    gBattleStruct->lastTakenMove[battler] = 0;
    gBattleStruct->sameMoveTurns[battler] = 0;
    gBattleStruct->lastTakenMoveFrom[battler][0] = 0;
    gBattleStruct->lastTakenMoveFrom[battler][1] = 0;
    gBattleStruct->lastTakenMoveFrom[battler][2] = 0;
    gBattleStruct->lastTakenMoveFrom[battler][3] = 0;
    gBattleStruct->lastMoveFailed &= ~(1u << battler);
    gBattleStruct->palaceFlags &= ~(1u << battler);
    gBattleStruct->boosterEnergyActivates &= ~(1u << battler);
    gBattleStruct->canPickupItem &= ~(1u << battler);

    for (i = 0; i < ARRAY_COUNT(gSideTimers); i++)
    {
        // Switched into sticky web user slot, so reset stored battler ID
        if (gSideTimers[i].stickyWebBattlerId == battler)
            gSideTimers[i].stickyWebBattlerId = 0xFF;
    }

    for (i = 0; i < gBattlersCount; i++)
    {
        if (i != battler && GetBattlerSide(i) != GetBattlerSide(battler))
            gBattleStruct->lastTakenMove[i] = MOVE_NONE;

        gBattleStruct->lastTakenMoveFrom[i][battler] = 0;
    }

    gBattleStruct->choicedMove[battler] = MOVE_NONE;
    gBattleResources->flags->flags[battler] = 0;
    gCurrentMove = MOVE_NONE;
    gBattleStruct->arenaTurnCounter = 0xFF;

    // Reset damage to prevent things like red card activating if the switched-in mon is holding it
    gSpecialStatuses[battler].physicalDmg = 0;
    gSpecialStatuses[battler].specialDmg = 0;
    gBattleStruct->enduredDamage &= ~(1u << battler);

    // Reset Eject Button / Eject Pack switch detection
    AI_DATA->ejectButtonSwitch = FALSE;
    AI_DATA->ejectPackSwitch = FALSE;

    // Reset G-Max Chi Strike boosts.
    gBattleStruct->bonusCritStages[battler] = 0;

    gBattleStruct->overwrittenAbilities[battler] = ABILITY_NONE;

    // Clear selected party ID so Revival Blessing doesn't get confused.
    gSelectedMonPartyId = PARTY_SIZE;

    Ai_UpdateSwitchInData(battler);
}

const u8* FaintClearSetData(u32 battler)
{
    s32 i;
    const u8 *result = NULL;
    u8 battlerSide = GetBattlerSide(battler);

    for (i = 0; i < NUMERO_ESTADISTICAS_BATALLA; i++)
        gBattleMons[battler].statStages[i] = ESTADISTICA_NEUTRA;

    gBattleMons[battler].status2 = 0;
    gStatuses3[battler] &= STATUS3_GASTRO_ACID; // Edge case: Keep Gastro Acid if pokemon's ability can have effect after fainting, for example Innards Out.
    gStatuses4[battler] = 0;

    for (i = 0; i < gBattlersCount; i++)
    {
        if ((gBattleMons[i].status2 & STATUS2_ESCAPE_PREVENTION) && gDisableStructs[i].battlerPreventingEscape == battler)
            gBattleMons[i].status2 &= ~STATUS2_ESCAPE_PREVENTION;
        if (gBattleMons[i].status2 & STATUS2_INFATUATED_WITH(battler))
            gBattleMons[i].status2 &= ~STATUS2_INFATUATED_WITH(battler);
        if ((gBattleMons[i].status2 & STATUS2_WRAPPED) && *(gBattleStruct->wrappedBy + i) == battler)
            gBattleMons[i].status2 &= ~STATUS2_WRAPPED;
        if ((gStatuses4[i] & STATUS4_SYRUP_BOMB) && *(gBattleStruct->stickySyrupdBy + i) == battler)
            gStatuses4[i] &= ~STATUS4_SYRUP_BOMB;
    }

    gActionSelectionCursor[battler] = 0;
    gMoveSelectionCursor[battler] = 0;

    memset(&gDisableStructs[battler], 0, sizeof(struct DisableStruct));

    gProtectStructs[battler].protected = FALSE;
    gProtectStructs[battler].spikyShielded = FALSE;
    gProtectStructs[battler].kingsShielded = FALSE;
    gProtectStructs[battler].banefulBunkered = FALSE;
    gProtectStructs[battler].quash = FALSE;
    gProtectStructs[battler].obstructed = FALSE;
    gProtectStructs[battler].silkTrapped = FALSE;
    gProtectStructs[battler].burningBulwarked = FALSE;
    gProtectStructs[battler].endured = FALSE;
    gProtectStructs[battler].noValidMoves = FALSE;
    gProtectStructs[battler].helpingHand = FALSE;
    gProtectStructs[battler].bounceMove = FALSE;
    gProtectStructs[battler].stealMove = FALSE;
    gProtectStructs[battler].prlzImmobility = FALSE;
    gProtectStructs[battler].sleepImmobility = FALSE;
    gProtectStructs[battler].confusionSelfDmg = FALSE;
    gProtectStructs[battler].targetAffected = FALSE;
    gProtectStructs[battler].chargingTurn = FALSE;
    gProtectStructs[battler].fleeType = 0;
    gProtectStructs[battler].usedImprisonedMove = FALSE;
    gProtectStructs[battler].loveImmobility = FALSE;
    gProtectStructs[battler].usedDisabledMove = FALSE;
    gProtectStructs[battler].usedTauntedMove = FALSE;
    gProtectStructs[battler].flag2Unknown = FALSE;
    gProtectStructs[battler].flinchImmobility = FALSE;
    gProtectStructs[battler].notFirstStrike = FALSE;
    gProtectStructs[battler].usedHealBlockedMove = FALSE;
    gProtectStructs[battler].usedGravityPreventedMove = FALSE;
    gProtectStructs[battler].usedThroatChopPreventedMove = FALSE;
    gProtectStructs[battler].statRaised = FALSE;
    gProtectStructs[battler].statFell = FALSE;
    gProtectStructs[battler].pranksterElevated = FALSE;

    gDisableStructs[battler].isFirstTurn = 2;

    gLastMoves[battler] = MOVE_NONE;
    gLastLandedMoves[battler] = MOVE_NONE;
    gLastHitByType[battler] = 0;
    gLastUsedMoveType[battler] = 0;
    gLastResultingMoves[battler] = MOVE_NONE;
    gLastPrintedMoves[battler] = MOVE_NONE;
    gLastHitBy[battler] = 0xFF;

    gBattleStruct->choicedMove[battler] = MOVE_NONE;
    gBattleStruct->sameMoveTurns[battler] = 0;
    gBattleStruct->lastTakenMove[battler] = MOVE_NONE;
    gBattleStruct->lastTakenMoveFrom[battler][0] = 0;
    gBattleStruct->lastTakenMoveFrom[battler][1] = 0;
    gBattleStruct->lastTakenMoveFrom[battler][2] = 0;
    gBattleStruct->lastTakenMoveFrom[battler][3] = 0;

    gBattleStruct->palaceFlags &= ~(1u << battler);
    gBattleStruct->boosterEnergyActivates &= ~(1u << battler);

    if (gBattleStruct->commanderActive[battler] != SPECIES_NONE)
    {
        u32 partner = BATTLE_PARTNER(battler);
        if (IsBattlerAlive(partner))
        {
            BtlController_EmitSpriteInvisibility(partner, BUFFER_A, FALSE);
            MarkBattlerForControllerExec(partner);
        }
    }

    for (i = 0; i < ARRAY_COUNT(gSideTimers); i++)
    {
        // User of sticky web fainted, so reset the stored battler ID
        if (gSideTimers[i].stickyWebBattlerId == battler)
            gSideTimers[i].stickyWebBattlerId = 0xFF;
    }

    for (i = 0; i < gBattlersCount; i++)
    {
        if (i != battler && GetBattlerSide(i) != battlerSide)
            gBattleStruct->lastTakenMove[i] = MOVE_NONE;

        gBattleStruct->lastTakenMoveFrom[i][battler] = 0;
    }

    gBattleResources->flags->flags[battler] = 0;

    gBattleMons[battler].types[0] = gSpeciesInfo[gBattleMons[battler].species].types[0];
    gBattleMons[battler].types[1] = gSpeciesInfo[gBattleMons[battler].species].types[1];
    gBattleMons[battler].types[2] = TIPO_MISTERIO;

    Ai_UpdateFaintData(battler);
    TryBattleFormChange(battler, FORM_CHANGE_FAINT);

    gBattleStruct->overwrittenAbilities[battler] = ABILITY_NONE;

    // If the fainted mon was involved in a Sky Drop
    if (gBattleStruct->skyDropTargets[battler] != 0xFF)
    {
        // Get battler id of the other Pokemon involved in this Sky Drop
        u8 otherSkyDropper = gBattleStruct->skyDropTargets[battler];

        // Clear Sky Drop data
        gBattleStruct->skyDropTargets[battler] = 0xFF;
        gBattleStruct->skyDropTargets[otherSkyDropper] = 0xFF;

        // If the other Pokemon involved in this Sky Drop was the target, not the attacker
        if (gStatuses3[otherSkyDropper] & STATUS3_SKY_DROPPED)
        {
            // Release the target and take them out of the semi-invulnerable state
            gStatuses3[otherSkyDropper] &= ~(STATUS3_SKY_DROPPED | STATUS3_ON_AIR);

            // Make the target's sprite visible
            gSprites[gBattlerSpriteIds[otherSkyDropper]].invisible = FALSE;

            // If the target was sky dropped in the middle of using Outrage/Petal Dance/Thrash,
            // confuse them upon release and print "confused via fatigue" message and animation.
            if (gBattleMons[otherSkyDropper].status2 & STATUS2_LOCK_CONFUSE)
            {
                gBattleMons[otherSkyDropper].status2 &= ~(STATUS2_LOCK_CONFUSE);

                // If the released mon can be confused, do so.
                // Don't use CanBeConfused here, since it can cause issues in edge cases.
                if (!(GetBattlerAbility(otherSkyDropper) == ABILITY_OWN_TEMPO
                    || gBattleMons[otherSkyDropper].status2 & STATUS2_CONFUSION
                    || IsBattlerTerrainAffected(otherSkyDropper, STATUS_FIELD_MISTY_TERRAIN)))
                {
                    gBattleMons[otherSkyDropper].status2 |= STATUS2_CONFUSION_TURN(((Random()) % 4) + 2);
                    gBattlerAttacker = otherSkyDropper;
                    result = BattleScript_ThrashConfuses;
                }
            }
        }
    }
    return result;
}

static void DoBattleIntro(void)
{
    s32 i;
    u32 battler;

    switch (gBattleStruct->estadoIntro)
    {
    case BATTLE_INTRO_STATE_GET_MON_DATA:
        battler = gBattleCommunication[1];
        BtlController_EmitGetMonData(battler, BUFFER_A, REQUEST_ALL_BATTLE, 0);
        MarkBattlerForControllerExec(battler);
        gBattleStruct->estadoIntro++;
        break;
    case BATTLE_INTRO_STATE_LOOP_BATTLER_DATA:
        if (!gBattleControllerExecFlags)
        {
            if (++gBattleCommunication[1] == gBattlersCount)
                gBattleStruct->estadoIntro++;
            else
                gBattleStruct->estadoIntro = BATTLE_INTRO_STATE_GET_MON_DATA;
        }
        break;
    case BATTLE_INTRO_STATE_PREPARE_BG_SLIDE:
        if (!gBattleControllerExecFlags)
        {
            battler = GetBattlerAtPosition(0);
            BtlController_EmitIntroSlide(battler, BUFFER_A, gBattleTerrain);
            MarkBattlerForControllerExec(battler);
            gBattleCommunication[0] = 0;
            gBattleCommunication[1] = 0;
            gBattleStruct->estadoIntro++;
        }
        break;
    case BATTLE_INTRO_STATE_WAIT_FOR_BG_SLIDE:
        if (!gBattleControllerExecFlags)
            gBattleStruct->estadoIntro++;
        break;
    case BATTLE_INTRO_STATE_DRAW_SPRITES:
        for (battler = 0; battler < gBattlersCount; battler++)
        {
            memcpy(&gBattleMons[battler], &gBattleResources->bufferB[battler][4], sizeof(struct BattlePokemon));
            gBattleMons[battler].types[0] = gSpeciesInfo[gBattleMons[battler].species].types[0];
            gBattleMons[battler].types[1] = gSpeciesInfo[gBattleMons[battler].species].types[1];
            gBattleMons[battler].types[2] = TIPO_MISTERIO;
            gBattleMons[battler].ability = GetAbilityBySpecies(gBattleMons[battler].species, gBattleMons[battler].abilityNum);
            gBattleStruct->hpOnSwitchout[GetBattlerSide(battler)] = gBattleMons[battler].hp;
            gBattleMons[battler].status2 = 0;
            for (i = 0; i < NUMERO_ESTADISTICAS_BATALLA; i++)
                gBattleMons[battler].statStages[i] = ESTADISTICA_NEUTRA;

            // Draw sprite.
            switch (GetBattlerPosition(battler))
            {
            case B_POSITION_PLAYER_LEFT: // player sprite
                BtlController_EmitDrawTrainerPic(battler, BUFFER_A);
                MarkBattlerForControllerExec(battler);
                break;
            case B_POSITION_OPPONENT_LEFT:
                if (EsContraEntrenador()) // opponent 1 sprite
                {
                    BtlController_EmitDrawTrainerPic(battler, BUFFER_A);
                    MarkBattlerForControllerExec(battler);
                }
                else // wild mon 1
                {
                    BtlController_EmitLoadMonSprite(battler, BUFFER_A);
                    MarkBattlerForControllerExec(battler);
                    gBattleResults.lastOpponentSpecies = GetMonData(&gEnemyParty[gBattlerPartyIndexes[battler]], MON_DATA_SPECIES, NULL);
                }
                break;
            case B_POSITION_PLAYER_RIGHT:
                break;
            case B_POSITION_OPPONENT_RIGHT:
                if (!(EsContraEntrenador()) && IsBattlerAlive(battler))
                {
                    BtlController_EmitLoadMonSprite(battler, BUFFER_A);
                    MarkBattlerForControllerExec(battler);
                    gBattleResults.lastOpponentSpecies = GetMonData(&gEnemyParty[gBattlerPartyIndexes[battler]], MON_DATA_SPECIES, NULL);
                }
                break;
            }
        }

        if (EsContraEntrenador())
            gBattleStruct->estadoIntro++;
        else // Skip party summary since it is a wild battle.
            gBattleStruct->estadoIntro = BATTLE_INTRO_STATE_INTRO_TEXT; // Don't wait for sprite, print message at the same time.
        break;
    case BATTLE_INTRO_STATE_DRAW_PARTY_SUMMARY:
        if (!gBattleControllerExecFlags)
        {
            struct HpAndStatus hpStatus[PARTY_SIZE];

            for (i = 0; i < PARTY_SIZE; i++)
            {
                if (GetMonData(&gEnemyParty[i], MON_DATA_SPECIES_OR_EGG) == SPECIES_NONE || GetMonData(&gEnemyParty[i], MON_DATA_SPECIES_OR_EGG) == SPECIES_EGG)
                {
                    hpStatus[i].hp = HP_EMPTY_SLOT;
                    hpStatus[i].status = 0;
                }
                else
                {
                    hpStatus[i].hp = GetMonData(&gEnemyParty[i], MON_DATA_HP);
                    hpStatus[i].status = GetMonData(&gEnemyParty[i], MON_DATA_STATUS);
                }
            }

            battler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
            BtlController_EmitDrawPartyStatusSummary(battler, BUFFER_A, hpStatus, PARTY_SUMM_SKIP_DRAW_DELAY);
            MarkBattlerForControllerExec(battler);

            for (i = 0; i < PARTY_SIZE; i++)
            {
                if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG) == SPECIES_NONE || GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG) == SPECIES_EGG)
                {
                    hpStatus[i].hp = HP_EMPTY_SLOT;
                    hpStatus[i].status = 0;
                }
                else
                {
                    hpStatus[i].hp = GetMonData(&gPlayerParty[i], MON_DATA_HP);
                    hpStatus[i].status = GetMonData(&gPlayerParty[i], MON_DATA_STATUS);
                }
            }

            battler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
            BtlController_EmitDrawPartyStatusSummary(battler, BUFFER_A, hpStatus, PARTY_SUMM_SKIP_DRAW_DELAY);
            MarkBattlerForControllerExec(battler);

            gBattleStruct->estadoIntro++;
        }
        break;
    case BATTLE_INTRO_STATE_WAIT_FOR_PARTY_SUMMARY:
        if (!gBattleControllerExecFlags)
            gBattleStruct->estadoIntro++;
        break;
    case BATTLE_INTRO_STATE_INTRO_TEXT:
        if (!IsBattlerMarkedForControllerExec(GetBattlerAtPosition(B_POSITION_PLAYER_LEFT)))
        {
            PrepareStringBattle(TEXTO_BATALLA_INTRO, GetBattlerAtPosition(B_POSITION_PLAYER_LEFT));
            gBattleStruct->estadoIntro++;
        }
        break;
    case BATTLE_INTRO_STATE_WAIT_FOR_INTRO_TEXT:
        if (!IsBattlerMarkedForControllerExec(GetBattlerAtPosition(B_POSITION_PLAYER_LEFT)))
        {
            if (EsContraEntrenador())
                gBattleStruct->estadoIntro++;
            else
                gBattleStruct->estadoIntro = BATTLE_INTRO_STATE_WAIT_FOR_WILD_BATTLE_TEXT;
        }
        break;
    case ESTADO_INTRO_BATALLA_ENTRENADOR_TEXTO_ENVIAR_POKEMON:
        PrepareStringBattle(TEXTO_BATALLA_ENVIAR_POKEMON, GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT));
        gBattleStruct->estadoIntro++;
        break;
    case BATTLE_INTRO_STATE_WAIT_FOR_TRAINER_SEND_OUT_TEXT:
        if (!gBattleControllerExecFlags)
            gBattleStruct->estadoIntro++;
        break;
    case BATTLE_INTRO_STATE_TRAINER_SEND_OUT_ANIM:
        battler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        BtlController_EmitIntroTrainerBallThrow(battler, BUFFER_A);
        MarkBattlerForControllerExec(battler);
        gBattleStruct->estadoIntro++;
        break;
    case BATTLE_INTRO_STATE_WAIT_FOR_WILD_BATTLE_TEXT:
        if (!IsBattlerMarkedForControllerExec(GetBattlerAtPosition(B_POSITION_PLAYER_LEFT)))
            gBattleStruct->estadoIntro++;
        break;
    case ESTADO_INTRO_BATALLA_JUGADOR_TEXTO_ENVIAR_POKEMON:
        battler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        PrepareStringBattle(TEXTO_BATALLA_ENVIAR_POKEMON, battler);
        BtlController_EmitIntroTrainerBallThrow(battler, BUFFER_A);
        MarkBattlerForControllerExec(battler);
        gBattleStruct->estadoIntro++;
        break;
    case BATTLE_INTRO_STATE_WAIT_FOR_PLAYER_SEND_OUT_TEXT:
        battler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        if (!IsBattlerMarkedForControllerExec(battler))
            gBattleStruct->estadoIntro++;
        break;
    case BATTLE_INTRO_STATE_SET_DEX_AND_BATTLE_VARS:
        if (!gBattleControllerExecFlags)
        {
            for (battler = 0; battler < gBattlersCount; battler++)
            {
                if (GetBattlerSide(battler) == B_SIDE_OPPONENT)
                {
                    HandleSetPokedexFlag(SpeciesToNationalPokedexNum(gBattleMons[battler].species), FLAG_SET_SEEN);
                }
            }

            gBattleStruct->eventsBeforeFirstTurnState = 0;
            gBattleStruct->switchInBattlerCounter = 0;
            gBattleStruct->overworldWeatherDone = FALSE;
            Ai_InitPartyStruct(); // Save mons party counts, and first 2/4 mons on the battlefield.

            // Try to set a status to start the battle with
            gBattleStruct->startingStatus = 0;
            if (EsContraEntrenador() && GetTrainerStartingStatusFromId(gTrainerBattleOpponent_A))
            {
                gBattleStruct->startingStatus = GetTrainerStartingStatusFromId(gTrainerBattleOpponent_A);
                gBattleStruct->startingStatusTimer = 0; // infinite
            }
            else if (B_VAR_STARTING_STATUS != 0)
            {
                gBattleStruct->startingStatus = VarGet(B_VAR_STARTING_STATUS);
                gBattleStruct->startingStatusTimer = VarGet(B_VAR_STARTING_STATUS_TIMER);
            }
            gBattleMainFunc = TryDoEventsBeforeFirstTurn;
        }
        break;
    }
}

static void TryDoEventsBeforeFirstTurn(void)
{
    s32 i, j;

    if (gBattleControllerExecFlags)
        return;

    switch (gBattleStruct->eventsBeforeFirstTurnState)
    {
    case FIRST_TURN_EVENTS_START:
        // Set invalid mons as absent(for example when starting a double battle with only one pokemon).
        for (i = 0; i < gBattlersCount; i++)
        {
            struct Pokemon *party = GetBattlerParty(i);
            struct Pokemon *mon = &party[gBattlerPartyIndexes[i]];
            if (!IsBattlerAlive(i) || gBattleMons[i].species == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
                gAbsentBattlerFlags |= 1u << i;
        }

        gBattleStruct->speedTieBreaks = RandomUniform(RNG_SPEED_TIE, 0, Factorial(MAX_BATTLERS_COUNT) - 1);

        for (i = 0; i < gBattlersCount; i++)
            gBattlerByTurnOrder[i] = i;
        for (i = 0; i < gBattlersCount - 1; i++)
        {
            for (j = i + 1; j < gBattlersCount; j++)
            {
                if (GetWhichBattlerFaster(gBattlerByTurnOrder[i], gBattlerByTurnOrder[j], TRUE) == -1)
                    SwapTurnOrder(i, j);
            }
        }
        gBattleStruct->eventsBeforeFirstTurnState++;
        break;
    case FIRST_TURN_EVENTS_OVERWORLD_WEATHER:
        if (!gBattleStruct->overworldWeatherDone
         && AbilityBattleEffects(ABILITYEFFECT_SWITCH_IN_WEATHER, 0, 0, ABILITYEFFECT_SWITCH_IN_WEATHER, 0) != 0)
        {
            gBattleStruct->overworldWeatherDone = TRUE;
            return;
        }
        gBattleStruct->eventsBeforeFirstTurnState++;
        break;
    case FIRST_TURN_EVENTS_TERRAIN:
        if (!gBattleStruct->terrainDone
         && AbilityBattleEffects(ABILITYEFFECT_SWITCH_IN_TERRAIN, 0, 0, ABILITYEFFECT_SWITCH_IN_TERRAIN, 0) != 0)
        {
            gBattleStruct->terrainDone = TRUE;
            return;
        }
        gBattleStruct->eventsBeforeFirstTurnState++;
        break;
    case FIRST_TURN_EVENTS_STARTING_STATUS:
        if (!gBattleStruct->startingStatusDone
         && gBattleStruct->startingStatus
         && AbilityBattleEffects(ABILITYEFFECT_SWITCH_IN_STATUSES, 0, 0, ABILITYEFFECT_SWITCH_IN_STATUSES, 0) != 0)
        {
            gBattleStruct->startingStatusDone = TRUE;
            return;
        }
        gBattleStruct->eventsBeforeFirstTurnState++;
        break;
    case FIRST_TURN_EVENTS_NEUTRALIZING_GAS:
        if (AbilityBattleEffects(ABILITYEFFECT_NEUTRALIZINGGAS, 0, 0, 0, 0) != 0)
            return;
        gBattleStruct->eventsBeforeFirstTurnState++;
        break;
    case FIRST_TURN_EVENTS_SWITCH_IN_ABILITIES:
        while (gBattleStruct->switchInBattlerCounter < gBattlersCount) // From fastest to slowest
        {
            i = gBattlerByTurnOrder[gBattleStruct->switchInBattlerCounter++];

            if (AbilityBattleEffects(ABILITYEFFECT_ON_SWITCHIN, i, 0, 0, 0) != 0)
                return;
        }
        gBattleStruct->switchInBattlerCounter = 0;
        gBattleStruct->eventsBeforeFirstTurnState++;
        break;
    case FIRST_TURN_EVENTS_OPPORTUNIST_1:
        if (AbilityBattleEffects(ABILITYEFFECT_OPPORTUNIST, 0, 0, 0, 0))
            return;
        gBattleStruct->eventsBeforeFirstTurnState++;
        break;
    case FIRST_TURN_EVENTS_ITEM_EFFECTS:
        while (gBattleStruct->switchInBattlerCounter < gBattlersCount) // From fastest to slowest
        {
            if (ItemBattleEffects(ITEMEFFECT_ON_SWITCH_IN, gBattlerByTurnOrder[gBattleStruct->switchInBattlerCounter++], FALSE))
                return;
        }
        gBattleStruct->switchInBattlerCounter = 0;
        gBattleStruct->eventsBeforeFirstTurnState++;
        break;
    case FIRST_TURN_EVENTS_OPPORTUNIST_2:
        if (AbilityBattleEffects(ABILITYEFFECT_OPPORTUNIST, 0, 0, 0, 0))
            return;
        gBattleStruct->eventsBeforeFirstTurnState++;
        break;
    case FIRST_TURN_EVENTS_END:
        for (i = 0; i < MAX_BATTLERS_COUNT; i++)
        {
            *(gBattleStruct->monToSwitchIntoId + i) = PARTY_SIZE;
            gChosenActionByBattler[i] = B_ACTION_NONE;
            gChosenMoveByBattler[i] = MOVE_NONE;
        }
        TurnValuesCleanUp(FALSE);
        SpecialStatusesClear();
        *(&gBattleStruct->absentBattlerFlags) = gAbsentBattlerFlags;
        BattlePutTextOnWindow(gText_EmptyString, B_WIN_MSG);
        gBattleMainFunc = HandleTurnActionSelectionState;
        ResetSentPokesToOpponentValue();

        for (i = 0; i < BATTLE_COMMUNICATION_ENTRIES_COUNT; i++)
            gBattleCommunication[i] = 0;

        for (i = 0; i < gBattlersCount; i++)
        {
            gBattleMons[i].status2 &= ~STATUS2_FLINCHED;
            // Record party slots of player's mons that appeared in battle
            if (!BattlerHasAi(i))
                gBattleStruct->appearedInBattle |= 1u << gBattlerPartyIndexes[i];
        }

        *(&gBattleStruct->turnEffectsTracker) = 0;
        *(&gBattleStruct->turnEffectsBattlerId) = 0;
        *(&gBattleStruct->wishPerishSongState) = 0;
        *(&gBattleStruct->wishPerishSongBattlerId) = 0;
        gBattleScripting.moveendState = 0;
        gBattleStruct->faintedActionsState = 0;
        gBattleStruct->turnCountersTracker = 0;
        gMoveResultFlags = 0;

        memset(gQueuedStatBoosts, 0, sizeof(gQueuedStatBoosts));
        SetShellSideArmCategory();
        SetAiLogicDataForTurn(AI_DATA); // get assumed abilities, hold effects, etc of all battlers

        if ((i = ShouldDoTrainerSlide(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), TRAINER_SLIDE_BEFORE_FIRST_TURN)))
            BattleScriptExecute(i == 1 ? BattleScript_TrainerASlideMsgEnd2 : BattleScript_TrainerBSlideMsgEnd2);
        gBattleStruct->eventsBeforeFirstTurnState = 0;
        break;
    }
}

static void HandleEndTurn_ContinueBattle(void)
{
    s32 i;

    if (gBattleControllerExecFlags == 0)
    {
        gBattleMainFunc = BattleTurnPassed;
        for (i = 0; i < BATTLE_COMMUNICATION_ENTRIES_COUNT; i++)
            gBattleCommunication[i] = 0;
        for (i = 0; i < gBattlersCount; i++)
        {
            gBattleMons[i].status2 &= ~STATUS2_FLINCHED;
            if ((gBattleMons[i].status1 & STATUS1_SLEEP) && (gBattleMons[i].status2 & STATUS2_MULTIPLETURNS))
                CancelMultiTurnMoves(i);
        }
        gBattleStruct->turnEffectsTracker = 0;
        gBattleStruct->turnEffectsBattlerId = 0;
        gBattleStruct->wishPerishSongState = 0;
        gBattleStruct->wishPerishSongBattlerId = 0;
        gBattleStruct->turnCountersTracker = 0;
        gMoveResultFlags = 0;
    }
}

void BattleTurnPassed(void)
{
    s32 i;

    gBattleStruct->speedTieBreaks = RandomUniform(RNG_SPEED_TIE, 0, Factorial(MAX_BATTLERS_COUNT) - 1);

    TurnValuesCleanUp(TRUE);
    if (gBattleOutcome == 0)
    {
        if (DoFieldEndTurnEffects())
            return;
        if (DoBattlerEndTurnEffects())
            return;
    }
    if (HandleWishPerishSongOnTurnEnd())
        return;
    if (HandleFaintedMonActions())
        return;
    gBattleStruct->faintedActionsState = 0;

    TurnValuesCleanUp(FALSE);
    gHitMarker &= ~HITMARKER_NO_ATTACKSTRING;
    gHitMarker &= ~HITMARKER_UNABLE_TO_USE_MOVE;
    gHitMarker &= ~HITMARKER_PLAYER_FAINTED;
    gHitMarker &= ~HITMARKER_PASSIVE_DAMAGE;
    gBattleScripting.animTurn = 0;
    gBattleScripting.animTargetsHit = 0;
    gBattleScripting.moveendState = 0;
    gBattleMoveDamage = 0;
    gMoveResultFlags = 0;

    for (i = 0; i < 5; i++)
        gBattleCommunication[i] = 0;

    if (gBattleOutcome != 0)
    {
        gCurrentActionFuncId = B_ACTION_FINISHED;
        gBattleMainFunc = RunTurnActionsFunctions;
        return;
    }

    if (gBattleResults.battleTurnCounter < 0xFF)
    {
        gBattleResults.battleTurnCounter++;
        gBattleStruct->arenaTurnCounter++;
    }

    for (i = 0; i < gBattlersCount; i++)
    {
        gChosenActionByBattler[i] = B_ACTION_NONE;
        gChosenMoveByBattler[i] = MOVE_NONE;
    }

    for (i = 0; i < MAX_BATTLERS_COUNT; i++)
        *(gBattleStruct->monToSwitchIntoId + i) = PARTY_SIZE;

    *(&gBattleStruct->absentBattlerFlags) = gAbsentBattlerFlags;
    BattlePutTextOnWindow(gText_EmptyString, B_WIN_MSG);
    SetShellSideArmCategory();
    SetAiLogicDataForTurn(AI_DATA); // get assumed abilities, hold effects, etc of all battlers
    gBattleMainFunc = HandleTurnActionSelectionState;

    if (gSideTimers[B_SIDE_PLAYER].retaliateTimer > 0)
        gSideTimers[B_SIDE_PLAYER].retaliateTimer--;
    if (gSideTimers[B_SIDE_OPPONENT].retaliateTimer > 0)
        gSideTimers[B_SIDE_OPPONENT].retaliateTimer--;

    if ((i = ShouldDoTrainerSlide(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), TRAINER_SLIDE_LAST_LOW_HP)))
        BattleScriptExecute(i == 1 ? BattleScript_TrainerASlideMsgEnd2 : BattleScript_TrainerBSlideMsgEnd2);
    else if ((i = ShouldDoTrainerSlide(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), TRAINER_SLIDE_LAST_HALF_HP)))
        BattleScriptExecute(i == 1 ? BattleScript_TrainerASlideMsgEnd2 : BattleScript_TrainerBSlideMsgEnd2);
    else if ((i = ShouldDoTrainerSlide(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), TRAINER_SLIDE_FIRST_CRITICAL_HIT)))
        BattleScriptExecute(i == 1 ? BattleScript_TrainerASlideMsgEnd2 : BattleScript_TrainerBSlideMsgEnd2);
    else if ((i = ShouldDoTrainerSlide(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), TRAINER_SLIDE_FIRST_SUPER_EFFECTIVE_HIT)))
        BattleScriptExecute(i == 1 ? BattleScript_TrainerASlideMsgEnd2 : BattleScript_TrainerBSlideMsgEnd2);
    else if ((i = ShouldDoTrainerSlide(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), TRAINER_SLIDE_FIRST_STAB_MOVE)))
        BattleScriptExecute(i == 1 ? BattleScript_TrainerASlideMsgEnd2 : BattleScript_TrainerBSlideMsgEnd2);
    else if ((i = ShouldDoTrainerSlide(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), TRAINER_SLIDE_PLAYER_MON_UNAFFECTED)))
        BattleScriptExecute(i == 1 ? BattleScript_TrainerASlideMsgEnd2 : BattleScript_TrainerBSlideMsgEnd2);
}

u8 IsRunningFromBattleImpossible(u32 battler)
{
    u32 holdEffect, i;

    holdEffect = ItemId_GetHoldEffect(gBattleMons[battler].item);

    gPotentialItemEffectBattler = battler;

    if (holdEffect == HOLD_EFFECT_CAN_ALWAYS_RUN)
        return BATTLE_RUN_SUCCESS;
    if (B_GHOSTS_ESCAPE >= GEN_6 && IS_BATTLER_OF_TYPE(battler, TIPO_FANTASMA))
        return BATTLE_RUN_SUCCESS;
    if (GetBattlerAbility(battler) == ABILITY_RUN_AWAY)
        return BATTLE_RUN_SUCCESS;

    if ((i = IsAbilityPreventingEscape(battler)))
    {
        gBattleScripting.battler = i - 1;
        gLastUsedAbility = gBattleMons[i - 1].ability;
        gBattleCommunication[MULTISTRING_CHOOSER] = B_MSG_PREVENTS_ESCAPE;
        return BATTLE_RUN_FAILURE;
    }

    if (!CanBattlerEscape(battler))
    {
        gBattleCommunication[MULTISTRING_CHOOSER] = B_MSG_CANT_ESCAPE;
        return BATTLE_RUN_FORBIDDEN;
    }
    return BATTLE_RUN_SUCCESS;
}

void SwitchTwoBattlersInParty(u32 battler, u32 battler2)
{
    s32 i;
    u32 partyId1, partyId2;

    for (i = 0; i < (int)ARRAY_COUNT(gBattlePartyCurrentOrder); i++)
        gBattlePartyCurrentOrder[i] = *(battler * 3 + i + (u8 *)(gBattleStruct->battlerPartyOrders));

    partyId1 = GetPartyIdFromBattlePartyId(gBattlerPartyIndexes[battler]);
    partyId2 = GetPartyIdFromBattlePartyId(gBattlerPartyIndexes[battler2]);
    SwitchPartyMonSlots(partyId1, partyId2);

    for (i = 0; i < (int)ARRAY_COUNT(gBattlePartyCurrentOrder); i++)
    {
        *(battler * 3 + i + (u8 *)(gBattleStruct->battlerPartyOrders)) = gBattlePartyCurrentOrder[i];
        *(BATTLE_PARTNER(battler) * 3 + i + (u8 *)(gBattleStruct->battlerPartyOrders)) = gBattlePartyCurrentOrder[i];
    }
}

void SwitchPartyOrder(u32 battler)
{
    s32 i;
    u32 partyId1, partyId2;

    for (i = 0; i < (int)ARRAY_COUNT(gBattlePartyCurrentOrder); i++)
        gBattlePartyCurrentOrder[i] = *(battler * 3 + i + (u8 *)(gBattleStruct->battlerPartyOrders));

    partyId1 = GetPartyIdFromBattlePartyId(gBattlerPartyIndexes[battler]);
    partyId2 = GetPartyIdFromBattlePartyId(*(gBattleStruct->monToSwitchIntoId + battler));
    SwitchPartyMonSlots(partyId1, partyId2);

    if (EsContraEntrenador())
    {
        for (i = 0; i < (int)ARRAY_COUNT(gBattlePartyCurrentOrder); i++)
        {
            *(battler * 3 + i + (u8 *)(gBattleStruct->battlerPartyOrders)) = gBattlePartyCurrentOrder[i];
            *(BATTLE_PARTNER(battler) * 3 + i + (u8 *)(gBattleStruct->battlerPartyOrders)) = gBattlePartyCurrentOrder[i];
        }
    }
    else
    {
        for (i = 0; i < (int)ARRAY_COUNT(gBattlePartyCurrentOrder); i++)
        {
            *(battler * 3 + i + (u8 *)(gBattleStruct->battlerPartyOrders)) = gBattlePartyCurrentOrder[i];
        }
    }
}

enum
{
    STATE_TURN_START_RECORD,
    STATE_BEFORE_ACTION_CHOSEN,
    STATE_WAIT_ACTION_CHOSEN,
    STATE_WAIT_ACTION_CASE_CHOSEN,
    STATE_WAIT_ACTION_CONFIRMED,
    STATE_SELECTION_SCRIPT,
    STATE_WAIT_SET_BEFORE_ACTION,
    STATE_SELECTION_SCRIPT_MAY_RUN
};

static void HandleTurnActionSelectionState(void)
{
    s32 i, battler;

    gBattleCommunication[ACTIONS_CONFIRMED_COUNT] = 0;
    for (battler = 0; battler < gBattlersCount; battler++)
    {
        u32 position = GetBattlerPosition(battler);
        switch (gBattleCommunication[battler])
        {
        case STATE_TURN_START_RECORD: // Recorded battle related action on start of every turn.
            gBattleCommunication[battler] = STATE_BEFORE_ACTION_CHOSEN;
            u32 isAiRisky = AI_THINKING_STRUCT->aiFlags[battler] & AI_FLAG_RISKY; // Risky AI switches aggressively even mid battle

            // Do AI score computations here so we can use them in AI_TrySwitchOrUseItem
            if ((EsContraEntrenador() || IsWildMonSmart()) && (BattlerHasAi(battler)))
            {
                AI_DATA->aiCalcInProgress = TRUE;

                // Setup battler data
                sBattler_AI = battler;
                BattleAI_SetupAIData(0xF, sBattler_AI);

                // Setup switching data
                AI_DATA->mostSuitableMonId[battler] = GetMostSuitableMonToSwitchInto(battler, isAiRisky);
                if (ShouldSwitch(battler))
                    AI_DATA->shouldSwitch |= (1u << battler);

                // Do scoring
                gBattleStruct->aiMoveOrAction[battler] = BattleAI_ChooseMoveOrAction();
                AI_DATA->aiCalcInProgress = FALSE;
            }
            // fallthrough
        case STATE_BEFORE_ACTION_CHOSEN: // Choose an action.
            *(gBattleStruct->monToSwitchIntoId + battler) = PARTY_SIZE;
            if ((position & BIT_FLANK) == B_FLANK_LEFT
                || gBattleStruct->absentBattlerFlags & (1u << GetBattlerAtPosition(BATTLE_PARTNER(position)))
                || gBattleCommunication[GetBattlerAtPosition(BATTLE_PARTNER(position))] == STATE_WAIT_ACTION_CONFIRMED)
            {
                if ((gBattleStruct->absentBattlerFlags & (1u << battler)) || (gBattleStruct->commandingDondozo & (1u << battler)))
                {
                    gChosenActionByBattler[battler] = B_ACTION_NOTHING_FAINTED;
                    gBattleCommunication[battler] = STATE_WAIT_ACTION_CONFIRMED;
                }
                else
                {
                    if (gBattleMons[battler].status2 & STATUS2_MULTIPLETURNS
                        || gBattleMons[battler].status2 & STATUS2_RECHARGE)
                    {
                        gChosenActionByBattler[battler] = B_ACTION_USE_MOVE;
                        gBattleCommunication[battler] = STATE_WAIT_ACTION_CONFIRMED;
                    }
                    else
                    {
                        gBattleStruct->itemPartyIndex[battler] = PARTY_SIZE;
                        BtlController_EmitChooseAction(battler, BUFFER_A, gChosenActionByBattler[0], gBattleResources->bufferB[0][1] | (gBattleResources->bufferB[0][2] << 8));
                        MarkBattlerForControllerExec(battler);
                        gBattleCommunication[battler]++;
                    }
                }
            }
            break;
        case STATE_WAIT_ACTION_CHOSEN: // Try to perform an action.
            if (!(gBattleControllerExecFlags & (((1u << battler)) | (0xF << 28) | ((1u << battler) << 4) | ((1u << battler) << 8) | ((1u << battler) << 12))))
            {
                gChosenActionByBattler[battler] = gBattleResources->bufferB[battler][1];

                switch (gBattleResources->bufferB[battler][1])
                {
                case B_ACTION_USE_MOVE:
                    if (AreAllMovesUnusable(battler))
                    {
                        gBattleCommunication[battler] = STATE_SELECTION_SCRIPT;
                        *(gBattleStruct->selectionScriptFinished + battler) = FALSE;
                        *(gBattleStruct->stateIdAfterSelScript + battler) = STATE_WAIT_ACTION_CONFIRMED;
                        *(gBattleStruct->moveTarget + battler) = gBattleResources->bufferB[battler][3];
                        return;
                    }
                    else if (gDisableStructs[battler].encoredMove != 0)
                    {
                        gChosenMoveByBattler[battler] = gDisableStructs[battler].encoredMove;
                        *(gBattleStruct->chosenMovePositions + battler) = gDisableStructs[battler].encoredMovePos;
                        gBattleCommunication[battler] = STATE_WAIT_ACTION_CONFIRMED;
                        return;
                    }
                    else
                    {
                        struct ChooseMoveStruct moveInfo;

                        moveInfo.species = gBattleMons[battler].species;
                        moveInfo.monTypes[0] = gBattleMons[battler].types[0];
                        moveInfo.monTypes[1] = gBattleMons[battler].types[1];
                        moveInfo.monTypes[2] = gBattleMons[battler].types[2];

                        for (i = 0; i < MAX_MON_MOVES; i++)
                        {
                            moveInfo.moves[i] = gBattleMons[battler].moves[i];
                            moveInfo.currentPp[i] = gBattleMons[battler].pp[i];
                            moveInfo.maxPp[i] = CalculatePPWithBonus(
                                                            gBattleMons[battler].moves[i],
                                                            gBattleMons[battler].ppBonuses,
                                                            i);
                        }

                        BtlController_EmitChooseMove(battler, BUFFER_A, EsContraEntrenador() != 0, FALSE, &moveInfo);
                        MarkBattlerForControllerExec(battler);
                    }
                    break;
                case B_ACTION_USE_ITEM:
                    if (FlagGet(B_FLAG_NO_BAG_USE))
                    {
                        gSelectionBattleScripts[battler] = BattleScript_ActionSelectionItemsCantBeUsed;
                        gBattleCommunication[battler] = STATE_SELECTION_SCRIPT;
                        *(gBattleStruct->selectionScriptFinished + battler) = FALSE;
                        *(gBattleStruct->stateIdAfterSelScript + battler) = STATE_BEFORE_ACTION_CHOSEN;
                        return;
                    }

                    if (gStatuses3[battler] & STATUS3_SKY_DROPPED)
                    {
                        gSelectionBattleScripts[battler] = BattleScript_ActionSelectionItemsCantBeUsed;
                        gBattleCommunication[battler] = STATE_SELECTION_SCRIPT;
                        *(gBattleStruct->selectionScriptFinished + battler) = FALSE;
                        *(gBattleStruct->stateIdAfterSelScript + battler) = STATE_BEFORE_ACTION_CHOSEN;
                        return;
                    }
                    else
                    {
                        BtlController_EmitChooseItem(battler, BUFFER_A, gBattleStruct->battlerPartyOrders[battler]);
                        MarkBattlerForControllerExec(battler);
                    }
                    break;
                case B_ACTION_SWITCH:
                    *(gBattleStruct->battlerPartyIndexes + battler) = gBattlerPartyIndexes[battler];
                    if (ItemId_GetHoldEffect(gBattleMons[battler].item) != HOLD_EFFECT_SHED_SHELL
                      && (i = IsAbilityPreventingEscape(battler)))   // must be last to keep i value integrity
                    {
                        BtlController_EmitChoosePokemon(battler, BUFFER_A, ((i - 1) << 4) | PARTY_ACTION_ABILITY_PREVENTS, PARTY_SIZE, gBattleMons[i - 1].ability, gBattleStruct->battlerPartyOrders[battler]);
                    }
                    else
                    {
                        if (battler == 2 && gChosenActionByBattler[0] == B_ACTION_SWITCH)
                            BtlController_EmitChoosePokemon(battler, BUFFER_A, PARTY_ACTION_CHOOSE_MON, *(gBattleStruct->monToSwitchIntoId + 0), ABILITY_NONE, gBattleStruct->battlerPartyOrders[battler]);
                        else if (battler == 3 && gChosenActionByBattler[1] == B_ACTION_SWITCH)
                            BtlController_EmitChoosePokemon(battler, BUFFER_A, PARTY_ACTION_CHOOSE_MON, *(gBattleStruct->monToSwitchIntoId + 1), ABILITY_NONE, gBattleStruct->battlerPartyOrders[battler]);
                        else
                            BtlController_EmitChoosePokemon(battler, BUFFER_A, PARTY_ACTION_CHOOSE_MON, PARTY_SIZE, ABILITY_NONE, gBattleStruct->battlerPartyOrders[battler]);
                    }
                    MarkBattlerForControllerExec(battler);
                    break;
                case B_ACTION_SAFARI_BALL:
                    if (IsPlayerPartyAndPokemonStorageFull())
                    {
                        gSelectionBattleScripts[battler] = BattleScript_PrintFullBox;
                        gBattleCommunication[battler] = STATE_SELECTION_SCRIPT;
                        *(gBattleStruct->selectionScriptFinished + battler) = FALSE;
                        *(gBattleStruct->stateIdAfterSelScript + battler) = STATE_BEFORE_ACTION_CHOSEN;
                        return;
                    }
                    break;
                case B_ACTION_SAFARI_POKEBLOCK:
                    BtlController_EmitChooseItem(battler, BUFFER_A, gBattleStruct->battlerPartyOrders[battler]);
                    MarkBattlerForControllerExec(battler);
                    break;
                case B_ACTION_CANCEL_PARTNER:
                    gBattleCommunication[battler] = STATE_WAIT_SET_BEFORE_ACTION;
                    gBattleCommunication[GetBattlerAtPosition(BATTLE_PARTNER(GetBattlerPosition(battler)))] = STATE_BEFORE_ACTION_CHOSEN;
                    if (gBattleMons[GetBattlerAtPosition(BATTLE_PARTNER(GetBattlerPosition(battler)))].status2 & STATUS2_MULTIPLETURNS
                        || gBattleMons[GetBattlerAtPosition(BATTLE_PARTNER(GetBattlerPosition(battler)))].status2 & STATUS2_RECHARGE)
                    {
                        BtlController_EmitEndBounceEffect(battler, BUFFER_A);
                        MarkBattlerForControllerExec(battler);
                        return;
                    }

                    BtlController_EmitEndBounceEffect(battler, BUFFER_A);
                    MarkBattlerForControllerExec(battler);
                    return;
                case B_ACTION_DEBUG:
                    BtlController_EmitDebugMenu(battler, BUFFER_A);
                    MarkBattlerForControllerExec(battler);
                    break;
                }

                if (EsContraEntrenador() && gBattleResources->bufferB[battler][1] == B_ACTION_RUN)
                {
                    BattleScriptExecute(BattleScript_PrintCantRunFromTrainer);
                    gBattleCommunication[battler] = STATE_BEFORE_ACTION_CHOSEN;
                }
                else if ((IsRunningFromBattleImpossible(battler) != BATTLE_RUN_SUCCESS
                         && gBattleResources->bufferB[battler][1] == B_ACTION_RUN)
                         || (FlagGet(B_FLAG_NO_RUNNING) == TRUE && gBattleResources->bufferB[battler][1] == B_ACTION_RUN))
                {
                    gSelectionBattleScripts[battler] = BattleScript_PrintCantEscapeFromBattle;
                    gBattleCommunication[battler] = STATE_SELECTION_SCRIPT;
                    *(gBattleStruct->selectionScriptFinished + battler) = FALSE;
                    *(gBattleStruct->stateIdAfterSelScript + battler) = STATE_BEFORE_ACTION_CHOSEN;
                    return;
                }
                else
                {
                    gBattleCommunication[battler]++;
                }
            }
            break;
        case STATE_WAIT_ACTION_CASE_CHOSEN:
            if (!(gBattleControllerExecFlags & (((1u << battler)) | (0xF << 28) | ((1u << battler) << 4) | ((1u << battler) << 8) | ((1u << battler) << 12))))
            {
                switch (gChosenActionByBattler[battler])
                {
                case B_ACTION_USE_MOVE:
                    switch (gBattleResources->bufferB[battler][1])
                    {
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                    case 9:
                        gChosenActionByBattler[battler] = gBattleResources->bufferB[battler][1];
                        return;
                    case 15:
                        gChosenActionByBattler[battler] = B_ACTION_SWITCH;
                        UpdateBattlerPartyOrdersOnSwitch(battler);
                        return;
                    default:
                        if ((gBattleResources->bufferB[battler][2] | (gBattleResources->bufferB[battler][3] << 8)) == 0xFFFF)
                        {
                            gBattleCommunication[battler] = STATE_BEFORE_ACTION_CHOSEN;
                        }
                        else if (TrySetCantSelectMoveBattleScript(battler))
                        {
                            gBattleCommunication[battler] = STATE_SELECTION_SCRIPT;
                            *(gBattleStruct->selectionScriptFinished + battler) = FALSE;
                            gBattleResources->bufferB[battler][1] = B_ACTION_USE_MOVE;
                            *(gBattleStruct->stateIdAfterSelScript + battler) = STATE_WAIT_ACTION_CHOSEN;
                            return;
                        }
                        else
                        {
                            // Get the chosen move position (and thus the chosen move) and target from the returned buffer.
                            gChosenMoveByBattler[battler] = gBattleMons[battler].moves[gBattleStruct->chosenMovePositions[battler]];
                            gBattleStruct->moveTarget[battler] = gBattleResources->bufferB[battler][3];
                            gBattleCommunication[battler]++;
                        }
                        break;
                    }
                    break;
                case B_ACTION_USE_ITEM:
                    if ((gBattleResources->bufferB[battler][1] | (gBattleResources->bufferB[battler][2] << 8)) == 0)
                    {
                        gBattleCommunication[battler] = STATE_BEFORE_ACTION_CHOSEN;
                    }
                    else
                    {
                        gLastUsedItem = (gBattleResources->bufferB[battler][1] | (gBattleResources->bufferB[battler][2] << 8));
                        if (ItemId_GetPocket(gLastUsedItem) == POCKET_POKE_BALLS)
                            gBattleStruct->throwingPokeBall = TRUE;
                        gBattleCommunication[battler]++;
                    }
                    break;
                case B_ACTION_SWITCH:
                    if (gBattleResources->bufferB[battler][1] == PARTY_SIZE)
                    {
                        gBattleCommunication[battler] = STATE_BEFORE_ACTION_CHOSEN;
                    }
                    else
                    {
                        UpdateBattlerPartyOrdersOnSwitch(battler);
                        gBattleCommunication[battler]++;
                    }
                    break;
                case B_ACTION_RUN:
                    gHitMarker |= HITMARKER_RUN;
                    gBattleCommunication[battler]++;
                    break;
                case B_ACTION_SAFARI_WATCH_CAREFULLY:
                    gBattleCommunication[battler]++;
                    break;
                case B_ACTION_SAFARI_BALL:
                    gBattleCommunication[battler]++;
                    break;
                case B_ACTION_THROW_BALL:
                    gBattleStruct->throwingPokeBall = TRUE;
                    gBattleCommunication[battler]++;
                    break;
                case B_ACTION_SAFARI_POKEBLOCK:
                    if ((gBattleResources->bufferB[battler][1] | (gBattleResources->bufferB[battler][2] << 8)) != 0)
                        gBattleCommunication[battler]++;
                    else
                        gBattleCommunication[battler] = STATE_BEFORE_ACTION_CHOSEN;
                    break;
                case B_ACTION_SAFARI_GO_NEAR:
                    gBattleCommunication[battler]++;
                    break;
                case B_ACTION_SAFARI_RUN:
                    gHitMarker |= HITMARKER_RUN;
                    gBattleCommunication[battler]++;
                    break;
                case B_ACTION_WALLY_THROW:
                    gBattleCommunication[battler]++;
                    break;
                case B_ACTION_DEBUG:
                    gBattleCommunication[battler] = STATE_BEFORE_ACTION_CHOSEN;
                    break;
                }
            }
            break;
        case STATE_WAIT_ACTION_CONFIRMED:
            if (!(gBattleControllerExecFlags & ((1u << battler) | (0xF << 28) | (1u << (battler + 4)) | (1u << (battler + 8)) | (1u << (battler + 12)))))
            {
                gBattleCommunication[ACTIONS_CONFIRMED_COUNT]++;
            }
            break;
        case STATE_SELECTION_SCRIPT:
            if (*(gBattleStruct->selectionScriptFinished + battler))
            {
                gBattleCommunication[battler] = *(gBattleStruct->stateIdAfterSelScript + battler);
            }
            else
            {
                gBattlerAttacker = battler;
                gBattlescriptCurrInstr = gSelectionBattleScripts[battler];
                if (!(gBattleControllerExecFlags & ((1u << battler) | (0xF << 28) | (1u << (battler + 4)) | (1u << (battler + 8)) | (1u << (battler + 12)))))
                {
                    gBattleScriptingCommandsTable[gBattlescriptCurrInstr[0]]();
                }
                gSelectionBattleScripts[battler] = gBattlescriptCurrInstr;
            }
            break;
        case STATE_WAIT_SET_BEFORE_ACTION:
                if (!(gBattleControllerExecFlags & ((1u << battler) | (0xF << 28) | (1u << (battler + 4)) | (1u << (battler + 8)) | (1u << (battler + 12)))))
            {
                gBattleCommunication[battler] = STATE_BEFORE_ACTION_CHOSEN;
            }
            break;
        case STATE_SELECTION_SCRIPT_MAY_RUN:
            if (*(gBattleStruct->selectionScriptFinished + battler))
            {
                if (gBattleResources->bufferB[battler][1] == B_ACTION_NOTHING_FAINTED)
                {
                    gHitMarker |= HITMARKER_RUN;
                    gChosenActionByBattler[battler] = B_ACTION_RUN;
                    gBattleCommunication[battler] = STATE_WAIT_ACTION_CONFIRMED;
                }
                else
                {
                    gBattleCommunication[battler] = *(gBattleStruct->stateIdAfterSelScript + battler);
                }
            }
            else
            {
                gBattlerAttacker = battler;
                gBattlescriptCurrInstr = gSelectionBattleScripts[battler];
                if (!(gBattleControllerExecFlags & ((1u << battler) | (0xF << 28) | (1u << (battler + 4)) | (1u << (battler + 8)) | (1u << (battler + 12)))))
                {
                    gBattleScriptingCommandsTable[gBattlescriptCurrInstr[0]]();
                }
                gSelectionBattleScripts[battler] = gBattlescriptCurrInstr;
            }
            break;
        }
    }

    // Check if everyone chose actions.
    if (gBattleCommunication[ACTIONS_CONFIRMED_COUNT] == gBattlersCount)
    {
        gBattleMainFunc = SetActionsAndBattlersTurnOrder;
    }
}

static bool8 AllAtActionConfirmed(void)
{
    s32 i, count;

    for (count = 0, i = 0; i < gBattlersCount; i++)
    {
        if (gBattleCommunication[i] == STATE_WAIT_ACTION_CONFIRMED)
            count++;
    }

    if (count + 1 == gBattlersCount)
        return TRUE;
    else
        return FALSE;
}

static void UpdateBattlerPartyOrdersOnSwitch(u32 battler)
{
    gBattleStruct->monToSwitchIntoId[battler] = gBattleResources->bufferB[battler][1];
}

void SwapTurnOrder(u8 id1, u8 id2)
{
    u32 temp;

    SWAP(gActionsByTurnOrder[id1], gActionsByTurnOrder[id2], temp);
    SWAP(gBattlerByTurnOrder[id1], gBattlerByTurnOrder[id2], temp);
}

// For AI, so it doesn't 'cheat' by knowing player's ability
u32 GetBattlerTotalSpeedStatArgs(u32 battler, u32 ability, u32 holdEffect)
{
    u32 speed = gBattleMons[battler].speed;

    if (WEATHER_HAS_EFFECT)
    {
        if (ability == ABILITY_SWIFT_SWIM       && gBattleWeather & B_WEATHER_RAIN)
            speed = (speed * 150) / 100;
        else if (ability == ABILITY_CHLOROPHYLL && gBattleWeather & B_WEATHER_SUN)
            speed = (speed * 150) / 100;
        else if (ability == ABILITY_SAND_RUSH   && gBattleWeather & B_WEATHER_SANDSTORM)
            speed = (speed * 150) / 100;
        else if (ability == ABILITY_SLUSH_RUSH  && (gBattleWeather & (B_WEATHER_HAIL | B_WEATHER_SNOW)))
            speed = (speed * 150) / 100;
    }

    if (ability == ABILITY_QUICK_FEET && gBattleMons[battler].status1 & STATUS1_ANY)
        speed = (speed * 150) / 100;
    else if (ability == ABILITY_SLOW_START && gDisableStructs[battler].slowStartTimer != 0)
        speed /= 2;

    speed = (speed * gMultiplicadoresEstadisticas[gBattleMons[battler].statStages[ESTADISTICA_VELOCIDAD]]) >> 8;

    if (holdEffect == HOLD_EFFECT_MACHO_BRACE || holdEffect == HOLD_EFFECT_POWER_ITEM)
        speed /= 2;
    else if (holdEffect == HOLD_EFFECT_CHOICE_SCARF)
        speed = (speed * 150) / 100;

    if (gSideStatuses[GetBattlerSide(battler)] & SIDE_STATUS_TAILWIND)
        speed *= 2;
    if (gBattleResources->flags->flags[battler] & RESOURCE_FLAG_UNBURDEN)
        speed *= 2;

    if (gBattleMons[battler].status1 & STATUS1_PARALYSIS && ability != ABILITY_QUICK_FEET)
        speed /= 2;

    return speed;
}

u32 GetBattlerTotalSpeedStat(u32 battler)
{
    u32 ability = GetBattlerAbility(battler);
    u32 holdEffect = GetBattlerHoldEffect(battler, TRUE);
    return GetBattlerTotalSpeedStatArgs(battler, ability, holdEffect);
}

s8 GetChosenMovePriority(u32 battler)
{
    u16 move;

    gProtectStructs[battler].pranksterElevated = 0;
    if (gProtectStructs[battler].noValidMoves)
        move = MOVE_STRUGGLE;
    else
        move = gBattleMons[battler].moves[*(gBattleStruct->chosenMovePositions + battler)];

    return GetMovePriority(battler, move);
}

s8 GetMovePriority(u32 battler, u16 move)
{
    s8 priority;
    u16 ability = GetBattlerAbility(battler);

    if (ability == ABILITY_GALE_WINGS && gMovesInfo[move].type == TIPO_VOLADOR)
        priority++;
    else if (ability == ABILITY_TIERRA_SUELTA && gMovesInfo[move].type == TIPO_TIERRA)
        priority++;
    else if (ability == ABILITY_PACIFICADOR && gMovesInfo[move].type == TIPO_HADA)
        priority++;
    else if (ability == ABILITY_SUPERORDENADOR && gMovesInfo[move].type == TIPO_PSIQUICO)
        priority++;
    else if (ability == ABILITY_ENVIO_EXPRESS  && gMovesInfo[move].type == TIPO_HADA)
        priority++;
    else if (ability == ABILITY_BAILARIN && gMovesInfo[move].danceMove)
        priority++;
    else if (ability == ABILITY_PRANKSTER && IS_MOVE_STATUS(move))
    {
        gProtectStructs[battler].pranksterElevated = 1;
        priority++;
    }
    else if (ability == ABILITY_TRIAGE && IsHealingMove(move))
        priority += 3;
    else if (ability == ABILITY_ATAQUE_RELAMPAGO && gMovesInfo[move].ballisticMove)
        priority++;
    else if (ability == ABILITY_OJOS_PRESTOS && gMovesInfo[move].eyesMove)
        priority++;
    else if (ability == ABILITY_RUN_AWAY && gMovesInfo[move].effect == EFFECT_HIT_ESCAPE)
        priority++;
    else if (ability == ABILITY_VOZ_CANTANTE && gMovesInfo[move].soundMove)
        priority++;
    if (gProtectStructs[battler].quash)
        priority = -8;

    return priority;
}

// Function for AI with variables provided as arguments to speed the computation time
s32 GetWhichBattlerFasterArgs(u32 battler1, u32 battler2, bool32 ignoreChosenMoves, u32 ability1, u32 ability2,
                              u32 holdEffectBattler1, u32 holdEffectBattler2, u32 speedBattler1, u32 speedBattler2, s32 priority1, s32 priority2)
{
    u32 strikesFirst = 0;

    if (priority1 == priority2)
    {
        // Quick Claw / Quick Draw / Custap Berry - always first
        // Stall / Mycelium Might - last but before Lagging Tail
        // Lagging Tail - always last
        bool32 battler1HasQuickEffect = gProtectStructs[battler1].quickDraw || gProtectStructs[battler1].usedCustapBerry;
        bool32 battler2HasQuickEffect = gProtectStructs[battler2].quickDraw || gProtectStructs[battler2].usedCustapBerry;
        bool32 battler1HasStallingAbility = ability1 == ABILITY_STALL || (ability1 == ABILITY_MYCELIUM_MIGHT && IS_MOVE_STATUS(gChosenMoveByBattler[battler1]));
        bool32 battler2HasStallingAbility = ability2 == ABILITY_STALL || (ability2 == ABILITY_MYCELIUM_MIGHT && IS_MOVE_STATUS(gChosenMoveByBattler[battler2]));

        if (battler1HasQuickEffect && !battler2HasQuickEffect)
            strikesFirst = 1;
        else if (battler2HasQuickEffect && !battler1HasQuickEffect)
            strikesFirst = -1;
        else if (holdEffectBattler1 == HOLD_EFFECT_LAGGING_TAIL && holdEffectBattler2 != HOLD_EFFECT_LAGGING_TAIL)
            strikesFirst = -1;
        else if (holdEffectBattler2 == HOLD_EFFECT_LAGGING_TAIL && holdEffectBattler1 != HOLD_EFFECT_LAGGING_TAIL)
            strikesFirst = 1;
        else if (battler1HasStallingAbility && !battler2HasStallingAbility)
            strikesFirst = -1;
        else if (battler2HasStallingAbility && !battler1HasStallingAbility)
            strikesFirst = 1;
        else
        {
            if (speedBattler1 == speedBattler2)
            {
                // same speeds, same priorities
                strikesFirst = 0;
            }
            else if (speedBattler1 < speedBattler2)
            {
                // battler2 has more speed
                if (gFieldStatuses & STATUS_FIELD_TRICK_ROOM)
                    strikesFirst = 1;
                else
                    strikesFirst = -1;
            }
            else
            {
                // battler1 has more speed
                if (gFieldStatuses & STATUS_FIELD_TRICK_ROOM)
                    strikesFirst = -1;
                else
                    strikesFirst = 1;
            }
        }
    }
    else if (priority1 < priority2)
    {
        strikesFirst = -1; // battler2's move has greater priority
    }
    else
    {
        strikesFirst = 1; // battler1's move has greater priority
    }
    return strikesFirst;
}

s32 GetWhichBattlerFasterOrTies(u32 battler1, u32 battler2, bool32 ignoreChosenMoves)
{
    s32 priority1 = 0, priority2 = 0;
    u32 ability1 = GetBattlerAbility(battler1);
    u32 speedBattler1 = GetBattlerTotalSpeedStat(battler1);
    u32 holdEffectBattler1 = GetBattlerHoldEffect(battler1, TRUE);
    u32 speedBattler2 = GetBattlerTotalSpeedStat(battler2);
    u32 holdEffectBattler2 = GetBattlerHoldEffect(battler2, TRUE);
    u32 ability2 = GetBattlerAbility(battler2);

    if (!ignoreChosenMoves)
    {
        if (gChosenActionByBattler[battler1] == B_ACTION_USE_MOVE)
            priority1 = GetChosenMovePriority(battler1);
        if (gChosenActionByBattler[battler2] == B_ACTION_USE_MOVE)
            priority2 = GetChosenMovePriority(battler2);
    }

    return GetWhichBattlerFasterArgs(
        battler1, battler2,
        ignoreChosenMoves,
        ability1, ability2,
        holdEffectBattler1, holdEffectBattler2,
        speedBattler1, speedBattler2,
        priority1, priority2
    );
}

// 24 == MAX_BATTLERS_COUNT!.
// These are the possible orders if all the battlers speed tie. An order
// is chosen at the start of the turn.
static const u8 sBattlerOrders[24][4] =
{
    { 0, 1, 2, 3 },
    { 0, 1, 3, 2 },
    { 0, 2, 1, 3 },
    { 0, 2, 3, 1 },
    { 0, 3, 1, 2 },
    { 0, 3, 2, 1 },
    { 1, 0, 2, 3 },
    { 1, 0, 3, 2 },
    { 1, 2, 0, 3 },
    { 1, 2, 3, 0 },
    { 1, 3, 0, 2 },
    { 1, 3, 2, 0 },
    { 2, 0, 1, 3 },
    { 2, 0, 3, 1 },
    { 2, 1, 0, 3 },
    { 2, 1, 3, 0 },
    { 2, 3, 0, 1 },
    { 2, 3, 1, 0 },
    { 3, 0, 1, 2 },
    { 3, 0, 2, 1 },
    { 3, 1, 0, 2 },
    { 3, 1, 2, 0 },
    { 3, 2, 0, 1 },
    { 3, 2, 1, 0 },
};

s32 GetWhichBattlerFaster(u32 battler1, u32 battler2, bool32 ignoreChosenMoves)
{
    s32 strikesFirst = GetWhichBattlerFasterOrTies(battler1, battler2, ignoreChosenMoves);
    if (strikesFirst == 0)
    {
        s32 order1 = sBattlerOrders[gBattleStruct->speedTieBreaks][battler1];
        s32 order2 = sBattlerOrders[gBattleStruct->speedTieBreaks][battler2];
        if (order1 < order2)
            strikesFirst = 1;
        else
            strikesFirst = -1;
    }
    return strikesFirst;
}

static void SetActionsAndBattlersTurnOrder(void)
{
    s32 turnOrderId = 0;
    s32 i, j, battler;

    if (gChosenActionByBattler[0] == B_ACTION_RUN)
    {
        battler = 0;
        turnOrderId = 5;
    }
    if (gChosenActionByBattler[2] == B_ACTION_RUN)
    {
        battler = 2;
        turnOrderId = 5;
    }

    if (turnOrderId == 5) // One of battlers wants to run.
    {
        gActionsByTurnOrder[0] = gChosenActionByBattler[battler];
        gBattlerByTurnOrder[0] = battler;
        turnOrderId = 1;
        for (i = 0; i < gBattlersCount; i++)
        {
            if (i != battler)
            {
                gActionsByTurnOrder[turnOrderId] = gChosenActionByBattler[i];
                gBattlerByTurnOrder[turnOrderId] = i;
                turnOrderId++;
            }
        }
    }
    else
    {
        for (battler = 0; battler < gBattlersCount; battler++)
        {
            if (gChosenActionByBattler[battler] == B_ACTION_USE_ITEM
                || gChosenActionByBattler[battler] == B_ACTION_SWITCH
                || gChosenActionByBattler[battler] == B_ACTION_THROW_BALL)
            {
                gActionsByTurnOrder[turnOrderId] = gChosenActionByBattler[battler];
                gBattlerByTurnOrder[turnOrderId] = battler;
                turnOrderId++;
            }
        }
        for (battler = 0; battler < gBattlersCount; battler++)
        {
            if (gChosenActionByBattler[battler] != B_ACTION_USE_ITEM
                && gChosenActionByBattler[battler] != B_ACTION_SWITCH
                && gChosenActionByBattler[battler] != B_ACTION_THROW_BALL)
            {
                gActionsByTurnOrder[turnOrderId] = gChosenActionByBattler[battler];
                gBattlerByTurnOrder[turnOrderId] = battler;
                gBattleStruct->quickClawRandom[battler] = RandomPercentage(RNG_QUICK_CLAW, GetBattlerHoldEffectParam(battler));
                gBattleStruct->quickDrawRandom[battler] = RandomPercentage(RNG_QUICK_DRAW, 30);
                turnOrderId++;
            }
        }
        for (i = 0; i < gBattlersCount - 1; i++)
        {
            for (j = i + 1; j < gBattlersCount; j++)
            {
                u8 battler1 = gBattlerByTurnOrder[i];
                u8 battler2 = gBattlerByTurnOrder[j];
                TryChangingTurnOrderEffects(battler1, battler2);
                if (gActionsByTurnOrder[i] != B_ACTION_USE_ITEM
                    && gActionsByTurnOrder[j] != B_ACTION_USE_ITEM
                    && gActionsByTurnOrder[i] != B_ACTION_SWITCH
                    && gActionsByTurnOrder[j] != B_ACTION_SWITCH
                    && gActionsByTurnOrder[i] != B_ACTION_THROW_BALL
                    && gActionsByTurnOrder[j] != B_ACTION_THROW_BALL)
                {
                    if (GetWhichBattlerFaster(battler1, battler2, FALSE) == -1)
                        SwapTurnOrder(i, j);
                }
            }
        }
    }
    gBattleMainFunc = CheckChangingTurnOrderEffects;
    gBattleStruct->quickClawBattlerId = 0;
}

static void TurnValuesCleanUp(bool8 var0)
{
    s32 i;

    for (i = 0; i < gBattlersCount; i++)
    {
        if (var0)
        {
            gProtectStructs[i].protected = FALSE;
            gProtectStructs[i].spikyShielded = FALSE;
            gProtectStructs[i].kingsShielded = FALSE;
            gProtectStructs[i].banefulBunkered = FALSE;
            gProtectStructs[i].quash = FALSE;
            gProtectStructs[i].usedCustapBerry = FALSE;
            gProtectStructs[i].quickDraw = FALSE;
            memset(&gQueuedStatBoosts[i], 0, sizeof(struct QueuedStatBoost));
        }
        else
        {
            memset(&gProtectStructs[i], 0, sizeof(struct ProtectStruct));

            if (gDisableStructs[i].isFirstTurn)
                gDisableStructs[i].isFirstTurn--;

            if (gDisableStructs[i].rechargeTimer)
            {
                gDisableStructs[i].rechargeTimer--;
                if (gDisableStructs[i].rechargeTimer == 0)
                    gBattleMons[i].status2 &= ~STATUS2_RECHARGE;
            }
            gBattleStruct->canPickupItem &= ~(1u << i);
        }

        if (gDisableStructs[i].substituteHP == 0)
            gBattleMons[i].status2 &= ~STATUS2_SUBSTITUTE;

        if (!(gStatuses3[i] & STATUS3_COMMANDER))
            gBattleStruct->commandingDondozo &= ~(1u << i);

        gSpecialStatuses[i].parentalBondState = PARENTAL_BOND_OFF;
    }

    gSideStatuses[B_SIDE_PLAYER] &= ~(SIDE_STATUS_QUICK_GUARD | SIDE_STATUS_WIDE_GUARD | SIDE_STATUS_CRAFTY_SHIELD | SIDE_STATUS_MAT_BLOCK);
    gSideStatuses[B_SIDE_OPPONENT] &= ~(SIDE_STATUS_QUICK_GUARD | SIDE_STATUS_WIDE_GUARD | SIDE_STATUS_CRAFTY_SHIELD | SIDE_STATUS_MAT_BLOCK);
    gSideTimers[B_SIDE_PLAYER].followmeTimer = 0;
    gSideTimers[B_SIDE_OPPONENT].followmeTimer = 0;

    gBattleStruct->usedEjectItem = 0;
    gBattleStruct->pledgeMove = FALSE; // combined pledge move may not have been used due to a canceller
}

void SpecialStatusesClear(void)
{
    memset(&gSpecialStatuses, 0, sizeof(gSpecialStatuses));
}

static void PopulateArrayWithBattlers(u8 *battlers)
{
    u32 i;
    for (i = 0; i < gBattlersCount; i++)
        battlers[i] = i;
}

static bool32 TryDoMoveEffectsBeforeMoves(void)
{
    if (!(gHitMarker & HITMARKER_RUN))
    {
        u32 i;
        u8 battlers[MAX_BATTLERS_COUNT];

        PopulateArrayWithBattlers(battlers);
        SortBattlersBySpeed(battlers, FALSE);
        for (i = 0; i < gBattlersCount; i++)
        {
            if (!(gBattleStruct->focusPunchBattlers & (1u << battlers[i]))
                && !(gBattleMons[battlers[i]].status1 & STATUS1_SLEEP)
                && !(gDisableStructs[battlers[i]].truantCounter)
                && !(gProtectStructs[battlers[i]].noValidMoves))
            {
                gBattleStruct->focusPunchBattlers |= 1u << battlers[i];
                gBattlerAttacker = battlers[i];
                switch (gMovesInfo[gChosenMoveByBattler[gBattlerAttacker]].effect)
                {
                case EFFECT_FOCUS_PUNCH:
                    BattleScriptExecute(BattleScript_FocusPunchSetUp);
                    return TRUE;
                case EFFECT_BEAK_BLAST:
                    BattleScriptExecute(BattleScript_BeakBlastSetUp);
                    return TRUE;
                case EFFECT_SHELL_TRAP:
                    BattleScriptExecute(BattleScript_ShellTrapSetUp);
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

static void TryChangingTurnOrderEffects(u32 battler1, u32 battler2)
{
    u32 ability1 = GetBattlerAbility(battler1);
    u32 holdEffectBattler1 = GetBattlerHoldEffect(battler1, TRUE);
    u32 holdEffectBattler2 = GetBattlerHoldEffect(battler2, TRUE);
    u32 ability2 = GetBattlerAbility(battler2);

    // Battler 1
    // Quick Draw
    if (ability1 == ABILITY_QUICK_DRAW && !IS_MOVE_STATUS(gChosenMoveByBattler[battler1]) && gBattleStruct->quickDrawRandom[battler1])
        gProtectStructs[battler1].quickDraw = TRUE;
    // Quick Claw and Custap Berry
    if (!gProtectStructs[battler1].quickDraw
     && ((holdEffectBattler1 == HOLD_EFFECT_QUICK_CLAW && gBattleStruct->quickClawRandom[battler1])
     || (holdEffectBattler1 == HOLD_EFFECT_CUSTAP_BERRY && HasEnoughHpToEatBerry(battler1, 4, gBattleMons[battler1].item))))
        gProtectStructs[battler1].usedCustapBerry = TRUE;

    // Battler 2
    // Quick Draw
    if (ability2 == ABILITY_QUICK_DRAW && !IS_MOVE_STATUS(gChosenMoveByBattler[battler2]) && gBattleStruct->quickDrawRandom[battler2])
        gProtectStructs[battler2].quickDraw = TRUE;
    // Quick Claw and Custap Berry
    if (!gProtectStructs[battler2].quickDraw
     && ((holdEffectBattler2 == HOLD_EFFECT_QUICK_CLAW && gBattleStruct->quickClawRandom[battler2])
     || (holdEffectBattler2 == HOLD_EFFECT_CUSTAP_BERRY && HasEnoughHpToEatBerry(battler2, 4, gBattleMons[battler2].item))))
        gProtectStructs[battler2].usedCustapBerry = TRUE;
}

static void CheckChangingTurnOrderEffects(void)
{
    u32 i, battler;

    if (!(gHitMarker & HITMARKER_RUN))
    {
        while (gBattleStruct->quickClawBattlerId < gBattlersCount)
        {
            battler = gBattlerAttacker = gBattleStruct->quickClawBattlerId;
            gBattleStruct->quickClawBattlerId++;
            if (gChosenActionByBattler[battler] == B_ACTION_USE_MOVE
             && gChosenMoveByBattler[battler] != MOVE_FOCUS_PUNCH   // quick claw message doesn't need to activate here
             && (gProtectStructs[battler].usedCustapBerry || gProtectStructs[battler].quickDraw)
             && !(gDisableStructs[gBattlerAttacker].truantCounter)
             && !(gProtectStructs[battler].noValidMoves))
            {
                if (gProtectStructs[battler].usedCustapBerry)
                {
                    gLastUsedItem = gBattleMons[battler].item;
                    PREPARE_ITEM_BUFFER(gBattleTextBuff1, gLastUsedItem);
                    if (GetBattlerHoldEffect(battler, FALSE) == HOLD_EFFECT_CUSTAP_BERRY)
                    {
                        // don't record berry since its gone now
                        BattleScriptExecute(BattleScript_CustapBerryActivation);
                    }
                    else
                    {
                        RecordItemEffectBattle(battler, GetBattlerHoldEffect(battler, FALSE));
                        BattleScriptExecute(BattleScript_QuickClawActivation);
                    }
                }
                else if (gProtectStructs[battler].quickDraw)
                {
                    gBattlerAbility = battler;
                    gLastUsedAbility = gBattleMons[battler].ability;
                    PREPARE_ABILITY_BUFFER(gBattleTextBuff1, gLastUsedAbility);
                    RecordAbilityBattle(battler, gLastUsedAbility);
                    BattleScriptExecute(BattleScript_QuickDrawActivation);
                }
                return;
            }
        }
    }

    // setup stuff before turns/actions
    TryClearRageAndFuryCutter();
    gCurrentTurnActionNumber = 0;
    gCurrentActionFuncId = gActionsByTurnOrder[0];
    gBattleStruct->dynamicMoveType = 0;
    gBattleStruct->effectsBeforeUsingMoveDone = FALSE;
    gBattleStruct->focusPunchBattlers = 0;
    for (i = 0; i < MAX_BATTLERS_COUNT; i++)
    {
        gBattleStruct->ateBoost[i] = FALSE;
        gSpecialStatuses[i].gemBoost = FALSE;
    }

    gBattleMainFunc = RunTurnActionsFunctions;
    gBattleCommunication[3] = 0;
    gBattleCommunication[4] = 0;
    gBattleScripting.multihitMoveEffect = 0;
    gBattleResources->battleScriptsStack->size = 0;
}

static void RunTurnActionsFunctions(void)
{
    if (gBattleOutcome != 0)
        gCurrentActionFuncId = B_ACTION_FINISHED;

    // Focus Punch-like moves after switching, items, running, but before using a move.
    if (gCurrentActionFuncId == B_ACTION_USE_MOVE && !gBattleStruct->effectsBeforeUsingMoveDone)
    {
        if (TryDoMoveEffectsBeforeMoves())
            return;
        gBattleStruct->effectsBeforeUsingMoveDone = TRUE;
    }

    *(&gBattleStruct->savedTurnActionNumber) = gCurrentTurnActionNumber;
    sTurnActionsFuncsTable[gCurrentActionFuncId]();

    if (gCurrentTurnActionNumber >= gBattlersCount) // everyone did their actions, turn finished
    {
        gHitMarker &= ~HITMARKER_PASSIVE_DAMAGE;
        gBattleMainFunc = sEndTurnFuncsTable[gBattleOutcome & 0x7F];
    }
    else
    {
        if (gBattleStruct->savedTurnActionNumber != gCurrentTurnActionNumber) // action turn has been done, clear hitmarker bits for another battler
        {
            gHitMarker &= ~HITMARKER_NO_ATTACKSTRING;
            gHitMarker &= ~HITMARKER_UNABLE_TO_USE_MOVE;
        }
    }
}

static void HandleEndTurn_BattleWon(void)
{
    gCurrentActionFuncId = 0;

    if (EsContraEntrenador())
    {
        BattleStopLowHpSound();
        gBattlescriptCurrInstr = BattleScript_LocalTrainerBattleWon;

        switch (GetTrainerClassFromId(gTrainerBattleOpponent_A))
        {
        case TRAINER_CLASS_ELITE_FOUR:
        case TRAINER_CLASS_CHAMPION:
            PlayBGM(MUS_VICTORY_LEAGUE);
            break;
        case TRAINER_CLASS_TEAM_AQUA:
        case TRAINER_CLASS_TEAM_MAGMA:
        case TRAINER_CLASS_AQUA_ADMIN:
        case TRAINER_CLASS_AQUA_LEADER:
        case TRAINER_CLASS_MAGMA_ADMIN:
        case TRAINER_CLASS_MAGMA_LEADER:
            PlayBGM(MUS_VICTORY_AQUA_MAGMA);
            break;
        case TRAINER_CLASS_LEADER:
            PlayBGM(MUS_VICTORY_GYM_LEADER);
            break;
        default:
            PlayBGM(MUS_VICTORY_TRAINER);
            break;
        }
    }
    else
    {
        gBattlescriptCurrInstr = BattleScript_PayDayMoneyAndPickUpItems;
    }

    gBattleMainFunc = HandleEndTurn_FinishBattle;
}

static void HandleEndTurn_BattleLost(void)
{
    gCurrentActionFuncId = 0;
    gBattlescriptCurrInstr = BattleScript_LocalBattleLost;
    gBattleMainFunc = HandleEndTurn_FinishBattle;
}

static void HandleEndTurn_RanFromBattle(void)
{
    gCurrentActionFuncId = 0;

    switch (gProtectStructs[gBattlerAttacker].fleeType)
    {
    default:
        gBattlescriptCurrInstr = BattleScript_GotAwaySafely;
        break;
    case FLEE_ITEM:
        gBattlescriptCurrInstr = BattleScript_SmokeBallEscape;
        break;
    case FLEE_ABILITY:
        gBattlescriptCurrInstr = BattleScript_RanAwayUsingMonAbility;
        break;
    }

    gBattleMainFunc = HandleEndTurn_FinishBattle;
}

static void HandleEndTurn_MonFled(void)
{
    gCurrentActionFuncId = 0;

    PREPARE_MON_NICK_BUFFER(gBattleTextBuff1, gBattlerAttacker, gBattlerPartyIndexes[gBattlerAttacker]);
    gBattlescriptCurrInstr = BattleScript_WildMonFled;

    gBattleMainFunc = HandleEndTurn_FinishBattle;
}

static void HandleEndTurn_FinishBattle(void)
{
    u32 i, battler;

    if (gCurrentActionFuncId == B_ACTION_TRY_FINISH || gCurrentActionFuncId == B_ACTION_FINISHED)
    {
        for (battler = 0; battler < gBattlersCount; battler++)
        {
            if (GetBattlerSide(battler) == B_SIDE_PLAYER)
            {
                if (gBattleResults.playerMon1Species == SPECIES_NONE)
                {
                    gBattleResults.playerMon1Species = GetMonData(&gPlayerParty[gBattlerPartyIndexes[battler]], MON_DATA_SPECIES, NULL);
                    GetMonData(&gPlayerParty[gBattlerPartyIndexes[battler]], MON_DATA_NICKNAME, gBattleResults.playerMon1Name);
                }
                else
                {
                    gBattleResults.playerMon2Species = GetMonData(&gPlayerParty[gBattlerPartyIndexes[battler]], MON_DATA_SPECIES, NULL);
                    GetMonData(&gPlayerParty[gBattlerPartyIndexes[battler]], MON_DATA_NICKNAME, gBattleResults.playerMon2Name);
                }
            }
        }

        if (!(gBattleTypeFlags & (TIPO_BATALLA_ENTRENADOR))
            && gBattleResults.shinyWildMon)

        EmpiezaFundidoPaletasRapido(FUNDIDO_A_NEGRO);
        FadeOutMapMusic(5);
        if (B_TRAINERS_KNOCK_OFF_ITEMS == TRUE || B_RESTORE_HELD_BATTLE_ITEMS >= GEN_9)
            TryRestoreHeldItems();

        for (i = 0; i < PARTY_SIZE; i++)
        {
            bool8 changedForm = FALSE;

            if (!changedForm)
                changedForm = TryFormChange(i, B_SIDE_PLAYER, FORM_CHANGE_END_BATTLE);

            // Clear original species field
            gBattleStruct->changedSpecies[B_SIDE_PLAYER][i] = SPECIES_NONE;
            gBattleStruct->changedSpecies[B_SIDE_OPPONENT][i] = SPECIES_NONE;

            // Recalculate the stats of every party member before the end
            if (!changedForm)
                CalculateMonStats(&gPlayerParty[i]);
        }
        // Clear battle mon species to avoid a bug on the next battle that causes
        // healthboxes loading incorrectly due to it trying to create a Mega Indicator
        // if the previous battler would've had it.
        for (i = 0; i < MAX_BATTLERS_COUNT; i++)
        {
            gBattleMons[i].species = SPECIES_NONE;
        }
        gBattleMainFunc = FreeResetData_ReturnToOvOrDoEvolutions;
        gCB2_AfterEvolution = BattleMainCB2;
    }
    else
    {
        if (gBattleControllerExecFlags == 0)
            gBattleScriptingCommandsTable[gBattlescriptCurrInstr[0]]();
    }
}

static void FreeResetData_ReturnToOvOrDoEvolutions(void)
{
    if (!gFundidoPaletas.activo)
    {
        gIsFishingEncounter = FALSE;
        gIsSurfingEncounter = FALSE;
        ResetSpriteData();
        if ((B_EVOLUTION_AFTER_WHITEOUT >= GEN_6 || gBattleOutcome == B_OUTCOME_WON || gBattleOutcome == B_OUTCOME_CAUGHT))
        {
            gBattleMainFunc = TryEvolvePokemon;
        }
        else
        {
            gBattleMainFunc = ReturnFromBattleToOverworld;
            return;
        }
    }

    FreeAllWindowBuffers();
    ZeroEnemyPartyMons();
    ResetDynamicAiFunc();
    FreeMonSpritesGfx();
    FreeBattleResources();
    FreeBattleSpritesData();
}

static void TryEvolvePokemon(void)
{
    s32 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (!(sTriedEvolving & (1u << i)))
        {
            u16 species = GetEvolutionTargetSpecies(&gPlayerParty[i], ITEM_NONE);
            sTriedEvolving |= 1u << i;

            if (species != SPECIES_NONE)
            {
                FreeAllWindowBuffers();
                gBattleMainFunc = WaitForEvoSceneToFinish;
                EvolutionScene(&gPlayerParty[i], species, i);
                return;
            }
        }
    }
    gBattleMainFunc = ReturnFromBattleToOverworld;
}

static void WaitForEvoSceneToFinish(void)
{
    if (gMain.callback2 == BattleMainCB2)
        gBattleMainFunc = TryEvolvePokemon;
}

static void ReturnFromBattleToOverworld(void)
{
    gSpecialVar_Result = gBattleOutcome;
    gMain.inBattle = FALSE;
    gMain.callback1 = gPreBattleCallback1;

    m4aSongNumStop(SE_LOW_HEALTH);
    SetMainCallback2(gMain.savedCallback);
}

void RunBattleScriptCommands_PopCallbacksStack(void)
{
    if (gCurrentActionFuncId == B_ACTION_TRY_FINISH || gCurrentActionFuncId == B_ACTION_FINISHED)
    {
        if (gBattleResources->battleCallbackStack->size != 0)
            gBattleResources->battleCallbackStack->size--;
        gBattleMainFunc = gBattleResources->battleCallbackStack->function[gBattleResources->battleCallbackStack->size];
    }
    else
    {
        if (gBattleControllerExecFlags == 0)
            gBattleScriptingCommandsTable[gBattlescriptCurrInstr[0]]();
    }
}

void RunBattleScriptCommands(void)
{
    if (gBattleControllerExecFlags == 0)
        gBattleScriptingCommandsTable[gBattlescriptCurrInstr[0]]();
}
// Returns TYPE_NONE if type doesn't change.
// NULL can be passed to ateBoost to avoid applying ate-ability boosts when opening the summary screen in-battle.
u32 GetDynamicMoveType(struct Pokemon *mon, u32 move, u32 battler, u8 *ateBoost)
{
    u32 moveType = gMovesInfo[move].type;
    u32 moveEffect = gMovesInfo[move].effect;
    u32 heldItem, holdEffect, ability;

    if (move == MOVE_STRUGGLE)
        return TIPO_NORMAL;

    if (gMain.inBattle)
    {
        heldItem = gBattleMons[battler].item;
        holdEffect = GetBattlerHoldEffect(battler, TRUE);
        ability = GetBattlerAbility(battler);
    }
    else
    {
        heldItem = GetMonData(mon, MON_DATA_HELD_ITEM, 0);
        holdEffect = ItemId_GetHoldEffect(heldItem);
        ability = GetMonAbility(mon);
    }

    switch (moveEffect)
    {
    case EFFECT_WEATHER_BALL:
        if (gMain.inBattle && WEATHER_HAS_EFFECT)
        {
            if (gBattleWeather & B_WEATHER_RAIN && holdEffect != HOLD_EFFECT_UTILITY_UMBRELLA)
                return TIPO_AGUA;
            else if (gBattleWeather & B_WEATHER_SANDSTORM)
                return TIPO_ROCA;
            else if (gBattleWeather & B_WEATHER_SUN && holdEffect != HOLD_EFFECT_UTILITY_UMBRELLA)
                return TIPO_FUEGO;
            else if (gBattleWeather & (B_WEATHER_SNOW | B_WEATHER_HAIL))
                return TIPO_HIELO;
            else
                return moveType;
        }
        else
        {
            switch (gWeatherPtr->currWeather)
            {
            case WEATHER_DROUGHT:
                if (holdEffect != HOLD_EFFECT_UTILITY_UMBRELLA)
                    return TIPO_FUEGO;
                break;
            case WEATHER_RAIN:
            case WEATHER_RAIN_THUNDERSTORM:
                if (holdEffect != HOLD_EFFECT_UTILITY_UMBRELLA)
                    return TIPO_AGUA;
                break;
            case WEATHER_SNOW:
                return TIPO_HIELO;
            case WEATHER_SANDSTORM:
                return TIPO_ROCA;
            }
            return moveType;
        }
        break;
    case EFFECT_HIDDEN_POWER:
        {
            u32 typeBits = 0;
            if (gMain.inBattle)
            {
                typeBits = ((gBattleMons[battler].hpIV & 1) << 0)
                        | ((gBattleMons[battler].attackIV & 1) << 1)
                        | ((gBattleMons[battler].defenseIV & 1) << 2)
                        | ((gBattleMons[battler].speedIV & 1) << 3)
                        | ((gBattleMons[battler].spAttackIV & 1) << 4)
                        | ((gBattleMons[battler].spDefenseIV & 1) << 5);
            }
            else
            {
                typeBits = ((GetMonData(mon, MON_DATA_HP_IV) & 1) << 0)
                        | ((GetMonData(mon, MON_DATA_ATK_IV) & 1) << 1)
                        | ((GetMonData(mon, MON_DATA_DEF_IV) & 1) << 2)
                        | ((GetMonData(mon, MON_DATA_SPEED_IV) & 1) << 3)
                        | ((GetMonData(mon, MON_DATA_SPATK_IV) & 1) << 4)
                        | ((GetMonData(mon, MON_DATA_SPDEF_IV) & 1) << 5);
            }

            // Subtract 6 instead of 1 below because 5 types are excluded (TYPE_NONE, TIPO_NORMAL, TIPO_MISTERIO & TIPO_HADA)
            // The final + 2 skips past TYPE_NONE and Normal.
            moveType = ((NUMERO_DE_TIPOS - 6) * typeBits) / 63 + 2;
            if (moveType >= TIPO_MISTERIO)
                moveType++;
            return ((moveType | F_DYNAMIC_TYPE_IGNORE_PHYSICALITY) & 0x3F);
        }
        break;
    case EFFECT_CHANGE_TYPE_ON_ITEM:
        if (holdEffect == gMovesInfo[move].argument)
            return ItemId_GetSecondaryId(heldItem);
        break;
    case EFFECT_REVELATION_DANCE:
    case EFFECT_RAGING_BULL:
    case EFFECT_IVY_CUDGEL:
    case EFFECT_NATURAL_GIFT:
    case EFFECT_TERRAIN_PULSE:
        break;
    }

    if (moveType == TIPO_NORMAL
     && ((!gMain.inBattle)))
    {
        if (gMain.inBattle && ateBoost != NULL)
            *ateBoost = TRUE;
    }
    else if (moveType != TIPO_NORMAL
          && moveEffect != EFFECT_HIDDEN_POWER
          && moveEffect != EFFECT_WEATHER_BALL
          && ability == ABILITY_NORMALIZE)
    {
        if (gMain.inBattle && ateBoost != NULL)
            *ateBoost = TRUE;
        return TIPO_NORMAL;
    }
    else if (gMovesInfo[move].soundMove && ability == ABILITY_LIQUID_VOICE)
    {
        return TIPO_AGUA;
    }

    return TIPO_NINGUNO;
}

void SetTypeBeforeUsingMove(u32 move, u32 battler)
{
    u32 moveType;
    u32 heldItem = gBattleMons[battler].item;
    u32 holdEffect = GetBattlerHoldEffect(battler, TRUE);

    gBattleStruct->dynamicMoveType = 0;
    gBattleStruct->ateBoost[battler] = FALSE;
    gSpecialStatuses[battler].gemBoost = FALSE;

    moveType = GetDynamicMoveType(&GetBattlerParty(battler)[gBattlerPartyIndexes[battler]],
                                  move,
                                  battler,
                                  &gBattleStruct->ateBoost[battler]);
    if (moveType != TIPO_NINGUNO)
        gBattleStruct->dynamicMoveType = moveType | F_DYNAMIC_TYPE_SET;

    moveType = GetMoveType(move);
    if ((gFieldStatuses & STATUS_FIELD_ION_DELUGE && moveType == TIPO_NORMAL)
        || gStatuses4[battler] & STATUS4_ELECTRIFIED)
        gBattleStruct->dynamicMoveType = TIPO_ELECTRICO | F_DYNAMIC_TYPE_SET;

    // Check if a gem should activate.
    if (holdEffect == HOLD_EFFECT_GEMS && GetMoveType(move) == ItemId_GetSecondaryId(heldItem))
    {
        gSpecialStatuses[battler].gemParam = GetBattlerHoldEffectParam(battler);
        gSpecialStatuses[battler].gemBoost = TRUE;
    }
}

bool32 IsWildMonSmart(void)
{
#if B_SMART_WILD_AI_FLAG != 0
    return (FlagGet(B_SMART_WILD_AI_FLAG));
#else
    return FALSE;
#endif
}

static s32 Factorial(s32 n)
{
    s32 f = 1, i;
    for (i = 2; i <= n; i++)
        f *= i;
    return f;
}
