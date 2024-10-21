#include "global.h"
#include "m4a.h"
#include "malloc.h"
#include "reload_save.h"
#include "save.h"
#include "bg.h"
#include "window.h"
#include "random.h"
#include "decompress.h"
#include "string_util.h"
#include "event_data.h"
#include "item_menu.h"
#include "overworld.h"
#include "gpu_regs.h"
#include "palette.h"
#include "task.h"
#include "scanline_effect.h"
#include "menu.h"
#include "text.h"
#include "strings.h"
#include "constants/songs.h"
#include "sound.h"
#include "trade.h"
#include "battle.h"
#include "link.h"
#include "constants/rgb.h"
#include "constants/trade.h"

// Window IDs for the link error screens
enum {
    WIN_LINK_ERROR_TOP,
    WIN_LINK_ERROR_MID,
    WIN_LINK_ERROR_BOTTOM,
};

struct BlockTransfer
{
    u16 pos;
    u16 size;
    const u8 *src;
    bool8 active;
    u8 multiplayerId;
};

struct LinkTestBGInfo
{
    u32 screenBaseBlock;
    u32 paletteNum;
    u32 baseChar;
    u32 unused;
};

static struct BlockTransfer sBlockSend;
static u32 sBlockSendDelayCounter;
static u32 sPlayerDataExchangeStatus;
static u8 sNumVBlanksWithoutSerialIntr;
static bool8 sSendBufferEmpty;
static u16 sSendNonzeroCheck;
static u16 sRecvNonzeroCheck;
static u8 sChecksumAvailable;
static u8 sHandshakePlayerCount;

COMMON_DATA u16 gLinkPartnersHeldKeys[6] = {0};
COMMON_DATA u32 gLinkDebugSeed = 0;
COMMON_DATA struct LinkPlayerBlock gLocalLinkPlayerBlock = {0};
COMMON_DATA bool8 gLinkErrorOccurred = 0;
COMMON_DATA u32 gLinkDebugFlags = 0;
COMMON_DATA bool8 gRemoteLinkPlayersNotReceived[MAX_LINK_PLAYERS] = {0};
COMMON_DATA u8 gBlockReceivedStatus[MAX_LINK_PLAYERS] = {0};
COMMON_DATA u16 gLinkHeldKeys = 0;
COMMON_DATA u16 ALIGNED(4) gRecvCmds[MAX_RFU_PLAYERS][CMD_LENGTH] = {0};
COMMON_DATA u32 gLinkStatus = 0;
COMMON_DATA bool8 gReadyToExitStandby[MAX_LINK_PLAYERS] = {0};
COMMON_DATA bool8 gReadyToCloseLink[MAX_LINK_PLAYERS] = {0};
COMMON_DATA u8 gSuppressLinkErrorMessage = 0;
COMMON_DATA bool8 gSavedLinkPlayerCount = 0;
COMMON_DATA u16 gSendCmd[CMD_LENGTH] = {0};
COMMON_DATA u8 gSavedMultiplayerId = 0;
COMMON_DATA bool8 gReceivedRemoteLinkPlayers = 0;
COMMON_DATA struct LinkTestBGInfo gLinkTestBGInfo = {0};
COMMON_DATA void (*gLinkCallback)(void) = NULL;
COMMON_DATA u8 gShouldAdvanceLinkState = 0;
COMMON_DATA u16 gLinkTestBlockChecksums[MAX_LINK_PLAYERS] = {0};
COMMON_DATA u8 gBlockRequestType = 0;
COMMON_DATA u8 gLastSendQueueCount = 0;
COMMON_DATA struct Link gLink = {0};
COMMON_DATA u8 gLastRecvQueueCount = 0;
COMMON_DATA u16 gLinkSavedIme = 0;

EWRAM_DATA u8 gBlockSendBuffer[BLOCK_BUFFER_SIZE] = {};
static EWRAM_DATA bool8 sLinkOpen = FALSE;
EWRAM_DATA u16 gLinkType = 0;
EWRAM_DATA struct LinkPlayer gLocalLinkPlayer = {};
EWRAM_DATA struct LinkPlayer gLinkPlayers[MAX_RFU_PLAYERS] = {};
static EWRAM_DATA struct LinkPlayer sSavedLinkPlayers[MAX_RFU_PLAYERS] = {};
static EWRAM_DATA struct {
    u32 status;
    u8 lastRecvQueueCount;
    u8 lastSendQueueCount;
    bool8 disconnected;
} sLinkErrorBuffer = {};

static void LinkCB_SendHeldKeys(void);
static void ResetBlockSend(void);
static bool32 InitBlockSend(const void *, size_t);
static void LinkCB_BlockSendBegin(void);
static void LinkCB_BlockSend(void);
static void LinkCB_BlockSendEnd(void);
static void LinkCB_RequestPlayerDataExchange(void);

static void LinkCB_ReadyCloseLink(void);
static void LinkCB_WaitCloseLink(void);
static void LinkCB_ReadyCloseLinkWithJP(void);
static void LinkCB_WaitCloseLinkWithJP(void);
static void LinkCB_Standby(void);
static void LinkCB_StandbyForAll(void);

static void TrySetLinkErrorBuffer(void);
static void DisableSerial(void);
static void EnableSerial(void);
static void CheckMasterOrSlave(void);
static void InitTimer(void);
static void EnqueueSendCmd(u16 *sendCmd);
static void DequeueRecvCmds(u16 (*recvCmds)[CMD_LENGTH]);

