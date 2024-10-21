#include "global.h"
#include "battle_main.h"
#include "battle_util.h"
#include "bg.h"
#include "contest_effect.h"
#include "data.h"
#include "daycare.h"
#include "decompress.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "graphics.h"
#include "international_string_util.h"
#include "item.h"
#include "item_icon.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "m4a.h"
#include "overworld.h"
#include "palette.h"
#include "party_menu.h"
#include "pokedex.h"
#include "pokedex_area_screen.h"
#include "pokemon_icon.h"
#include "pokemon_summary_screen.h"
#include "region_map.h"
#include "pokemon.h"
#include "reset_rtc_screen.h"
#include "scanline_effect.h"
#include "shop.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text_window.h"
#include "trainer_pokemon_sprites.h"
#include "trig.h"
#include "window.h"
#include "constants/abilities.h"
#include "constants/form_change_types.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/party_menu.h"
#include "constants/rgb.h"
#include "constants/songs.h"

enum
{
    PAGE_MAIN,
    PAGE_INFO,
    PAGE_UNK,
    PAGE_AREA
};

enum
{
    INFO_SCREEN,
    STATS_SCREEN,
    EVO_SCREEN,
    FORMS_SCREEN,
    AREA_SCREEN,
    CANCEL_SCREEN,
    SCREEN_COUNT
};

enum {
    COLOR_LIGHT,
    COLOR_DARK,
    COLOR_CURSOR_SELECTED,
    COLOR_MOVE_INFO,
    COLOR_TITLE
};

enum {
    WIN_POKEMON_LIST,
    WIN_TITLE,
    WIN_SEEN,
    WIN_OWN
};

enum {
    BG_TEXTO,
    BG_CURSOR,
    BG_FONDO,
    BG_SCROLLING
};

// static .rodata strings

static const u8 sText_No0000[] = _("0000");
static const u8 sCaughtBall_Gfx[] = INCBIN_U8("graphics/pokedex/caught_ball.4bpp");
static const u8 sText_TenDashes[] = _("----------");
ALIGNED(4) static const u8 sExpandedPlaceholder_PokedexDescription[] = _("");

static const u8 sText_Stats_Buttons[] = _("{A_BUTTON}TOGGLE   {DPAD_UPDOWN}MOVES");
static const u8 sText_Stats_Buttons_Decapped[] = _("{A_BUTTON}Toggle   {DPAD_UPDOWN}Moves");
static const u8 sText_Stats_HP[] = _("PS:");
static const u8 sText_Stats_Attack[] = _("Ata:");
static const u8 sText_Stats_Defense[] = _("Def:");
static const u8 sText_Stats_Speed[] = _("Vel:");
static const u8 sText_Stats_SpAttack[] = _("AEsp:");
static const u8 sText_Stats_SpDefense[] = _("DEsp:");
static const u8 sText_Stats_Total[] = _("Total:");
static const u8 sText_Stats_EV_Plus1[] = _("{UP_ARROW_2}");
static const u8 sText_Stats_EV_Plus2[] = _("{UP_ARROW_2}{UP_ARROW_2}");
static const u8 sText_Stats_EV_Plus3[] = _("{UP_ARROW_2}{UP_ARROW_2}{UP_ARROW_2}");
static const u8 sText_Stats_EvStr1Str2[] = _("{STR_VAR_1}{STR_VAR_2}");
static const u8 sText_Stats_MoveSelectedMax[] = _("{STR_VAR_1} / {STR_VAR_2}");
static const u8 sText_Stats_MoveLevel[] = _("LVL");
static const u8 sText_Stats_Gender_0[] = _("♂");
static const u8 sText_Stats_Gender_12_5[] = _("♀ 1/7 ♂"); //_("♀ 12.5 / 87.5 ♂");
static const u8 sText_Stats_Gender_25[] = _("♀ 1/3 ♂");     //_("♀ 25 / 75 ♂");
static const u8 sText_Stats_Gender_50[] = _("♀ 1/1 ♂");     //_("♀ 50 / 50 ♂");
static const u8 sText_Stats_Gender_75[] = _("♀ 3/1 ♂");     //_("♀ 75 / 25 ♂");
static const u8 sText_Stats_Gender_87_5[] = _("♀ 7/1 ♂");
static const u8 sText_Stats_Gender_100[] = _("♀");
static const u8 sText_Stats_ExpYield[] = _("EXP YIELD: ");
static const u8 sText_Stats_EggCycles[] = _("HATCH STEPS:");
static const u8 sText_Stats_EggCycles_VeryFast[] = _("{EMOJI_BOLT}{EMOJI_DIZZYEGG}");
static const u8 sText_Stats_EggCycles_Fast[] = _("{EMOJI_DIZZYEGG}");
static const u8 sText_Stats_EggCycles_Normal[] = _("{EMOJI_DIZZYEGG}{EMOJI_DIZZYEGG}");
static const u8 sText_Stats_EggCycles_Slow[] = _("{EMOJI_DIZZYEGG}{EMOJI_DIZZYEGG}{EMOJI_DIZZYEGG}");
static const u8 sText_Stats_Growthrate[] = _("GROW: ");
static const u8 sText_Stats_Friendship[] = _("FRIENDSHIP:");
static const u8 sText_Stats_Friendship_BigAnger[] = _("{EMOJI_BIGANGER}");
static const u8 sText_Stats_Friendship_Neutral[] = _("{EMOJI_NEUTRAL}");
static const u8 sText_Stats_Friendship_Happy[] = _("{EMOJI_HAPPY}");
static const u8 sText_Stats_Friendship_BigSmile[] = _("{EMOJI_BIGSMILE}");
static const u8 sText_Stats_MEDIUM_FAST[] = _("MED. FAST");
static const u8 sText_Stats_ERRATIC[] = _("ERRATIC");
static const u8 sText_Stats_FLUCTUATING[] = _("FLUCTUATING");
static const u8 sText_Stats_MEDIUM_SLOW[] = _("MED. SLOW");
static const u8 sText_Stats_FAST[] = _("FAST");
static const u8 sText_Stats_SLOW[] = _("SLOW");
static const u8 sText_Stats_ContestHeart[] = _("H");
static const u8 sText_Stats_Minus[] = _("-");
static const u8 sText_Stats_eggGroup[] = _("EGG G1:");
static const u8 sText_Stats_eggGroup_Groups[] = _("{STR_VAR_1}/{STR_VAR_2}");
static const u8 sText_Stats_eggGroup_MONSTER[] = _("MONSTER");
static const u8 sText_Stats_eggGroup_WATER_1[] = _("WATER {CIRCLE_1}");
static const u8 sText_Stats_eggGroup_BUG[] = _("BUG");
static const u8 sText_Stats_eggGroup_FLYING[] = _("FLYING");
static const u8 sText_Stats_eggGroup_FIELD[] = _("FIELD");
static const u8 sText_Stats_eggGroup_FAIRY[] = _("FAIRY");
static const u8 sText_Stats_eggGroup_GRASS[] = _("GRASS");
static const u8 sText_Stats_eggGroup_HUMAN_LIKE[] = _("HUMANE");
static const u8 sText_Stats_eggGroup_WATER_3[] = _("WATER {CIRCLE_3}");
static const u8 sText_Stats_eggGroup_MINERAL[] = _("MINERAL");
static const u8 sText_Stats_eggGroup_AMORPHOUS[] = _("AMORPHOUS");
static const u8 sText_Stats_eggGroup_WATER_2[] = _("WATER {CIRCLE_2}");
static const u8 sText_Stats_eggGroup_DITTO[] = _("DITTO");
static const u8 sText_Stats_eggGroup_DRAGON[] = _("DRAGON");
static const u8 sText_Stats_eggGroup_NO_EGGS_DISCOVERED[] = _("---");

static const u8 sText_EVO_Buttons[] = _("{DPAD_UPDOWN}EVOs  {A_BUTTON}CHECK");
static const u8 sText_EVO_Buttons_Decapped[] = _("{DPAD_UPDOWN}Evos  {A_BUTTON}Check");
static const u8 sText_EVO_Buttons_PE[] = _("{DPAD_UPDOWN}EVOs  {A_BUTTON}CHECK  {START_BUTTON}FORMs");
static const u8 sText_EVO_Buttons_Decapped_PE[] = _("{DPAD_UPDOWN}Evos  {A_BUTTON}Check  {START_BUTTON}Forms");
static const u8 sText_EVO_Name[] = _("{STR_VAR_3}:");
static const u8 sText_EVO_PreEvo[] = _("{STR_VAR_1} evolves from {STR_VAR_2}");
static const u8 sText_EVO_PreEvo_PE_Mega[] = _("{STR_VAR_1} Mega Evolves with {STR_VAR_2}");
static const u8 sText_EVO_FRIENDSHIP[] = _("{LV}{UP_ARROW}, high friendship");
static const u8 sText_EVO_FRIENDSHIP_DAY[] = _("{LV}{UP_ARROW}, high friendship, day");
static const u8 sText_EVO_FRIENDSHIP_NIGHT[] = _("{LV}{UP_ARROW}, high friendship, night");
static const u8 sText_EVO_FRIENDSHIP_MOVE_TYPE[] = _("{LV}{UP_ARROW}, high friendship, {STR_VAR_2} move");
static const u8 sText_EVO_LEVEL[] = _("{LV}{UP_ARROW} to {STR_VAR_2}");
static const u8 sText_EVO_TRADE[] = _("Trading");
static const u8 sText_EVO_TRADE_ITEM[] = _("Trading, holding {STR_VAR_2}");
static const u8 sText_EVO_ITEM[] = _("{STR_VAR_2} is used");
static const u8 sText_EVO_LEVEL_ATK_GT_DEF[] = _("{LV}{UP_ARROW} to {STR_VAR_2}, Atk > Def");
static const u8 sText_EVO_LEVEL_ATK_EQ_DEF[] = _("{LV}{UP_ARROW} to {STR_VAR_2}, Atk = Def");
static const u8 sText_EVO_LEVEL_ATK_LT_DEF[] = _("{LV}{UP_ARROW} to {STR_VAR_2}, Atk < Def");
static const u8 sText_EVO_LEVEL_SILCOON[] = _("{LV}{UP_ARROW} to {STR_VAR_2}, Silcoon persona");
static const u8 sText_EVO_LEVEL_CASCOON[] = _("{LV}{UP_ARROW} to {STR_VAR_2}, Cascoon persona");
static const u8 sText_EVO_LEVEL_NINJASK[] = _("{LV}{UP_ARROW} to {STR_VAR_2}");
static const u8 sText_EVO_LEVEL_SHEDINJA[] = _("{LV}{UP_ARROW} to {STR_VAR_2}, party<6, 1x POKéBALL");
static const u8 sText_EVO_BEAUTY[] = _("{LV}{UP_ARROW}, high beauty");
static const u8 sText_EVO_LEVEL_FEMALE[] = _("{LV}{UP_ARROW} to {STR_VAR_2}, is female");
static const u8 sText_EVO_LEVEL_MALE[] = _("{LV}{UP_ARROW} to {STR_VAR_2}, is male");
static const u8 sText_EVO_LEVEL_NIGHT[] = _("{LV}{UP_ARROW} to {STR_VAR_2}, night");
static const u8 sText_EVO_LEVEL_DAY[] = _("{LV}{UP_ARROW} to {STR_VAR_2}, day");
static const u8 sText_EVO_LEVEL_DUSK[] = _("{LV}{UP_ARROW} to {STR_VAR_2}, dusk (5-6PM)");
static const u8 sText_EVO_ITEM_HOLD_DAY[] = _("{LV}{UP_ARROW}, holds {STR_VAR_2}, day");
static const u8 sText_EVO_ITEM_HOLD_NIGHT[] = _("{LV}{UP_ARROW}, holds {STR_VAR_2}, night");
static const u8 sText_EVO_MOVE[] = _("{LV}{UP_ARROW}, knows {STR_VAR_2}");
static const u8 sText_EVO_MAPSEC[] = _("{LV}{UP_ARROW} on {STR_VAR_2}");
static const u8 sText_EVO_ITEM_MALE[] = _("{STR_VAR_2} used on male");
static const u8 sText_EVO_ITEM_FEMALE[] = _("{STR_VAR_2} used on female");
static const u8 sText_EVO_LEVEL_RAIN[] = _("{LV}{UP_ARROW} to {STR_VAR_2} while raining");
static const u8 sText_EVO_SPECIFIC_MON_IN_PARTY[] = _("{LV}{UP_ARROW} with {STR_VAR_2} in party");
static const u8 sText_EVO_LEVEL_DARK_TYPE_MON_IN_PARTY[] = _("{LV}{UP_ARROW} with dark type in party");
static const u8 sText_EVO_TRADE_SPECIFIC_MON[] = _("Traded for {STR_VAR_2}");
static const u8 sText_EVO_SPECIFIC_MAP[] = _("{LV}{UP_ARROW} on {STR_VAR_2}");
static const u8 sText_EVO_LEVEL_NATURE_AMPED[] = _("{LV}{UP_ARROW} to {STR_VAR_2}, Amped natures");
static const u8 sText_EVO_LEVEL_NATURE_LOW_KEY[] = _("{LV}{UP_ARROW} to {STR_VAR_2}, Low Key natures");
static const u8 sText_EVO_CRITICAL_HITS[] = _("Land {STR_VAR_2} critical hits in\nsingle battle");
static const u8 sText_EVO_SCRIPT_TRIGGER_DMG[] = _("Takes at least {STR_VAR_2} HP in damage");
static const u8 sText_EVO_DARK_SCROLL[] = _("ScrllOfDrknss is used");
static const u8 sText_EVO_WATER_SCROLL[] = _("ScrollOfWatrs is used");
static const u8 sText_EVO_ITEM_NIGHT[] = _("{STR_VAR_2} is used, night");
static const u8 sText_EVO_ITEM_DAY[] = _("{STR_VAR_2} is used, day");
static const u8 sText_EVO_ITEM_HOLD[] = _("{LV}{UP_ARROW}, holds {STR_VAR_2}");
static const u8 sText_EVO_USE_MOVE_TWENTY_TIMES[] = _("{LV}{UP_ARROW} after 20x {STR_VAR_2}");
static const u8 sText_EVO_RECOIL_DAMAGE_MALE[] = _("{LV}{UP_ARROW} with {STR_VAR_2} recoil, male");
static const u8 sText_EVO_RECOIL_DAMAGE_FEMALE[] = _("{LV}{UP_ARROW} with {STR_VAR_2} recoil, female");
static const u8 sText_EVO_ITEM_COUNT_999[] = _("{LV}{UP_ARROW} with 999 {STR_VAR_2} in bag");
static const u8 sText_EVO_UNKNOWN[] = _("Method unknown");
static const u8 sText_EVO_NONE[] = _("{STR_VAR_1} has no evolution.");

static const u8 sText_FORMS_Buttons_PE[] = _("{A_BUTTON}FORM MODE  {START_BUTTON}EVOs");
static const u8 sText_FORMS_Buttons_Decapped_PE[] = _("{START_BUTTON}Evos");
static const u8 sText_FORMS_Buttons_Submenu_PE[] = _("{DPAD_NONE}FORMs {A_BUTTON}CHECK {START_BUTTON}EVOs");
static const u8 sText_FORMS_Buttons_Submenu_Decapped_PE[] = _("{START_BUTTON}Evos");
static const u8 sText_FORMS_NONE[] = _("{STR_VAR_1} has no alternate forms.");
static const u8 sText_PlusSymbol[] = _("+");

// static .rodata graphics

static const u16 sFondoPal[] = INCBIN_U16("graphics/pokedex/fondo.gbapal");
static const u32 sPokedexPlusHGSS_Interface_Gfx[] = INCBIN_U32("graphics/pokedex/tileset_interface.4bpp.lz");
static const u32 sPokedexPlusHGSS_Interface_DECA_Gfx[] = INCBIN_U32("graphics/pokedex/tileset_interface_DECA.4bpp.lz");
static const u32 sPokedexPlusHGSS_Menu_1_Gfx[] = INCBIN_U32("graphics/pokedex/tileset_menu1.4bpp.lz");
static const u32 sPokedexPlusHGSS_Menu_2_Gfx[] = INCBIN_U32("graphics/pokedex/tileset_menu2.4bpp.lz");
static const u32 sPokedexPlusHGSS_Menu_3_Gfx[] = INCBIN_U32("graphics/pokedex/tileset_menu3.4bpp.lz");
static const u32 sPokedexPlusHGSS_ScreenSelectBarSubmenu_Tilemap[] = INCBIN_U32("graphics/pokedex/SelectBar.bin.lz");
static const u32 sPokedexPlusHGSS_ScreenSelectBarSubmenu_Tilemap_Clear[] = INCBIN_U32("graphics/pokedex/SelectBar_clear.bin.lz");
static const u32 sFondoLista[] = INCBIN_U32("graphics/pokedex/fondo_lista.bin.lz");
static const u32 sTilesFondo[] = INCBIN_U32("graphics/pokedex/tiles_fondo.4bpp.lz");
static const u32 sFondoScrolling[] = INCBIN_U32("graphics/pokedex/fondo_scrolling.bin.lz");
static const u32 sPokedexPlusHGSS_ScreenInfo_Tilemap[] = INCBIN_U32("graphics/pokedex/tilemap_info_screen.bin.lz");
static const u32 sPokedexPlusHGSS_ScreenStats_Tilemap[] = INCBIN_U32("graphics/pokedex/tilemap_stats_screen.bin.lz");
static const u32 sPokedexPlusHGSS_ScreenEvolution_Tilemap[] = INCBIN_U32("graphics/pokedex/tilemap_evo_screen.bin.lz");
static const u32 sPokedexPlusHGSS_ScreenEvolution_Tilemap_PE[] = INCBIN_U32("graphics/pokedex/tilemap_evo_screen_PE.bin.lz");
static const u32 sPokedexPlusHGSS_ScreenForms_Tilemap[] = INCBIN_U32("graphics/pokedex/tilemap_forms_screen.bin.lz");

#define SCROLLING_MON_X 30
#define HGSS_DECAPPED TRUE
#define HGSS_DARK_MODE TRUE
#define HGSS_HIDE_UNSEEN_EVOLUTION_NAMES FALSE

#define MAX_MONS_ON_SCREEN 4

#define LIST_SCROLL_STEP         16

#define POKEBALL_ROTATION_TOP    64
#define POKEBALL_ROTATION_BOTTOM (POKEBALL_ROTATION_TOP - 16)

// Coordinates of the Pokémon sprite on its page (info screen)
#define MON_PAGE_X 48
#define MON_PAGE_Y 56

static EWRAM_DATA struct PokedexView *sPokedexView = NULL;
static EWRAM_DATA u16 sLastSelectedPokemon = 0;
static EWRAM_DATA u8 sPokeBallRotation = 0;
static EWRAM_DATA struct PokedexListItem *sPokedexListItem = NULL;
#define MOVES_COUNT_TOTAL (EGG_MOVES_ARRAY_COUNT + MAX_LEVEL_UP_MOVES + NUM_TECHNICAL_MACHINES + NUM_HIDDEN_MACHINES)
EWRAM_DATA static u16 sStatsMoves[MOVES_COUNT_TOTAL] = {0};
EWRAM_DATA static u16 sStatsMovesTMHM_ID[NUM_TECHNICAL_MACHINES + NUM_HIDDEN_MACHINES] = {0};

struct PokedexListItem
{
    u16 dexNum;
    u16 seen:1;
    u16 owned:1;
};

struct PokemonStats
{
    u16 species;
    u8  genderRatio;
    u8  baseHP;
    u8  baseSpeed;
    u8  baseAttack;
    u8  baseSpAttack;
    u8  baseDefense;
    u8  baseSpDefense;
    u8  differentEVs;
    u8  evYield_HP;
    u8  evYield_Speed;
    u8  evYield_Attack;
    u8  evYield_SpAttack;
    u8  evYield_Defense;
    u8  evYield_SpDefense;
    u8  growthRate;
    u8  eggGroup1;
    u8  eggGroup2;
    u8  eggCycles;
    u8  expYield;
    u8  friendship;
    u16 ability0;
    u16 ability1;
    u16 abilityHidden;
};

struct EvoScreenData
{
    bool8 fromEvoPage;
    u8 numAllEvolutions;
    u16 targetSpecies[10];
    u8 numSeen;
    bool8 seen[10];
    u8 menuPos;
    u8 arrowSpriteId;
    bool8 isMega;
};

struct FromScreenData
{
    u8 numForms;
    u16 formIds[30];
    bool8 inSubmenu;
    u8 menuPos;
    u8 arrowSpriteId;
};

struct PokedexView
{
    struct PokedexListItem pokedexList[DEX_COUNT + 1];
    u16 pokemonListCount;
    u16 selectedPokemon;
    u16 selectedPokemonBackup;
    u16 seenCount;
    u16 ownCount;
    u16 monSpriteIds[MAX_MONS_ON_SCREEN];
    u8 typeIconSpriteIds[2];
    u16 moveSelected;
    u8 movesTotal;
    u8 statBarsSpriteId;
    u8 statBarsBgSpriteId;
    bool8 justScrolled;
    u8 categoryIconSpriteId; //Physical/Special/Status category
    u8 numEggMoves;
    u8 numLevelUpMoves;
    u8 numTMHMMoves;
    u8 numTutorMoves;
    u8 numPreEvolutions;
    struct PokemonStats sPokemonStats;
    struct EvoScreenData sEvoScreenData;
    struct FromScreenData sFormScreenData;
    u16 formSpecies;
    u16 selectedMonSpriteId;
    u16 pokeBallRotationStep;
    u16 pokeBallRotationBackup;
    u8 pokeBallRotation;
    u8 initialVOffset;
    u8 scrollTimer;
    u8 scrollDirection;
    s16 listVOffset;
    s16 listMovingVOffset;
    u16 scrollMonIncrement;
    u16 maxScrollTimer;
    u16 scrollSpeed;
    u8 currentPage;
    u8 currentPageBackup;
    u8 selectedScreen;
    u8 screenSwitchState;
    u8 menuIsOpen;
    u16 menuCursorPos;
    s16 menuY;     //Menu Y position (inverted because we use REG_BG0VOFS for this)
};

static void ResetPokedexView(struct PokedexView *pokedexView);
static void VBlankCB_Pokedex(void);
static void CB2_Pokedex(void);
static void Task_OpenPokedexMainPage(u8);
static void Task_HandlePokedexInput(u8);
static void Task_WaitForScroll(u8);
static void Task_OpenInfoScreenAfterMonMovement(u8);
static void Task_WaitForExitInfoScreen(u8);
static void Task_ClosePokedex(u8);
static bool8 LoadPokedexListPage(u8);
static void LoadPokedexBgPalette(void);
static void FreeWindowAndBgBuffers(void);
static void CreatePokedexList(void);
static void CreateMonDexNum(u16, u8, u8);
static void CreateCaughtBall(u16, u8, u8);
static u8 CreateMonName(u16, u8, u8);
static void ClearMonListEntry(u8 x, u8 y);
static void CreateMonSpritesAtPos(u16);
static bool8 UpdateDexListScroll(u8, u8, u8);
static u16 TryDoPokedexScroll(u16);
static void UpdateSelectedMonSpriteId(void);
static bool8 TryDoInfoScreenScroll(void);
static u8 ClearMonSprites(void);
static u16 GetPokemonSpriteToDisplay(u16);
static u32 CreatePokedexMonSprite(u16, s16, s16);
static void CreateInterfaceSprites(u8);
static void SpriteCB_MoveMonForInfoScreen(struct Sprite *sprite);
static void SpriteCB_Scrollbar(struct Sprite *sprite);
static void SpriteCB_ScrollArrow(struct Sprite *sprite);
static void SpriteCB_DexListInterfaceText(struct Sprite *sprite);
static void SpriteCB_RotatingPokeBall(struct Sprite *sprite);
static void SpriteCB_DexListStartMenuCursor(struct Sprite *sprite);
static void SpriteCB_PokedexListMonSprite(struct Sprite *sprite);
static u8 LoadInfoScreen(struct PokedexListItem *, u8 monSpriteId);
static bool8 IsInfoScreenScrolling(u8);
static u8 StartInfoScreenScroll(struct PokedexListItem *, u8);
static void Task_LoadInfoScreen(u8);
static void Task_HandleInfoScreenInput(u8);
static void Task_SwitchScreensFromInfoScreen(u8);
static void Task_LoadInfoScreenWaitForFade(u8);
static void Task_ExitInfoScreen(u8);
static void Task_LoadAreaScreen(u8);
static void Task_WaitForAreaScreenInput(u8 taskId);
static void Task_SwitchScreensFromAreaScreen(u8);
static void LoadScreenSelectBarMain(void);
static void Task_HandleCaughtMonPageInput(u8);
static void Task_ExitCaughtMonPage(u8);
static void SpriteCB_SlideCaughtMonToCenter(struct Sprite *sprite);
static void PrintMonInfo(u32 num, u32 owned, u32 newEntry);
static u32 GetMeasurementTextPositions(u32 textElement);
static void PrintUnknownMonMeasurements(void);
static u8* GetUnknownMonHeightString(void);
static u8* GetUnknownMonWeightString(void);
static u8* ReplaceDecimalSeparator(const u8* originalString);
static void PrintOwnedMonMeasurements(u16 species);
static void PrintOwnedMonHeight(u16 species);
static void PrintOwnedMonWeight(u16 species);
static u8* ConvertMonHeightToMetricString(u32 height);
static u8* ConvertMonWeightToMetricString(u32 weight);
static u8* ConvertMeasurementToMetricString(u32 num, u32* index);
static void PrintMonInfo(u32 num, u32 owned, u32 newEntry);
static void ResetOtherVideoRegisters(u16);
u16 CreateMonSpriteFromNationalDexNumberHGSS(u16 nationalNum, s16 x, s16 y, u16 paletteSlot);
static u16 GetNextPosition(u8, u16, u16, u16);
static void SetSpriteInvisibility(u8 spriteArrayId, bool8 invisible);
static void CreateTypeIconSprites(void);
static void ScrollBackground(void);
static void PrintTitleSeenAndOwn(void);

//Stats screen HGSS_Ui
static void LoadTilesetTilemapHGSS(u8 page);
static void Task_HandleStatsScreenInput(u8 taskId);
static void Task_LoadStatsScreen(u8 taskId);
static void Task_SwitchScreensFromStatsScreen(u8 taskId);
static void Task_ExitStatsScreen(u8 taskId);
static bool8 CalculateMoves(void);
static void PrintStatsScreen_NameGender(u8 taskId, u32 num);
static void PrintStatsScreen_DestroyMoveItemIcon(u8 taskId);
static void PrintStatsScreen_Moves_Top(u8 taskId);
static void PrintStatsScreen_Moves_Description(u8 taskId);
static void PrintStatsScreen_Moves_BottomText(u8 taskId);
static void PrintStatsScreen_Moves_Bottom(u8 taskId);
static void PrintStatsScreen_Left(u8 taskId);
static void PrintStatsScreen_Abilities(u8 taskId);
static void PrintInfoScreenTextWhite(const u8* str, u8 left, u8 top);
static void PrintInfoScreenTextSmall(const u8* str, u8 left, u8 top);
static void Task_LoadEvolutionScreen(u8 taskId);
static void Task_HandleEvolutionScreenInput(u8 taskId);
static void Task_SwitchScreensFromEvolutionScreen(u8 taskId);
static void Task_ExitEvolutionScreen(u8 taskId);
static u8 PrintEvolutionTargetSpeciesAndMethod(u8 taskId, u16 species, u8 depth, u8 depth_i);
static u8 PrintPreEvolutions(u8 taskId, u16 species);
//Stat bars on scrolling screens
static void TryDestroyStatBars(void);
static void TryDestroyStatBarsBg(void);
static void CreateStatBars(struct PokedexListItem *dexMon);
static void CreateStatBarsBg(void);
static void SpriteCB_StatBars(struct Sprite *sprite);
static void SpriteCB_StatBarsBg(struct Sprite *sprite);

