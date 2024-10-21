#include "global.h"
#include "battle_pyramid.h"
#include "bg.h"
#include "event_data.h"
#include "field_weather.h"
#include "gpu_regs.h"
#include "graphics.h"
#include "international_string_util.h"
#include "main.h"
#include "menu.h"
#include "map_name_popup.h"
#include "palette.h"
#include "region_map.h"
#include "rtc.h"
#include "start_menu.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "constants/battle_frontier.h"
#include "constants/layouts.h"
#include "constants/region_map_sections.h"
#include "constants/weather.h"
#include "config/general.h"
#include "config/overworld.h"

enum MapPopUp_Themes_BW
{
    MAPPOPUP_THEME_BW_DEFAULT,
};

// static functions
static void Task_MapNamePopUpWindow(u8 taskId);
static void ShowMapNamePopUpWindow(void);
static void LoadMapNamePopUpWindowBg(void);

// EWRAM
EWRAM_DATA u8 gPopupTaskId = 0;

static const u8 sMapPopUpTilesPrimary_BW[] = INCBIN_U8("graphics/map_popup/bw_primary.4bpp");
static const u8 sMapPopUpTilesSecondary_BW[] = INCBIN_U8("graphics/map_popup/bw_secondary.4bpp");
static const u16 sMapPopUpTilesPalette_BW_Black[16] = INCBIN_U16("graphics/map_popup/black.gbapal");
static const u16 sMapPopUpTilesPalette_BW_White[16] = INCBIN_U16("graphics/map_popup/white.gbapal");

static const u8 sText_PyramidFloor1[] = _("PYRAMID FLOOR 1");
static const u8 sText_PyramidFloor2[] = _("PYRAMID FLOOR 2");
static const u8 sText_PyramidFloor3[] = _("PYRAMID FLOOR 3");
static const u8 sText_PyramidFloor4[] = _("PYRAMID FLOOR 4");
static const u8 sText_PyramidFloor5[] = _("PYRAMID FLOOR 5");
static const u8 sText_PyramidFloor6[] = _("PYRAMID FLOOR 6");
static const u8 sText_PyramidFloor7[] = _("PYRAMID FLOOR 7");
static const u8 sText_Pyramid[] = _("PYRAMID");

static const u8 *const sBattlePyramid_MapHeaderStrings[FRONTIER_STAGES_PER_CHALLENGE + 1] =
{
    sText_PyramidFloor1,
    sText_PyramidFloor2,
    sText_PyramidFloor3,
    sText_PyramidFloor4,
    sText_PyramidFloor5,
    sText_PyramidFloor6,
    sText_PyramidFloor7,
    sText_Pyramid,
};

// States and data defines for Task_MapNamePopUpWindow
enum {
    STATE_SLIDE_IN,
    STATE_WAIT,
    STATE_SLIDE_OUT,
    STATE_UNUSED,
    STATE_ERASE,
    STATE_END,
    STATE_PRINT, // For some reason the first state is numerically last.
};

#define POPUP_OFFSCREEN_Y  ((OW_POPUP_GENERATION == GEN_5) ? 24 : 40)
#define POPUP_SLIDE_SPEED  2

#define tState         data[0]
#define tOnscreenTimer data[1]
#define tYOffset       data[2]
#define tIncomingPopUp data[3]
#define tPrintTimer    data[4]

void ShowMapNamePopup(void)
{
    if (FlagGet(FLAG_HIDE_MAP_NAME_POPUP) != TRUE)
    {
        if (!FuncIsActiveTask(Task_MapNamePopUpWindow))
        {
            // New pop up window
            if (OW_POPUP_GENERATION == GEN_5)
            {
                gPopupTaskId = CreateTask(Task_MapNamePopUpWindow, 100);

                if (OW_POPUP_BW_ALPHA_BLEND && !IsWeatherAlphaBlend())
                    SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_BG0 | BLDCNT_TGT2_ALL | BLDCNT_EFFECT_BLEND);
            }
            else
            {
                gPopupTaskId = CreateTask(Task_MapNamePopUpWindow, 90);
                SetGpuReg(REG_OFFSET_BG0VOFS, POPUP_OFFSCREEN_Y);
            }

            gTasks[gPopupTaskId].tState = STATE_PRINT;
            gTasks[gPopupTaskId].tYOffset = POPUP_OFFSCREEN_Y;
        }
        else
        {
            // There's already a pop up window running.
            // Hurry the old pop up offscreen so the new one can appear.
            if (gTasks[gPopupTaskId].tState != STATE_SLIDE_OUT)
                gTasks[gPopupTaskId].tState = STATE_SLIDE_OUT;
            gTasks[gPopupTaskId].tIncomingPopUp = TRUE;
        }
    }
}