static void StartTransfer(void);
static bool8 DoHandshake(void);
static void DoRecv(void);
static void DoSend(void);
static void StopTimer(void);
static void SendRecvDone(void);

static const u16 sWirelessLinkDisplayPal[] = INCBIN_U16("graphics/link/wireless_display.gbapal");
static const u32 sWirelessLinkDisplayGfx[] = INCBIN_U32("graphics/link/wireless_display.4bpp.lz");
static const u32 sWirelessLinkDisplayTilemap[] = INCBIN_U32("graphics/link/wireless_display.bin.lz");
static const u16 sLinkTestDigitsPal[] = INCBIN_U16("graphics/link/test_digits.gbapal");
static const u16 sLinkTestDigitsGfx[] = INCBIN_U16("graphics/link/test_digits.4bpp");
static const u16 sCommErrorBg_Gfx[] = INCBIN_U16("graphics/link/comm_error_bg.4bpp");
static const struct BlockRequest sBlockRequests[] = {
    [BLOCK_REQ_SIZE_NONE] = {gBlockSendBuffer, 200},
    [BLOCK_REQ_SIZE_200]  = {gBlockSendBuffer, 200},
    [BLOCK_REQ_SIZE_100]  = {gBlockSendBuffer, 100},
    [BLOCK_REQ_SIZE_220]  = {gBlockSendBuffer, 220},
    [BLOCK_REQ_SIZE_40]   = {gBlockSendBuffer,  40}
};
static const u8 sBGControlRegs[] = {
    REG_OFFSET_BG0CNT,
    REG_OFFSET_BG1CNT,
    REG_OFFSET_BG2CNT,
    REG_OFFSET_BG3CNT
};
static const char sASCIIGameFreakInc[] = "GameFreak inc.";
static const char sASCIITestPrint[] = "TEST PRINT\nP0\nP1\nP2\nP3";
static const struct BgTemplate sLinkErrorBgTemplates[] = {
    {
        .bg = 0,
        .charBaseIndex = 2,
        .mapBaseIndex = 31,
        .priority = 0
    }, {
        .bg = 1,
        .charBaseIndex = 0,
        .mapBaseIndex = 8,
        .priority = 1
    }
};

static const struct WindowTemplate sLinkErrorWindowTemplates[] = {
    [WIN_LINK_ERROR_TOP] = {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = DISPLAY_TILE_WIDTH,
        .height = 5,
        .paletteNum = 15,
        .baseBlock = 0x002
    },
    [WIN_LINK_ERROR_MID] = {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 6,
        .width = DISPLAY_TILE_WIDTH,
        .height = 7,
        .paletteNum = 15,
        .baseBlock = 0x098
    },
    [WIN_LINK_ERROR_BOTTOM] = {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 13,
        .width = DISPLAY_TILE_WIDTH,
        .height = 7,
        .paletteNum = 15,
        .baseBlock = 0x16A
    }, DUMMY_WIN_TEMPLATE
};

static const u8 sTextColors[] = { TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY };

void Task_DestroySelf(u8 taskId)
{
    DestroyTask(taskId);
}

void SetLocalLinkPlayerId(u8 playerId)
{
    gLocalLinkPlayer.id = playerId;
}

static void InitLink(void)
{
    int i;

    for (i = 0; i < CMD_LENGTH; i++)
        gSendCmd[i] = LINKCMD_NONE;

    sLinkOpen = TRUE;
    EnableSerial();
}

static void Task_TriggerHandshake(u8 taskId)
{
    if (++gTasks[taskId].data[0] == 5)
    {
        gShouldAdvanceLinkState = 1;
        DestroyTask(taskId);
    }
}

void OpenLink(void)
{
    int i;

    ResetSerial();
    InitLink();
    gLinkCallback = LinkCB_RequestPlayerDataExchange;
    gLinkVSyncDisabled = FALSE;
    gLinkErrorOccurred = FALSE;
    gSuppressLinkErrorMessage = FALSE;
    ResetBlockSend();
    CreateTask(Task_TriggerHandshake, 2);
    gReceivedRemoteLinkPlayers = 0;
    for (i = 0; i < MAX_LINK_PLAYERS; i++)
    {
        gRemoteLinkPlayersNotReceived[i] = TRUE;
        gReadyToCloseLink[i] = FALSE;
        gReadyToExitStandby[i] = FALSE;
    }
}

void CloseLink(void)
{
    gReceivedRemoteLinkPlayers = FALSE;
    sLinkOpen = FALSE;
    DisableSerial();
}

u16 LinkMain2(const u16 *heldKeys)
{
    u8 i;

    if (!sLinkOpen)
        return 0;

    for (i = 0; i < CMD_LENGTH; i++)
        gSendCmd[i] = 0;

    gLinkHeldKeys = *heldKeys;
    if (gLinkStatus & LINK_STAT_CONN_ESTABLISHED)
    {
        if (gLinkCallback != NULL)
            gLinkCallback();
        TrySetLinkErrorBuffer();
    }
    return gLinkStatus;
}

void StartSendingKeysToLink(void)
{
    gLinkCallback = LinkCB_SendHeldKeys;
}

bool32 IsSendingKeysToLink(void)
{
    if (gLinkCallback == LinkCB_SendHeldKeys)
        return TRUE;

    return FALSE;
}

static void LinkCB_SendHeldKeys(void)
{

}