//Forms screen
static void Task_LoadFormsScreen(u8 taskId);
static void Task_HandleFormsScreenInput(u8 taskId);
static void PrintForms(u8 taskId, u16 species);
static void Task_SwitchScreensFromFormsScreen(u8 taskId);
static void Task_ExitFormsScreen(u8 taskId);

//Physical/Special/Status category icon
static u8 ShowCategoryIcon(u32 category);
static void DestroyCategoryIcon(void);

static u16 NationalPokedexNumToSpeciesHGSS(u16 nationalNum);

void (*gPokedexVBlankCB)(void);

static const union AnimCmd sSpriteAnim_CategoryIcon0[] =
{
    ANIMCMD_FRAME(0, 0),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_CategoryIcon1[] =
{
    ANIMCMD_FRAME(8, 0),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_CategoryIcon2[] =
{
    ANIMCMD_FRAME(16, 0),
    ANIMCMD_END
};
static const union AnimCmd *const sSpriteAnimTable_CategoryIcons[] =
{
    sSpriteAnim_CategoryIcon0,
    sSpriteAnim_CategoryIcon1,
    sSpriteAnim_CategoryIcon2,
};

static const u8 sTextColors[][3] = {
    [COLOR_TITLE] = {0, 7, 9},
};

//Stat bars by DizzyEgg
#define TAG_STAT_BAR 4097
#define TAG_STAT_BAR_BG 4098
static const struct OamData sOamData_StatBar =
{
    .y = 160,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(64x64),
    .size = SPRITE_SIZE(64x64),
};
static const struct OamData sOamData_StatBarBg =
{
    .y = 160,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(64x64),
    .size = SPRITE_SIZE(64x64),
};
static const struct SpriteTemplate sStatBarSpriteTemplate =
{
    .tileTag = TAG_STAT_BAR,
    .paletteTag = TAG_STAT_BAR,
    .oam = &sOamData_StatBar,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_StatBars,
};
static const struct SpriteTemplate sStatBarBgSpriteTemplate =
{
    .tileTag = TAG_STAT_BAR_BG,
    .paletteTag = TAG_STAT_BAR_BG,
    .oam = &sOamData_StatBarBg,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_StatBarsBg,
};
enum
{
    COLOR_ID_ALPHA,
    COLOR_ID_BAR_WHITE,
    // These are repeated 6 times
    COLOR_ID_FILL,
    COLOR_ID_FILL_SHADOW,
    COLOR_ID_FONT = 14,
    COLOR_ID_FONT_SHADOW = 15,
};
enum
{
    COLOR_BEST, // Light blue
    COLOR_VERY_GOOD, // Green
    COLOR_GOOD, // Light Green
    COLOR_AVERAGE, // Yellow
    COLOR_BAD, // Orange
    COLOR_WORST, // Red
};
static const u8 sStatBarsGfx[] = INCBIN_U8("graphics/pokedex/stat_bars.4bpp");
static const u16 sStatBarPalette[16] = {
    [COLOR_ID_ALPHA] = RGB(0, 0, 10),
    [COLOR_ID_BAR_WHITE] = RGB_WHITE,

    [COLOR_ID_FILL + COLOR_BEST * 2] = RGB(2, 25, 25),
    [COLOR_ID_FILL_SHADOW + COLOR_BEST * 2] = RGB(13, 27, 27),

    [COLOR_ID_FILL + COLOR_VERY_GOOD * 2] = RGB(11, 25, 2),
    [COLOR_ID_FILL_SHADOW + COLOR_VERY_GOOD * 2] = RGB(19, 27, 13),

    [COLOR_ID_FILL + COLOR_GOOD * 2] = RGB(22, 25, 2),
    [COLOR_ID_FILL_SHADOW + COLOR_GOOD * 2] = RGB(26, 27, 13),

    [COLOR_ID_FILL + COLOR_AVERAGE * 2] = RGB(25, 22, 2),
    [COLOR_ID_FILL_SHADOW + COLOR_AVERAGE * 2] = RGB(27, 26, 13),

    [COLOR_ID_FILL + COLOR_BAD * 2] = RGB(25, 17, 2),
    [COLOR_ID_FILL_SHADOW + COLOR_BAD * 2] = RGB(27, 22, 13),

    [COLOR_ID_FILL + COLOR_WORST * 2] = RGB(25, 4, 2),
    [COLOR_ID_FILL_SHADOW + COLOR_WORST * 2] = RGB(27, 15, 13),

    [COLOR_ID_FONT] = RGB_BLACK,
    [COLOR_ID_FONT_SHADOW] = RGB(22, 22, 22),
};
static const struct SpritePalette sStatBarSpritePal[] =
{
    {sStatBarPalette, TAG_STAT_BAR},
    {sStatBarPalette, TAG_STAT_BAR_BG},
    {0}
};

static const struct OamData sOamData_ScrollBar =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(8x8),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(8x8),
    .tileNum = 0,
    .priority = 1,
    .paletteNum = 0,
    .affineParam = 0
};

static const struct OamData sOamData_ScrollArrow =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(16x8),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(16x8),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0
};

static const struct OamData sOamData_InterfaceText =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(32x16),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(32x16),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0
};

static const struct OamData sOamData_RotatingPokeBall =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_WINDOW,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(32x32),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(32x32),
    .tileNum = 0,
    .priority = 1,
    .paletteNum = 0,
    .affineParam = 0
};

static const struct OamData sOamData_Dex8x16 =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(8x16),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(8x16),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0
};

