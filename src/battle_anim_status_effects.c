#include "global.h"
#include "battle.h"
#include "battle_anim.h"
#include "decompress.h"
#include "gpu_regs.h"
#include "palette.h"
#include "sprite.h"
#include "task.h"
#include "trig.h"
#include "util.h"
#include "constants/battle_anim.h"
#include "constants/rgb.h"

extern const struct CompressedBattleAnim gTablaAnimacionesBatalla[];
extern const struct OamData gOamData_AffineOff_ObjNormal_8x8;
extern const struct OamData gOamData_AffineOff_ObjBlend_64x64;

static void AnimTask_FrozenIceCube_Step1(u8 taskId);
static void AnimTask_FrozenIceCube_Step2(u8 taskId);
static void AnimTask_FrozenIceCube_Step3(u8 taskId);
static void AnimTask_FrozenIceCube_Step4(u8 taskId);
static void Task_DoStatusAnimation(u8 taskId);

static const union AnimCmd sAnim_WeatherBallNormal[] =
{
    ANIMCMD_FRAME(0, 3),
    ANIMCMD_JUMP(0)
};

static const union AnimCmd *const sAnims_WeatherBallNormal[] =
{
    sAnim_WeatherBallNormal
};

const struct SpriteTemplate gWeatherBallUpSpriteTemplate =
{
    .tileTag = ANIM_TAG_WEATHER_BALL,
    .paletteTag = ANIM_TAG_WEATHER_BALL,
    .oam = &gOamData_AffineOff_ObjNormal_32x32,
    .anims = sAnims_WeatherBallNormal,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = AnimWeatherBallUp,
};

const struct SpriteTemplate gWeatherBallNormalDownSpriteTemplate =
{
    .tileTag = ANIM_TAG_WEATHER_BALL,
    .paletteTag = ANIM_TAG_WEATHER_BALL,
    .oam = &gOamData_AffineOff_ObjNormal_32x32,
    .anims = sAnims_WeatherBallNormal,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = AnimWeatherBallDown,
};

static const union AnimCmd sAnim_SpinningSparkle[] =
{
    ANIMCMD_FRAME(0, 3),
    ANIMCMD_FRAME(16, 3),
    ANIMCMD_FRAME(32, 3),
    ANIMCMD_FRAME(48, 3),
    ANIMCMD_FRAME(64, 3),
    ANIMCMD_END
};

const union AnimCmd *const gAnims_SpinningSparkle[] =
{
    sAnim_SpinningSparkle
};

const struct SpriteTemplate gSpinningSparkleSpriteTemplate =
{
    .tileTag = ANIM_TAG_SPARKLE_4,
    .paletteTag = ANIM_TAG_SPARKLE_4,
    .oam = &gOamData_AffineOff_ObjNormal_32x32,
    .anims = gAnims_SpinningSparkle,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = AnimSpinningSparkle,
};

static const struct Subsprite sFrozenIceCubeSubsprites[] =
{
    {
        .x = -16,
        .y = -16,
        .shape = SPRITE_SHAPE(64x64),
        .size = SPRITE_SIZE(64x64),
        .tileOffset = 0,
        .priority = 2
    },
    {
        .x = -16,
        .y =  48,
        .shape = SPRITE_SHAPE(64x32),
        .size = SPRITE_SIZE(64x32),
        .tileOffset = 64,
        .priority = 2
    },
    {
        .x =  48,
        .y = -16,
        .shape = SPRITE_SHAPE(32x64),
        .size = SPRITE_SIZE(32x64),
        .tileOffset = 96,
        .priority = 2
    },
    {
        .x =  48,
        .y =  48,
        .shape = SPRITE_SHAPE(32x32),
        .size = SPRITE_SIZE(32x32),
        .tileOffset = 128,
        .priority = 2
    },
};

static const struct SubspriteTable sFrozenIceCubeSubspriteTable[] =
{
    {ARRAY_COUNT(sFrozenIceCubeSubsprites), sFrozenIceCubeSubsprites},
};

static const struct SpriteTemplate sFrozenIceCubeSpriteTemplate =
{
    .tileTag = ANIM_TAG_ICE_CUBE,
    .paletteTag = ANIM_TAG_ICE_CUBE,
    .oam = &gOamData_AffineOff_ObjBlend_64x64,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

void AnimTask_FrozenIceCubeAttacker(u8 taskId)
{
    s16 x = GetBattlerSpriteCoord(gBattleAnimAttacker, BATTLER_COORD_X_2) - 32;
    s16 y = GetBattlerSpriteCoord(gBattleAnimAttacker, BATTLER_COORD_Y_PIC_OFFSET) - 36;
    u8 spriteId;

    SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_EFFECT_BLEND | BLDCNT_TGT2_ALL);
    SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(0, 16));
    spriteId = CreateSprite(&sFrozenIceCubeSpriteTemplate, x, y, 4);
    if (GetSpriteTileStartByTag(ANIM_TAG_ICE_CUBE) == 0xFFFF)
        gSprites[spriteId].invisible = TRUE;
    SetSubspriteTables(&gSprites[spriteId], sFrozenIceCubeSubspriteTable);
    gTasks[taskId].data[15] = spriteId;
    gTasks[taskId].func = AnimTask_FrozenIceCube_Step1;
}