void ClearLinkCallback(void)
{
    gLinkCallback = NULL;
}

void ClearLinkCallback_2(void)
{
    gLinkCallback = NULL;
}

u8 GetLinkPlayerCount(void)
{
    return EXTRACT_PLAYER_COUNT(gLinkStatus);
}

static int AreAnyLinkPlayersUsingVersions(u32 version1, u32 version2)
{
    int i;
    u8 nPlayers;

    nPlayers = GetLinkPlayerCount();
    for (i = 0; i < nPlayers; i++)
    {
        if ((gLinkPlayers[i].version & 0xFF) == version1
         || (gLinkPlayers[i].version & 0xFF) == version2)
            return 1;
    }
    return -1;
}

bool32 Link_AnyPartnersPlayingRubyOrSapphire(void)
{
    if (AreAnyLinkPlayersUsingVersions(VERSION_RUBY, VERSION_SAPPHIRE) >= 0)
    {
        return TRUE;
    }
    return FALSE;
}

void OpenLinkTimed(void)
{
    sPlayerDataExchangeStatus = EXCHANGE_NOT_STARTED;
    OpenLink();
}

u8 GetLinkPlayerDataExchangeStatusTimed(int minPlayers, int maxPlayers)
{
    int i;
    int count;
    u32 index;
    u8 numPlayers;
    u32 linkType1;
    u32 linkType2;

    count = 0;
    if (gReceivedRemoteLinkPlayers == TRUE)
    {
        numPlayers = GetLinkPlayerCount_2();
        if (minPlayers > numPlayers || numPlayers > maxPlayers)
        {
            sPlayerDataExchangeStatus = EXCHANGE_WRONG_NUM_PLAYERS;
            return sPlayerDataExchangeStatus;
        }
        else
        {
            if (GetLinkPlayerCount() == 0)
            {
                gLinkErrorOccurred = TRUE;
                CloseLink();
            }
            for (i = 0, index = 0; i < GetLinkPlayerCount(); index++, i++)
            {
                if (gLinkPlayers[index].linkType == gLinkPlayers[0].linkType)
                {
                    count++;
                }
            }
            if (count == GetLinkPlayerCount())
            {
                if (gLinkPlayers[0].linkType == LINKTYPE_TRADE_SETUP)
                {
                    switch (GetGameProgressForLinkTrade())
                    {
                    case TRADE_PLAYER_NOT_READY:
                        sPlayerDataExchangeStatus = EXCHANGE_PLAYER_NOT_READY;
                        break;
                    case TRADE_PARTNER_NOT_READY:
                        sPlayerDataExchangeStatus = EXCHANGE_PARTNER_NOT_READY;
                        break;
                    case TRADE_BOTH_PLAYERS_READY:
                        sPlayerDataExchangeStatus = EXCHANGE_COMPLETE;
                        break;
                    }
                }
                else
                {
                    sPlayerDataExchangeStatus = EXCHANGE_COMPLETE;
                }
            }
            else
            {
                sPlayerDataExchangeStatus = EXCHANGE_DIFF_SELECTIONS;
                linkType1 = gLinkPlayers[GetMultiplayerId()].linkType;
                linkType2 = gLinkPlayers[GetMultiplayerId() ^ 1].linkType;
                if ((linkType1 == LINKTYPE_BATTLE_TOWER_50 && linkType2 == LINKTYPE_BATTLE_TOWER_OPEN)
                 || (linkType1 == LINKTYPE_BATTLE_TOWER_OPEN && linkType2 == LINKTYPE_BATTLE_TOWER_50))
                {
                    // 3 below indicates partner made different level mode selection
                    // See BattleFrontier_BattleTowerLobby_EventScript_AbortLinkDifferentSelections
                    gSpecialVar_0x8005 = 3;
                }
            }
        }
    }
    return sPlayerDataExchangeStatus;
}

bool8 IsLinkPlayerDataExchangeComplete(void)
{
    u8 i;
    u8 count;
    bool8 retval;

    count = 0;
    for (i = 0; i < GetLinkPlayerCount(); i++)
    {
        if (gLinkPlayers[i].linkType == gLinkPlayers[0].linkType)
            count++;
    }
    if (count == GetLinkPlayerCount())
    {
        retval = TRUE;
        sPlayerDataExchangeStatus = EXCHANGE_COMPLETE;
    }
    else
    {
        retval = FALSE;
        sPlayerDataExchangeStatus = EXCHANGE_DIFF_SELECTIONS;
    }
    return retval;
}

u32 GetLinkPlayerTrainerId(u8 who)
{
    return gLinkPlayers[who].trainerId;
}

void ResetLinkPlayers(void)
{
    int i;

    for (i = 0; i <= MAX_LINK_PLAYERS; i++)
        gLinkPlayers[i] = (struct LinkPlayer){};
}

static void ResetBlockSend(void)
{
    sBlockSend.active = FALSE;
    sBlockSend.pos = 0;
    sBlockSend.size = 0;
    sBlockSend.src = NULL;
}

static bool32 InitBlockSend(const void *src, size_t size)
{
    if (sBlockSend.active)
    {
        return FALSE;
    }
    sBlockSend.multiplayerId = GetMultiplayerId();
    sBlockSend.active = TRUE;
    sBlockSend.size = size;
    sBlockSend.pos = 0;
    if (size > BLOCK_BUFFER_SIZE)
    {
        sBlockSend.src = src;
    }
    else
    {
        if (src != gBlockSendBuffer)
            memcpy(gBlockSendBuffer, src, size);

        sBlockSend.src = gBlockSendBuffer;
    }
    gLinkCallback = LinkCB_BlockSendBegin;
    sBlockSendDelayCounter = 0;
    return TRUE;
}

