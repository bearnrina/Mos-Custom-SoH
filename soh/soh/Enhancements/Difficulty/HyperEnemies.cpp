#include <libultraship/bridge/consolevariablebridge.h>

#include "soh/Enhancements/game-interactor/GameInteractor.h"
#include "soh/ShipInit.hpp"

extern "C" {
#include "functions.h"
#include "macros.h"
}

extern "C" PlayState* gPlayState;

static constexpr int32_t CVAR_HYPER_ENEMIES_DEFAULT = 0;
#define CVAR_HYPER_ENEMIES_NAME CVAR_ENHANCEMENT("HyperEnemies")
#define CVAR_HYPER_ENEMIES_VALUE CVarGetInteger(CVAR_HYPER_ENEMIES_NAME, CVAR_HYPER_ENEMIES_DEFAULT)

// Schedule fractional extra updates without adding persistent actor state.
// The slider displays two decimal places, so use an exact 100-frame schedule.
static bool ShouldRunExtraEnemiesUpdate() {
    float speed = CVarGetFloat(CVAR_ENHANCEMENT("HyperEnemiesSpeed"), 2.0f);
    if (!(speed >= 1.0f)) {
        speed = 1.0f;
    } else if (speed > 2.0f) {
        speed = 2.0f;
    }
    const uint32_t extraPercent = static_cast<uint32_t>(speed * 100.0f + 0.5f) - 100;
    const uint32_t progress = (gPlayState->gameplayFrames % 100) * extraPercent % 100;
    return progress + extraPercent >= 100;
}

static void MakeHyperEnemies(void* refActor) {
    // Run selected extra updates to speed up enemies and minibosses.

    Player* player = GET_PLAYER(gPlayState);
    Actor* actor = static_cast<Actor*>(refActor);

    // Some enemies are not in the ACTORCAT_ENEMY category, and some are that aren't really enemies.
    bool isEnemy = actor->category == ACTORCAT_ENEMY || actor->id == ACTOR_EN_TORCH2;
    bool isExcludedEnemy = actor->id == ACTOR_EN_FIRE_ROCK || actor->id == ACTOR_EN_ENCOUNT2;

    // Don't apply during cutscenes because it causes weird behaviour and/or crashes on some cutscenes.
    if (isEnemy && !isExcludedEnemy && !Player_InBlockingCsMode(gPlayState, player) && ShouldRunExtraEnemiesUpdate()) {
        GameInteractor::RawAction::UpdateActor(actor);
    }
}

static void UpdateHyperEnemiesState() {
    COND_HOOK(OnActorUpdate, CVAR_HYPER_ENEMIES_VALUE, MakeHyperEnemies);
}

static RegisterShipInitFunc initFunc(UpdateHyperEnemiesState, { CVAR_HYPER_ENEMIES_NAME });
