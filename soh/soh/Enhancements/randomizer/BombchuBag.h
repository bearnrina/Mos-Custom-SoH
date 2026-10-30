#pragma once

#include <z64.h>

#define TEXT_BOMBCHU_BAG_SHOP_DESC 0x9423
#define TEXT_BOMBCHU_BAG_SHOP_PROMPT 0x9424
#define TEXT_BOMBCHU_BAG_SHOP_PICKUP 0x9425

// The installer appends this ownership flag before RAND_INF_MAX without renumbering existing flags.
// Base saves already serialize ship.randomizerInf. Fail compilation if its storage has no spare bit.
typedef char BombchuBagOwnershipFitsSave[
    (RAND_INF_OBTAINED_SHOP_BOMBCHU_BAG < sizeof(((SaveContext*)0)->ship.randomizerInf) * 8) ? 1 : -1];

// Enhancement-only messages, independent of randomizer seed items.
#define TEXT_BOMBCHU_BAG_30_PICKUP 0x94E0
#define TEXT_BOMBCHU_BAG_40_PICKUP 0x94E1
#define TEXT_BOMBCHU_BAG_40_TOKEN_OFFER 0x94E2
#define TEXT_BOMBCHU_BAG_100_TOKEN_OFFER 0x94E3
#define TEXT_BOMBCHU_BAG_REWARD_THANKS 0x94E4

typedef char BombchuBagUpgradesFitSave[
    (RAND_INF_OBTAINED_SKULLTULA_BOMBCHU_BAG_40 < sizeof(((SaveContext*)0)->ship.randomizerInf) * 8) ? 1 : -1];

#ifdef __cplusplus
extern "C" {
#endif
s32 BombchuBag_IsEnabled(void);
s32 BombchuBag_HasBag(void);
s32 BombchuBag_GetCapacity(void);
s32 BombchuBag_IsPickup(GetItemEntry entry);
void BombchuBag_Give(u16 textId);
s32 BombchuBag_IsSkulltulaRewardClaimed(s16 actorParams);
GetItemEntry BombchuBag_GetSkulltulaReward(s16 actorParams);
#ifdef __cplusplus
}
#endif
