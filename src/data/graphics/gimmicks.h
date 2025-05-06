// trigger data
static const u8 ALIGNED(4) sMegaTriggerGfx[] = INCBIN_U8("graphics/battle_interface/mega_trigger.4bpp");
static const u8 ALIGNED(4) sZMoveTriggerGfx[] = INCBIN_U8("graphics/battle_interface/z_move_trigger.4bpp");
static const u8 ALIGNED(4) sBurstTriggerGfx[] = INCBIN_U8("graphics/battle_interface/burst_trigger.4bpp");

static const u16 sMegaTriggerPal[] = INCBIN_U16("graphics/battle_interface/mega_trigger.gbapal");
static const u16 sZMoveTriggerPal[] = INCBIN_U16("graphics/battle_interface/z_move_trigger.gbapal");
static const u16 sBurstTriggerPal[] = INCBIN_U16("graphics/battle_interface/burst_trigger.gbapal");

static const struct SpriteSheet sSpriteSheet_MegaTrigger = {sMegaTriggerGfx, sizeof(sMegaTriggerGfx), TAG_GIMMICK_TRIGGER_TILE};
static const struct SpriteSheet sSpriteSheet_ZMoveTrigger = {sZMoveTriggerGfx, sizeof(sZMoveTriggerGfx), TAG_GIMMICK_TRIGGER_TILE};
static const struct SpriteSheet sSpriteSheet_BurstTrigger = {sBurstTriggerGfx, sizeof(sBurstTriggerGfx), TAG_GIMMICK_TRIGGER_TILE};

static const struct SpritePalette sSpritePalette_MegaTrigger = {sMegaTriggerPal, TAG_GIMMICK_TRIGGER_TILE};
static const struct SpritePalette sSpritePalette_ZMoveTrigger = {sZMoveTriggerPal, TAG_GIMMICK_TRIGGER_PAL};
static const struct SpritePalette sSpritePalette_BurstTrigger = {sBurstTriggerPal, TAG_GIMMICK_TRIGGER_TILE};

static const struct OamData sOamData_GimmickTrigger =
{
    .y = 0,
    .affineMode = 0,
    .objMode = 0,
    .mosaic = 0,
    .bpp = 0,
    .shape = ST_OAM_SQUARE,
    .x = 0,
    .matrixNum = 0,
    .size = 2,
    .tileNum = 0,
    .priority = 1,
    .paletteNum = 0,
    .affineParam = 0,
};

static const union AnimCmd sSpriteAnim_GimmickTriggerOff[] =
{
    ANIMCMD_FRAME(0, 0),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_GimmickTriggerOn[] =
{
    ANIMCMD_FRAME(16, 0),
    ANIMCMD_END
};

static const union AnimCmd *const sSpriteAnimTable_GimmickTrigger[] =
{
    sSpriteAnim_GimmickTriggerOff,
    sSpriteAnim_GimmickTriggerOn,
};

static void SpriteCb_GimmickTrigger(struct Sprite *sprite);
static const struct SpriteTemplate sSpriteTemplate_GimmickTrigger =
{
    .tileTag = TAG_GIMMICK_TRIGGER_TILE,
    .paletteTag = TAG_GIMMICK_TRIGGER_PAL,
    .oam = &sOamData_GimmickTrigger,
    .anims = sSpriteAnimTable_GimmickTrigger,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCb_GimmickTrigger,
};

// indicator data
static const u8 ALIGNED(4) sMegaIndicatorGfx[] = INCBIN_U8("graphics/battle_interface/mega_indicator.4bpp");
static const u8 ALIGNED(4) sAlphaIndicatorGfx[] = INCBIN_U8("graphics/battle_interface/alpha_indicator.4bpp");
static const u8 ALIGNED(4) sOmegaIndicatorGfx[] = INCBIN_U8("graphics/battle_interface/omega_indicator.4bpp");

static const u16 sMiscIndicatorPal[] = INCBIN_U16("graphics/battle_interface/misc_indicator.gbapal"); // has room for more colors
static const u16 sMegaIndicatorPal[] = INCBIN_U16("graphics/battle_interface/mega_indicator.gbapal");

static const struct SpriteSheet sSpriteSheet_MegaIndicator = {sMegaIndicatorGfx, sizeof(sMegaIndicatorGfx), TAG_MEGA_INDICATOR_TILE};
static const struct SpriteSheet sSpriteSheet_AlphaIndicator = {sAlphaIndicatorGfx, sizeof(sAlphaIndicatorGfx), TAG_ALPHA_INDICATOR_TILE};
static const struct SpriteSheet sSpriteSheet_OmegaIndicator = {sOmegaIndicatorGfx, sizeof(sOmegaIndicatorGfx), TAG_OMEGA_INDICATOR_TILE};

static const struct SpritePalette sSpritePalette_MiscIndicator = {sMiscIndicatorPal, TAG_MISC_INDICATOR_PAL};
static const struct SpritePalette sSpritePalette_MegaIndicator = {sMegaIndicatorPal, TAG_MEGA_INDICATOR_PAL};

static const struct OamData sOamData_GimmickIndicator =
{
    .shape = SPRITE_SHAPE(16x16),
    .size = SPRITE_SIZE(16x16),
    .priority = 1,
};

static void SpriteCb_GimmickIndicator(struct Sprite *sprite);
static const struct SpriteTemplate sSpriteTemplate_GimmickIndicator =
{
    .tileTag = TAG_GIMMICK_TRIGGER_TILE,   // updated dynamically
    .paletteTag = TAG_GIMMICK_TRIGGER_PAL,   // updated dynamically
    .oam = &sOamData_GimmickIndicator,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCb_GimmickIndicator,
};
