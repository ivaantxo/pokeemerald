#include "global.h"
#include "malloc.h"
#include "bg.h"
#include "data.h"
#include "decompress.h"
#include "dma3.h"
#include "dynamic_placeholder_text_util.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_screen_effect.h"
#include "field_weather.h"
#include "fldeff_misc.h"
#include "gpu_regs.h"
#include "graphics.h"
#include "international_string_util.h"
#include "item.h"
#include "item_icon.h"
#include "item_menu.h"
#include "mail.h"
#include "main.h"
#include "menu.h"
#include "naming_screen.h"
#include "overworld.h"
#include "palette.h"
#include "pc_screen_effect.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "pokemon_summary_screen.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "text.h"
#include "text_window.h"
#include "trig.h"
#include "util.h"
#include "window.h"
#include "constants/form_change_types.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "constants/pokemon_icon.h"

/*
    NOTE: This file is large. Some general groups of functions have
          been labeled with commented headers to make navigation easier.
          Search for "SECTION:" to locate them. These sections are not
          hard and fast rules, but give a basic idea of where certain
          types of functions are likely located.
*/

#define BOX_SCROLL_SPEED_FACTOR 4

#define BOX_SCROLL_SPEED(input) ((input) * 6 * BOX_SCROLL_SPEED_FACTOR)

#define COMPACT_PARTY_SPRITES_DELTA 2

#define MON_HAND_DELTA 2

// PC main menu options
enum {
    OPTION_MOVE_MONS,
    OPTION_WITHDRAW,
    OPTION_DEPOSIT,
    OPTION_MOVE_ITEMS,
    OPTIONS_COUNT
};

// IDs for messages to print with PrintMessage
enum {
    MSG_WHAT_YOU_DO,
    MSG_PICK_A_THEME,
    MSG_IS_SELECTED,
    MSG_JUMP_TO_WHICH_BOX,
    MSG_DEPOSIT_IN_WHICH_BOX,
    MSG_WAS_DEPOSITED,
    MSG_BOX_IS_FULL,
    MSG_RELEASE_POKE,
    MSG_WAS_RELEASED,
    MSG_LAST_POKE,
    MSG_PARTY_FULL,
    MSG_HOLDING_POKE,
    MSG_WHICH_ONE_WILL_TAKE,
    MSG_CANT_RELEASE_EGG,
    MSG_CONTINUE_BOX,
    MSG_CAME_BACK,
    MSG_SURPRISE,
    MSG_PLEASE_REMOVE_MAIL,
    MSG_IS_SELECTED2,
    MSG_GIVE_TO_MON,
    MSG_PLACED_IN_BAG,
    MSG_BAG_FULL,
    MSG_PUT_IN_BAG,
    MSG_ITEM_IS_HELD,
    MSG_CHANGED_TO_ITEM,
    MSG_CANT_STORE_MAIL,
};

// IDs for how to resolve variables in the above messages
enum {
    MSG_VAR_NONE,
    MSG_VAR_MON_NAME,
    MSG_VAR_RELEASE_MON,
    MSG_VAR_ITEM_NAME,
};

// IDs for menu selection items. See SetMenuText, HandleMenuInput, etc
enum {
    MENU_CANCEL,
    MENU_STORE,
    MENU_WITHDRAW,
    MENU_MOVE,
    MENU_SHIFT,
    MENU_PLACE,
    MENU_SUMMARY,
    MENU_RELEASE,
    MENU_JUMP,
    MENU_NAME,
    MENU_TAKE,
    MENU_GIVE,
    MENU_GIVE_2,
    MENU_SWITCH,
    MENU_BAG,
    MENU_INFO,
};

#define GENDER_MASK 0x7FFF

// Return IDs for input handlers
enum {
    INPUT_NONE,
    INPUT_MOVE_CURSOR,
    INPUT_SHOW_PARTY,
    INPUT_HIDE_PARTY,
    INPUT_BOX_OPTIONS,
    INPUT_IN_MENU,
    INPUT_SCROLL_RIGHT,
    INPUT_SCROLL_LEFT,
    INPUT_DEPOSIT,
    INPUT_WITHDRAW,
    INPUT_MOVE_MON,
    INPUT_SHIFT_MON,
    INPUT_PLACE_MON,
    INPUT_TAKE_ITEM,
    INPUT_GIVE_ITEM,
    INPUT_SWITCH_ITEMS,
    INPUT_PRESSED_B,
};

enum {
    SCREEN_CHANGE_EXIT_BOX,
    SCREEN_CHANGE_SUMMARY_SCREEN,
    SCREEN_CHANGE_NAME_BOX,
    SCREEN_CHANGE_ITEM_FROM_BAG,
};

enum {
    MODE_PARTY,
    MODE_BOX,
    MODE_MOVE,
};

enum {
    CURSOR_AREA_IN_BOX,
    CURSOR_AREA_IN_PARTY,
    CURSOR_AREA_BOX_TITLE,
    CURSOR_AREA_BUTTONS, // Party Pokémon and Close Box
};

enum {
    CURSOR_ANIM_BOUNCE,
    CURSOR_ANIM_STILL,
    CURSOR_ANIM_OPEN,
    CURSOR_ANIM_FIST,
};

// Special box ids for the choose box menu
#define BOXID_NONE_CHOSEN 200
#define BOXID_CANCELED    201

enum {
    PALTAG_DISPLAY_MON,
    PALTAG_MISC,
    PALTAG_BOX_TITLE,
    PALTAG_ITEM_ICON_0,
    PALTAG_ITEM_ICON_1, // Used implicitly in CreateItemIconSprites
    PALTAG_ITEM_ICON_2, // Used implicitly in CreateItemIconSprites
};

enum {
    GFXTAG_CURSOR,
    GFXTAG_DISPLAY_MON,
    GFXTAG_BOX_TITLE,
    GFXTAG_BOX_TITLE_ALT,
    GFXTAG_ITEM_ICON_0,
    GFXTAG_ITEM_ICON_1, // Used implicitly in CreateItemIconSprites
    GFXTAG_ITEM_ICON_2, // Used implicitly in CreateItemIconSprites
    GFXTAG_CHOOSE_BOX_MENU,
    GFXTAG_CHOOSE_BOX_MENU_SIDES,
    GFXTAG_MON_ICON,
};

// The maximum number of Pokémon icons that can appear on-screen.
// By default the limit is 40 (though in practice only 37 can be).
#define MAX_MON_ICONS max(IN_BOX_COUNT + PARTY_SIZE + 1, 40)

// The maximum number of item icons that can appear on-screen while
// moving held items. 1 in the cursor, and 2 more while switching
// between 2 Pokémon with held items
#define MAX_ITEM_ICONS 3

// IDs for the item icons affine anims
enum {
    ITEM_ANIM_NONE,
    ITEM_ANIM_APPEAR,
    ITEM_ANIM_DISAPPEAR,
    ITEM_ANIM_PICK_UP,
    ITEM_ANIM_PUT_DOWN,
    ITEM_ANIM_PUT_AWAY,
    ITEM_ANIM_LARGE,
};

// IDs for the item icon sprite callbacks
enum {
    ITEM_CB_WAIT_ANIM,
    ITEM_CB_TO_HAND,
    ITEM_CB_TO_MON,
    ITEM_CB_SWAP_TO_HAND,
    ITEM_CB_SWAP_TO_MON,
    ITEM_CB_HIDE_PARTY,
};

enum {
    RELEASE_ANIM_RELEASE,
    RELEASE_ANIM_CAME_BACK,
};

// IDs for InitMonPlaceChange
enum {
    CHANGE_GRAB,
    CHANGE_PLACE,
    CHANGE_SHIFT,
};

// IDs for TilemapUtil
enum {
    TILEMAPID_PKMN_DATA, // The "Pkmn Data" text at the top of the display
    TILEMAPID_PARTY_MENU,
    TILEMAPID_COUNT
};

// Window IDs for sWindowTemplates
enum {
    WIN_DISPLAY_INFO,
    WIN_MESSAGE,
    WIN_ITEM_DESC,
};

struct StorageMessage
{
    const u8 *text;
    u8 format;
};

struct StorageMenu
{
    const u8 *text;
    int textId;
};

struct ChooseBoxMenu
{
    struct Sprite *menuSprite;
    struct Sprite *menuSideSprites[4];
    u8 curBox;
};

struct ItemIcon
{
    struct Sprite *sprite;
    u8 *tiles;
    u16 palIndex;
    u8 area;
    u8 pos;
    bool8 active;
};

struct PokemonStorageSystemData
{
    u8 state;
    u8 boxOption;
    u8 screenChangeType;
    bool8 isReopening;
    u8 taskId;
    u16 partyMenuTilemapBuffer[264];
    u16 partyMenuY;
    u8 partyMenuMoveTimer;
    u8 showPartyMenuState;
    s16 newCurrBoxId;
    s16 scrollSpeed;
    u16 scrollTimer;
    u8 ALIGNED(2) boxTitleTiles[1024];
    u8 boxTitleCycleId;
    u16 boxTitlePal[16];
    u16 boxTitlePalOffset;
    struct Sprite *curBoxTitleSprites[2];
    struct Sprite *nextBoxTitleSprites[2];
    u16 ALIGNED(4) chooseBoxSwapPal[16]; // Holds dynamic palette to swap into choose box gfx
    u16 swapInPal[16];
    void *swapInPalDst;
    s8 transferWholePlttFrames; // if >0, number of frames to transfer whole palette buffer
    u8 scrollState;
    u8 scrollToBoxId;
    s8 scrollDirection;
    struct Sprite *movingMonSprite;
    struct Sprite *partySprites[PARTY_SIZE];
    struct Sprite *boxMonsSprites[IN_BOX_COUNT + 1];
    struct Sprite **shiftMonSpritePtr;
    struct Sprite **releaseMonSpritePtr;
    u16 numIconsPerSpecies[MAX_MON_ICONS];
    u16 iconSpeciesList[MAX_MON_ICONS];
    u16 boxSpecies[IN_BOX_COUNT];
    u32 boxPersonalities[IN_BOX_COUNT];
    u8 incomingBoxId;
    u8 shiftTimer;
    u8 numPartyToCompact;
    s16 iconScrollDistance;
    s16 iconScrollPos;
    s16 iconScrollSpeed;
    u16 iconScrollNumIncoming;
    u8 iconScrollCurColumn;
    u8 iconScrollState;
    struct WindowTemplate menuWindow;
    struct StorageMenu menuItems[7];
    u8 menuItemsCount;
    u8 menuWidth;
    u16 menuWindowId;
    struct Sprite *cursorSprite;
    struct Sprite *cursorShadowSprite;
    s32 cursorNewX;
    s32 cursorNewY;
    u32 cursorSpeedX;
    u32 cursorSpeedY;
    s16 cursorTargetX;
    s16 cursorTargetY;
    u16 cursorMoveSteps;
    s8 cursorVerticalWrap;
    s8 cursorHorizontalWrap;
    u8 newCursorArea;
    u8 newCursorPosition;
    u8 cursorPrevHorizPos;
    const u32 *displayMonPalette;
    u32 displayMonPersonality;
    bool8 displayMonIsShiny;
    u16 displayMonSpecies;
    u16 displayMonItemId;
    bool8 setMosaic;
    u8 displayMonLevel;
    bool8 displayMonIsEgg;
    u8 displayMonName[POKEMON_NAME_LENGTH + 1];
    u8 displayMonNameText[36];
    u8 displayMonSpeciesName[36];
    u8 displayMonGenderLvlText[36];
    u8 displayMonItemName[36];
    bool8 (*monPlaceChangeFunc)(void);
    u8 monPlaceChangeState;
    u8 shiftBoxId;
    struct Sprite *waveformSprites[2];
    struct ChooseBoxMenu chooseBoxMenu;
    struct Pokemon movingMon;
    struct Pokemon tempMon;
    s8 canReleaseMon;
    bool8 releaseStatusResolved;
    s8 releaseCheckBoxId;
    s8 releaseCheckBoxPos;
    s8 releaseBoxId;
    s8 releaseBoxPos;
    u16 releaseCheckState;
    u16 restrictedReleaseMonMoves;
    u16 restrictedMoveList[8];
    u8 summaryMaxPos;
    u8 summaryStartPos;
    u8 summaryScreenMode;
    union
    {
        struct Pokemon *mon;
        struct BoxPokemon *box;
    } summaryMon;
    u8 messageText[40];
    u8 boxTitleText[40];
    u8 releaseMonName[POKEMON_NAME_LENGTH + 1];
    u8 itemName[20];
    struct ItemIcon itemIcons[MAX_ITEM_ICONS];
    u16 movingItemId;
    u16 itemInfoWindowOffset;
    u16 displayMonPalOffset;
    u16 *displayMonTilePtr;
    struct Sprite *displayMonSprite;
    u16 displayMonPalBuffer[64];
    u8 ALIGNED(4) tileBuffer[MON_PIC_SIZE * MAX_MON_PIC_FRAMES];
    u8 ALIGNED(4) itemIconBuffer[2048];
    u8 ALIGNED(4) displayMenuTilemapBuffer[4096];
    u8 ALIGNED(4) eggPalette[2];
};

static u32 sItemIconGfxBuffer[98];

EWRAM_DATA static u8 sPreviousBoxOption = 0;
EWRAM_DATA static struct ChooseBoxMenu *sChooseBoxMenu = NULL;
EWRAM_DATA static struct PokemonStorageSystemData *sStorage = NULL;
EWRAM_DATA static bool8 sInPartyMenu = 0;
EWRAM_DATA static u8 sCurrentBoxOption = 0;
EWRAM_DATA static u8 sDepositBoxId = 0;
EWRAM_DATA static u8 sWhichToReshow = 0;
EWRAM_DATA static u8 sLastUsedBox = 0;
EWRAM_DATA static u16 sMovingItemId = 0;
EWRAM_DATA static struct Pokemon sSavedMovingMon = {0};
EWRAM_DATA static s8 sCursorArea = 0;
EWRAM_DATA static s8 sCursorPosition = 0;
EWRAM_DATA static bool8 sIsMonBeingMoved = 0;
EWRAM_DATA static u8 sMovingMonOrigBoxId = 0;
EWRAM_DATA static u8 sMovingMonOrigBoxPos = 0;
EWRAM_DATA static bool8 sJustOpenedBag = 0;
EWRAM_DATA static u16 *sPaletteSwapBuffer = NULL; // dynamically-allocated buffer to hold box palettes
EWRAM_DATA static u8 allocCount = 0; // Track number of alloc's vs frees
EWRAM_DATA static struct BoxPokemon sCurrentBoxPokemon = {0};

// Main tasks
static void Task_InitPokeStorage(u8);
static void Task_PlaceMon(u8);
static void Task_ChangeScreen(u8);
static void Task_ShowPokeStorage(u8);
static void Task_OnBPressed(u8);
static void Task_HandleBoxOptions(u8);
static void Task_OnSelectedMon(u8);
static void Task_HidePartyPokemon(u8);
static void Task_DepositMenu(u8);
static void Task_MoveMon(u8);
static void Task_GiveMovingItemToMon(u8);
static void Task_SwitchSelectedItem(u8);
static void Task_TakeItemForMoving(u8);
static void Task_WithdrawMon(u8);
static void Task_ShiftMon(u8);
static void Task_ShowPartyPokemon(u8);
static void Task_ShowItemInfo(u8);
static void Task_GiveItemFromBag(u8);
static void Task_ItemToBag(u8);
static void Task_TakeItemForMoving(u8);
static void Task_ShowMonSummary(u8);
static void Task_ReleaseMon(u8);
static void Task_ReshowPokeStorage(u8);
static void Task_PokeStorageMain(u8);
static void Task_JumpBox(u8);
static void Task_NameBox(u8);
static void Task_PrintCantStoreMail(u8);
static void Task_HandleMovingMonFromParty(u8);

// Input handlers
static u8 HandleInput(void);
static void AddBoxOptionsMenu(void);
static u8 SetSelectionMenuTexts(void);
static bool8 SetMenuTexts_Mon(void);
static bool8 SetMenuTexts_Item(void);

// Choose box menu
static void ChooseBoxMenu_CreateSprites(u8);
static void ChooseBoxMenu_DestroySprites(void);
static void ChooseBoxMenu_MoveLeft(void);
static void ChooseBoxMenu_MoveRight(void);
static void ChooseBoxMenu_PrintInfo(void);

// Options menus
static void InitMenu(void);
static void SetMenuText(u8);
static s8 GetMenuItemTextId(u8);
static void AddMenu(void);
static bool8 IsMenuLoading(void);
static s16 HandleMenuInput(void);
static void RemoveMenu(void);

// Pokémon sprites
static void InitMonIconFields(void);
static void SpriteCB_BoxMonIconScrollOut(struct Sprite *);
static void GetIncomingBoxMonData(u8);
static void CreatePartyMonsSprites(bool8);
static void CompactPartySprites(void);
static u8 GetNumPartySpritesCompacting(void);
static void MovePartySpriteToNextSlot(struct Sprite *, u16);
static void SpriteCB_MovePartyMonToNextSlot(struct Sprite *);
static void MovePartySprites(s16);
static void DestroyAllPartyMonIcons(void);
static void ReshowReleaseMon(void);
static bool8 ResetReleaseMonSpritePtr(void);
static void SetMovingMonPriority(u8);
static void SpriteCB_HeldMon(struct Sprite *);
static struct Sprite *CreateMonIconSprite(u16, u32, s16, s16, u8, u8);
static void DestroyBoxMonIcon(struct Sprite *);

// Pokémon data
static void MoveMon(void);
static void PlaceMon(void);
static void RefreshDisplayMon(void);
static void SetMovingMonData(u8, u8);
static void SetPlacedMonData(u8, u8);
static void PurgeMonOrBoxMon(u8, u8);
static void SetShiftedMonData(u8, u8);
static void SetShiftedMonSprites(u8, u8);
static bool8 TryStorePartyMonInBox(u8);
static void ResetSelectionAfterDeposit(void);
static void InitReleaseMon(void);
static bool8 TryHideReleaseMon(void);
static void InitCanReleaseMonVars(void);
static void ReleaseMon(void);
static bool32 AtLeastThreeUsableMons(void);
static s8 RunCanReleaseMon(void);
static void SaveMovingMon(void);
static void LoadSavedMovingMon(void);
static void InitSummaryScreenData(void);
static void SetSelectionAfterSummaryScreen(void);
static bool8 IsRemovingLastPartyMon(void);
static bool8 CanPlaceMon(void);
static bool8 CanShiftMon(void);
static bool8 IsMonBeingMoved(void);
static void TryRefreshDisplayMon(void);
static void ReshowDisplayMon(void);
static void SetDisplayMonData(void *, u8);

// Move Items mode
static bool32 IsItemIconAtPosition(u8, u8);
static u8 GetNewItemIconIdx(void);
static void SetItemIconPosition(u8, u8, u8);
static void LoadItemIconGfx(u8, const u32 *, const u32 *);
static void SetItemIconAffineAnim(u8, u8);
static void SetItemIconActive(u8, bool8);
static u8 GetItemIconIdxByPosition(u8, u8);
static void CreateItemIconSprites(void);
static void TryLoadItemIconAtPos(u8, u8);
static void TryHideItemIconAtPos(u8, u8, bool32);
static void TakeItemFromMon(u8, u8);
static void InitItemIconInCursor(u16);
static void SwapItemsWithMon(u8, u8);
static void GiveItemToMon(u8, u8);
static void MoveItemFromMonToBag(u8, u8);
static void MoveItemFromCursorToBag(void);
static void MoveHeldItemWithPartyMenu(void);
static bool8 IsItemIconAnimActive(void);
static bool8 IsMovingItem(void);
static const u8 *GetMovingItemName(void);
static u16 GetMovingItemId(void);
static void PrintItemDescription(void);
static void InitItemInfoWindow(void);
static bool8 UpdateItemInfoWindowSlideIn(void);
static bool8 UpdateItemInfoWindowSlideOut(void);
static void DrawItemInfoWindow(u32);
static void SetItemIconCallback(u8, u8, u8, u8);
static void SpriteCB_ItemIcon_SetPosToCursor(struct Sprite *);
static void SpriteCB_ItemIcon_WaitAnim(struct Sprite *);
static void SpriteCB_ItemIcon_ToHand(struct Sprite *);
static void SpriteCB_ItemIcon_ToMon(struct Sprite *);
static void SpriteCB_ItemIcon_SwapToHand(struct Sprite *);
static void SpriteCB_ItemIcon_HideParty(struct Sprite *);
static void SpriteCB_ItemIcon_SwapToMon(struct Sprite *);

// Cursor
static void CreateCursorSprites(void);
static u8 GetCursorPosition(void);
static void StartCursorAnim(u8);
static void TryHideItemAtCursor(void);
static void TryShowItemAtCursor(void);
static void InitCursor(void);
static void InitCursorOnReopen(void);
static void GetCursorCoordsByPos(u8, u8, u16 *, u16 *);
static bool8 UpdateCursorPos(void);
static void DoCursorNewPosUpdate(void);
static void SetCursorInParty(void);
static void SetCursorBoxPosition(u8);
static void ClearSavedCursorPos(void);
static void SaveCursorPos(void);
static u8 GetSavedCursorPos(void);
static void InitMonPlaceChange(u8);
static bool8 DoMonPlaceChange(void);
static bool8 MonPlaceChange_Shift(void);
static bool8 MonPlaceChange_Grab(void);
static bool8 MonPlaceChange_Place(void);
static bool8 MonPlaceChange_CursorDown(void);
static bool8 MonPlaceChange_CursorUp(void);
static void TrySetCursorFistAnim(void);
static bool8 IsCursorOnBoxTitle(void);
static bool8 IsCursorInBox(void);

// Box title
static void InitBoxTitle(u8);
static void CreateIncomingBoxTitle(u8 boxId, s8 direction);
static void CycleBoxTitleSprites(void);
static void SpriteCB_IncomingBoxTitle(struct Sprite *);
static void SpriteCB_OutgoingBoxTitle(struct Sprite *);
static s16 GetBoxTitleBaseX(const u8 *);

// General box
static void CreateInitBoxTask(u8);
static bool8 IsInitBoxActive(void);
static void Task_InitBox(u8);
static void SetUpScrollToBox(u8);
static bool8 ScrollToBox(void);
static s8 DetermineBoxScrollDirection(u8);
static void SetCurrentBox(u8);

// Misc
static void CreateMainMenu(u8, s16 *);
static u8 GetCurrentBoxOption(void);
static void ScrollBackground(void);
static void TintBackground(void);
static void GiveChosenBagItem(void);
static void SetUpHidePartyMenu(void);
static void LoadPokeStorageMenuGfx(void);
static void InitPokeStorageBg0(void);
static void SetScrollingBackground(void);
static void UpdateBoxToSendMons(void);
static void InitCursorItemIcon(void);
static void InitPalettesAndSprites(void);
static void RefreshDisplayMonData(void);
static void CreateDisplayMonSprite(void);
static void ClearBottomWindow(void);
static void InitSupplementalTilemaps(void);
static void PrintDisplayMonInfo(void);
static void SetPartySlotTilemaps(void);
static void FreePokeStorageData(void);
static void UpdatePartySlotColors(void);
static void SetUpDoShowPartyMenu(void);
static void StartDisplayMonMosaicEffect(void);
static bool8 InitPokeStorageWindows(void);
static bool8 DoShowPartyMenu(void);
static bool8 HidePartyMenu(void);
static bool8 IsDisplayMosaicActive(void);
static void ShowYesNoWindow(s8);
static void PrintMessage(u8 id);
static void LoadDisplayMonGfx(u16, u32);
static void SpriteCB_DisplayMonMosaic(struct Sprite *);
static void SetPartySlotTilemap(u8, bool8);

// Tilemap utility
static void TilemapUtil_SetRect(u8, u16, u16, u16, u16);
static void TilemapUtil_Move(s8 val);
static void TilemapUtil_SetMap(u8, u8, const void *, u16, u16);
static void TilemapUtil_SetPos(u8, u16, u16);
static void TilemapUtil_Init(u8);
static void TilemapUtil_Free(void);
static void TilemapUtil_Update(u8);
static void TilemapUtil_Draw(u8);

static const u8 gText_JustOnePkmn[] = _("There is just one POKéMON with you.");
static const u8 gText_PartyFull[] = _("Your party is full!");
static const u8 gText_Box[] = _("BOX");

struct {
    const u8 *text;
    const u8 *desc;
}

static const sMainMenuTexts[OPTIONS_COUNT] =
{
    [OPTION_MOVE_MONS]  = {COMPOUND_STRING("MOVE POKéMON"),     COMPOUND_STRING("Organize the POKéMON in BOXES and\nin your party.")},
    [OPTION_WITHDRAW]   = {COMPOUND_STRING("WITHDRAW POKéMON"), COMPOUND_STRING("Move POKéMON stored in BOXES to\nyour party.")},
    [OPTION_DEPOSIT]    = {COMPOUND_STRING("DEPOSIT POKéMON"),  COMPOUND_STRING("Store POKéMON in your party in BOXES.")},
    [OPTION_MOVE_ITEMS] = {COMPOUND_STRING("MOVE ITEMS"),       COMPOUND_STRING("Move items held by any POKéMON\nin a BOX or your party.")},
};

static const struct WindowTemplate sWindowTemplate_MainMenu =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = 17,
    .height = 8,
    .paletteNum = 15,
    .baseBlock = 1,
};

static const union AnimCmd sAnim_ChooseBoxMenu_TopLeft[] =
{
    ANIMCMD_FRAME(0, 5),
    ANIMCMD_END
};

static const union AnimCmd sAnim_ChooseBoxMenu_BottomLeft[] =
{
    ANIMCMD_FRAME(4, 5),
    ANIMCMD_END
};

static const union AnimCmd sAnim_ChooseBoxMenu_TopRight[] =
{
    ANIMCMD_FRAME(6, 5),
    ANIMCMD_END
};

static const union AnimCmd sAnim_ChooseBoxMenu_BottomRight[] =
{
    ANIMCMD_FRAME(10, 5),
    ANIMCMD_END
};

static const union AnimCmd *const sAnims_ChooseBoxMenu[] =
{
    sAnim_ChooseBoxMenu_TopLeft,
    sAnim_ChooseBoxMenu_BottomLeft,
    sAnim_ChooseBoxMenu_TopRight,
    sAnim_ChooseBoxMenu_BottomRight
};

static const union AffineAnimCmd sAffineAnim_ChooseBoxMenu[] =
{
    AFFINEANIMCMD_FRAME(224, 224, 0, 0),
    AFFINEANIMCMD_END
};

static const u8 sChooseBoxMenu_TextColors[] = {TEXT_COLOR_RED, TEXT_DYNAMIC_COLOR_6, TEXT_DYNAMIC_COLOR_5};
static const u8 sText_OutOf30[] = _("/30");

static const u16 sChooseBoxMenu_Pal[]        = INCBIN_U16("graphics/pokemon_storage/box_selection_popup.gbapal");
static const u8 sChooseBoxMenuCenter_Gfx[]   = INCBIN_U8("graphics/pokemon_storage/box_selection_popup_center.4bpp");
static const u8 sChooseBoxMenuSides_Gfx[]    = INCBIN_U8("graphics/pokemon_storage/box_selection_popup_sides.4bpp");
static const u32 sScrollingBg_Gfx[]          = INCBIN_U32("graphics/pokemon_storage/scrolling_bg.4bpp.lz");
static const u32 sScrollingBg_Tilemap[]      = INCBIN_U32("graphics/pokemon_storage/scrolling_bg.bin.lz");
static const u32 sDisplayMenu_Tilemap[]      = INCBIN_U32("graphics/pokemon_storage/display_menu.bin.lz");
static const u16 sPkmnData_Tilemap[]         = INCBIN_U16("graphics/pokemon_storage/pkmn_data.bin");
// sInterface_Pal - parts of the display frame, "PkmnData"'s normal color, Close Box
static const u16 sInterface_Pal[]            = INCBIN_U16("graphics/pokemon_storage/interface.gbapal");
static const u16 sPkmnDataGray_Pal[]         = INCBIN_U16("graphics/pokemon_storage/pkmn_data_gray.gbapal");
static const u16 sScrollingBg_Pal[]          = INCBIN_U16("graphics/pokemon_storage/scrolling_bg.gbapal");
static const u16 sPartySlotFilled_Tilemap[]  = INCBIN_U16("graphics/pokemon_storage/party_slot_filled.bin");
static const u16 sPartySlotEmpty_Tilemap[]   = INCBIN_U16("graphics/pokemon_storage/party_slot_empty.bin");
static const u16 sTextWindows_Pal[]          = INCBIN_U16("graphics/pokemon_storage/text_windows.gbapal");

static const struct WindowTemplate sWindowTemplates[] =
{
    // The panel below the currently displayed Pokémon
    [WIN_DISPLAY_INFO] = {
        .bg = 1,
        .tilemapLeft = 0,
        .tilemapTop = 11,
        .width = 9,
        .height = 7,
        .paletteNum = 3,
        .baseBlock = 192,
    },
    [WIN_MESSAGE] = {
        .bg = 0,
        .tilemapLeft = 11,
        .tilemapTop = 17,
        .width = 18,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 20,
    },
    [WIN_ITEM_DESC] = {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 13,
        .width = 21,
        .height = 7,
        .paletteNum = 15,
        .baseBlock = 20,
    },
    DUMMY_WIN_TEMPLATE
};

static const struct BgTemplate sBgTemplates[] =
{
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 29,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
    {
        .bg = 1,
        .charBaseIndex = 1,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 256
    },
    {
        .bg = 2,
        .charBaseIndex = 2,
        .mapBaseIndex = 27,
        .screenSize = 1,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0
    },
    {
        .bg = 3,
        .charBaseIndex = 3,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 3,
        .baseTile = 0
    },
};

static const struct OamData sOamData_DisplayMon;
static const struct SpriteTemplate sSpriteTemplate_DisplayMon =
{
    .tileTag = GFXTAG_DISPLAY_MON,
    .paletteTag = PALTAG_DISPLAY_MON,
    .oam = &sOamData_DisplayMon,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const u8 gText_PkmnIsSelected[] = _("{DYNAMIC 0} is selected.");

static const struct StorageMessage sMessages[] =
{
    [MSG_WHAT_YOU_DO]          = {COMPOUND_STRING("What do you want to do?"),    MSG_VAR_NONE},
    [MSG_PICK_A_THEME]         = {COMPOUND_STRING("Please pick a theme."),       MSG_VAR_NONE},
    [MSG_IS_SELECTED]          = {gText_PkmnIsSelected,                          MSG_VAR_MON_NAME},
    [MSG_JUMP_TO_WHICH_BOX]    = {COMPOUND_STRING("Jump to which BOX?"),         MSG_VAR_NONE},
    [MSG_DEPOSIT_IN_WHICH_BOX] = {COMPOUND_STRING("Deposit in which BOX?"),      MSG_VAR_NONE},
    [MSG_WAS_DEPOSITED]        = {COMPOUND_STRING("{DYNAMIC 0} was deposited."), MSG_VAR_MON_NAME},
    [MSG_BOX_IS_FULL]          = {COMPOUND_STRING("The BOX is full."),           MSG_VAR_NONE},
    [MSG_RELEASE_POKE]         = {COMPOUND_STRING("Release this POKéMON?"),      MSG_VAR_NONE},
    [MSG_WAS_RELEASED]         = {COMPOUND_STRING("{DYNAMIC 0} was released."),  MSG_VAR_RELEASE_MON},
    [MSG_LAST_POKE]            = {COMPOUND_STRING("That's your last POKéMON!"),  MSG_VAR_NONE},
    [MSG_PARTY_FULL]           = {gText_YourPartysFull,                          MSG_VAR_NONE},
    [MSG_HOLDING_POKE]         = {COMPOUND_STRING("You're holding a POKéMON!"),  MSG_VAR_NONE},
    [MSG_WHICH_ONE_WILL_TAKE]  = {COMPOUND_STRING("Which one will you take?"),   MSG_VAR_NONE},
    [MSG_CANT_RELEASE_EGG]     = {COMPOUND_STRING("You can't release an EGG."),  MSG_VAR_NONE},
    [MSG_CONTINUE_BOX]         = {COMPOUND_STRING("Continue BOX operations?"),   MSG_VAR_NONE},
    [MSG_CAME_BACK]            = {COMPOUND_STRING("{DYNAMIC 0} came back!"),     MSG_VAR_MON_NAME},
    [MSG_SURPRISE]             = {COMPOUND_STRING("… … … … !"),                  MSG_VAR_NONE},
    [MSG_PLEASE_REMOVE_MAIL]   = {COMPOUND_STRING("Please remove the MAIL."),    MSG_VAR_NONE},
    [MSG_IS_SELECTED2]         = {gText_PkmnIsSelected,                          MSG_VAR_ITEM_NAME},
    [MSG_GIVE_TO_MON]          = {COMPOUND_STRING("GIVE to a POKéMON?"),         MSG_VAR_NONE},
    [MSG_PLACED_IN_BAG]        = {COMPOUND_STRING("Placed item in the BAG."),    MSG_VAR_ITEM_NAME},
    [MSG_BAG_FULL]             = {COMPOUND_STRING("The BAG is full."),           MSG_VAR_NONE},
    [MSG_PUT_IN_BAG]           = {COMPOUND_STRING("Put this item in the BAG?"),  MSG_VAR_NONE},
    [MSG_ITEM_IS_HELD]         = {COMPOUND_STRING("{DYNAMIC 0} is now held."),   MSG_VAR_ITEM_NAME},
    [MSG_CHANGED_TO_ITEM]      = {COMPOUND_STRING("Changed to {DYNAMIC 0}."),    MSG_VAR_ITEM_NAME},
    [MSG_CANT_STORE_MAIL]      = {COMPOUND_STRING("MAIL can't be stored!"),      MSG_VAR_NONE},
};

static const struct WindowTemplate sYesNoWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 24,
    .tilemapTop = 11,
    .width = 5,
    .height = 4,
    .paletteNum = 15,
    .baseBlock = 92,
};