static const union AnimCmd sSpriteAnim_ScrollBar[] =
{
    ANIMCMD_FRAME(3, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_ScrollArrow[] =
{
    ANIMCMD_FRAME(1, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_RotatingPokeBall[] =
{
    ANIMCMD_FRAME(16, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_StartButton[] =
{
    ANIMCMD_FRAME(48, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_SelectButton[] =
{
    ANIMCMD_FRAME(32, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_MenuText[] =
{
    ANIMCMD_FRAME(56, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_DexListStartMenuCursor[] =
{
    ANIMCMD_FRAME(4, 30),
    ANIMCMD_END
};

static const union AnimCmd *const sSpriteAnimTable_ScrollBar[] =
{
    sSpriteAnim_ScrollBar
};

static const union AnimCmd *const sSpriteAnimTable_ScrollArrow[] =
{
    sSpriteAnim_ScrollArrow
};

static const union AnimCmd *const sSpriteAnimTable_RotatingPokeBall[] =
{
    sSpriteAnim_RotatingPokeBall
};

static const union AnimCmd *const sSpriteAnimTable_InterfaceText[] =
{
    sSpriteAnim_StartButton,
    sSpriteAnim_SelectButton,
    sSpriteAnim_MenuText
};

static const union AnimCmd *const sSpriteAnimTable_DexListStartMenuCursor[] =
{
    sSpriteAnim_DexListStartMenuCursor
};

#define TAG_DEX_INTERFACE 4096 // Tile and pal tag used for all interface sprites.

static const struct SpriteTemplate sScrollBarSpriteTemplate =
{
    .tileTag = TAG_DEX_INTERFACE,
    .paletteTag = TAG_DEX_INTERFACE,
    .oam = &sOamData_ScrollBar,
    .anims = sSpriteAnimTable_ScrollBar,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Scrollbar,
};

static const struct SpriteTemplate sScrollArrowSpriteTemplate =
{
    .tileTag = TAG_DEX_INTERFACE,
    .paletteTag = TAG_DEX_INTERFACE,
    .oam = &sOamData_ScrollArrow,
    .anims = sSpriteAnimTable_ScrollArrow,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_ScrollArrow,
};

static const struct SpriteTemplate sInterfaceTextSpriteTemplate =
{
    .tileTag = TAG_DEX_INTERFACE,
    .paletteTag = TAG_DEX_INTERFACE,
    .oam = &sOamData_InterfaceText,
    .anims = sSpriteAnimTable_InterfaceText,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_DexListInterfaceText,
};

static const struct SpriteTemplate sRotatingPokeBallSpriteTemplate =
{
    .tileTag = TAG_DEX_INTERFACE,
    .paletteTag = TAG_DEX_INTERFACE,
    .oam = &sOamData_RotatingPokeBall,
    .anims = sSpriteAnimTable_RotatingPokeBall,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_RotatingPokeBall,
};

static const struct SpriteTemplate sDexListStartMenuCursorSpriteTemplate =
{
    .tileTag = TAG_DEX_INTERFACE,
    .paletteTag = TAG_DEX_INTERFACE,
    .oam = &sOamData_Dex8x16,
    .anims = sSpriteAnimTable_DexListStartMenuCursor,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_DexListStartMenuCursor,
};

static const struct CompressedSpriteSheet sInterfaceSpriteSheet[] =
{
    {sPokedexPlusHGSS_Interface_Gfx, 0x2000, TAG_DEX_INTERFACE},
    {sPokedexPlusHGSS_Interface_DECA_Gfx, 0x2000, TAG_DEX_INTERFACE},
    {0}
};

static const struct SpritePalette sInterfaceSpritePalette[] =
{
    {sFondoPal, TAG_DEX_INTERFACE},
    {0}
};

// By scroll speed. Last element of each unused
static const u8 sScrollMonIncrements[] = {4, 8, 16, 32, 32};
static const u8 sScrollTimers[] = {8, 4, 2, 1, 1};

static const struct BgTemplate sPokedex_BgTemplate[] =
{
    [BG_TEXTO] =
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 12,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
    [BG_CURSOR] =
    {
        .bg = 1,
        .charBaseIndex = 0,
        .mapBaseIndex = 13,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    },
    [BG_FONDO] =
    {
        .bg = 2,
        .charBaseIndex = 2,
        .mapBaseIndex = 14,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0
    },
    [BG_SCROLLING] =
    {
        .bg = 3,
        .charBaseIndex = 0,
        .mapBaseIndex = 15,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 3,
        .baseTile = 0
    }
};

static const struct WindowTemplate sPokemonList_WindowTemplate[] =
{
    [WIN_POKEMON_LIST] =
    {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 32,
        .height = 32,
        .paletteNum = 0,
        .baseBlock = 0,
    },
    [WIN_TITLE] =
    {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 0,
        .width = 7,
        .height = 2,
        .paletteNum = 0,
        .baseBlock = 0,
    },
    [WIN_SEEN] =
    {
        .bg = 0,
        .tilemapLeft = 17,
        .tilemapTop = 0,
        .width = 4,
        .height = 2,
        .paletteNum = 0,
        .baseBlock = 0,
    },
    [WIN_OWN] =
    {
        .bg = 0,
        .tilemapLeft = 24,
        .tilemapTop = 0,
        .width = 4,
        .height = 2,
        .paletteNum = 0,
        .baseBlock = 0,
    },
    DUMMY_WIN_TEMPLATE
};

static const struct BgTemplate sInfoScreen_BgTemplate[] =
{
    {
        .bg = 0,
        .charBaseIndex = 2,
        .mapBaseIndex = 12,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 3,
        .baseTile = 0
    },
    {
        .bg = 1,
        .charBaseIndex = 0,
        .mapBaseIndex = 13,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
    {
        .bg = 2,
        .charBaseIndex = 2,
        .mapBaseIndex = 14,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    },
    {
        .bg = 3,
        .charBaseIndex = 0,
        .mapBaseIndex = 15,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0
    }
};

#define WIN_INFO 0
#define WIN_NAVIGATION_BUTTONS 1

static const struct WindowTemplate sInfoScreen_WindowTemplates[] =
{
    [WIN_INFO] =
    {
        .bg = 2,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 32,
        .height = 20,
        .paletteNum = 0,
        .baseBlock = 1,
    },
    [WIN_NAVIGATION_BUTTONS] =
    {
        .bg = 2,
        .tilemapLeft = 0,
        .tilemapTop = 18,
        .width = 20,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 641,
    },
    DUMMY_WIN_TEMPLATE
};

#define WIN_STATS_TOPBAR 0
#define WIN_STATS_SIDEBAR 1
#define WIN_STATS_NAME_GENDER 2
#define WIN_STATS_LEFT 3
#define WIN_STATS_NAVIGATION_BUTTONS 4
#define WIN_STATS_MOVES_TOP 5
#define WIN_STATS_MOVES_DESCRIPTION 7
#define WIN_STATS_MOVES_BOTTOM 8
#define WIN_STATS_ABILITIES 9
#define WIN_STATS_LEFT_BLANK 10

#define WIN_STATS_END WIN_STATS_LEFT_BLANK

static const struct WindowTemplate sStatsScreen_WindowTemplates[] =
{
    [WIN_STATS_TOPBAR] =
    {
        .bg = 2,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 30,
        .height = 2,
        .paletteNum = 0,
        .baseBlock = 1,
    },
    [WIN_STATS_SIDEBAR] =
    {
        .bg = 2,
        .tilemapLeft = 30,
        .tilemapTop = 0,
        .width = 2,
        .height = 20,
        .paletteNum = 0,
        .baseBlock = 1 + 60,
    },
    [WIN_STATS_NAME_GENDER] =
    {
        .bg = 2,
        .tilemapLeft = 0,
        .tilemapTop = 2,
        .width = 12,
        .height = 4,
        .paletteNum = 0,
        .baseBlock = 1 + 60 + 40,
    },
    [WIN_STATS_LEFT] =
    {
        .bg = 2,
        .tilemapLeft = 0,
        .tilemapTop = 6,
        .width = 12,
        .height = 8,
        .paletteNum = 0,
        .baseBlock = 1 + 60 + 40 + 48,
    },
    [WIN_STATS_NAVIGATION_BUTTONS] =
    {
        .bg = 2,
        .tilemapLeft = 0,
        .tilemapTop = 18,
        .width = 12,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 1 + 60 + 40 + 48 + 96,
    },
    [WIN_STATS_MOVES_TOP] =
    {
        .bg = 2,
        .tilemapLeft = 12,
        .tilemapTop = 2,
        .width = 18,
        .height = 4,
        .paletteNum = 0,
        .baseBlock = 1 + 60 + 40 + 48 + 96 + 24,
    },
    [WIN_STATS_MOVES_DESCRIPTION] =
    {
        .bg = 2,
        .tilemapLeft = 12,
        .tilemapTop = 6,
        .width = 18,
        .height = 4,
        .paletteNum = 0,
        .baseBlock = 1 + 60 + 40 + 48 + 96 + 24 + 72,
    },
    [WIN_STATS_MOVES_BOTTOM] =
    {
        .bg = 2,
        .tilemapLeft = 12,
        .tilemapTop = 10,
        .width = 18,
        .height = 2,
        .paletteNum = 0,
        .baseBlock = 1 + 60 + 40 + 48 + 96 + 24 + 72 + 72,
    },
    [WIN_STATS_ABILITIES] =
    {
        .bg = 2,
        .tilemapLeft = 12,
        .tilemapTop = 12,
        .width = 18,
        .height = 8,
        .paletteNum = 0,
        .baseBlock = 1 + 60 + 40 + 48 + 96 + 24 + 72 + 72 + 36,
    },
    [WIN_STATS_LEFT_BLANK] =
    {
        .bg = 2,
        .tilemapLeft = 0,
        .tilemapTop = 14,
        .width = 12,
        .height = 4,
        .paletteNum = 0,
        .baseBlock = 1 + 60 + 40 + 48 + 96 + 24 + 72 + 72 + 36 + 144,
    },
    DUMMY_WIN_TEMPLATE
};

static const struct BgTemplate sNewEntryInfoScreen_BgTemplate[] =
{
    {
        .bg = 2,
        .charBaseIndex = 2,
        .mapBaseIndex = 14,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0
    },
    {
        .bg = 3,
        .charBaseIndex = 1,
        .mapBaseIndex = 15,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 3,
        .baseTile = 0
    },
};

static const struct WindowTemplate sNewEntryInfoScreen_WindowTemplates[] =
{
    [WIN_INFO] =
    {
        .bg = 2,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 32,
        .height = 20,
        .paletteNum = 0,
        .baseBlock = 1,
    },
    DUMMY_WIN_TEMPLATE
};

//************************************
//*                                  *
//*        MAIN                      *
//*                                  *
//************************************
void CB2_OpenPokedexPlusHGSS(void)
{
    switch (gMain.state)
    {
    case 0:
    default:
        SetVBlankCallback(NULL);
        ResetOtherVideoRegisters(0);
        DmaFillLarge16(3, 0, (u8 *)VRAM, VRAM_SIZE, 0x1000);
        DmaClear32(3, OAM, OAM_SIZE);
        DmaClear16(3, PLTT, PLTT_SIZE);
        gMain.state = 1;
        break;
    case 1:
        ScanlineEffect_Stop();
        ResetTasks();
        ResetSpriteData();
        ResetPaletteFade();
        FreeAllSpritePalettes();
        gReservedSpritePaletteCount = 8;
        ResetAllPicSprites();
        gMain.state++;
        break;
    case 2:
        sPokedexView = AllocZeroed(sizeof(struct PokedexView));
        ResetPokedexView(sPokedexView);
        CreateTask(Task_OpenPokedexMainPage, 0);
        sPokedexView->selectedPokemon = sLastSelectedPokemon;
        sPokedexView->pokeBallRotation = sPokeBallRotation;
        sPokedexView->selectedScreen = AREA_SCREEN;
        sPokedexView->seenCount = GetNationalPokedexCount(FLAG_GET_SEEN);
        sPokedexView->ownCount = GetNationalPokedexCount(FLAG_GET_CAUGHT);
        sPokedexView->initialVOffset = 8;
        gMain.state++;
        break;
    case 3:
        EnableInterrupts(1);
        SetVBlankCallback(VBlankCB_Pokedex);
        SetMainCallback2(CB2_Pokedex);
        FadeOutAndPlayNewMapMusic(MUS_HG_RADIO_VARIETY, 4);
        break;
    }
}

static void ResetPokedexView(struct PokedexView *pokedexView)
{
    u16 i;

    for (i = 0; i < DEX_COUNT; i++)
    {
        pokedexView->pokedexList[i].dexNum = 0xFFFF;
        pokedexView->pokedexList[i].seen = FALSE;
        pokedexView->pokedexList[i].owned = FALSE;
    }
    pokedexView->pokedexList[DEX_COUNT].dexNum = 0;
    pokedexView->pokedexList[DEX_COUNT].seen = FALSE;
    pokedexView->pokedexList[DEX_COUNT].owned = FALSE;
    pokedexView->pokemonListCount = 0;
    pokedexView->selectedPokemon = 0;
    pokedexView->selectedPokemonBackup = 0;
    pokedexView->seenCount = 0;
    pokedexView->ownCount = 0;
    for (i = 0; i < MAX_MONS_ON_SCREEN; i++)
        pokedexView->monSpriteIds[i] = 0xFFFF;
    pokedexView->pokeBallRotationStep = 0;
    pokedexView->pokeBallRotationBackup = 0;
    pokedexView->pokeBallRotation = 0;
    pokedexView->initialVOffset = 0;
    pokedexView->scrollTimer = 0;
    pokedexView->scrollDirection = 0;
    pokedexView->listVOffset = 0;
    pokedexView->listMovingVOffset = 0;
    pokedexView->scrollMonIncrement = 0;
    pokedexView->maxScrollTimer = 0;
    pokedexView->scrollSpeed = 0;
    pokedexView->currentPage = PAGE_MAIN;
    pokedexView->currentPageBackup = PAGE_MAIN;
    pokedexView->selectedScreen = AREA_SCREEN;
    pokedexView->screenSwitchState = 0;
    pokedexView->menuIsOpen = 0;
    pokedexView->menuCursorPos = 0;
    pokedexView->menuY = 0;
}

void ResetPokedexScrollPositions(void)
{
    sLastSelectedPokemon = 0;
    sPokeBallRotation = POKEBALL_ROTATION_TOP;
}

static void VBlankCB_Pokedex(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_Pokedex(void)
{
    RunTasks();
    AnimateSprites();
    ScrollBackground();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void Task_OpenPokedexMainPage(u8 taskId)
{
    sPokedexView->sEvoScreenData.fromEvoPage = FALSE;
    sPokedexView->formSpecies = 0;
    if (LoadPokedexListPage(PAGE_MAIN))
        gTasks[taskId].func = Task_HandlePokedexInput;
}

#define tLoadScreenTaskId data[0]

static void Task_HandlePokedexInput(u8 taskId)
{
    SetGpuReg(REG_OFFSET_BG0VOFS, sPokedexView->menuY);

    if (sPokedexView->menuY)
    {
        sPokedexView->menuY -= 8;
        if (sPokedexView->menuIsOpen == FALSE && sPokedexView->menuY == 8)
        {
            CreateStatBars(&sPokedexView->pokedexList[sPokedexView->selectedPokemon]);
            CreateStatBarsBg();
        }
    }
    else
    {
        if (JOY_NEW(A_BUTTON) && sPokedexView->pokedexList[sPokedexView->selectedPokemon].seen)
        {
            TryDestroyStatBars();
            UpdateSelectedMonSpriteId();
            BeginNormalPaletteFade(~(1 << (gSprites[sPokedexView->selectedMonSpriteId].oam.paletteNum + 16)), 0, 0, 0x10, RGB_BLACK);
            gSprites[sPokedexView->selectedMonSpriteId].callback = SpriteCB_MoveMonForInfoScreen;
            gTasks[taskId].func = Task_OpenInfoScreenAfterMonMovement;
            PlaySE(SE_PIN);
            FreeWindowAndBgBuffers();
        }
        else if (JOY_NEW(B_BUTTON))
        {
            TryDestroyStatBars();
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
            gTasks[taskId].func = Task_ClosePokedex;
            PlaySE(SE_PC_OFF);
        }
        else
        {
            //Handle D-pad
            sPokedexView->selectedPokemon = TryDoPokedexScroll(sPokedexView->selectedPokemon);
            if (sPokedexView->scrollTimer)
                gTasks[taskId].func = Task_WaitForScroll;
            else if (!sPokedexView->scrollTimer && !sPokedexView->scrollSpeed &&sPokedexView->justScrolled)
                CreateStatBars(&sPokedexView->pokedexList[sPokedexView->selectedPokemon]);
        }
    }
}

static void Task_WaitForScroll(u8 taskId)
{
    TryDestroyStatBars();
    if (UpdateDexListScroll(sPokedexView->scrollDirection, sPokedexView->scrollMonIncrement, sPokedexView->maxScrollTimer))
        gTasks[taskId].func = Task_HandlePokedexInput;
}

// Opening the info screen from list view. Pokémon sprite is moving to its new position, wait for it to arrive
static void Task_OpenInfoScreenAfterMonMovement(u8 taskId)
{
    if (gSprites[sPokedexView->selectedMonSpriteId].x == MON_PAGE_X && gSprites[sPokedexView->selectedMonSpriteId].y == MON_PAGE_Y)
    {
        sPokedexView->currentPageBackup = sPokedexView->currentPage;
        gTasks[taskId].tLoadScreenTaskId = LoadInfoScreen(&sPokedexView->pokedexList[sPokedexView->selectedPokemon], sPokedexView->selectedMonSpriteId);
        gTasks[taskId].func = Task_WaitForExitInfoScreen;
    }
}

static void Task_WaitForExitInfoScreen(u8 taskId)
{
    if (gTasks[gTasks[taskId].tLoadScreenTaskId].isActive)
    {
        // While active, handle scroll input
        if (sPokedexView->currentPage == PAGE_INFO && !IsInfoScreenScrolling(gTasks[taskId].tLoadScreenTaskId) && TryDoInfoScreenScroll())
            StartInfoScreenScroll(&sPokedexView->pokedexList[sPokedexView->selectedPokemon], gTasks[taskId].tLoadScreenTaskId);
    }
    else
    {
        // Exiting, back to list view
        sLastSelectedPokemon = sPokedexView->selectedPokemon;
        sPokeBallRotation = sPokedexView->pokeBallRotation;
        gTasks[taskId].func = Task_OpenPokedexMainPage;
    }
}

static void ScrollBackground(void)
{
    ChangeBgX(3, 128, BG_COORD_ADD);
    ChangeBgY(3, 128, BG_COORD_SUB);
}

static void PrintTitleSeenAndOwn(void)
{
    AddTextPrinterParameterized3(WIN_TITLE, FONT_BIG, 0, 0, sTextColors[COLOR_TITLE], 0, gText_SavingPokedex);
    ConvertIntToDecimalStringN(gStringVar1, sPokedexView->ownCount, STR_CONV_MODE_RIGHT_ALIGN, 3);
    AddTextPrinterParameterized3(WIN_SEEN, FONT_BIG, 6, 0, sTextColors[COLOR_TITLE], 0, gStringVar1);
    ConvertIntToDecimalStringN(gStringVar1, sPokedexView->seenCount, STR_CONV_MODE_RIGHT_ALIGN, 3);
    AddTextPrinterParameterized3(WIN_OWN, FONT_BIG, 5, 0, sTextColors[COLOR_TITLE], 0, gStringVar1);
}

static void Task_ClosePokedex(u8 taskId)
{
    u16 music = GetCurrLocationDefaultMusic();

    if (!gPaletteFade.active)
    {
        ClearMonSprites();
        FreeWindowAndBgBuffers();
        DestroyTask(taskId);
        SetMainCallback2(CB2_ReturnToFieldWithOpenMenu);
        FadeOutAndPlayNewMapMusic(music, 8);
        Free(sPokedexView);
    }
}

static void LoadPokedexBgPalette(void)
{
    LoadPalette(sFondoPal, BG_PLTT_ID(0), PLTT_SIZE_4BPP);
}


//************************************
//*                                  *
//*    Main scrolling list screen    *
//*                                  *
//************************************

// For loading main pokedex page or pokedex search results
static bool8 LoadPokedexListPage(u8 page)
{
    switch (gMain.state)
    {
    case 0:
    default:
        if (gPaletteFade.active)
            return 0;
        SetVBlankCallback(NULL);
        sPokedexView->currentPage = page;
        ResetOtherVideoRegisters(0);
        SetGpuReg(REG_OFFSET_BG2VOFS, sPokedexView->initialVOffset);
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sPokedex_BgTemplate, ARRAY_COUNT(sPokedex_BgTemplate));
        SetBgTilemapBuffer(3, AllocZeroed(BG_SCREEN_SIZE));
        SetBgTilemapBuffer(2, AllocZeroed(BG_SCREEN_SIZE));
        SetBgTilemapBuffer(1, AllocZeroed(BG_SCREEN_SIZE));
        SetBgTilemapBuffer(0, AllocZeroed(BG_SCREEN_SIZE));
        DecompressAndLoadBgGfxUsingHeap(3, sTilesFondo, 0x2000, 0, 0);
        CopyToBgTilemapBuffer(1, sFondoLista, 0, 0);
        CopyToBgTilemapBuffer(3, sFondoScrolling, 0, 0);
        ResetPaletteFade();
        LoadPokedexBgPalette();
        //InitWindows(sPokemonList_WindowTemplate);
        DeactivateAllTextPrinters();
        //PutWindowTilemap(WIN_POKEMON_LIST);
        //CopyWindowToVram(WIN_POKEMON_LIST, COPYWIN_FULL);
        //PutWindowTilemap(WIN_TITLE);
        //PutWindowTilemap(WIN_SEEN);
        //PutWindowTilemap(WIN_OWN);
        gMain.state = 1;
        break;
    case 1:
        ResetSpriteData();
        FreeAllSpritePalettes();
        gReservedSpritePaletteCount = 8;
        LoadCompressedSpriteSheet(&sInterfaceSpriteSheet[HGSS_DECAPPED]);
        LoadSpritePalettes(sInterfaceSpritePalette);
        LoadSpritePalettes(sStatBarSpritePal);
        CreateInterfaceSprites(page);
        gMain.state++;
        break;
    case 2:
        gMain.state++;
        break;
    case 3:
        if (page == PAGE_MAIN)
            CreatePokedexList();
        PrintTitleSeenAndOwn();
        CreateMonSpritesAtPos(sPokedexView->selectedPokemon);
        sPokedexView->statBarsSpriteId = 0xFF;  //stat bars
        CreateStatBars(&sPokedexView->pokedexList[sPokedexView->selectedPokemon]); //stat bars
        sPokedexView->statBarsBgSpriteId = 0xFF;  //stat bars background
        CreateStatBarsBg(); //stat bars background
        sPokedexView->menuIsOpen = FALSE;
        sPokedexView->menuY = 0;
        CopyBgTilemapBufferToVram(0);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(3);
        gMain.state++;
        break;
    case 4:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0x10, 0, RGB_BLACK);
        SetVBlankCallback(VBlankCB_Pokedex);
        gMain.state++;
        break;
    case 5:
        SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_ALL | WININ_WIN1_ALL);
        SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_ALL | WINOUT_WINOBJ_BG0 | WINOUT_WINOBJ_BG2 | WINOUT_WINOBJ_BG3 | WINOUT_WINOBJ_OBJ);
        SetGpuReg(REG_OFFSET_WIN0H, 0);
        SetGpuReg(REG_OFFSET_WIN0V, 0);
        SetGpuReg(REG_OFFSET_WIN1H, 0);
        SetGpuReg(REG_OFFSET_WIN1V, 0);
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0 | DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON | DISPCNT_OBJWIN_ON);
        ShowBg(0);
        ShowBg(1);
        ShowBg(2);
        ShowBg(3);
        gMain.state++;
        break;
    case 6:
        if (!gPaletteFade.active)
        {
            gMain.state = 0;
            return TRUE;
        }
        break;
    }
    return FALSE;
}

static void CreatePokedexList(void)
{
    u16 dexNum;
    s16 i;
    s16 r5, r10;

    sPokedexView->pokemonListCount = 0;

    for (i = 0, r5 = 0, r10 = 0; i < DEX_COUNT; i++)
    {
        dexNum = i + 1;
        if (GetSetPokedexFlag(dexNum, FLAG_GET_SEEN))
            r10 = 1;
        if (r10)
        {
            sPokedexView->pokedexList[r5].dexNum = dexNum;
            sPokedexView->pokedexList[r5].seen = GetSetPokedexFlag(dexNum, FLAG_GET_SEEN);
            sPokedexView->pokedexList[r5].owned = GetSetPokedexFlag(dexNum, FLAG_GET_CAUGHT);
            if (sPokedexView->pokedexList[r5].seen)
                sPokedexView->pokemonListCount = r5 + 1;
            r5++;
        }
    }

    for (i = sPokedexView->pokemonListCount; i < DEX_COUNT; i++)
    {
        sPokedexView->pokedexList[i].dexNum = 0xFFFF;
        sPokedexView->pokedexList[i].seen = FALSE;
        sPokedexView->pokedexList[i].owned = FALSE;
    }
}

static void PrintMonDexNum(u8 windowId, u8 fontId, const u8 *str, u8 left, u8 top)
{
    static const u8 color[3] = { TEXT_COLOR_TRANSPARENT, TEXT_DYNAMIC_COLOR_6, TEXT_COLOR_LIGHT_GRAY };
    AddTextPrinterParameterized4(windowId, fontId, left * 8, (top * 8) + 1, 0, 0, color, TEXT_SKIP_DRAW, str);
}

static void PrintMonName(u8 windowId, u8 fontId, const u8 *str, u8 left, u8 top)
{
    static const u8 color[3] = { TEXT_COLOR_TRANSPARENT, TEXT_DYNAMIC_COLOR_6, TEXT_COLOR_LIGHT_GRAY };
    AddTextPrinterParameterized4(windowId, FONT_NORMAL, left * 8, (top * 8) + 1, 0, 0, color, TEXT_SKIP_DRAW, str);
}

#define MON_LIST_X 2

static void CreateMonListEntry(u8 position, u16 b)
{
    s16 entryNum;
    u16 i;
    u16 vOffset;

    switch (position)
    {
    case 0: // Initial
    default:
        entryNum = b - 5;
        for (i = 0; i <= 10; i++)
        {
            if (entryNum < 0 || entryNum >= DEX_COUNT || sPokedexView->pokedexList[entryNum].dexNum == 0xFFFF)
            {
                ClearMonListEntry(MON_LIST_X, i * 2);
            }
            else
            {
                ClearMonListEntry(MON_LIST_X, i * 2);
                if (sPokedexView->pokedexList[entryNum].seen)
                {
                    CreateMonDexNum(entryNum, MON_LIST_X+1, i * 2);
                    CreateCaughtBall(sPokedexView->pokedexList[entryNum].owned, MON_LIST_X, i * 2);
                    CreateMonName(sPokedexView->pokedexList[entryNum].dexNum, MON_LIST_X + 5, i * 2);
                }
                else
                {
                    CreateMonDexNum(entryNum, MON_LIST_X+1, i * 2);
                    CreateCaughtBall(FALSE, MON_LIST_X, i * 2);
                    CreateMonName(0, MON_LIST_X + 5, i * 2);
                }
            }
            entryNum++;
        }
        break;
    case 1: // Up
        entryNum = b - 5;
        if (entryNum < 0 || entryNum >= DEX_COUNT || sPokedexView->pokedexList[entryNum].dexNum == 0xFFFF)
        {
            ClearMonListEntry(MON_LIST_X, sPokedexView->listVOffset * 2);
        }
        else
        {
            ClearMonListEntry(MON_LIST_X, sPokedexView->listVOffset * 2);
            if (sPokedexView->pokedexList[entryNum].seen)
            {
                CreateMonDexNum(entryNum, MON_LIST_X+1, sPokedexView->listVOffset * 2);
                CreateCaughtBall(sPokedexView->pokedexList[entryNum].owned, MON_LIST_X, sPokedexView->listVOffset * 2);
                CreateMonName(sPokedexView->pokedexList[entryNum].dexNum, MON_LIST_X + 5, sPokedexView->listVOffset * 2);
            }
            else
            {
                CreateMonDexNum(entryNum, MON_LIST_X+1, sPokedexView->listVOffset * 2);
                CreateCaughtBall(FALSE, MON_LIST_X, sPokedexView->listVOffset * 2);
                CreateMonName(0, MON_LIST_X + 5, sPokedexView->listVOffset * 2);
            }
        }
        break;
    case 2: // Down
        entryNum = b + 5;
        vOffset = sPokedexView->listVOffset + 10;
        if (vOffset >= LIST_SCROLL_STEP)
            vOffset -= LIST_SCROLL_STEP;
        if (entryNum < 0 || entryNum >= DEX_COUNT || sPokedexView->pokedexList[entryNum].dexNum == 0xFFFF)
            ClearMonListEntry(MON_LIST_X, vOffset * 2);
        else
        {
            ClearMonListEntry(MON_LIST_X, vOffset * 2);
            if (sPokedexView->pokedexList[entryNum].seen)
            {
                CreateMonDexNum(entryNum, MON_LIST_X+1, vOffset * 2);
                CreateCaughtBall(sPokedexView->pokedexList[entryNum].owned, MON_LIST_X, vOffset * 2);
                CreateMonName(sPokedexView->pokedexList[entryNum].dexNum, MON_LIST_X + 5, vOffset * 2);
            }
            else
            {
                CreateMonDexNum(entryNum, MON_LIST_X+1, vOffset * 2);
                CreateCaughtBall(FALSE, MON_LIST_X, vOffset * 2);
                CreateMonName(0, MON_LIST_X + 5, vOffset * 2);
            }
        }
        break;
    }
    CopyWindowToVram(0, COPYWIN_GFX);
}

static void CreateMonDexNum(u16 entryNum, u8 left, u8 top)
{
    u8 text[5];
    u16 dexNum, offset = 0;

    dexNum = sPokedexView->pokedexList[entryNum].dexNum;
    memcpy(text, sText_No0000, ARRAY_COUNT(sText_No0000));
    if (DEX_COUNT > 999)
    {
        text[0] = CHAR_0 + dexNum / 1000;
        offset++;
    }
    text[offset++] = CHAR_0 + (dexNum % 1000) / 100;
    text[offset++] = CHAR_0 + ((dexNum % 1000) % 100) / 10;
    text[offset++] = CHAR_0 + ((dexNum % 1000) % 100) % 10;
    text[offset++] = EOS;
    PrintMonDexNum(0, FONT_NARROW, text, left, top);
}

u8* ConvertMonHeightToString(u32 height)
{
    return ConvertMonHeightToMetricString(height);
}

static u8* ConvertMeasurementToMetricString(u32 num, u32* index)
{
    u8* string = Alloc(WEIGHT_HEIGHT_STR_MEM);
    bool32 outputted = FALSE;
    u32 result;

    result = num / 1000;
    if (result == 0)
    {
        string[(*index)++] = CHAR_SPACER;
        outputted = FALSE;
    }
    else
    {
        string[(*index)++] = CHAR_0 + result;
        outputted = TRUE;
    }

    result = (num % 1000) / 100;
    if (result == 0 && !outputted)
    {
        string[(*index)++] = CHAR_SPACER;
        outputted = FALSE;
    }
    else
    {
        string[(*index)++] = CHAR_0 + result;
        outputted = TRUE;
    }

    string[(*index)++] = CHAR_0 + ((num % 1000) % 100) / 10;
    string[(*index)++] = CHAR_DEC_SEPARATOR;
    string[(*index)++] = CHAR_0 + ((num % 1000) % 100) % 10;
    string[(*index)++] = CHAR_SPACE;

    return string;
}

void ResetPokedex(void)
{
    u16 i;

    sLastSelectedPokemon = 0;
    sPokeBallRotation = POKEBALL_ROTATION_TOP;
    gSaveBlock2Ptr->pokedex.order = 0;
    for (i = 0; i < NUM_DEX_FLAG_BYTES; i++)
    {
        gSaveBlock1Ptr->dexCaught[i] = 0;
        gSaveBlock1Ptr->dexSeen[i] = 0;
    }
}

u16 GetNationalPokedexCount(u8 caseID)
{
    u16 count = 0;
    u16 i;
    for (i = 0; i < DEX_COUNT; i++)
    {
        switch (caseID)
        {
        case FLAG_GET_SEEN:
            if (GetSetPokedexFlag(i + 1, FLAG_GET_SEEN))
                count++;
            break;
        case FLAG_GET_CAUGHT:
            if (GetSetPokedexFlag(i + 1, FLAG_GET_CAUGHT))
                count++;
            break;
        }
    }
    return count;
}

s8 GetSetPokedexFlag(u16 nationalDexNo, u8 caseID)
{
    u32 index, bit, mask;
    s8 retVal = 0;

    nationalDexNo--;
    index = nationalDexNo / 8;
    bit = nationalDexNo % 8;
    mask = 1 << bit;

    switch (caseID)
    {
    case FLAG_GET_SEEN:
        retVal = ((gSaveBlock1Ptr->dexSeen[index] & mask) != 0);
        break;
    case FLAG_GET_CAUGHT:
         retVal = ((gSaveBlock1Ptr->dexCaught[index] & mask) != 0);
        break;
    case FLAG_SET_SEEN:
        gSaveBlock1Ptr->dexSeen[index] |= mask;
        break;
    case FLAG_SET_CAUGHT:
        gSaveBlock1Ptr->dexCaught[index] |= mask;
        break;
    }

    return retVal;
}

#define tState         data[0]
#define tSpecies        data[1]
#define tPalTimer      data[2]
#define tMonSpriteId   data[3]
#define tIsShiny       data[13]
#define tPersonalityLo data[14]
#define tPersonalityHi data[15]

u8 DisplayCaughtMonDexPage(u16 species, bool32 isShiny, u32 personality)
{
    u8 taskId = 0;

    taskId = CreateTask(Task_DisplayCaughtMonDexPageHGSS, 0);
    gTasks[taskId].tState = 0;
    gTasks[taskId].tSpecies = species;
    gTasks[taskId].tIsShiny = isShiny;
    gTasks[taskId].tPersonalityLo = personality;
    gTasks[taskId].tPersonalityHi = personality >> 16;
    return taskId;
}

#undef tState
#undef tSpecies
#undef tPalTimer
#undef tMonSpriteId
#undef tOtIdLo
#undef tOtIdHi
#undef tPersonalityLo
#undef tPersonalityHi

bool16 HasAllMons(void)
{
    u32 i, j;

    for (i = 1; i < DEX_COUNT + 1; i++)
    {
        j = NationalPokedexNumToSpecies(i);
        if (!(gSpeciesInfo[j].isMythical && !gSpeciesInfo[j].dexForceRequired) && !GetSetPokedexFlag(j, FLAG_GET_CAUGHT))
            return FALSE;
    }
    return TRUE;
}

static void CreateCaughtBall(bool16 owned, u8 x, u8 y)
{
    if (owned)
        BlitBitmapToWindow(0, sCaughtBall_Gfx, x * 6, (y * 8) + 1, 8, 16);
    else
        FillWindowPixelRect(0, PIXEL_FILL(0), x * 6, y * 8, 8, 16);
}

static u8 CreateMonName(u16 num, u8 left, u8 top)
{
    const u8 *str;

    num = NationalPokedexNumToSpecies(num);
    if (num)
        str = GetSpeciesName(num);
    else
        str = sText_TenDashes;
    PrintMonName(0, FONT_NARROW, str, left, top);
    return StringLength(str);
}

static void ClearMonListEntry(u8 x, u8 y)
{
    FillWindowPixelRect(0, PIXEL_FILL(0), x * 6, y * 8, 0x60, 16);
}

static void CreateMonSpritesAtPos(u16 selectedMon)
{
    u8 i;
    u16 dexNum;
    u8 spriteId;

    gPaletteFade.bufferTransferDisabled = TRUE;

    for (i = 0; i < MAX_MONS_ON_SCREEN; i++)
        sPokedexView->monSpriteIds[i] = 0xFFFF;
    sPokedexView->selectedMonSpriteId = 0xFFFF;

    // Create top mon sprite
    dexNum = GetPokemonSpriteToDisplay(selectedMon - 1);
    if (dexNum != 0xFFFF)
    {
        spriteId = CreatePokedexMonSprite(dexNum, SCROLLING_MON_X, 0x50);
        gSprites[spriteId].callback = SpriteCB_PokedexListMonSprite;
        gSprites[spriteId].data[5] = -32;
    }

    // Create mid mon sprite
    dexNum = GetPokemonSpriteToDisplay(selectedMon);
    if (dexNum != 0xFFFF)
    {
        spriteId = CreatePokedexMonSprite(dexNum, SCROLLING_MON_X, 0x50);
        gSprites[spriteId].callback = SpriteCB_PokedexListMonSprite;
        gSprites[spriteId].data[5] = 0;
    }

    // Create bottom mon sprite
    dexNum = GetPokemonSpriteToDisplay(selectedMon + 1);
    if (dexNum != 0xFFFF)
    {
        spriteId = CreatePokedexMonSprite(dexNum, SCROLLING_MON_X, 0x50);
        gSprites[spriteId].callback = SpriteCB_PokedexListMonSprite;
        gSprites[spriteId].data[5] = 32;
    }

    CreateMonListEntry(0, selectedMon);
    SetGpuReg(REG_OFFSET_BG2VOFS, sPokedexView->initialVOffset);

    sPokedexView->listVOffset = 0;
    sPokedexView->listMovingVOffset = 0;

    gPaletteFade.bufferTransferDisabled = FALSE;
}

static bool8 UpdateDexListScroll(u8 direction, u8 monMoveIncrement, u8 scrollTimerMax)
{
    u16 i;
    u8 step;

    if (sPokedexView->scrollTimer)
    {
        sPokedexView->scrollTimer--;
        switch (direction)
        {
        case 1: // Up
            for (i = 0; i < MAX_MONS_ON_SCREEN; i++)
            {
                if (sPokedexView->monSpriteIds[i] != 0xFFFF)
                    gSprites[sPokedexView->monSpriteIds[i]].data[5] += monMoveIncrement;
            }
            step = LIST_SCROLL_STEP * (scrollTimerMax - sPokedexView->scrollTimer) / scrollTimerMax;
            SetGpuReg(REG_OFFSET_BG2VOFS, sPokedexView->initialVOffset + sPokedexView->listMovingVOffset * LIST_SCROLL_STEP - step);
            sPokedexView->pokeBallRotation -= sPokedexView->pokeBallRotationStep;
            break;
        case 2: // Down
            for (i = 0; i < MAX_MONS_ON_SCREEN; i++)
            {
                if (sPokedexView->monSpriteIds[i] != 0xFFFF)
                    gSprites[sPokedexView->monSpriteIds[i]].data[5] -= monMoveIncrement;
            }
            step = LIST_SCROLL_STEP * (scrollTimerMax - sPokedexView->scrollTimer) / scrollTimerMax;
            SetGpuReg(REG_OFFSET_BG2VOFS, sPokedexView->initialVOffset + sPokedexView->listMovingVOffset * LIST_SCROLL_STEP + step);
            sPokedexView->pokeBallRotation += sPokedexView->pokeBallRotationStep;
            break;
        }
        return FALSE;
    }
    else
    {
        SetGpuReg(REG_OFFSET_BG2VOFS, sPokedexView->initialVOffset + sPokedexView->listVOffset * LIST_SCROLL_STEP);
        return TRUE;
    }
}

static void CreateScrollingPokemonSprite(u8 direction, u16 selectedMon)
{
    u16 dexNum;
    u8 spriteId;

    sPokedexView->listMovingVOffset = sPokedexView->listVOffset;
    switch (direction)
    {
    case 1: // up
        dexNum = GetPokemonSpriteToDisplay(selectedMon - 1);
        if (dexNum != 0xFFFF)
        {
            spriteId = CreatePokedexMonSprite(dexNum, SCROLLING_MON_X, 0x50);
            gSprites[spriteId].callback = SpriteCB_PokedexListMonSprite;
            gSprites[spriteId].data[5] = -64;
        }
        if (sPokedexView->listVOffset > 0)
            sPokedexView->listVOffset--;
        else
            sPokedexView->listVOffset = LIST_SCROLL_STEP - 1;
        break;
    case 2: // down
        dexNum = GetPokemonSpriteToDisplay(selectedMon + 1);
        if (dexNum != 0xFFFF)
        {
            spriteId = CreatePokedexMonSprite(dexNum, SCROLLING_MON_X, 0x50);
            gSprites[spriteId].callback = SpriteCB_PokedexListMonSprite;
            gSprites[spriteId].data[5] = 64;
        }
        if (sPokedexView->listVOffset < LIST_SCROLL_STEP - 1)
            sPokedexView->listVOffset++;
        else
            sPokedexView->listVOffset = 0;
        break;
    }
}

static u16 TryDoPokedexScroll(u16 selectedMon)
{
    u8 scrollTimer;
    u8 scrollMonIncrement;
    u8 i;
    u16 startingPos;
    u8 scrollDir = 0;

    if (JOY_HELD(DPAD_UP) && (selectedMon > 0))
    {
        scrollDir = 1;
        selectedMon = GetNextPosition(1, selectedMon, 0, sPokedexView->pokemonListCount - 1);
        CreateScrollingPokemonSprite(1, selectedMon);
        CreateMonListEntry(1, selectedMon);
        sPokedexView->justScrolled = TRUE;
        PlaySE(SE_DEX_SCROLL);
    }
    else if (JOY_HELD(DPAD_DOWN) && (selectedMon < sPokedexView->pokemonListCount - 1))
    {
        scrollDir = 2;
        selectedMon = GetNextPosition(0, selectedMon, 0, sPokedexView->pokemonListCount - 1);
        CreateScrollingPokemonSprite(2, selectedMon);
        CreateMonListEntry(2, selectedMon);
        sPokedexView->justScrolled = TRUE;
        PlaySE(SE_DEX_SCROLL);
    }
    else if (JOY_NEW(DPAD_LEFT) && (selectedMon > 0))
    {
        startingPos = selectedMon;

        for (i = 0; i < 7; i++)
            selectedMon = GetNextPosition(1, selectedMon, 0, sPokedexView->pokemonListCount - 1);
        sPokedexView->pokeBallRotation += 16 * (selectedMon - startingPos);
        ClearMonSprites();
        CreateMonSpritesAtPos(selectedMon);
        sPokedexView->justScrolled = TRUE;
        PlaySE(SE_DEX_PAGE);
    }
    else if (JOY_NEW(DPAD_RIGHT) && (selectedMon < sPokedexView->pokemonListCount - 1))
    {
        startingPos = selectedMon;
        for (i = 0; i < 7; i++)
            selectedMon = GetNextPosition(0, selectedMon, 0, sPokedexView->pokemonListCount - 1);
        sPokedexView->pokeBallRotation += 16 * (selectedMon - startingPos);
        ClearMonSprites();
        CreateMonSpritesAtPos(selectedMon);
        sPokedexView->justScrolled = TRUE;
        PlaySE(SE_DEX_PAGE);
    }

    if (scrollDir == 0)
    {
        // Left/right input just snaps up/down, no scrolling
        sPokedexView->scrollSpeed = 0;
        return selectedMon;
    }

    scrollMonIncrement = sScrollMonIncrements[sPokedexView->scrollSpeed / 4];
    scrollTimer = sScrollTimers[sPokedexView->scrollSpeed / 4];
    sPokedexView->scrollTimer = scrollTimer;
    sPokedexView->maxScrollTimer = scrollTimer;
    sPokedexView->scrollMonIncrement = scrollMonIncrement;
    sPokedexView->scrollDirection = scrollDir;
    sPokedexView->pokeBallRotationStep = scrollMonIncrement / 2;
    UpdateDexListScroll(sPokedexView->scrollDirection, sPokedexView->scrollMonIncrement, sPokedexView->maxScrollTimer);
    if (sPokedexView->scrollSpeed < 12)
        sPokedexView->scrollSpeed++;
    return selectedMon;
}

static void UpdateSelectedMonSpriteId(void)
{
    u16 i;

    for (i = 0; i < MAX_MONS_ON_SCREEN; i++)
    {
        u16 spriteId = sPokedexView->monSpriteIds[i];

        if (gSprites[spriteId].x2 == 0 && gSprites[spriteId].y2 == 0 && spriteId != 0xFFFF)
            sPokedexView->selectedMonSpriteId = spriteId;
    }
}

static bool8 TryDoInfoScreenScroll(void)
{
    u16 nextPokemon;
    u16 selectedPokemon = sPokedexView->selectedPokemon;

    if (sPokedexView->sEvoScreenData.fromEvoPage)
        return FALSE;

    if (JOY_NEW(DPAD_UP) && selectedPokemon)
    {
        nextPokemon = selectedPokemon;
        while (nextPokemon != 0)
        {
            nextPokemon = GetNextPosition(1, nextPokemon, 0, sPokedexView->pokemonListCount - 1);

            if (sPokedexView->pokedexList[nextPokemon].seen)
            {
                selectedPokemon = nextPokemon;
                break;
            }
        }

        if (sPokedexView->selectedPokemon == selectedPokemon)
            return FALSE;
        else
        {
            sPokedexView->selectedPokemon = selectedPokemon;
            sPokedexView->pokeBallRotation -= 16;
            return TRUE;
        }
    }
    else if (JOY_NEW(DPAD_DOWN) && selectedPokemon < sPokedexView->pokemonListCount - 1)
    {
        nextPokemon = selectedPokemon;
        while (nextPokemon < sPokedexView->pokemonListCount - 1)
        {
            nextPokemon = GetNextPosition(0, nextPokemon, 0, sPokedexView->pokemonListCount - 1);

            if (sPokedexView->pokedexList[nextPokemon].seen)
            {
                selectedPokemon = nextPokemon;
                break;
            }
        }

        if (sPokedexView->selectedPokemon == selectedPokemon)
            return FALSE;
        else
        {
            sPokedexView->selectedPokemon = selectedPokemon;
            sPokedexView->pokeBallRotation += 16;
            return TRUE;
        }
    }
    return FALSE;
}

static u8 ClearMonSprites(void)
{
    u16 i;

    for (i = 0; i < MAX_MONS_ON_SCREEN; i++)
    {
        if (sPokedexView->monSpriteIds[i] != 0xFFFF)
        {
            FreeAndDestroyMonPicSprite(sPokedexView->monSpriteIds[i]);
            sPokedexView->monSpriteIds[i] = 0xFFFF;
        }
    }
    return FALSE;
}

static u16 GetPokemonSpriteToDisplay(u16 species)
{
    if (species >= DEX_COUNT || sPokedexView->pokedexList[species].dexNum == 0xFFFF)
        return 0xFFFF;
    else
        return sPokedexView->pokedexList[species].dexNum;
}

static u32 CreatePokedexMonSprite(u16 num, s16 x, s16 y)
{
    u8 i;
    for (i = 0; i < MAX_MONS_ON_SCREEN; i++)
    {
        if (sPokedexView->monSpriteIds[i] == 0xFFFF)
        {
            u8 spriteId = CreateMonSpriteFromNationalDexNumberHGSS(num, x, y, i);
            gSprites[spriteId].oam.affineMode = ST_OAM_AFFINE_NORMAL;
            gSprites[spriteId].oam.priority = 3;
            gSprites[spriteId].data[0] = 0;
            gSprites[spriteId].data[1] = i;
            gSprites[spriteId].data[2] = NationalPokedexNumToSpecies(num);
            sPokedexView->monSpriteIds[i] = spriteId;
            if (!GetSetPokedexFlag(num, FLAG_GET_CAUGHT))
            {
                SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT2_ALL | BLDCNT_EFFECT_BLEND);
                SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(7, 11));
                gSprites[spriteId].oam.objMode = ST_OAM_OBJ_BLEND;
                if (!GetSetPokedexFlag(num, FLAG_GET_SEEN))
                {
                    FillPalette(RGB_BLACK, OBJ_PLTT_ID(i), PLTT_SIZE_4BPP);
                }
            }
            return spriteId;
        }
    }
    return 0xFFFF;
}

#define sIsDownArrow data[1]

static void CreateInterfaceSprites(u8 page)
{
    u8 spriteId;

    // Scroll arrows
    spriteId = CreateSprite(&sScrollArrowSpriteTemplate, 10, 4, 0);
    gSprites[spriteId].sIsDownArrow = FALSE;
    spriteId = CreateSprite(&sScrollArrowSpriteTemplate, 10, 156, 0);
    gSprites[spriteId].sIsDownArrow = TRUE;
    gSprites[spriteId].vFlip = TRUE;

    CreateSprite(&sScrollBarSpriteTemplate, 6, 20, 0);

    if (page == PAGE_MAIN)
    {
        spriteId = CreateSprite(&sDexListStartMenuCursorSpriteTemplate, 136, 96, 1);
        gSprites[spriteId].invisible = TRUE;
    }
}

static void SpriteCB_MoveMonForInfoScreen(struct Sprite *sprite)
{
    sprite->oam.priority = 0;
    sprite->oam.affineMode = ST_OAM_AFFINE_OFF;
    sprite->x2 = 0;
    sprite->y2 = 0;
    if (sprite->x != MON_PAGE_X || sprite->y != MON_PAGE_Y)
    {
        if (sprite->x > 48)
            sprite->x -= 4;
        if (sprite->x < 48)
            sprite->x = 48;

        if (sprite->y > 56)
            sprite->y -= 4;
        if (sprite->y < 56)
            sprite->y = 56;
    }
}

static void SpriteCB_PokedexListMonSprite(struct Sprite *sprite)
{
    u8 monId = sprite->data[1];

    if (sPokedexView->currentPage != PAGE_MAIN)
    {
        FreeAndDestroyMonPicSprite(sPokedexView->monSpriteIds[monId]);
        sPokedexView->monSpriteIds[monId] = 0xFFFF;
    }
    else
    {
        u32 var;
        sprite->y2 = gSineTable[(u8)sprite->data[5]] * 76 / 256;
        var = SAFE_DIV(0x10000, gSineTable[sprite->data[5] + 64]);
        if (var > 0xFFFF)
            var = 0xFFFF;
        SetOamMatrix(sprite->data[1] + 1, 0x100, 0, 0, var);
        sprite->oam.matrixNum = monId + 1;

        if (sprite->data[5] > -64 && sprite->data[5] < 64)
        {
            sprite->invisible = FALSE;
            sprite->data[0] = 1;
        }
        else
        {
            sprite->invisible = TRUE;
        }

        if ((sprite->data[5] <= -64 || sprite->data[5] >= 64) && sprite->data[0] != 0)
        {
            FreeAndDestroyMonPicSprite(sPokedexView->monSpriteIds[monId]);
            sPokedexView->monSpriteIds[monId] = 0xFFFF;
        }
    }
}

static void SpriteCB_Scrollbar(struct Sprite *sprite)
{
    if (sPokedexView->currentPage != PAGE_MAIN)
        DestroySprite(sprite);
    else
        sprite->y2 = sPokedexView->selectedPokemon * 120 / (sPokedexView->pokemonListCount - 1);
}

static void SpriteCB_ScrollArrow(struct Sprite *sprite)
{
    if (sPokedexView->currentPage != PAGE_MAIN)
    {
        DestroySprite(sprite);
    }
    else
    {
        u8 r0;

        if (sprite->sIsDownArrow)
        {
            if (sPokedexView->selectedPokemon == sPokedexView->pokemonListCount - 1)
                sprite->invisible = TRUE;
            else
                sprite->invisible = FALSE;
            r0 = sprite->data[2];
        }
        else
        {
            if (sPokedexView->selectedPokemon == 0)
                sprite->invisible = TRUE;
            else
                sprite->invisible = FALSE;
            r0 = sprite->data[2] - 128;
        }
        sprite->y2 = gSineTable[r0] / 64;
        sprite->data[2] = sprite->data[2] + 8;
        if (sPokedexView->menuIsOpen == FALSE && sPokedexView->menuY == 0 && sprite->invisible == FALSE)
            sprite->invisible = FALSE;
        else
            sprite->invisible = TRUE;
    }
}

static void SpriteCB_DexListInterfaceText(struct Sprite *sprite)
{
    if (sPokedexView->currentPage != PAGE_MAIN)
        DestroySprite(sprite);
}

static void SpriteCB_RotatingPokeBall(struct Sprite *sprite)
{
    if (sPokedexView->currentPage != PAGE_MAIN)
    {
        DestroySprite(sprite);
    }
    else
    {
        u8 val;
        s16 r3;
        s16 r0;

        val = sPokedexView->pokeBallRotation + sprite->data[1];
        r3 = gSineTable[val];
        r0 = gSineTable[val + 64];
        SetOamMatrix(sprite->data[0], r0, r3, -r3, r0);

        val = sPokedexView->pokeBallRotation + (sprite->data[1] + 64);
        r3 = gSineTable[val];
        r0 = gSineTable[val + 64];
        sprite->x2 = r0 * 40 / 256;
        sprite->y2 = r3 * 40 / 256;
    }
}

static void SpriteCB_DexListStartMenuCursor(struct Sprite *sprite)
{
    if (sPokedexView->currentPage != PAGE_MAIN)
    {
        DestroySprite(sprite);
    }
    else
    {
        u16 r1 = sPokedexView->currentPage == PAGE_MAIN ? 80 : 96;

        if (sPokedexView->menuIsOpen && sPokedexView->menuY == r1)
        {
            sprite->invisible = FALSE;
            sprite->y2 = sPokedexView->menuCursorPos * 16;
            sprite->x2 = gSineTable[(u8)sprite->data[2]] / 64;
            sprite->data[2] += 8;
        }
        else
        {
            sprite->invisible = TRUE;
        }
    }
}

//************************************
//*                                  *
//*        STATS BARS                *
//*                                  *
//************************************

//Stat bars on main screen, code by DizzyEgg, idea by Jaizu
#define PIXEL_COORDS_TO_OFFSET(x, y)(			\
/*Add tiles by X*/								\
((y / 8) * 32 * 8)								\
/*Add tiles by X*/								\
+ ((x / 8) * 32)								\
/*Add pixels by Y*/								\
+ ((((y) - ((y / 8) * 8))) * 4)				    \
/*Add pixels by X*/								\
+ ((((x) - ((x / 8) * 8)) / 2)))

static inline void WritePixel(u8 *dst, u32 x, u32 y, u32 value)
{
    if (x & 1)
    {
        dst[PIXEL_COORDS_TO_OFFSET(x, y)] &= ~0xF0;
        dst[PIXEL_COORDS_TO_OFFSET(x, y)] |= (value << 4);
    }
    else
    {
        dst[PIXEL_COORDS_TO_OFFSET(x, y)] &= ~0xF;
        dst[PIXEL_COORDS_TO_OFFSET(x, y)] |= (value);
    }
}
#define STAT_BAR_X_OFFSET 10
static void CreateStatBar(u8 *dst, u32 y, u32 width)
{
    u32 i, color;

    switch (width)
    {
    case 0 ... 5:
        color = COLOR_WORST;
        break;
    case 6 ... 15:
        color = COLOR_BAD;
        break;
    case 16 ... 25:
        color = COLOR_AVERAGE;
        break;
    case 26 ... 31:
        color = COLOR_GOOD;
        break;
    case 32 ... 37:
        color = COLOR_VERY_GOOD;
        break;
    default:
        color = COLOR_BEST;
        break;
    }

    // white pixes left side
    WritePixel(dst, STAT_BAR_X_OFFSET, y + 0, COLOR_ID_BAR_WHITE);
    WritePixel(dst, STAT_BAR_X_OFFSET, y + 1, COLOR_ID_BAR_WHITE);
    WritePixel(dst, STAT_BAR_X_OFFSET, y + 2, COLOR_ID_BAR_WHITE);
    WritePixel(dst, STAT_BAR_X_OFFSET, y + 3, COLOR_ID_BAR_WHITE);
    WritePixel(dst, STAT_BAR_X_OFFSET, y + 4, COLOR_ID_BAR_WHITE);

    // white pixels right side
    WritePixel(dst, STAT_BAR_X_OFFSET + width - 1, y + 0, COLOR_ID_BAR_WHITE);
    WritePixel(dst, STAT_BAR_X_OFFSET + width - 1, y + 1, COLOR_ID_BAR_WHITE);
    WritePixel(dst, STAT_BAR_X_OFFSET + width - 1, y + 2, COLOR_ID_BAR_WHITE);
    WritePixel(dst, STAT_BAR_X_OFFSET + width - 1, y + 3, COLOR_ID_BAR_WHITE);
    WritePixel(dst, STAT_BAR_X_OFFSET + width - 1, y + 4, COLOR_ID_BAR_WHITE);

    // Fill
    for (i = 1; i < width - 1; i++)
    {
        WritePixel(dst, STAT_BAR_X_OFFSET + i, y + 0, COLOR_ID_BAR_WHITE);
        WritePixel(dst, STAT_BAR_X_OFFSET + i, y + 1, COLOR_ID_FILL_SHADOW + color * 2);
        WritePixel(dst, STAT_BAR_X_OFFSET + i, y + 2, COLOR_ID_FILL + color * 2);
        WritePixel(dst, STAT_BAR_X_OFFSET + i, y + 3, COLOR_ID_FILL + color * 2);
        WritePixel(dst, STAT_BAR_X_OFFSET + i, y + 4, COLOR_ID_BAR_WHITE);
    }
}
static const u8 sBaseStatOffsets[] =
{
    offsetof(struct SpeciesInfo, baseHP),
    offsetof(struct SpeciesInfo, baseAttack),
    offsetof(struct SpeciesInfo, baseDefense),
    offsetof(struct SpeciesInfo, baseSpAttack),
    offsetof(struct SpeciesInfo, baseSpDefense),
    offsetof(struct SpeciesInfo, baseSpeed),
};
static void TryDestroyStatBars(void)
{
    if (sPokedexView->statBarsSpriteId != 0xFF)
    {
        FreeSpriteTilesByTag(TAG_STAT_BAR);
        //FreeSpriteOamMatrix(&gSprites[sPokedexView->statBarsSpriteId]);
        DestroySprite(&gSprites[sPokedexView->statBarsSpriteId]);
        sPokedexView->statBarsSpriteId = 0xFF;
    }
}
static void TryDestroyStatBarsBg(void)
{
    if (sPokedexView->statBarsBgSpriteId != 0xFF)
    {
        FreeSpriteTilesByTag(TAG_STAT_BAR_BG);
        //FreeSpriteOamMatrix(&gSprites[sPokedexView->statBarsBgSpriteId]);
        DestroySprite(&gSprites[sPokedexView->statBarsBgSpriteId]);
        sPokedexView->statBarsBgSpriteId = 0xFF;
    }
}
static void CreateStatBars(struct PokedexListItem *dexMon)
{
    u8 offset_x = 184; //Moves the complete stat box left/right
    u8 offset_y = 16; //Moves the complete stat box up/down
    TryDestroyStatBars();

    sPokedexView->justScrolled = FALSE;

    if (dexMon->owned) // Show filed bars
    {
        u8 i;
        u32 width, statValue;
        u8 *gfx = Alloc(64 * 64);
        static const u8 sBarsYOffset[] = {3, 13, 23, 33, 43, 53};
        struct SpriteSheet sheet = {gfx, 64 * 64, TAG_STAT_BAR};
        u32 species = NationalPokedexNumToSpecies(dexMon->dexNum);

        memcpy(gfx, sStatBarsGfx, sizeof(sStatBarsGfx));
        for (i = 0; i < NUM_STATS; i++)
        {
            statValue = *((u8*)(&gSpeciesInfo[species]) + sBaseStatOffsets[i]);
            if (statValue <= 100)
            {
                width = statValue / 3;
                if (width >= 33)
                    width -= 1;
            }
            else
                width = (100 / 3) + ((statValue - 100) / 14);

            if (width > 39) // Max pixels
                width = 39;
            if (width < 3)
                width = 3;

            CreateStatBar(gfx, sBarsYOffset[i], width);
        }

        LoadSpriteSheet(&sheet);
        Free(gfx);
    }
    else if (dexMon->seen) // Just HP/ATK/DEF
    {
        static const struct SpriteSheet sheet = {sStatBarsGfx, 64 * 64, TAG_STAT_BAR};

        LoadSpriteSheet(&sheet);
    }
    else // neither seen nor owned
    {
        return;
    }
    sPokedexView->statBarsSpriteId = CreateSprite(&sStatBarSpriteTemplate, 36+offset_x, 107+offset_y, 10);
}
static void CreateStatBarsBg(void) //stat bars background text
{
    static const struct SpriteSheet sheetStatBarsBg = {sStatBarsGfx, 64 * 64, TAG_STAT_BAR_BG};
    u8 offset_x = 184; //Moves the complete stat box left/right
    u8 offset_y = 16; //Moves the complete stat box up/down

    TryDestroyStatBarsBg();

    LoadSpriteSheet(&sheetStatBarsBg);
    sPokedexView->statBarsBgSpriteId = CreateSprite(&sStatBarBgSpriteTemplate, 36+offset_x, 107+offset_y, 0);
}
// Hack to destroy sprites when a pokemon data is being loaded in
static bool32 IsMonInfoBeingLoaded(void)
{
    return (gSprites[sPokedexView->selectedMonSpriteId].callback == SpriteCB_MoveMonForInfoScreen);
}
static void SpriteCB_StatBars(struct Sprite *sprite)
{
    if (IsMonInfoBeingLoaded())
        sprite->invisible = TRUE;
    if (sPokedexView->currentPage != PAGE_MAIN)
    {
        FreeSpriteTilesByTag(TAG_STAT_BAR);
        FreeSpriteOamMatrix(&gSprites[sPokedexView->statBarsSpriteId]);
        DestroySprite(&gSprites[sPokedexView->statBarsSpriteId]);
        sPokedexView->statBarsSpriteId = 0xFF;
    }
}
static void SpriteCB_StatBarsBg(struct Sprite *sprite)
{
    if (IsMonInfoBeingLoaded())
        sprite->invisible = TRUE;
    if (sPokedexView->currentPage != PAGE_MAIN)
    {
        FreeSpriteTilesByTag(TAG_STAT_BAR_BG);
        FreeSpriteOamMatrix(&gSprites[sPokedexView->statBarsBgSpriteId]);
        DestroySprite(&gSprites[sPokedexView->statBarsBgSpriteId]);
        sPokedexView->statBarsBgSpriteId = 0xFF;
    }
}



//************************************
//*                                  *
//*        Info screen               *
//*                                  *
//************************************
#define tScrolling       data[0]
#define tMonSpriteDone   data[1]
#define tBgLoaded        data[2]
#define tSkipCry         data[3]
#define tMonSpriteId     data[4]
#define tTrainerSpriteId data[5]

static u8 LoadInfoScreen(struct PokedexListItem *item, u8 monSpriteId)
{
    u8 taskId;

    sPokedexListItem = item;
    taskId = CreateTask(Task_LoadInfoScreen, 0);
    gTasks[taskId].tScrolling = FALSE;
    gTasks[taskId].tMonSpriteDone = TRUE; // Already has sprite from list view
    gTasks[taskId].tBgLoaded = FALSE;
    gTasks[taskId].tSkipCry = FALSE;
    gTasks[taskId].tMonSpriteId = monSpriteId;
    gTasks[taskId].tTrainerSpriteId = SPRITE_NONE;
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sInfoScreen_BgTemplate, ARRAY_COUNT(sInfoScreen_BgTemplate));
    SetBgTilemapBuffer(3, AllocZeroed(BG_SCREEN_SIZE));
    SetBgTilemapBuffer(2, AllocZeroed(BG_SCREEN_SIZE));
    SetBgTilemapBuffer(1, AllocZeroed(BG_SCREEN_SIZE));
    SetBgTilemapBuffer(0, AllocZeroed(BG_SCREEN_SIZE));
    InitWindows(sInfoScreen_WindowTemplates);
    DeactivateAllTextPrinters();

    return taskId;
}

static bool8 IsInfoScreenScrolling(u8 taskId)
{
    if (!gTasks[taskId].tScrolling && gTasks[taskId].func == Task_HandleInfoScreenInput)
        return FALSE;
    else
        return TRUE;
}

static u8 StartInfoScreenScroll(struct PokedexListItem *item, u8 taskId)
{
    sPokedexListItem = item;
    gTasks[taskId].tScrolling = TRUE;
    gTasks[taskId].tMonSpriteDone = FALSE;
    gTasks[taskId].tBgLoaded = FALSE;
    gTasks[taskId].tSkipCry = FALSE;
    return taskId;
}

static void Task_LoadInfoScreen(u8 taskId)
{
    switch (gMain.state)
    {
    case 0:
    default:
        if (!gPaletteFade.active)
        {
            u16 r2;

            sPokedexView->currentPage = PAGE_INFO;
            gPokedexVBlankCB = gMain.vblankCallback;
            SetVBlankCallback(NULL);
            r2 = 0;
            if (gTasks[taskId].tMonSpriteDone)
                r2 += DISPCNT_OBJ_ON;
            if (gTasks[taskId].tBgLoaded)
                r2 |= DISPCNT_BG1_ON;
            ResetOtherVideoRegisters(r2);
            gMain.state = 1;
        }
        break;
    case 1:
        LoadTilesetTilemapHGSS(INFO_SCREEN);
        FillWindowPixelBuffer(WIN_INFO, PIXEL_FILL(0));
        PutWindowTilemap(WIN_INFO);
        gMain.state++;
        break;
    case 2:
        LoadScreenSelectBarMain();
        LoadPokedexBgPalette();
        gMain.state++;
        break;
    case 3:
        sPokedexView->typeIconSpriteIds[0] = 0xFF;
        sPokedexView->typeIconSpriteIds[1] = 0xFF;
        CreateTypeIconSprites();
        gMain.state++;
        break;
    case 4:
        PrintMonInfo(sPokedexListItem->dexNum, sPokedexListItem->owned, 0);
        if (!sPokedexListItem->owned)
            LoadPalette(gPlttBufferUnfaded + 1, BG_PLTT_ID(3) + 1, PLTT_SIZEOF(16 - 1));
        CopyWindowToVram(WIN_INFO, COPYWIN_FULL);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(3);
        gMain.state++;
        break;
    case 5:
        if (!gTasks[taskId].tMonSpriteDone)
        {
            gTasks[taskId].tMonSpriteId = (u16)CreateMonSpriteFromNationalDexNumberHGSS(sPokedexListItem->dexNum, MON_PAGE_X, MON_PAGE_Y, 0);
            gSprites[gTasks[taskId].tMonSpriteId].oam.priority = 0;
        }
        gMain.state++;
        break;
    case 6:
        {
            u32 preservedPalettes = 0;

            if (gTasks[taskId].tBgLoaded)
                preservedPalettes = 0x14; // each bit represents a palette index
            if (gTasks[taskId].tMonSpriteDone)
                preservedPalettes |= (1 << (gSprites[gTasks[taskId].tMonSpriteId].oam.paletteNum + 16));
            BeginNormalPaletteFade(~preservedPalettes, 0, 16, 0, RGB_BLACK);
            SetVBlankCallback(gPokedexVBlankCB);
            gMain.state++;
        }
        break;
    case 7:
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
        HideBg(0);
        ShowBg(1);
        ShowBg(2);
        ShowBg(3);
        gMain.state++;
        break;
    case 8:
        if (!gPaletteFade.active)
        {
            gMain.state++;
        }
        break;
    case 9:
        gMain.state++;
        break;
    case 10:
        gTasks[taskId].tScrolling = FALSE;
        gTasks[taskId].tMonSpriteDone = FALSE; // Reload next time screen comes up
        gTasks[taskId].tBgLoaded = TRUE;
        gTasks[taskId].tSkipCry = TRUE;
        gTasks[taskId].func = Task_HandleInfoScreenInput;
        gMain.state = 0;
        break;
    }
}

static void FreeInfoScreenWindowAndBgBuffers(void)
{
    void *tilemapBuffer;

    FreeAllWindowBuffers();
    tilemapBuffer = GetBgTilemapBuffer(0);
    if (tilemapBuffer)
        Free(tilemapBuffer);
    tilemapBuffer = GetBgTilemapBuffer(1);
    if (tilemapBuffer)
        Free(tilemapBuffer);
    tilemapBuffer = GetBgTilemapBuffer(2);
    if (tilemapBuffer)
        Free(tilemapBuffer);
    tilemapBuffer = GetBgTilemapBuffer(3);
    if (tilemapBuffer)
        Free(tilemapBuffer);
}

static void Task_HandleInfoScreenInput(u8 taskId)
{
    if (gTasks[taskId].tScrolling)
    {
        // Scroll up/down
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_LoadInfoScreenWaitForFade;
        PlaySE(SE_DEX_SCROLL);
        return;
    }
    if (JOY_NEW(B_BUTTON))
    {
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_ExitInfoScreen;
        PlaySE(SE_PC_OFF);
        return;
    }

    if ((JOY_NEW(DPAD_RIGHT) || (JOY_NEW(R_BUTTON) && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_LR)))
    {
        sPokedexView->selectedScreen = AREA_SCREEN;
        BeginNormalPaletteFade(0xFFFFFFEB, 0, 0, 0x10, RGB_BLACK);
        sPokedexView->screenSwitchState = 1;
        gTasks[taskId].func = Task_SwitchScreensFromInfoScreen;
        PlaySE(SE_PIN);
    }

}

static void Task_SwitchScreensFromInfoScreen(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);
        switch (sPokedexView->screenSwitchState)
        {
        case 1:
        default:
            gTasks[taskId].func = Task_LoadAreaScreen;
            break;
        }
    }
}

static void Task_LoadInfoScreenWaitForFade(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);
        gTasks[taskId].func = Task_LoadInfoScreen;
    }
}

static void Task_ExitInfoScreen(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);
        FreeInfoScreenWindowAndBgBuffers();
        DestroyTask(taskId);
    }
}

