#pragma once

#include <z64.h>

#define TEXT_SKELETON_KEY_SHOP_DESC 0x942A
#define TEXT_SKELETON_KEY_SHOP_PROMPT 0x942B
#define TEXT_SKELETON_KEY_SHOP_PICKUP 0x942C

// Appended before RAND_INF_MAX without renumbering old flags; saved by the base save manager.
typedef char SkeletonKeyOwnershipFitsSave[
    (RAND_INF_OBTAINED_SHOP_SKELETON_KEY < sizeof(((SaveContext*)0)->ship.randomizerInf) * 8) ? 1 : -1];

#ifdef __cplusplus
extern "C" {
#endif
GetItemEntry SkeletonKeyShop_GetItemEntry(void);
s32 SkeletonKeyShop_UnlocksDoors(PlayState* play);
#ifdef __cplusplus
}
#endif