static const struct OamData sOamData_DisplayMon =
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

static const struct OamData sOamData_MonIcon =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(32x32),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(32x32),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0
};

static const struct SpriteTemplate sSpriteTemplate_MonIcon =
{
    .tileTag = GFXTAG_MON_ICON,
    .paletteTag = POKE_ICON_BASE_PAL_TAG,
    .oam = &sOamData_MonIcon,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const union AffineAnimCmd sAffineAnim_ReleaseMon_Release[] =
{
    AFFINEANIMCMD_FRAME(-2, -2, 0, 120),
    AFFINEANIMCMD_END
};

static const union AffineAnimCmd sAffineAnim_ReleaseMon_CameBack[] =
{
    AFFINEANIMCMD_FRAME(16, 16, 0, 0),
    AFFINEANIMCMD_FRAME(16, 16, 0, 15),
    AFFINEANIMCMD_END
};

static const union AffineAnimCmd *const sAffineAnims_ReleaseMon[] =
{
    [RELEASE_ANIM_RELEASE]   = sAffineAnim_ReleaseMon_Release,
    [RELEASE_ANIM_CAME_BACK] = sAffineAnim_ReleaseMon_CameBack
};

static const struct OamData sOamData_BoxTitle =
{
    .shape = SPRITE_SHAPE(32x16),
    .size = SPRITE_SIZE(32x16),
    .priority = 2
};

static const union AnimCmd sAnim_BoxTitle_Left[] =
{
    ANIMCMD_FRAME(0, 5),
    ANIMCMD_END
};

static const union AnimCmd sAnim_BoxTitle_Right[] =
{
    ANIMCMD_FRAME(8, 5),
    ANIMCMD_END
};

static const union AnimCmd *const sAnims_BoxTitle[] =
{
    sAnim_BoxTitle_Left,
    sAnim_BoxTitle_Right
};

static const struct SpriteTemplate sSpriteTemplate_BoxTitle =
{
    .tileTag = GFXTAG_BOX_TITLE,
    .paletteTag = PALTAG_MISC,
    .oam = &sOamData_BoxTitle,
    .anims = sAnims_BoxTitle,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy
};

static const u16 ALIGNED(4) sHandCursor_Pal[] = INCBIN_U16("graphics/pokemon_storage/hand_cursor.gbapal");
static const u8 sHandCursor_Gfx[] = INCBIN_U8("graphics/pokemon_storage/hand_cursor.4bpp");

//------------------------------------------------------------------------------
//  SECTION: Misc utility
//------------------------------------------------------------------------------

void DrawTextWindowAndBufferTiles(const u8 *string, void *dst, s32 bytesToBuffer)
{
    s32 i, tileBytesToBuffer;
    u16 windowId;
    u8 txtColor[3];
    u8 *tileData1, *tileData2;
    struct WindowTemplate winTemplate = {0};

    winTemplate.width = 24;
    winTemplate.height = 2;
    windowId = AddWindow(&winTemplate);
    FillWindowPixelBuffer(windowId, PIXEL_FILL(0));
    tileData1 = (u8 *) GetWindowAttribute(windowId, WINDOW_TILE_DATA);
    tileData2 = (winTemplate.width * TILE_SIZE_4BPP) + tileData1;

    txtColor[0] = TEXT_COLOR_TRANSPARENT;
    txtColor[1] = TEXT_DYNAMIC_COLOR_6;
    txtColor[2] = TEXT_DYNAMIC_COLOR_5;
    AddTextPrinterParameterized4(windowId, FONT_NORMAL, 0, 1, 0, 0, txtColor, TEXT_SKIP_DRAW, string);

    tileBytesToBuffer = bytesToBuffer;
    if (tileBytesToBuffer > 6u)
        tileBytesToBuffer = 6;
    if (tileBytesToBuffer > 0)
    {
        for (i = tileBytesToBuffer; i != 0; i--)
        {
            CpuCopy16(tileData1, dst, 128);
            CpuCopy16(tileData2, dst + 128, 128);
            tileData1 += 128;
            tileData2 += 128;
            dst += 256;
        }
    }

    RemoveWindow(windowId);
}

u8 CountMonsInBox(u8 boxId)
{
    u16 i, count;

    for (i = 0, count = 0; i < IN_BOX_COUNT; i++)
    {
        if (GetBoxMonDataAt(boxId, i, MON_DATA_SPECIES) != SPECIES_NONE)
            count++;
    }

    return count;
}

s16 GetFirstFreeBoxSpot(u8 boxId)
{
    u16 i;

    for (i = 0; i < IN_BOX_COUNT; i++)
    {
        if (GetBoxMonDataAt(boxId, i, MON_DATA_SPECIES) == SPECIES_NONE)
            return i;
    }

    return -1; // all spots are taken
}

u32 CountPartyNonEggMons(void)
{
    u32 i, count;

    for (i = 0, count = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE
            && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
        {
            count++;
        }
    }

    return count;
}

u8 CountPartyAliveNonEggMonsExcept(u8 slotToIgnore)
{
    u16 i, count;

    for (i = 0, count = 0; i < PARTY_SIZE; i++)
    {
        if (i != slotToIgnore
            && GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE
            && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG)
            && GetMonData(&gPlayerParty[i], MON_DATA_HP) != 0)
        {
            count++;
        }
    }

    return count;
}

u16 CountPartyAliveNonEggMons_IgnoreVar0x8004Slot(void)
{
    return CountPartyAliveNonEggMonsExcept(gSpecialVar_4);
}

u8 CountPartyMons(void)
{
    u16 i, count;

    for (i = 0, count = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE)
        {
            count++;
        }
    }

    return count;
}

u8 *StringCopyAndFillWithSpaces(u8 *dst, const u8 *src, u16 n)
{
    u8 *str;

    for (str = StringCopy(dst, src); str < dst + n; str++)
        *str = CHAR_SPACE;

    *str = EOS;
    return str;
}

//------------------------------------------------------------------------------
//  SECTION: Main menu
//
//  The below functions generally handle the PC main menu where the main
//  options can be selected (Withdraw, Deposit, etc.), as well as exiting
//  Pokémon Storage back to this menu.
//------------------------------------------------------------------------------

enum {
    STATE_LOAD,
    STATE_FADE_IN,
    STATE_HANDLE_INPUT,
    STATE_ERROR_MSG,
    STATE_ENTER_PC,
};

#define tState          data[0]
#define tSelectedOption data[1]
#define tInput          data[2]
#define tNextOption     data[3]
#define tWindowId       data[15]

static void Task_PCMainMenu(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    switch (task->tState)
    {
    case STATE_LOAD:
        CreateMainMenu(task->tSelectedOption, &task->tWindowId);
        LoadMessageBoxAndBorderGfx();
        DrawDialogueFrame(0, FALSE);
        FillWindowPixelBuffer(0, PIXEL_FILL(1));
        AddTextPrinterParameterized2(0, FONT_NORMAL, sMainMenuTexts[task->tSelectedOption].desc, TEXT_SKIP_DRAW, NULL, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY);
        CopyWindowToVram(0, COPYWIN_FULL);
        CopyWindowToVram(task->tWindowId, COPYWIN_FULL);
        task->tState++;
        break;
    case STATE_FADE_IN:
        if (IsWeatherNotFadingIn())
            task->tState++;
        break;
    case STATE_HANDLE_INPUT:
        task->tInput = Menu_ProcessInput();
        switch(task->tInput)
        {
        case MENU_NOTHING_CHOSEN:
            task->tNextOption = task->tSelectedOption;
            if (JOY_NEW(DPAD_UP) && --task->tNextOption < 0)
                task->tNextOption = OPTIONS_COUNT - 1;
            if (JOY_NEW(DPAD_DOWN) && ++task->tNextOption > OPTIONS_COUNT - 1)
                task->tNextOption = 0;

            if (task->tSelectedOption != task->tNextOption)
            {
                task->tSelectedOption = task->tNextOption;
                FillWindowPixelBuffer(0, PIXEL_FILL(1));
                AddTextPrinterParameterized2(0, FONT_NORMAL, sMainMenuTexts[task->tSelectedOption].desc, 0, NULL, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY);
            }
            break;
        case MENU_B_PRESSED:
            ClearStdWindowAndFrame(task->tWindowId, TRUE);
            UnlockPlayerFieldControls();
            ScriptContext_Enable();
            RemoveWindow(task->tWindowId);
            DestroyTask(taskId);
            break;
        default:
            if (task->tInput == OPTION_WITHDRAW && CountPartyMons() == PARTY_SIZE)
            {
                // Can't withdraw
                FillWindowPixelBuffer(0, PIXEL_FILL(1));
                AddTextPrinterParameterized2(0, FONT_NORMAL, gText_PartyFull, 0, NULL, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY);
                task->tState = STATE_ERROR_MSG;
            }
            else if (task->tInput == OPTION_DEPOSIT && CountPartyMons() == 1)
            {
                // Can't deposit
                FillWindowPixelBuffer(0, PIXEL_FILL(1));
                AddTextPrinterParameterized2(0, FONT_NORMAL, gText_JustOnePkmn, 0, NULL, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY);
                task->tState = STATE_ERROR_MSG;
            }
            else
            {
                // Enter PC
                FadeScreen(FADE_TO_BLACK, 0);
                task->tState = STATE_ENTER_PC;
            }
            break;
        }
        break;
    case STATE_ERROR_MSG:
        // Printed "can't do PC option message"
        // Wait for new input after message
        if (JOY_NEW(A_BUTTON | B_BUTTON))
        {
            FillWindowPixelBuffer(0, PIXEL_FILL(1));
            AddTextPrinterParameterized2(0, FONT_NORMAL, sMainMenuTexts[task->tSelectedOption].desc, 0, NULL, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY);
            task->tState = STATE_HANDLE_INPUT;
        }
        else if (JOY_NEW(DPAD_UP))
        {
            if (--task->tSelectedOption < 0)
                task->tSelectedOption = OPTIONS_COUNT - 1;
            Menu_MoveCursor(-1);
            task->tSelectedOption = Menu_GetCursorPos();
            FillWindowPixelBuffer(0, PIXEL_FILL(1));
            AddTextPrinterParameterized2(0, FONT_NORMAL, sMainMenuTexts[task->tSelectedOption].desc, 0, NULL, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY);
            task->tState = STATE_HANDLE_INPUT;
        }
        else if (JOY_NEW(DPAD_DOWN))
        {
            if (++task->tSelectedOption >= OPTIONS_COUNT - 1)
                task->tSelectedOption = 0;
            Menu_MoveCursor(1);
            task->tSelectedOption = Menu_GetCursorPos();
            FillWindowPixelBuffer(0, PIXEL_FILL(1));
            AddTextPrinterParameterized2(0, FONT_NORMAL, sMainMenuTexts[task->tSelectedOption].desc, 0, NULL, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY);
            task->tState = STATE_HANDLE_INPUT;
        }
        break;
    case STATE_ENTER_PC:
        if (!gPaletteFade.active)
        {
            CleanupOverworldWindowsAndTilemaps();
            EnterPokeStorage(task->tInput);
            RemoveWindow(task->tWindowId);
            DestroyTask(taskId);
        }
        break;
    }
}

void ShowPokemonStorageSystemPC(void)
{
    u8 taskId = CreateTask(Task_PCMainMenu, 80);
    gTasks[taskId].tState = 0;
    gTasks[taskId].tSelectedOption = 0;
    LockPlayerFieldControls();
}

static void FieldTask_ReturnToPcMenu(void)
{
    u8 taskId;
    MainCallback vblankCb = gMain.vblankCallback;

    SetVBlankCallback(NULL);
    taskId = CreateTask(Task_PCMainMenu, 80);
    gTasks[taskId].tState = 0;
    gTasks[taskId].tSelectedOption = sPreviousBoxOption;
    Task_PCMainMenu(taskId);
    SetVBlankCallback(vblankCb);
    FadeInFromBlack();
}

#undef tState
#undef tSelectedOption
#undef tInput
#undef tNextOption
#undef tWindowId

static void CreateMainMenu(u8 whichMenu, s16 *windowIdPtr)
{
    s16 windowId;
    struct WindowTemplate template = sWindowTemplate_MainMenu;
    template.width = GetMaxWidthInMenuTable((void *)sMainMenuTexts, OPTIONS_COUNT);
    windowId = AddWindow(&template);

    DrawStdWindowFrame(windowId, FALSE);
    PrintMenuTable(windowId, OPTIONS_COUNT, (void *)sMainMenuTexts);
    InitMenuInUpperLeftCornerNormal(windowId, OPTIONS_COUNT, whichMenu);
    *windowIdPtr = windowId;
}

static void CB2_ExitPokeStorage(void)
{
    sPreviousBoxOption = GetCurrentBoxOption();
    gFieldCallback = FieldTask_ReturnToPcMenu;
    SetMainCallback2(CB2_ReturnToField);
}

void ResetPokemonStorageSystem(void)
{
    u16 boxId, boxPosition;

    SetCurrentBox(0);
    for (boxId = 0; boxId < TOTAL_BOXES_COUNT; boxId++)
    {
        for (boxPosition = 0; boxPosition < IN_BOX_COUNT; boxPosition++)
            ZeroBoxMonAt(boxId, boxPosition);
    }
    for (boxId = 0; boxId < TOTAL_BOXES_COUNT; boxId++)
    {
        u8 *dest = StringCopy(GetBoxNamePtr(boxId), gText_Box);
        ConvertIntToDecimalStringN(dest, boxId + 1, STR_CONV_MODE_LEFT_ALIGN, 2);
    }
}

//------------------------------------------------------------------------------
//  SECTION: Choose Box menu
//
//  The below functions handle the popup menu that allows the player to cycle
//  through the boxes and select one. Used when storing Pokémon in Deposit mode
//  and for the Jump feature.
//------------------------------------------------------------------------------

static void LoadChooseBoxMenuGfx(struct ChooseBoxMenu *menu)
{
    struct SpriteSheet sheets[] =
    {
        {sChooseBoxMenuCenter_Gfx, 2048, GFXTAG_CHOOSE_BOX_MENU},
        {sChooseBoxMenuSides_Gfx,  384, GFXTAG_CHOOSE_BOX_MENU_SIDES},
        {}
    };

    CpuFastCopy(sHandCursor_Pal, sStorage->chooseBoxSwapPal, PLTT_SIZE_4BPP);
    LoadSpriteSheets(sheets);
    sChooseBoxMenu = menu;
}

static void FreeChooseBoxMenu(void)
{
    FreeSpritePaletteByTag(PALTAG_MISC);
    FreeSpriteTilesByTag(GFXTAG_CHOOSE_BOX_MENU);
    FreeSpriteTilesByTag(GFXTAG_CHOOSE_BOX_MENU_SIDES);
    sStorage->chooseBoxSwapPal[0] = 0; // Stop dynamically loading choose box palette
}

static void CreateChooseBoxMenuSprites(u8 curBox)
{
    ChooseBoxMenu_CreateSprites(curBox);
}

static void DestroyChooseBoxMenuSprites(void)
{
    ChooseBoxMenu_DestroySprites();
}

// For the popout window when choosing a box to deposit in or jump to
static u8 HandleChooseBoxMenuInput(void)
{
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        return BOXID_CANCELED;
    }
    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        return sChooseBoxMenu->curBox;
    }
    if (JOY_NEW(DPAD_LEFT) || JOY_NEW(L_BUTTON))
    {
        PlaySE(SE_SELECT);
        ChooseBoxMenu_MoveLeft();
    }
    else if (JOY_NEW(DPAD_RIGHT) || JOY_NEW(R_BUTTON))
    {
        PlaySE(SE_SELECT);
        ChooseBoxMenu_MoveRight();
    }
    return BOXID_NONE_CHOSEN;
}

static void ChooseBoxMenu_CreateSprites(u8 curBox)
{
    u16 i;
    u8 spriteId;
    struct SpriteTemplate template;
    struct OamData oamData = {};
    oamData.size = SPRITE_SIZE(64x64);
    oamData.paletteNum = 1;
    template = (struct SpriteTemplate)
    {
        0, 0, &oamData, gDummySpriteAnimTable, NULL, gDummySpriteAffineAnimTable, SpriteCallbackDummy
    };

    sChooseBoxMenu->curBox = curBox;
    template.tileTag = GFXTAG_CHOOSE_BOX_MENU;
    template.paletteTag = PALTAG_MISC;

    spriteId = CreateSprite(&template, 160, 96, 0);
    sChooseBoxMenu->menuSprite = &gSprites[spriteId];

    oamData.shape = SPRITE_SHAPE(8x32);
    oamData.size = SPRITE_SIZE(8x32);
    template.tileTag = GFXTAG_CHOOSE_BOX_MENU_SIDES;
    template.anims = sAnims_ChooseBoxMenu;
    for (i = 0; i < ARRAY_COUNT(sChooseBoxMenu->menuSideSprites); i++)
    {
        u16 anim;
        spriteId = CreateSprite(&template, 124, 80, 3);
        sChooseBoxMenu->menuSideSprites[i] = &gSprites[spriteId];
        anim = 0;
        if (i & 2)
        {
            sChooseBoxMenu->menuSideSprites[i]->x = 196;
            anim = 2;
        }
        if (i & 1)
        {
            sChooseBoxMenu->menuSideSprites[i]->y = 112;
            sChooseBoxMenu->menuSideSprites[i]->oam.size = 0;
            anim++;
        }
        StartSpriteAnim(sChooseBoxMenu->menuSideSprites[i], anim);
    }
    ChooseBoxMenu_PrintInfo();
}

static void ChooseBoxMenu_DestroySprites(void)
{
    u16 i;
    if (sChooseBoxMenu->menuSprite)
    {
        DestroySprite(sChooseBoxMenu->menuSprite);
        sChooseBoxMenu->menuSprite = NULL;
    }
    for (i = 0; i < ARRAY_COUNT(sChooseBoxMenu->menuSideSprites); i++)
    {
        if (sChooseBoxMenu->menuSideSprites[i])
        {
            DestroySprite(sChooseBoxMenu->menuSideSprites[i]);
            sChooseBoxMenu->menuSideSprites[i] = NULL;
        }
    }
}

static void ChooseBoxMenu_MoveRight(void)
{
    if (++sChooseBoxMenu->curBox >= TOTAL_BOXES_COUNT)
        sChooseBoxMenu->curBox = 0;
    ChooseBoxMenu_PrintInfo();
}

static void ChooseBoxMenu_MoveLeft(void)
{
    sChooseBoxMenu->curBox = (sChooseBoxMenu->curBox == 0 ? TOTAL_BOXES_COUNT - 1 : sChooseBoxMenu->curBox - 1);
    ChooseBoxMenu_PrintInfo();
}

static void ChooseBoxMenu_PrintInfo(void)
{
    u8 numBoxMonsText[16];
    struct WindowTemplate template;
    u8 windowId;
    u8 *boxName = GetBoxNamePtr(sChooseBoxMenu->curBox);
    u8 numInBox = CountMonsInBox(sChooseBoxMenu->curBox);
    u32 winTileData;
    s32 center;

    memset(&template, 0, sizeof(template));
    template.width = 8;
    template.height = 4;

    windowId = AddWindow(&template);
    FillWindowPixelBuffer(windowId, PIXEL_FILL(4));

    // Print box name
    center = GetStringCenterAlignXOffset(FONT_NORMAL, boxName, 64);
    AddTextPrinterParameterized3(windowId, FONT_NORMAL, center, 1, sChooseBoxMenu_TextColors, TEXT_SKIP_DRAW, boxName);

    // Print #/30 for number of Pokémon in the box
    ConvertIntToDecimalStringN(numBoxMonsText, numInBox, STR_CONV_MODE_RIGHT_ALIGN, 2);
    StringAppend(numBoxMonsText, sText_OutOf30);
    center = GetStringCenterAlignXOffset(FONT_NORMAL, numBoxMonsText, 64);
    AddTextPrinterParameterized3(windowId, FONT_NORMAL, center, 17, sChooseBoxMenu_TextColors, TEXT_SKIP_DRAW, numBoxMonsText);

    winTileData = GetWindowAttribute(windowId, WINDOW_TILE_DATA);
    CpuCopy32((void *)winTileData, (void *)OBJ_VRAM0 + OBJ_PLTT_OFFSET + (GetSpriteTileStartByTag(GFXTAG_CHOOSE_BOX_MENU) * 32), 1024);

    RemoveWindow(windowId);
}

//------------------------------------------------------------------------------
//  SECTION: Main tasks
//
//  Below are the main task callbacks that handle the primary actions the
//  player can take in the PC. The most 'important' of these tasks is the
//  primary one, Task_PokeStorageMain. Also included are some basic
//  initialization functions.
//------------------------------------------------------------------------------

static void VBlankCB_PokeStorage(void)
{
    // swap in palette
    if (sPaletteSwapBuffer && sStorage->swapInPalDst) 
    {
        CpuFastCopy(&sStorage->swapInPal[0], sStorage->swapInPalDst, PLTT_SIZE_4BPP);
        sStorage->swapInPalDst = NULL;
    }
    LoadOam();
    ProcessSpriteCopyRequests();
    // Instead of transferring the entire palette buffer, transfer bg and non-dynamic palettes
    if (sPaletteSwapBuffer && !gPaletteFade.bufferTransferDisabled && !gPaletteFade.active && !sStorage->transferWholePlttFrames)
    {
        RequestDma3Copy(gPlttBufferFaded, (void*)PLTT, 32*17, 0);
        // Skip the 12-1 palettes that are being dynamically swapped anyway
        RequestDma3Copy(&gPlttBufferFaded[PLTT_ID(28)], (void*) 0x05000380, 32*4, 0);
    }
    else
    {
        if (sStorage && sStorage->transferWholePlttFrames > 0)
            sStorage->transferWholePlttFrames--;
        TransferPlttBuffer();
    }
}

static s8 SwapInPalNextVBlank(void *palette, void *dst)
{
    if (!sStorage || gMain.vblankCallback != VBlankCB_PokeStorage)
        return -1;
    CpuFastCopy(palette, &sStorage->swapInPal[0], PLTT_SIZE_4BPP);
    sStorage->swapInPalDst = dst;
    return 0;
}

static void CB2_PokeStorage(void)
{
    RunTasks();
    DoScheduledBgTilemapCopiesToVram();
    ScrollBackground();
    AnimateSprites();
    BuildOamBuffer();
}

void EnterPokeStorage(u8 boxOption)
{
    ResetTasks();
    sCurrentBoxOption = boxOption;
    sStorage = Alloc(sizeof(*sStorage));
    sPaletteSwapBuffer = AllocZeroed(32*30);
    allocCount++;
    if (sStorage == NULL || sPaletteSwapBuffer == NULL) 
    {
        SetMainCallback2(CB2_ExitPokeStorage);
    }
    else
    {
        sStorage->transferWholePlttFrames = 0;
        sStorage->chooseBoxSwapPal[0] = 0;
        sStorage->boxOption = boxOption;
        sStorage->isReopening = FALSE;
        sMovingItemId = ITEM_NONE;
        sStorage->state = 0;
        sStorage->taskId = CreateTask(Task_InitPokeStorage, 3);
        sLastUsedBox = StorageGetCurrentBox();
        SetMainCallback2(CB2_PokeStorage);
    }
}

void CB2_ReturnToPokeStorage(void)
{
    ResetTasks();
    sStorage = Alloc(sizeof(*sStorage));
    sPaletteSwapBuffer = AllocZeroed(32*30);
    allocCount++;
    if (sStorage == NULL || sPaletteSwapBuffer == NULL)
    {
        SetMainCallback2(CB2_ExitPokeStorage);
    }
    else
    {
        sStorage->boxOption = sCurrentBoxOption;
        sStorage->isReopening = TRUE;
        sStorage->state = 0;
        sStorage->taskId = CreateTask(Task_InitPokeStorage, 3);
        SetMainCallback2(CB2_PokeStorage);
    }
}

static void ResetAllBgCoords(void)
{
    SetGpuReg(REG_OFFSET_BG0HOFS, 0);
    SetGpuReg(REG_OFFSET_BG0VOFS, 0);
    SetGpuReg(REG_OFFSET_BG1HOFS, 0);
    SetGpuReg(REG_OFFSET_BG1VOFS, 0);
    SetGpuReg(REG_OFFSET_BG2HOFS, 0);
    SetGpuReg(REG_OFFSET_BG2VOFS, 0);
    SetGpuReg(REG_OFFSET_BG3HOFS, 0);
    SetGpuReg(REG_OFFSET_BG3VOFS, 0);
}

static void ResetForPokeStorage(void)
{
    ResetPaletteFade();
    ResetSpriteData();
    FreeSpriteTileRanges();
    FreeAllSpritePalettes();
    ClearDma3Requests();
    gReservedSpriteTileCount = 640;
    gKeyRepeatStartDelay = 20;
    ClearScheduledBgCopiesToVram();
    TilemapUtil_Init(TILEMAPID_COUNT);
    TilemapUtil_SetMap(TILEMAPID_PKMN_DATA, 1, sPkmnData_Tilemap, 8, 4);
    TilemapUtil_SetPos(TILEMAPID_PKMN_DATA, 1, 0);
}

static void InitStartingPosData(void)
{
    ClearSavedCursorPos();
    sInPartyMenu = (sStorage->boxOption == OPTION_DEPOSIT);
    sDepositBoxId = 0;
}

static void SetMonIconTransparency(void)
{
    if (sStorage->boxOption == OPTION_MOVE_ITEMS)
    {
        SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT2_ALL | BLDCNT_EFFECT_BLEND);
        SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(7, 11));
    }
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_BG_ALL_ON | DISPCNT_OBJ_1D_MAP);
}

static void SetPokeStorageTask(TaskFunc newFunc)
{
    gTasks[sStorage->taskId].func = newFunc;
    sStorage->state = 0;
}

// Manages swapping palettes mid draw to make all icon palettes appear
static void HBlankCB_PokeStorage(void)
{
    u8 vCount = REG_VCOUNT;
    u32 i;
    if (vCount >= DISPLAY_HEIGHT || !sPaletteSwapBuffer || (gPaletteFade.active && gPaletteFade.y == 16 && gPaletteFade.mode == 2)) // HARDWARE_FADE
        return;
    // For each row in the pc box
    for (i = 0; i < IN_BOX_ROWS; i++)
    {
        if (vCount == 28 - 8 + 24 * i)
        { 
            // -8 is to keep the right palette when being switched
            u32 position = PLTT_ID(IN_BOX_COLUMNS * i);
            u16* dst = (u16*) (OBJ_PLTT + PLTT_ID(i & 1 ? 7 : 1) * 2); // Points into Palette RAM directly
            u32 j;
            for (j = 0; j < IN_BOX_COLUMNS; j++, position += 16, dst += 16)
            {
                // If palette color is empty, skip
                if (!(sPaletteSwapBuffer[position] & 0x7FFF))
                    continue;
                CpuFastCopy(&sPaletteSwapBuffer[position], dst, PLTT_SIZE_4BPP);
            }
            break;
        }
    }
    if (vCount == 63 && sStorage && sStorage->chooseBoxSwapPal[0])
    // copy choose box palette
    {
        u16 *dst = (u16*) (OBJ_PLTT + PLTT_ID(0) * 2);
        CpuFastCopy(sStorage->chooseBoxSwapPal, dst, PLTT_SIZE_4BPP);
    }
}

static void DisableBoxMonDynamicPalette(u8 position, u8 count)
{
    u8 i;
    for (i = position; i < position+count && i < IN_BOX_COUNT; i++)
    {
        if (sPaletteSwapBuffer[PLTT_ID(i)] & 0x7FFF)
            sPaletteSwapBuffer[PLTT_ID(i)] = 0x8000;
    }
}

static void EnableBoxMonDynamicPalette(u8 position, u8 count)
{
    u8 i;
    for (i = position; i < position+count && i < IN_BOX_COUNT; i++)
    {
        if (sPaletteSwapBuffer[PLTT_ID(i)] == 0x8000)
            sPaletteSwapBuffer[PLTT_ID(i)] = 0x7FFF;
    }
}

static void Task_InitPokeStorage(u8 taskId)
{
    switch (sStorage->state)
    {
    case 0:
        SetVBlankCallback(NULL);
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        ResetForPokeStorage();
        FadeOutAndPlayNewMapMusic(MUS_HG_POKEWALKER, 4);
        if (sStorage->isReopening)
        {
            switch (sWhichToReshow)
            {
            case SCREEN_CHANGE_NAME_BOX - 1:
                // Return from naming box
                LoadSavedMovingMon();
                break;
            case SCREEN_CHANGE_SUMMARY_SCREEN - 1:
                // Return from summary screen
                SetSelectionAfterSummaryScreen();
                break;
            case SCREEN_CHANGE_ITEM_FROM_BAG - 1:
                // Return from bag menu
                GiveChosenBagItem();
                break;
            }
        }
        LoadPokeStorageMenuGfx();
        break;
    case 1:
        if (!InitPokeStorageWindows())
        {
            SetPokeStorageTask(Task_ChangeScreen);
            return;
        }
        break;
    case 2:
        PutWindowTilemap(WIN_DISPLAY_INFO);
        ClearWindowTilemap(WIN_MESSAGE);
        CpuFill32(0, (void *)VRAM, 512);
        LoadUserWindowBorderGfx(WIN_MESSAGE, 11, BG_PLTT_ID(14));
        break;
    case 3:
        ResetAllBgCoords();
        if (!sStorage->isReopening)
            InitStartingPosData();
        break;
    case 4:
        InitMonIconFields();
        if (!sStorage->isReopening)
            InitCursor();
        else
            InitCursorOnReopen();
        break;
    case 5:
        SetScrollingBackground();
        InitPokeStorageBg0();
        break;
    case 6:
        InitPalettesAndSprites();
        break;
    case 7:
        InitSupplementalTilemaps();
        break;
    case 8:
        CreateInitBoxTask(StorageGetCurrentBox());
        break;
    case 9:
        if (IsInitBoxActive())
            return;
        CreateItemIconSprites();
        InitCursorItemIcon();
        break;
    case 10:
        SetMonIconTransparency();
        if (!sStorage->isReopening)
        {
            BlendPalettes(PALETTES_ALL, 16, RGB_BLACK);
            SetPokeStorageTask(Task_ShowPokeStorage);
        }
        else
        {
            BlendPalettes(PALETTES_ALL, 16, RGB_BLACK);
            SetPokeStorageTask(Task_ReshowPokeStorage);
        }
        SetVBlankCallback(VBlankCB_PokeStorage);
        return;
    default:
        return;
    }

    sStorage->state++;
}

static void Task_ShowPokeStorage(u8 taskId)
{
    switch (sStorage->state)
    {
    case 0:
        PlaySE(SE_PC_LOGIN);
        ComputerScreenOpenEffect(20, 0, 1);
        EnableInterrupts(INTR_FLAG_VBLANK | INTR_FLAG_HBLANK);
        SetHBlankCallback(HBlankCB_PokeStorage);
        sStorage->state++;
        break;
    case 1:
        if (!IsComputerScreenOpenEffectActive())
            SetPokeStorageTask(Task_PokeStorageMain);
        break;
    }
}

static void Task_ReshowPokeStorage(u8 taskId)
{
    switch (sStorage->state)
    {
    case 0:
        BlendPalettes(PALETTES_ALL, 0, RGB_BLACK);
        BeginHardwarePaletteFade(0xFF, 0, 16, 0, TRUE);
        EnableInterrupts(INTR_FLAG_VBLANK | INTR_FLAG_HBLANK);
        SetHBlankCallback(HBlankCB_PokeStorage);
        sStorage->state++;
        break;
    case 1:
        if (!UpdatePaletteFade())
        {
            if (sWhichToReshow == SCREEN_CHANGE_ITEM_FROM_BAG - 1 && gSpecialVar_ItemId != ITEM_NONE)
            {
                PrintMessage(MSG_ITEM_IS_HELD);
                sStorage->state++;
            }
            else
            {
                SetPokeStorageTask(Task_PokeStorageMain);
            }
            SetMonIconTransparency(); // Set transparency after fade-in
        }
        break;
    case 2:
        if (!IsDma3ManagerBusyWithBgCopy() && JOY_NEW(A_BUTTON | B_BUTTON))
        {
            ClearBottomWindow();
            sStorage->state++;
        }
        break;
    case 3:
        if (!IsDma3ManagerBusyWithBgCopy())
            SetPokeStorageTask(Task_PokeStorageMain);
        break;
    }
}

// States for the outer switch in Task_PokeStorageMain
enum {
    MSTATE_HANDLE_INPUT,
    MSTATE_MOVE_CURSOR,
    MSTATE_SCROLL_BOX,
    MSTATE_WAIT_MSG,
    MSTATE_ERROR_LAST_PARTY_MON,
    MSTATE_ERROR_HAS_MAIL,
    MSTATE_WAIT_ERROR_MSG,
    MSTATE_SCROLL_BOX_ITEM,
    MSTATE_WAIT_ITEM_ANIM,
};