static void Task_MapNamePopUpWindow(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    switch (task->tState)
    {
    case STATE_PRINT:
        // Wait, then create and print the pop up window
        if (++task->tPrintTimer > 30)
        {
            task->tState = STATE_SLIDE_IN;
            task->tPrintTimer = 0;
            ShowMapNamePopUpWindow();
            if (OW_POPUP_GENERATION == GEN_5)
            {
                EnableInterrupts(INTR_FLAG_HBLANK);
                SetHBlankCallback(HBlankCB_DoublePopupWindow);
            }
        }
        break;
    case STATE_SLIDE_IN:
        // Slide the window onscreen.
        task->tYOffset -= POPUP_SLIDE_SPEED;
        if (task->tYOffset <= 0 )
        {
            task->tYOffset = 0;
            task->tState = STATE_WAIT;
            gTasks[gPopupTaskId].tOnscreenTimer = 0;
        }
        break;
    case STATE_WAIT:
        // Wait while the window is fully onscreen.
        if (++task->tOnscreenTimer > 120)
        {
            task->tOnscreenTimer = 0;
            task->tState = STATE_SLIDE_OUT;
        }
        break;
    case STATE_SLIDE_OUT:
        // Slide the window offscreen.
        task->tYOffset += POPUP_SLIDE_SPEED;
        if (task->tYOffset >= POPUP_OFFSCREEN_Y)
        {
            task->tYOffset = POPUP_OFFSCREEN_Y;
            if (task->tIncomingPopUp)
            {
                // A new pop up window is incoming,
                // return to the first state to show it.
                task->tState = STATE_PRINT;
                task->tPrintTimer = 0;
                task->tIncomingPopUp = FALSE;
            }
            else
            {
                task->tState = STATE_ERASE;
                return;
            }
        }
        break;
    case STATE_ERASE:
        ClearStdWindowAndFrame(GetMapNamePopUpWindowId(), TRUE);
        if (OW_POPUP_GENERATION == GEN_5)
            ClearStdWindowAndFrame(GetSecondaryPopUpWindowId(), TRUE);
        task->tState = STATE_END;
        break;
    case STATE_END:
        HideMapNamePopUpWindow();
        return;
    }
    if (OW_POPUP_GENERATION != GEN_5)
        SetGpuReg(REG_OFFSET_BG0VOFS, task->tYOffset);
}

void HideMapNamePopUpWindow(void)
{
    if (FuncIsActiveTask(Task_MapNamePopUpWindow))
    {
    #ifdef UBFIX
        if (GetMapNamePopUpWindowId() != WINDOW_NONE)
    #endif // UBFIX
        {
            ClearStdWindowAndFrame(GetMapNamePopUpWindowId(), TRUE);
            RemoveMapNamePopUpWindow();
        }

        if (OW_POPUP_GENERATION == GEN_5)
        {
            if (GetSecondaryPopUpWindowId() != WINDOW_NONE)
            {
                ClearStdWindowAndFrame(GetSecondaryPopUpWindowId(), TRUE);
                RemoveSecondaryPopUpWindow();
            }

            DisableInterrupts(INTR_FLAG_HBLANK);
            SetHBlankCallback(NULL);

            if (OW_POPUP_BW_ALPHA_BLEND && !IsWeatherAlphaBlend())
            {
                SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_BG_ALL | WININ_WIN0_OBJ | WININ_WIN1_BG_ALL | WININ_WIN1_OBJ);
                SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT2_BG1 | BLDCNT_TGT2_BG2 | BLDCNT_TGT2_BG3 | BLDCNT_TGT2_OBJ | BLDCNT_EFFECT_BLEND);
                SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(8, 10));
            }
        }

        SetGpuReg_ForcedBlank(REG_OFFSET_BG0VOFS, 0);
        DestroyTask(gPopupTaskId);
    }
}

