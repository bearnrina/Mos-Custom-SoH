#pragma once
#include <z64.h>
#define GORON_KEY_RING_SHOP_FIRST_ID 0x38
#define GORON_KEY_RING_SHOP_COUNT 9
#define TEXT_GORON_KEY_RING_DESC_BASE 0x9432
#define TEXT_GORON_KEY_RING_PROMPT_BASE 0x943B
#define TEXT_GORON_KEY_RING_PICKUP_BASE 0x9444
typedef char GoronKeyRingOwnershipFitsSave[
    (RAND_INF_OBTAINED_SHOP_GANONS_CASTLE_KEY_RING < sizeof(((SaveContext*)0)->ship.randomizerInf) * 8) ? 1 : -1];
#ifdef __cplusplus
extern "C" {
#endif
s32 GoronKeyRingsShop_GetIndex(s16 shopItemId);
s16 GoronKeyRingsShop_GetStock(void);
s16 GoronKeyRingsShop_GetRandomizerGet(s16 shopItemId);
GetItemEntry GoronKeyRingsShop_GetItemEntry(s16 shopItemId);
s32 GoronKeyRingsShop_IsPickup(GetItemEntry entry);
void GoronKeyRingsShop_Give(s16 randomizerGet);
#ifdef __cplusplus
}
#endif
