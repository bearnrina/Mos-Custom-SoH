#ifndef MO_HEALING_ITEM_EFFECTS_H
#define MO_HEALING_ITEM_EFFECTS_H

// Include after global.h. Option 0 preserves the original game's behavior.
#include <libultraship/bridge/consolevariablebridge.h>
#include "soh/Enhancements/enhancementTypes.h"

static inline s32 MoHealing_HealthAmount(const char* key, s32 vanilla) {
    static const s32 amounts[] = { 0, 16, 32, 48, 64, 80, 160, -1 };
    s32 option = CVarGetInteger(key, 0);
    if (option < 1 || option > 7) return vanilla;
    return amounts[option] < 0 ? MAX_HEALTH : amounts[option];
}

static inline void MoHealing_AddMagic(PlayState* play, const char* key, s32 vanilla) {
    static const s32 percentages[] = { 0, 10, 25, 50, 75, 100 };
    s32 option = CVarGetInteger(key, 0);
    if (option < 1 || option > 5) {
        if (vanilla < 0) Magic_Fill(play);
        else Magic_RequestChange(play, vanilla, MAGIC_ADD);
        return;
    }
    if (!gSaveContext.isMagicAcquired) return;
    // Calculate against the currently acquired meter, including Double Magic.
    s32 capacity = gSaveContext.magicCapacity;
    s32 amount = (capacity * percentages[option] + 99) / 100;
    if (option == 5) amount = capacity;
    Magic_RequestChange(play, amount, MAGIC_ADD);
}

#endif