static void ShowMapNamePopUpWindow(void)
{
    u8 mapDisplayHeader[24];
    u8 *withoutPrefixPtr;
    u8 x;
    const u8 *mapDisplayHeaderSource;
    u8 mapNamePopUpWindowId, secondaryPopUpWindowId;

    if (InBattlePyramid())
    {
        if (gMapHeader.mapLayoutId == LAYOUT_BATTLE_FRONTIER_BATTLE_PYRAMID_TOP)
        {
            withoutPrefixPtr = &(mapDisplayHeader[3]);
            mapDisplayHeaderSource = sBattlePyramid_MapHeaderStrings[FRONTIER_STAGES_PER_CHALLENGE];
        }
        else
        {
            withoutPrefixPtr = &(mapDisplayHeader[3]);
            mapDisplayHeaderSource = sBattlePyramid_MapHeaderStrings[gSaveBlock2Ptr->frontier.curChallengeBattleNum];
        }
        StringCopy(withoutPrefixPtr, mapDisplayHeaderSource);
    }
    else
    {
        withoutPrefixPtr = &(mapDisplayHeader[3]);
        GetMapName(withoutPrefixPtr, gMapHeader.regionMapSectionId, 0);
    }

    if (OW_POPUP_GENERATION == GEN_5)
    {
        if (OW_POPUP_BW_ALPHA_BLEND && !IsWeatherAlphaBlend())
            SetGpuRegBits(REG_OFFSET_WININ, WININ_WIN0_CLR);

        mapNamePopUpWindowId = AddMapNamePopUpWindow();
        secondaryPopUpWindowId = AddSecondaryPopUpWindow();
    }
    else
    {
        AddMapNamePopUpWindow();
    }

    LoadMapNamePopUpWindowBg();

    mapDisplayHeader[0] = EXT_CTRL_CODE_BEGIN;
    mapDisplayHeader[1] = EXT_CTRL_CODE_HIGHLIGHT;
    mapDisplayHeader[2] = TEXT_COLOR_TRANSPARENT;

    if (OW_POPUP_GENERATION == GEN_5)
    {
        AddTextPrinterParameterized(mapNamePopUpWindowId, FONT_SHORT, mapDisplayHeader, 8, 2, TEXT_SKIP_DRAW, NULL);

        if (OW_POPUP_BW_TIME_MODE != OW_POPUP_BW_TIME_NONE)
        {
            RtcCalcLocalTime();
            FormatDecimalTimeWithoutSeconds(withoutPrefixPtr, gLocalTime.hours, gLocalTime.minutes, OW_POPUP_BW_TIME_MODE == OW_POPUP_BW_TIME_24_HR);
            AddTextPrinterParameterized(secondaryPopUpWindowId, FONT_SMALL, mapDisplayHeader, GetStringRightAlignXOffset(FONT_SMALL, mapDisplayHeader, DISPLAY_WIDTH) - 5, 8, TEXT_SKIP_DRAW, NULL);
        }

        CopyWindowToVram(mapNamePopUpWindowId, COPYWIN_FULL);
        CopyWindowToVram(secondaryPopUpWindowId, COPYWIN_FULL);
    }
    else
    {
        x = GetStringCenterAlignXOffset(FONT_NARROW, withoutPrefixPtr, 80);
        AddTextPrinterParameterized(GetMapNamePopUpWindowId(), FONT_NARROW, mapDisplayHeader, x, 3, TEXT_SKIP_DRAW, NULL);
        CopyWindowToVram(GetMapNamePopUpWindowId(), COPYWIN_FULL);
    }
}

#define TILE_TOP_EDGE_START 0x21D
#define TILE_TOP_EDGE_END   0x228
#define TILE_LEFT_EDGE_TOP  0x229
#define TILE_RIGHT_EDGE_TOP 0x22A
#define TILE_LEFT_EDGE_MID  0x22B
#define TILE_RIGHT_EDGE_MID 0x22C
#define TILE_LEFT_EDGE_BOT  0x22D
#define TILE_RIGHT_EDGE_BOT 0x22E
#define TILE_BOT_EDGE_START 0x22F
#define TILE_BOT_EDGE_END   0x23A

static void LoadMapNamePopUpWindowBg(void)
{
    u8 popUpThemeId;
    u8 popupWindowId = GetMapNamePopUpWindowId();
    u16 regionMapSectionId = gMapHeader.regionMapSectionId;
    u8 secondaryPopUpWindowId;

    if (OW_POPUP_GENERATION == GEN_5)
        secondaryPopUpWindowId = GetSecondaryPopUpWindowId();

    if (regionMapSectionId >= KANTO_MAPSEC_START)
    {
        if (regionMapSectionId > KANTO_MAPSEC_END)
            regionMapSectionId -= KANTO_MAPSEC_COUNT;
        else
            regionMapSectionId = 0; // Discard kanto region sections;
    }

    if (OW_POPUP_GENERATION == GEN_5)
    {
        popUpThemeId = MAPPOPUP_THEME_BW_DEFAULT;
        switch (popUpThemeId) 
        {
            // add additional gen 5-style pop-up themes as cases here
            default: // MAPPOPUP_THEME_BW_DEFAULT
                if (OW_POPUP_BW_COLOR == OW_POPUP_BW_COLOR_WHITE)
                    LoadPalette(sMapPopUpTilesPalette_BW_White, BG_PLTT_ID(14), sizeof(sMapPopUpTilesPalette_BW_White));
                else
                    LoadPalette(sMapPopUpTilesPalette_BW_Black, BG_PLTT_ID(14), sizeof(sMapPopUpTilesPalette_BW_Black));

                CopyToWindowPixelBuffer(popupWindowId, sMapPopUpTilesPrimary_BW, sizeof(sMapPopUpTilesPrimary_BW), 0);
                CopyToWindowPixelBuffer(secondaryPopUpWindowId, sMapPopUpTilesSecondary_BW, sizeof(sMapPopUpTilesSecondary_BW), 0);
                break;
        }

        PutWindowTilemap(popupWindowId);
        PutWindowTilemap(secondaryPopUpWindowId);
    }
}