static void LinkCB_BlockSendBegin(void)
{
    if (++sBlockSendDelayCounter > 2)
        gLinkCallback = LinkCB_BlockSend;
}

static void LinkCB_BlockSend(void)
{
    int i;
    const u8 *src;

    src = sBlockSend.src;
    gSendCmd[0] = LINKCMD_CONT_BLOCK;
    for (i = 0; i < CMD_LENGTH - 1; i++)
    {
        gSendCmd[i + 1] = (src[sBlockSend.pos + i * 2 + 1] << 8) | src[sBlockSend.pos + i * 2];
    }
    sBlockSend.pos += 14;
    if (sBlockSend.size <= sBlockSend.pos)
    {
        sBlockSend.active = FALSE;
        gLinkCallback = LinkCB_BlockSendEnd;
    }
}

static void LinkCB_BlockSendEnd(void)
{
    gLinkCallback = NULL;
}

u8 GetMultiplayerId(void)
{
    return SIO_MULTI_CNT->id;
}

u8 BitmaskAllOtherLinkPlayers(void)
{
    u8 mpId;

    mpId = GetMultiplayerId();
    return ((1 << MAX_LINK_PLAYERS) - 1) ^ (1 << mpId);
}

bool8 SendBlock(u8 unused, const void *src, u16 size)
{
    return InitBlockSend(src, size);
}

bool8 SendBlockRequest(u8 blockReqType)
{
    if (gLinkCallback == NULL)
    {
        gBlockRequestType = blockReqType;
        return TRUE;
    }
    return FALSE;
}

bool8 IsLinkTaskFinished(void)
{
    return gLinkCallback == NULL;
}

u8 GetBlockReceivedStatus(void)
{
    return (gBlockReceivedStatus[3] << 3) | (gBlockReceivedStatus[2] << 2) | (gBlockReceivedStatus[1] << 1) | (gBlockReceivedStatus[0] << 0);
}

void CheckShouldAdvanceLinkState(void)
{
    if ((gLinkStatus & LINK_STAT_MASTER) && EXTRACT_PLAYER_COUNT(gLinkStatus) > 1)
        gShouldAdvanceLinkState = 1;
}

static void LinkCB_RequestPlayerDataExchange(void)
{
    gLinkCallback = NULL;
}

void SetLinkDebugValues(u32 seed, u32 flags)
{
    gLinkDebugSeed = seed;
    gLinkDebugFlags = flags;
}

u8 GetSavedLinkPlayerCountAsBitFlags(void)
{
    int i;
    u8 flags;

    flags = 0;
    for (i = 0; i < gSavedLinkPlayerCount; i++)
        flags |= (1 << i);

    return flags;
}

u8 GetLinkPlayerCountAsBitFlags(void)
{
    int i;
    u8 flags;

    flags = 0;
    for (i = 0; i < GetLinkPlayerCount(); i++)
        flags |= (1 << i);

    return flags;
}

void SaveLinkPlayers(u8 playerCount)
{
    int i;

    gSavedLinkPlayerCount = playerCount;
    gSavedMultiplayerId = GetMultiplayerId();
    for (i = 0; i < MAX_RFU_PLAYERS; i++)
        sSavedLinkPlayers[i] = gLinkPlayers[i];
}

// The number of players when trading began. This is frequently compared against the
// current number of connected players to check if anyone dropped out.
u8 GetSavedPlayerCount(void)
{
    return gSavedLinkPlayerCount;
}

bool8 DoesLinkPlayerCountMatchSaved(void)
{
    int i;
    u32 count = 0;

    for (i = 0; i < gSavedLinkPlayerCount; i++)
    {
        if (gLinkPlayers[i].trainerId == sSavedLinkPlayers[i].trainerId)
        {
            if (gLinkType == LINKTYPE_BATTLE_TOWER)
            {
                if (gLinkType == gLinkPlayers[i].linkType)
                    count++;
            }
            else
            {
                count++;
            }
        }
    }
    if (count == gSavedLinkPlayerCount)
    {
        if (GetLinkPlayerCount_2() == gSavedLinkPlayerCount)
        {
            return TRUE;
        }
    }
    return FALSE;
}

void ClearSavedLinkPlayers(void)
{
    // The CpuSet loop below is incorrectly writing to NULL
    // instead of sSavedLinkPlayers.
    // Additionally it's using the wrong array size.
#ifdef UBFIX
    memset(sSavedLinkPlayers, 0, sizeof(sSavedLinkPlayers));
#else
    int i;
    for (i = 0; i < MAX_LINK_PLAYERS; i++)
        CpuSet(&sSavedLinkPlayers[i], NULL, sizeof(struct LinkPlayer));
#endif
}

void CheckLinkPlayersMatchSaved(void)
{
    u8 i;

    for (i = 0; i < gSavedLinkPlayerCount; i++)
    {
        if (sSavedLinkPlayers[i].trainerId != gLinkPlayers[i].trainerId
         || StringCompare(sSavedLinkPlayers[i].name, gLinkPlayers[i].name) != 0)
        {
            gLinkErrorOccurred = TRUE;
            CloseLink();
        }
    }
}

