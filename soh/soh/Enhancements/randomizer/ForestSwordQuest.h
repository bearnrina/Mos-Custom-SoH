#pragma once

#include <z64.h>

#define TEXT_FOREST_CHILD_WALLET_PICKUP 0x9429
#define FOREST_SWORD_CHEST_PARAMS ((s16)((0xF << 12) | (GI_SWORD_KOKIRI << 5) | 0x1F))
#define FOREST_SWORD_REWARD_NONE 0
#define FOREST_SWORD_REWARD_WALLET 1
#define FOREST_SWORD_REWARD_SWORD 2

// Appended by the installer. Normal saves already serialize this flag array.
typedef char ForestSwordOwnershipFitsSave[
    (RAND_INF_OBTAINED_LOST_WOODS_SWORD_CHEST < sizeof(((SaveContext*)0)->ship.randomizerInf) * 8) ? 1 : -1];

#ifdef __cplusplus
extern "C" {
#endif
s32 ForestSwordQuest_IsEnabled(void);
s32 ForestSwordQuest_HasWallet(void);
GetItemEntry ForestSwordQuest_GetWalletEntry(void);
#ifdef __cplusplus
}
#endif
