#include "constants/battle.h"
#include "constants/pokemon.h"

#define X UQ_4_12
#define ______ X(1.0) // Regular effectiveness.

const uq4_12_t gTypeEffectivenessTable[NUMERO_DE_TIPOS][NUMERO_DE_TIPOS] =
{//                   Defender -->
 //  Attacker             Ninguno  Normal  Lucha   Volador Veneno  Tierra  Roca    Bicho   Fantasma Acero   Misterio Fuego   Agua    Planta  Eléctrico Psíquico Hielo   Dragón  Siniestro Hada
    [TIPO_NINGUNO]      = {______, ______, ______, ______, ______, ______, ______, ______, ______,  ______, ______,  ______, ______, ______, ______,   ______,  ______, ______, ______,   ______},
    [TIPO_NORMAL]       = {______, ______, ______, ______, ______, ______, X(0.5), ______, X(0.0),  ______, ______,  ______, ______, ______, ______,   ______,  ______, ______, ______,   ______},
    [TIPO_LUCHA]        = {______, X(2.0), ______, X(0.5), X(0.5), ______, X(2.0), X(0.5), X(0.0),  X(2.0), ______,  ______, ______, ______, ______,   X(0.5),  X(2.0), ______, X(2.0),   X(0.5)},
    [TIPO_VOLADOR]      = {______, ______, X(2.0), ______, ______, ______, X(0.5), X(2.0), ______,  X(0.5), ______,  ______, ______, X(2.0), X(0.5),   ______,  X(0.5), ______, ______,   ______},
    [TIPO_VENENO]       = {______, ______, ______, ______, X(0.5), X(0.5), X(0.5), X(2.0), X(0.5),  X(0.0), ______,  ______, ______, X(2.0), ______,   ______,  ______, ______, ______,   X(2.0)},
    [TIPO_TIERRA]       = {______, X(0.5), ______, X(0.0), X(2.0), ______, X(2.0), X(0.5), ______,  X(2.0), ______,  X(2.0), ______, X(0.5), X(2.0),   ______,  ______, ______, ______,   ______},
    [TIPO_ROCA]         = {______, ______, X(0.5), X(2.0), ______, X(0.5), X(0.5), X(2.0), ______,  X(0.5), ______,  X(2.0), ______, ______, ______,   ______,  X(2.0), ______, ______,   ______},
    [TIPO_BICHO]        = {______, ______, X(0.5), X(0.5), X(0.5), ______, ______, ______, X(2.0),  X(0.5), ______,  X(0.5), ______, X(2.0), ______,   X(2.0),  ______, ______, X(2.0),   X(2.0)},
    [TIPO_FANTASMA]     = {______, X(0.0), ______, ______, ______, ______, ______, ______, X(2.0),  ______, ______,  ______, ______, ______, ______,   X(2.0),  ______, ______, X(0.5),   ______},
    [TIPO_ACERO]        = {______, ______, ______, ______, ______, ______, X(2.0), ______, ______,  X(0.5), ______,  X(0.5), X(0.5), ______, X(0.5),   ______,  X(2.0), ______, ______,   X(2.0)},
    [TIPO_MISTERIO]     = {______, ______, ______, ______, ______, ______, ______, ______, ______,  ______, ______,  ______, ______, ______, ______,   ______,  ______, ______, ______,   ______},
    [TIPO_FUEGO]        = {______, ______, ______, ______, ______, ______, X(0.5), X(2.0), ______,  X(2.0), ______,  X(0.5), X(0.5), X(2.0), ______,   ______,  X(2.0), X(0.0), ______,   ______},
    [TIPO_AGUA]         = {______, ______, ______, ______, ______, X(2.0), X(2.0), ______, ______,  ______, ______,  X(2.0), X(0.5), X(0.5), ______,   ______,  X(0.5), X(0.5), ______,   ______},
    [TIPO_PLANTA]       = {______, ______, ______, X(0.5), X(0.5), X(2.0), X(2.0), X(0.5), ______,  ______, ______,  X(0.5), X(2.0), X(0.5), ______,   ______,  ______, ______, ______,   ______},
    [TIPO_ELECTRICO]    = {______, ______, ______, X(2.0), ______, X(0.0), ______, ______, ______,  X(2.0), ______,  ______, X(2.0), X(0.5), X(0.5),   ______,  ______, X(0.5), ______,   ______},
    [TIPO_PSIQUICO]     = {______, ______, X(2.0), ______, X(2.0), ______, ______, ______, ______,  X(2.0), ______,  ______, ______, ______, ______,   X(0.5),  ______, ______, X(0.0),   ______},
    [TIPO_HIELO]        = {______, ______, ______, X(2.0), ______, X(2.0), ______, ______, ______,  X(0.5), ______,  X(0.5), X(0.5), X(2.0), ______,   ______,  X(0.5), X(2.0), ______,   ______},
    [TIPO_DRAGON]       = {______, ______, ______, ______, ______, ______, ______, ______, ______,  X(0.5), ______,  ______, ______, ______, ______,   ______,  ______, X(2.0), ______,   X(0.0)},
    [TIPO_SINIESTRO]    = {______, ______, X(0.5), ______, ______, ______, ______, ______, X(2.0),  ______, ______,  ______, ______, ______, ______,   X(2.0),  ______, ______, X(0.5),   X(0.5)},
    [TIPO_HADA]         = {______, ______, X(2.0), ______, X(0.5), ______, ______, ______, ______,  X(0.5), ______,  X(0.5), ______, ______, ______,   ______,  ______, X(2.0), X(2.0),   ______},
};