void ResetLinkPlayerCount(void)
{
    gSavedLinkPlayerCount = 0;
    gSavedMultiplayerId = 0;
}

u8 GetLinkPlayerCount_2(void)
{
    return EXTRACT_PLAYER_COUNT(gLinkStatus);
}

bool8 IsLinkMaster(void)
{
    return EXTRACT_MASTER(gLinkStatus);
}

void SetCloseLinkCallbackAndType(u16 type)
{
    if (gLinkCallback == NULL)
    {
        gLinkCallback = LinkCB_ReadyCloseLink;
    }
}

void SetCloseLinkCallback(void)
{
    gLinkCallback = LinkCB_ReadyCloseLink;
}

static void LinkCB_ReadyCloseLink(void)
{
    if (gLastRecvQueueCount == 0)
    {
        gLinkCallback = LinkCB_WaitCloseLink;
    }
}

static void LinkCB_WaitCloseLink(void)
{
    int i;
    unsigned count;

    // Wait for all players to be ready
    u8 linkPlayerCount = GetLinkPlayerCount();
    count = 0;
    for (i = 0; i < linkPlayerCount; i++)
    {
        if (gReadyToCloseLink[i])
            count++;
    }

    if (count == linkPlayerCount)
    {
        // All ready, close link
        gBattleTypeFlags &= ~BATTLE_TYPE_LINK_IN_BATTLE;
        gLinkVSyncDisabled = TRUE;
        CloseLink();
        gLinkCallback = NULL;
    }
}

// Used instead of SetCloseLinkCallback when disconnecting from an attempt to link with a foreign game
void SetCloseLinkCallbackHandleJP(void)
{
    gLinkCallback = LinkCB_ReadyCloseLinkWithJP;
}

static void LinkCB_ReadyCloseLinkWithJP(void)
{
    if (gLastRecvQueueCount == 0)
    {
        gLinkCallback = LinkCB_WaitCloseLinkWithJP;
    }
}

static void LinkCB_WaitCloseLinkWithJP(void)
{
    int i;
    unsigned count;
    u8 linkPlayerCount;

    linkPlayerCount = GetLinkPlayerCount();
    count = 0;

    // Wait for all non-foreign players to be ready
    for (i = 0; i < linkPlayerCount; i++)
    {
        // Rather than communicate with the foreign game
        // just assume they're ready to disconnect
        if (gLinkPlayers[i].language == LANGUAGE_JAPANESE)
            count++;
        else if (gReadyToCloseLink[i])
            count++;
    }

    if (count == linkPlayerCount)
    {
        // All ready, close link
        gBattleTypeFlags &= ~BATTLE_TYPE_LINK_IN_BATTLE;
        gLinkVSyncDisabled = TRUE;
        CloseLink();
        gLinkCallback = NULL;
    }
}

void SetLinkStandbyCallback(void)
{
    if (gLinkCallback == NULL)
        gLinkCallback = LinkCB_Standby;
}

static void LinkCB_Standby(void)
{
    if (gLastRecvQueueCount == 0)
    {
        gLinkCallback = LinkCB_StandbyForAll;
    }
}

static void LinkCB_StandbyForAll(void)
{
    u8 i;
    u8 linkPlayerCount = GetLinkPlayerCount();
    for (i = 0; i < linkPlayerCount; i++)
    {
        if (!gReadyToExitStandby[i])
            break;
    }

    // If true, all players ready to exit standby
    if (i == linkPlayerCount)
    {
        for (i = 0; i < MAX_LINK_PLAYERS; i++)
            gReadyToExitStandby[i] = FALSE;

        gLinkCallback = NULL;
    }
}

static void TrySetLinkErrorBuffer(void)
{
    // Check if a link error has occurred
    if (sLinkOpen && EXTRACT_LINK_ERRORS(gLinkStatus))
    {
        // Link error has occurred, handle message details if
        // necessary, then stop the link.
        if (!gSuppressLinkErrorMessage)
        {
            sLinkErrorBuffer.status = gLinkStatus;
            sLinkErrorBuffer.lastRecvQueueCount = gLastRecvQueueCount;
            sLinkErrorBuffer.lastSendQueueCount = gLastSendQueueCount;
        }
        gLinkErrorOccurred = TRUE;
        CloseLink();
    }
}

void SetLinkErrorBuffer(u32 status, u8 lastSendQueueCount, u8 lastRecvQueueCount, bool8 disconnected)
{
    sLinkErrorBuffer.status = status;
    sLinkErrorBuffer.lastSendQueueCount = lastSendQueueCount;
    sLinkErrorBuffer.lastRecvQueueCount = lastRecvQueueCount;
    sLinkErrorBuffer.disconnected = disconnected;
}

bool8 GetSioMultiSI(void)
{
    return (REG_SIOCNT & SIO_MULTI_SI) != 0;
}

bool8 IsLinkConnectionEstablished(void)
{
    return EXTRACT_CONN_ESTABLISHED(gLinkStatus);
}

void SetSuppressLinkErrorMessage(bool8 flag)
{
    gSuppressLinkErrorMessage = flag;
}

bool8 HasLinkErrorOccurred(void)
{
    return gLinkErrorOccurred;
}