static void Task_PokeStorageMain(u8 taskId)
{
    switch (sStorage->state)
    {
    case MSTATE_HANDLE_INPUT:
        switch (HandleInput())
        {
        case INPUT_MOVE_CURSOR:
            PlaySE(SE_SELECT);
            sStorage->state = MSTATE_MOVE_CURSOR;
            break;
        case INPUT_SHOW_PARTY:
            if (sStorage->boxOption != OPTION_MOVE_MONS && sStorage->boxOption != OPTION_MOVE_ITEMS)
            {
                PrintMessage(MSG_WHICH_ONE_WILL_TAKE);
                sStorage->state = MSTATE_WAIT_MSG;
            }
            else
            {
                ClearSavedCursorPos();
                SetPokeStorageTask(Task_ShowPartyPokemon);
            }
            break;
        case INPUT_HIDE_PARTY:
            if (sStorage->boxOption == OPTION_MOVE_MONS)
            {
                if (IsMonBeingMoved() && ItemIsMail(sStorage->displayMonItemId))
                    sStorage->state = MSTATE_ERROR_HAS_MAIL;
                else
                    SetPokeStorageTask(Task_HidePartyPokemon);
            }
            else if (sStorage->boxOption == OPTION_MOVE_ITEMS)
            {
                SetPokeStorageTask(Task_HidePartyPokemon);
            }
            break;
        case INPUT_PRESSED_B:
            SetPokeStorageTask(Task_OnBPressed);
            break;
        case INPUT_BOX_OPTIONS:
            PlaySE(SE_SELECT);
            SetPokeStorageTask(Task_HandleBoxOptions);
            break;
        case INPUT_IN_MENU:
            SetPokeStorageTask(Task_OnSelectedMon);
            break;
        case INPUT_SCROLL_RIGHT:
            TintBackground();
            PlaySE(SE_SELECT);
            sStorage->newCurrBoxId = StorageGetCurrentBox() + 1;
            if (sStorage->newCurrBoxId >= TOTAL_BOXES_COUNT)
                sStorage->newCurrBoxId = 0;
            if (sStorage->boxOption != OPTION_MOVE_ITEMS)
            {
                SetUpScrollToBox(sStorage->newCurrBoxId);
                sStorage->state = MSTATE_SCROLL_BOX;
            }
            else
            {
                TryHideItemAtCursor();
                sStorage->state = MSTATE_SCROLL_BOX_ITEM;
            }
            break;
        case INPUT_SCROLL_LEFT:
            TintBackground();
            PlaySE(SE_SELECT);
            sStorage->newCurrBoxId = StorageGetCurrentBox() - 1;
            if (sStorage->newCurrBoxId < 0)
                sStorage->newCurrBoxId = TOTAL_BOXES_COUNT - 1;
            if (sStorage->boxOption != OPTION_MOVE_ITEMS)
            {
                SetUpScrollToBox(sStorage->newCurrBoxId);
                sStorage->state = MSTATE_SCROLL_BOX;
            }
            else
            {
                TryHideItemAtCursor();
                sStorage->state = MSTATE_SCROLL_BOX_ITEM;
            }
            break;
        case INPUT_DEPOSIT:
            if (!IsRemovingLastPartyMon())
            {
                if (ItemIsMail(sStorage->displayMonItemId))
                {
                    sStorage->state = MSTATE_ERROR_HAS_MAIL;
                }
                else
                {
                    PlaySE(SE_SELECT);
                    SetPokeStorageTask(Task_DepositMenu);
                }
            }
            else
            {
                sStorage->state = MSTATE_ERROR_LAST_PARTY_MON;
            }
            break;
        case INPUT_MOVE_MON:
            if (IsRemovingLastPartyMon())
            {
                sStorage->state = MSTATE_ERROR_LAST_PARTY_MON;
            }
            else
            {
                PlaySE(SE_SELECT);
                SetPokeStorageTask(Task_MoveMon);
            }
            break;
        case INPUT_SHIFT_MON:
            if (!CanShiftMon())
            {
                sStorage->state = MSTATE_ERROR_LAST_PARTY_MON;
            }
            else
            {
                PlaySE(SE_SELECT);
                SetPokeStorageTask(Task_ShiftMon);
            }
            break;
        case INPUT_WITHDRAW:
            PlaySE(SE_SELECT);
            SetPokeStorageTask(Task_WithdrawMon);
            break;
        case INPUT_PLACE_MON:
            PlaySE(SE_SELECT);
            SetPokeStorageTask(Task_PlaceMon);
            break;
        case INPUT_TAKE_ITEM:
            PlaySE(SE_SELECT);
            SetPokeStorageTask(Task_TakeItemForMoving);
            break;
        case INPUT_GIVE_ITEM:
            PlaySE(SE_SELECT);
            SetPokeStorageTask(Task_GiveMovingItemToMon);
            break;
        case INPUT_SWITCH_ITEMS:
            PlaySE(SE_SELECT);
            SetPokeStorageTask(Task_SwitchSelectedItem);
            break;
        }
        break;
    case MSTATE_MOVE_CURSOR:
        if (!UpdateCursorPos())
        {
            if (sStorage->setMosaic)
                StartDisplayMonMosaicEffect();
            sStorage->state = MSTATE_HANDLE_INPUT;
        }
        break;
    case MSTATE_SCROLL_BOX:
        if (!ScrollToBox())
        {
            SetCurrentBox(sStorage->newCurrBoxId);
            if (!sInPartyMenu && !IsMonBeingMoved())
            {
                RefreshDisplayMon();
                StartDisplayMonMosaicEffect();
            }

            if (sStorage->boxOption == OPTION_MOVE_ITEMS)
            {
                TryShowItemAtCursor();
                sStorage->state = MSTATE_WAIT_ITEM_ANIM;
            }
            else
            {
                sStorage->state = MSTATE_HANDLE_INPUT;
            }
        }
        break;
    case MSTATE_WAIT_MSG:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            ClearBottomWindow();
            sStorage->state = MSTATE_HANDLE_INPUT;
        }
        break;
    case MSTATE_ERROR_LAST_PARTY_MON:
        PlaySE(SE_FAILURE);
        PrintMessage(MSG_LAST_POKE);
        sStorage->state = MSTATE_WAIT_ERROR_MSG;
        break;
    case MSTATE_ERROR_HAS_MAIL:
        PlaySE(SE_FAILURE);
        PrintMessage(MSG_PLEASE_REMOVE_MAIL);
        sStorage->state = MSTATE_WAIT_ERROR_MSG;
        break;
    case MSTATE_WAIT_ERROR_MSG:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            ClearBottomWindow();
            SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    case MSTATE_SCROLL_BOX_ITEM:
        if (!IsItemIconAnimActive())
        {
            SetUpScrollToBox(sStorage->newCurrBoxId);
            sStorage->state = MSTATE_SCROLL_BOX;
        }
        break;
    case MSTATE_WAIT_ITEM_ANIM:
        if (!IsItemIconAnimActive())
            sStorage->state = MSTATE_HANDLE_INPUT;
        break;
    }
}

static void Task_ShowPartyPokemon(u8 taskId)
{
    switch (sStorage->state)
    {
    case 0:
        SetUpDoShowPartyMenu();
        sStorage->state++;
        break;
    case 1:
        if (!DoShowPartyMenu())
            SetPokeStorageTask(Task_PokeStorageMain);
        break;
    }
}

static void Task_HidePartyPokemon(u8 taskId)
{
    switch (sStorage->state)
    {
    case 0:
        PlaySE(SE_SELECT);
        SetUpHidePartyMenu();
        sStorage->state++;
        break;
    case 1:
        if (!HidePartyMenu())
        {
            SetCursorBoxPosition(GetSavedCursorPos());
            sStorage->state++;
        }
        break;
    case 2:
        if (!UpdateCursorPos())
        {
            if (sStorage->setMosaic)
                StartDisplayMonMosaicEffect();
            SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    }
}

static void Task_OnSelectedMon(u8 taskId)
{
    switch (sStorage->state)
    {
    case 0:
        if (!IsDisplayMosaicActive())
        {
            PlaySE(SE_SELECT);
            if (sStorage->boxOption != OPTION_MOVE_ITEMS)
                PrintMessage(MSG_IS_SELECTED);
            else if (IsMovingItem() || sStorage->displayMonItemId != ITEM_NONE)
                PrintMessage(MSG_IS_SELECTED2);
            else
                PrintMessage(MSG_GIVE_TO_MON);

            AddMenu();
            sStorage->state = 1;
        }
        break;
    case 1:
        if (!IsMenuLoading())
            sStorage->state = 2;
        break;
    case 2:
        switch (HandleMenuInput())
        {
        case MENU_B_PRESSED:
        case MENU_CANCEL:
            ClearBottomWindow();
            SetPokeStorageTask(Task_PokeStorageMain);
            break;
        case MENU_MOVE:
            if (IsRemovingLastPartyMon())
            {
                sStorage->state = 3;
            }
            else
            {
                PlaySE(SE_SELECT);
                ClearBottomWindow();
                SetPokeStorageTask(Task_MoveMon);
            }
            break;
        case MENU_PLACE:
            PlaySE(SE_SELECT);
            ClearBottomWindow();
            SetPokeStorageTask(Task_PlaceMon);
            break;
        case MENU_SHIFT:
            if (!CanShiftMon())
            {
                sStorage->state = 3;
            }
            else
            {
                PlaySE(SE_SELECT);
                ClearBottomWindow();
                SetPokeStorageTask(Task_ShiftMon);
            }
            break;
        case MENU_WITHDRAW:
            PlaySE(SE_SELECT);
            ClearBottomWindow();
            SetPokeStorageTask(Task_WithdrawMon);
            break;
        case MENU_STORE:
            if (IsRemovingLastPartyMon())
            {
                sStorage->state = 3;
            }
            else if (ItemIsMail(sStorage->displayMonItemId))
            {
                sStorage->state = 4;
            }
            else
            {
                PlaySE(SE_SELECT);
                ClearBottomWindow();
                SetPokeStorageTask(Task_DepositMenu);
            }
            break;
        case MENU_RELEASE:
            if (IsRemovingLastPartyMon())
            {
                sStorage->state = 3;
            }
            else if (sStorage->displayMonIsEgg)
            {
                sStorage->state = 5; // Cannot release an Egg.
            }
            else if (ItemIsMail(sStorage->displayMonItemId))
            {
                sStorage->state = 4;
            }
            else
            {
                PlaySE(SE_SELECT);
                SetPokeStorageTask(Task_ReleaseMon);
            }
            break;
        case MENU_SUMMARY:
            PlaySE(SE_SELECT);
            SetPokeStorageTask(Task_ShowMonSummary);
            break;
        case MENU_TAKE:
            PlaySE(SE_SELECT);
            SetPokeStorageTask(Task_TakeItemForMoving);
            break;
        case MENU_GIVE:
            PlaySE(SE_SELECT);
            SetPokeStorageTask(Task_GiveMovingItemToMon);
            break;
        case MENU_BAG:
            SetPokeStorageTask(Task_ItemToBag);
            break;
        case MENU_SWITCH:
            PlaySE(SE_SELECT);
            SetPokeStorageTask(Task_SwitchSelectedItem);
            break;
        case MENU_GIVE_2:
            PlaySE(SE_SELECT);
            SetPokeStorageTask(Task_GiveItemFromBag);
            break;
        case MENU_INFO:
            SetPokeStorageTask(Task_ShowItemInfo);
            break;
        }
        break;
    case 3:
        PlaySE(SE_FAILURE);
        PrintMessage(MSG_LAST_POKE);
        sStorage->state = 6;
        break;
    case 5:
        PlaySE(SE_FAILURE);
        PrintMessage(MSG_CANT_RELEASE_EGG);
        sStorage->state = 6;
        break;
    case 4:
        PlaySE(SE_FAILURE);
        PrintMessage(MSG_PLEASE_REMOVE_MAIL);
        sStorage->state = 6;
        break;
    case 6:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            ClearBottomWindow();
            SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    }
}

static void Task_MoveMon(u8 taskId)
{
    switch (sStorage->state)
    {
    case 0:
        InitMonPlaceChange(CHANGE_GRAB);
        sStorage->state++;
        break;
    case 1:
        if (!DoMonPlaceChange())
        {
            if (sInPartyMenu)
                SetPokeStorageTask(Task_HandleMovingMonFromParty);
            else
                SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    }
}

static void Task_PlaceMon(u8 taskId)
{
    switch (sStorage->state)
    {
    case 0:
        InitMonPlaceChange(CHANGE_PLACE);
        sStorage->state++;
        break;
    case 1:
        if (!DoMonPlaceChange())
        {
            if (sInPartyMenu)
                SetPokeStorageTask(Task_HandleMovingMonFromParty);
            else
                SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    }
}

static void Task_ShiftMon(u8 taskId)
{
    switch (sStorage->state)
    {
    case 0:
        InitMonPlaceChange(CHANGE_SHIFT);
        sStorage->state++;
        break;
    case 1:
        if (!DoMonPlaceChange())
        {
            StartDisplayMonMosaicEffect();
            SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    }
}

static void Task_WithdrawMon(u8 taskId)
{
    switch (sStorage->state)
    {
    case 0:
        if (CalculatePlayerPartyCount() == PARTY_SIZE)
        {
            PrintMessage(MSG_PARTY_FULL);
            sStorage->state = 1;
        }
        else
        {
            SaveCursorPos();
            InitMonPlaceChange(CHANGE_GRAB);
            sStorage->state = 2;
        }
        break;
    case 1:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            ClearBottomWindow();
            SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    case 2:
        if (!DoMonPlaceChange())
        {
            SetMovingMonPriority(1);
            SetUpDoShowPartyMenu();
            sStorage->state++;
        }
        break;
    case 3:
        if (!DoShowPartyMenu())
        {
            InitMonPlaceChange(CHANGE_PLACE);
            sStorage->state++;
        }
        break;
    case 4:
        if (!DoMonPlaceChange())
        {
            UpdatePartySlotColors();
            sStorage->state++;
        }
        break;
    case 5:
        SetPokeStorageTask(Task_HidePartyPokemon);
        break;
    }
}

static void Task_DepositMenu(u8 taskId)
{
    u8 boxId;

    switch (sStorage->state)
    {
    case 0:
        PrintMessage(MSG_DEPOSIT_IN_WHICH_BOX);
        LoadChooseBoxMenuGfx(&sStorage->chooseBoxMenu);
        CreateChooseBoxMenuSprites(sDepositBoxId);
        sStorage->state++;
        break;
    case 1:
        boxId = HandleChooseBoxMenuInput();
        switch (boxId)
        {
        case BOXID_NONE_CHOSEN:
            break;
        case BOXID_CANCELED:
            ClearBottomWindow();
            DestroyChooseBoxMenuSprites();
            FreeChooseBoxMenu();
            SetPokeStorageTask(Task_PokeStorageMain);
            break;
        default:
            if (TryStorePartyMonInBox(boxId))
            {
                sDepositBoxId = boxId;
                ClearBottomWindow();
                DestroyChooseBoxMenuSprites();
                FreeChooseBoxMenu();
                sStorage->state = 2;
            }
            else
            {
                PrintMessage(MSG_BOX_IS_FULL);
                sStorage->state = 4;
            }
            break;
        }
        break;
    case 2:
        CompactPartySlots();
        CompactPartySprites();
        sStorage->state++;
        break;
    case 3:
        if (GetNumPartySpritesCompacting() == 0)
        {
            ResetSelectionAfterDeposit();
            StartDisplayMonMosaicEffect();
            UpdatePartySlotColors();
            SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    case 4:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            PrintMessage(MSG_DEPOSIT_IN_WHICH_BOX);
            sStorage->state = 1;
        }
        break;
    }
}

static void Task_ReleaseMon(u8 taskId)
{
    switch (sStorage->state)
    {
    case 0:
        PrintMessage(MSG_RELEASE_POKE);
        ShowYesNoWindow(1);
        sStorage->state++;
        // fallthrough
    case 1:
        switch (Menu_ProcessInputNoWrapClearOnChoose())
        {
        case MENU_B_PRESSED:
        case  1: // No
            ClearBottomWindow();
            SetPokeStorageTask(Task_PokeStorageMain);
            break;
        case  0: // Yes
            ClearBottomWindow();
            InitCanReleaseMonVars();
            InitReleaseMon();
            sStorage->state++;
            break;
        }
        break;
    case 2:
        RunCanReleaseMon();
        if (!TryHideReleaseMon())
        {
            while (1)
            {
                s8 canRelease = RunCanReleaseMon();
                if (canRelease == TRUE)
                {
                    sStorage->state++;
                    break;
                }
                else if (!canRelease)
                {
                    sStorage->state = 8;
                    break;
                }
            }
        }
        break;
    case 3:
        ReleaseMon();
        RefreshDisplayMonData();
        PrintMessage(MSG_WAS_RELEASED);
        sStorage->state++;
        break;
    case 4:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            PrintMessage(MSG_WAS_RELEASED);
            sStorage->state++;
        }
        break;
    case 5:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            ClearBottomWindow();
            if (sInPartyMenu)
            {
                CompactPartySlots();
                CompactPartySprites();
                sStorage->state++;
            }
            else
            {
                sStorage->state = 7;
            }
        }
        break;
    case 6:
        if (GetNumPartySpritesCompacting() == 0)
        {
            RefreshDisplayMon();
            StartDisplayMonMosaicEffect();
            UpdatePartySlotColors();
            sStorage->state++;
        }
        break;
    case 7:
        SetPokeStorageTask(Task_PokeStorageMain);
        break;
    case 8:
        // Start "can't release" sequence
        PrintMessage(MSG_WAS_RELEASED);
        sStorage->state++;
        break;
    case 9:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            PrintMessage(MSG_SURPRISE);
            sStorage->state++;
        }
        break;
    case 10:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            ClearBottomWindow();
            ReshowReleaseMon();
            sStorage->state++;
        }
        break;
    case 11:
        if (!ResetReleaseMonSpritePtr())
        {
            TrySetCursorFistAnim();
            PrintMessage(MSG_CAME_BACK);
            sStorage->state++;
        }
        break;
    case 12:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            ClearBottomWindow();
            SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    }
}

static void Task_TakeItemForMoving(u8 taskId)
{
    switch (sStorage->state)
    {
    case 0:
        if (!ItemIsMail(sStorage->displayMonItemId))
        {
            ClearBottomWindow();
            sStorage->state++;
        }
        else
        {
            SetPokeStorageTask(Task_PrintCantStoreMail);
        }
        break;
    case 1:
        StartCursorAnim(CURSOR_ANIM_OPEN);
        TakeItemFromMon(sInPartyMenu ? CURSOR_AREA_IN_PARTY : CURSOR_AREA_IN_BOX, GetCursorPosition());
        sStorage->state++;
        sJustOpenedBag = FALSE;
        break;
    case 2:
        if (!IsItemIconAnimActive())
        {
            StartCursorAnim(CURSOR_ANIM_FIST);
            ClearBottomWindow();
            RefreshDisplayMon();
            PrintDisplayMonInfo();
            sStorage->state++;
        }
        break;
    case 3:
        if (!IsDma3ManagerBusyWithBgCopy())
            SetPokeStorageTask(Task_PokeStorageMain);
        break;
    }
}

static void Task_GiveMovingItemToMon(u8 taskId)
{
    switch (sStorage->state)
    {
    case 0:
        ClearBottomWindow();
        sStorage->state++;
        break;
    case 1:
        StartCursorAnim(CURSOR_ANIM_OPEN);
        GiveItemToMon(sInPartyMenu ? CURSOR_AREA_IN_PARTY : CURSOR_AREA_IN_BOX, GetCursorPosition());
        sStorage->state++;
        break;
    case 2:
        if (!IsItemIconAnimActive())
        {
            StartCursorAnim(CURSOR_ANIM_BOUNCE);
            RefreshDisplayMon();
            PrintDisplayMonInfo();
            PrintMessage(MSG_ITEM_IS_HELD);
            sStorage->state++;
        }
        break;
    case 3:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            ClearBottomWindow();
            sStorage->state++;
        }
        break;
    case 4:
        if (!IsDma3ManagerBusyWithBgCopy())
            SetPokeStorageTask(Task_PokeStorageMain);
        break;
    }
}

static void Task_ItemToBag(u8 taskId)
{
    switch (sStorage->state)
    {
    case 0:
        if (!AddBagItem(sStorage->displayMonItemId, 1))
        {
            PlaySE(SE_FAILURE);
            PrintMessage(MSG_BAG_FULL);
            sStorage->state = 3;
        }
        else
        {
            PlaySE(SE_SELECT);
            MoveItemFromMonToBag(sInPartyMenu ? CURSOR_AREA_IN_PARTY : CURSOR_AREA_IN_BOX, GetCursorPosition());
            sStorage->state = 1;
        }
        break;
    case 1:
        if (!IsItemIconAnimActive())
        {
            PrintMessage(MSG_PLACED_IN_BAG);
            sStorage->state = 2;
        }
        break;
    case 2:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            ClearBottomWindow();
            RefreshDisplayMon();
            PrintDisplayMonInfo();
            sStorage->state = 4;
        }
        break;
    case 4:
        if (!IsDma3ManagerBusyWithBgCopy())
            SetPokeStorageTask(Task_PokeStorageMain);
        break;
    case 3:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            ClearBottomWindow();
            SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    }
}

static void Task_SwitchSelectedItem(u8 taskId)
{
    switch (sStorage->state)
    {
    case 0:
        if (!ItemIsMail(sStorage->displayMonItemId))
        {
            ClearBottomWindow();
            sStorage->state++;
        }
        else
        {
            SetPokeStorageTask(Task_PrintCantStoreMail);
        }
        break;
    case 1:
        StartCursorAnim(CURSOR_ANIM_OPEN);
        SwapItemsWithMon(sInPartyMenu ? CURSOR_AREA_IN_PARTY : CURSOR_AREA_IN_BOX, GetCursorPosition());
        sStorage->state++;
        break;
    case 2:
        if (!IsItemIconAnimActive())
        {
            StartCursorAnim(CURSOR_ANIM_FIST);
            RefreshDisplayMon();
            PrintDisplayMonInfo();
            PrintMessage(MSG_CHANGED_TO_ITEM);
            sStorage->state++;
        }
        break;
    case 3:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            ClearBottomWindow();
            sStorage->state++;
        }
        break;
    case 4:
        if (!IsDma3ManagerBusyWithBgCopy())
            SetPokeStorageTask(Task_PokeStorageMain);
        break;
    }
}

static void Task_ShowItemInfo(u8 taskId)
{
    switch (sStorage->state)
    {
    case 0:
        ClearBottomWindow();
        sStorage->state++;
        break;
    case 1:
        if (!IsDma3ManagerBusyWithBgCopy())
        {
            PlaySE(SE_WIN_OPEN);
            PrintItemDescription();
            InitItemInfoWindow();
            sStorage->state++;
        }
        break;
    case 2:
        if (!UpdateItemInfoWindowSlideIn())
            sStorage->state++;
        break;
    case 3:
        if (!IsDma3ManagerBusyWithBgCopy())
            sStorage->state++;
        break;
    case 4:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            PlaySE(SE_WIN_OPEN);
            sStorage->state++;
        }
        break;
    case 5:
        if (!UpdateItemInfoWindowSlideOut())
            sStorage->state++;
        break;
    case 6:
        if (!IsDma3ManagerBusyWithBgCopy())
            SetPokeStorageTask(Task_PokeStorageMain);
        break;
    }
}

static void Task_CloseBoxWhileHoldingItem(u8 taskId)
{
    switch (sStorage->state)
    {
    case 0:
        PlaySE(SE_SELECT);
        PrintMessage(MSG_PUT_IN_BAG);
        ShowYesNoWindow(0);
        sStorage->state = 1;
        break;
    case 1:
        switch (Menu_ProcessInputNoWrapClearOnChoose())
        {
        case MENU_B_PRESSED:
        case 1: // No
            ClearBottomWindow();
            SetPokeStorageTask(Task_PokeStorageMain);
            break;
        case 0:// Yes
            if (AddBagItem(sStorage->movingItemId, 1) == TRUE)
            {
                ClearBottomWindow();
                sStorage->state = 3;
            }
            else
            {
                PrintMessage(MSG_BAG_FULL);
                sStorage->state = 2;
            }
            break;
        }
        break;
    case 2:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            ClearBottomWindow();
            sStorage->state = 5;
        }
        break;
    case 3:
        MoveItemFromCursorToBag();
        sStorage->state = 4;
        break;
    case 4:
        if (!IsItemIconAnimActive())
        {
            StartCursorAnim(CURSOR_ANIM_BOUNCE);
            SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    case 5:
        if (!IsDma3ManagerBusyWithBgCopy())
            SetPokeStorageTask(Task_PokeStorageMain);
        break;
    }
}

static void Task_HandleMovingMonFromParty(u8 taskId)
{
    switch (sStorage->state)
    {
    case 0:
        CompactPartySlots();
        CompactPartySprites();
        sStorage->state++;
        break;
    case 1:
        if (GetNumPartySpritesCompacting() == 0)
        {
            UpdatePartySlotColors();
            SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    }
}

static void Task_PrintCantStoreMail(u8 taskId)
{
    switch (sStorage->state)
    {
    case 0:
        PrintMessage(MSG_CANT_STORE_MAIL);
        sStorage->state++;
        break;
    case 1:
        if (!IsDma3ManagerBusyWithBgCopy())
            sStorage->state++;
        break;
    case 2:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            ClearBottomWindow();
            sStorage->state++;
        }
        break;
    case 3:
        if (!IsDma3ManagerBusyWithBgCopy())
            SetPokeStorageTask(Task_PokeStorageMain);
        break;
    }
}

// Handle options menu that shows when the box title bar is selected
static void Task_HandleBoxOptions(u8 taskId)
{
    switch (sStorage->state)
    {
    case 0:
        PrintMessage(MSG_WHAT_YOU_DO);
        AddMenu();
        sStorage->state++;
        break;
    case 1:
        if (IsMenuLoading())
            return;
        sStorage->state++;
    case 2:
        switch (HandleMenuInput())
        {
        case MENU_B_PRESSED:
        case MENU_CANCEL:
            ClearBottomWindow();
            SetPokeStorageTask(Task_PokeStorageMain);
            break;
        case MENU_NAME:
            PlaySE(SE_SELECT);
            SetPokeStorageTask(Task_NameBox);
            break;
        case MENU_JUMP:
            PlaySE(SE_SELECT);
            ClearBottomWindow();
            SetPokeStorageTask(Task_JumpBox);
            break;
        }
        break;
    }
}

