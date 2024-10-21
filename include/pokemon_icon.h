#ifndef GUARD_POKEMON_ICON_H
#define GUARD_POKEMON_ICON_H

const u32 *GetMonIconTiles(u16 species, u32 personality);
const u32 *GetMonIconPtr(u16 speciesId, u32 personality);
u16 GetIconSpecies(u16 species, u32 personality);
u8 CreateMonIconNoPersonality(u16 species, void (*callback)(struct Sprite *), s16 x, s16 y, u8 subpriority);
void FreeAndDestroyMonIconSprite(struct Sprite *sprite);
u8 CreateMonIcon(u16 species, void (*callback)(struct Sprite *), s16 x, s16 y, u8 subpriority, u32 personality);
u8 SetMonIconPalette(struct Pokemon *, struct Sprite *, u8);
u8 UpdateMonIconFrame(struct Sprite *sprite);
void SpriteCB_MonIcon(struct Sprite *sprite);
void SetPartyHPBarSprite(struct Sprite *sprite, u8 animNum);

#endif // GUARD_POKEMON_ICON_H