void LocalLinkPlayerToBlock(void)
{
    struct LinkPlayerBlock *block;

    block = &gLocalLinkPlayerBlock;
    block->linkPlayer = gLocalLinkPlayer;
    memcpy(block->magic1, sASCIIGameFreakInc, sizeof(block->magic1) - 1);
    memcpy(block->magic2, sASCIIGameFreakInc, sizeof(block->magic2) - 1);
    memcpy(gBlockSendBuffer, block, sizeof(*block));
}

void LinkPlayerFromBlock(u32 who)
{

}

u32 GetLinkRecvQueueLength(void)
{
    return gLink.recvQueue.count;
}

bool32 IsLinkRecvQueueAtOverworldMax(void)
{
    if (GetLinkRecvQueueLength() >= OVERWORLD_RECV_QUEUE_MAX)
        return TRUE;

    return FALSE;
}

void ConvertLinkPlayerName(struct LinkPlayer *player)
{
    player->progressFlagsCopy = player->progressFlags; // ? Perhaps relocating for a longer name field
    ConvertInternationalString(player->name, player->language);
}

static void DisableSerial(void)
{
    DisableInterrupts(INTR_FLAG_TIMER3 | INTR_FLAG_SERIAL);
    REG_SIOCNT = SIO_MULTI_MODE;
    REG_TMCNT_H(3) = 0;
    REG_IF = INTR_FLAG_TIMER3 | INTR_FLAG_SERIAL;
    REG_SIOMLT_SEND = 0;
    REG_SIOMLT_RECV = 0;
    CpuFill32(0, &gLink, sizeof(gLink));
}

static void EnableSerial(void)
{
    DisableInterrupts(INTR_FLAG_TIMER3 | INTR_FLAG_SERIAL);
    REG_RCNT = 0;
    REG_SIOCNT = SIO_MULTI_MODE;
    REG_SIOCNT |= SIO_115200_BPS | SIO_INTR_ENABLE;
    EnableInterrupts(INTR_FLAG_SERIAL);
    REG_SIOMLT_SEND = 0;
    CpuFill32(0, &gLink, sizeof(gLink));
    sNumVBlanksWithoutSerialIntr = 0;
    sSendNonzeroCheck = 0;
    sRecvNonzeroCheck = 0;
    sChecksumAvailable = 0;
    sHandshakePlayerCount = 0;
    gLastSendQueueCount = 0;
    gLastRecvQueueCount = 0;
}

void ResetSerial(void)
{
    EnableSerial();
    DisableSerial();
}

// link_main1.c

u32 LinkMain1(u8 *shouldAdvanceLinkState, u16 *sendCmd, u16 (*recvCmds)[CMD_LENGTH])
{
    u32 retVal;
    u32 retVal2;

    switch (gLink.state)
    {
        case LINK_STATE_START0:
            DisableSerial();
            gLink.state = 1;
            break;
        case LINK_STATE_START1:
            if (*shouldAdvanceLinkState == 1)
            {
                EnableSerial();
                gLink.state = 2;
            }
            break;
        case LINK_STATE_HANDSHAKE:
            switch (*shouldAdvanceLinkState)
            {
                default:
                    CheckMasterOrSlave();
                    break;
                case 1:
                    if (gLink.isMaster == LINK_MASTER && gLink.playerCount > 1)
                        gLink.handshakeAsMaster = TRUE;
                    break;
                case 2:
                    gLink.state = LINK_STATE_START0;
                    REG_SIOMLT_SEND = 0;
                    break;
            }
            break;
        case LINK_STATE_INIT_TIMER:
            InitTimer();
            gLink.state = LINK_STATE_CONN_ESTABLISHED;
            // fallthrough
        case LINK_STATE_CONN_ESTABLISHED:
            EnqueueSendCmd(sendCmd);
            DequeueRecvCmds(recvCmds);
            break;
    }
    *shouldAdvanceLinkState = 0;
    retVal = gLink.localId;
    retVal |= (gLink.playerCount << LINK_STAT_PLAYER_COUNT_SHIFT);
    if (gLink.isMaster == LINK_MASTER)
    {
        retVal |= LINK_STAT_MASTER;
    }
    {
        u32 receivedNothing = gLink.receivedNothing << LINK_STAT_RECEIVED_NOTHING_SHIFT;
        u32 link_field_F = gLink.link_field_F << LINK_STAT_UNK_FLAG_9_SHIFT;
        u32 hardwareError = gLink.hardwareError << LINK_STAT_ERROR_HARDWARE_SHIFT;
        u32 badChecksum = gLink.badChecksum << LINK_STAT_ERROR_CHECKSUM_SHIFT;
        u32 queueFull = gLink.queueFull << LINK_STAT_ERROR_QUEUE_FULL_SHIFT;
        u32 val;

        if (gLink.state == LINK_STATE_CONN_ESTABLISHED)
        {
            val = LINK_STAT_CONN_ESTABLISHED;
            val |= receivedNothing;
            val |= retVal;
            val |= link_field_F;
            val |= hardwareError;
            val |= badChecksum;
            val |= queueFull;
        }
        else
        {
            val = retVal;
            val |= receivedNothing;
            val |= link_field_F;
            val |= hardwareError;
            val |= badChecksum;
            val |= queueFull;
        }

        retVal = val;
    }

    if (gLink.lag == LAG_MASTER)
        retVal |= LINK_STAT_ERROR_LAG_MASTER;

    if (gLink.localId >= MAX_LINK_PLAYERS)
        retVal |= LINK_STAT_ERROR_INVALID_ID;

    retVal2 = retVal;
    if (gLink.lag == LAG_SLAVE)
        retVal2 |= LINK_STAT_ERROR_LAG_SLAVE;

    return retVal2;
}

