#include "global.h"
#include "malloc.h"
#include "battle_main.h"
#include "bg.h"
#include "decompress.h"
#include "gpu_regs.h"
#include "graphics.h"
#include "task.h"
#include "text_window.h"
#include "menu.h"
#include "menu_helpers.h"
#include "list_menu.h"
#include "item.h"
#include "item_menu.h"
#include "link.h"
#include "money.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon_icon.h"
#include "pokemon_summary_screen.h"
#include "pokemon_storage_system.h"
#include "party_menu.h"
#include "data.h"
#include "scanline_effect.h"
#include "shop.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "text.h"
#include "tm_case.h"
#include "constants/items.h"
#include "constants/pokemon_icon.h"
#include "constants/songs.h"
#include "constants/rgb.h"

#define TAG_SCROLL_ARROW 110

enum {
    WIN_LIST,
    WIN_TITLE,
    WIN_MOVE_INFO
};

// Window IDs for the context menu that opens when a TM/HM is selected
enum {
    WIN_USE_EXIT,
    WIN_EXIT
};

// IDs for the actions in the context menu
enum {
    ACTION_USE,
    ACTION_EXIT
};

enum {
    COLOR_LIGHT,
    COLOR_DARK,
    COLOR_CURSOR_SELECTED,
    COLOR_MOVE_INFO,
    COLOR_TITLE
};

// The "static" resources are preserved even if the TM case is exited. This is
// useful for when its left temporarily (e.g. going to the party menu to teach a TM)
// but also to preserve the selected item when the TM case is fully closed.
static EWRAM_DATA struct {
    void (* exitCallback)(void);
    u8 menuType;
    bool8 allowSelectClose;
    u16 selectedRow;
    u16 scrollOffset;
} sTMCaseStaticResources = {};

// The "dynamic" resources will be reset any time the TM case is exited, even temporarily.
static EWRAM_DATA struct {
    void (* nextScreenCallback)(void);
    u8 maxTMsShown;
    u8 numTMs;
    u8 contextMenuWindowId;
    u8 scrollArrowsTaskId;
    u16 currItem;
    const u8 * menuActionIndices;
    u8 numMenuActions;
    s16 seqId;
    u8 typeIconSpriteId;
    u8 categoryIconSpriteId;
} * sTMCaseDynamicResources = NULL;

static EWRAM_DATA void *sTilemapBuffer = NULL;
static EWRAM_DATA struct ListMenuItem * sListMenuItemsBuffer = NULL;
static EWRAM_DATA u8 (* sListMenuStringsBuffer)[29] = NULL;
static EWRAM_DATA u8    spriteIdData[PARTY_SIZE] = {};

static void CB2_SetUpTMCaseUI_Blocking(void);
static bool8 DoSetUpTMCaseUI(void);
static void ResetBufferPointers_NoFree(void);
static void LoadBGTemplates(void);
static bool8 HandleLoadTMCaseGraphicsAndPalettes(void);
static void CreateTMCaseListMenuBuffers(void);
static void InitTMCaseListMenuItems(void);
static void GetTMNumberAndMoveString(u8 * dest, u16 itemId);
static void List_MoveCursorFunc(s32 itemIndex, bool8 onInit, struct ListMenu *list);
static void PrintMoveInfo(u16 itemId);
static void PrintListCursorAtRow(u8 y, u8 colorIdx);
static void CreateListScrollArrows(void);
static void TMCaseSetup_GetTMCount(void);
static void TMCaseSetup_InitListMenuPositions(void);
static void TMCaseSetup_UpdateVisualMenuOffset(void);
static void Task_FadeOutAndCloseTMCase(u8 taskId);
static void Task_HandleListInput(u8 taskId);
static void Task_SelectedTMHM_Field(u8 taskId);
static void Task_ContextMenu_HandleInput(u8 taskId);
static void Action_Use(u8 taskId);
static void Action_Exit(u8 taskId);
static void InitWindowTemplatesAndPals(void);
static void TMCase_Print(u8 windowId, u8 fontId, const u8 * str, u8 x, u8 y, u8 letterSpacing, u8 lineSpacing, u8 speed, u8 colorIdx);
static void TMCase_SetWindowBorder(u8 windowId);
static void PrintTitle(void);
static u8 AddContextMenu(u8 * windowId, u8 windowIndex);
static void RemoveContextMenu(u8 * windowId);
static void DrawPartyMonIcons(void);
static void TintPartyMonIcons(u16 tm);

static const struct BgTemplate sBGTemplates[] = {
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    }, {
        .bg = 1,
        .charBaseIndex = 0,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    }, {
        .bg = 2,
        .charBaseIndex = 0,
        .mapBaseIndex = 29,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0
    }
};