#undef tMonSpriteId

//************************************
//*                                  *
//*        Area screen               *
//*                                  *
//************************************
static void Task_LoadAreaScreen(u8 taskId)
{
    switch (gMain.state)
    {
    case 0:
    default:
        if (!gPaletteFade.active)
        {
            sPokedexView->currentPage = PAGE_AREA;
            gPokedexVBlankCB = gMain.vblankCallback;
            SetVBlankCallback(NULL);
            ResetOtherVideoRegisters(DISPCNT_BG1_ON);
            sPokedexView->selectedScreen = AREA_SCREEN;
            gMain.state = 1;
        }
        break;
    case 1:
        LoadPokedexBgPalette();
        SetGpuReg(REG_OFFSET_BG1CNT, BGCNT_PRIORITY(0) | BGCNT_CHARBASE(0) | BGCNT_SCREENBASE(13) | BGCNT_16COLOR | BGCNT_TXT256x256);
        gMain.state++;
        break;
    case 2:
        ShowPokedexAreaScreen(NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum), &sPokedexView->screenSwitchState);
        SetVBlankCallback(gPokedexVBlankCB);
        sPokedexView->screenSwitchState = 0;
        gMain.state = 0;
        gTasks[taskId].func = Task_WaitForAreaScreenInput;
        break;
    }
}

static void Task_WaitForAreaScreenInput(u8 taskId)
{
// See Task_HandlePokedexAreaScreenInput() in pokedex_area_screen.c
    if (sPokedexView->screenSwitchState != 0)
        gTasks[taskId].func = Task_SwitchScreensFromAreaScreen;
}

static void Task_SwitchScreensFromAreaScreen(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        switch (sPokedexView->screenSwitchState)
        {
        case 1:
        default:
            gTasks[taskId].func = Task_LoadInfoScreen;
            break;
        case 2:
            if (!sPokedexListItem->owned)
                PlaySE(SE_FAILURE);
            else
                gTasks[taskId].func = Task_LoadStatsScreen;
            break;
        }
    }
}



//************************************
//*                                  *
//*        Select bar                *
//*                                  *
//************************************
static void LoadScreenSelectBarMain(void)
{
    CopyToBgTilemapBuffer(1, sPokedexPlusHGSS_ScreenSelectBarSubmenu_Tilemap_Clear, 0, 0);
    CopyBgTilemapBufferToVram(1);
}

//************************************
//*                                  *
//*        CAUGHT MON                *
//*                                  *
//************************************
#define tState         data[0]
#define tSpecies       data[1]
#define tPalTimer      data[2]
#define tMonSpriteId   data[3]
#define tOtIdLo        data[12]
#define tOtIdHi        data[13]
#define tPersonalityLo data[14]
#define tPersonalityHi data[15]

void Task_DisplayCaughtMonDexPageHGSS(u8 taskId)
{
    u8 spriteId;
    u16 species;
    u16 dexNum;

    species = gTasks[taskId].tSpecies;
    dexNum = SpeciesToNationalPokedexNum(species);
    switch (gTasks[taskId].tState)
    {
    case 0:
    default:
        if (!gPaletteFade.active)
        {
            gPokedexVBlankCB = gMain.vblankCallback;
            SetVBlankCallback(NULL);
            ResetOtherVideoRegisters(DISPCNT_BG0_ON);
            ResetBgsAndClearDma3BusyFlags(0);
            InitBgsFromTemplates(0, sNewEntryInfoScreen_BgTemplate, ARRAY_COUNT(sNewEntryInfoScreen_BgTemplate));
            SetBgTilemapBuffer(3, AllocZeroed(BG_SCREEN_SIZE));
            SetBgTilemapBuffer(2, AllocZeroed(BG_SCREEN_SIZE));
            InitWindows(sNewEntryInfoScreen_WindowTemplates);
            DeactivateAllTextPrinters();
            gTasks[taskId].tState = 1;
        }
        break;
    case 1:
        sPokedexView = AllocZeroed(sizeof(struct PokedexView)); //for type icons
        ResetPokedexView(sPokedexView);

        if (GetSpeciesFormTable(species) != NULL)
            sPokedexView->formSpecies = species;
        else
            sPokedexView->formSpecies = 0;

        LoadTilesetTilemapHGSS(INFO_SCREEN);
        FillWindowPixelBuffer(WIN_INFO, PIXEL_FILL(0));
        PutWindowTilemap(WIN_INFO);
        ResetPaletteFade();
        LoadPokedexBgPalette();
        gTasks[taskId].tState++;
        break;
    case 2:
        sPokedexView->typeIconSpriteIds[0] = 0xFF;
        sPokedexView->typeIconSpriteIds[1] = 0xFF;
        CreateTypeIconSprites();
        gTasks[taskId].tState++;
        break;
    case 3:
        PrintMonInfo(dexNum, 1, 1);
        CopyWindowToVram(WIN_INFO, COPYWIN_FULL);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(3);
        gTasks[taskId].tState++;
        break;
    case 4:
        spriteId = CreateMonSpriteFromNationalDexNumberHGSS(dexNum, MON_PAGE_X, MON_PAGE_Y, 0);
        gSprites[spriteId].oam.priority = 0;
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0x10, 0, RGB_BLACK);
        SetVBlankCallback(gPokedexVBlankCB);
        gTasks[taskId].tMonSpriteId = spriteId;
        gTasks[taskId].tState++;
        break;
    case 5:
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
        ShowBg(2);
        ShowBg(3);
        gTasks[taskId].tState++;
        break;
    case 6:
        if (!gPaletteFade.active)
        {
            PlayCry_Normal(species, 0);
            gTasks[taskId].tPalTimer = 0;
            gTasks[taskId].func = Task_HandleCaughtMonPageInput;
        }
        break;
    }
}

static void Task_HandleCaughtMonPageInput(u8 taskId)
{
    if (JOY_NEW(A_BUTTON | B_BUTTON))
    {
        BeginNormalPaletteFade(PALETTES_BG, 0, 0, 16, RGB_BLACK);
        SetSpriteInvisibility(0, TRUE);
        SetSpriteInvisibility(1, TRUE);
        gSprites[gTasks[taskId].tMonSpriteId].callback = SpriteCB_SlideCaughtMonToCenter;
        gTasks[taskId].func = Task_ExitCaughtMonPage;
    }
}

static void Task_ExitCaughtMonPage(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        u16 species;
        u32 otId;
        u32 personality;
        u8 paletteNum;
        const u32 *lzPaletteData;
        void *buffer;

        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
        FreeAllWindowBuffers();
        buffer = GetBgTilemapBuffer(2);
        if (buffer)
            Free(buffer);
        buffer = GetBgTilemapBuffer(3);
        if (buffer)
            Free(buffer);

        species = gTasks[taskId].tSpecies;
        otId = ((u16)gTasks[taskId].tOtIdHi << 16) | (u16)gTasks[taskId].tOtIdLo;
        personality = ((u16)gTasks[taskId].tPersonalityHi << 16) | (u16)gTasks[taskId].tPersonalityLo;
        paletteNum = gSprites[gTasks[taskId].tMonSpriteId].oam.paletteNum;
        lzPaletteData = GetMonSpritePalFromSpeciesAndPersonality(species, otId, personality);
        LoadCompressedPalette(lzPaletteData, OBJ_PLTT_ID(paletteNum), PLTT_SIZE_4BPP);
        DestroyTask(taskId);
    }
}

static void SpriteCB_SlideCaughtMonToCenter(struct Sprite *sprite)
{
    if (sprite->x < 0x78)
        sprite->x += 2;
    if (sprite->x > 0x78)
        sprite->x -= 2;

    if (sprite->y < 0x50)
        sprite->y += 1;
    if (sprite->y > 0x50)
        sprite->y -= 1;
}

