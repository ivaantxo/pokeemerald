#include "global.h"
#include "event_data.h"
#include "field_message_box.h"
#include "pokedex.h"
#include "strings.h"

bool16 ScriptGetPokedexInfo(void)
{
    gSpecialVar_5 = GetNationalPokedexCount(FLAG_GET_SEEN);
    gSpecialVar_6 = GetNationalPokedexCount(FLAG_GET_CAUGHT);

    return TRUE;
}

#define BIRCH_DEX_STRINGS 21

static const u8 *const sBirchDexRatingTexts[BIRCH_DEX_STRINGS] =
{
    gBirchDexRatingText_LessThan10,
    gBirchDexRatingText_LessThan20,
    gBirchDexRatingText_LessThan30,
    gBirchDexRatingText_LessThan40,
    gBirchDexRatingText_LessThan50,
    gBirchDexRatingText_LessThan60,
    gBirchDexRatingText_LessThan70,
    gBirchDexRatingText_LessThan80,
    gBirchDexRatingText_LessThan90,
    gBirchDexRatingText_LessThan100,
    gBirchDexRatingText_LessThan110,
    gBirchDexRatingText_LessThan120,
    gBirchDexRatingText_LessThan130,
    gBirchDexRatingText_LessThan140,
    gBirchDexRatingText_LessThan150,
    gBirchDexRatingText_LessThan160,
    gBirchDexRatingText_LessThan170,
    gBirchDexRatingText_LessThan180,
    gBirchDexRatingText_LessThan190,
    gBirchDexRatingText_LessThan200,
    gBirchDexRatingText_DexCompleted,
};

// This shows your Hoenn Pokédex rating and not your National Dex.
const u8 *GetPokedexRatingText(u32 count)
{
    u16 maxDex = DEX_COUNT - 1;
    return sBirchDexRatingTexts[(count * (BIRCH_DEX_STRINGS - 1)) / maxDex];
}

void ShowPokedexRatingMessage(void)
{
    ShowFieldMessage(GetPokedexRatingText(gSpecialVar_4));
}