static const struct MenuAction sMenuActions[] = {
    [ACTION_USE]  = {gText_Use,  {Action_Use}},
    [ACTION_EXIT] = {gText_Salir, {Action_Exit}},
};

static const u8 sMenuActionIndices_Field[] = {ACTION_USE, ACTION_EXIT};

static const u8 sText_SingleSpace[] = _(" ");

static const u8 sTextColors[][3] = {
    [COLOR_LIGHT] = {0, 6, 5},
    [COLOR_DARK] = {0, 2, 3},
    [COLOR_CURSOR_SELECTED] = {0, 3, 6},
    [COLOR_MOVE_INFO] = {0, 4, 6},
    [COLOR_TITLE] = {0, 1, 2},
};

static const struct WindowTemplate sWindowTemplates[] = {
    [WIN_LIST] = {
        .bg = 0,
        .tilemapLeft = 15,
        .tilemapTop = 2,
        .width = 19,
        .height = 14,
        .paletteNum = 1,
        .baseBlock = 129
    },
    [WIN_TITLE] = {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 0,
        .width = 14,
        .height = 2,
        .paletteNum = 1,
        .baseBlock = 565
    },
    [WIN_MOVE_INFO] = {
        .bg = 0,
        .tilemapLeft = 8,
        .tilemapTop = 18,
        .width = 22,
        .height = 2,
        .paletteNum = 1,
        .baseBlock = 800
    },
    DUMMY_WIN_TEMPLATE
};

static const struct WindowTemplate sYesNoWindowTemplate = {
    .bg = 1,
    .tilemapLeft = 21,
    .tilemapTop = 9,
    .width = 6,
    .height = 4,
    .paletteNum = 15,
    .baseBlock = 821
};

static const struct WindowTemplate sWindowTemplates_ContextMenu[] = {
    [WIN_USE_EXIT] = {
        .bg = 1,
        .tilemapLeft = 24,
        .tilemapTop = 15,
        .width = 5,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 463
    },
    [WIN_EXIT] = {
        .bg = 1,
        .tilemapLeft = 22,
        .tilemapTop = 15,
        .width = 5,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 463
    },
};

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

void InitTMCase(u8 type, void (* exitCallback)(void), bool8 allowSelectClose)
{
    ResetBufferPointers_NoFree();
    sTMCaseDynamicResources = Alloc(sizeof(*sTMCaseDynamicResources));
    sTMCaseDynamicResources->nextScreenCallback = NULL;
    sTMCaseDynamicResources->scrollArrowsTaskId = TASK_NONE;
    sTMCaseDynamicResources->contextMenuWindowId = WINDOW_NONE;
    sTMCaseDynamicResources->typeIconSpriteId = 0xFF;
    sTMCaseDynamicResources->categoryIconSpriteId = 0xFF;
    if (type != TMCASE_REOPENING)
        sTMCaseStaticResources.menuType = type;
    if (exitCallback != NULL)
        sTMCaseStaticResources.exitCallback = exitCallback;
    if (allowSelectClose != TMCASE_KEEP_PREV)
        sTMCaseStaticResources.allowSelectClose = allowSelectClose;
    gTextFlags.autoScroll = FALSE;
    SetMainCallback2(CB2_SetUpTMCaseUI_Blocking);
    FadeOutAndPlayNewMapMusic(MUS_DP_TV_STATION, 4);
}

static void CB2_Idle(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    DoScheduledBgTilemapCopiesToVram();
    UpdatePaletteFade();
}

