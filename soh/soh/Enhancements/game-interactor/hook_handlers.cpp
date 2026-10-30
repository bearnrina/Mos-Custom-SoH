// Mo's Custom SoH: game-interactor-specific enemy health hooks.
// Randomizer hooks remain in Enhancements/randomizer/hook_handlers.cpp.
#include <cstdint>
#include "soh/OTRGlobals.h"
#include "soh/Enhancements/game-interactor/GameInteractor.h"
#include "soh/ShipInit.hpp"
#include "soh/ObjectExtension/ActorMaximumHealth.h"
#include "src/overlays/actors/ovl_En_Zf/z_en_zf.h"

// Mo's Custom SoH: Adjustable Enemy Health (central initialization hook).
// The explicit whitelist is intentionally conservative while actor-specific
// reset/death behavior is being integrated. No boss or miniboss actor IDs.
static bool MoAdjustableEnemyHealth_IsEligible(const Actor* actor) {
    switch (actor->id) {
        case ACTOR_EN_BILI:     // Biri (including spawned Biri)
        case ACTOR_EN_SB:       // Shell Blade
        case ACTOR_EN_FIREFLY:  // Keese
        case ACTOR_EN_WF:       // Wolfos
        case ACTOR_EN_RD:       // ReDead / Gibdo
        case ACTOR_EN_TITE:     // Tektite
        case ACTOR_EN_MB:       // Moblin
        case ACTOR_EN_DODONGO:  // Dodongo
        case ACTOR_EN_BA:       // Bari
        case ACTOR_EN_CROW:     // Guay
        case ACTOR_EN_BW:       // Torch Slug
        case ACTOR_EN_FZ:       // Freezard
        case ACTOR_EN_GOMA:     // Gohma Larva
        case ACTOR_EN_RR:       // Like Like
        case ACTOR_EN_REEBA:    // Leever
        case ACTOR_EN_VM:       // Beamos, small and large
        case ACTOR_EN_ST:       // Skulltula, all ordinary variants
        case ACTOR_EN_SKB:      // Stalchild
            return true; // Ordinary variants must not inherit Octorok's params restriction.
        case ACTOR_EN_POH:      // Graveyard Poe: ordinary combat types only; exclude Flat and Sharp
            // EnPoh_Init normalizes params >= 4 to EN_POH_NORMAL.
            // EN_POH_NORMAL and EN_POH_RUPEE both use the ordinary Poe combat model.
            return actor->params == 0 || actor->params == 1;
        case ACTOR_EN_SW:       // Ordinary Skullwalltula only; exclude Gold Skulltulas
            return actor->category == ACTORCAT_ENEMY && ((actor->params & 0xE000) == 0);
        case ACTOR_EN_NY:       // En_Ny: only the active combat variant; nonzero params use unused dummy behavior
            return actor->params == 0;
        case ACTOR_EN_SKJ:      // Hostile adult Skull Kid only; EnSkj_Init normalizes actor.params
            // Friendly 0-2 and stump helper 5-6 are excluded.
            // Rear-attack scripted instant defeat remains unchanged.
            return actor->category == ACTORCAT_ENEMY &&
                   ((actor->params >= 3 && actor->params <= 4) || actor->params >= 7);
        case ACTOR_EN_OKUTA:    // Octorok; exclude projectile actors
            // Octorok actor only; projectiles use nonzero low-byte params.
            return actor->params == 0;
        case ACTOR_EN_BB:       // Bubble: exclude flame-trail effect actors
            // EnBb_Init selects combat variants only when upper parameter bits
            // are present; otherwise it initializes a noncombat flame trail.
            return (actor->params & 0xFF00) != 0;
        case ACTOR_EN_FLOORMAS: // Floormaster
            // SPAWN_SMALL (0x10) denotes pre-spawned dormant linked helpers.
            // Only the original large Floormaster is scaled at initialization.
            return actor->params != 0x10;
        case ACTOR_EN_WALLMAS: // Wallmaster; Shadow Tag is handled separately
            return true;
        case ACTOR_EN_PEEHAT:   // Adult Peahat only; larva is a separate variant
            return actor->params <= 0;
        case ACTOR_EN_ZF:       // Ordinary Lizalfos / Dinolfos only
            // EnZf_Init strips the upper parameter byte and sign-extends the low byte.
            // Ordinary Lizalfos/Dinolfos have params below the miniboss threshold.
            return actor->params < ENZF_TYPE_LIZALFOS_MINIBOSS_A;
        default:
            return false;
    }
}

static void MoAdjustableEnemyHealth_OnActorInit(void* actorRef) {
    Actor* actor = static_cast<Actor*>(actorRef);
    if (actor == nullptr || !MoAdjustableEnemyHealth_IsEligible(actor)) {
        return;
    }

    const int32_t percent = CVarGetInteger(CVAR_ENHANCEMENT("MoEnemyHealthPercent"), 100);
    switch (percent) {
        case 50:
        case 100:
        case 150:
        case 200:
        case 300:
        case 400:
        case 500:
            break;
        default:
            return;
    }
    if (percent == 100 || actor->colChkInfo.health == 0) {
        return;
    }

    // Round up fractional HP (e.g. 1 HP at 50% stays 1 HP).
    const int32_t scaled = (static_cast<int32_t>(actor->colChkInfo.health) * percent + 99) / 100;
    actor->colChkInfo.health = static_cast<uint8_t>(scaled > 255 ? 255 : scaled);
    // Keep the health bar maximum in sync, independent of hook registration order.
    SetActorMaximumHealth(actor, actor->colChkInfo.health);
}

static void MoAdjustableEnemyHealth_RegisterHook() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnActorInit>(MoAdjustableEnemyHealth_OnActorInit);
}

static RegisterShipInitFunc moAdjustableEnemyHealthInit(MoAdjustableEnemyHealth_RegisterHook);
