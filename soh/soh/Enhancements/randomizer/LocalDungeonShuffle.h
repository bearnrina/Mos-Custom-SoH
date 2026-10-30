#ifndef LOCAL_DUNGEON_SHUFFLE_H
#define LOCAL_DUNGEON_SHUFFLE_H
#include "z64save.h"
#ifdef __cplusplus
extern "C" {
#endif
// Returns the vanilla ID for every chest outside this save's recorded shuffle.
s16 LocalDungeonShuffle_GetItemId(s16 scene, u8 treasureFlag, s16 vanillaItemId);
#ifdef __cplusplus
}
#endif
#endif
