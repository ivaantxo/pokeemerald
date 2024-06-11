#ifndef GUARD_CONFIG_DEBUG_H
#define GUARD_CONFIG_DEBUG_H

// Overworld Debug
#define DEBUG_OVERWORLD_MENU            TRUE                // Enables an overworld debug menu to change flags, variables, giving pokemon and more, accessed by holding R and pressing START while in the overworld by default.
#define DEBUG_OVERWORLD_HELD_KEYS       (R_BUTTON)          // The keys required to be held to open the debug menu.
#define DEBUG_OVERWORLD_TRIGGER_EVENT   pressedStartButton  // The event that opens the menu when holding the key(s) defined in DEBUG_OVERWORLD_HELD_KEYS.
#define DEBUG_OVERWORLD_IN_MENU         FALSE               // Replaces the overworld debug menu button combination with a start menu entry (above Pokédex).

// Battle Debug Menu
#define DEBUG_BATTLE_MENU               TRUE    // If set to TRUE, enables a debug menu to use in battles by pressing the Select button.

// Pokémon Debug
#define DEBUG_POKEMON_SPRITE_VISUALIZER TRUE    // Enables a debug menu for Pokémon sprites and icons, accessed by pressing Select in the summary screen.

#endif // GUARD_CONFIG_DEBUG_H