#undef tState
#undef tDexNum
#undef tPalTimer
#undef tMonSpriteId
#undef tOtIdLo
#undef tOtIdHi
#undef tPersonalityLo
#undef tPersonalityHi







//************************************
//*                                  *
//*        Print data                *
//*                                  *
//************************************
static void PrintInfoScreenText(const u8 *str, u8 left, u8 top)
{
    u8 color[3];
    color[0] = TEXT_COLOR_TRANSPARENT;
    color[1] = TEXT_DYNAMIC_COLOR_6;
    color[2] = TEXT_COLOR_LIGHT_GRAY;

    AddTextPrinterParameterized4(0, 1, left, top, 0, 0, color, -1, str);
}
static void PrintInfoScreenTextWhite(const u8* str, u8 left, u8 top)
{
    u8 color[3];
    color[0] = TEXT_COLOR_TRANSPARENT;
    color[1] = TEXT_COLOR_WHITE;
    color[2] = TEXT_DYNAMIC_COLOR_6;

    AddTextPrinterParameterized4(0, FONT_NORMAL, left, top, 0, 0, color, TEXT_SKIP_DRAW, str);
}

static void PrintInfoScreenTextSmall(const u8* str, u8 left, u8 top)
{
    u8 color[3];
    color[0] = TEXT_COLOR_TRANSPARENT;
    color[1] = TEXT_DYNAMIC_COLOR_6;
    color[2] = TEXT_COLOR_LIGHT_GRAY;

    AddTextPrinterParameterized4(0, 0, left, top, 0, 0, color, 0, str);
}

//Stats screen
static void PrintStatsScreenTextSmall(u8 windowId, const u8* str, u8 left, u8 top)
{
    u8 color[3];
    color[0] = TEXT_COLOR_TRANSPARENT;
    color[1] = TEXT_DYNAMIC_COLOR_6;
    color[2] = TEXT_COLOR_LIGHT_GRAY;

    AddTextPrinterParameterized4(windowId, 0, left, top, 0, 0, color, 0, str);
}
static void PrintStatsScreenTextSmallWhite(u8 windowId, const u8* str, u8 left, u8 top)
{
    u8 color[3];
    color[0] = TEXT_COLOR_TRANSPARENT;
    color[1] = TEXT_COLOR_WHITE;
    color[2] = TEXT_DYNAMIC_COLOR_6;

    AddTextPrinterParameterized4(windowId, 0, left, top, 0, 0, color, 0, str);
}

//Type Icon
static void SetSpriteInvisibility(u8 spriteArrayId, bool8 invisible)
{
    gSprites[sPokedexView->typeIconSpriteIds[spriteArrayId]].invisible = invisible;
}
static const u8 sContestCategoryToOamPaletteNum[CONTEST_CATEGORIES_COUNT] =
{
    [CONTEST_CATEGORY_COOL] = 13,
    [CONTEST_CATEGORY_BEAUTY] = 14,
    [CONTEST_CATEGORY_CUTE] = 14,
    [CONTEST_CATEGORY_SMART] = 15,
    [CONTEST_CATEGORY_TOUGH] = 13,
};

static void SetTypeIconPosAndPal(u8 typeId, u8 x, u8 y, u8 spriteArrayId)
{
    struct Sprite *sprite;

    sprite = &gSprites[sPokedexView->typeIconSpriteIds[spriteArrayId]];
    StartSpriteAnim(sprite, typeId);
    if (typeId < NUMBER_OF_MON_TYPES)
        sprite->oam.paletteNum = gTypesInfo[typeId].palette;
    else
        sprite->oam.paletteNum = sContestCategoryToOamPaletteNum[typeId - NUMBER_OF_MON_TYPES];
    sprite->x = x + 16;
    sprite->y = y + 8;
    SetSpriteInvisibility(spriteArrayId, FALSE);
}

static void PrintCurrentSpeciesTypeInfo(u8 newEntry, u16 species)
{
    u8 type1, type2;

    if (!newEntry)
    {
        species = NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum);
    }
    //type icon(s)
    type1 = gSpeciesInfo[species].types[0];
    type2 = gSpeciesInfo[species].types[1];
    if (species == SPECIES_NONE)
        type1 = type2 = TYPE_MYSTERY;

    if (type1 == type2)
    {
        SetTypeIconPosAndPal(type1, 147, 47, 0);
        SetSpriteInvisibility(1, TRUE);
    }
    else
    {
        SetTypeIconPosAndPal(type1, 147, 47, 0);
        SetTypeIconPosAndPal(type2, 147 + 33, 47, 1);
    }
}

static void CreateTypeIconSprites(void)
{
    u8 i;

    LoadCompressedSpriteSheet(&gSpriteSheet_MoveTypes);
    LoadCompressedPalette(gMoveTypes_Pal, OBJ_PLTT_ID(14), 2 * PLTT_SIZE_4BPP);
    for (i = 0; i < 2; i++)
    {
        if (sPokedexView->typeIconSpriteIds[i] == 0xFF)
            sPokedexView->typeIconSpriteIds[i] = CreateSprite(&gSpriteTemplate_MoveTypes, 10, 10, 2);

        SetSpriteInvisibility(i, TRUE);
    }
}

void PrintMonMeasurements(u16 species, u32 owned)
{
    u32 x = GetMeasurementTextPositions(DEX_HEADER_X);
    u32 yTop = GetMeasurementTextPositions(DEX_Y_TOP);
    u32 yBottom = GetMeasurementTextPositions(DEX_Y_BOTTOM);

    PrintInfoScreenText(gText_HTHeight, x, yTop);
    PrintInfoScreenText(gText_WTWeight, x, yBottom);

    if (owned)
        PrintOwnedMonMeasurements(species);
    else
        PrintUnknownMonMeasurements();
}

static void PrintOwnedMonMeasurements(u16 species)
{
    PrintOwnedMonHeight(species);
    PrintOwnedMonWeight(species);
}

static void PrintOwnedMonHeight(u16 species)
{
    u32 height = GetSpeciesHeight(species);
    u8* heightString;

    u32 x = GetMeasurementTextPositions(DEX_MEASUREMENT_X);
    u32 yTop = GetMeasurementTextPositions(DEX_Y_TOP);

    heightString = ConvertMonHeightToString(height);

    PrintInfoScreenText(heightString, x, yTop);
    Free(heightString);
}

static void PrintOwnedMonWeight(u16 species)
{
    u32 weight = GetSpeciesWeight(species);
    u8* weightString;
    u32 x = GetMeasurementTextPositions(DEX_MEASUREMENT_X);
    u32 yBottom = GetMeasurementTextPositions(DEX_Y_BOTTOM);

    weightString = ConvertMonWeightToString(weight);

    PrintInfoScreenText(weightString, x, yBottom);
    Free(weightString);
}

u8* ConvertMonWeightToString(u32 weight)
{
    return ConvertMonWeightToMetricString(weight);
}

static u8* ConvertMonHeightToMetricString(u32 height)
{
    u32 index = 0;
    u8* heightString = ConvertMeasurementToMetricString(height, &index);

    heightString[index++] = CHAR_m;
    heightString[index++] = EOS;
    return heightString;
}

static u8* ConvertMonWeightToMetricString(u32 weight)
{
    u32 index = 0;
    u8* weightString = ConvertMeasurementToMetricString(weight, &index);

    weightString[index++] = CHAR_k;
    weightString[index++] = CHAR_g;
    weightString[index++] = CHAR_PERIOD;
    weightString[index++] = EOS;
    return weightString;
}

static u32 GetMeasurementTextPositions(u32 textElement)
{
    switch(textElement)
    {
        case DEX_HEADER_X:
            return (DEX_HEADER_X + DEX_HGSS_HEADER_X_PADDING);
        case DEX_Y_TOP:
            return (DEX_Y_TOP + DEX_HGSS_Y_TOP_PADDING);
        case DEX_Y_BOTTOM:
            return (DEX_Y_BOTTOM + DEX_HGSS_Y_BOTTOM_PADDING);
        default:
        case DEX_MEASUREMENT_X:
            return (DEX_MEASUREMENT_X + DEX_HGSS_MEASUREMENT_X_PADDING);
    }
}

static void PrintUnknownMonMeasurements(void)
{
    u8* heightString = GetUnknownMonHeightString();
    u8* weightString = GetUnknownMonWeightString();

    u32 x = GetMeasurementTextPositions(DEX_MEASUREMENT_X);
    u32 yTop = GetMeasurementTextPositions(DEX_Y_TOP);
    u32 yBottom = GetMeasurementTextPositions(DEX_Y_BOTTOM);

    PrintInfoScreenText(heightString, x, yTop);
    PrintInfoScreenText(weightString, x, yBottom);

    Free(heightString);
    Free(weightString);
}

static u8* GetUnknownMonHeightString(void)
{
    return ReplaceDecimalSeparator(gText_UnkHeight);
}

static u8* GetUnknownMonWeightString(void)
{
    return ReplaceDecimalSeparator(gText_UnkWeight);
}

static u8* ReplaceDecimalSeparator(const u8* originalString)
{
    bool32 replaced = FALSE;
    u32 length = StringLength(originalString), i;
    u8* modifiedString = Alloc(WEIGHT_HEIGHT_STR_MEM);

    for (i = 0; i < length; i++)
    {
        if ((originalString[i] != CHAR_PERIOD) || replaced)
        {
            modifiedString[i] = originalString[i];
            continue;
        }

        modifiedString[i] = CHAR_DEC_SEPARATOR;
        replaced = TRUE;
    }
    modifiedString[length] = EOS;
    return modifiedString;
}

static void PrintMonInfo(u32 num, u32 owned, u32 newEntry)
{
    u8 str[16];
    u8 str2[32];
    u16 species;
    const u8 *name;
    const u8 *category;
    const u8 *description;
    u8 digitCount = (DEX_COUNT > 999) ? 4 : 3;

    ConvertIntToDecimalStringN(StringCopy(str, gText_NumberClear01), num, STR_CONV_MODE_LEADING_ZEROS, digitCount);
    PrintInfoScreenTextWhite(str, 123, 17);
    species = NationalPokedexNumToSpeciesHGSS(num);
    if (species)
        name = GetSpeciesName(species);
    else
        name = sText_TenDashes;
    PrintInfoScreenTextWhite(name, 139 + (6 * digitCount), 17);
    if (owned)
    {
        CopyMonCategoryText(species, str2);
        category = str2;
    }
    else
    {
        category = gText_5MarksPokemon;
    }
    PrintInfoScreenText(category, 123, 31);
    PrintMonMeasurements(species, owned);
    if (owned)
        description = GetSpeciesPokedexDescription(species);
    else
        description = sExpandedPlaceholder_PokedexDescription;
    PrintInfoScreenText(description, GetStringCenterAlignXOffset(FONT_NORMAL, description, 0xF0), 93);

    //Type Icon(s)
    if (owned)
        PrintCurrentSpeciesTypeInfo(newEntry, species);
}

u16 CreateMonSpriteFromNationalDexNumberHGSS(u16 nationalNum, s16 x, s16 y, u16 paletteSlot)
{
    nationalNum = NationalPokedexNumToSpeciesHGSS(nationalNum);
    return CreateMonPicSprite(nationalNum, FALSE, 0xFF, TRUE, x, y, paletteSlot, TAG_NONE);
}

//************************************
//*                                  *
//*        Helper functions          *
//*                                  *
//************************************

static void ResetOtherVideoRegisters(u16 regBits)
{
    if (!(regBits & DISPCNT_BG0_ON))
    {
        ClearGpuRegBits(0, DISPCNT_BG0_ON);
        SetGpuReg(REG_OFFSET_BG0CNT, 0);
        SetGpuReg(REG_OFFSET_BG0HOFS, 0);
        SetGpuReg(REG_OFFSET_BG0VOFS, 0);
    }
    if (!(regBits & DISPCNT_BG1_ON))
    {
        ClearGpuRegBits(0, DISPCNT_BG1_ON);
        SetGpuReg(REG_OFFSET_BG1CNT, 0);
        SetGpuReg(REG_OFFSET_BG1HOFS, 0);
        SetGpuReg(REG_OFFSET_BG1VOFS, 0);
    }
    if (!(regBits & DISPCNT_BG2_ON))
    {
        ClearGpuRegBits(0, DISPCNT_BG2_ON);
        SetGpuReg(REG_OFFSET_BG2CNT, 0);
        SetGpuReg(REG_OFFSET_BG2HOFS, 0);
        SetGpuReg(REG_OFFSET_BG2VOFS, 0);
    }
    if (!(regBits & DISPCNT_BG3_ON))
    {
        ClearGpuRegBits(0, DISPCNT_BG3_ON);
        SetGpuReg(REG_OFFSET_BG3CNT, 0);
        SetGpuReg(REG_OFFSET_BG3HOFS, 0);
        SetGpuReg(REG_OFFSET_BG3VOFS, 0);
    }
    if (!(regBits & DISPCNT_OBJ_ON))
    {
        ClearGpuRegBits(0, DISPCNT_OBJ_ON);
        ResetSpriteData();
        FreeAllSpritePalettes();
        gReservedSpritePaletteCount = 8;
    }
}

static void FreeWindowAndBgBuffers(void)
{
    void *tilemapBuffer;

    FreeAllWindowBuffers();
    tilemapBuffer = GetBgTilemapBuffer(0);
    if (tilemapBuffer)
        Free(tilemapBuffer);
    tilemapBuffer = GetBgTilemapBuffer(1);
    if (tilemapBuffer)
        Free(tilemapBuffer);
    tilemapBuffer = GetBgTilemapBuffer(2);
    if (tilemapBuffer)
        Free(tilemapBuffer);
    tilemapBuffer = GetBgTilemapBuffer(3);
    if (tilemapBuffer)
        Free(tilemapBuffer);
}

static u16 GetNextPosition(u8 direction, u16 position, u16 min, u16 max)
{
    switch (direction)
    {
    case 1: // Up/Left
        if (position > min)
            position--;
        break;
    case 0: // Down/Right
        if (position < max)
            position++;
        break;
    case 3: // Up/Left with loop (unused)
        if (position > min)
            position--;
        else
            position = max;
        break;
    case 2: // Down/Right with loop (unused)
        if (position < max)
            position++;
        else
            position = min;
        break;
    }
    return position;
}



//************************************
//*                                  *
//*        HGSS                      *
//*                                  *
//************************************
static u16 NationalPokedexNumToSpeciesHGSS(u16 nationalNum)
{
    if (!nationalNum)
        return 0;

    if (sPokedexView->formSpecies != 0)
        return sPokedexView->formSpecies;
    else
        return NationalPokedexNumToSpecies(nationalNum);
}

static void LoadTilesetTilemapHGSS(u8 page)
{
    switch (page)
    {
    case INFO_SCREEN:
        if (!HGSS_DECAPPED)
            DecompressAndLoadBgGfxUsingHeap(3, sPokedexPlusHGSS_Menu_1_Gfx, 0x2000, 0, 0);
        else
            DecompressAndLoadBgGfxUsingHeap(3, sPokedexPlusHGSS_Menu_1_Gfx, 0x2000, 0, 0);
        CopyToBgTilemapBuffer(3, sPokedexPlusHGSS_ScreenInfo_Tilemap, 0, 0);
        break;
    case STATS_SCREEN:
        if (!HGSS_DECAPPED)
            DecompressAndLoadBgGfxUsingHeap(3, sPokedexPlusHGSS_Menu_1_Gfx, 0x2000, 0, 0);
        else
            DecompressAndLoadBgGfxUsingHeap(3, sPokedexPlusHGSS_Menu_1_Gfx, 0x2000, 0, 0);
        CopyToBgTilemapBuffer(3, sPokedexPlusHGSS_ScreenStats_Tilemap, 0, 0);
        break;
    case EVO_SCREEN:
        if (!HGSS_DECAPPED)
            DecompressAndLoadBgGfxUsingHeap(3, sPokedexPlusHGSS_Menu_2_Gfx, 0x2000, 0, 0);
        else
            DecompressAndLoadBgGfxUsingHeap(3, sPokedexPlusHGSS_Menu_2_Gfx, 0x2000, 0, 0);
            CopyToBgTilemapBuffer(3, sPokedexPlusHGSS_ScreenEvolution_Tilemap_PE, 0, 0);
        break;
    case FORMS_SCREEN:
        if (!HGSS_DECAPPED)
            DecompressAndLoadBgGfxUsingHeap(3, sPokedexPlusHGSS_Menu_2_Gfx, 0x2000, 0, 0);
        else
            DecompressAndLoadBgGfxUsingHeap(3, sPokedexPlusHGSS_Menu_2_Gfx, 0x2000, 0, 0);
        CopyToBgTilemapBuffer(3, sPokedexPlusHGSS_ScreenForms_Tilemap, 0, 0);
        break;
    }
}

//Physical/Special/Status category
static u8 ShowCategoryIcon(u32 category)
{
    if (sPokedexView->categoryIconSpriteId == 0xFF)
        sPokedexView->categoryIconSpriteId = CreateSprite(&gSpriteTemplate_CategoryIcons, 139, 90, 0);

    gSprites[sPokedexView->categoryIconSpriteId].invisible = FALSE;
    StartSpriteAnim(&gSprites[sPokedexView->categoryIconSpriteId], category);
    return sPokedexView->categoryIconSpriteId;
}

static void DestroyCategoryIcon(void)
{
    if (sPokedexView->categoryIconSpriteId != 0xFF)
        DestroySprite(&gSprites[sPokedexView->categoryIconSpriteId]);
    sPokedexView->categoryIconSpriteId = 0xFF;
}

//************************************
//*                                  *
//*        STATS                     *
//*                                  *
//************************************
static const u8 sStatsPageNavigationTextColor[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY};

static void StatsPage_PrintNavigationButtons(void)
{
    u8 x = 9;
    u8 y = 0;
    if (!HGSS_DECAPPED)
        AddTextPrinterParameterized3(WIN_STATS_NAVIGATION_BUTTONS, 0, x, y, sStatsPageNavigationTextColor, 0, sText_Stats_Buttons);
    else
        AddTextPrinterParameterized3(WIN_STATS_NAVIGATION_BUTTONS, 0, x, y, sStatsPageNavigationTextColor, 0, sText_Stats_Buttons_Decapped);

    PutWindowTilemap(WIN_STATS_NAVIGATION_BUTTONS);
    CopyWindowToVram(WIN_STATS_NAVIGATION_BUTTONS, 3);
}

static void ResetStatsWindows(void)
{
    u8 i;

    FreeAllWindowBuffers();
    InitWindows(sStatsScreen_WindowTemplates);

    for (i = 0; i < WIN_STATS_END + 1; i++)
    {
        FillWindowPixelBuffer(i, PIXEL_FILL(0));
        PutWindowTilemap(i);
        CopyWindowToVram(i, COPYWIN_FULL);
    }
}

static void SaveMonDataInStruct(void)
{
    u16 species = NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum);
    u8 evs[NUM_STATS] =
    {
        [STAT_HP]    = gSpeciesInfo[species].evYield_HP,
        [STAT_ATK]   = gSpeciesInfo[species].evYield_Speed,
        [STAT_DEF]   = gSpeciesInfo[species].evYield_Attack,
        [STAT_SPEED] = gSpeciesInfo[species].evYield_SpAttack,
        [STAT_SPATK] = gSpeciesInfo[species].evYield_Defense,
        [STAT_SPDEF] = gSpeciesInfo[species].evYield_SpDefense
    };
    u8 differentEVs = 0;
    u8 i;

    //Count how many different EVs
    for (i = 0; i < NUM_STATS; i++)
    {
        if (evs[i] > 0) //HP//Speed//Attack//Special Attack//Defense//Special Defense
            differentEVs++;
    }

    sPokedexView->sPokemonStats.species             = species;
    sPokedexView->sPokemonStats.genderRatio         = gSpeciesInfo[species].genderRatio;
    sPokedexView->sPokemonStats.baseHP              = gSpeciesInfo[species].baseHP;
    sPokedexView->sPokemonStats.baseSpeed           = gSpeciesInfo[species].baseSpeed;
    sPokedexView->sPokemonStats.baseAttack          = gSpeciesInfo[species].baseAttack;
    sPokedexView->sPokemonStats.baseSpAttack        = gSpeciesInfo[species].baseSpAttack;
    sPokedexView->sPokemonStats.baseDefense         = gSpeciesInfo[species].baseDefense;
    sPokedexView->sPokemonStats.baseSpDefense       = gSpeciesInfo[species].baseSpDefense;
    sPokedexView->sPokemonStats.differentEVs        = differentEVs;
    sPokedexView->sPokemonStats.evYield_HP          = evs[STAT_HP];
    sPokedexView->sPokemonStats.evYield_Speed       = evs[STAT_ATK];
    sPokedexView->sPokemonStats.evYield_Attack      = evs[STAT_DEF];
    sPokedexView->sPokemonStats.evYield_SpAttack    = evs[STAT_SPEED];
    sPokedexView->sPokemonStats.evYield_Defense     = evs[STAT_SPATK];
    sPokedexView->sPokemonStats.evYield_SpDefense   = evs[STAT_SPDEF];
    sPokedexView->sPokemonStats.growthRate          = gSpeciesInfo[species].growthRate;
    sPokedexView->sPokemonStats.eggGroup1           = gSpeciesInfo[species].eggGroups[0];
    sPokedexView->sPokemonStats.eggGroup2           = gSpeciesInfo[species].eggGroups[1];
    sPokedexView->sPokemonStats.eggCycles           = gSpeciesInfo[species].eggCycles;
    sPokedexView->sPokemonStats.expYield            = gSpeciesInfo[species].expYield;
    sPokedexView->sPokemonStats.friendship          = gSpeciesInfo[species].friendship;
    sPokedexView->sPokemonStats.ability0            = GetAbilityBySpecies(species, 0);
    sPokedexView->sPokemonStats.ability1            = GetAbilityBySpecies(species, 1);
    sPokedexView->sPokemonStats.abilityHidden       = GetAbilityBySpecies(species, 2);
}

#define tMonSpriteId data[4]

static void Task_LoadStatsScreen(u8 taskId)
{
    switch (gMain.state)
    {
    case 0:
    default:
        if (!gPaletteFade.active)
        {
            u16 r2;

            sPokedexView->currentPage = STATS_SCREEN;
            gPokedexVBlankCB = gMain.vblankCallback;
            SetVBlankCallback(NULL);
            r2 = 0;
            if (gTasks[taskId].data[1] != 0)
                r2 += DISPCNT_OBJ_ON;
            if (gTasks[taskId].data[2] != 0)
                r2 |= DISPCNT_BG1_ON;
            ResetOtherVideoRegisters(r2);
            gMain.state = 1;
        }
        break;
    case 1:
        LoadTilesetTilemapHGSS(STATS_SCREEN);

        ResetStatsWindows();

        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(3);
        gMain.state++;
        break;
    case 2:
        LoadScreenSelectBarMain();
        LoadPokedexBgPalette();
        gMain.state++;
        break;
    case 3:
        sPokedexView->typeIconSpriteIds[0] = 0xFF;
        sPokedexView->typeIconSpriteIds[1] = 0xFF;
        CreateTypeIconSprites();
        sPokedexView->categoryIconSpriteId = 0xFF;
        LoadCompressedPalette(gMoveTypes_Pal, OBJ_PLTT_ID(14), 2 * PLTT_SIZE_4BPP);
        LoadCompressedSpriteSheet(&gSpriteSheet_CategoryIcons);
        LoadSpritePalette(&gSpritePal_CategoryIcons);
        gMain.state++;
        break;
    case 4:
        SaveMonDataInStruct();
        sPokedexView->moveSelected = 0;
        sPokedexView->movesTotal = 0;
        sPokedexView->numEggMoves = 0;
        sPokedexView->numLevelUpMoves = 0;
        sPokedexView->numTMHMMoves = 0;
        if (CalculateMoves())
            gMain.state++;
        break;
    case 5: //icono de STATS
        if (gTasks[taskId].data[1] == 0)
        {
            //Icon
            //FreeMonIconPalettes(); //Free space for new pallete
            LoadCompressedPalette(GetMonSpritePalFromSpeciesAndPersonality(NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum), 0, 0), OBJ_PLTT_ID(4), PLTT_SIZE_4BPP);
            gTasks[taskId].data[4] = CreateMonIconNoPersonality(NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum), SpriteCB_MonIcon, 18, 31, 4); //Create pokemon sprite
            gSprites[gTasks[taskId].data[4]].oam.priority = 0;
            gSprites[gTasks[taskId].data[4]].oam.paletteNum = 4;
        }
        gMain.state++;
        break;
    case 6:
        gTasks[taskId].data[5] = 0;
        PrintStatsScreen_NameGender(taskId, sPokedexListItem->dexNum);
        PrintStatsScreen_Left(taskId);
        PrintStatsScreen_Abilities(taskId);
        PrintStatsScreen_Moves_Top(taskId);
        PrintStatsScreen_Moves_Description(taskId);
        PrintStatsScreen_Moves_BottomText(taskId);
        PrintStatsScreen_Moves_Bottom(taskId);
        if (!sPokedexListItem->owned)
            LoadPalette(gPlttBufferUnfaded + 1, 0x31, 0x1E);
        StatsPage_PrintNavigationButtons(); //sText_Stats_Buttons
        gMain.state++;
        break;
    case 7:
        {
        u32 preservedPalettes = 0;

        if (gTasks[taskId].data[2] != 0)
            preservedPalettes = 0x14; // each bit represents a palette index
        if (gTasks[taskId].data[1] != 0)
            preservedPalettes |= (1 << (gSprites[gTasks[taskId].tMonSpriteId].oam.paletteNum + 16));
        BeginNormalPaletteFade(~preservedPalettes, 0, 16, 0, RGB_BLACK);
        SetVBlankCallback(gPokedexVBlankCB);
        gMain.state++;
        }
        break;
    case 8:
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
        HideBg(0);
        ShowBg(1);
        ShowBg(2);
        ShowBg(3);
        gMain.state++;
        break;
    case 9:
        if (!gPaletteFade.active)
            gMain.state++;
        break;
    case 10:
        gMain.state++;
        break;
    case 11:
        gTasks[taskId].data[0] = 0;
        gTasks[taskId].data[1] = 0;
        gTasks[taskId].data[2] = 1;
        gTasks[taskId].func = Task_HandleStatsScreenInput;
        gMain.state = 0;
        break;
    }
}