static void VBlankCB_Idle(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_SetUpTMCaseUI_Blocking(void)
{
    while (1)
    {
        if (MenuHelpers_ShouldWaitForLinkRecv() == TRUE)
            break;
        if (DoSetUpTMCaseUI() == TRUE)
            break;
        if (MenuHelpers_IsLinkActive() == TRUE)
            break;
    }
}

#define tListTaskId       data[0]
#define tSelection        data[1]

static bool8 DoSetUpTMCaseUI(void)
{
    u8 taskId;

    switch (gMain.state)
    {
    case 0:
        SetVBlankHBlankCallbacksToNull();
        ClearScheduledBgCopiesToVram();
        gMain.state++;
        break;
    case 1:
        ScanlineEffect_Stop();
        gMain.state++;
        break;
    case 2:
        FreeAllSpritePalettes();
        gMain.state++;
        break;
    case 3:
        ResetPaletteFade();
        gMain.state++;
        break;
    case 4:
        ResetSpriteData();
        gMain.state++;
        break;
    case 5:
        ResetTasks();
        gMain.state++;
        break;
    case 6:
        LoadBGTemplates();
        sTMCaseDynamicResources->seqId = 0;
        gMain.state++;
        break;
    case 7:
        InitWindowTemplatesAndPals();
        gMain.state++;
        break;
    case 8:
        if (HandleLoadTMCaseGraphicsAndPalettes())
            gMain.state++;
        break;
    case 9:
        UpdatePocketItemList(TMHM_POCKET);
        gMain.state++;
        break;
    case 10:
        TMCaseSetup_GetTMCount();
        TMCaseSetup_InitListMenuPositions();
        TMCaseSetup_UpdateVisualMenuOffset();
        gMain.state++;
        break;
    case 11:
        DrawPartyMonIcons();
        gMain.state++;
        break;
    case 12:
        CreateTMCaseListMenuBuffers();
        InitTMCaseListMenuItems();
        gMain.state++;
        break;
    case 13:
        PrintTitle();
        gMain.state++;
        break;
    case 14:
        taskId = CreateTask(Task_HandleListInput, 0);
        gTasks[taskId].tListTaskId = ListMenuInit(&gMultiuseListMenuTemplate, sTMCaseStaticResources.scrollOffset, sTMCaseStaticResources.selectedRow);
        gMain.state++;
        break;
    case 15:
        CreateListScrollArrows();
        gMain.state++;
        break;
    case 16:
        BlendPalettes(PALETTES_ALL, 16, 0);
        gMain.state++;
        break;
    case 17:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        gMain.state++;
        break;
    default:
        SetVBlankCallback(VBlankCB_Idle);
        SetMainCallback2(CB2_Idle);
        return TRUE;
    }

    return FALSE;
}

static void ResetBufferPointers_NoFree(void)
{
    sTMCaseDynamicResources = NULL;
    sTilemapBuffer = NULL;
    sListMenuItemsBuffer = NULL;
    sListMenuStringsBuffer = NULL;
}

static void LoadBGTemplates(void)
{
    void ** ptr;
    ResetVramOamAndBgCntRegs();
    ResetAllBgsCoordinates();
    ptr = &sTilemapBuffer;
    *ptr = AllocZeroed(2048);
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sBGTemplates, ARRAY_COUNT(sBGTemplates));
    SetBgTilemapBuffer(2, *ptr);
    ScheduleBgCopyTilemapToVram(1);
    ScheduleBgCopyTilemapToVram(2);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
    SetGpuReg(REG_OFFSET_BLDCNT, 0);
    ShowBg(0);
    ShowBg(1);
    ShowBg(2);
}

static bool8 HandleLoadTMCaseGraphicsAndPalettes(void)
{
    switch (sTMCaseDynamicResources->seqId)
    {
    case 0:
        ResetTempTileDataBuffers();
        DecompressAndCopyTileDataToVram(1, gTMCase_Gfx, 0, 0, 0);
        sTMCaseDynamicResources->seqId++;
        break;
    case 1:
        if (FreeTempTileDataBuffersIfPossible() != TRUE)
        {
            LZDecompressWram(gTMCaseMenu_Tilemap, sTilemapBuffer);
            sTMCaseDynamicResources->seqId++;
        }
        break;
    case 2:
        LoadCompressedPalette(gTMCaseMenu_Pal, BG_PLTT_ID(0), PLTT_SIZE_4BPP);
        LoadCompressedPalette(gTMCaseText_Pal, BG_PLTT_ID(1), PLTT_SIZE_4BPP);
        sTMCaseDynamicResources->seqId++;
        break;
    default:
        sTMCaseDynamicResources->seqId = 0;
        return TRUE;
    }

    return FALSE;
}

static void CreateTMCaseListMenuBuffers(void)
{
    struct BagPocket * pocket = &gBagPockets[POCKET_TM_HM - 1];
    sListMenuItemsBuffer = Alloc((pocket->capacity + 1) * sizeof(struct ListMenuItem));
    sListMenuStringsBuffer = Alloc(sTMCaseDynamicResources->numTMs * 29);
}

