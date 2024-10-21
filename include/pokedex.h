#ifndef GUARD_POKEDEX_H
#define GUARD_POKEDEX_H

extern void (*gPokedexVBlankCB)(void);

void ResetPokedex(void);
u16 GetNationalPokedexCount(u8);
u8 DisplayCaughtMonDexPage(u16 species, bool32 isShiny, u32 personality);
s8 GetSetPokedexFlag(u16 nationalDexNo, u8 caseID);
u16 CreateMonSpriteFromNationalDexNumberHGSS(u16 nationalNum, s16 x, s16 y, u16 paletteSlot);
void ResetPokedexScrollPositions(void);
bool16 HasAllMons(void);
void PrintMonMeasurements(u16 species, u32 owned);
u8* ConvertMonHeightToString(u32 height);
u8* ConvertMonWeightToString(u32 weight);
void CB2_OpenPokedexPlusHGSS(void);
void Task_DisplayCaughtMonDexPageHGSS(u8);

#endif // GUARD_POKEDEX_H