static void Task_HandleStatsScreenInput(u8 taskId)
{
    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_DEX_PAGE);
        if (gTasks[taskId].data[5] == 0)
            gTasks[taskId].data[5] = 1;
        else
            gTasks[taskId].data[5] = 0;

        FillWindowPixelBuffer(WIN_STATS_LEFT, PIXEL_FILL(0));
        PrintStatsScreen_Left(taskId);

        FillWindowPixelBuffer(WIN_STATS_MOVES_DESCRIPTION, PIXEL_FILL(0));
        PrintStatsScreen_Moves_Description(taskId);

        FillWindowPixelBuffer(WIN_STATS_MOVES_BOTTOM, PIXEL_FILL(0));
        PrintStatsScreen_Moves_BottomText(taskId);
        PrintStatsScreen_Moves_Bottom(taskId);

        FillWindowPixelBuffer(WIN_STATS_ABILITIES, PIXEL_FILL(0));
        PrintStatsScreen_Abilities(taskId);
    }
    if (JOY_NEW(B_BUTTON))
    {
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_ExitStatsScreen;
        PlaySE(SE_PC_OFF);
        return;
    }

    //Change moves
    if (JOY_REPEAT(DPAD_UP) && sPokedexView->moveSelected > 0)
    {
        sPokedexView->moveSelected -= 1;
        PlaySE(SE_SELECT);
        FillWindowPixelBuffer(WIN_STATS_MOVES_TOP, PIXEL_FILL(0));
        PrintStatsScreen_DestroyMoveItemIcon(taskId);
        PrintStatsScreen_Moves_Top(taskId);

        FillWindowPixelBuffer(WIN_STATS_MOVES_DESCRIPTION, PIXEL_FILL(0));
        PrintStatsScreen_Moves_Description(taskId);

        FillWindowPixelRect(WIN_STATS_MOVES_BOTTOM, PIXEL_FILL(0), 50, 0, 20, 16);
        FillWindowPixelRect(WIN_STATS_MOVES_BOTTOM, PIXEL_FILL(0), 120, 0, 20, 16);
        PrintStatsScreen_Moves_Bottom(taskId);
    }
    if (JOY_REPEAT(DPAD_DOWN) && sPokedexView->moveSelected < sPokedexView->movesTotal -1 )
    {
        sPokedexView->moveSelected = sPokedexView->moveSelected + 1;
        PlaySE(SE_SELECT);
        FillWindowPixelBuffer(WIN_STATS_MOVES_TOP, PIXEL_FILL(0));
        PrintStatsScreen_DestroyMoveItemIcon(taskId);
        PrintStatsScreen_Moves_Top(taskId);

        FillWindowPixelBuffer(WIN_STATS_MOVES_DESCRIPTION, PIXEL_FILL(0));
        PrintStatsScreen_Moves_Description(taskId);

        FillWindowPixelRect(WIN_STATS_MOVES_BOTTOM, PIXEL_FILL(0), 50, 0, 20, 16);
        FillWindowPixelRect(WIN_STATS_MOVES_BOTTOM, PIXEL_FILL(0), 120, 0, 20, 16);
        PrintStatsScreen_Moves_Bottom(taskId);
    }

    //Switch screens
    if ((JOY_NEW(DPAD_LEFT) || (JOY_NEW(L_BUTTON) && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_LR)))
    {
        sPokedexView->selectedScreen = INFO_SCREEN;
        BeginNormalPaletteFade(0xFFFFFFEB, 0, 0, 0x10, RGB_BLACK);
        sPokedexView->screenSwitchState = 1;
        gTasks[taskId].func = Task_SwitchScreensFromStatsScreen;
        PlaySE(SE_PIN);
    }
    if ((JOY_NEW(DPAD_RIGHT) || (JOY_NEW(R_BUTTON) && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_LR)))
    {
        if (!sPokedexListItem->owned)
            PlaySE(SE_FAILURE);
        else
        {
            sPokedexView->selectedScreen = EVO_SCREEN;
            BeginNormalPaletteFade(0xFFFFFFEB, 0, 0, 0x10, RGB_BLACK);
            sPokedexView->screenSwitchState = 3;
            gTasks[taskId].func = Task_SwitchScreensFromStatsScreen;
            PlaySE(SE_PIN);
        }
    }
}

#define ITEM_TAG 0xFDF3

static void PrintStatsScreen_DestroyMoveItemIcon(u8 taskId)
{
    FreeSpriteTilesByTag(ITEM_TAG);                         //Destroy item icon
    FreeSpritePaletteByTag(ITEM_TAG);                       //Destroy item icon
    FreeSpriteOamMatrix(&gSprites[gTasks[taskId].data[3]]); //Destroy item icon
    DestroySprite(&gSprites[gTasks[taskId].data[3]]);       //Destroy item icon
}

static bool8 CalculateMoves(void)
{
    u16 species = NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum);
    const u16 *teachableLearnset = GetSpeciesTeachableLearnset(species);

    u16 statsMovesEgg[EGG_MOVES_ARRAY_COUNT] = {0};
    u16 statsMovesLevelUp[MAX_LEVEL_UP_MOVES] = {0};
    u16 move;

    u8 numEggMoves = 0;
    u8 numLevelUpMoves = 0;
    u8 numTMHMMoves = 0;
    u8 numTutorMoves = 0;
    u16 movesTotal = 0;
    u8 i,j;

    //Calculate amount of Egg and LevelUp moves
    numEggMoves = GetEggMovesBySpecies(species, statsMovesEgg);
    numLevelUpMoves = GetLevelUpMovesBySpecies(species, statsMovesLevelUp);

    //Egg moves
    for (i=0; i < numEggMoves; i++)
    {
        sStatsMoves[movesTotal] = statsMovesEgg[i];
        movesTotal++;
    }

    //Level up moves
    for (i=0; i < numLevelUpMoves; i++)
    {
        sStatsMoves[movesTotal] = statsMovesLevelUp[i];
        movesTotal++;
    }

    for (i = 0; teachableLearnset[i] != MOVE_UNAVAILABLE; i++)
    {
        move = teachableLearnset[i];
        for (j = 0; j < NUM_TECHNICAL_MACHINES + NUM_HIDDEN_MACHINES; j++)
        {
            if (ItemIdToBattleMoveId(ITEM_TM01 + j) == move)
            {
                sStatsMovesTMHM_ID[numTMHMMoves] = (ITEM_TM01 + j);
                numTMHMMoves++;

                sStatsMoves[movesTotal] = move;
                movesTotal++;
                break;
            }
        }
    }

    for (i = 0; teachableLearnset[i] != MOVE_UNAVAILABLE; i++)
    {
        move = teachableLearnset[i];
        for (j = 0; j < NUM_TECHNICAL_MACHINES + NUM_HIDDEN_MACHINES; j++)
        {
            if (ItemIdToBattleMoveId(ITEM_TM01 + j) == move)
                break;
        }

        if (j >= NUM_TECHNICAL_MACHINES + NUM_HIDDEN_MACHINES)
        {
            numTutorMoves++;

            sStatsMoves[movesTotal] = move;
            movesTotal++;
        }
    }

    sPokedexView->numEggMoves = numEggMoves;
    sPokedexView->numLevelUpMoves = numLevelUpMoves;
    sPokedexView->numTMHMMoves = numTMHMMoves;
    sPokedexView->numTutorMoves = numTutorMoves;
    sPokedexView->movesTotal = movesTotal;

    return TRUE;
}

static void PrintStatsScreen_Moves_Top(u8 taskId)
{
    u8 numEggMoves      = sPokedexView->numEggMoves;
    u8 numLevelUpMoves  = sPokedexView->numLevelUpMoves;
    u8 numTMHMMoves     = sPokedexView->numTMHMMoves;
    u8 numTutorMoves    = sPokedexView->numTutorMoves;
    u8 movesTotal       = sPokedexView->movesTotal;
    u8 selected         = sPokedexView->moveSelected;
    u8 level;
    u8 moves_x = 5;
    u8 moves_y = 3;
    u16 move;
    u16 item;

    u16 species = NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum);

    //Move
    move = sStatsMoves[selected];

    //Moves selected from move max
    ConvertIntToDecimalStringN(gStringVar1, (selected+1), STR_CONV_MODE_RIGHT_ALIGN, 3);
    ConvertIntToDecimalStringN(gStringVar2, movesTotal, STR_CONV_MODE_RIGHT_ALIGN, 3);
    StringExpandPlaceholders(gStringVar1, sText_Stats_MoveSelectedMax);
    PrintStatsScreenTextSmallWhite(WIN_STATS_MOVES_TOP, gStringVar1, moves_x-1, moves_y+1);

    //Move name
    StringCopy(gStringVar3, GetMoveName(move));
    StringCopyPadded(gStringVar3, gStringVar3, CHAR_SPACE, 20);
    PrintStatsScreenTextSmall(WIN_STATS_MOVES_TOP, gStringVar3, moves_x, moves_y + 17);

    //Draw move type icon
    if (gTasks[taskId].data[5] == 0)
    {
        SetTypeIconPosAndPal(gMovesInfo[move].type, moves_x + 146, moves_y + 17, 0);
        SetSpriteInvisibility(1, TRUE);
    }
    else
    {
        SetTypeIconPosAndPal(NUMBER_OF_MON_TYPES + gMovesInfo[move].contestCategory, moves_x + 146, moves_y + 17, 1);
        SetSpriteInvisibility(0, TRUE);
    }

    //Calculate and retrieve correct move from the arrays
    if (selected < numEggMoves)
    {
        PrintStatsScreenTextSmall(WIN_STATS_MOVES_TOP, gText_ThreeDashes, moves_x + 113, moves_y + 9);
        item = ITEM_LUCKY_EGG;
    }
    else if (selected < (numEggMoves + numLevelUpMoves))
    {
        level = GetSpeciesLevelUpLearnset(species)[(selected-numEggMoves)].level;
        ConvertIntToDecimalStringN(gStringVar1, level, STR_CONV_MODE_LEFT_ALIGN, 3); //Move learn lvl
        PrintStatsScreenTextSmall(WIN_STATS_MOVES_TOP, sText_Stats_MoveLevel, moves_x + 113, moves_y + 3); //Level text
        PrintStatsScreenTextSmall(WIN_STATS_MOVES_TOP, gStringVar1, moves_x + 113, moves_y + 14); //Print level
        item = ITEM_RARE_CANDY;
    }
    else if (selected < (numEggMoves + numLevelUpMoves + numTMHMMoves))
    {
        CopyItemName(sStatsMovesTMHM_ID[(selected-numEggMoves-numLevelUpMoves)], gStringVar1); //TM name
        PrintStatsScreenTextSmall(WIN_STATS_MOVES_TOP, gStringVar1, moves_x + 113, moves_y + 9);
        item = sStatsMovesTMHM_ID[(selected-numEggMoves-numLevelUpMoves)];
    }
    else if (selected < (numEggMoves + numLevelUpMoves + numTMHMMoves + numTutorMoves))
    {
        PrintStatsScreenTextSmall(WIN_STATS_MOVES_TOP, gText_ThreeDashes, moves_x + 113, moves_y + 9);
        item = ITEM_TEACHY_TV;
    }
    else
    {
        StringCopy(gStringVar4, gText_No);
        item = ITEM_MASTER_BALL;
    }

    //Egg/TM/Level/Tutor Item Icon
    gTasks[taskId].data[3] = AddItemIconSprite(ITEM_TAG, ITEM_TAG, item);
    gSprites[gTasks[taskId].data[3]].x2 = 203;
    gSprites[gTasks[taskId].data[3]].y2 = 39;
    gSprites[gTasks[taskId].data[3]].oam.priority = 0;

}

static void PrintStatsScreen_Moves_Description(u8 taskId)
{
    u8 selected = sPokedexView->moveSelected;
    u16 move;
    u8 moves_x = 5;
    u8 moves_y = 5;

    //Move
    move = sStatsMoves[selected];

    //Move description
    if (gTasks[taskId].data[5] == 0)
    {
        StringCopy(gStringVar4, gMovesInfo[move].description);
        PrintStatsScreenTextSmall(WIN_STATS_MOVES_DESCRIPTION, gStringVar4, moves_x, moves_y);
    }
    else
    {
        StringCopy(gStringVar4, gContestEffectDescriptionPointers[gMovesInfo[move].contestEffect]);
        PrintStatsScreenTextSmall(WIN_STATS_MOVES_DESCRIPTION, gStringVar4, moves_x, moves_y);
    }
}

static void PrintStatsScreen_Moves_BottomText(u8 taskId)
{
    u8 moves_x = 8;
    u8 moves_y = 3;
    if (gTasks[taskId].data[5] == 0)
    {
        PrintStatsScreenTextSmall(WIN_STATS_MOVES_BOTTOM, gText_Potencia,  moves_x, moves_y);
        PrintStatsScreenTextSmall(WIN_STATS_MOVES_BOTTOM, gText_Precision,  moves_x + 66, moves_y);
    }
    else
    {
        PrintStatsScreenTextSmall(WIN_STATS_MOVES_BOTTOM, gText_Appeal,  moves_x, moves_y);
        PrintStatsScreenTextSmall(WIN_STATS_MOVES_BOTTOM, gText_Jam,  moves_x + 66, moves_y);
    }
}

static void PrintStatsScreen_Moves_Bottom(u8 taskId)
{
    u8 moves_x = 8;
    u8 moves_y = 3;
    u8 selected = sPokedexView->moveSelected;
    u16 move;
    //Contest
    u8 contest_effectValue;
    u8 contest_appeal = 0;
    u8 contest_jam = 0;

    //Move
    move = sStatsMoves[selected];

    //Power + Accuracy
    if (gTasks[taskId].data[5] == 0)
    {
        //Power
        if (gMovesInfo[move].power < 2)
            StringCopy(gStringVar1, gText_ThreeDashes);
        else
            ConvertIntToDecimalStringN(gStringVar1, gMovesInfo[move].power, STR_CONV_MODE_RIGHT_ALIGN, 3);
        PrintStatsScreenTextSmall(WIN_STATS_MOVES_BOTTOM, gStringVar1, moves_x + 45, moves_y);
        //Physical/Special/Status Category
        DestroyCategoryIcon();
        ShowCategoryIcon(GetBattleMoveCategory(move));
        //Accuracy
        if (gMovesInfo[move].accuracy == 0)
            StringCopy(gStringVar1, gText_ThreeDashes);
        else
            ConvertIntToDecimalStringN(gStringVar1, gMovesInfo[move].accuracy, STR_CONV_MODE_RIGHT_ALIGN, 3);
        PrintStatsScreenTextSmall(WIN_STATS_MOVES_BOTTOM, gStringVar1,  moves_x + 114, moves_y);
    }
    else //Appeal + Jam
    {
        DestroyCategoryIcon();
        gSprites[sPokedexView->categoryIconSpriteId].invisible = TRUE;
        //Appeal
        contest_effectValue = gContestEffects[gMovesInfo[move].contestEffect].appeal;
        if (contest_effectValue != 0xFF)
            contest_appeal = contest_effectValue / 10;
        ConvertIntToDecimalStringN(gStringVar1, contest_appeal, STR_CONV_MODE_RIGHT_ALIGN, 1);
        StringCopy(gStringVar2, sText_PlusSymbol);
        StringAppend(gStringVar2, gStringVar1);
        PrintStatsScreenTextSmall(WIN_STATS_MOVES_BOTTOM, gStringVar2, moves_x + 45, moves_y);

        //Jam
        contest_effectValue = gContestEffects[gMovesInfo[move].contestEffect].jam;
        if (contest_effectValue != 0xFF)
            contest_jam = contest_effectValue / 10;
        ConvertIntToDecimalStringN(gStringVar1, contest_jam, STR_CONV_MODE_RIGHT_ALIGN, 1);
        StringCopy(gStringVar2, sText_Stats_Minus);
        StringAppend(gStringVar2, gStringVar1);
        PrintStatsScreenTextSmall(WIN_STATS_MOVES_BOTTOM, gStringVar2,  moves_x + 119, moves_y);
    }
}

static void PrintStatsScreen_NameGender(u8 taskId, u32 num)
{
    u8 str[16];
    u16 species = NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum);

    u8 base_x = 38;
    u8 base_y = 0;
    u8 gender_x, gender_y;

    //Name
    PrintStatsScreenTextSmall(WIN_STATS_NAME_GENDER, GetSpeciesName(species), base_x, base_y);

    ConvertIntToDecimalStringN(StringCopy(str, gText_NumberClear01), num, STR_CONV_MODE_LEADING_ZEROS, 3);
    PrintStatsScreenTextSmall(WIN_STATS_NAME_GENDER, str, base_x, base_y + 10);

    //Gender ratio //MON_GENDERLESS == 0xFF
    gender_x = base_x;
    gender_y = base_y + 20;
    switch (sPokedexView->sPokemonStats.genderRatio)
    {
    case 0:
        PrintStatsScreenTextSmall(WIN_STATS_NAME_GENDER, sText_Stats_Gender_0, gender_x, gender_y);
        break;
    case 31:
        PrintStatsScreenTextSmall(WIN_STATS_NAME_GENDER, sText_Stats_Gender_12_5, gender_x, gender_y);
        break;
    case 63:
        PrintStatsScreenTextSmall(WIN_STATS_NAME_GENDER, sText_Stats_Gender_25, gender_x, gender_y);
        break;
    case 127:
        PrintStatsScreenTextSmall(WIN_STATS_NAME_GENDER, sText_Stats_Gender_50, gender_x, gender_y);
        break;
    case 191:
        PrintStatsScreenTextSmall(WIN_STATS_NAME_GENDER, sText_Stats_Gender_75, gender_x, gender_y);
        break;
    case 223:
        PrintStatsScreenTextSmall(WIN_STATS_NAME_GENDER, sText_Stats_Gender_87_5, gender_x, gender_y);
        break;
    case 254:
        PrintStatsScreenTextSmall(WIN_STATS_NAME_GENDER, sText_Stats_Gender_100, gender_x, gender_y);
        break;
    default:
        PrintStatsScreenTextSmall(WIN_STATS_NAME_GENDER, gText_ThreeDashes, gender_x, gender_y);
        break;
    }
}

static u8 PrintMonStatsToggle_DifferentEVsColumn(u8 differentEVs)
{
    if (differentEVs == 1 || differentEVs == 3)
        return 0;
    else
        return 1;
}

static u8 PrintMonStatsToggle_DifferentEVsRow(u8 differentEVs)
{
    if (differentEVs == 1 || differentEVs == 2)
        return 0;
    else
        return 1;
}

static u8* PrintMonStatsToggle_EV_Arrows(u8 *dest, u8 value)
{
    switch (value)
    {
        case 1:
            StringCopy(dest, sText_Stats_EV_Plus1);
            break;
        case 2:
            StringCopy(dest, sText_Stats_EV_Plus2);
            break;
        case 3:
            StringCopy(dest, sText_Stats_EV_Plus3);
            break;
    }
    return dest;
}

static void PrintStatsScreen_Left(u8 taskId)
{
    u8 base_x = 8;
    u8 x_offset_column = 43;
    u8 column = 0;
    u8 base_x_offset = 70;
    u8 base_x_first_row = 23;
    u8 base_x_second_row = 43;
    u8 base_y_offset = 11;
    u8 base_i = 0;
    u8 base_y = 5;
    u32 align_x;
    u8 total_x = 93;
    u8 strEV[25];
    u8 strBase[14];
    u8 EVs[6] = {sPokedexView->sPokemonStats.evYield_HP, sPokedexView->sPokemonStats.evYield_Speed, sPokedexView->sPokemonStats.evYield_Attack, sPokedexView->sPokemonStats.evYield_SpAttack, sPokedexView->sPokemonStats.evYield_Defense, sPokedexView->sPokemonStats.evYield_SpDefense};
    u8 differentEVs = 0;

    //Base stats
    if (gTasks[taskId].data[5] == 0)
    {
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, sText_Stats_HP, base_x, base_y + base_y_offset*base_i);
        ConvertIntToDecimalStringN(strBase, sPokedexView->sPokemonStats.baseHP, STR_CONV_MODE_RIGHT_ALIGN, 3);
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, strBase, base_x+base_x_first_row, base_y + base_y_offset*base_i);

        PrintStatsScreenTextSmall(WIN_STATS_LEFT, sText_Stats_Speed, base_x+base_x_second_row, base_y + base_y_offset*base_i);
        ConvertIntToDecimalStringN(strBase, sPokedexView->sPokemonStats.baseSpeed, STR_CONV_MODE_RIGHT_ALIGN, 3);
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, strBase, base_x+base_x_offset, base_y + base_y_offset*base_i);

        base_i++;
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, sText_Stats_Attack, base_x, base_y + base_y_offset*base_i);
        ConvertIntToDecimalStringN(strBase, sPokedexView->sPokemonStats.baseAttack, STR_CONV_MODE_RIGHT_ALIGN, 3);
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, strBase, base_x+base_x_first_row, base_y + base_y_offset*base_i);

        PrintStatsScreenTextSmall(WIN_STATS_LEFT, sText_Stats_SpAttack, base_x+base_x_second_row, base_y + base_y_offset*base_i);
        ConvertIntToDecimalStringN(strBase, sPokedexView->sPokemonStats.baseSpAttack, STR_CONV_MODE_RIGHT_ALIGN, 3);
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, strBase, base_x+base_x_offset, base_y + base_y_offset*base_i);

        base_i++;
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, sText_Stats_Defense, base_x, base_y + base_y_offset*base_i);
        ConvertIntToDecimalStringN(strBase, sPokedexView->sPokemonStats.baseDefense, STR_CONV_MODE_RIGHT_ALIGN, 3);
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, strBase, base_x+base_x_first_row, base_y + base_y_offset*base_i);

        PrintStatsScreenTextSmall(WIN_STATS_LEFT, sText_Stats_SpDefense, base_x+base_x_second_row, base_y + base_y_offset*base_i);
        ConvertIntToDecimalStringN(strBase, sPokedexView->sPokemonStats.baseSpDefense, STR_CONV_MODE_RIGHT_ALIGN, 3);
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, strBase, base_x+base_x_offset, base_y + base_y_offset*base_i);
        base_i++;
    }
    else //EV increases
    {
        //If 1 or 2 EVs display with the same layout as the base stats
        if (sPokedexView->sPokemonStats.differentEVs < 3)
        {
            differentEVs = 0;
            //HP
            if (EVs[0] > 0)
            {
                differentEVs++;
                column = PrintMonStatsToggle_DifferentEVsColumn(differentEVs);
                base_i = PrintMonStatsToggle_DifferentEVsRow(differentEVs);
                StringCopy(gStringVar1, sText_Stats_HP);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[0]);
                StringExpandPlaceholders(gStringVar3, sText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_LEFT, gStringVar3, base_x + x_offset_column*column, base_y + base_y_offset*base_i);
            }
            //Speed
            if (EVs[1]> 0)
            {
                differentEVs++;
                column = PrintMonStatsToggle_DifferentEVsColumn(differentEVs);
                base_i = PrintMonStatsToggle_DifferentEVsRow(differentEVs);
                StringCopy(gStringVar1, sText_Stats_Speed);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[1]);
                StringExpandPlaceholders(gStringVar3, sText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_LEFT, gStringVar3, base_x + x_offset_column*column, base_y + base_y_offset*base_i);
            }
            //Attack
            if (EVs[2] > 0)
            {
                differentEVs++;
                column = PrintMonStatsToggle_DifferentEVsColumn(differentEVs);
                base_i = PrintMonStatsToggle_DifferentEVsRow(differentEVs);
                StringCopy(gStringVar1, sText_Stats_Attack);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[2]);
                StringExpandPlaceholders(gStringVar3, sText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_LEFT, gStringVar3, base_x + x_offset_column*column, base_y + base_y_offset*base_i);
            }
            //Special Attack
            if (EVs[3] > 0)
            {
                differentEVs++;
                column = PrintMonStatsToggle_DifferentEVsColumn(differentEVs);
                base_i = PrintMonStatsToggle_DifferentEVsRow(differentEVs);
                StringCopy(gStringVar1, sText_Stats_SpAttack);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[3]);
                StringExpandPlaceholders(gStringVar3, sText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_LEFT, gStringVar3, base_x + x_offset_column*column, base_y + base_y_offset*base_i);
            }
            //Defense
            if (EVs[4] > 0)
            {
                differentEVs++;
                column = PrintMonStatsToggle_DifferentEVsColumn(differentEVs);
                base_i = PrintMonStatsToggle_DifferentEVsRow(differentEVs);
                StringCopy(gStringVar1, sText_Stats_Defense);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[4]);
                StringExpandPlaceholders(gStringVar3, sText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_LEFT, gStringVar3, base_x + x_offset_column*column, base_y + base_y_offset*base_i);
            }
            //Special Defense
            if (EVs[5] > 0)
            {
                differentEVs++;
                column = PrintMonStatsToggle_DifferentEVsColumn(differentEVs);
                base_i = PrintMonStatsToggle_DifferentEVsRow(differentEVs);
                StringCopy(gStringVar1, sText_Stats_SpDefense);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[5]);
                StringExpandPlaceholders(gStringVar3, sText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_LEFT, gStringVar3, base_x + x_offset_column*column, base_y + base_y_offset*base_i);
            }
        }
        else //3 different EVs in 1 row
        {
            column = 0;
            //HP
            if (EVs[0] > 0)
            {
                StringCopy(gStringVar1, sText_Stats_HP);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[0]);
                StringExpandPlaceholders(gStringVar3, sText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_LEFT, gStringVar3, base_x + 29*column, base_y + base_y_offset*base_i);
                column++;
            }
            //Speed
            if (EVs[1] > 0)
            {
                StringCopy(gStringVar1, sText_Stats_Speed);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[1]);
                StringExpandPlaceholders(gStringVar3, sText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_LEFT, gStringVar3, base_x + 29*column, base_y + base_y_offset*base_i);
                column++;
            }
            //Attack
            if (EVs[2] > 0)
            {
                StringCopy(gStringVar1, sText_Stats_Attack);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[2]);
                StringExpandPlaceholders(gStringVar3, sText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_LEFT, gStringVar3, base_x + 29*column, base_y + base_y_offset*base_i);
                column++;
            }
            //Special Attack
            if (EVs[3] > 0)
            {
                StringCopy(gStringVar1, sText_Stats_SpAttack);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[3]);
                StringExpandPlaceholders(gStringVar3, sText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_LEFT, gStringVar3, base_x + 29*column, base_y + base_y_offset*base_i);
                column++;
            }
            //Defense
            if (EVs[4] > 0)
            {
                StringCopy(gStringVar1, sText_Stats_Defense);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[4]);
                StringExpandPlaceholders(gStringVar3, sText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_LEFT, gStringVar3, base_x + 29*column, base_y + base_y_offset*base_i);
                column++;
            }
            //Special Defense
            if (EVs[5] > 0)
            {
                StringCopy(gStringVar1, sText_Stats_SpDefense);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[5]);
                StringExpandPlaceholders(gStringVar3, sText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_LEFT, gStringVar3, base_x + 29*column, base_y + base_y_offset*base_i);
                column++;
            }
        }
        base_i++;
    }

    //TOGGLE--------------------------------------
    if (gTasks[taskId].data[5] == 0)
    {
        u32 growthRate = sPokedexView->sPokemonStats.growthRate;
        u32 totalStats = sPokedexView->sPokemonStats.baseHP + sPokedexView->sPokemonStats.baseAttack + sPokedexView->sPokemonStats.baseDefense + sPokedexView->sPokemonStats.baseSpAttack +sPokedexView->sPokemonStats.baseSpDefense + sPokedexView->sPokemonStats.baseSpeed;

        //Total stats
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, sText_Stats_Total, base_x, base_y + base_y_offset*base_i);
        ConvertIntToDecimalStringN(strBase, totalStats, STR_CONV_MODE_RIGHT_ALIGN, 3);
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, strBase, base_x+base_x_offset, base_y + base_y_offset*base_i);
        base_i++;

        //Growth rate
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, sText_Stats_Growthrate, base_x, base_y + base_y_offset*base_i);
        switch (growthRate)
        {
        case GROWTH_MEDIUM_FAST:
            StringCopy(strEV, sText_Stats_MEDIUM_FAST);
            break;
        case GROWTH_ERRATIC:
            StringCopy(strEV, sText_Stats_ERRATIC);
            break;
        case GROWTH_FLUCTUATING:
            StringCopy(strEV, sText_Stats_FLUCTUATING);
            break;
        case GROWTH_MEDIUM_SLOW:
            StringCopy(strEV, sText_Stats_MEDIUM_SLOW);
            break;
        case GROWTH_FAST:
            StringCopy(strEV, sText_Stats_FAST);
            break;
        case GROWTH_SLOW:
            StringCopy(strEV, sText_Stats_SLOW);
            break;
        default:
            break;
        }
        align_x = GetStringRightAlignXOffset(0, strEV, total_x);
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, strEV, align_x, base_y + base_y_offset*base_i);
    }
    else
    {
        //Exp Yield
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, sText_Stats_ExpYield, base_x, base_y + base_y_offset*base_i);
        ConvertIntToDecimalStringN(gStringVar1, sPokedexView->sPokemonStats.expYield, STR_CONV_MODE_RIGHT_ALIGN, 3);
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, gStringVar1, base_x + base_x_offset, base_y + base_y_offset*base_i);
        base_i++;

        //Friendship
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, sText_Stats_Friendship, base_x, base_y + base_y_offset*base_i);
        switch (sPokedexView->sPokemonStats.friendship)
        {
        case 35:
            StringCopy(strEV, sText_Stats_Friendship_BigAnger);
            break;
        case 70:
            StringCopy(strEV, sText_Stats_Friendship_Neutral);
            break;
        case 90:
            StringCopy(strEV, sText_Stats_Friendship_Happy);
            break;
        case 100:
            StringCopy(strEV, sText_Stats_Friendship_Happy);
            break;
        case 140:
            StringCopy(strEV, sText_Stats_Friendship_BigSmile);
            break;
        default:
            ConvertIntToDecimalStringN(strEV, sPokedexView->sPokemonStats.friendship, STR_CONV_MODE_RIGHT_ALIGN, 3);
            break;
        }
        align_x = GetStringRightAlignXOffset(0, strEV, total_x);
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, strEV, align_x, base_y + base_y_offset*base_i);
        base_i++;

        //Egg cycles
        if (sPokedexView->sPokemonStats.eggGroup1 == EGG_GROUP_BABY || sPokedexView->sPokemonStats.eggGroup2 == EGG_GROUP_BABY) //Species without eggs (legendaries etc)
        {
            PrintStatsScreenTextSmall(WIN_STATS_LEFT, sText_Stats_EggCycles, base_x, base_y + base_y_offset*base_i);
            PrintStatsScreenTextSmall(WIN_STATS_LEFT, gText_ThreeDashes, 78, base_y + base_y_offset*base_i);
        }
        else
        {
            PrintStatsScreenTextSmall(WIN_STATS_LEFT, sText_Stats_EggCycles, base_x, base_y + base_y_offset*base_i);
            if (sPokedexView->sPokemonStats.eggCycles <= 10)
            {
                StringCopy(strEV, sText_Stats_EggCycles_VeryFast);
                align_x = 76;
            }
            else if (sPokedexView->sPokemonStats.eggCycles <= 20)
            {
                StringCopy(strEV, sText_Stats_EggCycles_Fast);
                align_x = 85;
            }
            else if (sPokedexView->sPokemonStats.eggCycles <= 30)
            {
                StringCopy(strEV, sText_Stats_EggCycles_Normal);
                align_x = 76;
            }
            else
            {
                StringCopy(strEV, sText_Stats_EggCycles_Slow);
                align_x = 67;
            }
            PrintStatsScreenTextSmall(WIN_STATS_LEFT, strEV, align_x, base_y + base_y_offset*base_i);
        }
        base_i++;

        //Egg group 1
        switch (sPokedexView->sPokemonStats.eggGroup1)
        {
        case EGG_GROUP_MONSTER     :
            StringCopy(gStringVar1, sText_Stats_eggGroup_MONSTER);
            break;
        case EGG_GROUP_ANFIBIO     :
            StringCopy(gStringVar1, sText_Stats_eggGroup_WATER_1);
            break;
        case EGG_GROUP_BUG         :
            StringCopy(gStringVar1, sText_Stats_eggGroup_BUG);
            break;
        case EGG_GROUP_FLYING      :
            StringCopy(gStringVar1, sText_Stats_eggGroup_FLYING);
            break;
        case EGG_GROUP_FIELD       :
            StringCopy(gStringVar1, sText_Stats_eggGroup_FIELD);
            break;
        case EGG_GROUP_FAIRY       :
            StringCopy(gStringVar1, sText_Stats_eggGroup_FAIRY);
            break;
        case EGG_GROUP_GRASS       :
            StringCopy(gStringVar1, sText_Stats_eggGroup_GRASS);
            break;
        case EGG_GROUP_HUMAN_LIKE  :
            StringCopy(gStringVar1, sText_Stats_eggGroup_HUMAN_LIKE);
            break;
        case EGG_GROUP_INVERTEBRADO     :
            StringCopy(gStringVar1, sText_Stats_eggGroup_WATER_3);
            break;
        case EGG_GROUP_MINERAL     :
            StringCopy(gStringVar1, sText_Stats_eggGroup_MINERAL);
            break;
        case EGG_GROUP_AMORPHOUS   :
            StringCopy(gStringVar1, sText_Stats_eggGroup_AMORPHOUS);
            break;
        case EGG_GROUP_PEZ     :
            StringCopy(gStringVar1, sText_Stats_eggGroup_WATER_2);
            break;
        case EGG_GROUP_DITTO       :
            StringCopy(gStringVar1, sText_Stats_eggGroup_DITTO);
            break;
        case EGG_GROUP_DRAGON      :
            StringCopy(gStringVar1, sText_Stats_eggGroup_DRAGON);
            break;
        case EGG_GROUP_BABY:
            StringCopy(gStringVar1, sText_Stats_eggGroup_NO_EGGS_DISCOVERED);
            break;
        }
        //Egg group 2
        if (sPokedexView->sPokemonStats.eggGroup1 != sPokedexView->sPokemonStats.eggGroup2)
        {
            switch (sPokedexView->sPokemonStats.eggGroup2)
            {
            case EGG_GROUP_MONSTER     :
                StringCopy(gStringVar2, sText_Stats_eggGroup_MONSTER);
                break;
            case EGG_GROUP_ANFIBIO     :
                StringCopy(gStringVar2, sText_Stats_eggGroup_WATER_1);
                break;
            case EGG_GROUP_BUG         :
                StringCopy(gStringVar2, sText_Stats_eggGroup_BUG);
                break;
            case EGG_GROUP_FLYING      :
                StringCopy(gStringVar2, sText_Stats_eggGroup_FLYING);
                break;
            case EGG_GROUP_FIELD       :
                StringCopy(gStringVar2, sText_Stats_eggGroup_FIELD);
                break;
            case EGG_GROUP_FAIRY       :
                StringCopy(gStringVar2, sText_Stats_eggGroup_FAIRY);
                break;
            case EGG_GROUP_GRASS       :
                StringCopy(gStringVar2, sText_Stats_eggGroup_GRASS);
                break;
            case EGG_GROUP_HUMAN_LIKE  :
                StringCopy(gStringVar2, sText_Stats_eggGroup_HUMAN_LIKE);
                break;
            case EGG_GROUP_INVERTEBRADO     :
                StringCopy(gStringVar2, sText_Stats_eggGroup_WATER_3);
                break;
            case EGG_GROUP_MINERAL     :
                StringCopy(gStringVar2, sText_Stats_eggGroup_MINERAL);
                break;
            case EGG_GROUP_AMORPHOUS   :
                StringCopy(gStringVar2, sText_Stats_eggGroup_AMORPHOUS);
                break;
            case EGG_GROUP_PEZ     :
                StringCopy(gStringVar2, sText_Stats_eggGroup_WATER_2);
                break;
            case EGG_GROUP_DITTO       :
                StringCopy(gStringVar2, sText_Stats_eggGroup_DITTO);
                break;
            case EGG_GROUP_DRAGON      :
                StringCopy(gStringVar2, sText_Stats_eggGroup_DRAGON);
                break;
            case EGG_GROUP_BABY:
                StringCopy(gStringVar2, sText_Stats_eggGroup_NO_EGGS_DISCOVERED);
                break;
            }
            StringExpandPlaceholders(gStringVar3, sText_Stats_eggGroup_Groups);
            align_x = GetStringRightAlignXOffset(0, gStringVar3, total_x);
            PrintStatsScreenTextSmall(WIN_STATS_LEFT, gStringVar3, base_x, base_y + base_y_offset*base_i);
        }
        else
        {
            align_x = GetStringRightAlignXOffset(0, gStringVar1, total_x);
            PrintStatsScreenTextSmall(WIN_STATS_LEFT, gStringVar1, base_x, base_y + base_y_offset*base_i);
        }
        base_i++;
    }
}