static void InitTMCaseListMenuItems(void)
{
    struct BagPocket * pocket = &gBagPockets[POCKET_TM_HM - 1];
    u16 i;

    for (i = 0; i < sTMCaseDynamicResources->numTMs; i++)
    {
        GetTMNumberAndMoveString(sListMenuStringsBuffer[i], pocket->itemSlots[i].itemId);
        sListMenuItemsBuffer[i].name = sListMenuStringsBuffer[i];
        sListMenuItemsBuffer[i].id = i;
    }

    gMultiuseListMenuTemplate.items = sListMenuItemsBuffer;
    gMultiuseListMenuTemplate.totalItems = sTMCaseDynamicResources->numTMs;
    gMultiuseListMenuTemplate.windowId = WIN_LIST;
    gMultiuseListMenuTemplate.header_X = 0;
    gMultiuseListMenuTemplate.item_X = 8;
    gMultiuseListMenuTemplate.cursor_X = 0;
    gMultiuseListMenuTemplate.lettersSpacing = 0;
    gMultiuseListMenuTemplate.itemVerticalPadding = 2;
    gMultiuseListMenuTemplate.upText_Y = 0;
    gMultiuseListMenuTemplate.maxShowed = sTMCaseDynamicResources->maxTMsShown;
    gMultiuseListMenuTemplate.fontId = FONT_SHORT;
    gMultiuseListMenuTemplate.cursorPal = 2;
    gMultiuseListMenuTemplate.fillValue = 0;
    gMultiuseListMenuTemplate.cursorShadowPal = 3;
    gMultiuseListMenuTemplate.moveCursorFunc = List_MoveCursorFunc;
    gMultiuseListMenuTemplate.cursorKind = CURSOR_BLACK_ARROW;
    gMultiuseListMenuTemplate.itemPrintFunc = NULL;
    gMultiuseListMenuTemplate.scrollMultiple = LIST_MULTIPLE_SCROLL_L_R;
}

static void GetTMNumberAndMoveString(u8 * dest, u16 itemId)
{
    StringCopy(gStringVar4, gText_FontBig);
    if (itemId >= ITEM_HM01)
    {
        StringAppend(gStringVar4, gText_MO);
        ConvertIntToDecimalStringN(gStringVar1, itemId - ITEM_HM01 + 1, STR_CONV_MODE_LEADING_ZEROS, 2);
        StringAppend(gStringVar4, gStringVar1);
    }
    else
    {
        StringAppend(gStringVar4, gText_MT);
        ConvertIntToDecimalStringN(gStringVar1, itemId - ITEM_TM01 + 1, STR_CONV_MODE_LEADING_ZEROS, 2);
        StringAppend(gStringVar4, gStringVar1);
    }
    StringAppend(gStringVar4, sText_SingleSpace);
    StringAppend(gStringVar4, gText_FontNormal);
    StringAppend(gStringVar4, gMovesInfo[ItemIdToBattleMoveId(itemId)].name);
    StringCopy(dest, gStringVar4);
}

static void List_MoveCursorFunc(s32 itemIndex, bool8 onInit, struct ListMenu *list)
{
    u16 itemId;

    itemId = BagGetItemIdByPocketPosition(POCKET_TM_HM, itemIndex);
    if (onInit != TRUE)
    {
        PlaySE(SE_RG_BAG_CURSOR);
    }
    if (sTMCaseDynamicResources->typeIconSpriteId != 0xFF)
    {
        DestroySpriteAndFreeResources(&gSprites[sTMCaseDynamicResources->typeIconSpriteId]);
        sTMCaseDynamicResources->typeIconSpriteId = 0xFF;
    }
    if (sTMCaseDynamicResources->categoryIconSpriteId != 0xFF)
    {
        DestroySpriteAndFreeResources(&gSprites[sTMCaseDynamicResources->categoryIconSpriteId]);
        sTMCaseDynamicResources->categoryIconSpriteId = 0xFF;
    }
    TintPartyMonIcons(itemId);
    PrintMoveInfo(itemId);
}