static void CheckMasterOrSlave(void)
{
    u32 terminals;

    terminals = *(vu32 *)REG_ADDR_SIOCNT & (SIO_MULTI_SD | SIO_MULTI_SI);
    if (terminals == SIO_MULTI_SD && gLink.localId == 0)
    {
        gLink.isMaster = LINK_MASTER;
    }
    else
    {
        gLink.isMaster = LINK_SLAVE;
    }
}

static void InitTimer(void)
{
    if (gLink.isMaster)
    {
        REG_TM3CNT_L = -197;
        REG_TM3CNT_H = TIMER_64CLK | TIMER_INTR_ENABLE;
        EnableInterrupts(INTR_FLAG_TIMER3);
    }
}

static void EnqueueSendCmd(u16 *sendCmd)
{
    u8 i;
    u8 offset;

    gLinkSavedIme = REG_IME;
    REG_IME = 0;
    if (gLink.sendQueue.count < QUEUE_CAPACITY)
    {
        offset = gLink.sendQueue.pos + gLink.sendQueue.count;
        if (offset >= QUEUE_CAPACITY)
        {
            offset -= QUEUE_CAPACITY;
        }
        for (i = 0; i < CMD_LENGTH; i++)
        {
            sSendNonzeroCheck |= *sendCmd;
            gLink.sendQueue.data[i][offset] = *sendCmd;
            *sendCmd = 0;
            sendCmd++;
        }
    }
    else
    {
        gLink.queueFull = QUEUE_FULL_SEND;
    }
    if (sSendNonzeroCheck)
    {
        gLink.sendQueue.count++;
        sSendNonzeroCheck = 0;
    }
    REG_IME = gLinkSavedIme;
    gLastSendQueueCount = gLink.sendQueue.count;
}


static void DequeueRecvCmds(u16 (*recvCmds)[CMD_LENGTH])
{
    u8 i;
    u8 j;

    gLinkSavedIme = REG_IME;
    REG_IME = 0;
    if (gLink.recvQueue.count == 0)
    {
        for (i = 0; i < gLink.playerCount; i++)
        {
            for (j = 0; j < CMD_LENGTH; j++)
            {
                recvCmds[i][j] = 0;
            }
        }

        gLink.receivedNothing = TRUE;
    }
    else
    {
        for (i = 0; i < gLink.playerCount; i++)
        {
            for (j = 0; j < CMD_LENGTH; j++)
            {
                recvCmds[i][j] = gLink.recvQueue.data[i][j][gLink.recvQueue.pos];
            }
        }
        gLink.recvQueue.count--;
        gLink.recvQueue.pos++;
        if (gLink.recvQueue.pos >= QUEUE_CAPACITY)
        {
            gLink.recvQueue.pos = 0;
        }
        gLink.receivedNothing = FALSE;
    }
    REG_IME = gLinkSavedIme;
}

// link_intr.c

void LinkVSync(void)
{
    if (gLink.isMaster)
    {
        switch (gLink.state)
        {
            case LINK_STATE_CONN_ESTABLISHED:
                if (gLink.serialIntrCounter < 9)
                {
                    if (gLink.hardwareError != TRUE)
                    {
                        gLink.lag = LAG_MASTER;
                    }
                    else
                    {
                        StartTransfer();
                    }
                }
                else if (gLink.lag != LAG_MASTER)
                {
                    gLink.serialIntrCounter = 0;
                    StartTransfer();
                }
                break;
            case LINK_STATE_HANDSHAKE:
                StartTransfer();
                break;
        }
    }
    else if (gLink.state == LINK_STATE_CONN_ESTABLISHED || gLink.state == LINK_STATE_HANDSHAKE)
    {
        if (++sNumVBlanksWithoutSerialIntr > 10)
        {
            if (gLink.state == LINK_STATE_CONN_ESTABLISHED)
            {
                gLink.lag = LAG_SLAVE;
            }
            if (gLink.state == LINK_STATE_HANDSHAKE)
            {
                gLink.playerCount = 0;
                gLink.link_field_F = FALSE;
            }
        }
    }
}

void Timer3Intr(void)
{
    StopTimer();
    StartTransfer();
}

void SerialCB(void)
{
    gLink.localId = SIO_MULTI_CNT->id;
    switch (gLink.state)
    {
        case LINK_STATE_CONN_ESTABLISHED:
            gLink.hardwareError = SIO_MULTI_CNT->error;
            DoRecv();
            DoSend();
            SendRecvDone();
            break;
        case LINK_STATE_HANDSHAKE:
            if (DoHandshake())
            {
                if (gLink.isMaster)
                {
                    gLink.state = LINK_STATE_INIT_TIMER;
                    gLink.serialIntrCounter = 8;
                }
                else
                {
                    gLink.state = LINK_STATE_CONN_ESTABLISHED;
                }
            }
            break;
    }
    gLink.serialIntrCounter++;
    sNumVBlanksWithoutSerialIntr = 0;
    if (gLink.serialIntrCounter == 8)
    {
        gLastRecvQueueCount = gLink.recvQueue.count;
    }
}

static void StartTransfer(void)
{
    REG_SIOCNT |= SIO_START;
}