static void PrintStatsScreen_Abilities(u8 taskId)
{
    u8 abilities_x = 5;
    u8 abilities_y = 3;
    u16 ability0;
    u16 ability1;
    u16 abilityHidden;

    //Abilitie(s)

    if (gTasks[taskId].data[5] == 0)
    {
        ability0 = sPokedexView->sPokemonStats.ability0;
        PrintStatsScreenTextSmallWhite(WIN_STATS_ABILITIES, gAbilitiesInfo[ability0].name, abilities_x, abilities_y);
        PrintStatsScreenTextSmall(WIN_STATS_ABILITIES, gAbilitiesInfo[ability0].description, abilities_x, abilities_y + 14);

        ability1 = sPokedexView->sPokemonStats.ability1;
        if (ability1 != ABILITY_NONE && ability1 != ability0)
        {
            PrintStatsScreenTextSmallWhite(WIN_STATS_ABILITIES, gAbilitiesInfo[ability1].name, abilities_x, abilities_y + 30);
            PrintStatsScreenTextSmall(WIN_STATS_ABILITIES, gAbilitiesInfo[ability1].description, abilities_x, abilities_y + 56);
        }
    }
    else //Hidden abilities
    {
        abilityHidden = sPokedexView->sPokemonStats.abilityHidden;
        PrintStatsScreenTextSmallWhite(WIN_STATS_ABILITIES, gAbilitiesInfo[abilityHidden].name, abilities_x, abilities_y);
        PrintStatsScreenTextSmall(WIN_STATS_ABILITIES, gAbilitiesInfo[abilityHidden].description, abilities_x, abilities_y + 14);
    }
}

static void Task_SwitchScreensFromStatsScreen(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeSpriteTilesByTag(ITEM_TAG);                         //Destroy item icon
        FreeSpritePaletteByTag(ITEM_TAG);                       //Destroy item icon
        FreeSpriteOamMatrix(&gSprites[gTasks[taskId].data[3]]); //Destroy item icon
        DestroySprite(&gSprites[gTasks[taskId].data[3]]);       //Destroy item icon
        //FreeMonIconPalettes();                                          //Destroy pokemon icon sprite
        FreeAndDestroyMonIconSprite(&gSprites[gTasks[taskId].data[4]]); //Destroy pokemon icon sprite

        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);
        switch (sPokedexView->screenSwitchState)
        {
        case 1:
            FreeAllWindowBuffers();
            InitWindows(sInfoScreen_WindowTemplates);
            gTasks[taskId].func = Task_LoadAreaScreen;
            break;
        case 3:
            FreeAllWindowBuffers();
            InitWindows(sInfoScreen_WindowTemplates);
            gTasks[taskId].func = Task_LoadEvolutionScreen;
            break;
        default:
            gTasks[taskId].func = Task_LoadInfoScreen;
            break;
        }
    }
}

static void Task_ExitStatsScreen(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeSpriteTilesByTag(ITEM_TAG);                         //Destroy item icon
        FreeSpritePaletteByTag(ITEM_TAG);                       //Destroy item icon
        FreeSpriteOamMatrix(&gSprites[gTasks[taskId].data[3]]); //Destroy item icon
        DestroySprite(&gSprites[gTasks[taskId].data[3]]);       //Destroy item icon
        //FreeMonIconPalettes();                                          //Destroy pokemon icon sprite
        FreeAndDestroyMonIconSprite(&gSprites[gTasks[taskId].data[4]]); //Destroy pokemon icon sprite

        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);
        FreeInfoScreenWindowAndBgBuffers();
        DestroyTask(taskId);
    }
}


//************************************
//*                                  *
//*        EVOS                      *
//*                                  *
//************************************
static const u8 sEvoFormsPageNavigationTextColor[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY};

static void EvoFormsPage_PrintNavigationButtons(void)
{
    u8 x = 6;
    u8 y = 0;

    FillWindowPixelBuffer(WIN_NAVIGATION_BUTTONS, PIXEL_FILL(0));

        if (sPokedexView->selectedScreen == EVO_SCREEN)
        {
            if (!HGSS_DECAPPED)
                AddTextPrinterParameterized3(WIN_NAVIGATION_BUTTONS, 0, x+9, y, sStatsPageNavigationTextColor, 0, sText_EVO_Buttons_PE);
            else
                AddTextPrinterParameterized3(WIN_NAVIGATION_BUTTONS, 0, x+9, y, sStatsPageNavigationTextColor, 0, sText_EVO_Buttons_Decapped_PE);
        }
        else if (sPokedexView->selectedScreen == FORMS_SCREEN)
        {
            if (sPokedexView->sFormScreenData.inSubmenu)
            {
                if (!HGSS_DECAPPED)
                    AddTextPrinterParameterized3(WIN_NAVIGATION_BUTTONS, 0, x, y, sStatsPageNavigationTextColor, 0, sText_FORMS_Buttons_Submenu_PE);
                else
                    AddTextPrinterParameterized3(WIN_NAVIGATION_BUTTONS, 0, x, y, sStatsPageNavigationTextColor, 0, sText_FORMS_Buttons_Submenu_Decapped_PE);
            }
            else
            {
                if (!HGSS_DECAPPED)
                    AddTextPrinterParameterized3(WIN_NAVIGATION_BUTTONS, 0, x, y, sStatsPageNavigationTextColor, 0, sText_FORMS_Buttons_PE);
                else
                    AddTextPrinterParameterized3(WIN_NAVIGATION_BUTTONS, 0, x, y, sStatsPageNavigationTextColor, 0, sText_FORMS_Buttons_Decapped_PE);
            }
        }

    PutWindowTilemap(WIN_NAVIGATION_BUTTONS);
    CopyWindowToVram(WIN_NAVIGATION_BUTTONS, COPYWIN_FULL);
}

static void ResetEvoScreenDataStruct(void)
{
    u8 i;
    sPokedexView->sEvoScreenData.numAllEvolutions = 0;
    sPokedexView->sEvoScreenData.numSeen = 0;
    sPokedexView->sEvoScreenData.menuPos = 0;
    for (i = 0; i < 10; i++)
    {
        sPokedexView->sEvoScreenData.targetSpecies[i] = 0;
        sPokedexView->sEvoScreenData.seen[i] = 0;
    }

}

static void GetSeenFlagTargetSpecies(void)
{
    u8 i;
    u16 species;
    for (i = 0; i < sPokedexView->sEvoScreenData.numAllEvolutions; i++)
    {
        species = sPokedexView->sEvoScreenData.targetSpecies[i];
        if (GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_SEEN))
        {
            sPokedexView->sEvoScreenData.seen[i] = TRUE;
            sPokedexView->sEvoScreenData.numSeen += 1;
        }

    }
}

static void Task_LoadEvolutionScreen(u8 taskId)
{
    switch (gMain.state)
    {
    case 0:
    default:
        if (!gPaletteFade.active)
        {
            u16 r2;

            sPokedexView->currentPage = EVO_SCREEN;
            gPokedexVBlankCB = gMain.vblankCallback;
            SetVBlankCallback(NULL);
            r2 = 0;
            if (gTasks[taskId].data[1] != 0)
                r2 += DISPCNT_OBJ_ON;
            if (gTasks[taskId].data[2] != 0)
                r2 |= DISPCNT_BG1_ON;
            ResetOtherVideoRegisters(r2);
            gMain.state = 1;
        }
        break;
    case 1:
        LoadTilesetTilemapHGSS(EVO_SCREEN);
        FillWindowPixelBuffer(WIN_INFO, PIXEL_FILL(0));
        PutWindowTilemap(WIN_INFO);
        CopyWindowToVram(WIN_INFO, COPYWIN_FULL);
        FillWindowPixelBuffer(WIN_NAVIGATION_BUTTONS, PIXEL_FILL(0));
        PutWindowTilemap(WIN_NAVIGATION_BUTTONS);
        CopyWindowToVram(WIN_NAVIGATION_BUTTONS, COPYWIN_FULL);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(3);
        gMain.state++;
        break;
    case 2:
        LoadScreenSelectBarMain();
        LoadPokedexBgPalette();
        gMain.state++;
        break;
    case 3: //icono de Pokémon desde el que hemos entrado a sus stats
        if (gTasks[taskId].data[1] == 0)
        {
            sPokedexView->selectedScreen = EVO_SCREEN;
            ResetEvoScreenDataStruct();
            //Icon
            //FreeMonIconPalettes(); //Free space for new pallete
            LoadCompressedPalette(GetMonSpritePalFromSpeciesAndPersonality(NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum), 0, 0), OBJ_PLTT_ID(4), PLTT_SIZE_4BPP);
            PrintPreEvolutions(taskId, NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum));
            gTasks[taskId].data[4] = CreateMonIconNoPersonality(NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum), SpriteCB_MonIcon, 18 + 32*sPokedexView->numPreEvolutions, 31, 4); //Create pokemon sprite
            EvoFormsPage_PrintNavigationButtons(); // Navigation buttons
            gSprites[gTasks[taskId].data[4]].oam.priority = 0;
            gSprites[gTasks[taskId].data[4]].oam.paletteNum = 4;
        }
        gMain.state++;
        break;
    case 4:
        //Print evo info and icons
        gTasks[taskId].data[3] = 0;
        PrintEvolutionTargetSpeciesAndMethod(taskId, NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum), 0, sPokedexView->numPreEvolutions);
        LoadSpritePalette(&gSpritePalette_Arrow);
        GetSeenFlagTargetSpecies();
        if (sPokedexView->sEvoScreenData.numAllEvolutions != 0 && sPokedexView->sEvoScreenData.numSeen != 0)
        {
            sPokedexView->sEvoScreenData.arrowSpriteId = CreateSprite(&gSpriteTemplate_Arrow, 7, 58, 0);
            gSprites[sPokedexView->sEvoScreenData.arrowSpriteId].animNum = 2;
        }
        gMain.state++;
        break;
    case 5:
        {
        u32 preservedPalettes = 0;

        if (gTasks[taskId].data[2] != 0)
            preservedPalettes = 0x14; // each bit represents a palette index
        if (gTasks[taskId].data[1] != 0)
            preservedPalettes |= (1 << (gSprites[gTasks[taskId].tMonSpriteId].oam.paletteNum + 16));
        BeginNormalPaletteFade(~preservedPalettes, 0, 16, 0, RGB_BLACK);
        SetVBlankCallback(gPokedexVBlankCB);
        gMain.state++;
        }
        break;
    case 6:
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
        HideBg(0);
        ShowBg(1);
        ShowBg(2);
        ShowBg(3);
        gMain.state++;
        break;
    case 7:
        if (!gPaletteFade.active)
            gMain.state++;
        break;
    case 8:
        gMain.state++;
        break;
    case 9:
        sPokedexView->screenSwitchState = 0;
        gTasks[taskId].data[0] = 0;
        gTasks[taskId].data[1] = 0;
        gTasks[taskId].data[2] = 1;
        gTasks[taskId].func = Task_HandleEvolutionScreenInput;
        gMain.state = 0;
        break;
    }
}

static void Task_HandleEvolutionScreenInput(u8 taskId)
{
    //Switch to forms screen, Pokemon Expansion only (rhh)
    if (JOY_NEW(START_BUTTON))
    {
        sPokedexView->selectedScreen = FORMS_SCREEN;
        BeginNormalPaletteFade(0xFFFFFFEB, 0, 0, 0x10, RGB_BLACK);
        sPokedexView->screenSwitchState = 3;
        gTasks[taskId].func = Task_SwitchScreensFromEvolutionScreen;
        PlaySE(SE_PIN);
    }

    if (sPokedexView->sEvoScreenData.numAllEvolutions != 0 && sPokedexView->sEvoScreenData.numSeen != 0)
    {
        u8 base_y = 58;
        u8 base_y_offset = 9;
        u8 pos = sPokedexView->sEvoScreenData.menuPos;
        u8 max = sPokedexView->sEvoScreenData.numAllEvolutions;
        if (JOY_NEW(DPAD_DOWN))
        {
            while (TRUE)
            {
                pos += 1;
                if (pos >= max)
                    pos = 0;

                if (sPokedexView->sEvoScreenData.seen[pos] == TRUE)
                    break;
            }
            gSprites[sPokedexView->sEvoScreenData.arrowSpriteId].y = base_y + base_y_offset * pos;
            sPokedexView->sEvoScreenData.menuPos = pos;
        }
        else if (JOY_NEW(DPAD_UP))
        {
            if (sPokedexView->sEvoScreenData.menuPos == 0)
                sPokedexView->sEvoScreenData.menuPos = sPokedexView->sEvoScreenData.numAllEvolutions - 1;
            else
                sPokedexView->sEvoScreenData.menuPos -= 1;

            gSprites[sPokedexView->sEvoScreenData.arrowSpriteId].y = base_y + base_y_offset * sPokedexView->sEvoScreenData.menuPos;
        }

        if (JOY_NEW(A_BUTTON))
        {
            u16 targetSpecies   = sPokedexView->sEvoScreenData.targetSpecies[sPokedexView->sEvoScreenData.menuPos];
            u16 dexNum          = SpeciesToNationalPokedexNum(targetSpecies);
            sPokedexListItem->dexNum = dexNum;
            sPokedexListItem->seen   = GetSetPokedexFlag(dexNum, FLAG_GET_SEEN);
            sPokedexListItem->owned  = GetSetPokedexFlag(dexNum, FLAG_GET_CAUGHT);

                if (GetSpeciesFormTable(targetSpecies) != NULL)
                    sPokedexView->formSpecies = targetSpecies;
                else
                    sPokedexView->formSpecies = 0;

            sPokedexView->sEvoScreenData.fromEvoPage = TRUE;
            PlaySE(SE_PIN);
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            gTasks[taskId].func = Task_LoadInfoScreenWaitForFade;
        }
    }

    //Exit to overview
    if (JOY_NEW(B_BUTTON))
    {
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_ExitEvolutionScreen;
        PlaySE(SE_PC_OFF);
        return;
    }

    //Switch screens
    if ((JOY_NEW(DPAD_LEFT) || (JOY_NEW(L_BUTTON) && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_LR)))
    {
        sPokedexView->selectedScreen = STATS_SCREEN;
        BeginNormalPaletteFade(0xFFFFFFEB, 0, 0, 0x10, RGB_BLACK);
        sPokedexView->screenSwitchState = 1;
        gTasks[taskId].func = Task_SwitchScreensFromEvolutionScreen;
        PlaySE(SE_PIN);
    }
}

static void HandleTargetSpeciesPrint(u8 taskId, u16 targetSpecies, u16 previousTargetSpecies, u8 base_x, u8 base_y, u8 base_y_offset, u8 base_i, bool8 isEevee)
{
    u8 iterations = 6;
    bool8 seen = GetSetPokedexFlag(SpeciesToNationalPokedexNum(targetSpecies), FLAG_GET_SEEN);

    if (seen || !HGSS_HIDE_UNSEEN_EVOLUTION_NAMES)
        StringCopy(gStringVar3, GetSpeciesName(targetSpecies)); //evolution mon name
    else
        StringCopy(gStringVar3, gText_ThreeQuestionMarks); //show questionmarks instead of name
    StringExpandPlaceholders(gStringVar3, sText_EVO_Name); //evolution mon name
    PrintInfoScreenTextSmall(gStringVar3, base_x, base_y + base_y_offset*base_i); //evolution mon name

    //Print mon icon in the top row
    if (isEevee)
    {
        iterations = 9;
        if (targetSpecies == previousTargetSpecies)
            return;
            else if (targetSpecies == SPECIES_GLACEON)
                base_i -= 1;
            else if (targetSpecies == SPECIES_SYLVEON)
                base_i -= 2;
    }

    if (base_i < iterations)
    {
        LoadCompressedPalette(GetMonSpritePalFromSpeciesAndPersonality(targetSpecies, 0, 0), OBJ_PLTT_ID(5 + base_i), PLTT_SIZE_4BPP);
            if (isEevee)
                gTasks[taskId].data[5 + base_i] = CreateMonIconNoPersonality(targetSpecies, SpriteCB_MonIcon, 45 + 26*base_i, 31, 4); //Create pokemon sprite
            else
                gTasks[taskId].data[5 + base_i] = CreateMonIconNoPersonality(targetSpecies, SpriteCB_MonIcon, 50 + 32*base_i, 31, 4); //Create pokemon sprite
        gSprites[gTasks[taskId].data[5 + base_i]].oam.priority = 0;
        gSprites[gTasks[taskId].data[5 + base_i]].oam.paletteNum = 5 + base_i;
    }
}

static void CreateCaughtBallEvolutionScreen(u16 targetSpecies, u8 x, u8 y)
{
    bool8 owned = GetSetPokedexFlag(SpeciesToNationalPokedexNum(targetSpecies), FLAG_GET_CAUGHT);
    if (owned)
        BlitBitmapToWindow(0, sCaughtBall_Gfx, x, y-1, 8, 16);
    else
    {
        PrintInfoScreenTextSmall(gText_OneDash, x+1, y-1);
    }
}

static void HandlePreEvolutionSpeciesPrint(u8 taskId, u16 preSpecies, u16 species, u8 base_x, u8 base_y, u8 base_y_offset, u8 base_i)
{
    bool8 seen = GetSetPokedexFlag(SpeciesToNationalPokedexNum(preSpecies), FLAG_GET_SEEN);

    StringCopy(gStringVar1, GetSpeciesName(species)); //evolution mon name

    if (sPokedexView->sEvoScreenData.isMega)
        StringExpandPlaceholders(gStringVar3, sText_EVO_PreEvo_PE_Mega);
    else
    {

        if (seen || !HGSS_HIDE_UNSEEN_EVOLUTION_NAMES)
            StringCopy(gStringVar2, GetSpeciesName(preSpecies)); //evolution mon name
        else
            StringCopy(gStringVar2, gText_ThreeQuestionMarks); //show questionmarks instead of name

        StringExpandPlaceholders(gStringVar3, sText_EVO_PreEvo); //evolution mon name

    }

    PrintInfoScreenTextSmall(gStringVar3, base_x, base_y + base_y_offset*base_i); //evolution mon name

    if (base_i < 3)
    {
        LoadCompressedPalette(GetMonSpritePalFromSpeciesAndPersonality(preSpecies, 0, 0), OBJ_PLTT_ID(5 + base_i), PLTT_SIZE_4BPP);
        gTasks[taskId].data[5 + base_i] = CreateMonIconNoPersonality(preSpecies, SpriteCB_MonIcon, 18 + 32*base_i, 31, 4); //Create pokemon sprite
        gSprites[gTasks[taskId].data[5 + base_i]].oam.priority = 0;
        gSprites[gTasks[taskId].data[5 + base_i]].oam.paletteNum = 5 + base_i;
    }
}