static void PrintMoveInfo(u16 itemId)
{
    u8 i;
    u16 move;
    const u8 * str;
    struct Sprite *sprite;

    FillWindowPixelBuffer(WIN_MOVE_INFO, 0);
    if (itemId == ITEM_NONE)
    {
        for (i = 0; i < 4; i++)
            TMCase_Print(WIN_MOVE_INFO, FONT_NORMAL, gText_ThreeDashes, 2, 0, 1, 0, 0, COLOR_LIGHT);
    }
    else
    {
        // Draw type icon
        LoadCompressedSpriteSheet(&gSpriteSheet_MoveTypes);
        LoadCompressedPalette(gMoveTypes_Pal, OBJ_PLTT_ID(14), 2 * PLTT_SIZE_4BPP);
        move = ItemIdToBattleMoveId(itemId);
        if (sTMCaseDynamicResources->typeIconSpriteId == 0xFF)
        {
		    sTMCaseDynamicResources->typeIconSpriteId = CreateSpriteAtEnd(&gSpriteTemplate_MoveTypes, 16, 152, 0);
		    sprite = &gSprites[sTMCaseDynamicResources->typeIconSpriteId];
		    StartSpriteAnim(sprite, gMovesInfo[move].type);
		    sprite->oam.paletteNum = gTypesInfo[gMovesInfo[move].type].palette;
        }
        // Draw category icon
        LoadCompressedSpriteSheet(&gSpriteSheet_CategoryIcons);
        LoadSpritePalette(&gSpritePal_CategoryIcons);
        move = ItemIdToBattleMoveId(itemId);
        if (sTMCaseDynamicResources->categoryIconSpriteId == 0xFF)
        {
		    sTMCaseDynamicResources->categoryIconSpriteId = CreateSpriteAtEnd(&gSpriteTemplate_CategoryIcons, 48, 152, 0);
		    sprite = &gSprites[sTMCaseDynamicResources->categoryIconSpriteId];
		    StartSpriteAnim(sprite, gMovesInfo[move].category);
        }
        // Print power
        if (gMovesInfo[move].power < 2)
            str = gText_ThreeDashes;
        else
        {
            ConvertIntToDecimalStringN(gStringVar1, gMovesInfo[move].power, STR_CONV_MODE_RIGHT_ALIGN, 3);
            str = gStringVar1;
        }
            TMCase_Print(WIN_MOVE_INFO, FONT_BIG, gText_Pot, 9, 0, 1, 0, 0, COLOR_TITLE);
            TMCase_Print(WIN_MOVE_INFO, FONT_NORMAL, str, 36, 0, 1, 0, 0, COLOR_LIGHT);

        // Print accuracy
        if (gMovesInfo[move].accuracy == 0)
            str = gText_ThreeDashes;
        else
        {
            ConvertIntToDecimalStringN(gStringVar1, gMovesInfo[move].accuracy, STR_CONV_MODE_RIGHT_ALIGN, 3);
            str = gStringVar1;
        }
            TMCase_Print(WIN_MOVE_INFO, FONT_BIG, gText_Pre, 56, 0, 1, 0, 0, COLOR_TITLE);
            TMCase_Print(WIN_MOVE_INFO, FONT_NORMAL, str, 84, 0, 1, 0, 0, COLOR_LIGHT);

        // Print PP
        ConvertIntToDecimalStringN(gStringVar1, gMovesInfo[move].pp, STR_CONV_MODE_RIGHT_ALIGN, 3);
        TMCase_Print(WIN_MOVE_INFO, FONT_BIG, gText_PP, 105, 0, 1, 0, 0, COLOR_TITLE);
        TMCase_Print(WIN_MOVE_INFO, FONT_NORMAL, gStringVar1, 116, 0, 1, 0, 0, COLOR_LIGHT);

        // Print Priority
        TMCase_Print(WIN_MOVE_INFO, FONT_BIG, gText_Pri, 137, 0, 1, 0, 0, COLOR_TITLE);
        if (gMovesInfo[move].priority == 0)
        {
            TMCase_Print(WIN_MOVE_INFO, FONT_NORMAL, gText_OneDash, 164, 0, 1, 0, 0, COLOR_LIGHT);
        }
        if (gMovesInfo[move].priority > 0)
        {
            TMCase_Print(WIN_MOVE_INFO, FONT_BIG, gText_UpArrow, 164, 0, 1, 0, 0, COLOR_LIGHT);
        }
        if (gMovesInfo[move].priority < 0)
        {
            TMCase_Print(WIN_MOVE_INFO, FONT_BIG, gText_DownArrow, 164, 0, 1, 0, 0, COLOR_LIGHT);
        }
    }
}

static void PrintListCursor(u8 listTaskId, u8 colorIdx)
{
    PrintListCursorAtRow(ListMenuGetYCoordForPrintingArrowCursor(listTaskId), colorIdx);
}

static void PrintListCursorAtRow(u8 y, u8 colorIdx)
{
    TMCase_Print(WIN_LIST, FONT_NORMAL, gText_SelectorArrow2, 0, y, 0, 0, 0, colorIdx);
}


static void CreateListScrollArrows(void)
{
    sTMCaseDynamicResources->scrollArrowsTaskId = AddScrollIndicatorArrowPairParameterized(SCROLL_ARROW_UP,
                                                                                           180, 8, 132,
                                                                                           sTMCaseDynamicResources->numTMs - sTMCaseDynamicResources->maxTMsShown,
                                                                                           TAG_SCROLL_ARROW, TAG_SCROLL_ARROW,
                                                                                           &sTMCaseStaticResources.scrollOffset);
}

static void RemoveScrollArrows(void)
{
    if (sTMCaseDynamicResources->scrollArrowsTaskId != TASK_NONE)
    {
        RemoveScrollIndicatorArrowPair(sTMCaseDynamicResources->scrollArrowsTaskId);
        sTMCaseDynamicResources->scrollArrowsTaskId = TASK_NONE;
    }
}

static void TMCaseSetup_GetTMCount(void)
{
    struct BagPocket * pocket = &gBagPockets[POCKET_TM_HM - 1];
    u16 i;

    UpdatePocketItemList(TMHM_POCKET);
    sTMCaseDynamicResources->numTMs = 0;
    for (i = 0; i < pocket->capacity; i++)
    {
        if (pocket->itemSlots[i].itemId == ITEM_NONE)
            break;
        sTMCaseDynamicResources->numTMs++;
    }
    sTMCaseDynamicResources->maxTMsShown = min(sTMCaseDynamicResources->numTMs, 7);
}