static void Task_JumpBox(u8 taskId)
{
    switch (sStorage->state)
    {
    case 0:
        PrintMessage(MSG_JUMP_TO_WHICH_BOX);
        LoadChooseBoxMenuGfx(&sStorage->chooseBoxMenu);
        CreateChooseBoxMenuSprites(StorageGetCurrentBox());
        sStorage->state++;
        break;
    case 1:
        sStorage->newCurrBoxId = HandleChooseBoxMenuInput();
        switch (sStorage->newCurrBoxId)
        {
        case BOXID_NONE_CHOSEN:
            break;
        default:
            ClearBottomWindow();
            DestroyChooseBoxMenuSprites();
            FreeChooseBoxMenu();
            if (sStorage->newCurrBoxId == BOXID_CANCELED || sStorage->newCurrBoxId == StorageGetCurrentBox())
            {
                SetPokeStorageTask(Task_PokeStorageMain);
            }
            else
            {
                sStorage->state++;
            }
            break;
        }
        break;
    case 2:
        SetUpScrollToBox(sStorage->newCurrBoxId);
        sStorage->state++;
        break;
    case 3:
        if (!ScrollToBox())
        {
            SetCurrentBox(sStorage->newCurrBoxId);
            SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    }
}

static void Task_NameBox(u8 taskId)
{
    switch (sStorage->state)
    {
    case 0:
        SaveMovingMon();
        BeginHardwarePaletteFade(0xFF, 0, 0, 16, TRUE);
        sStorage->state++;
        break;
    case 1:
        if (gPaletteFade.y == 16) // blend last frame of hardware fade
            BlendPalettes(PALETTES_ALL, 16, RGB_BLACK);
        if (!UpdatePaletteFade())
        {
            SetHBlankCallback(NULL); // avoid palette flickering
            sWhichToReshow = SCREEN_CHANGE_NAME_BOX - 1;
            sStorage->screenChangeType = SCREEN_CHANGE_NAME_BOX;
            SetPokeStorageTask(Task_ChangeScreen);
        }
        break;
    }
}

static void Task_ShowMonSummary(u8 taskId)
{
    switch (sStorage->state)
    {
    case 0:
        InitSummaryScreenData();
        BeginHardwarePaletteFade(0xFF, 0, 0, 16, TRUE);
        sStorage->state++;
        break;
    case 1:
        if (gPaletteFade.y == 16) // blend last frame of hardware fade
            BlendPalettes(PALETTES_ALL, 16, RGB_BLACK);
        if (!UpdatePaletteFade())
        {
            SetHBlankCallback(NULL); // avoid palette flickering
            sWhichToReshow = SCREEN_CHANGE_SUMMARY_SCREEN - 1;
            sStorage->screenChangeType = SCREEN_CHANGE_SUMMARY_SCREEN;
            SetPokeStorageTask(Task_ChangeScreen);
        }
        break;
    }
}

static void Task_GiveItemFromBag(u8 taskId)
{
    switch (sStorage->state)
    {
    case 0:
        BeginHardwarePaletteFade(0xFF, 0, 0, 16, TRUE);
        sStorage->state++;
        break;
    case 1:
        if (gPaletteFade.y == 16) // blend last frame of hardware fade
            BlendPalettes(PALETTES_ALL, 16, RGB_BLACK);
        if (!UpdatePaletteFade())
        {
            SetHBlankCallback(NULL); // avoid palette flickering
            sWhichToReshow = SCREEN_CHANGE_ITEM_FROM_BAG - 1;
            sStorage->screenChangeType = SCREEN_CHANGE_ITEM_FROM_BAG;
            SetPokeStorageTask(Task_ChangeScreen);
            sJustOpenedBag = TRUE;
        }
        break;
    }
}

static void Task_OnBPressed(u8 taskId)
{
    u16 music = GetCurrLocationDefaultMusic();

    switch (sStorage->state)
    {
    case 0:
        if (IsMonBeingMoved())
        {
            if (OW_PC_PRESS_B < GEN_4)
            {
                PlaySE(SE_FAILURE);
                PrintMessage(MSG_HOLDING_POKE);
                sStorage->state = 1;
            }
            else if (CanPlaceMon())
            {
                PlaySE(SE_SELECT);
                SetPokeStorageTask(Task_PlaceMon);
            }
            else
            {
                SetPokeStorageTask(Task_PokeStorageMain);
            }
        }
        else if (IsMovingItem())
        {
            SetPokeStorageTask(Task_CloseBoxWhileHoldingItem);
        }
        else
        {
            PlaySE(SE_SELECT);
            PrintMessage(MSG_CONTINUE_BOX);
            ShowYesNoWindow(0);
            sStorage->state = 2;
        }
        break;
    case 1:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            ClearBottomWindow();
            SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    case 2:
        switch (Menu_ProcessInputNoWrapClearOnChoose())
        {
        case 0:
            ClearBottomWindow();
            SetPokeStorageTask(Task_PokeStorageMain);
            break;
        case 1:
        case MENU_B_PRESSED:
            FadeOutAndPlayNewMapMusic(music, 8);
            PlaySE(SE_PC_OFF);
            ClearBottomWindow();
            sStorage->state++;
            break;
        }
        break;
    case 3:
        sStorage->transferWholePlttFrames = -1;
        ComputerScreenCloseEffect(20, 0, 0);
        sStorage->state++;
        break;
    case 4:
        if (!IsComputerScreenCloseEffectActive())
        {
            SetHBlankCallback(NULL); // Need to null callback here to avoid flickering
            UpdateBoxToSendMons();
            gPlayerPartyCount = CalculatePlayerPartyCount();
            sStorage->screenChangeType = SCREEN_CHANGE_EXIT_BOX;
            SetPokeStorageTask(Task_ChangeScreen);
        }
        break;
    }
}

static void Task_ChangeScreen(u8 taskId)
{
    struct BoxPokemon *boxMons;
    u8 mode, monIndex, maxMonIndex;
    u8 screenChangeType = sStorage->screenChangeType;

    if (sStorage->boxOption == OPTION_MOVE_ITEMS && IsMovingItem() == TRUE)
        sMovingItemId = GetMovingItemId();
    else
        sMovingItemId = ITEM_NONE;

    switch (screenChangeType)
    {
    case SCREEN_CHANGE_EXIT_BOX:
    default:
        FreePokeStorageData();
        SetMainCallback2(CB2_ExitPokeStorage);
        break;
    case SCREEN_CHANGE_SUMMARY_SCREEN:
        boxMons = sStorage->summaryMon.box;
        monIndex = sStorage->summaryStartPos;
        maxMonIndex = sStorage->summaryMaxPos;
        mode = sStorage->summaryScreenMode;
        FreePokeStorageData();
        if (mode == SUMMARY_MODE_NORMAL && boxMons == &sSavedMovingMon.box)
            ShowPokemonSummaryScreenHandleDeoxys(mode, boxMons, monIndex, maxMonIndex, CB2_ReturnToPokeStorage);
        else
            ShowPokemonSummaryScreen(mode, boxMons, monIndex, maxMonIndex, CB2_ReturnToPokeStorage);
        break;
    case SCREEN_CHANGE_NAME_BOX:
        FreePokeStorageData();
        DoNamingScreen(NAMING_SCREEN_BOX, GetBoxNamePtr(StorageGetCurrentBox()), 0, 0, 0, CB2_ReturnToPokeStorage, 0);
        break;
    case SCREEN_CHANGE_ITEM_FROM_BAG:
        FreePokeStorageData();
        GoToBagMenu(ITEMMENULOCATION_PCBOX, 0, CB2_ReturnToPokeStorage);
        break;
    }

    DestroyTask(taskId);
}

static void GiveChosenBagItem(void)
{
    u16 itemId = gSpecialVar_ItemId;

    if (itemId != ITEM_NONE)
    {
        u8 pos = GetCursorPosition();
        if (sInPartyMenu)
        {
            SetMonData(&gPlayerParty[pos], MON_DATA_HELD_ITEM, &itemId);
        }
        else
        {
            SetCurrentBoxMonData(pos, MON_DATA_HELD_ITEM, &itemId);
        }

        RemoveBagItem(itemId, 1);
    }
}

static void FreePokeStorageData(void)
{
    TilemapUtil_Free();
    FREE_AND_SET_NULL(sStorage);
    FREE_AND_SET_NULL(sPaletteSwapBuffer);
    allocCount--;
    SetHBlankCallback(NULL);
    FreeAllWindowBuffers();
}

//------------------------------------------------------------------------------
//  SECTION: Misc
//
//  No real uniform section below. Misc functions including more initialization,
//  showing/hiding the party menu, updating the Close Box button, printing
//  messages, doing the mosaic effect when transitioning between Pokémon, etc.
//------------------------------------------------------------------------------
static void SetScrollingBackground(void)
{
    SetGpuReg(REG_OFFSET_BG3CNT, BGCNT_PRIORITY(3) | BGCNT_CHARBASE(3) | BGCNT_16COLOR | BGCNT_SCREENBASE(31));
    DecompressAndLoadBgGfxUsingHeap(3, sScrollingBg_Gfx, 0, 0, 0);
    LZ77UnCompVram(sScrollingBg_Tilemap, (void *)BG_SCREEN_ADDR(31));
}

static void ScrollBackground(void)
{
    ChangeBgX(3, 128, BG_COORD_ADD);
    ChangeBgY(3, 128, BG_COORD_SUB);
}

static const u16 sColoresStorage[] =
{
    RGB_BLACK,
    RGB_WHITE,
    RGB_RED,
    RGB_GREEN,
    RGB_BLUE,
    RGB_PURPLE,
    RGB_YELLOW,
    RGB_MAGENTA,
    RGB_CYAN,
    RGB_LIME_GREEN,
    RGB_AZUL_MARINO,
    RGB_NARANJA,
    RGB_AMARILLO_CLARO,
    RGB_AZUL_PALIDO,
    RGB_AZUL_OSCURO,
    RGB_VERDE_AMARILLENTO,
    RGB_ROSA_CLARO,
    RGB_ROJO_VIVO,
    RGB_MARRON,
    RGB_GRIS_CLARO,
    RGB_GRIS_OSCURO,
};

static void TintBackground(void)
{
    u8 coeffScrollBg = 4;
    u32 blendColorBg = RandomElement(RNG_NONE, sColoresStorage);

    BlendPalette(BG_PLTT_ID(3), 4, coeffScrollBg, blendColorBg);
}

static void LoadPokeStorageMenuGfx(void)
{
    InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
    DecompressAndLoadBgGfxUsingHeap(1, gStorageSystemMenu_Gfx, 0, 0, 0);
    LZ77UnCompWram(sDisplayMenu_Tilemap, sStorage->displayMenuTilemapBuffer);
    SetBgTilemapBuffer(1, sStorage->displayMenuTilemapBuffer);
    ShowBg(1);
    ScheduleBgCopyTilemapToVram(1);
}

static bool8 InitPokeStorageWindows(void)
{
    if (!InitWindows(sWindowTemplates))
    {
        return FALSE;
    }
    else
    {
        DeactivateAllTextPrinters();
        return TRUE;
    }
}

static void InitPalettesAndSprites(void)
{
    LoadPalette(sInterface_Pal, BG_PLTT_ID(0), PLTT_SIZE_4BPP);
    LoadPalette(sPkmnDataGray_Pal, BG_PLTT_ID(2), PLTT_SIZE_4BPP);
    LoadPalette(sTextWindows_Pal, BG_PLTT_ID(15), PLTT_SIZE_4BPP);
    LoadPalette(sScrollingBg_Pal, BG_PLTT_ID(3), PLTT_SIZE_4BPP);

    SetGpuReg(REG_OFFSET_BG1CNT, BGCNT_PRIORITY(1) | BGCNT_CHARBASE(1) | BGCNT_16COLOR | BGCNT_SCREENBASE(30));
    CreateDisplayMonSprite();
    RefreshDisplayMonData();
}

static void RefreshDisplayMonData(void)
{
    LoadDisplayMonGfx(sStorage->displayMonSpecies, sStorage->displayMonPersonality);
    PrintDisplayMonInfo();
    ScheduleBgCopyTilemapToVram(0);
}

static void StartDisplayMonMosaicEffect(void)
{
    RefreshDisplayMonData();
    if (sStorage->displayMonSprite)
    {
        sStorage->displayMonSprite->oam.mosaic = TRUE;
        sStorage->displayMonSprite->data[0] = 10;
        sStorage->displayMonSprite->data[1] = 1;
        sStorage->displayMonSprite->callback = SpriteCB_DisplayMonMosaic;
        SetGpuReg(REG_OFFSET_MOSAIC, (sStorage->displayMonSprite->data[0] << 12) | (sStorage->displayMonSprite->data[0] << 8));
    }
}

static u8 IsDisplayMosaicActive(void)
{
    return sStorage->displayMonSprite->oam.mosaic;
}

static void SpriteCB_DisplayMonMosaic(struct Sprite *sprite)
{
    sprite->data[0] -= sprite->data[1];
    if (sprite->data[0] < 0)
        sprite->data[0] = 0;
    SetGpuReg(REG_OFFSET_MOSAIC, (sprite->data[0] << 12) | (sprite->data[0] << 8));
    if (sprite->data[0] == 0)
    {
        sprite->oam.mosaic = FALSE;
        sprite->callback = SpriteCallbackDummy;
    }
}

static void CreateDisplayMonSprite(void)
{
    u16 i;
    u16 tileStart;
    u8 palSlot;
    u8 spriteId;
    struct SpriteSheet sheet = {sStorage->tileBuffer, MON_PIC_SIZE, GFXTAG_DISPLAY_MON};
    struct SpritePalette palette = {sStorage->displayMonPalBuffer, PALTAG_DISPLAY_MON};
    struct SpriteTemplate template = sSpriteTemplate_DisplayMon;

    for (i = 0; i < MON_PIC_SIZE; i++)
        sStorage->tileBuffer[i] = 0;
    for (i = 0; i < 16; i++)
        sStorage->displayMonPalBuffer[i] = 0;

    sStorage->displayMonSprite = NULL;

    do
    {
        tileStart = LoadSpriteSheet(&sheet);
        if (tileStart == 0)
            break;

        palSlot = LoadSpritePalette(&palette);
        if (palSlot == 0xFF)
            break;

        spriteId = CreateSprite(&template, 40, 48, 0);
        if (spriteId == MAX_SPRITES)
            break;

        sStorage->displayMonSprite = &gSprites[spriteId];
        sStorage->displayMonPalOffset = OBJ_PLTT_ID(palSlot);
        sStorage->displayMonTilePtr = (void *) OBJ_VRAM0 + tileStart * TILE_SIZE_4BPP;
    } while (0);

    if (sStorage->displayMonSprite == NULL)
    {
        FreeSpriteTilesByTag(GFXTAG_DISPLAY_MON);
        FreeSpritePaletteByTag(PALTAG_DISPLAY_MON);
    }
}

static void LoadDisplayMonGfx(u16 species, u32 pid)
{
    const struct CompressedSpritePalette *pal1, *pal2;
    if (sStorage->displayMonSprite == NULL)
        return;

    if (species == SPECIES_EGG)
    {
        pal1 = &gEgg1PaletteTable[sStorage->eggPalette[0]];
        pal2 = &gEgg2PaletteTable[sStorage->eggPalette[1]];
        LoadSpecialPokePic(sStorage->tileBuffer, species, pid, TRUE);
        LZ77UnCompWram(pal1->data, sStorage->displayMonPalBuffer);
        LZ77UnCompWram(pal2->data, gDecompressionBuffer);
        CpuFastCopy(sStorage->tileBuffer, sStorage->displayMonTilePtr, MON_PIC_SIZE);
        LoadPalette(sStorage->displayMonPalBuffer, sStorage->displayMonPalOffset, PLTT_SIZE_4BPP/2);
        LoadPalette(gDecompressionBuffer, sStorage->displayMonPalOffset + 8, PLTT_SIZE_4BPP/2);
        sStorage->displayMonSprite->invisible = FALSE;
    }
    else if (species != SPECIES_NONE)
    {
        LoadSpecialPokePic(sStorage->tileBuffer, species, pid, TRUE);
        CpuFastCopy(sStorage->tileBuffer, sStorage->displayMonTilePtr, MON_PIC_SIZE);
        LoadCompressedPaletteFast(sStorage->displayMonPalette, sStorage->displayMonPalOffset, PLTT_SIZE_4BPP);
        UniquePalette(sStorage->displayMonPalOffset, pid);
        CpuFastCopy(&gPlttBufferFaded[sStorage->displayMonPalOffset], &gPlttBufferUnfaded[sStorage->displayMonPalOffset], PLTT_SIZE_4BPP);
        sStorage->displayMonSprite->invisible = FALSE;
    }
    else
    {
        sStorage->displayMonSprite->invisible = TRUE;
    }
}

static void PrintDisplayMonInfo(void)
{
    FillWindowPixelBuffer(WIN_DISPLAY_INFO, PIXEL_FILL(1));
    if (sStorage->boxOption != OPTION_MOVE_ITEMS)
    {
        AddTextPrinterParameterized(WIN_DISPLAY_INFO, GetFontIdToFit(sStorage->displayMonNameText, FONT_NORMAL, 0, WindowWidthPx(WIN_DISPLAY_INFO) - 6), sStorage->displayMonNameText, 6, 0, TEXT_SKIP_DRAW, NULL);
        AddTextPrinterParameterized(WIN_DISPLAY_INFO, GetFontIdToFit(sStorage->displayMonNameText, FONT_SHORT, 0, WindowWidthPx(WIN_DISPLAY_INFO) - 12), sStorage->displayMonSpeciesName, 6, 15, TEXT_SKIP_DRAW, NULL);
        AddTextPrinterParameterized(WIN_DISPLAY_INFO, FONT_SHORT, sStorage->displayMonGenderLvlText, 10, 29, TEXT_SKIP_DRAW, NULL);
        AddTextPrinterParameterized(WIN_DISPLAY_INFO, GetFontIdToFit(sStorage->displayMonItemName, FONT_SMALL, 0, WindowWidthPx(WIN_DISPLAY_INFO) - 6), sStorage->displayMonItemName, 6, 43, TEXT_SKIP_DRAW, NULL);
    }
    else
    {
        AddTextPrinterParameterized(WIN_DISPLAY_INFO, GetFontIdToFit(sStorage->displayMonItemName, FONT_SMALL, 0, WindowWidthPx(WIN_DISPLAY_INFO) - 6), sStorage->displayMonItemName, 6, 0, TEXT_SKIP_DRAW, NULL);
        AddTextPrinterParameterized(WIN_DISPLAY_INFO, GetFontIdToFit(sStorage->displayMonNameText, FONT_NORMAL, 0, WindowWidthPx(WIN_DISPLAY_INFO) - 6), sStorage->displayMonNameText, 6, 13, TEXT_SKIP_DRAW, NULL);
        AddTextPrinterParameterized(WIN_DISPLAY_INFO, GetFontIdToFit(sStorage->displayMonSpeciesName, FONT_SHORT, 0, WindowWidthPx(WIN_DISPLAY_INFO) - 12), sStorage->displayMonSpeciesName, 6, 28, TEXT_SKIP_DRAW, NULL);
        AddTextPrinterParameterized(WIN_DISPLAY_INFO, FONT_SHORT, sStorage->displayMonGenderLvlText, 10, 42, TEXT_SKIP_DRAW, NULL);
    }

    CopyWindowToVram(WIN_DISPLAY_INFO, COPYWIN_GFX);
}

static void InitSupplementalTilemaps(void)
{
    LZ77UnCompWram(gStorageSystemPartyMenu_Tilemap, sStorage->partyMenuTilemapBuffer);
    LoadPalette(gStorageSystemPartyMenu_Pal, BG_PLTT_ID(1), PLTT_SIZE_4BPP);
    TilemapUtil_SetMap(TILEMAPID_PARTY_MENU, 1, sStorage->partyMenuTilemapBuffer, 12, 22);
    TilemapUtil_SetPos(TILEMAPID_PARTY_MENU, 10, 0);
    SetPartySlotTilemaps();
    if (sInPartyMenu)
    {
        CreatePartyMonsSprites(TRUE);
        TilemapUtil_Update(TILEMAPID_PARTY_MENU);
    }
    else
    {
        TilemapUtil_SetRect(TILEMAPID_PARTY_MENU, 0, 20, 12, 2);
        TilemapUtil_Update(TILEMAPID_PARTY_MENU);
    }

    ScheduleBgCopyTilemapToVram(1);
}

static void SetUpShowPartyMenu(void)
{
    sStorage->partyMenuY = 2;
    sStorage->partyMenuMoveTimer = 0;
    CreatePartyMonsSprites(FALSE);
}

static bool8 ShowPartyMenu(void)
{
    if (sStorage->partyMenuMoveTimer == 20)
        return FALSE;

    sStorage->partyMenuY++;
    TilemapUtil_Move(1);
    TilemapUtil_Update(TILEMAPID_PARTY_MENU);
    ScheduleBgCopyTilemapToVram(1);
    MovePartySprites(8);
    // Disable dynamic palettes for the first 3 slots of each row
    if (sStorage->partyMenuMoveTimer == 10) 
    {
        DisableBoxMonDynamicPalette(0 * 6, 3);
        DisableBoxMonDynamicPalette(1 * 6, 3);
    }
    else if (sStorage->partyMenuMoveTimer == 16)
    {
        DisableBoxMonDynamicPalette(2 * 6, 3);
        DisableBoxMonDynamicPalette(3 * 6, 3);
    }
    if (++sStorage->partyMenuMoveTimer == 20)
    {
        DisableBoxMonDynamicPalette(4 * 6, 3);
        sInPartyMenu = TRUE;
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

static void SetUpHidePartyMenu(void)
{
    sStorage->partyMenuY = 22;
    sStorage->partyMenuMoveTimer = 0;
    // Set moving sprite palette to currently displayed pokemon
    if (sStorage->movingMonSprite)
      sStorage->movingMonSprite->oam.paletteNum = IndexOfSpritePaletteTag(PALTAG_DISPLAY_MON);
    if (sStorage->boxOption == OPTION_MOVE_ITEMS)
        MoveHeldItemWithPartyMenu();
}

static bool8 HidePartyMenu(void)
{
    if (sStorage->partyMenuMoveTimer != 20)
    {
        sStorage->partyMenuY--;
        TilemapUtil_Move(-1);
        TilemapUtil_Update(TILEMAPID_PARTY_MENU);
        FillBgTilemapBufferRect_Palette0(1, 256, 10, sStorage->partyMenuY, 12, 1);
        MovePartySprites(-8);
        // Re-enable box palette swapping
        if (sStorage->partyMenuMoveTimer == 0) 
        {
            EnableBoxMonDynamicPalette(4 * 6, 3);
        }
        else if (sStorage->partyMenuMoveTimer == 5) 
        {
            EnableBoxMonDynamicPalette(3 * 6, 3);
        }
        else if (sStorage->partyMenuMoveTimer == 7) 
        {
            EnableBoxMonDynamicPalette(2 * 6, 3);
        }
        else if (sStorage->partyMenuMoveTimer == 11)
        {
            EnableBoxMonDynamicPalette(1 * 6, 3);
            EnableBoxMonDynamicPalette(0 * 6, 3);
        }
        if (++sStorage->partyMenuMoveTimer != 20)
        {
            ScheduleBgCopyTilemapToVram(1);
            return TRUE;
        }
        else
        {
            sInPartyMenu = FALSE;
            DestroyAllPartyMonIcons();
            CompactPartySlots();

            ScheduleBgCopyTilemapToVram(1);
            sStorage->transferWholePlttFrames = 0; // transfer only non-dynamic palettes
            return FALSE;
        }
    }

    return FALSE;
}

static void SetPartySlotTilemaps(void)
{
    u8 i;

    // Skips first party slot, it should always be drawn
    // as if it has a Pokémon in it
    for (i = 1; i < PARTY_SIZE; i++)
    {
        s32 species = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES);
        SetPartySlotTilemap(i, species != SPECIES_NONE);
    }
}

static void SetPartySlotTilemap(u8 partyId, bool8 hasMon)
{
    u16 i, j, index;
    const u16 *data;

    if (hasMon)
        data = sPartySlotFilled_Tilemap;
    else
        data = sPartySlotEmpty_Tilemap;

    index = 3 * (3 * (partyId - 1) + 1);
    index *= 4;
    index += 7;
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 4; j++)
            sStorage->partyMenuTilemapBuffer[index + j] = data[j];

        data += 4;
        index += 12;
    }
}

static void UpdatePartySlotColors(void)
{
    SetPartySlotTilemaps();
    TilemapUtil_SetRect(TILEMAPID_PARTY_MENU, 0, 0, 12, 22);
    TilemapUtil_Update(TILEMAPID_PARTY_MENU);
    ScheduleBgCopyTilemapToVram(1);
}

static void SetUpDoShowPartyMenu(void)
{
    sStorage->showPartyMenuState = 0;
    PlaySE(SE_WIN_OPEN);
    SetUpShowPartyMenu();
}

static bool8 DoShowPartyMenu(void)
{
    switch (sStorage->showPartyMenuState)
    {
    case 0:
        if (!ShowPartyMenu())
        {
            SetCursorInParty();
            sStorage->showPartyMenuState++;
        }
        break;
    case 1:
        if (!UpdateCursorPos())
        {
            if (sStorage->setMosaic)
                StartDisplayMonMosaicEffect();
            sStorage->showPartyMenuState++;
        }
        break;
    case 2:
        return FALSE;
    }
    return TRUE;
}

static void UpdateBoxToSendMons(void)
{
    if (sLastUsedBox != StorageGetCurrentBox())
    {
        VarSet(VAR_PC_BOX_TO_SEND_MON, StorageGetCurrentBox());
    }
}

static void InitPokeStorageBg0(void)
{
    SetGpuReg(REG_OFFSET_BG0CNT, BGCNT_PRIORITY(0) | BGCNT_CHARBASE(0) | BGCNT_SCREENBASE(29));
    LoadUserWindowBorderGfx(WIN_MESSAGE, 2, BG_PLTT_ID(13));
    FillBgTilemapBufferRect(0, 0, 0, 0, 32, 20, 17);
    CopyBgTilemapBufferToVram(0);
}

static void PrintMessage(u8 id)
{
    u8 *txtPtr;

    DynamicPlaceholderTextUtil_Reset();
    switch (sMessages[id].format)
    {
    case MSG_VAR_NONE:
        break;
    case MSG_VAR_MON_NAME:
        DynamicPlaceholderTextUtil_SetPlaceholderPtr(0, sStorage->displayMonName);
        break;
    case MSG_VAR_RELEASE_MON:
        DynamicPlaceholderTextUtil_SetPlaceholderPtr(0, sStorage->releaseMonName);
        break;
    case MSG_VAR_ITEM_NAME:
        if (IsMovingItem())
            txtPtr = StringCopy(sStorage->itemName, GetMovingItemName());
        else
            txtPtr = StringCopy(sStorage->itemName, sStorage->displayMonItemName);

        while (*(txtPtr - 1) == CHAR_SPACE)
            txtPtr--;

        *txtPtr = EOS;
        DynamicPlaceholderTextUtil_SetPlaceholderPtr(0, sStorage->itemName);
        break;
    }

    DynamicPlaceholderTextUtil_ExpandPlaceholders(sStorage->messageText, sMessages[id].text);
    FillWindowPixelBuffer(WIN_MESSAGE, PIXEL_FILL(1));
    AddTextPrinterParameterized(WIN_MESSAGE, FONT_NORMAL, sStorage->messageText, 0, 1, TEXT_SKIP_DRAW, NULL);
    DrawTextBorderOuter(WIN_MESSAGE, 2, 14);
    PutWindowTilemap(WIN_MESSAGE);
    CopyWindowToVram(WIN_MESSAGE, COPYWIN_GFX);
    ScheduleBgCopyTilemapToVram(0);
}

static void ShowYesNoWindow(s8 cursorPos)
{
    CreateYesNoMenu(&sYesNoWindowTemplate, 11, 14, 0);
    Menu_MoveCursorNoWrapAround(cursorPos);
}

static void ClearBottomWindow(void)
{
    ClearStdWindowAndFrameToTransparent(WIN_MESSAGE, FALSE);
    ScheduleBgCopyTilemapToVram(0);
}

static u8 GetCurrentBoxOption(void)
{
    return sCurrentBoxOption;
}

static void InitCursorItemIcon(void)
{
    if (!IsCursorOnBoxTitle())
    {
        if (sInPartyMenu)
            TryLoadItemIconAtPos(CURSOR_AREA_IN_PARTY, GetCursorPosition());
        else
            TryLoadItemIconAtPos(CURSOR_AREA_IN_BOX, GetCursorPosition());
    }

    if (sMovingItemId != ITEM_NONE)
    {
        InitItemIconInCursor(sMovingItemId);
        StartCursorAnim(CURSOR_ANIM_FIST);
    }
}

//------------------------------------------------------------------------------
//  SECTION: Pokémon sprites
//
//  The below functions generally handle the Pokémon icon sprites, including
//  moving them with a scrolling box, shifting the party sprites, and
//  animating released Pokémon.
//------------------------------------------------------------------------------

static void InitMonIconFields(void)
{
    u16 i;

    for (i = 0; i < MAX_MON_ICONS; i++)
        sStorage->numIconsPerSpecies[i] = 0;
    for (i = 0; i < MAX_MON_ICONS; i++)
        sStorage->iconSpeciesList[i] = SPECIES_NONE;
    for (i = 0; i < PARTY_SIZE; i++)
        sStorage->partySprites[i] = NULL;
    for (i = 0; i < IN_BOX_COUNT; i++)
        sStorage->boxMonsSprites[i] = NULL;

    sStorage->movingMonSprite = NULL;
}

static u8 GetMonIconPriorityByCursorPos(void)
{
    return (IsCursorInBox() ? 2 : 1);
}

// Only called when returning from naming, etc while holding a pokemon
// Its palette is therefore always the same as the displayed pokemon's
static void CreateMovingMonIcon(void)
{
    u32 personality = GetMonData(&sStorage->movingMon, MON_DATA_PERSONALITY);
    u16 species = GetMonData(&sStorage->movingMon, MON_DATA_SPECIES_OR_EGG);
    u8 priority = GetMonIconPriorityByCursorPos();

    sStorage->movingMonSprite = CreateMonIconSprite(species, personality, 0, 0, priority, 7);
    sStorage->movingMonSprite->oam.paletteNum = IndexOfSpritePaletteTag(PALTAG_DISPLAY_MON);
    sStorage->movingMonSprite->callback = SpriteCB_HeldMon;
}

static void SetBoxMonDynamicPalette(u8 boxId, u8 position)
{
    struct Pokemon *mon = (struct Pokemon *)&gPokemonStoragePtr->boxes[boxId][position];
    u16 species = GetMonData(mon, MON_DATA_SPECIES_OR_EGG);
    bool8 isShiny = GetMonData(mon, MON_DATA_IS_SHINY);
    u32 personality = GetMonData(mon, MON_DATA_PERSONALITY);
    const u32 *palette = GetMonSpritePalFromSpeciesAndPersonality(species, isShiny, personality);
    const struct CompressedSpritePalette *pal1, *pal2;

    if (GetMonData(mon, MON_DATA_IS_EGG))
    {
        pal1 = &gEgg1PaletteTable[gSpeciesInfo[GetMonData(mon, MON_DATA_SPECIES)].types[0]];
        pal2 = &gEgg2PaletteTable[gSpeciesInfo[GetMonData(mon, MON_DATA_SPECIES)].types[1]];
        LZ77UnCompWram(pal1->data, gEggDecompressionBuffer);
        CpuCopy16(gEggDecompressionBuffer, &sPaletteSwapBuffer[PLTT_ID(position)], PLTT_SIZE_4BPP);
        CpuCopy16(gEggDecompressionBuffer, &sPaletteSwapBuffer[PLTT_ID(position)], PLTT_SIZE_4BPP);
        LZ77UnCompWram(pal2->data, gEggDecompressionBuffer);
        CpuCopy16(gEggDecompressionBuffer, &sPaletteSwapBuffer[PLTT_ID(position) + 8], PLTT_SIZE_4BPP/2);
        CpuCopy16(gEggDecompressionBuffer, &sPaletteSwapBuffer[PLTT_ID(position) + 8], PLTT_SIZE_4BPP/2);
    }
    else
    {
        // Decompress species palette into swap buffer
        if (species == SPECIES_CASTFORM_NORMAL) //???
        { // needs more than 32 bytes of space; so decompress and copy
            LZ77UnCompWram(palette, gDecompressionBuffer);
            CpuFastCopy(gDecompressionBuffer, &sPaletteSwapBuffer[PLTT_ID(position)], PLTT_SIZE_4BPP);
        } 
        else
        {
            LZ77UnCompWram(palette, &sPaletteSwapBuffer[PLTT_ID(position)]);
            UniquePaletteBuffered(&sPaletteSwapBuffer[PLTT_ID(position)], GetMonData(mon, MON_DATA_PERSONALITY));
        }
    }
    sStorage->boxMonsSprites[position]->oam.paletteNum = ((position/6) & 1 ? 6 : 0) + (position % 6) + 1;
}

static void InitBoxMonSprites(u8 boxId)
{
    u8 boxPosition;
    u16 i, j, count;
    u16 species;
    u32 personality;

    count = 0;
    boxPosition = 0;

    // For each box slot, create a Pokémon icon if a species is present
    for (i = 0; i < IN_BOX_ROWS; i++)
    {
        for (j = 0; j < IN_BOX_COLUMNS; j++)
        {
            species = GetBoxMonDataAt(boxId, boxPosition, MON_DATA_SPECIES_OR_EGG);
            if (species != SPECIES_NONE)
            {
                personality = GetBoxMonDataAt(boxId, boxPosition, MON_DATA_PERSONALITY);
                sStorage->boxMonsSprites[count] = CreateMonIconSprite(species, personality, 8 * (3 * j) + 100, 8 * (3 * i) + 44, 2, 19 - j);
                SetBoxMonDynamicPalette(boxId, boxPosition);
                if (sInPartyMenu && j < 3) // if in 'DEPOSIT' mode, disable palette swapping for columns [0, 2]
                  DisableBoxMonDynamicPalette(boxPosition, 3);
            }
            else
            {
                sStorage->boxMonsSprites[count] = NULL;
            }
            boxPosition++;
            count++;
        }
    }

    // If in item mode, set all Pokémon icons with no item to be transparent
    if (sStorage->boxOption == OPTION_MOVE_ITEMS)
    {
        for (boxPosition = 0; boxPosition < IN_BOX_COUNT; boxPosition++)
        {
            if (GetBoxMonDataAt(boxId, boxPosition, MON_DATA_HELD_ITEM) == ITEM_NONE)
                sStorage->boxMonsSprites[boxPosition]->oam.objMode = ST_OAM_OBJ_BLEND;
        }
    }
}

static void CreateBoxMonIconAtPos(u8 boxPosition)
{
    u16 species = GetCurrentBoxMonData(boxPosition, MON_DATA_SPECIES_OR_EGG);

    if (species != SPECIES_NONE)
    {
        s16 x = 8 * (3 * (boxPosition % IN_BOX_COLUMNS)) + 100;
        s16 y = 8 * (3 * (boxPosition/IN_BOX_COLUMNS)) + 44;
        u32 personality = GetCurrentBoxMonData(boxPosition, MON_DATA_PERSONALITY);

        sStorage->boxMonsSprites[boxPosition] = CreateMonIconSprite(species, personality, x, y, 2, 19 - (boxPosition % IN_BOX_COLUMNS));
        SetBoxMonDynamicPalette(StorageGetCurrentBox(), boxPosition);
        // if in 'DEPOSIT' mode, disable palette swapping on columns under the party
        if (sInPartyMenu && (boxPosition % 6 < 3))
            DisableBoxMonDynamicPalette(boxPosition, 1);
        if (sStorage->boxOption == OPTION_MOVE_ITEMS)
            sStorage->boxMonsSprites[boxPosition]->oam.objMode = ST_OAM_OBJ_BLEND;
    }
}

#define sDistance      data[1]
#define sSpeed         data[2]
#define sScrollInDestX data[3]
#define sDelay         data[4]
#define sScrollOutX    data[5]

static void StartBoxMonIconsScrollOut(s16 speed)
{
    u16 i;

    for (i = 0; i < IN_BOX_COUNT; i++)
    {
        if (sStorage->boxMonsSprites[i] != NULL)
        {
            sStorage->boxMonsSprites[i]->sSpeed = speed;
            sStorage->boxMonsSprites[i]->sDelay = 1;
            sStorage->boxMonsSprites[i]->callback = SpriteCB_BoxMonIconScrollOut;
        }
    }
}

static void SpriteCB_BoxMonIconScrollOut(struct Sprite *sprite)
{
    if (sprite->sDelay != 0)
    {
        sprite->sDelay--;
    }
    else
    {
        // Icon moving
        sprite->x += sprite->sSpeed;
        sprite->sScrollOutX = sprite->x + sprite->x2;

        // Check if icon offscreen
        if (sprite->sScrollOutX <= 68 || sprite->sScrollOutX >= 252)
            sprite->callback = SpriteCallbackDummy;
    }
}

static void DestroyAllBoxMonIcons(void)
{
    u32 boxPosition;
    for (boxPosition = 0; boxPosition < IN_BOX_ROWS * IN_BOX_COLUMNS; boxPosition++)
    {
        if (sStorage->boxMonsSprites[boxPosition] != NULL)
        {
            DestroyBoxMonIcon(sStorage->boxMonsSprites[boxPosition]);
            sStorage->boxMonsSprites[boxPosition] = NULL;
        }
    }
}

#undef sDistance
#undef sSpeed
#undef sScrollInDestX
#undef sDelay
#undef sScrollOutX

static void InitBoxMonIconScroll(u8 boxId, s8 direction)
{
    sStorage->iconScrollState = 0;
    sStorage->scrollDirection = direction;
    sStorage->iconScrollDistance = 32;
    sStorage->iconScrollSpeed = -BOX_SCROLL_SPEED(direction);
    sStorage->iconScrollNumIncoming = 0;
    GetIncomingBoxMonData(boxId);
    if (direction > 0)
        sStorage->iconScrollCurColumn = 0;
    else
        sStorage->iconScrollCurColumn = IN_BOX_COLUMNS - 1;

    sStorage->iconScrollPos = (24 * sStorage->iconScrollCurColumn) + 100;
    StartBoxMonIconsScrollOut(sStorage->iconScrollSpeed);
}

static bool32 UpdateBoxMonIconScroll(u32 boxId)
{
    switch (sStorage->iconScrollState)
    {
    case 0:
        DestroyAllBoxMonIcons();
        sStorage->iconScrollState++;
        break;
    case 1:
        InitBoxMonSprites(boxId);
        sStorage->iconScrollState++;
        break;
    case 2:
        sStorage->iconScrollDistance = 0;
        return FALSE;
    }

    return TRUE;
}

static void GetIncomingBoxMonData(u8 boxId)
{
    s32 i, j, boxPosition;

    boxPosition = 0;
    for (i = 0; i < IN_BOX_ROWS; i++)
    {
        for (j = 0; j < IN_BOX_COLUMNS; j++)
        {
            sStorage->boxSpecies[boxPosition] = GetBoxMonDataAt(boxId, boxPosition, MON_DATA_SPECIES_OR_EGG);
            if (sStorage->boxSpecies[boxPosition] != SPECIES_NONE)
                sStorage->boxPersonalities[boxPosition] = GetBoxMonDataAt(boxId, boxPosition, MON_DATA_PERSONALITY);
            boxPosition++;
        }
    }

    sStorage->incomingBoxId = boxId;
}

static void SetBoxMonIconObjMode(u8 boxPosition, u8 objMode)
{
    if (sStorage->boxMonsSprites[boxPosition] != NULL)
        sStorage->boxMonsSprites[boxPosition]->oam.objMode = objMode;
}