#undef ______
#undef X

const struct TypeInfo gTypesInfo[NUMERO_DE_TIPOS] =
{
    [TIPO_NINGUNO] =
    {
        .name = _("None"),
        .palette = 14,
        .paletteTMHM = gItemIconPalette_NormalTMHM,
    },
    [TIPO_NORMAL] =
    {
        .name = _("Normal"),
        .palette = 14,
        .paletteTMHM = gItemIconPalette_NormalTMHM,
        //.enhanceItem = ITEM_SILK_SCARF,
        //.berry = ITEM_CHILAN_BERRY,
        //.gem = ITEM_NORMAL_GEM,
    },
    [TIPO_LUCHA] =
    {
        .name = _("Lucha"),
        .palette = 14,
        .paletteTMHM = gItemIconPalette_FightingTMHM,
        //.enhanceItem = ITEM_BLACK_BELT,
        //.berry = ITEM_CHOPLE_BERRY,
        //.gem = ITEM_FIGHTING_GEM,
    },
    [TIPO_VOLADOR] =
    {
        .name = _("Volador"),
        .palette = 14,
        .paletteTMHM = gItemIconPalette_FlyingTMHM,
        //.enhanceItem = ITEM_SHARP_BEAK,
        //.berry = ITEM_COBA_BERRY,
        //.gem = ITEM_FLYING_GEM,
    },
    [TIPO_VENENO] =
    {
        .name = _("Veneno"),
        .palette = 14,
        .paletteTMHM = gItemIconPalette_PoisonTMHM,
        //.enhanceItem = ITEM_POISON_BARB,
        //.berry = ITEM_KEBIA_BERRY,
        //.gem = ITEM_POISON_GEM,
    },
    [TIPO_TIERRA] =
    {
        .name = _("Tierra"),
        .palette = 14,
        .paletteTMHM = gItemIconPalette_GroundTMHM,
        //.enhanceItem = ITEM_SOFT_SAND,
        //.berry = ITEM_SHUCA_BERRY,
        //.gem = ITEM_GROUND_GEM,
    },
    [TIPO_ROCA] =
    {
        .name = _("Roca"),
        .palette = 15,
        .paletteTMHM = gItemIconPalette_RockTMHM,
        //.enhanceItem = ITEM_HARD_STONE,
        //.berry = ITEM_CHARTI_BERRY,
        //.gem = ITEM_ROCK_GEM,
    },
    [TIPO_BICHO] =
    {
        .name = _("Bicho"),
        .palette = 14,
        .paletteTMHM = gItemIconPalette_BugTMHM,
        //.enhanceItem = ITEM_SILVER_POWDER,
        //.berry = ITEM_TANGA_BERRY,
        //.gem = ITEM_BUG_GEM,
    },
    [TIPO_FANTASMA] =
    {
        .name = _("Fantasma"),
        .palette = 14,
        .paletteTMHM = gItemIconPalette_GhostTMHM,
        //.enhanceItem = ITEM_SPELL_TAG,
        //.berry = ITEM_KASIB_BERRY,
        //.gem = ITEM_GHOST_GEM,
    },
    [TIPO_ACERO] =
    {
        .name = _("Acero"),
        .palette = 14,
        .paletteTMHM = gItemIconPalette_SteelTMHM,
        //.enhanceItem = ITEM_METAL_COAT,
        //.berry = ITEM_BABIRI_BERRY,
        //.gem = ITEM_STEEL_GEM,
    },
    [TIPO_MISTERIO] =
    {
        .name = _("???"),
        .generic = _("a ??? move"),
        .palette = 14,
    },
    [TIPO_FUEGO] =
    {
        .name = _("Fuego"),
        .palette = 14,
        .paletteTMHM = gItemIconPalette_FireTMHM,
        //.enhanceItem = ITEM_CHARCOAL,
        //.berry = ITEM_OCCA_BERRY,
        //.gem = ITEM_FIRE_GEM,
    },
    [TIPO_AGUA] =
    {
        .name = _("Agua"),
        .palette = 14,
        .paletteTMHM = gItemIconPalette_WaterTMHM,
        //.enhanceItem = ITEM_MYSTIC_WATER,
        //.berry = ITEM_PASSHO_BERRY,
        //.gem = ITEM_WATER_GEM,
    },
    [TIPO_PLANTA] =
    {
        .name = _("Planta"),
        .palette = 14,
        .paletteTMHM = gItemIconPalette_GrassTMHM,
        //.enhanceItem = ITEM_MIRACLE_SEED,
        //.berry = ITEM_RINDO_BERRY,
        //.gem = ITEM_GRASS_GEM,
    },
    [TIPO_ELECTRICO] =
    {
        .name = _("Eléctrico"),
        .palette = 15,
        .paletteTMHM = gItemIconPalette_ElectricTMHM,
        //.enhanceItem = ITEM_MAGNET,
        //.berry = ITEM_WACAN_BERRY,
        //.gem = ITEM_ELECTRIC_GEM,
    },
    [TIPO_PSIQUICO] =
    {
        .name = _("Psíquico"),
        .palette = 15,
        .paletteTMHM = gItemIconPalette_PsychicTMHM,
        //.enhanceItem = ITEM_CUCHARA_TORCIDA,
        //.berry = ITEM_PAYAPA_BERRY,
        //.gem = ITEM_PSYCHIC_GEM,
    },
    [TIPO_HIELO] =
    {
        .name = _("Hielo"),
        .palette = 15,
        .paletteTMHM = gItemIconPalette_IceTMHM,
        //.enhanceItem = ITEM_NEVER_MELT_ICE,
        //.berry = ITEM_YACHE_BERRY,
        //.gem = ITEM_ICE_GEM,
    },
    [TIPO_DRAGON] =
    {
        .name = _("Dragón"),
        .palette = 15,
        .paletteTMHM = gItemIconPalette_DragonTMHM,
        //.enhanceItem = ITEM_DRAGON_FANG,
        //.berry = ITEM_HABAN_BERRY,
        //.gem = ITEM_DRAGON_GEM,
    },
    [TIPO_SINIESTRO] =
    {
        .name = _("Siniestro"),
        .palette = 14,
        .paletteTMHM = gItemIconPalette_DarkTMHM,
        //.enhanceItem = ITEM_BLACK_GLASSES,
        //.berry = ITEM_COLBUR_BERRY,
        //.gem = ITEM_DARK_GEM,
    },
    [TIPO_HADA] =
    {
        .name = _("Hada"),
        .palette = 15,
        .paletteTMHM = gItemIconPalette_FairyTMHM,
        //.enhanceItem = ITEM_FAIRY_FEATHER,
        //.berry = ITEM_ROSELI_BERRY,
        //.gem = ITEM_FAIRY_GEM,
    },
};