static bool8 DoHandshake(void)
{
    u8 i;
    u8 playerCount;
    u16 minRecv;

    playerCount = 0;
    minRecv = 0xFFFF;
    if (gLink.handshakeAsMaster == TRUE)
    {
        REG_SIOMLT_SEND = MASTER_HANDSHAKE;
    }
    else
    {
        REG_SIOMLT_SEND = SLAVE_HANDSHAKE;
    }
    *(u64 *)gLink.handshakeBuffer = REG_SIOMLT_RECV;
    REG_SIOMLT_RECV = 0;
    gLink.handshakeAsMaster = FALSE;
    for (i = 0; i < MAX_LINK_PLAYERS; i++)
    {
        if ((gLink.handshakeBuffer[i] & ~0x3) == SLAVE_HANDSHAKE || gLink.handshakeBuffer[i] == MASTER_HANDSHAKE)
        {
            playerCount++;
            if (minRecv > gLink.handshakeBuffer[i] && gLink.handshakeBuffer[i] != 0)
                minRecv = gLink.handshakeBuffer[i];
        }
        else
        {
            if (gLink.handshakeBuffer[i] != 0xFFFF)
                playerCount = 0;
            break;
        }
    }
    gLink.playerCount = playerCount;
    if (gLink.playerCount > 1 && gLink.playerCount == sHandshakePlayerCount && gLink.handshakeBuffer[0] == MASTER_HANDSHAKE)
    {
        return TRUE;
    }
    if (gLink.playerCount > 1)
    {
        gLink.link_field_F = (minRecv & 3) + 1;
    }
    else
    {
        gLink.link_field_F = 0;
    }
    sHandshakePlayerCount = gLink.playerCount;
    return FALSE;
}

static void DoRecv(void)
{
    u16 recv[4];
    u8 i;
    u8 index;

    *(u64 *)recv = REG_SIOMLT_RECV;
    if (gLink.sendCmdIndex == 0)
    {
        for (i = 0; i < gLink.playerCount; i++)
        {
            if (gLink.checksum != recv[i] && sChecksumAvailable)
            {
                gLink.badChecksum = TRUE;
            }
        }
        gLink.checksum = 0;
        sChecksumAvailable = TRUE;
    }
    else
    {
        index = gLink.recvQueue.pos + gLink.recvQueue.count;
        if (index >= QUEUE_CAPACITY)
        {
            index -= QUEUE_CAPACITY;
        }
        if (gLink.recvQueue.count < QUEUE_CAPACITY)
        {
            for (i = 0; i < gLink.playerCount; i++)
            {
                gLink.checksum += recv[i];
                sRecvNonzeroCheck |= recv[i];
                gLink.recvQueue.data[i][gLink.recvCmdIndex][index] = recv[i];
            }
        }
        else
        {
            gLink.queueFull = QUEUE_FULL_RECV;
        }
        gLink.recvCmdIndex++;
        if (gLink.recvCmdIndex == CMD_LENGTH && sRecvNonzeroCheck)
        {
            gLink.recvQueue.count++;
            sRecvNonzeroCheck = 0;
        }
    }
}

static void DoSend(void)
{
    if (gLink.sendCmdIndex == CMD_LENGTH)
    {
        REG_SIOMLT_SEND = gLink.checksum;
        if (!sSendBufferEmpty)
        {
            gLink.sendQueue.count--;
            gLink.sendQueue.pos++;
            if (gLink.sendQueue.pos >= QUEUE_CAPACITY)
            {
                gLink.sendQueue.pos = 0;
            }
        }
        else
        {
            sSendBufferEmpty = FALSE;
        }
    }
    else
    {
        if (!sSendBufferEmpty && gLink.sendQueue.count == 0)
        {
            sSendBufferEmpty = TRUE;
        }
        if (sSendBufferEmpty)
        {
            REG_SIOMLT_SEND = 0;
        }
        else
        {
            REG_SIOMLT_SEND = gLink.sendQueue.data[gLink.sendCmdIndex][gLink.sendQueue.pos];
        }
        gLink.sendCmdIndex++;
    }
}

static void StopTimer(void)
{
    if (gLink.isMaster)
    {
        REG_TM3CNT_H &= ~TIMER_ENABLE;
        REG_TM3CNT_L = -197;
    }
}

static void SendRecvDone(void)
{
    if (gLink.recvCmdIndex == CMD_LENGTH)
    {
        gLink.sendCmdIndex = 0;
        gLink.recvCmdIndex = 0;
    }
    else if (gLink.isMaster)
    {
        REG_TM3CNT_H |= TIMER_ENABLE;
    }
}

void ResetSendBuffer(void)
{
    u8 i;
    u8 j;

    gLink.sendQueue.count = 0;
    gLink.sendQueue.pos = 0;
    for (i = 0; i < CMD_LENGTH; i++)
    {
        for (j = 0; j < QUEUE_CAPACITY; j++)
            gLink.sendQueue.data[i][j] = LINKCMD_NONE;
    }
}

void ResetRecvBuffer(void)
{
    u8 i;
    u8 j;
    u8 k;

    gLink.recvQueue.count = 0;
    gLink.recvQueue.pos = 0;
    for (i = 0; i < MAX_LINK_PLAYERS; i++)
    {
        for (j = 0; j < CMD_LENGTH; j++)
        {
            for (k = 0; k < QUEUE_CAPACITY; k++)
                gLink.recvQueue.data[i][j][k] = LINKCMD_NONE;
        }
    }
}

bool32 ShouldCheckForUnionRoom(void)
{
    return FALSE;
}