static void CreatePartyMonsSprites(bool8 visible)
{
    u16 i, count;
    u16 species = GetMonData(&gPlayerParty[0], MON_DATA_SPECIES_OR_EGG);
    u32 personality = GetMonData(&gPlayerParty[0], MON_DATA_PERSONALITY);
    u8 paletteNum;
    const struct CompressedSpritePalette *pal1, *pal2;

    sStorage->transferWholePlttFrames = -1; // keep transferring entire palette buffer until done with party menu
    sStorage->partySprites[0] = CreateMonIconSprite(species, personality, 104, 64, 1, 12);
    if (GetMonData(&gPlayerParty[0], MON_DATA_IS_EGG))
    {
        pal1 = &gEgg1PaletteTable[gSpeciesInfo[GetMonData(&gPlayerParty[0], MON_DATA_SPECIES)].types[0]];
        pal2 = &gEgg2PaletteTable[gSpeciesInfo[GetMonData(&gPlayerParty[0], MON_DATA_SPECIES)].types[1]];
        LZ77UnCompWram(pal1->data, gEggDecompressionBuffer);
        CpuCopy16(gEggDecompressionBuffer, &gPlttBufferUnfaded[OBJ_PLTT_ID(1)], PLTT_SIZE_4BPP);
        CpuCopy16(gEggDecompressionBuffer, &gPlttBufferFaded[OBJ_PLTT_ID(1)], PLTT_SIZE_4BPP);
        LZ77UnCompWram(pal2->data, gEggDecompressionBuffer);
        CpuCopy16(gEggDecompressionBuffer, &gPlttBufferUnfaded[OBJ_PLTT_ID(1) + 8], PLTT_SIZE_4BPP);
        CpuCopy16(gEggDecompressionBuffer, &gPlttBufferFaded[OBJ_PLTT_ID(1) + 8], PLTT_SIZE_4BPP);
    }
    else
    {
        LoadCompressedPaletteFast(GetMonFrontSpritePal(&gPlayerParty[0]), OBJ_PLTT_ID(1), PLTT_SIZE_4BPP);
        UniquePalette(OBJ_PLTT_ID(1), personality);
        CpuFastCopy(&gPlttBufferFaded[OBJ_PLTT_ID(1)], &gPlttBufferUnfaded[OBJ_PLTT_ID(1)], PLTT_SIZE_4BPP);
    }
    sStorage->partySprites[0]->oam.paletteNum = 1;
    count = 1;
    for (i = 1; i < PARTY_SIZE; i++)
    {
        species = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG);
        paletteNum = (i >= 3 ? i + 3 : i) + 1;
        if (GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
        {
            pal1 = &gEgg1PaletteTable[gSpeciesInfo[GetMonData(&gPlayerParty[i], MON_DATA_SPECIES)].types[0]];
            pal2 = &gEgg2PaletteTable[gSpeciesInfo[GetMonData(&gPlayerParty[i], MON_DATA_SPECIES)].types[1]];
            personality = GetMonData(&gPlayerParty[i], MON_DATA_PERSONALITY);
            sStorage->partySprites[i] = CreateMonIconSprite(species, personality, 152,  8 * (3 * (i - 1)) + 16, 1, 12);
            LZ77UnCompWram(pal1->data, gEggDecompressionBuffer);
            CpuCopy16(gEggDecompressionBuffer, &gPlttBufferUnfaded[OBJ_PLTT_ID(paletteNum)], PLTT_SIZE_4BPP);
            CpuCopy16(gEggDecompressionBuffer, &gPlttBufferFaded[OBJ_PLTT_ID(paletteNum)], PLTT_SIZE_4BPP);
            LZ77UnCompWram(pal2->data, gEggDecompressionBuffer);
            CpuCopy16(gEggDecompressionBuffer, &gPlttBufferUnfaded[OBJ_PLTT_ID(paletteNum) + 8], PLTT_SIZE_4BPP);
            CpuCopy16(gEggDecompressionBuffer, &gPlttBufferFaded[OBJ_PLTT_ID(paletteNum) + 8], PLTT_SIZE_4BPP);
            sStorage->partySprites[i]->oam.paletteNum = paletteNum;
            count++;
        }
        else
        {
            if (species != SPECIES_NONE)
            {
                personality = GetMonData(&gPlayerParty[i], MON_DATA_PERSONALITY);
                sStorage->partySprites[i] = CreateMonIconSprite(species, personality, 152,  8 * (3 * (i - 1)) + 16, 1, 12);
                LoadCompressedPaletteFast(GetMonFrontSpritePal(&gPlayerParty[i]), OBJ_PLTT_ID(paletteNum), PLTT_SIZE_4BPP);
                UniquePalette(OBJ_PLTT_ID(paletteNum), personality);
                CpuFastCopy(&gPlttBufferFaded[OBJ_PLTT_ID(paletteNum)], &gPlttBufferUnfaded[OBJ_PLTT_ID(paletteNum)], PLTT_SIZE_4BPP);
                sStorage->partySprites[i]->oam.paletteNum = paletteNum;
                count++;
            }
            else
            {
                sStorage->partySprites[i] = NULL;
            }
        }
    }

    if (!visible)
    {
        for (i = 0; i < count; i++)
        {
            sStorage->partySprites[i]->y -= DISPLAY_HEIGHT;
            sStorage->partySprites[i]->invisible = TRUE;
        }
    }

    if (sStorage->boxOption == OPTION_MOVE_ITEMS)
    {
        for (i = 0; i < PARTY_SIZE; i++)
        {
            if (sStorage->partySprites[i] != NULL && GetMonData(&gPlayerParty[i], MON_DATA_HELD_ITEM) == ITEM_NONE)
                sStorage->partySprites[i]->oam.objMode = ST_OAM_OBJ_BLEND;
        }
    }
}

static void CompactPartySprites(void)
{
    u16 i, targetSlot;

    sStorage->numPartyToCompact = 0;
    for (i = 0, targetSlot = 0; i < PARTY_SIZE; i++)
    {
        if (sStorage->partySprites[i] != NULL)
        {
            if (i != targetSlot)
            {
                MovePartySpriteToNextSlot(sStorage->partySprites[i], targetSlot);
                sStorage->partySprites[i] = NULL;
                sStorage->numPartyToCompact++;
            }
            targetSlot++;
        }
    }
}

static u8 GetNumPartySpritesCompacting(void)
{
    return sStorage->numPartyToCompact;
}

#define sPartyId   data[1]
#define sMonX      data[2]
#define sMonY      data[3]
#define sSpeedX    data[4]
#define sSpeedY    data[5]
#define sMoveSteps data[6]

static void MovePartySpriteToNextSlot(struct Sprite *sprite, u16 partyId)
{
    s16 x, y;

    sprite->sPartyId = partyId;
    if (partyId == 0)
        x = 104, y = 64;
    else
        x = 152, y = 8 * (3 * (partyId - 1)) + 16;

    sprite->sMonX = (u16)(sprite->x) * 8;
    sprite->sMonY = (u16)(sprite->y) * 8;
    sprite->sSpeedX = ((x * 8) - sprite->sMonX) / (8 / COMPACT_PARTY_SPRITES_DELTA);
    sprite->sSpeedY = ((y * 8) - sprite->sMonY) / (8 / COMPACT_PARTY_SPRITES_DELTA);
    sprite->sMoveSteps = (8 / COMPACT_PARTY_SPRITES_DELTA);
    sprite->callback = SpriteCB_MovePartyMonToNextSlot;
}

static void SpriteCB_MovePartyMonToNextSlot(struct Sprite *sprite)
{
    if (sprite->sMoveSteps != 0)
    {
        s16 x = sprite->sMonX += sprite->sSpeedX;
        s16 y = sprite->sMonY += sprite->sSpeedY;
        sprite->x = x/8u;
        sprite->y = y/8u;
        sprite->sMoveSteps--;
    }
    else
    {
        if (sprite->sPartyId == 0)
        {
            sprite->x = 104;
            sprite->y = 64;
        }
        else
        {
            sprite->x = 152;
            sprite->y = 8 * (3 * (sprite->sPartyId - 1)) + 16;
        }
        sprite->callback = SpriteCallbackDummy;
        sStorage->partySprites[sprite->sPartyId] = sprite;
        sStorage->numPartyToCompact--;
    }
}

#undef sPartyId
#undef sMonX
#undef sMonY
#undef sSpeedX
#undef sSpeedY
#undef sMoveSteps

static void DestroyMovingMonIcon(void)
{
    if (sStorage->movingMonSprite != NULL)
    {
        DestroyBoxMonIcon(sStorage->movingMonSprite);
        sStorage->movingMonSprite = NULL;
    }
}

static void MovePartySprites(s16 yDelta)
{
    u16 i, posY;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (sStorage->partySprites[i] != NULL)
        {
            sStorage->partySprites[i]->y += yDelta;
            posY = sStorage->partySprites[i]->y + sStorage->partySprites[i]->y2 + sStorage->partySprites[i]->centerToCornerVecY;
            posY += 16;
            if (posY > 192)
                sStorage->partySprites[i]->invisible = TRUE;
            else
                sStorage->partySprites[i]->invisible = FALSE;
        }
    }
}

static void DestroyPartyMonIcon(u8 partyId)
{
    if (sStorage->partySprites[partyId] != NULL)
    {
        DestroyBoxMonIcon(sStorage->partySprites[partyId]);
        sStorage->partySprites[partyId] = NULL;
    }
}

static void DestroyAllPartyMonIcons(void)
{
    u16 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (sStorage->partySprites[i] != NULL)
        {
            DestroyBoxMonIcon(sStorage->partySprites[i]);
            sStorage->partySprites[i] = NULL;
        }
    }
}

static void SetPartyMonIconObjMode(u8 partyId, u8 objMode)
{
    if (sStorage->partySprites[partyId] != NULL)
    {
        sStorage->partySprites[partyId]->oam.objMode = objMode;
    }
}

static void SetMovingMonSprite(u8 mode, u8 id)
{
    if (mode == MODE_PARTY)
    {
        sStorage->movingMonSprite = sStorage->partySprites[id];
        sStorage->partySprites[id] = NULL;
    }
    else if (mode == MODE_BOX)
    {
        sStorage->movingMonSprite = sStorage->boxMonsSprites[id];
        sStorage->boxMonsSprites[id] = NULL;
    }
    else
    {
        return;
    }

    sStorage->movingMonSprite->callback = SpriteCB_HeldMon;
    sStorage->movingMonSprite->oam.priority = GetMonIconPriorityByCursorPos();
    sStorage->movingMonSprite->subpriority = 7;
}

// Find a free party palette slot (1-3, 7-9)
u8 FindFreePartyPaletteSlot(void)
{
    u32 i, j, paletteNum;
    bool32 inUse;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        inUse = FALSE;
        paletteNum = (i >= 3 ? i + 3 : i) + 1;
        for (j = 0; j < PARTY_SIZE; j++)
            if (sStorage->partySprites[j]->oam.paletteNum == paletteNum)
                inUse = TRUE;
        if (!inUse)
            return paletteNum;
    }
    return 0xFF;
}

static void SetPlacedMonSprite(u8 boxId, u8 position)
{
    u32 paletteNum;
    const struct CompressedSpritePalette *pal1, *pal2;

    if (boxId == TOTAL_BOXES_COUNT) // party mon
    {
        sStorage->partySprites[position] = sStorage->movingMonSprite;
        sStorage->partySprites[position]->oam.priority = 1;
        sStorage->partySprites[position]->subpriority = 12;

        if (GetMonData(&gPlayerParty[position], MON_DATA_IS_EGG))
        {
            paletteNum = FindFreePartyPaletteSlot();
            pal1 = &gEgg1PaletteTable[gSpeciesInfo[GetMonData(&gPlayerParty[position], MON_DATA_SPECIES)].types[0]];
            pal2 = &gEgg2PaletteTable[gSpeciesInfo[GetMonData(&gPlayerParty[position], MON_DATA_SPECIES)].types[1]];
            LZ77UnCompWram(pal1->data, gEggDecompressionBuffer);
            CpuCopy16(gEggDecompressionBuffer, &gPlttBufferUnfaded[OBJ_PLTT_ID(paletteNum)], PLTT_SIZE_4BPP);
            CpuCopy16(gEggDecompressionBuffer, &gPlttBufferFaded[OBJ_PLTT_ID(paletteNum)], PLTT_SIZE_4BPP);
            LZ77UnCompWram(pal2->data, gEggDecompressionBuffer);
            CpuCopy16(gEggDecompressionBuffer, &gPlttBufferUnfaded[OBJ_PLTT_ID(paletteNum) + 8], PLTT_SIZE_4BPP);
            CpuCopy16(gEggDecompressionBuffer, &gPlttBufferFaded[OBJ_PLTT_ID(paletteNum) + 8], PLTT_SIZE_4BPP);
            sStorage->partySprites[position]->oam.paletteNum = paletteNum;
        }
        else // If currently using displayed mon palette, load party sprite palette into free party palette slot
        {
            if (sStorage->partySprites[position]->oam.paletteNum == IndexOfSpritePaletteTag(PALTAG_DISPLAY_MON)) 
            {
                paletteNum = FindFreePartyPaletteSlot();
                LoadCompressedPaletteFast(GetMonFrontSpritePal(&gPlayerParty[position]), OBJ_PLTT_ID(paletteNum), PLTT_SIZE_4BPP);
                UniquePalette(OBJ_PLTT_ID(paletteNum), GetMonData(&gPlayerParty[position], MON_DATA_PERSONALITY));
                CpuFastCopy(&gPlttBufferFaded[OBJ_PLTT_ID(paletteNum)], &gPlttBufferUnfaded[OBJ_PLTT_ID(paletteNum)], PLTT_SIZE_4BPP);
                sStorage->partySprites[position]->oam.paletteNum = paletteNum;
            }
        }
    }
    else
    {
        sStorage->boxMonsSprites[position] = sStorage->movingMonSprite;
        sStorage->boxMonsSprites[position]->oam.priority = 2;
        sStorage->boxMonsSprites[position]->subpriority = 19 - (position % IN_BOX_COLUMNS);
        SetBoxMonDynamicPalette(boxId, position);
    }
    sStorage->movingMonSprite->callback = SpriteCallbackDummy;
    sStorage->movingMonSprite = NULL;
}

static void SaveMonSpriteAtPos(u8 boxId, u8 position)
{
    if (boxId == TOTAL_BOXES_COUNT) // party mon
        sStorage->shiftMonSpritePtr = &sStorage->partySprites[position];
    else
        sStorage->shiftMonSpritePtr = &sStorage->boxMonsSprites[position];

    sStorage->movingMonSprite->callback = SpriteCallbackDummy;
    sStorage->shiftTimer = 0;
}

static bool8 MoveShiftingMons(void)
{
    if (sStorage->shiftTimer == 16)
        return FALSE;

    sStorage->shiftTimer += MON_HAND_DELTA;
    if (MON_HAND_DELTA >= 2 || sStorage->shiftTimer & 1)
    {
        (*sStorage->shiftMonSpritePtr)->y--;
        sStorage->movingMonSprite->y++;
    }

    (*sStorage->shiftMonSpritePtr)->x2 = gSineTable[sStorage->shiftTimer * 8]/16;
    sStorage->movingMonSprite->x2 = -(gSineTable[sStorage->shiftTimer * 8]/16);
    if (sStorage->shiftTimer == 8)
    {
        sStorage->movingMonSprite->oam.priority = (*sStorage->shiftMonSpritePtr)->oam.priority;
        sStorage->movingMonSprite->subpriority = (*sStorage->shiftMonSpritePtr)->subpriority;
        (*sStorage->shiftMonSpritePtr)->oam.priority = GetMonIconPriorityByCursorPos();
        (*sStorage->shiftMonSpritePtr)->subpriority = 7;
    }

    if (sStorage->shiftTimer == 16)
    {
        struct Sprite *sprite = sStorage->movingMonSprite;
        sStorage->movingMonSprite = (*sStorage->shiftMonSpritePtr);
        *sStorage->shiftMonSpritePtr = sprite;

        sStorage->movingMonSprite->callback = SpriteCB_HeldMon;
        (*sStorage->shiftMonSpritePtr)->callback = SpriteCallbackDummy;
    }

    return TRUE;
}

static void SetReleaseMon(u8 mode, u8 position)
{
    switch (mode)
    {
    case MODE_PARTY:
        sStorage->releaseMonSpritePtr = &sStorage->partySprites[position];
        break;
    case MODE_BOX:
        sStorage->releaseMonSpritePtr = &sStorage->boxMonsSprites[position];
        break;
    case MODE_MOVE:
        sStorage->releaseMonSpritePtr = &sStorage->movingMonSprite;
        break;
    default:
        return;
    }

    if (*sStorage->releaseMonSpritePtr != NULL)
    {
        InitSpriteAffineAnim(*sStorage->releaseMonSpritePtr);
        (*sStorage->releaseMonSpritePtr)->oam.affineMode = ST_OAM_AFFINE_NORMAL;
        (*sStorage->releaseMonSpritePtr)->affineAnims = sAffineAnims_ReleaseMon;
        StartSpriteAffineAnim(*sStorage->releaseMonSpritePtr, RELEASE_ANIM_RELEASE);
    }
}

static bool8 TryHideReleaseMonSprite(void)
{
    if (*sStorage->releaseMonSpritePtr == NULL
    || (*sStorage->releaseMonSpritePtr)->invisible)
        return FALSE;

    if ((*sStorage->releaseMonSpritePtr)->affineAnimEnded)
        (*sStorage->releaseMonSpritePtr)->invisible = TRUE;

    return TRUE;
}

static void DestroyReleaseMonIcon(void)
{
    if (*sStorage->releaseMonSpritePtr != NULL)
    {
        FreeOamMatrix((*sStorage->releaseMonSpritePtr)->oam.matrixNum);
        DestroyBoxMonIcon(*sStorage->releaseMonSpritePtr);
        *sStorage->releaseMonSpritePtr = NULL;
    }
}

static void ReshowReleaseMon(void)
{
    if (*sStorage->releaseMonSpritePtr != NULL)
    {
        (*sStorage->releaseMonSpritePtr)->invisible = FALSE;
        StartSpriteAffineAnim(*sStorage->releaseMonSpritePtr, RELEASE_ANIM_CAME_BACK);
    }
}

static bool8 ResetReleaseMonSpritePtr(void)
{
    if (sStorage->releaseMonSpritePtr == NULL)
        return FALSE;

    if ((*sStorage->releaseMonSpritePtr)->affineAnimEnded)
        sStorage->releaseMonSpritePtr = NULL;

    return TRUE;
}

static void SetMovingMonPriority(u8 priority)
{
    sStorage->movingMonSprite->oam.priority = priority;
}

static void SpriteCB_HeldMon(struct Sprite *sprite)
{
    sprite->x = sStorage->cursorSprite->x;
    sprite->y = sStorage->cursorSprite->y + sStorage->cursorSprite->y2 + 4;
}

static u16 TryLoadMonIconTiles(u16 species, u32 personality)
{
    u16 i, offset;

    // Treat female mons as a seperate species as they may have a different icon than males
    if (gSpeciesInfo[species].iconSpriteFemale != NULL && IsPersonalityFemale(species, personality))
        species |= 1 << 15;

    // Search icon list for this species
    for (i = 0; i < MAX_MON_ICONS; i++)
    {
        if (sStorage->iconSpeciesList[i] == species)
            break;
    }

    if (i == MAX_MON_ICONS)
    {
        // Species not present in the list
        // Find first empty spot in the list to put it
        for (i = 0; i < MAX_MON_ICONS; i++)
        {
            if (sStorage->iconSpeciesList[i] == 0)
                break;
        }

        // Failed to find an empty spot
        if (i == MAX_MON_ICONS)
            return 0xFFFF;
    }

    // Add species to icon list and load tiles
    sStorage->iconSpeciesList[i] = species;
    sStorage->numIconsPerSpecies[i]++;
    offset = PLTT_ID(i);
    species &= GENDER_MASK;
    CpuFastCopy(GetMonIconTiles(species, personality), (void *)(OBJ_VRAM0) + offset * TILE_SIZE_4BPP, 512);

    return offset;
}

static void RemoveSpeciesFromIconList(u16 species)
{
    u16 i;
    bool8 hasFemale = FALSE;

    for (i = 0; i < MAX_MON_ICONS; i++)
    {
        if (sStorage->iconSpeciesList[i] == (species | 0x8000))
        {
            hasFemale = TRUE;
            break;
        }
    }

    for (i = 0; i < MAX_MON_ICONS; i++)
    {
        if (sStorage->iconSpeciesList[i] == species && !hasFemale)
        {
            if (--sStorage->numIconsPerSpecies[i] == 0)
                sStorage->iconSpeciesList[i] = SPECIES_NONE;
            break;
        }
    }
}

static struct Sprite *CreateMonIconSprite(u16 species, u32 personality, s16 x, s16 y, u8 oamPriority, u8 subpriority)
{
    u16 tileNum;
    u8 spriteId;
    struct SpriteTemplate template = sSpriteTemplate_MonIcon;

    species = GetIconSpecies(species, personality);
    tileNum = TryLoadMonIconTiles(species, personality);
    if (tileNum == 0xFFFF)
        return NULL;

    spriteId = CreateSprite(&template, x, y, subpriority);
    if (spriteId == MAX_SPRITES)
    {
        RemoveSpeciesFromIconList(species);
        return NULL;
    }

    gSprites[spriteId].oam.tileNum = tileNum;
    gSprites[spriteId].oam.priority = oamPriority;
    gSprites[spriteId].data[0] = species;
    return &gSprites[spriteId];
}

static void DestroyBoxMonIcon(struct Sprite *sprite)
{
    RemoveSpeciesFromIconList(sprite->data[0]);
    DestroySprite(sprite);
}

//------------------------------------------------------------------------------
//  SECTION: General box
//
//  Some basic box functions, including initializing the box and scrolling.
//------------------------------------------------------------------------------
#define tState  data[0]
#define tBoxId  data[2]

static void CreateInitBoxTask(u8 boxId)
{
    u8 taskId = CreateTask(Task_InitBox, 2);

    gTasks[taskId].tBoxId = boxId;
}

static bool8 IsInitBoxActive(void)
{
    return FuncIsActiveTask(Task_InitBox);
}

static void Task_InitBox(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    switch (task->tState)
    {
    case 0:
        ShowBg(2);
        break;
    case 1:
        InitBoxTitle(task->tBoxId);
        InitBoxMonSprites(task->tBoxId);
        SetGpuReg(REG_OFFSET_BG2CNT, BGCNT_PRIORITY(2) | BGCNT_CHARBASE(2) | BGCNT_SCREENBASE(27) | BGCNT_TXT512x256);
        break;
    case 2:
        DestroyTask(taskId);
        break;
    default:
        task->tState = 0;
        return;
    }

    task->tState++;
}

#undef tState
#undef tBoxId

static void SetUpScrollToBox(u8 boxId)
{
    s8 direction = DetermineBoxScrollDirection(boxId);

    sStorage->scrollSpeed = BOX_SCROLL_SPEED((direction > 0) ? 1 : -1);
    sStorage->scrollTimer = 32 / BOX_SCROLL_SPEED_FACTOR;
    sStorage->scrollToBoxId = boxId;
    sStorage->scrollDirection = direction;
    sStorage->scrollState = 0;
}

static bool8 ScrollToBox(void)
{
    switch (sStorage->scrollState)
    {
    case 0:
        InitBoxMonIconScroll(sStorage->scrollToBoxId, sStorage->scrollDirection);
        CreateIncomingBoxTitle(sStorage->scrollToBoxId, sStorage->scrollDirection);
        break;
    case 1:
        if (!UpdateBoxMonIconScroll(sStorage->scrollToBoxId))
        {
            sStorage->scrollTimer = 0;
            CycleBoxTitleSprites();
            return FALSE;
        }
        return TRUE;
    }
    sStorage->scrollState++;
    return TRUE;
}

static s8 DetermineBoxScrollDirection(u8 boxId)
{
    u8 i;
    u8 currentBox = StorageGetCurrentBox();

    for (i = 0; currentBox != boxId; i++)
    {
        currentBox++;
        if (currentBox >= TOTAL_BOXES_COUNT)
            currentBox = 0;
    }

    return (i < TOTAL_BOXES_COUNT/2) ? 1 : -1;
}

//------------------------------------------------------------------------------
//  SECTION: Box Title
//------------------------------------------------------------------------------

static void InitBoxTitle(u8 boxId)
{
    u8 tagIndex;
    s16 x;
    u16 i;

    struct SpriteSheet spriteSheet = {sStorage->boxTitleTiles, 512, GFXTAG_BOX_TITLE};

    tagIndex = IndexOfSpritePaletteTag(PALTAG_BOX_TITLE);
    sStorage->boxTitlePalOffset = OBJ_PLTT_ID(tagIndex) + 14;

    StringCopyPadded(sStorage->boxTitleText, GetBoxNamePtr(boxId), 0, BOX_NAME_LENGTH);
    DrawTextWindowAndBufferTiles(sStorage->boxTitleText, sStorage->boxTitleTiles, 2);
    LoadSpriteSheet(&spriteSheet);
    x = GetBoxTitleBaseX(GetBoxNamePtr(boxId));

    // Title is split across two sprites
    for (i = 0; i < 2; i++)
    {
        u8 spriteId = CreateSprite(&sSpriteTemplate_BoxTitle, x + i * 32, 28, 24);
        sStorage->curBoxTitleSprites[i] = &gSprites[spriteId];
        StartSpriteAnim(sStorage->curBoxTitleSprites[i], i);
    }
    sStorage->boxTitleCycleId = 0;
}

// Sprite data for moving title text
#define sSpeed data[0]
// Flipped between incoming/outgoing for some reason
#define sIncomingX     data[1]
#define sIncomingDelay data[2]
#define sOutgoingDelay data[1]
#define sOutgoingX     data[2]

static void CreateIncomingBoxTitle(u8 boxId, s8 direction)
{
    s16 x, adjustedX;
    u16 i;
    struct SpriteSheet spriteSheet = {sStorage->boxTitleTiles, 512, GFXTAG_BOX_TITLE};
    struct SpriteTemplate template = sSpriteTemplate_BoxTitle;

    sStorage->boxTitleCycleId = (sStorage->boxTitleCycleId == 0);
    if (sStorage->boxTitleCycleId == 0)
    {
        spriteSheet.tag = GFXTAG_BOX_TITLE;
    }
    else
    {
        spriteSheet.tag = GFXTAG_BOX_TITLE_ALT;
        template.tileTag = GFXTAG_BOX_TITLE_ALT;
    }

    StringCopyPadded(sStorage->boxTitleText, GetBoxNamePtr(boxId), 0, BOX_NAME_LENGTH);
    DrawTextWindowAndBufferTiles(sStorage->boxTitleText, sStorage->boxTitleTiles, 2);
    LoadSpriteSheet(&spriteSheet);
    x = GetBoxTitleBaseX(GetBoxNamePtr(boxId));
    adjustedX = x;
    adjustedX += direction * 192;

    // Title is split across two sprites
    for (i = 0; i < 2; i++)
    {
        u8 spriteId = CreateSprite(&template, i * 32 + x, 28, 24);

        sStorage->nextBoxTitleSprites[i] = &gSprites[spriteId];
        sStorage->nextBoxTitleSprites[i]->sIncomingDelay = 1;
        sStorage->nextBoxTitleSprites[i]->callback = SpriteCB_IncomingBoxTitle;
        sStorage->nextBoxTitleSprites[i]->invisible = TRUE;
        StartSpriteAnim(sStorage->nextBoxTitleSprites[i], i);

        sStorage->curBoxTitleSprites[i]->sOutgoingDelay = 1;
        sStorage->curBoxTitleSprites[i]->callback = SpriteCB_OutgoingBoxTitle;
    }
}

static void CycleBoxTitleSprites(void)
{
    if (sStorage->boxTitleCycleId == 0)
        FreeSpriteTilesByTag(GFXTAG_BOX_TITLE_ALT);
    else
        FreeSpriteTilesByTag(GFXTAG_BOX_TITLE);

    sStorage->curBoxTitleSprites[0] = sStorage->nextBoxTitleSprites[0];
    sStorage->curBoxTitleSprites[1] = sStorage->nextBoxTitleSprites[1];
}

static void SpriteCB_IncomingBoxTitle(struct Sprite *sprite)
{
    if (sprite->sIncomingDelay != 0)
        sprite->sIncomingDelay--;
    else {
        sprite->invisible = FALSE;
        sprite->callback = SpriteCallbackDummy;
    }
}

static void SpriteCB_OutgoingBoxTitle(struct Sprite *sprite)
{
    if (sprite->sOutgoingDelay != 0)
    {
        sprite->sOutgoingDelay--;
    }
    else
    {
        DestroySprite(sprite);
    }
}

#undef sSpeed
#undef sIncomingX
#undef sIncomingDelay
#undef sOutgoingDelay
#undef sOutgoingX

static s16 GetBoxTitleBaseX(const u8 *string)
{
    return DISPLAY_WIDTH - 64 - GetStringWidth(FONT_NORMAL, string, 0)/2;
}

//------------------------------------------------------------------------------
//  SECTION: Cursor movement
//
//  The functions below generally handle the cursor's movement, including
//  moving around the box and picking up/putting down Pokémon.
//------------------------------------------------------------------------------

static void InitCursor(void)
{
    if (sStorage->boxOption != OPTION_DEPOSIT)
        sCursorArea = CURSOR_AREA_IN_BOX;
    else
        sCursorArea = CURSOR_AREA_IN_PARTY;

    sCursorPosition = 0;
    sIsMonBeingMoved = FALSE;
    sMovingMonOrigBoxId = 0;
    sMovingMonOrigBoxPos = 0;
    ClearSavedCursorPos();
    CreateCursorSprites();
    sStorage->cursorPrevHorizPos = 1;
    TryRefreshDisplayMon();
}

static void InitCursorOnReopen(void)
{
    CreateCursorSprites();
    ReshowDisplayMon();
    sStorage->cursorPrevHorizPos = 1;
    if (sIsMonBeingMoved)
    {
        sStorage->movingMon = sSavedMovingMon;
        CreateMovingMonIcon();
    }
}

static void GetCursorCoordsByPos(u8 cursorArea, u8 cursorPosition, u16 *x, u16 *y)
{
    switch (cursorArea)
    {
    case CURSOR_AREA_IN_BOX:
        *x = (cursorPosition % IN_BOX_COLUMNS) * 24 + 100;
        *y = (cursorPosition/IN_BOX_COLUMNS) * 24 +  32;
        break;
    case CURSOR_AREA_IN_PARTY:
        if (cursorPosition == 0)
        {
            *x = 104;
            *y = 52;
        }
        else if (cursorPosition == PARTY_SIZE)
        {
            *x = 152;
            *y = 132;
        }
        else
        {
            *x = 152;
            *y = (cursorPosition - 1) * 24 + 4;
        }
        break;
    case CURSOR_AREA_BOX_TITLE:
        *x = 162;
        *y = 12;
        break;
    case CURSOR_AREA_BUTTONS:
        *y = sIsMonBeingMoved ? 8 : 14;
        *x = 164;
        break;
    case 4:
        *x = 160;
        *y = 96;
        break;
    }
}

static u16 GetSpeciesAtCursorPosition(void)
{
    switch (sCursorArea)
    {
    case CURSOR_AREA_IN_PARTY:
        return GetMonData(&gPlayerParty[sCursorPosition], MON_DATA_SPECIES);
    case CURSOR_AREA_IN_BOX:
        return GetCurrentBoxMonData(sCursorPosition, MON_DATA_SPECIES);
    default:
        return SPECIES_NONE;
    }
}

static bool8 UpdateCursorPos(void)
{
    if (sStorage->cursorMoveSteps == 0)
    {
        if (sStorage->boxOption != OPTION_MOVE_ITEMS)
            return FALSE;
        else
            return IsItemIconAnimActive();
    }
    else
    {
        sStorage->cursorMoveSteps = 0;
        // Time is up for cursor movement, make sure it's exactly at target
        sStorage->cursorSprite->x = sStorage->cursorTargetX;
        sStorage->cursorSprite->y = sStorage->cursorTargetY;
        DoCursorNewPosUpdate();
    }

    return TRUE;
}

static void InitNewCursorPos(u8 newCursorArea, u8 newCursorPosition)
{
    u16 x = 0, y = 0;

    GetCursorCoordsByPos(newCursorArea, newCursorPosition, &x, &y);
    sStorage->newCursorArea = newCursorArea;
    sStorage->newCursorPosition = newCursorPosition;
    sStorage->cursorTargetX = x;
    sStorage->cursorTargetY = y;
}

static void InitCursorMove(void)
{
    int yDistance, xDistance;

    if (sStorage->cursorVerticalWrap != 0 || sStorage->cursorHorizontalWrap != 0)
        sStorage->cursorMoveSteps = 12;
    else
        sStorage->cursorMoveSteps = 6;

    switch (sStorage->cursorVerticalWrap)
    {
        default: // No wrap
            yDistance = sStorage->cursorTargetY - sStorage->cursorSprite->y;
            break;
        case -1: // Wrap from top to bottom
            yDistance = sStorage->cursorTargetY - 192 - sStorage->cursorSprite->y;
            break;
        case 1: // Wrap from bottom to top
            yDistance = sStorage->cursorTargetY + 192 - sStorage->cursorSprite->y;
            break;
    }

    switch (sStorage->cursorHorizontalWrap)
    {
        default: // No Wrap
            xDistance = sStorage->cursorTargetX - sStorage->cursorSprite->x;
            break;
        case -1: // Wrap from left to right
            xDistance = sStorage->cursorTargetX - 192 - sStorage->cursorSprite->x;
            break;
        case 1: // Wrap from right to left
            xDistance = sStorage->cursorTargetX + 192 - sStorage->cursorSprite->x;
            break;
    }

    yDistance <<= 8;
    xDistance <<= 8;
    sStorage->cursorSpeedX = xDistance/sStorage->cursorMoveSteps;
    sStorage->cursorSpeedY = yDistance/sStorage->cursorMoveSteps;
    sStorage->cursorNewX = sStorage->cursorSprite->x << 8;
    sStorage->cursorNewY = sStorage->cursorSprite->y << 8;
}