void AnimTask_CentredFrozenIceCube(u8 taskId)
{
    // same as AnimTask_FrozenIceCube but center position on target(s)
    s16 x, y;
    u8 spriteId;
    u8 battler1 = gBattleAnimTarget;
    u8 battler2 = BATTLE_PARTNER(battler1);

    if (!EsContraEntrenador() || IsAlly(gBattleAnimAttacker, gBattleAnimTarget))
    {
        x = GetBattlerSpriteCoord(battler1, BATTLER_COORD_X_2);
        y = GetBattlerSpriteCoord(battler1, BATTLER_COORD_Y_PIC_OFFSET);
    }
    else
    {
        x = (GetBattlerSpriteCoord(battler1, BATTLER_COORD_X_2) + GetBattlerSpriteCoord(battler2, BATTLER_COORD_X_2)) / 2;
        y = (GetBattlerSpriteCoord(battler1, BATTLER_COORD_Y_PIC_OFFSET) + GetBattlerSpriteCoord(battler2, BATTLER_COORD_Y_PIC_OFFSET)) / 2;
    }

    x -= 32;
    y -= 36;

    spriteId = CreateSprite(&sFrozenIceCubeSpriteTemplate, x, y, 4);
    if (GetSpriteTileStartByTag(ANIM_TAG_ICE_CUBE) == 0xFFFF)
        gSprites[spriteId].invisible = TRUE;

    SetSubspriteTables(&gSprites[spriteId], sFrozenIceCubeSubspriteTable);
    gTasks[taskId].data[15] = spriteId;
    gTasks[taskId].func = AnimTask_FrozenIceCube_Step1;
}

void AnimTask_FrozenIceCube(u8 taskId)
{
    s16 x = GetBattlerSpriteCoord(gBattleAnimTarget, BATTLER_COORD_X_2) - 32;
    s16 y = GetBattlerSpriteCoord(gBattleAnimTarget, BATTLER_COORD_Y_PIC_OFFSET) - 36;
    u8 spriteId;

    SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_EFFECT_BLEND | BLDCNT_TGT2_ALL);
    SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(0, 16));
    spriteId = CreateSprite(&sFrozenIceCubeSpriteTemplate, x, y, 4);
    if (GetSpriteTileStartByTag(ANIM_TAG_ICE_CUBE) == 0xFFFF)
        gSprites[spriteId].invisible = TRUE;
    SetSubspriteTables(&gSprites[spriteId], sFrozenIceCubeSubspriteTable);
    gTasks[taskId].data[15] = spriteId;
    gTasks[taskId].func = AnimTask_FrozenIceCube_Step1;
}

static void AnimTask_FrozenIceCube_Step1(u8 taskId)
{
    gTasks[taskId].data[1]++;
    if (gTasks[taskId].data[1] == 10)
    {
        gTasks[taskId].func = AnimTask_FrozenIceCube_Step2;
        gTasks[taskId].data[1] = 0;
    }
    else
    {
        u8 var = gTasks[taskId].data[1];

        SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(var, 16 - var));
    }
}

static void AnimTask_FrozenIceCube_Step2(u8 taskId)
{
    u32 palIndex = IndexOfSpritePaletteTag(ANIM_TAG_ICE_CUBE);

    if (gTasks[taskId].data[1]++ > 13)
    {
        gTasks[taskId].data[2]++;
        if (gTasks[taskId].data[2] == 3)
        {
            u16 temp;

            temp = gPlttBufferFaded[OBJ_PLTT_ID(palIndex) + 13];
            gPlttBufferFaded[OBJ_PLTT_ID(palIndex) + 13] = gPlttBufferFaded[OBJ_PLTT_ID(palIndex) + 14];
            gPlttBufferFaded[OBJ_PLTT_ID(palIndex) + 14] = gPlttBufferFaded[OBJ_PLTT_ID(palIndex) + 15];
            gPlttBufferFaded[OBJ_PLTT_ID(palIndex) + 15] = temp;

            gTasks[taskId].data[2] = 0;
            gTasks[taskId].data[3]++;
            if (gTasks[taskId].data[3] == 3)
            {
                gTasks[taskId].data[3] = 0;
                gTasks[taskId].data[1] = 0;
                gTasks[taskId].data[4]++;
                if (gTasks[taskId].data[4] == 2)
                {
                    gTasks[taskId].data[1] = 9;
                    gTasks[taskId].func = AnimTask_FrozenIceCube_Step3;
                }
            }
        }
    }
}

