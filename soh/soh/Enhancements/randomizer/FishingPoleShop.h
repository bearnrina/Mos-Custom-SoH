#pragma once
#include <z64.h>
#define TEXT_FISHING_POLE_SHOP_DESC 0x942D
#define TEXT_FISHING_POLE_SHOP_PROMPT 0x942E
#define TEXT_FISHING_POLE_SHOP_PICKUP 0x942F
#define TEXT_FISHING_POLE_OWNER_NO_POLE 0x9430
#define TEXT_FISHING_POLE_OWNER_START 0x9431
// This existing randomizer ownership bit is also serialized by normal saves.
typedef char FishingPoleOwnershipFitsSave[
    (RAND_INF_FISHING_POLE_FOUND < sizeof(((SaveContext*)0)->ship.randomizerInf) * 8) ? 1 : -1];
#ifdef __cplusplus
extern "C" {
#endif
GetItemEntry FishingPoleShop_GetItemEntry(void);
s32 FishingPoleShop_IsRequired(void);
s32 FishingPoleShop_IsBlocked(void);
#ifdef __cplusplus
}
#endif