static void SetCursorPosition(u8 newCursorArea, u8 newCursorPosition)
{
    InitNewCursorPos(newCursorArea, newCursorPosition);
    InitCursorMove();
    if (sStorage->boxOption != OPTION_MOVE_ITEMS)
    {
        if (!sIsMonBeingMoved)
            StartSpriteAnim(sStorage->cursorSprite, CURSOR_ANIM_STILL);
    }
    else
    {
        if (!IsMovingItem())
            StartSpriteAnim(sStorage->cursorSprite, CURSOR_ANIM_STILL);
    }

    if (sStorage->boxOption == OPTION_MOVE_ITEMS)
    {
        if (sCursorArea == CURSOR_AREA_IN_BOX)
            TryHideItemIconAtPos(CURSOR_AREA_IN_BOX, sCursorPosition, IsMovingItem());
        else if (sCursorArea == CURSOR_AREA_IN_PARTY)
            TryHideItemIconAtPos(CURSOR_AREA_IN_PARTY, sCursorPosition, IsMovingItem());

        if (newCursorArea == CURSOR_AREA_IN_BOX)
            TryLoadItemIconAtPos(newCursorArea, newCursorPosition);
        else if (newCursorArea == CURSOR_AREA_IN_PARTY)
            TryLoadItemIconAtPos(newCursorArea, newCursorPosition);
    }

    if (newCursorArea == CURSOR_AREA_IN_PARTY && sCursorArea != CURSOR_AREA_IN_PARTY)
    {
        sStorage->cursorPrevHorizPos = 1;
    }

    switch (newCursorArea)
    {
    case CURSOR_AREA_BOX_TITLE:
    case CURSOR_AREA_IN_PARTY:
        sStorage->cursorSprite->oam.priority = 1;
        sStorage->cursorSprite->vFlip = FALSE;
        sStorage->cursorSprite->hFlip = FALSE;
        break;
    case CURSOR_AREA_BUTTONS:
        sStorage->cursorSprite->oam.priority = 1;
        sStorage->cursorSprite->vFlip = TRUE;
        sStorage->cursorSprite->hFlip = TRUE;
        break;
    case CURSOR_AREA_IN_BOX:
        sStorage->cursorSprite->oam.priority = 2;
        sStorage->cursorSprite->vFlip = FALSE;
        sStorage->cursorSprite->hFlip = FALSE;
        if (sCursorArea == CURSOR_AREA_IN_BOX && sIsMonBeingMoved)
            SetMovingMonPriority(2);
        break;
    }
}

static void DoCursorNewPosUpdate(void)
{
    sCursorArea = sStorage->newCursorArea;
    sCursorPosition = sStorage->newCursorPosition;
    if (sStorage->boxOption != OPTION_MOVE_ITEMS)
    {
        if (!sIsMonBeingMoved)
            StartSpriteAnim(sStorage->cursorSprite, CURSOR_ANIM_BOUNCE);
    }
    else
    {
        if (!IsMovingItem())
            StartSpriteAnim(sStorage->cursorSprite, CURSOR_ANIM_BOUNCE);
    }

    TryRefreshDisplayMon();
    switch (sCursorArea)
    {
    case CURSOR_AREA_BUTTONS:
        SetMovingMonPriority(1);
        break;
    case CURSOR_AREA_IN_PARTY:
        SetMovingMonPriority(1);
        break;
    case CURSOR_AREA_IN_BOX:
        sStorage->cursorSprite->oam.priority = 1;
        SetMovingMonPriority(2);
        break;
    }
}

static void SetCursorInParty(void)
{
    u8 partyCount;

    if (!sIsMonBeingMoved)
    {
        partyCount = 0;
    }
    else
    {
        partyCount = CalculatePlayerPartyCount();
        if (partyCount >= PARTY_SIZE)
            partyCount = PARTY_SIZE - 1;
    }
    SetCursorPosition(CURSOR_AREA_IN_PARTY, partyCount);
}

static void SetCursorBoxPosition(u8 cursorBoxPosition)
{
    SetCursorPosition(CURSOR_AREA_IN_BOX, cursorBoxPosition);
}

EWRAM_DATA static u8 sSavedCursorPosition = 0;

static void ClearSavedCursorPos(void)
{
    sSavedCursorPosition = 0;
}

static void SaveCursorPos(void)
{
    sSavedCursorPosition = sCursorPosition;
}

static u8 GetSavedCursorPos(void)
{
    return sSavedCursorPosition;
}

static void InitMonPlaceChange(u8 type)
{
    static bool8 (*const placeChangeFuncs[])(void) =
    {
        [CHANGE_GRAB]  = MonPlaceChange_Grab,
        [CHANGE_PLACE] = MonPlaceChange_Place,
        [CHANGE_SHIFT] = MonPlaceChange_Shift,
    };

    sStorage->monPlaceChangeFunc = placeChangeFuncs[type];
    sStorage->monPlaceChangeState = 0;
}

static bool8 DoMonPlaceChange(void)
{
    return sStorage->monPlaceChangeFunc();
}

static bool8 MonPlaceChange_Grab(void)
{
    switch (sStorage->monPlaceChangeState)
    {
    case 0:
        if (sIsMonBeingMoved)
            return FALSE;
        StartSpriteAnim(sStorage->cursorSprite, CURSOR_ANIM_OPEN);
        sStorage->monPlaceChangeState++;
        break;
    case 1:
        if (!MonPlaceChange_CursorDown())
        {
            StartSpriteAnim(sStorage->cursorSprite, CURSOR_ANIM_FIST);
            MoveMon();
            sStorage->monPlaceChangeState++;
        }
        break;
    case 2:
        if (!MonPlaceChange_CursorUp())
            sStorage->monPlaceChangeState++;
        break;
    case 3:
        return FALSE;
    }

    return TRUE;
}

static bool8 MonPlaceChange_Place(void)
{
    switch (sStorage->monPlaceChangeState)
    {
    case 0:
        if (!MonPlaceChange_CursorDown())
        {
            StartSpriteAnim(sStorage->cursorSprite, CURSOR_ANIM_OPEN);
            PlaceMon();
            sStorage->monPlaceChangeState++;
        }
        break;
    case 1:
        if (!MonPlaceChange_CursorUp())
        {
            StartSpriteAnim(sStorage->cursorSprite, CURSOR_ANIM_BOUNCE);
            sStorage->monPlaceChangeState++;
        }
        break;
    case 2:
        return FALSE;
    }

    return TRUE;
}

static bool8 MonPlaceChange_Shift(void)
{
    switch (sStorage->monPlaceChangeState)
    {
    case 0:
        switch (sCursorArea)
        {
        case CURSOR_AREA_IN_PARTY:
            sStorage->shiftBoxId = TOTAL_BOXES_COUNT;
            break;
        case CURSOR_AREA_IN_BOX:
            sStorage->shiftBoxId = StorageGetCurrentBox();
            break;
        default:
            return FALSE;
        }
        StartSpriteAnim(sStorage->cursorSprite, CURSOR_ANIM_OPEN);
        SaveMonSpriteAtPos(sStorage->shiftBoxId, sCursorPosition);
        sStorage->monPlaceChangeState++;
        break;
    case 1:
        SetShiftedMonData(sStorage->shiftBoxId, sCursorPosition);
        sStorage->monPlaceChangeState++;
        break;
    case 2:
        if (!MoveShiftingMons())
        {
            StartSpriteAnim(sStorage->cursorSprite, CURSOR_ANIM_FIST);
            SetShiftedMonSprites(sStorage->shiftBoxId, sCursorPosition);
            sStorage->monPlaceChangeState++;
        }
        break;
    case 3:
        return FALSE;
    }

    return TRUE;
}

static bool8 MonPlaceChange_CursorDown(void)
{
    switch (sStorage->cursorSprite->y2)
    {
    default:
    case 0:
        sStorage->cursorSprite->y2 += MON_HAND_DELTA;
        break;
    case 8: // Cursor has reached bottom
        return FALSE;
    }

    return TRUE;
}

