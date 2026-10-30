#ifndef MO_ENEMY_HEALTH_H
#define MO_ENEMY_HEALTH_H

#include <libultraship/bridge/consolevariablebridge.h>
#include "soh/Enhancements/game-interactor/GameInteractor_Hooks.h"

// Always scale from vanilla baseline HP, never from already scaled HP.
static inline int MoEnemyHealth_ScaleBase(int baseHealth) {
    int percent = CVarGetInteger(CVAR_ENHANCEMENT("MoEnemyHealthPercent"), 100);
    switch (percent) {
        case 50: case 100: case 150: case 200: case 300: case 400: case 500: break;
        default: percent = 100; break;
    }
    if (baseHealth <= 0) return 0;
    int scaled = (baseHealth * percent + 99) / 100;
    return scaled > 255 ? 255 : scaled;
}

#endif
