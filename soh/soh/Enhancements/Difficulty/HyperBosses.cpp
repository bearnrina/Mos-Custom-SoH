#include <libultraship/bridge/consolevariablebridge.h>

#include "soh/ShipInit.hpp"
#include "soh/Enhancements/game-interactor/GameInteractor.h"
#include "soh/Enhancements/boss-rush/BossRush.h"

extern "C" {
#include "functions.h"
#include "macros.h"
#include "variables.h"
}

extern "C" PlayState* gPlayState;

#define CVAR_HYPER_BOSSES_DEFAULT 0
#define CVAR_HYPER_BOSSES_NAME CVAR_ENHANCEMENT("HyperBosses")
#define CVAR_HYPER_BOSSES_VALUE CVarGetInteger(CVAR_HYPER_BOSSES_NAME, CVAR_HYPER_BOSSES_DEFAULT)

bool IsHyperBossesActive() {
    return CVAR_HYPER_BOSSES_VALUE ||
           (IS_BOSS_RUSH &&
            gSaveContext.ship.quest.data.bossRush.options[BR_OPTIONS_HYPERBOSSES] == BR_CHOICE_HYPERBOSSES_YES);
}

// Schedule fractional extra updates without adding persistent actor state.
// The slider displays two decimal places, so use an exact 100-frame schedule.
static bool ShouldRunExtraBossesUpdate() {
    // Boss Rush owns a separate Hyper Bosses option; retain its original double speed.
    if (IS_BOSS_RUSH &&
        gSaveContext.ship.quest.data.bossRush.options[BR_OPTIONS_HYPERBOSSES] == BR_CHOICE_HYPERBOSSES_YES) {
        return true;
    }
    float speed = CVarGetFloat(CVAR_ENHANCEMENT("HyperBossesSpeed"), 2.0f);
    if (!(speed >= 1.0f)) {
        speed = 1.0f;
    } else if (speed > 2.0f) {
        speed = 2.0f;
    }
    const uint32_t extraPercent = static_cast<uint32_t>(speed * 100.0f + 0.5f) - 100;
    const uint32_t progress = (gPlayState->gameplayFrames % 100) * extraPercent % 100;
    return progress + extraPercent >= 100;
}

void MakeHyperBosses(void* refActor) {
    // Run selected extra updates to speed up bosses.

    Player* player = GET_PLAYER(gPlayState);
    Actor* actor = static_cast<Actor*>(refActor);

    uint8_t isBossActor = actor->id == ACTOR_BOSS_GOMA ||      // Gohma
                          actor->id == ACTOR_BOSS_DODONGO ||   // King Dodongo
                          actor->id == ACTOR_EN_BDFIRE ||      // King Dodongo Fire Breath
                          actor->id == ACTOR_BOSS_VA ||        // Barinade
                          actor->id == ACTOR_BOSS_GANONDROF || // Phantom Ganon
                          actor->id == ACTOR_EN_FHG_FIRE ||    // Phantom Ganon/Ganondorf Energy Ball/Thunder
                          actor->id == ACTOR_EN_FHG ||         // Phantom Ganon's Horse
                          actor->id == ACTOR_BOSS_FD || actor->id == ACTOR_BOSS_FD2 || // Volvagia (grounded/flying)
                          actor->id == ACTOR_EN_VB_BALL ||                             // Volvagia Rocks
                          actor->id == ACTOR_BOSS_MO ||                                // Morpha
                          actor->id == ACTOR_BOSS_SST ||                               // Bongo Bongo
                          actor->id == ACTOR_BOSS_TW ||                                // Twinrova
                          actor->id == ACTOR_BOSS_GANON ||                             // Ganondorf
                          actor->id == ACTOR_BOSS_GANON2;                              // Ganon

    // Don't apply during cutscenes because it causes weird behaviour and/or crashes on some bosses.
    if (IsHyperBossesActive() && isBossActor && !Player_InBlockingCsMode(gPlayState, player) && ShouldRunExtraBossesUpdate()) {
        // Barinade needs to be updated in sequence to avoid unintended behaviour.
        if (actor->id == ACTOR_BOSS_VA) {
            // params -1 is BOSSVA_BODY
            if (actor->params == -1) {
                Actor* actorList = gPlayState->actorCtx.actorLists[ACTORCAT_BOSS].head;
                while (actorList != NULL) {
                    GameInteractor::RawAction::UpdateActor(actorList);
                    actorList = actorList->next;
                }
            }
        } else {
            GameInteractor::RawAction::UpdateActor(actor);
        }
    }
}

static void UpdateHyperBossesState() {
    COND_HOOK(OnActorUpdate, IsHyperBossesActive(), MakeHyperBosses);
}

static RegisterShipInitFunc initFunc(UpdateHyperBossesState, { CVAR_HYPER_BOSSES_NAME });