static bool8 MonPlaceChange_CursorUp(void)
{
    switch (sStorage->cursorSprite->y2)
    {
    case 0: // Cursor has reached top
        return FALSE;
    default:
        sStorage->cursorSprite->y2 -= MON_HAND_DELTA;;
        break;
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//  SECTION: Pokémon data
//
//  The functions below handle moving Pokémon data around while using the PC,
//  including changing the positions of Pokémon, releasing Pokémon, viewing the
//  summary screen, and updating the display of the currently selected Pokémon.
//------------------------------------------------------------------------------
// When a single pokemon is picked up
static void MoveMon(void)
{
    switch (sCursorArea)
    {
    case CURSOR_AREA_IN_PARTY:
        SetMovingMonData(TOTAL_BOXES_COUNT, sCursorPosition);
        SetMovingMonSprite(MODE_PARTY, sCursorPosition);
        // party pokemon will have their palette updated elsewhere when leaving the party menu
        break;
    case CURSOR_AREA_IN_BOX:
        u16 palette[16] = {0};
        SetMovingMonData(StorageGetCurrentBox(), sCursorPosition);
        SetMovingMonSprite(MODE_BOX, sCursorPosition);

        // Set moving sprite palette to currently displayed pokemon

        sStorage->movingMonSprite->oam.paletteNum = IndexOfSpritePaletteTag(PALTAG_DISPLAY_MON);
        palette[0] = 0x8000;
        SwapInPalNextVBlank(&palette[0], &sPaletteSwapBuffer[PLTT_ID(sCursorPosition)]);
        break;
    default:
        return;
    }

    sIsMonBeingMoved = TRUE;
}

static void PlaceMon(void)
{
    u8 boxId;

    switch (sCursorArea)
    {
    case CURSOR_AREA_IN_PARTY:
        SetPlacedMonData(TOTAL_BOXES_COUNT, sCursorPosition);
        SetPlacedMonSprite(TOTAL_BOXES_COUNT, sCursorPosition);
        break;
    case CURSOR_AREA_IN_BOX:
        boxId = StorageGetCurrentBox();
        SetPlacedMonData(boxId, sCursorPosition);
        SetPlacedMonSprite(boxId, sCursorPosition);
        break;
    default:
        return;
    }

    sIsMonBeingMoved = FALSE;
}

static void RefreshDisplayMon(void)
{
    TryRefreshDisplayMon();
}

static void SetMovingMonData(u8 boxId, u8 position)
{
    if (boxId == TOTAL_BOXES_COUNT)
    {
        sStorage->movingMon = gPlayerParty[sCursorPosition];
        if (&gPlayerParty[sCursorPosition] == GetFirstLiveMon())
            gFollowerSteps = 0;
    }
    else
    {
        BoxMonAtToMon(boxId, position, &sStorage->movingMon);
    }

    PurgeMonOrBoxMon(boxId, position);
    sMovingMonOrigBoxId = boxId;
    sMovingMonOrigBoxPos = position;
}

static void SetPlacedMonData(u8 boxId, u8 position)
{
    if (OW_PC_HEAL <= GEN_7)
        HealPokemon(&sStorage->movingMon);

    if (boxId == TOTAL_BOXES_COUNT)
    {
        gPlayerParty[position] = sStorage->movingMon;
        if (&gPlayerParty[position] == GetFirstLiveMon())
            gFollowerSteps = 0;
    }
    else
    {
        SetBoxMonAt(boxId, position, &sStorage->movingMon.box);
    }
}

static void PurgeMonOrBoxMon(u8 boxId, u8 position)
{
    if (boxId == TOTAL_BOXES_COUNT)
        ZeroMonData(&gPlayerParty[position]);
    else
        ZeroBoxMonAt(boxId, position);
}

static void SetShiftedMonData(u8 boxId, u8 position)
{
    if (boxId == TOTAL_BOXES_COUNT)
        sStorage->tempMon = gPlayerParty[position];
    else
        BoxMonAtToMon(boxId, position, &sStorage->tempMon);

    SetPlacedMonData(boxId, position);
    sStorage->movingMon = sStorage->tempMon;
}

static void SetShiftedMonSprites(u8 boxId, u8 position)
{
    u8 displayIndex = IndexOfSpritePaletteTag(PALTAG_DISPLAY_MON);
    if (boxId == TOTAL_BOXES_COUNT)
    { // party
        u32 paletteNum = FindFreePartyPaletteSlot();
        // Copy display palette into party palette slot
        CpuFastCopy(&gPlttBufferUnfaded[OBJ_PLTT_ID(displayIndex)], &gPlttBufferUnfaded[OBJ_PLTT_ID(paletteNum)], PLTT_SIZE_4BPP);
        CpuFastCopy(&gPlttBufferFaded[OBJ_PLTT_ID(displayIndex)], &gPlttBufferFaded[OBJ_PLTT_ID(paletteNum)], PLTT_SIZE_4BPP);
        sStorage->partySprites[position]->oam.paletteNum = paletteNum;
    }
    else
    {
        u8 i = position / 6;
        u8 j = position % 6;
        // Copy display palette into swap buffer (at next vblank)
        // This is necessary because copying it while the screen is being drawn will cause flickering
        SwapInPalNextVBlank(&gPlttBufferFaded[OBJ_PLTT_ID(displayIndex)], &sPaletteSwapBuffer[PLTT_ID(position)]);
        sStorage->boxMonsSprites[position]->oam.paletteNum = (i & 1 ? 6 : 0) + j + 1;
    }

    SetDisplayMonData(&sStorage->movingMon, MODE_PARTY);
    // Set moving sprite palette to currently displayed pokemon's palette
    sStorage->displayMonSprite->invisible = TRUE;
    LoadCompressedPaletteFast(sStorage->displayMonPalette, sStorage->displayMonPalOffset, PLTT_SIZE_4BPP);
    UniquePalette(sStorage->displayMonPalOffset, GetBoxMonData(GetBoxedMonPtr(boxId, position), MON_DATA_PERSONALITY));
    CpuFastCopy(&gPlttBufferFaded[sStorage->displayMonPalOffset], &gPlttBufferUnfaded[sStorage->displayMonPalOffset], PLTT_SIZE_4BPP);
    sStorage->movingMonSprite->oam.paletteNum = displayIndex;
    sMovingMonOrigBoxId = boxId;
    sMovingMonOrigBoxPos = position;
}

static bool8 TryStorePartyMonInBox(u8 boxId)
{
    s16 boxPosition = GetFirstFreeBoxSpot(boxId);
    if (boxPosition == -1)
        return FALSE;

    if (sIsMonBeingMoved)
    {
        SetPlacedMonData(boxId, boxPosition);
        DestroyMovingMonIcon();
        sIsMonBeingMoved = FALSE;
    }
    else
    {
        SetMovingMonData(TOTAL_BOXES_COUNT, sCursorPosition);
        SetPlacedMonData(boxId, boxPosition);
        DestroyPartyMonIcon(sCursorPosition);
    }

    if (boxId == StorageGetCurrentBox())
        CreateBoxMonIconAtPos(boxPosition);

    StartSpriteAnim(sStorage->cursorSprite, CURSOR_ANIM_STILL);
    return TRUE;
}

static void ResetSelectionAfterDeposit(void)
{
    StartSpriteAnim(sStorage->cursorSprite, CURSOR_ANIM_BOUNCE);
    TryRefreshDisplayMon();
}

static void InitReleaseMon(void)
{
    u8 mode;

    if (sIsMonBeingMoved)
        mode = MODE_MOVE;
    else if (sCursorArea == CURSOR_AREA_IN_PARTY)
        mode = MODE_PARTY;
    else
        mode = MODE_BOX;

    SetReleaseMon(mode, sCursorPosition);
    StringCopy(sStorage->releaseMonName, sStorage->displayMonName);
}

static bool8 TryHideReleaseMon(void)
{
    if (!TryHideReleaseMonSprite())
    {
        StartSpriteAnim(sStorage->cursorSprite, CURSOR_ANIM_BOUNCE);
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

static void ReleaseMon(void)
{
    u8 boxId;
    u16 item = ITEM_NONE;

    DestroyReleaseMonIcon();
    if (sIsMonBeingMoved)
    {
        sIsMonBeingMoved = FALSE;
    }
    else
    {
        if (sCursorArea == CURSOR_AREA_IN_PARTY)
        {
            boxId = TOTAL_BOXES_COUNT;
            if (OW_PC_RELEASE_ITEM >= GEN_8)
                item = GetMonData(&gPlayerParty[sCursorPosition], MON_DATA_HELD_ITEM);
        }
        else
        {
            boxId = StorageGetCurrentBox();
            if (OW_PC_RELEASE_ITEM >= GEN_8)
                item = GetBoxMonDataAt(boxId, sCursorPosition, MON_DATA_HELD_ITEM);
        }

        if (sCursorArea != CURSOR_AREA_IN_PARTY)
          DisableBoxMonDynamicPalette(sCursorPosition, 1);
        PurgeMonOrBoxMon(boxId, sCursorPosition);
        if (item != ITEM_NONE)
            AddBagItem(item, 1);
    }
    TryRefreshDisplayMon();
}

static void TrySetCursorFistAnim(void)
{
    if (sIsMonBeingMoved)
        StartSpriteAnim(sStorage->cursorSprite, CURSOR_ANIM_FIST);
}

// If the player is on the listed map (or any map, if none is specified),
// they may not release their last Pokémon that knows the specified move.
// This is to stop the player from softlocking themselves by not having
// a Pokémon that knows a required field move.
struct
{
    s8 mapGroup;
    s8 mapNum;
    u16 move;
} static const sRestrictedReleaseMoves[] =
{
    {MAP_GROUPS_COUNT, 0, MOVE_SURF},
    {MAP_GROUPS_COUNT, 0, MOVE_DIVE},
    {MAP_GROUP(EVER_GRANDE_CITY_POKEMON_LEAGUE_1F), MAP_NUM(EVER_GRANDE_CITY_POKEMON_LEAGUE_1F), MOVE_STRENGTH},
    {MAP_GROUP(EVER_GRANDE_CITY_POKEMON_LEAGUE_1F), MAP_NUM(EVER_GRANDE_CITY_POKEMON_LEAGUE_1F), MOVE_ROCK_SMASH},
    {MAP_GROUP(EVER_GRANDE_CITY_POKEMON_LEAGUE_2F), MAP_NUM(EVER_GRANDE_CITY_POKEMON_LEAGUE_2F), MOVE_STRENGTH},
    {MAP_GROUP(EVER_GRANDE_CITY_POKEMON_LEAGUE_2F), MAP_NUM(EVER_GRANDE_CITY_POKEMON_LEAGUE_2F), MOVE_ROCK_SMASH},
};

static void GetRestrictedReleaseMoves(u16 *moves)
{
    s32 i;

    for (i = 0; i < ARRAY_COUNT(sRestrictedReleaseMoves); i++)
    {
        if (sRestrictedReleaseMoves[i].mapGroup == MAP_GROUPS_COUNT
        || (sRestrictedReleaseMoves[i].mapGroup == gSaveBlock1Ptr->location.mapGroup
         && sRestrictedReleaseMoves[i].mapNum == gSaveBlock1Ptr->location.mapNum))
        {
            *moves = sRestrictedReleaseMoves[i].move;
            moves++;
        }
    }
    *moves = MOVES_COUNT;
}

static void InitCanReleaseMonVars(void)
{
    if (!AtLeastThreeUsableMons())
    {
        // The player only has 1 or 2 usable
        // Pokémon, this one can't be released
        sStorage->releaseStatusResolved = TRUE;
        sStorage->canReleaseMon = FALSE;
        return;
    }

    if (sIsMonBeingMoved)
    {
        sStorage->tempMon = sStorage->movingMon;
        sStorage->releaseBoxId = -1;
        sStorage->releaseBoxPos = -1;
    }
    else
    {
        if (sCursorArea == CURSOR_AREA_IN_PARTY)
        {
            sStorage->tempMon = gPlayerParty[sCursorPosition];
            sStorage->releaseBoxId = TOTAL_BOXES_COUNT;
        }
        else
        {
            BoxMonAtToMon(StorageGetCurrentBox(), sCursorPosition, &sStorage->tempMon);
            sStorage->releaseBoxId = StorageGetCurrentBox();
        }
        sStorage->releaseBoxPos = sCursorPosition;
    }

    GetRestrictedReleaseMoves(sStorage->restrictedMoveList);
    sStorage->restrictedReleaseMonMoves = GetMonData(&sStorage->tempMon, MON_DATA_KNOWN_MOVES, (u8 *)sStorage->restrictedMoveList);
    if (sStorage->restrictedReleaseMonMoves != 0)
    {
        // Pokémon knows at least one restricted release move
        // Need to check if another Pokémon has this move first
        sStorage->releaseStatusResolved = FALSE;
    }
    else
    {
        // Pokémon knows no restricted moves, can be released
        sStorage->releaseStatusResolved = TRUE;
        sStorage->canReleaseMon = TRUE;
    }

    sStorage->releaseCheckState = 0;
}

static bool32 AtLeastThreeUsableMons(void)
{
    s32 i, j;
    s32 count = (sIsMonBeingMoved != FALSE);

    // Check party for usable Pokémon
    for (j = 0; j < PARTY_SIZE; j++)
    {
        if (GetMonData(&gPlayerParty[j], MON_DATA_SANITY_HAS_SPECIES))
            count++;
    }

    if (count >= 3)
        return TRUE;

    // Check PC for usable Pokémon
    for (i = 0; i < TOTAL_BOXES_COUNT; i++)
    {
        for (j = 0; j < IN_BOX_COUNT; j++)
        {
            if (CheckBoxMonSanityAt(i, j))
            {
                if (++count >= 3)
                    return TRUE;
            }
        }
    }

    return FALSE;
}

static s8 RunCanReleaseMon(void)
{
    u16 i;
    u16 knownMoves;

    if (sStorage->releaseStatusResolved)
        return sStorage->canReleaseMon;

    switch (sStorage->releaseCheckState)
    {
    case 0:
        // Check party for other Pokémon that know any restricted
        // moves the release Pokémon knows
        for (i = 0; i < PARTY_SIZE; i++)
        {
            // Make sure party Pokémon isn't the one we're releasing first
            if (sStorage->releaseBoxId != TOTAL_BOXES_COUNT || sStorage->releaseBoxPos != i)
            {
                knownMoves = GetMonData(&gPlayerParty[i], MON_DATA_KNOWN_MOVES, (u8 *)sStorage->restrictedMoveList);
                sStorage->restrictedReleaseMonMoves &= ~(knownMoves);
            }
        }
        if (sStorage->restrictedReleaseMonMoves == 0)
        {
            // No restricted moves on release Pokémon that
            // aren't resolved by the party, it can be released.
            sStorage->releaseStatusResolved = TRUE;
            sStorage->canReleaseMon = TRUE;
        }
        else
        {
            // Release Pokémon has restricted moves not resolved by the party.
            // Continue and check the PC next
            sStorage->releaseCheckBoxId = 0;
            sStorage->releaseCheckBoxPos = 0;
            sStorage->releaseCheckState++;
        }
        break;
    case 1:
        // Check PC for other Pokémon that know any restricted
        // moves the release Pokémon knows
        for (i = 0; i < IN_BOX_COUNT; i++)
        {
            knownMoves = GetAndCopyBoxMonDataAt(sStorage->releaseCheckBoxId, sStorage->releaseCheckBoxPos, MON_DATA_KNOWN_MOVES, (u8 *)sStorage->restrictedMoveList);
            if (knownMoves != 0 && !(sStorage->releaseBoxId == sStorage->releaseCheckBoxId
                                  && sStorage->releaseBoxPos == sStorage->releaseCheckBoxPos))
            {
                // Found PC Pokémon with restricted move, clear move from list
                sStorage->restrictedReleaseMonMoves &= ~(knownMoves);
                if (sStorage->restrictedReleaseMonMoves == 0)
                {
                    // No restricted moves on release Pokémon that
                    // aren't resolved, it can be released.
                    sStorage->releaseStatusResolved = TRUE;
                    sStorage->canReleaseMon = TRUE;
                    break;
                }
            }
            if (++sStorage->releaseCheckBoxPos >= IN_BOX_COUNT)
            {
                sStorage->releaseCheckBoxPos = 0;
                if (++sStorage->releaseCheckBoxId >= TOTAL_BOXES_COUNT)
                {
                    // Checked every Pokémon in the PC, release Pokémon is
                    // the sole owner of at least one restricted move.
                    // It cannot be released.
                    sStorage->releaseStatusResolved = TRUE;
                    sStorage->canReleaseMon = FALSE;
                }
            }
        }
        break;
    }

    return -1;
}

static void SaveMovingMon(void)
{
    if (sIsMonBeingMoved)
        sSavedMovingMon = sStorage->movingMon;
}

static void LoadSavedMovingMon(void)
{
    if (sIsMonBeingMoved)
    {
        // If it came from the party load a struct Pokemon,
        // otherwise load a BoxPokemon
        if (sMovingMonOrigBoxId == TOTAL_BOXES_COUNT)
            sStorage->movingMon = sSavedMovingMon;
        else
            sStorage->movingMon.box = sSavedMovingMon.box;
    }
}

static void InitSummaryScreenData(void)
{
    if (sIsMonBeingMoved)
    {
        SaveMovingMon();
        sStorage->summaryMon.mon = &sSavedMovingMon;
        sStorage->summaryStartPos = 0;
        sStorage->summaryMaxPos = 0;
        sStorage->summaryScreenMode = SUMMARY_MODE_NORMAL;
    }
    else if (sCursorArea == CURSOR_AREA_IN_PARTY)
    {
        sStorage->summaryMon.mon = gPlayerParty;
        sStorage->summaryStartPos = sCursorPosition;
        sStorage->summaryMaxPos = CountPartyMons() - 1;
        sStorage->summaryScreenMode = SUMMARY_MODE_NORMAL;
    }
    else
    {
        sStorage->summaryMon.box = GetBoxedMonPtr(StorageGetCurrentBox(), 0);
        sStorage->summaryStartPos = sCursorPosition;
        sStorage->summaryMaxPos = IN_BOX_COUNT - 1;
        sStorage->summaryScreenMode = SUMMARY_MODE_BOX;
    }
}

static void SetSelectionAfterSummaryScreen(void)
{
    if (sIsMonBeingMoved)
        LoadSavedMovingMon();
    else
        sCursorPosition = gLastViewedMonIndex;
}

s16 CompactPartySlots(void)
{
    s16 retVal = -1;
    u16 i, last;

    for (i = 0, last = 0; i < PARTY_SIZE; i++)
    {
        u16 species = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES);
        if (species != SPECIES_NONE)
        {
            if (i != last)
                gPlayerParty[last] = gPlayerParty[i];
            last++;
        }
        else if (retVal == -1)
        {
            retVal = i;
        }
    }
    for (; last < PARTY_SIZE; last++)
        ZeroMonData(&gPlayerParty[last]);

    return retVal;
}

static bool8 IsRemovingLastPartyMon(void)
{
    if (sCursorArea == CURSOR_AREA_IN_PARTY && !sIsMonBeingMoved && CountPartyAliveNonEggMonsExcept(sCursorPosition) == 0)
        return TRUE;
    else
        return FALSE;
}

static bool8 CanPlaceMon(void)
{
    if (sIsMonBeingMoved)
    {
        if (sCursorArea == CURSOR_AREA_IN_PARTY && GetMonData(&gPlayerParty[sCursorPosition], MON_DATA_SPECIES) == SPECIES_NONE)
            return TRUE;
        else if (sCursorArea == CURSOR_AREA_IN_BOX && GetBoxMonDataAt(StorageGetCurrentBox(), sCursorPosition, MON_DATA_SPECIES_OR_EGG) == SPECIES_NONE)
            return TRUE;
        else
            return FALSE;
    }
    return FALSE;
}

static bool8 CanShiftMon(void)
{
    if (sIsMonBeingMoved)
    {
        if (sCursorArea == CURSOR_AREA_IN_PARTY && CountPartyAliveNonEggMonsExcept(sCursorPosition) == 0)
        {
            if (sStorage->displayMonIsEgg || GetMonData(&sStorage->movingMon, MON_DATA_HP) == 0)
                return FALSE;
        }
        return TRUE;
    }
    return FALSE;
}

static bool8 IsMonBeingMoved(void)
{
    return sIsMonBeingMoved;
}

static bool8 IsCursorOnBoxTitle(void)
{
    return (sCursorArea == CURSOR_AREA_BOX_TITLE);
}

static bool8 IsCursorInBox(void)
{
    return (sCursorArea == CURSOR_AREA_IN_BOX);
}

static void TryRefreshDisplayMon(void)
{
    // If a Pokémon is currently being moved, don't start
    // mosaic or update display. Keep displaying the
    // currently held Pokémon.
    sStorage->setMosaic = (sIsMonBeingMoved == FALSE);
    if (!sIsMonBeingMoved)
    {
        // Update display Pokémon
        switch (sCursorArea)
        {
        case CURSOR_AREA_IN_PARTY:
            if (sCursorPosition < PARTY_SIZE)
            {
                SetDisplayMonData(&gPlayerParty[sCursorPosition], MODE_PARTY);
                break;
            }
            // fallthrough
        case CURSOR_AREA_BUTTONS:
        case CURSOR_AREA_BOX_TITLE:
            SetDisplayMonData(NULL, MODE_MOVE);
            break;
        case CURSOR_AREA_IN_BOX:
            SetDisplayMonData(GetBoxedMonPtr(StorageGetCurrentBox(), sCursorPosition), MODE_BOX);
            break;
        }
    }
}

static void ReshowDisplayMon(void)
{
    if (sIsMonBeingMoved)
        SetDisplayMonData(&sSavedMovingMon, MODE_PARTY);
    else
        TryRefreshDisplayMon();
}

static void SetDisplayMonData(void *pokemon, u8 mode)
{
    u8 *txtPtr;
    u16 gender;

    sStorage->displayMonItemId = ITEM_NONE;
    gender = MON_MALE;

    if (mode == MODE_PARTY)
    {
        struct Pokemon *mon = (struct Pokemon *)pokemon;

        CopyMon(&sCurrentBoxPokemon, &mon->box, sizeof(sCurrentBoxPokemon));
        sStorage->displayMonSpecies = GetMonData(mon, MON_DATA_SPECIES_OR_EGG);
        if (sStorage->displayMonSpecies != SPECIES_NONE)
        {
            sStorage->displayMonIsEgg = GetMonData(mon, MON_DATA_IS_EGG);
            GetMonData(mon, MON_DATA_NICKNAME, sStorage->displayMonName);
            StringGet_Nickname(sStorage->displayMonName);
            sStorage->displayMonLevel = GetMonData(mon, MON_DATA_LEVEL);
            sStorage->displayMonPersonality = GetMonData(mon, MON_DATA_PERSONALITY);
            sStorage->displayMonIsShiny = GetMonData(mon, MON_DATA_IS_SHINY);
            sStorage->displayMonPalette = GetMonFrontSpritePal(mon);
            gender = GetMonGender(mon);
            sStorage->displayMonItemId = GetMonData(mon, MON_DATA_HELD_ITEM);
            sStorage->eggPalette[0] = gSpeciesInfo[GetMonData(mon, MON_DATA_SPECIES)].types[0];
            sStorage->eggPalette[1] = gSpeciesInfo[GetMonData(mon, MON_DATA_SPECIES)].types[1];
        }
    }
    else if (mode == MODE_BOX)
    {
        struct BoxPokemon *boxMon = (struct BoxPokemon *)pokemon;

        CopyMon(&sCurrentBoxPokemon, &boxMon, sizeof(sCurrentBoxPokemon));
        sStorage->displayMonSpecies = GetBoxMonData(pokemon, MON_DATA_SPECIES_OR_EGG);
        if (sStorage->displayMonSpecies != SPECIES_NONE)
        {
            bool8 isShiny = GetBoxMonData(boxMon, MON_DATA_IS_SHINY);
            sStorage->displayMonIsEgg = GetBoxMonData(boxMon, MON_DATA_IS_EGG);
            GetBoxMonData(boxMon, MON_DATA_NICKNAME, sStorage->displayMonName);
            StringGet_Nickname(sStorage->displayMonName);
            sStorage->displayMonLevel = GetLevelFromBoxMonExp(boxMon);
            sStorage->displayMonPersonality = GetBoxMonData(boxMon, MON_DATA_PERSONALITY);
            sStorage->displayMonIsShiny = GetBoxMonData(boxMon, MON_DATA_IS_SHINY);
            sStorage->displayMonPalette = GetMonSpritePalFromSpeciesAndPersonality(sStorage->displayMonSpecies, isShiny, sStorage->displayMonPersonality);
            gender = GetGenderFromSpeciesAndPersonality(sStorage->displayMonSpecies, sStorage->displayMonPersonality);
            sStorage->displayMonItemId = GetBoxMonData(boxMon, MON_DATA_HELD_ITEM);
            sStorage->eggPalette[0] = gSpeciesInfo[GetBoxMonData(pokemon, MON_DATA_SPECIES)].types[0];
            sStorage->eggPalette[1] = gSpeciesInfo[GetBoxMonData(pokemon, MON_DATA_SPECIES)].types[1];
        }
    }
    else
    {
        sStorage->displayMonSpecies = SPECIES_NONE;
        sStorage->displayMonItemId = ITEM_NONE;
    }

    if (sStorage->displayMonSpecies == SPECIES_NONE)
    {
        StringFill(sStorage->displayMonName, CHAR_SPACE, 5);
        StringFill(sStorage->displayMonNameText, CHAR_SPACE, 8);
        StringFill(sStorage->displayMonSpeciesName, CHAR_SPACE, 8);
        StringFill(sStorage->displayMonGenderLvlText, CHAR_SPACE, 8);
        StringFill(sStorage->displayMonItemName, CHAR_SPACE, 8);
    }
    else if (sStorage->displayMonIsEgg)
    {
        StringCopyPadded(sStorage->displayMonNameText, gText_EggNickname, CHAR_SPACE, 8);
        StringFill(sStorage->displayMonSpeciesName, CHAR_SPACE, 8);
        StringFill(sStorage->displayMonGenderLvlText, CHAR_SPACE, 8);
        StringFill(sStorage->displayMonItemName, CHAR_SPACE, 8);
    }
    else
    {
        if (sStorage->displayMonSpecies == SPECIES_NIDORAN_F || sStorage->displayMonSpecies == SPECIES_NIDORAN_M)
            gender = MON_GENDERLESS;

        StringCopyPadded(sStorage->displayMonNameText, sStorage->displayMonName, CHAR_SPACE, 5);

        txtPtr = sStorage->displayMonSpeciesName;
        *(txtPtr)++ = CHAR_SLASH;
        StringCopyPadded(txtPtr, GetSpeciesName(sStorage->displayMonSpecies), CHAR_SPACE, 5);

        txtPtr = sStorage->displayMonGenderLvlText;
        *(txtPtr)++ = EXT_CTRL_CODE_BEGIN;
        *(txtPtr)++ = EXT_CTRL_CODE_COLOR_HIGHLIGHT_SHADOW;
        switch (gender)
        {
        case MON_MALE:
            *(txtPtr)++ = TEXT_COLOR_RED;
            *(txtPtr)++ = TEXT_COLOR_WHITE;
            *(txtPtr)++ = TEXT_COLOR_LIGHT_RED;
            *(txtPtr)++ = CHAR_MALE;
            break;
        case MON_FEMALE:
            *(txtPtr)++ = TEXT_COLOR_GREEN;
            *(txtPtr)++ = TEXT_COLOR_WHITE;
            *(txtPtr)++ = TEXT_COLOR_LIGHT_GREEN;
            *(txtPtr)++ = CHAR_FEMALE;
            break;
        default:
            *(txtPtr)++ = TEXT_COLOR_DARK_GRAY;
            *(txtPtr)++ = TEXT_COLOR_WHITE;
            *(txtPtr)++ = TEXT_COLOR_LIGHT_GRAY;
            *(txtPtr)++ = CHAR_SPACER; // Genderless
            break;
        }

        *(txtPtr++) = EXT_CTRL_CODE_BEGIN;
        *(txtPtr++) = EXT_CTRL_CODE_COLOR_HIGHLIGHT_SHADOW;
        *(txtPtr++) = TEXT_COLOR_DARK_GRAY;
        *(txtPtr++) = TEXT_COLOR_WHITE;
        *(txtPtr++) = TEXT_COLOR_LIGHT_GRAY;
        *(txtPtr++) = CHAR_SPACE;
        *(txtPtr++) = CHAR_EXTRA_SYMBOL;
        *(txtPtr++) = CHAR_LV_2;

        txtPtr = ConvertIntToDecimalStringN(txtPtr, sStorage->displayMonLevel, STR_CONV_MODE_LEFT_ALIGN, 3);
        txtPtr[0] = CHAR_SPACE;
        txtPtr[1] = EOS;

        if (sStorage->displayMonItemId != ITEM_NONE)
            StringCopyPadded(sStorage->displayMonItemName, ItemId_GetName(sStorage->displayMonItemId), CHAR_SPACE, 8);
        else
            StringFill(sStorage->displayMonItemName, CHAR_SPACE, 8);
    }
}

//------------------------------------------------------------------------------
//  SECTION: Input handlers
//
//  The functions below process context-dependent input
//------------------------------------------------------------------------------
static u8 HandleInput_InBox(void)
{
    u8 retVal;
    s8 cursorArea;
    s8 cursorPosition;

    do
    {
        cursorArea = sCursorArea;
        cursorPosition = sCursorPosition;
        sStorage->cursorVerticalWrap = 0;
        sStorage->cursorHorizontalWrap = 0;

        if (JOY_REPEAT(DPAD_UP))
        {
            retVal = INPUT_MOVE_CURSOR;
            if (sCursorPosition >= IN_BOX_COLUMNS)
            {
                cursorPosition -= IN_BOX_COLUMNS;
            }
            else
            {
                cursorArea = CURSOR_AREA_BOX_TITLE;
                cursorPosition = 0;
            }
            break;
        }
        else if (JOY_REPEAT(DPAD_DOWN))
        {
            retVal = INPUT_MOVE_CURSOR;
            cursorPosition += IN_BOX_COLUMNS;
            if (cursorPosition >= IN_BOX_COUNT)
            {
                cursorArea = CURSOR_AREA_BUTTONS;
                cursorPosition -= IN_BOX_COUNT;
                cursorPosition /= 3;
                sStorage->cursorVerticalWrap = 1;
            }
            break;
        }
        else if (JOY_REPEAT(DPAD_LEFT))
        {
            retVal = INPUT_MOVE_CURSOR;
            if (sCursorPosition % IN_BOX_COLUMNS != 0)
            {
                cursorPosition--;
            }
            else
            {
                sStorage->cursorHorizontalWrap = -1;
                cursorPosition += (IN_BOX_COLUMNS - 1);
            }
            break;
        }
        else if (JOY_REPEAT(DPAD_RIGHT))
        {
            retVal = INPUT_MOVE_CURSOR;
            if ((sCursorPosition + 1) % IN_BOX_COLUMNS != 0)
            {
                cursorPosition++;
            }
            else
            {
                sStorage->cursorHorizontalWrap = 1;
                cursorPosition -= (IN_BOX_COLUMNS - 1);
            }
            break;
        }
        else if (JOY_NEW(START_BUTTON))
        {
            retVal = INPUT_MOVE_CURSOR;
            cursorArea = CURSOR_AREA_BOX_TITLE;
            cursorPosition = 0;
            break;
        }
        else if (JOY_HELD(L_BUTTON))
            return INPUT_SCROLL_LEFT;
        else if (JOY_HELD(R_BUTTON))
            return INPUT_SCROLL_RIGHT;

        if ((JOY_NEW(A_BUTTON)) && SetSelectionMenuTexts())
        {
            return INPUT_IN_MENU;

            if (sStorage->boxOption != OPTION_MOVE_MONS || sIsMonBeingMoved == TRUE)
            {
                switch (GetMenuItemTextId(0))
                {
                case MENU_STORE:
                    return INPUT_DEPOSIT;
                case MENU_WITHDRAW:
                    return INPUT_WITHDRAW;
                case MENU_MOVE:
                    return INPUT_MOVE_MON;
                case MENU_SHIFT:
                    return INPUT_SHIFT_MON;
                case MENU_PLACE:
                    return INPUT_PLACE_MON;
                case MENU_TAKE:
                    return INPUT_TAKE_ITEM;
                case MENU_GIVE:
                    return INPUT_GIVE_ITEM;
                case MENU_SWITCH:
                    return INPUT_SWITCH_ITEMS;
                }
            }
        }

        if (JOY_NEW(B_BUTTON))
            return INPUT_PRESSED_B;

        retVal = INPUT_NONE;

    } while (0);

    if (retVal)
        SetCursorPosition(cursorArea, cursorPosition);

    return retVal;
}

static u8 HandleInput_InParty(void)
{
    u8 retVal;
    bool8 gotoBox;
    s8 cursorArea;
    s8 cursorPosition;

    do
    {
        cursorArea = sCursorArea;
        cursorPosition = sCursorPosition;
        sStorage->cursorHorizontalWrap = 0;
        sStorage->cursorVerticalWrap = 0;
        gotoBox = FALSE;
        retVal = INPUT_NONE;

        if (JOY_REPEAT(DPAD_UP))
        {
            if (--cursorPosition < 0)
                cursorPosition = PARTY_SIZE;
            if (cursorPosition != sCursorPosition)
                retVal = INPUT_MOVE_CURSOR;
            break;
        }
        else if (JOY_REPEAT(DPAD_DOWN))
        {
            if (++cursorPosition > PARTY_SIZE)
                cursorPosition = 0;
            if (cursorPosition != sCursorPosition)
                retVal = INPUT_MOVE_CURSOR;
            break;
        }
        else if (JOY_REPEAT(DPAD_LEFT) && sCursorPosition != 0)
        {
            retVal = INPUT_MOVE_CURSOR;
            sStorage->cursorPrevHorizPos = sCursorPosition;
            cursorPosition = 0;
            break;
        }
        else if (JOY_REPEAT(DPAD_RIGHT))
        {
            if (sCursorPosition == 0)
            {
                retVal = INPUT_MOVE_CURSOR;
                cursorPosition = sStorage->cursorPrevHorizPos;
            }
            else
            {
                retVal = INPUT_HIDE_PARTY;
                cursorArea = CURSOR_AREA_IN_BOX;
                cursorPosition = 0;
            }
            break;
        }

        if (JOY_NEW(A_BUTTON))
        {
            if (sCursorPosition == PARTY_SIZE)
            {
                gotoBox = TRUE;
            }
            else if (SetSelectionMenuTexts())
            {
                return INPUT_IN_MENU;

                switch (GetMenuItemTextId(0))
                {
                case MENU_STORE:
                    return INPUT_DEPOSIT;
                case MENU_WITHDRAW:
                    return INPUT_WITHDRAW;
                case MENU_MOVE:
                    return INPUT_MOVE_MON;
                case MENU_SHIFT:
                    return INPUT_SHIFT_MON;
                case MENU_PLACE:
                    return INPUT_PLACE_MON;
                case MENU_TAKE:
                    return INPUT_TAKE_ITEM;
                case MENU_GIVE:
                    return INPUT_GIVE_ITEM;
                case MENU_SWITCH:
                    return INPUT_SWITCH_ITEMS;
                }
            }
        }

        if (JOY_NEW(B_BUTTON))
        {
            if (sStorage->boxOption == OPTION_DEPOSIT)
                return INPUT_PRESSED_B;

            gotoBox = TRUE;
        }

        if (gotoBox)
        {
            retVal = INPUT_HIDE_PARTY;
            cursorArea = CURSOR_AREA_IN_BOX;
            cursorPosition = 0;
        }

    } while (0);

    if (retVal != INPUT_NONE)
    {
        if (retVal != INPUT_HIDE_PARTY)
            SetCursorPosition(cursorArea, cursorPosition);
    }

    return retVal;
}

static u8 HandleInput_OnBox(void)
{
    u8 retVal;
    s8 cursorArea;
    s8 cursorPosition;

    do
    {
        sStorage->cursorHorizontalWrap = 0;
        sStorage->cursorVerticalWrap = 0;

        if (JOY_REPEAT(DPAD_UP))
        {
            retVal = INPUT_MOVE_CURSOR;
            cursorArea = CURSOR_AREA_BUTTONS;
            cursorPosition = 0;
            break;
        }
        else if (JOY_REPEAT(DPAD_DOWN))
        {
            retVal = INPUT_MOVE_CURSOR;
            cursorArea = CURSOR_AREA_IN_BOX;
            cursorPosition = 2;
            break;
        }

        if (JOY_HELD(L_BUTTON))
            return INPUT_SCROLL_LEFT;
        if (JOY_HELD(R_BUTTON))
            return INPUT_SCROLL_RIGHT;

        if (JOY_NEW(A_BUTTON))
        {
            AddBoxOptionsMenu();
            return INPUT_BOX_OPTIONS;
        }

        if (JOY_NEW(B_BUTTON))
            return INPUT_PRESSED_B;

        retVal = INPUT_NONE;

    } while (0);

    if (retVal != INPUT_NONE)
    {
        SetCursorPosition(cursorArea, cursorPosition);
    }

    return retVal;
}

static u8 HandleInput_OnButtons(void)
{
    u8 retVal;
    s8 cursorArea;
    s8 cursorPosition;

    do
    {
        cursorArea = sCursorArea;
        cursorPosition = sCursorPosition;
        sStorage->cursorHorizontalWrap = 0;
        sStorage->cursorVerticalWrap = 0;

        if (JOY_REPEAT(DPAD_UP))
        {
            retVal = INPUT_MOVE_CURSOR;
            cursorArea = CURSOR_AREA_IN_BOX;
            sStorage->cursorVerticalWrap = -1;
            if (sCursorPosition == 0)
                cursorPosition = IN_BOX_COUNT - 1 - 5;
            else
                cursorPosition = IN_BOX_COUNT - 1;
            break;
        }

        if (JOY_REPEAT(DPAD_DOWN))
        {
            retVal = INPUT_MOVE_CURSOR;
            cursorArea = CURSOR_AREA_BOX_TITLE;
            cursorPosition = 0;
            break;
        }

        if (JOY_NEW(A_BUTTON))
            return INPUT_SHOW_PARTY;

        if (JOY_NEW(B_BUTTON))
            return INPUT_PRESSED_B;

        retVal = INPUT_NONE;
    } while (0);

    if (retVal != INPUT_NONE)
        SetCursorPosition(cursorArea, cursorPosition);

    return retVal;
}

static u8 HandleInput(void)
{
    struct
    {
        u8 (*func)(void);
        s8 area;
    } static const inputFuncs[] =
    {
        {HandleInput_InBox,     CURSOR_AREA_IN_BOX},
        {HandleInput_InParty,   CURSOR_AREA_IN_PARTY},
        {HandleInput_OnBox,     CURSOR_AREA_BOX_TITLE},
        {HandleInput_OnButtons, CURSOR_AREA_BUTTONS},
        {},
    };

    u16 i = 0;
    while (inputFuncs[i].func != NULL)
    {
        if (inputFuncs[i].area == sCursorArea)
            return inputFuncs[i].func();
        i++;
    }

    return INPUT_NONE;
}

static void AddBoxOptionsMenu(void)
{
    InitMenu();
    SetMenuText(MENU_JUMP);
    SetMenuText(MENU_NAME);
    SetMenuText(MENU_CANCEL);
}

static u8 SetSelectionMenuTexts(void)
{
    InitMenu();
    if (sStorage->boxOption != OPTION_MOVE_ITEMS)
        return SetMenuTexts_Mon();
    else
        return SetMenuTexts_Item();
}

static bool8 SetMenuTexts_Mon(void)
{
    u16 species = GetSpeciesAtCursorPosition();

    switch (sStorage->boxOption)
    {
    case OPTION_DEPOSIT:
        if (species != SPECIES_NONE)
            SetMenuText(MENU_STORE);
        else
            return FALSE;
        break;
    case OPTION_WITHDRAW:
        if (species != SPECIES_NONE)
            SetMenuText(MENU_WITHDRAW);
        else
            return FALSE;
        break;
    case OPTION_MOVE_MONS:
        if (sIsMonBeingMoved)
        {
            if (species != SPECIES_NONE)
                SetMenuText(MENU_SHIFT);
            else
                SetMenuText(MENU_PLACE);
        }
        else
        {
            if (species != SPECIES_NONE)
                SetMenuText(MENU_MOVE);
            else
                return FALSE;
        }
        break;
    case OPTION_MOVE_ITEMS:
    default:
        return FALSE;
    }

    SetMenuText(MENU_SUMMARY);
    if (sStorage->boxOption == OPTION_MOVE_MONS)
    {
        if (sCursorArea == CURSOR_AREA_IN_BOX)
            SetMenuText(MENU_WITHDRAW);
        else
            SetMenuText(MENU_STORE);
    }

    SetMenuText(MENU_RELEASE);
    SetMenuText(MENU_CANCEL);
    return TRUE;
}

static bool8 SetMenuTexts_Item(void)
{
    if (sStorage->displayMonSpecies == SPECIES_EGG)
        return FALSE;

    if (!IsMovingItem())
    {
        if (sStorage->displayMonItemId == ITEM_NONE)
        {
            if (sStorage->displayMonSpecies == SPECIES_NONE)
                return FALSE;

            SetMenuText(MENU_GIVE_2);
        }
        else
        {
            if (!ItemIsMail(sStorage->displayMonItemId))
            {
                SetMenuText(MENU_TAKE);
                SetMenuText(MENU_BAG);
            }
            SetMenuText(MENU_INFO);
        }
    }
    else
    {
        if (sStorage->displayMonItemId == ITEM_NONE)
        {
            if (sStorage->displayMonSpecies == SPECIES_NONE)
                return FALSE;

            SetMenuText(MENU_GIVE);
        }
        else
        {
            if (ItemIsMail(sStorage->displayMonItemId) == TRUE)
                return FALSE;

            SetMenuText(MENU_SWITCH);
        }
    }

    SetMenuText(MENU_CANCEL);
    return TRUE;
}

//------------------------------------------------------------------------------
//  SECTION: Cursor
//
//  The functions below handle a few of the generic cursor features.
//------------------------------------------------------------------------------
static void CreateCursorSprites(void)
{
    u16 x, y;
    u8 spriteId;
    struct SpriteSheet spriteSheets[] =
    {
        {sHandCursor_Gfx, 2048, GFXTAG_CURSOR},
        {}
    };

    static const struct OamData sOamData_Cursor =
    {
        .shape = SPRITE_SHAPE(32x32),
        .size = SPRITE_SIZE(32x32),
        .priority = 1,
    };

    static const union AnimCmd sAnim_Cursor_Bouncing[] =
    {
        ANIMCMD_FRAME(0, 30),
        ANIMCMD_FRAME(16, 30),
        ANIMCMD_JUMP(0)
    };
    static const union AnimCmd sAnim_Cursor_Still[] =
    {
        ANIMCMD_FRAME(0, 5),
        ANIMCMD_END
    };
    static const union AnimCmd sAnim_Cursor_Open[] =
    {
        ANIMCMD_FRAME(32, 5),
        ANIMCMD_END
    };
    static const union AnimCmd sAnim_Cursor_Fist[] =
    {
        ANIMCMD_FRAME(48, 5),
        ANIMCMD_END
    };

    static const union AnimCmd *const sAnims_Cursor[] =
    {
        [CURSOR_ANIM_BOUNCE] = sAnim_Cursor_Bouncing,
        [CURSOR_ANIM_STILL]  = sAnim_Cursor_Still,
        [CURSOR_ANIM_OPEN]   = sAnim_Cursor_Open,
        [CURSOR_ANIM_FIST]   = sAnim_Cursor_Fist
    };

    static const struct SpriteTemplate sSpriteTemplate_Cursor =
    {
        .tileTag = GFXTAG_CURSOR,
        .paletteTag = PALTAG_MISC,
        .oam = &sOamData_Cursor,
        .anims = sAnims_Cursor,
        .images = NULL,
        .affineAnims = gDummySpriteAffineAnimTable,
        .callback = SpriteCallbackDummy,
    };

    LoadSpriteSheets(spriteSheets);

    GetCursorCoordsByPos(sCursorArea, sCursorPosition, &x, &y);
    spriteId = CreateSprite(&sSpriteTemplate_Cursor, x, y, 6);
    if (spriteId != MAX_SPRITES)
    {
        sStorage->cursorSprite = &gSprites[spriteId];
        sStorage->cursorSprite->oam.paletteNum = IndexOfSpritePaletteTag(PALTAG_MISC);
        sStorage->cursorSprite->oam.priority = 1;
        if (sIsMonBeingMoved)
            StartSpriteAnim(sStorage->cursorSprite, CURSOR_ANIM_FIST);
    }
    else
    {
        sStorage->cursorSprite = NULL;
    }
}

static u8 GetCursorPosition(void)
{
    return sCursorPosition;
}

static void StartCursorAnim(u8 animNum)
{
    StartSpriteAnim(sStorage->cursorSprite, animNum);
}

static void TryHideItemAtCursor(void)
{
    if (sCursorArea == CURSOR_AREA_IN_BOX)
        TryHideItemIconAtPos(CURSOR_AREA_IN_BOX, sCursorPosition, FALSE);
}

static void TryShowItemAtCursor(void)
{
    if (sCursorArea == CURSOR_AREA_IN_BOX)
        TryLoadItemIconAtPos(CURSOR_AREA_IN_BOX, sCursorPosition);
}

//------------------------------------------------------------------------------
//  SECTION: Menu
//
//  The functions below handle the generic options menu that comes up whenever
//  something in the PC is selected.
//------------------------------------------------------------------------------

static void InitMenu(void)
{
    sStorage->menuItemsCount = 0;
    sStorage->menuWidth = 0;
    sStorage->menuWindow.bg = 0;
    sStorage->menuWindow.paletteNum = 15;
    sStorage->menuWindow.baseBlock = 92;
}

static const u8 gPCText_Give[] = _("GIVE");

static const u8 *const sMenuTexts[] =
{
    [MENU_CANCEL]     = COMPOUND_STRING("CANCEL"),
    [MENU_STORE]      = COMPOUND_STRING("STORE"),
    [MENU_WITHDRAW]   = COMPOUND_STRING("WITHDRAW"),
    [MENU_MOVE]       = COMPOUND_STRING("MOVE"),
    [MENU_SHIFT]      = COMPOUND_STRING("SHIFT"),
    [MENU_PLACE]      = COMPOUND_STRING("PLACE"),
    [MENU_SUMMARY]    = COMPOUND_STRING("SUMMARY"),
    [MENU_RELEASE]    = COMPOUND_STRING("RELEASE"),
    [MENU_JUMP]       = COMPOUND_STRING("JUMP"),
    [MENU_NAME]       = COMPOUND_STRING("NAME"),
    [MENU_TAKE]       = COMPOUND_STRING("TAKE"),
    [MENU_GIVE]       = gPCText_Give,
    [MENU_GIVE_2]     = gPCText_Give,
    [MENU_SWITCH]     = COMPOUND_STRING("SWITCH"),
    [MENU_BAG]        = COMPOUND_STRING("BAG"),
    [MENU_INFO]       = COMPOUND_STRING("INFO"),
};

static void SetMenuText(u8 textId)
{
    if (sStorage->menuItemsCount < ARRAY_COUNT(sStorage->menuItems))
    {
        u8 len;
        struct StorageMenu *menu = &sStorage->menuItems[sStorage->menuItemsCount];

        menu->text = sMenuTexts[textId];
        menu->textId = textId;
        len = StringLength(menu->text);
        if (len > sStorage->menuWidth)
            sStorage->menuWidth = len;

        sStorage->menuItemsCount++;
    }
}

static s8 GetMenuItemTextId(u8 menuIdx)
{
    if (menuIdx >= sStorage->menuItemsCount)
        return -1;
    else
        return sStorage->menuItems[menuIdx].textId;
}

static void AddMenu(void)
{
    sStorage->menuWindow.width = sStorage->menuWidth + 2;
    sStorage->menuWindow.height = 2 * sStorage->menuItemsCount;
    sStorage->menuWindow.tilemapLeft = 29 - sStorage->menuWindow.width;
    sStorage->menuWindow.tilemapTop = 15 - sStorage->menuWindow.height;
    sStorage->menuWindowId = AddWindow(&sStorage->menuWindow);
    ClearWindowTilemap(sStorage->menuWindowId);
    DrawStdFrameWithCustomTileAndPalette(sStorage->menuWindowId, FALSE, 11, 14);
    PrintMenuTable(sStorage->menuWindowId, sStorage->menuItemsCount, (void *)sStorage->menuItems);
    InitMenuInUpperLeftCornerNormal(sStorage->menuWindowId, sStorage->menuItemsCount, 0);
    ScheduleBgCopyTilemapToVram(0);
}

// Called after AddMenu to determine whether or not the handler callback should
// wait to move on to the next state. Evidently there was no need to wait, and
// now it always returns FALSE
static bool8 IsMenuLoading(void)
{
    return FALSE;
}

static s16 HandleMenuInput(void)
{
    s32 input = MENU_NOTHING_CHOSEN;

    do
    {
        if (JOY_NEW(A_BUTTON))
        {
            input = Menu_GetCursorPos();
            break;
        }
        else if (JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            input = MENU_B_PRESSED;
        }

        if (JOY_NEW(DPAD_UP))
        {
            PlaySE(SE_SELECT);
            Menu_MoveCursor(-1);
        }
        else if (JOY_NEW(DPAD_DOWN))
        {
            PlaySE(SE_SELECT);
            Menu_MoveCursor(1);
        }
    } while (0);

    if (input != MENU_NOTHING_CHOSEN)
        RemoveMenu();

    if (input >= 0)
        input = sStorage->menuItems[input].textId;

    return input;
}

static void RemoveMenu(void)
{
    ClearStdWindowAndFrameToTransparent(sStorage->menuWindowId, TRUE);
    RemoveWindow(sStorage->menuWindowId);
}

//------------------------------------------------------------------------------
//  SECTION: Item mode
//
//  The functions below handle the Move Items mode
//------------------------------------------------------------------------------

static const u32 sItemInfoFrame_Gfx[] = INCBIN_U32("graphics/pokemon_storage/item_info_frame.4bpp");

static const struct OamData sOamData_ItemIcon =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_NORMAL,
    .objMode = ST_OAM_OBJ_NORMAL,
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

static const union AffineAnimCmd sAffineAnim_ItemIcon_Small[] =
{
    AFFINEANIMCMD_FRAME(128, 128, 0, 0),
    AFFINEANIMCMD_END
};

static const union AffineAnimCmd sAffineAnim_ItemIcon_Appear[] =
{
    AFFINEANIMCMD_FRAME(88, 88, 0, 0),
    AFFINEANIMCMD_FRAME(5, 5, 0, 8),
    AFFINEANIMCMD_END
};

static const union AffineAnimCmd sAffineAnim_ItemIcon_Disappear[] =
{
    AFFINEANIMCMD_FRAME(128, 128, 0, 0),
    AFFINEANIMCMD_FRAME(-5, -5, 0, 8),
    AFFINEANIMCMD_END
};

static const union AffineAnimCmd sAffineAnim_ItemIcon_PickUp[] =
{
    AFFINEANIMCMD_FRAME(128, 128, 0, 0),
    AFFINEANIMCMD_FRAME(10, 10, 0, 12),
    AFFINEANIMCMD_FRAME(256, 256, 0, 0),
    AFFINEANIMCMD_END
};

static const union AffineAnimCmd sAffineAnim_ItemIcon_PutDown[] =
{
    AFFINEANIMCMD_FRAME(256, 256, 0, 0),
    AFFINEANIMCMD_FRAME(-10, -10, 0, 12),
    AFFINEANIMCMD_FRAME(128, 128, 0, 0),
    AFFINEANIMCMD_END
};

static const union AffineAnimCmd sAffineAnim_ItemIcon_PutAway[] =
{
    AFFINEANIMCMD_FRAME(256, 256, 0, 0),
    AFFINEANIMCMD_FRAME(-5, -5, 0, 16),
    AFFINEANIMCMD_END
};

static const union AffineAnimCmd sAffineAnim_ItemIcon_Large[] =
{
    AFFINEANIMCMD_FRAME(256, 256, 0, 0),
    AFFINEANIMCMD_END
};

static const union AffineAnimCmd *const sAffineAnims_ItemIcon[] =
{
    [ITEM_ANIM_NONE] = sAffineAnim_ItemIcon_Small,
    [ITEM_ANIM_APPEAR] = sAffineAnim_ItemIcon_Appear,
    [ITEM_ANIM_DISAPPEAR] = sAffineAnim_ItemIcon_Disappear,
    [ITEM_ANIM_PICK_UP] = sAffineAnim_ItemIcon_PickUp,
    [ITEM_ANIM_PUT_DOWN] = sAffineAnim_ItemIcon_PutDown,
    [ITEM_ANIM_PUT_AWAY] = sAffineAnim_ItemIcon_PutAway,
    [ITEM_ANIM_LARGE] = sAffineAnim_ItemIcon_Large
};

static const struct SpriteTemplate sSpriteTemplate_ItemIcon =
{
    .tileTag = GFXTAG_ITEM_ICON_0,
    .paletteTag = PALTAG_ITEM_ICON_0,
    .oam = &sOamData_ItemIcon,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = sAffineAnims_ItemIcon,
    .callback = SpriteCallbackDummy,
};

static void CreateItemIconSprites(void)
{
    s32 i;
    u8 spriteId;
    struct CompressedSpriteSheet spriteSheet;
    struct SpriteTemplate spriteTemplate;

    if (sStorage->boxOption == OPTION_MOVE_ITEMS)
    {
        spriteSheet.data = sItemIconGfxBuffer;
        spriteSheet.size = 512;
        spriteTemplate = sSpriteTemplate_ItemIcon;

        for (i = 0; i < MAX_ITEM_ICONS; i++)
        {
            spriteSheet.tag = GFXTAG_ITEM_ICON_0 + i;
            LoadCompressedSpriteSheet(&spriteSheet);
            sStorage->itemIcons[i].tiles = GetSpriteTileStartByTag(spriteSheet.tag) * TILE_SIZE_4BPP + (void*)(OBJ_VRAM0);
            // No longer allocated; item icons use palettes 14 & 15 now
            spriteTemplate.tileTag = GFXTAG_ITEM_ICON_0 + i;
            spriteTemplate.paletteTag = PALTAG_ITEM_ICON_0 + i;
            spriteId = CreateSprite(&spriteTemplate, 0, 0, 11);
            sStorage->itemIcons[i].sprite = &gSprites[spriteId];
            sStorage->itemIcons[i].sprite->invisible = TRUE;
            sStorage->itemIcons[i].active = FALSE;
        }
    }
    sStorage->movingItemId = ITEM_NONE;
}

static void TryLoadItemIconAtPos(u8 cursorArea, u8 cursorPos)
{
    u16 heldItem;

    if (sStorage->boxOption != OPTION_MOVE_ITEMS)
        return;

    // If we've already loaded the item here, stop
    if (IsItemIconAtPosition(cursorArea, cursorPos))
        return;

    switch (cursorArea)
    {
    case CURSOR_AREA_IN_BOX:
        if (!GetCurrentBoxMonData(cursorPos, MON_DATA_SANITY_HAS_SPECIES))
            return;
        heldItem = GetCurrentBoxMonData(cursorPos, MON_DATA_HELD_ITEM);
        break;
    case CURSOR_AREA_IN_PARTY:
        if (cursorPos >= PARTY_SIZE || !GetMonData(&gPlayerParty[cursorPos], MON_DATA_SANITY_HAS_SPECIES))
            return;
        heldItem = GetMonData(&gPlayerParty[cursorPos], MON_DATA_HELD_ITEM);
        break;
    default:
        return;
    }

    if (heldItem != ITEM_NONE)
    {
        const u32 *tiles = GetItemIconPic(heldItem);
        const u32 *pal = GetItemIconPalette(heldItem);
        u8 id = GetNewItemIconIdx();

        SetItemIconPosition(id, cursorArea, cursorPos);
        LoadItemIconGfx(id, tiles, pal);
        SetItemIconAffineAnim(id, ITEM_ANIM_APPEAR);
        SetItemIconActive(id, TRUE);
    }
}

static void TryHideItemIconAtPos(u8 cursorArea, u8 cursorPos, bool32 instant)
{
    u8 id;

    if (sStorage->boxOption != OPTION_MOVE_ITEMS)
        return;

    id = GetItemIconIdxByPosition(cursorArea, cursorPos);
    if (!instant)
        SetItemIconAffineAnim(id, ITEM_ANIM_DISAPPEAR);
    SetItemIconCallback(id, ITEM_CB_WAIT_ANIM, cursorArea, cursorPos);
}

static void TakeItemFromMon(u8 cursorArea, u8 cursorPos)
{
    u8 id;
    u16 itemId;

    if (sStorage->boxOption != OPTION_MOVE_ITEMS)
        return;

    id = GetItemIconIdxByPosition(cursorArea, cursorPos);
    itemId = ITEM_NONE;
    SetItemIconAffineAnim(id, ITEM_ANIM_PICK_UP);
    SetItemIconCallback(id, ITEM_CB_TO_HAND, cursorArea, cursorPos);
    SetItemIconPosition(id, CURSOR_AREA_BOX_TITLE, 0);
    if (cursorArea == CURSOR_AREA_IN_BOX)
    {
        SetCurrentBoxMonData(cursorPos, MON_DATA_HELD_ITEM, &itemId);
        SetBoxMonIconObjMode(cursorPos, ST_OAM_OBJ_BLEND);
    }
    else
    {
        SetMonData(&gPlayerParty[cursorPos], MON_DATA_HELD_ITEM, &itemId);
        SetPartyMonIconObjMode(cursorPos, ST_OAM_OBJ_BLEND);
    }

    sStorage->movingItemId = sStorage->displayMonItemId;
}

static void InitItemIconInCursor(u16 itemId)
{
    const u32 *tiles = GetItemIconPic(itemId);
    const u32 *pal = GetItemIconPalette(itemId);
    u8 id = GetNewItemIconIdx();
    LoadItemIconGfx(id, tiles, pal);
    SetItemIconAffineAnim(id, ITEM_ANIM_LARGE);
    SetItemIconCallback(id, ITEM_CB_TO_HAND, CURSOR_AREA_IN_BOX, 0);
    SetItemIconPosition(id, CURSOR_AREA_BOX_TITLE, 0);
    SetItemIconActive(id, TRUE);
    sStorage->movingItemId = itemId;
}

static void SwapItemsWithMon(u8 cursorArea, u8 cursorPos)
{
    u8 id;
    u16 itemId;

    if (sStorage->boxOption != OPTION_MOVE_ITEMS)
        return;

    id = GetItemIconIdxByPosition(cursorArea, cursorPos);
    SetItemIconAffineAnim(id, ITEM_ANIM_PICK_UP);
    SetItemIconCallback(id, ITEM_CB_SWAP_TO_HAND, CURSOR_AREA_BOX_TITLE, 0);
    if (cursorArea == CURSOR_AREA_IN_BOX)
    {
        itemId = GetCurrentBoxMonData(cursorPos, MON_DATA_HELD_ITEM);
        SetCurrentBoxMonData(cursorPos, MON_DATA_HELD_ITEM, &sStorage->movingItemId);
        sStorage->movingItemId = itemId;
    }
    else
    {
        itemId = GetMonData(&gPlayerParty[cursorPos], MON_DATA_HELD_ITEM);
        SetMonData(&gPlayerParty[cursorPos], MON_DATA_HELD_ITEM, &sStorage->movingItemId);
        sStorage->movingItemId = itemId;
    }

    id = GetItemIconIdxByPosition(CURSOR_AREA_BOX_TITLE, 0);
    SetItemIconAffineAnim(id, ITEM_ANIM_PUT_DOWN);
    SetItemIconCallback(id, ITEM_CB_SWAP_TO_MON, cursorArea, cursorPos);
}

static void GiveItemToMon(u8 cursorArea, u8 cursorPos)
{
    u8 id;

    if (sStorage->boxOption != OPTION_MOVE_ITEMS)
        return;

    id = GetItemIconIdxByPosition(CURSOR_AREA_BOX_TITLE, 0);
    SetItemIconAffineAnim(id, ITEM_ANIM_PUT_DOWN);
    SetItemIconCallback(id, ITEM_CB_TO_MON, cursorArea, cursorPos);
    if (cursorArea == CURSOR_AREA_IN_BOX)
    {
        SetCurrentBoxMonData(cursorPos, MON_DATA_HELD_ITEM, &sStorage->movingItemId);
        SetBoxMonIconObjMode(cursorPos, ST_OAM_OBJ_NORMAL);
    }
    else
    {
        SetMonData(&gPlayerParty[cursorPos], MON_DATA_HELD_ITEM, &sStorage->movingItemId);
        SetPartyMonIconObjMode(cursorPos, ST_OAM_OBJ_NORMAL);
    }
}

static void MoveItemFromMonToBag(u8 cursorArea, u8 cursorPos)
{
    u8 id;
    u16 itemId;

    if (sStorage->boxOption != OPTION_MOVE_ITEMS)
        return;

    itemId = ITEM_NONE;
    id = GetItemIconIdxByPosition(cursorArea, cursorPos);
    SetItemIconAffineAnim(id, ITEM_ANIM_DISAPPEAR);
    SetItemIconCallback(id, ITEM_CB_WAIT_ANIM, cursorArea, cursorPos);
    if (cursorArea == CURSOR_AREA_IN_BOX)
    {
        SetCurrentBoxMonData(cursorPos, MON_DATA_HELD_ITEM, &itemId);
        SetBoxMonIconObjMode(cursorPos, ST_OAM_OBJ_BLEND);
    }
    else
    {
        SetMonData(&gPlayerParty[cursorPos], MON_DATA_HELD_ITEM, &itemId);
        SetPartyMonIconObjMode(cursorPos, ST_OAM_OBJ_BLEND);
    }
}

static void MoveItemFromCursorToBag(void)
{
    if (sStorage->boxOption == OPTION_MOVE_ITEMS)
    {
        u8 id = GetItemIconIdxByPosition(CURSOR_AREA_BOX_TITLE, 0);
        SetItemIconAffineAnim(id, ITEM_ANIM_PUT_AWAY);
        SetItemIconCallback(id, ITEM_CB_WAIT_ANIM, CURSOR_AREA_BOX_TITLE, 0);
    }
}

// The party menu is being closed, if the cursor is on
// a Pokémon that has a held item make sure it slides
// up along with the closing menu.
static void MoveHeldItemWithPartyMenu(void)
{
    s32 i;

    if (sStorage->boxOption != OPTION_MOVE_ITEMS)
        return;

    for (i = 0; i < MAX_ITEM_ICONS; i++)
    {
        if (sStorage->itemIcons[i].active
         && sStorage->itemIcons[i].area == CURSOR_AREA_IN_PARTY)
            SetItemIconCallback(i, ITEM_CB_HIDE_PARTY, CURSOR_AREA_BOX_TITLE, 0);
    }
}

static bool8 IsItemIconAnimActive(void)
{
    s32 i;

    for (i = 0; i < MAX_ITEM_ICONS; i++)
    {
        if (sStorage->itemIcons[i].active)
        {
            if (!sStorage->itemIcons[i].sprite->affineAnimEnded
              && sStorage->itemIcons[i].sprite->affineAnimBeginning)
                return TRUE;
            if (sStorage->itemIcons[i].sprite->callback != SpriteCallbackDummy
             && sStorage->itemIcons[i].sprite->callback != SpriteCB_ItemIcon_SetPosToCursor)
                return TRUE;
        }
    }
    return FALSE;
}

static bool8 IsMovingItem(void)
{
    s32 i;

    if (sStorage->boxOption == OPTION_MOVE_ITEMS)
    {
        for (i = 0; i < MAX_ITEM_ICONS; i++)
        {
            if (sStorage->itemIcons[i].active
             && sStorage->itemIcons[i].area == CURSOR_AREA_BOX_TITLE)
                return TRUE;
        }
    }
    return FALSE;
}

static const u8 *GetMovingItemName(void)
{
    return ItemId_GetName(sStorage->movingItemId);
}

static u16 GetMovingItemId(void)
{
    return sStorage->movingItemId;
}

static u8 GetNewItemIconIdx(void)
{
    u8 i;

    for (i = 0; i < MAX_ITEM_ICONS; i++)
    {
        if (!sStorage->itemIcons[i].active)
        {
            sStorage->itemIcons[i].active = TRUE;
            return i;
        }
    }
    return MAX_ITEM_ICONS;
}

static bool32 IsItemIconAtPosition(u8 cursorArea, u8 cursorPos)
{
    s32 i;

    for (i = 0; i < MAX_ITEM_ICONS; i++)
    {
        if (sStorage->itemIcons[i].active
         && sStorage->itemIcons[i].area == cursorArea
         && sStorage->itemIcons[i].pos == cursorPos)
            return TRUE;
    }
    return FALSE;
}

static u8 GetItemIconIdxByPosition(u8 cursorArea, u8 cursorPos)
{
    u8 i;

    for (i = 0; i < MAX_ITEM_ICONS; i++)
    {
        if (sStorage->itemIcons[i].active
         && sStorage->itemIcons[i].area == cursorArea
         && sStorage->itemIcons[i].pos == cursorPos)
            return i;
    }
    return MAX_ITEM_ICONS;
}

static u8 GetItemIconIdxBySprite(struct Sprite *sprite)
{
    u8 i;

    for (i = 0; i < MAX_ITEM_ICONS; i++)
    {
        if (sStorage->itemIcons[i].active
         && sStorage->itemIcons[i].sprite == sprite)
            return i;
    }
    return MAX_ITEM_ICONS;
}

static void SetItemIconPosition(u8 id, u8 cursorArea, u8 cursorPos)
{
    u8 x, y;

    if (id >= MAX_ITEM_ICONS)
        return;

    switch (cursorArea)
    {
    case CURSOR_AREA_IN_BOX:
        x = cursorPos % IN_BOX_COLUMNS;
        y = cursorPos/IN_BOX_COLUMNS;
        sStorage->itemIcons[id].sprite->x = (24 * x) + 112;
        sStorage->itemIcons[id].sprite->y = (24 * y) + 56;
        sStorage->itemIcons[id].sprite->oam.priority = 2;
        break;
    case CURSOR_AREA_IN_PARTY:
        if (cursorPos == 0)
        {
            sStorage->itemIcons[id].sprite->x = 116;
            sStorage->itemIcons[id].sprite->y = 76;
        }
        else
        {
            sStorage->itemIcons[id].sprite->x = 164;
            sStorage->itemIcons[id].sprite->y = 24 * (cursorPos - 1) + 28;
        }
        sStorage->itemIcons[id].sprite->oam.priority = 1;
        break;
    }

    sStorage->itemIcons[id].area = cursorArea;
    sStorage->itemIcons[id].pos = cursorPos;
}

static void LoadItemIconGfx(u8 id, const u32 *itemTiles, const u32 *itemPal)
{
    s32 i;
    u32 paletteNum = 14;

    if (id >= MAX_ITEM_ICONS)
        return;

    CpuFastFill(0, sStorage->itemIconBuffer, 512);
    LZ77UnCompWram(itemTiles, sStorage->tileBuffer);
    for (i = 0; i < 3; i++)
        CpuFastCopy(&sStorage->tileBuffer[i * 96], &sStorage->itemIconBuffer[i * 128], 96);

    CpuFastCopy(sStorage->itemIconBuffer, sStorage->itemIcons[id].tiles, 512);
    LZ77UnCompWram(itemPal, sStorage->itemIconBuffer);

    // Load into free item palette slot,
    // the one not being used by an item icon
    for (i = 0; i < MAX_ITEM_ICONS; i++)
    {
        if (i != id && sStorage->itemIcons[i].active)
        {
            paletteNum = sStorage->itemIcons[i].sprite->oam.paletteNum ^ 1;
            if (sStorage->itemIcons[i].area == CURSOR_AREA_BOX_TITLE)
                break;
        }
    }
    sStorage->itemIcons[id].sprite->oam.paletteNum = paletteNum;
    LoadPalette(sStorage->itemIconBuffer, OBJ_PLTT_ID(paletteNum), PLTT_SIZE_4BPP);
}

static void SetItemIconAffineAnim(u8 id, u8 animNum)
{
    if (id >= MAX_ITEM_ICONS)
        return;

    StartSpriteAffineAnim(sStorage->itemIcons[id].sprite, animNum);
}

#define sItemIconId data[0]
#define sState      data[0]
#define sCursorArea data[6]
#define sCursorPos  data[7]

static void SetItemIconCallback(u8 id, u8 callbackId, u8 cursorArea, u8 cursorPos)
{
    if (id >= MAX_ITEM_ICONS)
        return;

    switch (callbackId)
    {
    case ITEM_CB_WAIT_ANIM:
        sStorage->itemIcons[id].sprite->sItemIconId = id;
        sStorage->itemIcons[id].sprite->callback = SpriteCB_ItemIcon_WaitAnim;
        break;
    case ITEM_CB_TO_HAND:
        sStorage->itemIcons[id].sprite->sState = 0;
        sStorage->itemIcons[id].sprite->callback = SpriteCB_ItemIcon_ToHand;
        break;
    case ITEM_CB_TO_MON:
        sStorage->itemIcons[id].sprite->sState = 0;
        sStorage->itemIcons[id].sprite->sCursorArea = cursorArea;
        sStorage->itemIcons[id].sprite->sCursorPos = cursorPos;
        sStorage->itemIcons[id].sprite->callback = SpriteCB_ItemIcon_ToMon;
        break;
    case ITEM_CB_SWAP_TO_HAND:
        sStorage->itemIcons[id].sprite->sState = 0;
        sStorage->itemIcons[id].sprite->callback = SpriteCB_ItemIcon_SwapToHand;
        sStorage->itemIcons[id].sprite->sCursorArea = cursorArea;
        sStorage->itemIcons[id].sprite->sCursorPos = cursorPos;
        break;
    case ITEM_CB_SWAP_TO_MON:
        sStorage->itemIcons[id].sprite->sState = 0;
        sStorage->itemIcons[id].sprite->sCursorArea = cursorArea;
        sStorage->itemIcons[id].sprite->sCursorPos = cursorPos;
        sStorage->itemIcons[id].sprite->callback = SpriteCB_ItemIcon_SwapToMon;
        break;
    case ITEM_CB_HIDE_PARTY:
        // If cursor is on a Pokémon with a held item and
        // the player closes the party menu, have the held
        // item follow the Pokémon as the menu slides out
        sStorage->itemIcons[id].sprite->callback = SpriteCB_ItemIcon_HideParty;
        break;
    }
}

static void SetItemIconActive(u8 id, bool8 active)
{
    if (id >= MAX_ITEM_ICONS)
        return;

    sStorage->itemIcons[id].active = active;
    sStorage->itemIcons[id].sprite->invisible = (active == FALSE);
}

static void PrintItemDescription(void)
{
    const u8 *description;

    if (IsMovingItem())
        description = ItemId_GetDescription(sStorage->movingItemId);
    else
        description = ItemId_GetDescription(sStorage->displayMonItemId);

    FillWindowPixelBuffer(WIN_ITEM_DESC, PIXEL_FILL(1));
    AddTextPrinterParameterized5(WIN_ITEM_DESC, FONT_NORMAL, description, 4, 0, 0, NULL, 0, 1);
}

static void InitItemInfoWindow(void)
{
    sStorage->itemInfoWindowOffset = 21;
    LoadBgTiles(0, sItemInfoFrame_Gfx, 128, 311);
    DrawItemInfoWindow(0);
}

static bool8 UpdateItemInfoWindowSlideIn(void)
{
    s32 i, pos;

    if (sStorage->itemInfoWindowOffset == 0)
        return FALSE;

    sStorage->itemInfoWindowOffset--;
    pos = 21 - sStorage->itemInfoWindowOffset;
    for (i = 0; i < pos; i++)
        WriteSequenceToBgTilemapBuffer(0, GetBgAttribute(0, BG_ATTR_BASETILE) + 20 + sStorage->itemInfoWindowOffset + i, i, 13, 1, 7, 15, 21);

    DrawItemInfoWindow(pos);
    return (sStorage->itemInfoWindowOffset != 0);
}

static bool8 UpdateItemInfoWindowSlideOut(void)
{
    s32 i, pos;

    if (sStorage->itemInfoWindowOffset == 22)
        return FALSE;

    if (sStorage->itemInfoWindowOffset == 0)
        FillBgTilemapBufferRect(0, 0, 21, 12, 1, 9, 17);

    sStorage->itemInfoWindowOffset++;
    pos = 21 - sStorage->itemInfoWindowOffset;
    for (i = 0; i < pos; i++)
    {
        WriteSequenceToBgTilemapBuffer(0, GetBgAttribute(0, BG_ATTR_BASETILE) + 20 + sStorage->itemInfoWindowOffset + i, i, 13, 1, 7, 15, 21);
    }

    if (pos >= 0)
        DrawItemInfoWindow(pos);

    FillBgTilemapBufferRect(0, 0, pos + 1, 12, 1, 9, 17);
    ScheduleBgCopyTilemapToVram(0);
    return TRUE;
}

static void DrawItemInfoWindow(u32 x)
{
    if (x != 0)
    {
        FillBgTilemapBufferRect(0, 314, 0, 12, x, 1, 15);
        FillBgTilemapBufferRect(0, 2362, 0, 20, x, 1, 15);
    }
    FillBgTilemapBufferRect(0, 315, x, 13, 1, 7, 15);
    FillBgTilemapBufferRect(0, 316, x, 12, 1, 1, 15);
    FillBgTilemapBufferRect(0, 317, x, 20, 1, 1, 15);
    ScheduleBgCopyTilemapToVram(0);
}

static void SpriteCB_ItemIcon_WaitAnim(struct Sprite *sprite)
{
    if (sprite->affineAnimEnded)
    {
        SetItemIconActive(sprite->sItemIconId, FALSE);
        sprite->callback = SpriteCallbackDummy;
    }
}

static void SpriteCB_ItemIcon_ToHand(struct Sprite *sprite)
{
    switch (sprite->sState)
    {
    case 0:
        sprite->data[1] = sprite->x << 4;
        sprite->data[2] = sprite->y << 4;
        sprite->data[3] = 10;
        sprite->data[4] = 21;
        sprite->data[5] = 0;
        sprite->sState++;
    case 1:
        sprite->data[1] -= sprite->data[3];
        sprite->data[2] -= sprite->data[4];
        sprite->x = sprite->data[1] >> 4;
        sprite->y = sprite->data[2] >> 4;
        if (++sprite->data[5] > 11)
            sprite->callback = SpriteCB_ItemIcon_SetPosToCursor;
        break;
    }
}

static void SpriteCB_ItemIcon_SetPosToCursor(struct Sprite *sprite)
{
    sprite->x = sStorage->cursorSprite->x + 4;
    sprite->y = sStorage->cursorSprite->y + sStorage->cursorSprite->y2 + 8;
    sprite->oam.priority = sStorage->cursorSprite->oam.priority;
}

static void SpriteCB_ItemIcon_ToMon(struct Sprite *sprite)
{
    switch (sprite->sState)
    {
    case 0:
        sprite->data[1] = sprite->x << 4;
        sprite->data[2] = sprite->y << 4;
        sprite->data[3] = 10;
        sprite->data[4] = 21;
        sprite->data[5] = 0;
        sprite->sState++;
    case 1:
        sprite->data[1] += sprite->data[3];
        sprite->data[2] += sprite->data[4];
        sprite->x = sprite->data[1] >> 4;
        sprite->y = sprite->data[2] >> 4;
        if (++sprite->data[5] > 11)
        {
            SetItemIconPosition(GetItemIconIdxBySprite(sprite), sprite->sCursorArea, sprite->sCursorPos);
            sprite->callback = SpriteCallbackDummy;
        }
        break;
    }
}

static void SpriteCB_ItemIcon_SwapToHand(struct Sprite *sprite)
{
    switch (sprite->sState)
    {
    case 0:
        sprite->data[1] = sprite->x << 4;
        sprite->data[2] = sprite->y << 4;
        sprite->data[3] = 10;
        sprite->data[4] = 21;
        sprite->data[5] = 0;
        sprite->sState++;
    case 1:
        sprite->data[1] -= sprite->data[3];
        sprite->data[2] -= sprite->data[4];
        sprite->x = sprite->data[1] >> 4;
        sprite->y = sprite->data[2] >> 4;
        sprite->x2 = gSineTable[sprite->data[5] * 8] >> 4;
        if (++sprite->data[5] > 11)
        {
            SetItemIconPosition(GetItemIconIdxBySprite(sprite), sprite->sCursorArea, sprite->sCursorPos);
            sprite->x2 = 0;
            sprite->callback = SpriteCB_ItemIcon_SetPosToCursor;
        }
        break;
    }
}

static void SpriteCB_ItemIcon_SwapToMon(struct Sprite *sprite)
{
    switch (sprite->sState)
    {
    case 0:
        sprite->data[1] = sprite->x << 4;
        sprite->data[2] = sprite->y << 4;
        sprite->data[3] = 10;
        sprite->data[4] = 21;
        sprite->data[5] = 0;
        sprite->sState++;
    case 1:
        sprite->data[1] += sprite->data[3];
        sprite->data[2] += sprite->data[4];
        sprite->x = sprite->data[1] >> 4;
        sprite->y = sprite->data[2] >> 4;
        sprite->x2 = -(gSineTable[sprite->data[5] * 8] >> 4);
        if (++sprite->data[5] > 11)
        {
            SetItemIconPosition(GetItemIconIdxBySprite(sprite), sprite->sCursorArea, sprite->sCursorPos);
            sprite->callback = SpriteCallbackDummy;
            sprite->x2 = 0;
        }
        break;
    }
}

static void SpriteCB_ItemIcon_HideParty(struct Sprite *sprite)
{
    sprite->y -= 8;
    if (sprite->y + sprite->y2 < -16)
    {
        sprite->callback = SpriteCallbackDummy;
        SetItemIconActive(GetItemIconIdxBySprite(sprite), FALSE);
    }
}

#undef sState
#undef sItemIconId
#undef sCursorArea
#undef sCursorPos

//------------------------------------------------------------------------------
//  SECTION: General utility
//------------------------------------------------------------------------------
// Functions here are general utility functions.

u8 StorageGetCurrentBox(void)
{
    return gPokemonStoragePtr->currentBox;
}

static void SetCurrentBox(u8 boxId)
{
    if (boxId < TOTAL_BOXES_COUNT)
        gPokemonStoragePtr->currentBox = boxId;
}

u32 GetBoxMonDataAt(u8 boxId, u8 boxPosition, s32 request)
{
    if (boxId < TOTAL_BOXES_COUNT && boxPosition < IN_BOX_COUNT)
        return GetBoxMonData(&gPokemonStoragePtr->boxes[boxId][boxPosition], request);
    else
        return 0;
}

void SetBoxMonDataAt(u8 boxId, u8 boxPosition, s32 request, const void *value)
{
    if (boxId < TOTAL_BOXES_COUNT && boxPosition < IN_BOX_COUNT)
        SetBoxMonData(&gPokemonStoragePtr->boxes[boxId][boxPosition], request, value);
}

u32 GetCurrentBoxMonData(u8 boxPosition, s32 request)
{
    return GetBoxMonDataAt(gPokemonStoragePtr->currentBox, boxPosition, request);
}

void SetCurrentBoxMonData(u8 boxPosition, s32 request, const void *value)
{
    SetBoxMonDataAt(gPokemonStoragePtr->currentBox, boxPosition, request, value);
}

void GetBoxMonNickAt(u8 boxId, u8 boxPosition, u8 *dst)
{
    if (boxId < TOTAL_BOXES_COUNT && boxPosition < IN_BOX_COUNT)
        GetBoxMonData(&gPokemonStoragePtr->boxes[boxId][boxPosition], MON_DATA_NICKNAME, dst);
    else
        *dst = EOS;
}

u32 GetBoxMonLevelAt(u8 boxId, u8 boxPosition)
{
    u32 lvl;

    if (boxId < TOTAL_BOXES_COUNT && boxPosition < IN_BOX_COUNT && GetBoxMonData(&gPokemonStoragePtr->boxes[boxId][boxPosition], MON_DATA_SANITY_HAS_SPECIES))
        lvl = GetLevelFromBoxMonExp(&gPokemonStoragePtr->boxes[boxId][boxPosition]);
#ifdef BUGFIX
    else
#endif
        lvl = 0;

    return lvl;
}

void SetBoxMonNickAt(u8 boxId, u8 boxPosition, const u8 *nick)
{
    if (boxId < TOTAL_BOXES_COUNT && boxPosition < IN_BOX_COUNT)
        SetBoxMonData(&gPokemonStoragePtr->boxes[boxId][boxPosition], MON_DATA_NICKNAME, nick);
}

u32 GetAndCopyBoxMonDataAt(u8 boxId, u8 boxPosition, s32 request, void *dst)
{
    if (boxId < TOTAL_BOXES_COUNT && boxPosition < IN_BOX_COUNT)
        return GetBoxMonData(&gPokemonStoragePtr->boxes[boxId][boxPosition], request, dst);
    else
        return 0;
}

void SetBoxMonAt(u8 boxId, u8 boxPosition, struct BoxPokemon *src)
{
    if (boxId < TOTAL_BOXES_COUNT && boxPosition < IN_BOX_COUNT)
        gPokemonStoragePtr->boxes[boxId][boxPosition] = *src;
}

void CopyBoxMonAt(u8 boxId, u8 boxPosition, struct BoxPokemon *dst)
{
    if (boxId < TOTAL_BOXES_COUNT && boxPosition < IN_BOX_COUNT)
        *dst = gPokemonStoragePtr->boxes[boxId][boxPosition];
}

void CreateBoxMonAt(u8 boxId, u8 boxPosition, u16 species, u8 level, u8 fixedIV, u8 hasFixedPersonality, u32 personality, u8 otIDType, u32 otID)
{
    if (boxId < TOTAL_BOXES_COUNT && boxPosition < IN_BOX_COUNT)
    {
        CreateBoxMon(&gPokemonStoragePtr->boxes[boxId][boxPosition],
                     species,
                     level,
                     fixedIV,
                     hasFixedPersonality, personality,
                     otIDType, otID);
    }
}

void ZeroBoxMonAt(u8 boxId, u8 boxPosition)
{
    if (boxId < TOTAL_BOXES_COUNT && boxPosition < IN_BOX_COUNT)
        ZeroBoxMonData(&gPokemonStoragePtr->boxes[boxId][boxPosition]);
}

void BoxMonAtToMon(u8 boxId, u8 boxPosition, struct Pokemon *dst)
{
    if (boxId < TOTAL_BOXES_COUNT && boxPosition < IN_BOX_COUNT)
        BoxMonToMon(&gPokemonStoragePtr->boxes[boxId][boxPosition], dst);
}

struct BoxPokemon *GetBoxedMonPtr(u8 boxId, u8 boxPosition)
{
    if (boxId < TOTAL_BOXES_COUNT && boxPosition < IN_BOX_COUNT)
        return &gPokemonStoragePtr->boxes[boxId][boxPosition];
    else
        return NULL;
}

u8 *GetBoxNamePtr(u8 boxId)
{
    if (boxId < TOTAL_BOXES_COUNT)
        return gPokemonStoragePtr->boxNames[boxId];
    else
        return NULL;
}

// For moving to the next Pokémon while viewing the summary screen
s16 AdvanceStorageMonIndex(struct BoxPokemon *boxMons, u8 currIndex, u8 maxIndex, u8 mode)
{
    s16 i;
    s16 direction = -1;

    if (mode == 0 || mode == 1)
        direction = 1;

    if (mode == 1 || mode == 3)
    {
        for (i = (s8)currIndex + direction; i >= 0 && i <= maxIndex; i += direction)
        {
            if (GetBoxMonData(&boxMons[i], MON_DATA_SPECIES) != SPECIES_NONE)
                return i;
        }
    }
    else
    {
        for (i = (s8)currIndex + direction; i >= 0 && i <= maxIndex; i += direction)
        {
            if (GetBoxMonData(&boxMons[i], MON_DATA_SPECIES) != SPECIES_NONE
                && !GetBoxMonData(&boxMons[i], MON_DATA_IS_EGG))
                return i;
        }
    }

    return -1;
}

bool8 CheckFreePokemonStorageSpace(void)
{
    s32 i, j;

    for (i = 0; i < TOTAL_BOXES_COUNT; i++)
    {
        for (j = 0; j < IN_BOX_COUNT; j++)
        {
            if (!GetBoxMonData(&gPokemonStoragePtr->boxes[i][j], MON_DATA_SANITY_HAS_SPECIES))
                return TRUE;
        }
    }

    return FALSE;
}

bool32 CheckBoxMonSanityAt(u32 boxId, u32 boxPosition)
{
    if (boxId < TOTAL_BOXES_COUNT
        && boxPosition < IN_BOX_COUNT
        && GetBoxMonData(&gPokemonStoragePtr->boxes[boxId][boxPosition], MON_DATA_SANITY_HAS_SPECIES)
        && !GetBoxMonData(&gPokemonStoragePtr->boxes[boxId][boxPosition], MON_DATA_SANITY_IS_EGG)
        && !GetBoxMonData(&gPokemonStoragePtr->boxes[boxId][boxPosition], MON_DATA_SANITY_IS_BAD_EGG))
        return TRUE;
    else
        return FALSE;
}

u32 CountStorageNonEggMons(void)
{
    s32 i, j;
    u32 count = 0;

    for (i = 0; i < TOTAL_BOXES_COUNT; i++)
    {
        for (j = 0; j < IN_BOX_COUNT; j++)
        {
            if (GetBoxMonData(&gPokemonStoragePtr->boxes[i][j], MON_DATA_SANITY_HAS_SPECIES)
                && !GetBoxMonData(&gPokemonStoragePtr->boxes[i][j], MON_DATA_SANITY_IS_EGG))
                count++;
        }
    }

    return count;
}

u32 CountAllStorageMons(void)
{
    s32 i, j;
    u32 count = 0;

    for (i = 0; i < TOTAL_BOXES_COUNT; i++)
    {
        for (j = 0; j < IN_BOX_COUNT; j++)
        {
            if (GetBoxMonData(&gPokemonStoragePtr->boxes[i][j], MON_DATA_SANITY_HAS_SPECIES)
                || GetBoxMonData(&gPokemonStoragePtr->boxes[i][j], MON_DATA_SANITY_IS_EGG))
                count++;
        }
    }

    return count;
}

bool32 AnyStorageMonWithMove(u16 moveId)
{
    u16 moves[] = {moveId, MOVES_COUNT};
    s32 i, j;

    for (i = 0; i < TOTAL_BOXES_COUNT; i++)
    {
        for (j = 0; j < IN_BOX_COUNT; j++)
        {
            if (GetBoxMonData(&gPokemonStoragePtr->boxes[i][j], MON_DATA_SANITY_HAS_SPECIES)
                && !GetBoxMonData(&gPokemonStoragePtr->boxes[i][j], MON_DATA_SANITY_IS_EGG)
                && GetBoxMonData(&gPokemonStoragePtr->boxes[i][j], MON_DATA_KNOWN_MOVES, (u8 *)moves))
                return TRUE;
        }
    }

    return FALSE;
}

//------------------------------------------------------------------------------
//  SECTION: TilemapUtil
//
//  Handles 3 particular tilemaps in a somewhat unusual way.
//  For example, while the cursor is on the Close Box button it flashes between
//  two states alternately. Both these states are their own part of the same
//  tilemap that's always present. The utility shifts the tilemap up and down
//  to show/hide the states, and limits the view with a rectangle that only
//  reveals one at a time.
//  Each tilemap is tracked with a TILEMAPID that can be used to reference it.
//------------------------------------------------------------------------------

struct TilemapUtil_RectData
{
    s16 x;
    s16 y;
    u16 width;
    u16 height;
    s16 destX;
    s16 destY;
};

struct TilemapUtil
{
    struct TilemapUtil_RectData cur;
    const void *tilemap;
    u16 altWidth;
    u16 width;
    u8 tileSize;
    u8 bg;
    bool8 active;
};

EWRAM_DATA static struct TilemapUtil *sTilemapUtil = NULL;
EWRAM_DATA static u16 sNumTilemapUtilIds = 0;

static void TilemapUtil_Init(u8 count)
{
    u16 i;

    sTilemapUtil = Alloc(sizeof(*sTilemapUtil) * count);
    sNumTilemapUtilIds = (sTilemapUtil == NULL) ? 0 : count;
    for (i = 0; i < sNumTilemapUtilIds; i++)
    {
        sTilemapUtil[i].active = FALSE;
    }
}

static void TilemapUtil_Free(void)
{
    Free(sTilemapUtil);
}

struct
{
    u16 width;
    u16 height;
} static const sTilemapDimensions[][4] =
{
    [BG_TYPE_NORMAL] = {
        { 256,  256},
        { 512,  256},
        { 256,  512},
        { 512,  512},
    },
    [BG_TYPE_AFFINE] = {
        { 128,  128},
        { 256,  256},
        { 512,  512},
        {1024, 1024},
    },
};

static void TilemapUtil_SetMap(u8 id, u8 bg, const void *tilemap, u16 width, u16 height)
{
    u16 bgScreenSize, bgType;

    if (id >= sNumTilemapUtilIds)
        return;

    sTilemapUtil[id].tilemap = tilemap;
    sTilemapUtil[id].bg = bg;
    sTilemapUtil[id].width = width;

    bgScreenSize = GetBgAttribute(bg, BG_ATTR_SCREENSIZE);
    bgType = GetBgAttribute(bg, BG_ATTR_TYPE);
    sTilemapUtil[id].altWidth = sTilemapDimensions[bgType][bgScreenSize].width;
    if (bgType != BG_TYPE_NORMAL)
        sTilemapUtil[id].tileSize = 1;
    else
        sTilemapUtil[id].tileSize = 2;

    sTilemapUtil[id].cur.width = width;
    sTilemapUtil[id].cur.height = height;
    sTilemapUtil[id].cur.x = 0;
    sTilemapUtil[id].cur.y = 0;
    sTilemapUtil[id].cur.destX = 0;
    sTilemapUtil[id].cur.destY = 0;
    sTilemapUtil[id].active = TRUE;
}

static void TilemapUtil_SetPos(u8 id, u16 x, u16 y)
{
    if (id >= sNumTilemapUtilIds)
        return;

    sTilemapUtil[id].cur.destX = x;
    sTilemapUtil[id].cur.destY = y;
    sTilemapUtil[id].active = TRUE;
}

static void TilemapUtil_SetRect(u8 id, u16 x, u16 y, u16 width, u16 height)
{
    if (id >= sNumTilemapUtilIds)
        return;

    sTilemapUtil[id].cur.x = x;
    sTilemapUtil[id].cur.y = y;
    sTilemapUtil[id].cur.width = width;
    sTilemapUtil[id].cur.height = height;
    sTilemapUtil[id].active = TRUE;
}

static void TilemapUtil_Move(s8 val)
{
    sTilemapUtil[TILEMAPID_PARTY_MENU].cur.y -= val;
    sTilemapUtil[TILEMAPID_PARTY_MENU].cur.height += val;
    sTilemapUtil[TILEMAPID_PARTY_MENU].active = TRUE;
}

static void TilemapUtil_Update(u8 id)
{
    if (id >= sNumTilemapUtilIds)
        return;

    TilemapUtil_Draw(id);
}

static void TilemapUtil_Draw(u8 id)
{
    s32 i;
    u32 adder = sTilemapUtil[id].tileSize * sTilemapUtil[id].width;
    const void *tiles = (sTilemapUtil[id].tilemap + (adder * sTilemapUtil[id].cur.y))
                      + (sTilemapUtil[id].tileSize * sTilemapUtil[id].cur.x);

    for (i = 0; i < sTilemapUtil[id].cur.height; i++)
    {
        CopyToBgTilemapBufferRect(sTilemapUtil[id].bg,
                                  tiles,
                                  sTilemapUtil[id].cur.destX,
                                  sTilemapUtil[id].cur.destY + i,
                                  sTilemapUtil[id].cur.width,
                                  1);
        tiles += adder;
    }
}