static void AnimTask_FrozenIceCube_Step3(u8 taskId)
{
    gTasks[taskId].data[1]--;
    if (gTasks[taskId].data[1] == -1)
    {
        gTasks[taskId].func = AnimTask_FrozenIceCube_Step4;
        gTasks[taskId].data[1] = 0;
    }
    else
    {
        u8 var = gTasks[taskId].data[1];

        SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(var, 16 - var));
    }
}

static void AnimTask_FrozenIceCube_Step4(u8 taskId)
{
    gTasks[taskId].data[1]++;
    if (gTasks[taskId].data[1] == 37)
    {
        u8 spriteId = gTasks[taskId].data[15];

        FreeSpriteOamMatrix(&gSprites[spriteId]);
        DestroySprite(&gSprites[spriteId]);
    }
    else if (gTasks[taskId].data[1] == 39)
    {
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        DestroyAnimVisualTask(taskId);
    }
}

void AnimTask_StatsChange(u8 taskId)
{
    bool16 goesDown = FALSE;
    s16 animStatId = 0;
    bool16 sharply = FALSE;

    switch (gBattleSpritesDataPtr->animationData->animArg)
    {
    case STAT_ANIM_PLUS1 + ESTADISTICA_ATAQUE:     goesDown = FALSE;  animStatId = STAT_ANIM_PAL_ATK;      break;
    case STAT_ANIM_PLUS1 + ESTADISTICA_DEFENSA:     goesDown = FALSE;  animStatId = STAT_ANIM_PAL_DEF;      break;
    case STAT_ANIM_PLUS1 + ESTADISTICA_VELOCIDAD:   goesDown = FALSE;  animStatId = STAT_ANIM_PAL_SPEED;    break;
    case STAT_ANIM_PLUS1 + ESTADISTICA_ATAQUE_ESPECIAL:   goesDown = FALSE;  animStatId = STAT_ANIM_PAL_SPATK;    break;
    case STAT_ANIM_PLUS1 + ESTADISTICA_DEFENSA_ESPECIAL:   goesDown = FALSE;  animStatId = STAT_ANIM_PAL_SPDEF;    break;
    case STAT_ANIM_PLUS1 + ESTADISTICA_PRECISION:     goesDown = FALSE;  animStatId = STAT_ANIM_PAL_ACC;      break;
    case STAT_ANIM_PLUS1 + ESTADISTICA_EVASION: goesDown = FALSE;  animStatId = STAT_ANIM_PAL_EVASION;  break;

    case STAT_ANIM_MINUS1 + ESTADISTICA_ATAQUE:     goesDown = TRUE;  animStatId = STAT_ANIM_PAL_ATK;      break;
    case STAT_ANIM_MINUS1 + ESTADISTICA_DEFENSA:     goesDown = TRUE;  animStatId = STAT_ANIM_PAL_DEF;      break;
    case STAT_ANIM_MINUS1 + ESTADISTICA_VELOCIDAD:   goesDown = TRUE;  animStatId = STAT_ANIM_PAL_SPEED;    break;
    case STAT_ANIM_MINUS1 + ESTADISTICA_ATAQUE_ESPECIAL:   goesDown = TRUE;  animStatId = STAT_ANIM_PAL_SPATK;    break;
    case STAT_ANIM_MINUS1 + ESTADISTICA_DEFENSA_ESPECIAL:   goesDown = TRUE;  animStatId = STAT_ANIM_PAL_SPDEF;    break;
    case STAT_ANIM_MINUS1 + ESTADISTICA_PRECISION:     goesDown = TRUE;  animStatId = STAT_ANIM_PAL_ACC;      break;
    case STAT_ANIM_MINUS1 + ESTADISTICA_EVASION: goesDown = TRUE;  animStatId = STAT_ANIM_PAL_EVASION;  break;

    case STAT_ANIM_PLUS2 + ESTADISTICA_ATAQUE:     goesDown = FALSE;  animStatId = STAT_ANIM_PAL_ATK;      sharply = TRUE;  break;
    case STAT_ANIM_PLUS2 + ESTADISTICA_DEFENSA:     goesDown = FALSE;  animStatId = STAT_ANIM_PAL_DEF;      sharply = TRUE;  break;
    case STAT_ANIM_PLUS2 + ESTADISTICA_VELOCIDAD:   goesDown = FALSE;  animStatId = STAT_ANIM_PAL_SPEED;    sharply = TRUE;  break;
    case STAT_ANIM_PLUS2 + ESTADISTICA_ATAQUE_ESPECIAL:   goesDown = FALSE;  animStatId = STAT_ANIM_PAL_SPATK;    sharply = TRUE;  break;
    case STAT_ANIM_PLUS2 + ESTADISTICA_DEFENSA_ESPECIAL:   goesDown = FALSE;  animStatId = STAT_ANIM_PAL_SPDEF;    sharply = TRUE;  break;
    case STAT_ANIM_PLUS2 + ESTADISTICA_PRECISION:     goesDown = FALSE;  animStatId = STAT_ANIM_PAL_ACC;      sharply = TRUE;  break;
    case STAT_ANIM_PLUS2 + ESTADISTICA_EVASION: goesDown = FALSE;  animStatId = STAT_ANIM_PAL_EVASION;  sharply = TRUE;  break;

    case STAT_ANIM_MINUS2 + ESTADISTICA_ATAQUE:     goesDown = TRUE;  animStatId = STAT_ANIM_PAL_ATK;      sharply = TRUE;  break;
    case STAT_ANIM_MINUS2 + ESTADISTICA_DEFENSA:     goesDown = TRUE;  animStatId = STAT_ANIM_PAL_DEF;      sharply = TRUE;  break;
    case STAT_ANIM_MINUS2 + ESTADISTICA_VELOCIDAD:   goesDown = TRUE;  animStatId = STAT_ANIM_PAL_SPEED;    sharply = TRUE;  break;
    case STAT_ANIM_MINUS2 + ESTADISTICA_ATAQUE_ESPECIAL:   goesDown = TRUE;  animStatId = STAT_ANIM_PAL_SPATK;    sharply = TRUE;  break;
    case STAT_ANIM_MINUS2 + ESTADISTICA_DEFENSA_ESPECIAL:   goesDown = TRUE;  animStatId = STAT_ANIM_PAL_SPDEF;    sharply = TRUE;  break;
    case STAT_ANIM_MINUS2 + ESTADISTICA_PRECISION:     goesDown = TRUE;  animStatId = STAT_ANIM_PAL_ACC;      sharply = TRUE;  break;
    case STAT_ANIM_MINUS2 + ESTADISTICA_EVASION: goesDown = TRUE;  animStatId = STAT_ANIM_PAL_EVASION;  sharply = TRUE;  break;

    case STAT_ANIM_MULTIPLE_PLUS1:  goesDown = FALSE;  animStatId = STAT_ANIM_PAL_MULTIPLE;  sharply = FALSE;  break;
    case STAT_ANIM_MULTIPLE_PLUS2:  goesDown = FALSE;  animStatId = STAT_ANIM_PAL_MULTIPLE;  sharply = TRUE;   break;
    case STAT_ANIM_MULTIPLE_MINUS1: goesDown = TRUE;   animStatId = STAT_ANIM_PAL_MULTIPLE;  sharply = FALSE;  break;
    case STAT_ANIM_MULTIPLE_MINUS2: goesDown = TRUE;   animStatId = STAT_ANIM_PAL_MULTIPLE;  sharply = TRUE;   break;

    default:
        DestroyAnimVisualTask(taskId);
        return;
    }

    gBattleAnimArgs[0] = goesDown;
    gBattleAnimArgs[1] = animStatId;
    gBattleAnimArgs[2] = FALSE; // Whether it's the target, rather than attacker
    gBattleAnimArgs[3] = FALSE; // Whether it hits multiple battlers
    gBattleAnimArgs[4] = sharply;
    gTasks[taskId].func = InitStatsChangeAnimation;
    gTasks[taskId].func(taskId);
}

#undef CASE

void LaunchStatusAnimation(u8 battlerId, u8 statusAnimId)
{
    u8 taskId;

    gBattleAnimAttacker = battlerId;
    gBattleAnimTarget = battlerId;
    LaunchBattleAnimation(ANIM_TYPE_STATUS, statusAnimId);
    taskId = CreateTask(Task_DoStatusAnimation, 10);
    gTasks[taskId].data[0] = battlerId;
}

static void Task_DoStatusAnimation(u8 taskId)
{
    gAnimScriptCallback();
    if (!gAnimScriptActive)
    {
        gBattleSpritesDataPtr->healthBoxesData[gTasks[taskId].data[0]].statusAnimActive = FALSE;
        DestroyTask(taskId);
    }
}