static void TMCaseSetup_InitListMenuPositions(void)
{
    if (sTMCaseStaticResources.scrollOffset != 0)
    {
        if (sTMCaseStaticResources.scrollOffset + sTMCaseDynamicResources->maxTMsShown > sTMCaseDynamicResources->numTMs)
            sTMCaseStaticResources.scrollOffset = sTMCaseDynamicResources->numTMs - sTMCaseDynamicResources->maxTMsShown;
    }
    if (sTMCaseStaticResources.scrollOffset + sTMCaseStaticResources.selectedRow >= sTMCaseDynamicResources->numTMs)
    {
        if (sTMCaseDynamicResources->numTMs < 2)
            sTMCaseStaticResources.selectedRow = 0;
        else
            sTMCaseStaticResources.selectedRow = sTMCaseDynamicResources->numTMs;
    }
}

static void TMCaseSetup_UpdateVisualMenuOffset(void)
{
    u8 i;
    if (sTMCaseStaticResources.selectedRow > 3)
    {
        for (i = 0; i <= sTMCaseStaticResources.selectedRow - 3 && sTMCaseStaticResources.scrollOffset + sTMCaseDynamicResources->maxTMsShown != sTMCaseDynamicResources->numTMs; i++)
        {
            do {} while (0);
            sTMCaseStaticResources.selectedRow--;
            sTMCaseStaticResources.scrollOffset++;
        }
    }
}

static void DestroyTMCaseBuffers(void)
{
    if (sTMCaseDynamicResources != NULL)
        Free(sTMCaseDynamicResources);
    if (sTilemapBuffer != NULL)
        Free(sTilemapBuffer);
    if (sListMenuItemsBuffer != NULL)
        Free(sListMenuItemsBuffer);
    if (sListMenuStringsBuffer != NULL)
        Free(sListMenuStringsBuffer);
    FreeAllWindowBuffers();
}

static void Task_BeginFadeOutFromTMCase(u8 taskId)
{
    BeginNormalPaletteFade(PALETTES_ALL, -2, 0, 16, RGB_BLACK);
    gTasks[taskId].func = Task_FadeOutAndCloseTMCase;
}

static void Task_FadeOutAndCloseTMCase(u8 taskId)
{
    s16 * data = gTasks[taskId].data;
    u16 music = GetCurrLocationDefaultMusic();

    if (!gPaletteFade.active)
    {
        DestroyListMenuTask(tListTaskId, &sTMCaseStaticResources.scrollOffset, &sTMCaseStaticResources.selectedRow);
        if (sTMCaseDynamicResources->nextScreenCallback != NULL)
            SetMainCallback2(sTMCaseDynamicResources->nextScreenCallback);
        else
            SetMainCallback2(sTMCaseStaticResources.exitCallback);
        RemoveScrollArrows();
        DestroyTMCaseBuffers();
        DestroyTask(taskId);
        FadeOutAndPlayNewMapMusic(music, 8);
    }
}

static void Task_HandleListInput(u8 taskId)
{
    s16 * data = gTasks[taskId].data;
    s32 input;

    if (!gPaletteFade.active)
    {
        input = ListMenu_ProcessInput(tListTaskId);
        ListMenuGetScrollAndRow(tListTaskId, &sTMCaseStaticResources.scrollOffset, &sTMCaseStaticResources.selectedRow);
        if (JOY_NEW(SELECT_BUTTON) && sTMCaseStaticResources.allowSelectClose == TRUE)
        {
            PlaySE(SE_SELECT);
            gSpecialVar_ItemId = ITEM_NONE;
            Task_BeginFadeOutFromTMCase(taskId);
        }
        else
        {
            switch (input)
            {
            case LIST_NOTHING_CHOSEN:
                break;
            case LIST_CANCEL:
                PlaySE(SE_SELECT);
                gSpecialVar_ItemId = ITEM_NONE;
                Task_BeginFadeOutFromTMCase(taskId);
                break;
            default:
                PlaySE(SE_SELECT);
                RemoveScrollArrows();
                PrintListCursor(tListTaskId, COLOR_CURSOR_SELECTED);
                tSelection = input;
                gSpecialVar_ItemId = BagGetItemIdByPocketPosition(POCKET_TM_HM, input);
                gTasks[taskId].func = Task_SelectedTMHM_Field;
                break;
            }
        }
    }
}

static void ReturnToList(u8 taskId)
{
    CreateListScrollArrows();
    gTasks[taskId].func = Task_HandleListInput;
}