static u8 PrintPreEvolutions(u8 taskId, u16 species)
{
    u16 i;
    u16 j;

    u8 base_x = 13+8;
    u8 base_y = 51;
    u8 base_y_offset = 9;

    u16 preEvolutionOne = 0;
    u16 preEvolutionTwo = 0;
    u8 numPreEvolutions = 0;

    u16 baseFormSpecies;
    sPokedexView->sEvoScreenData.isMega = FALSE;

    //Check if it's a mega
    baseFormSpecies = GetFormSpeciesId(species, 0);
    if (baseFormSpecies != species)
    {
        const struct FormChange *formChanges = GetSpeciesFormChanges(baseFormSpecies);
        for (i = 0; formChanges != NULL && formChanges[i].method != FORM_CHANGE_TERMINATOR; i++)
        {
            if (formChanges[i].method == FORM_CHANGE_BATTLE_MEGA_EVOLUTION_ITEM
                && formChanges[i].targetSpecies == species)
            {
                preEvolutionOne = baseFormSpecies;
                numPreEvolutions += 1;
                sPokedexView->numPreEvolutions = numPreEvolutions;
                sPokedexView->sEvoScreenData.numAllEvolutions += numPreEvolutions;
                sPokedexView->sEvoScreenData.isMega = TRUE;

                CopyItemName(GetSpeciesFormChanges(species)->param1, gStringVar2); //item
                CreateCaughtBallEvolutionScreen(preEvolutionOne, base_x - 9 - 8, base_y + base_y_offset*(numPreEvolutions - 1));
                HandlePreEvolutionSpeciesPrint(taskId, preEvolutionOne, species, base_x - 8, base_y, base_y_offset, numPreEvolutions - 1);
                return numPreEvolutions;
            }
        }
    }

    //Calculate previous evolution
    for (i = 0; i < NUM_SPECIES; i++)
    {
        const struct Evolution *evolutions = GetSpeciesEvolutions(i);
        if (evolutions == NULL)
            continue;

        for (j = 0; evolutions[j].method != EVOLUTIONS_END; j++)
        {
            if (evolutions[j].targetSpecies == species)
            {
                preEvolutionOne = i;
                numPreEvolutions += 1;
                break;
            }
        }
    }

    //Calculate if previous evolution also has a previous evolution
    if (numPreEvolutions != 0)
    {
        for (i = 0; i < NUM_SPECIES; i++)
        {
            const struct Evolution *evolutions = GetSpeciesEvolutions(i);
            if (evolutions == NULL)
                continue;

            for (j = 0; evolutions[j].method != EVOLUTIONS_END; j++)
            {
                if (evolutions[j].targetSpecies == preEvolutionOne)
                {
                    preEvolutionTwo = i;
                    numPreEvolutions += 1;
                    CreateCaughtBallEvolutionScreen(preEvolutionTwo, base_x - 9, base_y + base_y_offset*0);
                    HandlePreEvolutionSpeciesPrint(taskId, preEvolutionTwo, preEvolutionOne, base_x, base_y, base_y_offset, 0);
                    break;
                }
            }
        }
    }

    //Print ball and name
    if (preEvolutionOne != 0)
    {
        CreateCaughtBallEvolutionScreen(preEvolutionOne, base_x - 9, base_y + base_y_offset*(numPreEvolutions - 1));
        HandlePreEvolutionSpeciesPrint(taskId, preEvolutionOne, species, base_x, base_y, base_y_offset, numPreEvolutions - 1);
    }

    if (preEvolutionTwo != 0)
    {
        sPokedexView->sEvoScreenData.targetSpecies[0] = preEvolutionTwo;
        sPokedexView->sEvoScreenData.targetSpecies[1] = preEvolutionOne;
    }
    else if (preEvolutionOne != 0)
    {
        sPokedexView->sEvoScreenData.targetSpecies[0] = preEvolutionOne;
    }

    //vertical line
    //FillWindowPixelRect(0, PIXEL_FILL(5), 33 + 32*numPreEvolutions, 18, 1, 32); //PIXEL_FILL(15) for black

    sPokedexView->numPreEvolutions = numPreEvolutions;
    sPokedexView->sEvoScreenData.numAllEvolutions += numPreEvolutions;

    return numPreEvolutions;
}

#define EVO_SCREEN_LVL_DIGITS 2
#define EVO_SCREEN_CRITS_DIGITS 1
#define EVO_SCREEN_DMG_DIGITS 2

static u8 PrintEvolutionTargetSpeciesAndMethod(u8 taskId, u16 species, u8 depth, u8 depth_i)
{
    u16 i;
    const struct MapHeader *mapHeader;
    u16 targetSpecies = 0;
    u16 previousTargetSpecies = 0;

    u16 item;

    bool8 left = TRUE;
    u8 base_x = 13+8;
    u8 base_x_offset = 54+8;
    u8 base_y = 51;
    u8 base_y_offset = 9;
    u8 base_i = 0;
    u8 times = 0;
    u8 depth_x = 16;
    bool8 isEevee = FALSE;
    const struct Evolution *evolutions = GetSpeciesEvolutions(species);

    if (sPokedexView->sEvoScreenData.isMega)
        return 0;
    if (evolutions == NULL)
        return 0;

    StringCopy(gStringVar1, GetSpeciesName(species));

    if (species == SPECIES_EEVEE)
        isEevee = TRUE;

    //Calculate number of possible direct evolutions (e.g. Eevee has 5 but torchic has 1)
    for (i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
    {
        if (evolutions[i].method != 0)
            times += 1;
    }
    gTasks[taskId].data[3] = times;
    sPokedexView->sEvoScreenData.numAllEvolutions += times;

    //If there are no evolutions print text
    if (times == 0 && depth == 0)
    {
        StringExpandPlaceholders(gStringVar4, sText_EVO_NONE);
        PrintInfoScreenTextSmall(gStringVar4, base_x-7-7, base_y + base_y_offset*depth_i);
    }

    //If there are evolutions find out which and print them 1 by 1
    for (i = 0; i < times; i++)
    {
        base_i = i + depth_i;
        left = !left;

        previousTargetSpecies = targetSpecies;
        targetSpecies = evolutions[i].targetSpecies;
        sPokedexView->sEvoScreenData.targetSpecies[base_i] = targetSpecies;
        CreateCaughtBallEvolutionScreen(targetSpecies, base_x + depth_x*depth-9, base_y + base_y_offset*base_i);
        HandleTargetSpeciesPrint(taskId, targetSpecies, previousTargetSpecies, base_x + depth_x*depth, base_y, base_y_offset, base_i, isEevee); //evolution mon name

        switch (evolutions[i].method)
        {
        case EVO_FRIENDSHIP:
            ConvertIntToDecimalStringN(gStringVar2, 220, STR_CONV_MODE_LEADING_ZEROS, 3); //friendship value
            StringExpandPlaceholders(gStringVar4, sText_EVO_FRIENDSHIP );
            break;
        case EVO_FRIENDSHIP_DAY:
            StringExpandPlaceholders(gStringVar4, sText_EVO_FRIENDSHIP_DAY );
            break;
        case EVO_FRIENDSHIP_NIGHT:
            StringExpandPlaceholders(gStringVar4, sText_EVO_FRIENDSHIP_NIGHT );
            break;
        case EVO_LEVEL:
            ConvertIntToDecimalStringN(gStringVar2, evolutions[i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, sText_EVO_LEVEL );
            break;
        case EVO_TRADE:
            StringExpandPlaceholders(gStringVar4, sText_EVO_TRADE );
            break;
        case EVO_TRADE_ITEM:
            item = evolutions[i].param; //item
            CopyItemName(item, gStringVar2); //item
            StringExpandPlaceholders(gStringVar4, sText_EVO_TRADE_ITEM );
            break;
        case EVO_ITEM:
            item = evolutions[i].param;
            CopyItemName(item, gStringVar2);
            StringExpandPlaceholders(gStringVar4, sText_EVO_ITEM );
            break;
        case EVO_LEVEL_ATK_GT_DEF:
            ConvertIntToDecimalStringN(gStringVar2, evolutions[i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, sText_EVO_LEVEL_ATK_GT_DEF );
            break;
        case EVO_LEVEL_ATK_EQ_DEF:
            ConvertIntToDecimalStringN(gStringVar2, evolutions[i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, sText_EVO_LEVEL_ATK_EQ_DEF );
            break;
        case EVO_LEVEL_ATK_LT_DEF:
            ConvertIntToDecimalStringN(gStringVar2, evolutions[i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, sText_EVO_LEVEL_ATK_LT_DEF );
            break;
        case EVO_LEVEL_SILCOON:
            ConvertIntToDecimalStringN(gStringVar2, evolutions[i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, sText_EVO_LEVEL_SILCOON );
            break;
        case EVO_LEVEL_CASCOON:
            ConvertIntToDecimalStringN(gStringVar2, evolutions[i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, sText_EVO_LEVEL_CASCOON );
            break;
        case EVO_LEVEL_NINJASK:
            ConvertIntToDecimalStringN(gStringVar2, evolutions[i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, sText_EVO_LEVEL_NINJASK );
            break;
        case EVO_LEVEL_SHEDINJA:
            ConvertIntToDecimalStringN(gStringVar2, evolutions[i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, sText_EVO_LEVEL_SHEDINJA );
            break;
        case EVO_BEAUTY:
            ConvertIntToDecimalStringN(gStringVar2, evolutions[i].param, STR_CONV_MODE_LEADING_ZEROS, 3); //beauty
            StringExpandPlaceholders(gStringVar4, sText_EVO_BEAUTY );
            break;
        case EVO_LEVEL_FEMALE:
            ConvertIntToDecimalStringN(gStringVar2, evolutions[i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, sText_EVO_LEVEL_FEMALE );
            break;
        case EVO_LEVEL_MALE:
            ConvertIntToDecimalStringN(gStringVar2, evolutions[i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, sText_EVO_LEVEL_MALE );
            break;
        case EVO_LEVEL_NIGHT:
            ConvertIntToDecimalStringN(gStringVar2, evolutions[i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, sText_EVO_LEVEL_NIGHT );
            break;
        case EVO_LEVEL_DAY:
            ConvertIntToDecimalStringN(gStringVar2, evolutions[i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, sText_EVO_LEVEL_DAY );
            break;
        case EVO_LEVEL_DUSK:
            ConvertIntToDecimalStringN(gStringVar2, evolutions[i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, sText_EVO_LEVEL_DUSK );
            break;
        case EVO_ITEM_HOLD_DAY:
            item = evolutions[i].param; //item
            CopyItemName(item, gStringVar2); //item
            StringExpandPlaceholders(gStringVar4, sText_EVO_ITEM_HOLD_DAY );
            break;
        case EVO_ITEM_HOLD_NIGHT:
            item = evolutions[i].param; //item
            CopyItemName(item, gStringVar2); //item
            StringExpandPlaceholders(gStringVar4, sText_EVO_ITEM_HOLD_NIGHT );
            break;
        case EVO_MOVE:
            StringCopy(gStringVar2, GetMoveName(evolutions[i].param));
            StringExpandPlaceholders(gStringVar4, sText_EVO_MOVE );
            break;
        case EVO_FRIENDSHIP_MOVE_TYPE:
            StringCopy(gStringVar2, gTypesInfo[evolutions[i].param].name);
            StringExpandPlaceholders(gStringVar4, sText_EVO_FRIENDSHIP_MOVE_TYPE );
            break;
        case EVO_MAPSEC:
            StringCopy(gStringVar2, gRegionMapEntries[evolutions[i].param].name);
            StringExpandPlaceholders(gStringVar4, sText_EVO_MAPSEC );
            break;
        case EVO_ITEM_MALE:
            item = evolutions[i].param; //item
            CopyItemName(item, gStringVar2); //item
            StringExpandPlaceholders(gStringVar4, sText_EVO_ITEM_MALE );
            break;
        case EVO_ITEM_FEMALE:
            item = evolutions[i].param; //item
            CopyItemName(item, gStringVar2); //item
            StringExpandPlaceholders(gStringVar4, sText_EVO_ITEM_FEMALE );
            break;
        case EVO_LEVEL_RAIN:
            //if (j == WEATHER_RAIN || j == WEATHER_RAIN_THUNDERSTORM || j == WEATHER_DOWNPOUR)
            StringExpandPlaceholders(gStringVar4, sText_EVO_LEVEL_RAIN );
            break;
        case EVO_SPECIFIC_MON_IN_PARTY:
            StringCopy(gStringVar2, GetSpeciesName(evolutions[i].param)); //mon name
            StringExpandPlaceholders(gStringVar4, sText_EVO_SPECIFIC_MON_IN_PARTY );
            break;
        case EVO_LEVEL_DARK_TYPE_MON_IN_PARTY:
            StringExpandPlaceholders(gStringVar4, sText_EVO_LEVEL_DARK_TYPE_MON_IN_PARTY );
            break;
        case EVO_TRADE_SPECIFIC_MON:
            StringCopy(gStringVar2, GetSpeciesName(evolutions[i].param)); //mon name
            StringExpandPlaceholders(gStringVar4, sText_EVO_TRADE_SPECIFIC_MON );
            break;
        case EVO_SPECIFIC_MAP:
            mapHeader = Overworld_GetMapHeaderByGroupAndId(evolutions[i].param >> 8, evolutions[i].param & 0xFF);
            GetMapName(gStringVar2, mapHeader->regionMapSectionId, 0);
            StringExpandPlaceholders(gStringVar4, sText_EVO_SPECIFIC_MAP );
            break;
        case EVO_LEVEL_NATURE_AMPED:
            ConvertIntToDecimalStringN(gStringVar2, evolutions[i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, sText_EVO_LEVEL_NATURE_AMPED);
            break;
        case EVO_LEVEL_NATURE_LOW_KEY:
            ConvertIntToDecimalStringN(gStringVar2, evolutions[i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, sText_EVO_LEVEL_NATURE_LOW_KEY);
            break;
        case EVO_CRITICAL_HITS:
            ConvertIntToDecimalStringN(gStringVar2, evolutions[i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_CRITS_DIGITS); //crits
            StringExpandPlaceholders(gStringVar4, sText_EVO_CRITICAL_HITS);
            break;
        case EVO_SCRIPT_TRIGGER_DMG:
            ConvertIntToDecimalStringN(gStringVar2, evolutions[i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_DMG_DIGITS); //damage
            StringExpandPlaceholders(gStringVar4, sText_EVO_SCRIPT_TRIGGER_DMG);
            break;
        case EVO_DARK_SCROLL:
            item = evolutions[i].param;
            CopyItemName(item, gStringVar2);
            StringExpandPlaceholders(gStringVar4, sText_EVO_DARK_SCROLL );
            break;
        case EVO_WATER_SCROLL:
            item = evolutions[i].param;
            CopyItemName(item, gStringVar2);
            StringExpandPlaceholders(gStringVar4, sText_EVO_WATER_SCROLL );
            break;
        case EVO_ITEM_NIGHT:
            item = evolutions[i].param;
            CopyItemName(item, gStringVar2);
            StringExpandPlaceholders(gStringVar4, sText_EVO_ITEM_NIGHT );
            break;
        case EVO_ITEM_DAY:
            item = evolutions[i].param;
            CopyItemName(item, gStringVar2);
            StringExpandPlaceholders(gStringVar4, sText_EVO_ITEM_DAY );
            break;
        case EVO_ITEM_HOLD:
            item = evolutions[i].param;
            CopyItemName(item, gStringVar2);
            StringExpandPlaceholders(gStringVar4, sText_EVO_ITEM_HOLD );
            break;
        case EVO_USE_MOVE_TWENTY_TIMES:
            StringCopy(gStringVar2, GetMoveName(evolutions[i].param));
            StringExpandPlaceholders(gStringVar4, sText_EVO_USE_MOVE_TWENTY_TIMES );
            break;
        case EVO_RECOIL_DAMAGE_MALE:
            ConvertIntToDecimalStringN(gStringVar2, evolutions[i].param, STR_CONV_MODE_LEADING_ZEROS, 3);
            StringExpandPlaceholders(gStringVar4, sText_EVO_RECOIL_DAMAGE_MALE);
            break;
        case EVO_RECOIL_DAMAGE_FEMALE:
            ConvertIntToDecimalStringN(gStringVar2, evolutions[i].param, STR_CONV_MODE_LEADING_ZEROS, 3);
            StringExpandPlaceholders(gStringVar4, sText_EVO_RECOIL_DAMAGE_FEMALE);
            break;
        case EVO_ITEM_COUNT_999:
            item = evolutions[i].param;
            CopyItemName(item, gStringVar2);
            StringExpandPlaceholders(gStringVar4, sText_EVO_ITEM_COUNT_999);
            break;
        default:
            StringExpandPlaceholders(gStringVar4, sText_EVO_UNKNOWN );
            break;
        }//Switch end
        PrintInfoScreenTextSmall(gStringVar4, base_x + depth_x*depth+base_x_offset, base_y + base_y_offset*base_i); //Print actual instructions

        depth_i += PrintEvolutionTargetSpeciesAndMethod(taskId, targetSpecies, depth+1, base_i+1);
    }//For loop end

    return times;
}

static void Task_SwitchScreensFromEvolutionScreen(u8 taskId)
{
    u8 i;
    if (!gPaletteFade.active)
    {
        //FreeMonIconPalettes();                                          //Destroy pokemon icon sprite
        FreeAndDestroyMonIconSprite(&gSprites[gTasks[taskId].data[4]]); //Destroy pokemon icon sprite
        for (i = 1; i <= gTasks[taskId].data[3]; i++)
        {
            FreeAndDestroyMonIconSprite(&gSprites[gTasks[taskId].data[4+i]]); //Destroy pokemon icon sprite
        }
        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);

        switch (sPokedexView->screenSwitchState)
        {
        case 1:
            gTasks[taskId].func = Task_LoadStatsScreen;
            break;
        case 3:
                gTasks[taskId].func = Task_LoadFormsScreen;
                break;
        default:
            gTasks[taskId].func = Task_LoadInfoScreen;
            break;
        }
    }
}

static void Task_ExitEvolutionScreen(u8 taskId)
{
    u8 i;
    if (!gPaletteFade.active)
    {
        //FreeMonIconPalettes();                                          //Destroy pokemon icon sprite
        FreeAndDestroyMonIconSprite(&gSprites[gTasks[taskId].data[4]]); //Destroy pokemon icon sprite
        for (i = 1; i <= gTasks[taskId].data[3]; i++)
        {
            FreeAndDestroyMonIconSprite(&gSprites[gTasks[taskId].data[4+i]]); //Destroy pokemon icon sprite
        }
        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);

        FreeInfoScreenWindowAndBgBuffers();
        DestroyTask(taskId);
    }
}


//************************************
//*                                  *
//*        FORMS                      *
//*                                  *
//************************************

static void Task_LoadFormsScreen(u8 taskId)
{
    switch (gMain.state)
    {
    case 0:
    default:
        if (!gPaletteFade.active)
        {
            u16 r2;

            sPokedexView->currentPage = FORMS_SCREEN;
            gPokedexVBlankCB = gMain.vblankCallback;
            SetVBlankCallback(NULL);
            r2 = 0;
            if (gTasks[taskId].data[1] != 0)
                r2 += DISPCNT_OBJ_ON;
            if (gTasks[taskId].data[2] != 0)
                r2 |= DISPCNT_BG1_ON;
            ResetOtherVideoRegisters(r2);
            gMain.state = 1;
        }
        break;
    case 1:
        LoadTilesetTilemapHGSS(FORMS_SCREEN);
        FillWindowPixelBuffer(WIN_INFO, PIXEL_FILL(0));
        PutWindowTilemap(WIN_INFO);
        CopyWindowToVram(WIN_INFO, COPYWIN_FULL);
        FillWindowPixelBuffer(WIN_NAVIGATION_BUTTONS, PIXEL_FILL(0));
        PutWindowTilemap(WIN_NAVIGATION_BUTTONS);
        CopyWindowToVram(WIN_NAVIGATION_BUTTONS, COPYWIN_FULL);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(3);
        gMain.state++;
        break;
    case 2:
        LoadScreenSelectBarMain();
        LoadPokedexBgPalette();
        gMain.state++;
        break;
    case 3:
        if (gTasks[taskId].data[1] == 0)
        {
            //Icon
            //FreeMonIconPalettes(); //Free space for new pallete
            LoadCompressedPalette(GetMonSpritePalFromSpeciesAndPersonality(NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum), 0, 0), OBJ_PLTT_ID(4), PLTT_SIZE_4BPP);
            gTasks[taskId].data[4] = CreateMonIconNoPersonality(NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum), SpriteCB_MonIcon, 18, 31, 4); //Create pokemon sprite
            gSprites[gTasks[taskId].data[4]].oam.priority = 0;
            gSprites[gTasks[taskId].data[4]].oam.paletteNum = 4;
        }
        EvoFormsPage_PrintNavigationButtons(); // Navigation buttons
        sPokedexView->sFormScreenData.menuPos = 1;
        gMain.state++;
        break;
    case 4:
        //Print form icons
        gTasks[taskId].data[3] = 0;
        PrintForms(taskId, NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum));
        LoadSpritePalette(&gSpritePalette_Arrow);
        gMain.state++;
        break;
    case 5:
        {
        u32 preservedPalettes = 0;

        if (gTasks[taskId].data[2] != 0)
            preservedPalettes = 0x14; // each bit represents a palette index
        if (gTasks[taskId].data[1] != 0)
            preservedPalettes |= (1 << (gSprites[gTasks[taskId].tMonSpriteId].oam.paletteNum + 16));
        BeginNormalPaletteFade(~preservedPalettes, 0, 16, 0, RGB_BLACK);
        SetVBlankCallback(gPokedexVBlankCB);
        gMain.state++;
        }
        break;
    case 6:
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
        HideBg(0);
        ShowBg(1);
        ShowBg(2);
        ShowBg(3);
        gMain.state++;
        break;
    case 7:
        if (!gPaletteFade.active)
            gMain.state++;
        break;
    case 8:
        gMain.state++;
        break;
    case 9:
        sPokedexView->screenSwitchState = 0;
        gTasks[taskId].data[0] = 0;
        gTasks[taskId].data[1] = 0;
        gTasks[taskId].data[2] = 1;
        gTasks[taskId].func = Task_HandleFormsScreenInput;
        gMain.state = 0;
        break;
    }
}

static void Task_HandleFormsScreenInput(u8 taskId)
{
    u8 base_x = 5;
    u8 base_y = 34;
    u8 offset_x = 34;
    u8 offset_y = 34;
    if (!sPokedexView->sFormScreenData.inSubmenu)
    {
        if (JOY_NEW(A_BUTTON) && sPokedexView->sFormScreenData.numForms != 0)
        {
            sPokedexView->sFormScreenData.inSubmenu = TRUE;
            sPokedexView->sFormScreenData.arrowSpriteId = CreateSprite(&gSpriteTemplate_Arrow, base_x + offset_x, base_y, 0);
            gSprites[sPokedexView->sFormScreenData.arrowSpriteId].animNum = 2;
            EvoFormsPage_PrintNavigationButtons();
        }

        if (JOY_NEW(START_BUTTON))
        {
            sPokedexView->selectedScreen = EVO_SCREEN;
            BeginNormalPaletteFade(0xFFFFFFEB, 0, 0, 0x10, RGB_BLACK);
            sPokedexView->screenSwitchState = 1;
            gTasks[taskId].func = Task_SwitchScreensFromFormsScreen;
            PlaySE(SE_PIN);
        }

        //Exit to overview
        if (JOY_NEW(B_BUTTON))
        {
            BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
            gTasks[taskId].func = Task_ExitFormsScreen;
            PlaySE(SE_PC_OFF);
            return;
        }
    }
    else //inSubmenu
    {
        u8 row = 0;
        u8 column = 0;
        u8 menuPos = sPokedexView->sFormScreenData.menuPos;
        //Grid navigation
        if (JOY_NEW(DPAD_RIGHT))
            menuPos += 1;
        else if (JOY_NEW(DPAD_LEFT))
            menuPos -= 1;
        else if (JOY_NEW(DPAD_DOWN))
            menuPos += 7;
        else if (JOY_NEW(DPAD_UP))
        {
            if (menuPos > 7)
                menuPos -= 7;
            else
                menuPos = 1;
        }

        if (menuPos <= 0)
            menuPos = 1;
        else if (menuPos >= sPokedexView->sFormScreenData.numForms)
            menuPos = sPokedexView->sFormScreenData.numForms;

        row = menuPos / 7;
        column = menuPos % 7;
        gSprites[sPokedexView->sFormScreenData.arrowSpriteId].x = base_x + offset_x * column;
        gSprites[sPokedexView->sFormScreenData.arrowSpriteId].y = base_y + offset_y * row;

        sPokedexView->sFormScreenData.menuPos = menuPos;


        if (JOY_NEW(A_BUTTON))
        {
            u8 formId = sPokedexView->sFormScreenData.formIds[menuPos];
            u16 formSpecies = GetFormSpeciesId(NationalPokedexNumToSpecies(sPokedexListItem->dexNum), formId);
            if (formSpecies == GetFormSpeciesId(formSpecies, 0))
                sPokedexView->formSpecies = 0;
            else
                sPokedexView->formSpecies = formSpecies;

            sPokedexView->sEvoScreenData.fromEvoPage = TRUE;
            sPokedexView->sFormScreenData.inSubmenu = FALSE;
            PlaySE(SE_PIN);
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            gTasks[taskId].func = Task_LoadInfoScreenWaitForFade;
        }

        if (JOY_NEW(B_BUTTON))
        {
            DestroySprite(&gSprites[sPokedexView->sFormScreenData.arrowSpriteId]);
            sPokedexView->sFormScreenData.inSubmenu = FALSE;
            EvoFormsPage_PrintNavigationButtons();
        }
    }
}

#define FORM_SPECIES_END (0xffff)

static void PrintForms(u8 taskId, u16 species)
{
    u8 i;
    u8 j = 1;
    u16 speciesForm;

    u8 base_x = 5;
    u8 base_y = 52;
    u8 base_y_offset = 9;
    u8 times = 0;

    StringCopy(gStringVar1, GetSpeciesName(species));

    for (i=0; i < 30; i++)
    {
        speciesForm = GetFormSpeciesId(species, i);
        if (speciesForm == FORM_SPECIES_END)
            break;
        else if (speciesForm == species)
            continue;
        else
        {
            sPokedexView->sFormScreenData.formIds[j++] = i;
            times += 1;
            LoadCompressedPalette(GetMonSpritePalFromSpeciesAndPersonality(speciesForm, 0, 0), OBJ_PLTT_ID(5 + times), PLTT_SIZE_4BPP);
            if (times < 7)
                gTasks[taskId].data[5 + times] = CreateMonIconNoPersonality(speciesForm, SpriteCB_MonIcon, 52 + 34 * (times - 1), 31, 4); //Create pokemon sprite
            else if (times < 14)
                gTasks[taskId].data[5 + times] = CreateMonIconNoPersonality(speciesForm, SpriteCB_MonIcon, 18 + 34 * (times - 7), 70, 4); //Create pokemon sprite
            gSprites[gTasks[taskId].data[5 + times]].oam.priority   = 0;
            gSprites[gTasks[taskId].data[5 + times]].oam.paletteNum = 5 + times;
        }
    }
    gTasks[taskId].data[3] = times;
    sPokedexView->sFormScreenData.numForms = times;

    //If there are no forms print text
    if (times == 0)
    {
        StringExpandPlaceholders(gStringVar4, sText_FORMS_NONE);
        PrintInfoScreenTextSmall(gStringVar4, base_x, base_y + base_y_offset*times);
    }
}

static void Task_SwitchScreensFromFormsScreen(u8 taskId)
{
    u8 i;
    if (!gPaletteFade.active)
    {
        //FreeMonIconPalettes();                                          //Destroy pokemon icon sprite
        FreeAndDestroyMonIconSprite(&gSprites[gTasks[taskId].data[4]]); //Destroy pokemon icon sprite
        for (i = 1; i <= gTasks[taskId].data[3]; i++)
        {
            FreeAndDestroyMonIconSprite(&gSprites[gTasks[taskId].data[4+i]]); //Destroy pokemon icon sprite
        }
        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);

        switch (sPokedexView->screenSwitchState)
        {
        case 1:
            gTasks[taskId].func = Task_LoadEvolutionScreen;
            break;
        default:
            gTasks[taskId].func = Task_LoadInfoScreen;
            break;
        }
    }
}

static void Task_ExitFormsScreen(u8 taskId)
{
    u8 i;
    if (!gPaletteFade.active)
    {
        //FreeMonIconPalettes();                                          //Destroy pokemon icon sprite
        FreeAndDestroyMonIconSprite(&gSprites[gTasks[taskId].data[4]]); //Destroy pokemon icon sprite
        for (i = 1; i <= gTasks[taskId].data[3]; i++)
        {
            FreeAndDestroyMonIconSprite(&gSprites[gTasks[taskId].data[4+i]]); //Destroy pokemon icon sprite
        }
        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);

        FreeInfoScreenWindowAndBgBuffers();
        DestroyTask(taskId);
    }
}

#undef tMonSpriteId
#undef tScrolling
#undef tMonSpriteDone
#undef tBgLoaded
#undef tSkipCry
#undef tTrainerSpriteId