// When a TM/HM in the list is selected in the field, create a context
// menu with a list of actions that can be taken.
static void Task_SelectedTMHM_Field(u8 taskId)
{
    u8 * strbuf;
    
    // Regular TM/HM context menu
    AddContextMenu(&sTMCaseDynamicResources->contextMenuWindowId, WIN_USE_EXIT);
    sTMCaseDynamicResources->menuActionIndices = sMenuActionIndices_Field;
    sTMCaseDynamicResources->numMenuActions = ARRAY_COUNT(sMenuActionIndices_Field);

    // Print context window actions
    PrintMenuActionTexts(sTMCaseDynamicResources->contextMenuWindowId,
                                  FONT_NORMAL,
                                  GetMenuCursorDimensionByFont(FONT_NORMAL, 0),
                                  2,
                                  0,
                                  GetFontAttribute(FONT_NORMAL, FONTATTR_MAX_LETTER_HEIGHT) + 2,
                                  sTMCaseDynamicResources->numMenuActions,
                                  sMenuActions,
                                  sTMCaseDynamicResources->menuActionIndices);

    Menu_InitCursor(sTMCaseDynamicResources->contextMenuWindowId, FONT_NORMAL, 0, 2, GetFontAttribute(FONT_NORMAL, FONTATTR_MAX_LETTER_HEIGHT) + 2, sTMCaseDynamicResources->numMenuActions, 0);
    
    // Print label text next to the context window
    strbuf = Alloc(256);
    GetTMNumberAndMoveString(strbuf, gSpecialVar_ItemId);
    StringAppend(strbuf, gText_Var1IsSelected + 2); // +2 skips over the stringvar
    Free(strbuf);
    ScheduleBgCopyTilemapToVram(0);
    ScheduleBgCopyTilemapToVram(1);
    gTasks[taskId].func = Task_ContextMenu_HandleInput;
}

static void Task_ContextMenu_HandleInput(u8 taskId)
{
    s8 input;

    input = Menu_ProcessInputNoWrapAround();
        switch (input)
        {
        case MENU_B_PRESSED:
            // Run last action in list (Exit)
            PlaySE(SE_SELECT);
            sMenuActions[sTMCaseDynamicResources->menuActionIndices[sTMCaseDynamicResources->numMenuActions - 1]].func.void_u8(taskId);
            break;
        case MENU_NOTHING_CHOSEN:
            break;
        default:
            PlaySE(SE_SELECT);
            sMenuActions[sTMCaseDynamicResources->menuActionIndices[input]].func.void_u8(taskId);
            break;
        }
}

static void Action_Use(u8 taskId)
{
    RemoveContextMenu(&sTMCaseDynamicResources->contextMenuWindowId);
    PutWindowTilemap(WIN_LIST);
    ScheduleBgCopyTilemapToVram(0);
    ScheduleBgCopyTilemapToVram(1);
    // Chose a TM/HM to use, exit TM case for party menu
    gItemUseCB = ItemUseCB_TMHM;
    sTMCaseDynamicResources->nextScreenCallback = CB2_ShowPartyMenuForItemUse;
    Task_BeginFadeOutFromTMCase(taskId);
}

static void Action_Exit(u8 taskId)
{
    s16 * data = gTasks[taskId].data;

    RemoveContextMenu(&sTMCaseDynamicResources->contextMenuWindowId);
    PutWindowTilemap(WIN_LIST);
    PrintListCursor(tListTaskId, COLOR_DARK);
    PutWindowTilemap(WIN_MOVE_INFO);
    ScheduleBgCopyTilemapToVram(0);
    ScheduleBgCopyTilemapToVram(1);
    ReturnToList(taskId);
}

static void InitWindowTemplatesAndPals(void)
{
    u8 i;

    InitWindows(sWindowTemplates);
    DeactivateAllTextPrinters();
    LoadUserWindowBorderGfx(0, 120, BG_PLTT_ID(13));
    LoadPalette(gStandardMenuPalette, BG_PLTT_ID(15), PLTT_SIZE_4BPP);
    LoadPalette(gStandardMenuPalette, BG_PLTT_ID(10), PLTT_SIZE_4BPP);
    for (i = 0; i < ARRAY_COUNT(sWindowTemplates) - 1; i++)
        FillWindowPixelBuffer(i, 0);
    PutWindowTilemap(WIN_LIST);
    PutWindowTilemap(WIN_TITLE);
    PutWindowTilemap(WIN_MOVE_INFO);
    ScheduleBgCopyTilemapToVram(0);
}

static void TMCase_Print(u8 windowId, u8 fontId, const u8 * str, u8 x, u8 y, u8 letterSpacing, u8 lineSpacing, u8 speed, u8 colorIdx)
{
    AddTextPrinterParameterized4(windowId, fontId, x, y, letterSpacing, lineSpacing, sTextColors[colorIdx], speed, str);
}

static void TMCase_SetWindowBorder(u8 windowId)
{
    DrawStdFrameWithCustomTileAndPalette(windowId, FALSE, 120, 13);
}

static void PrintTitle(void)
{
    AddTextPrinterParameterized3(WIN_TITLE, FONT_BIG, 0, 0, sTextColors[COLOR_TITLE], 0, gText_TMCase);
}

static u8 AddContextMenu(u8 * windowId, u8 windowIndex)
{
    if (*windowId == WINDOW_NONE)
    {
        *windowId = AddWindow(&sWindowTemplates_ContextMenu[windowIndex]);
        TMCase_SetWindowBorder(*windowId);
        ScheduleBgCopyTilemapToVram(0);
    }
    return *windowId;
}

static void RemoveContextMenu(u8 * windowId)
{
    ClearStdWindowAndFrameToTransparent(*windowId, FALSE);
    ClearWindowTilemap(*windowId);
    RemoveWindow(*windowId);
    ScheduleBgCopyTilemapToVram(0);
    *windowId = WINDOW_NONE;
}

#define sMonIconStill data[3]
static void SpriteCb_MonIcon(struct Sprite *sprite)
{
    if (!sprite->sMonIconStill)
        UpdateMonIconFrame(sprite);
}
#undef sMonIconStill

#define MON_ICON_START_X  8
#define MON_ICON_START_Y  30
#define MON_ICON_PADDING  16

static void DrawPartyMonIcons(void)
{
    u8 i;
    u16 species;
    u8 icon_x = 0;
    u8 icon_y = 0;

    //LoadMonIconPalettes();

    for (i = 0; i < gPlayerPartyCount; i++)
    {
        //calc icon position (centered)
        if (gPlayerPartyCount == 1)
        {
            icon_x = MON_ICON_START_X + MON_ICON_PADDING*0.5;
            icon_y = MON_ICON_START_Y + MON_ICON_PADDING;
        }
        else if (gPlayerPartyCount == 2)
        {
            icon_x = MON_ICON_START_X + MON_ICON_PADDING*0.5;
            icon_y = i < 2 ? MON_ICON_START_Y + MON_ICON_PADDING*0.5 + MON_ICON_PADDING * i : MON_ICON_START_Y + MON_ICON_PADDING*0.5 + MON_ICON_PADDING * (i - 2);
        }
        else if (gPlayerPartyCount == 3)
        {
            icon_x = MON_ICON_START_X + MON_ICON_PADDING*0.5;
            icon_y = i < 3 ? MON_ICON_START_Y + MON_ICON_PADDING * i : MON_ICON_START_Y + MON_ICON_PADDING * (i - 3);
        }
        else if (gPlayerPartyCount == 4)
        {
            icon_x = i < 2 ? MON_ICON_START_X : MON_ICON_START_X + MON_ICON_PADDING;
            icon_y = i < 2 ? MON_ICON_START_Y + MON_ICON_PADDING*0.5 + MON_ICON_PADDING * i : MON_ICON_START_Y + MON_ICON_PADDING*0.5 + MON_ICON_PADDING * (i - 2);
        }
        else
        {
            icon_x = i < 3 ? MON_ICON_START_X : MON_ICON_START_X + MON_ICON_PADDING;
            icon_y = i < 3 ? MON_ICON_START_Y + MON_ICON_PADDING * i : MON_ICON_START_Y + MON_ICON_PADDING * (i - 3);
        }
        species = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG);
        spriteIdData[i] = CreateMonIcon(species, SpriteCb_MonIcon, icon_x, icon_y, 1, GetMonData(&gPlayerParty[i], MON_DATA_PERSONALITY));
        u8 index = i < gPlayerPartyCount ? IndexOfSpritePaletteTag(POKE_ICON_BASE_PAL_TAG + i) : 0xFF;
        SetMonIconPalette(&gPlayerParty[i], &gSprites[spriteIdData[i]], index);
        gSprites[spriteIdData[i]].oam.priority = 0;
        StartSpriteAnim(&gSprites[spriteIdData[i]], 0);
    }
}

static void TintPartyMonIcons(u16 tm)
{
    u8 i;
    u16 species;

    for (i = 0; i < gPlayerPartyCount; i++)
    {
        species = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG);
        SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT2_ALL);
        SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(7, 11));
        if (!CanLearnTeachableMove(species, ItemIdToBattleMoveId(tm)))
        {
            gSprites[spriteIdData[i]].oam.objMode = ST_OAM_OBJ_BLEND;        
        }
        else
        {
            gSprites[spriteIdData[i]].oam.objMode = ST_OAM_OBJ_NORMAL;
        }
    }
}
