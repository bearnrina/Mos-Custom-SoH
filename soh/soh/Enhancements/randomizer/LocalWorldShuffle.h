#ifndef LOCAL_WORLD_SHUFFLE_H
#define LOCAL_WORLD_SHUFFLE_H
#include "z64.h"
#include "randomizerEnums.h"
#ifdef __cplusplus
extern "C" {
#endif
s32 LocalWorldShuffle_Create(void);
GetItemEntry LocalWorldShuffle_ResolveUpgrade(GetItemEntry entry);
s32 LocalWorldShuffle_IsGenerating(void);
// 0: still generating, 1: review ready, -1: failed. Poll on the menu thread.
s32 LocalWorldShuffle_PollGeneration(void);
void LocalWorldShuffle_PrepareSeedReview(void);
void LocalWorldShuffle_BeginSeedReview(u16 titleDayTime);
s32 LocalWorldShuffle_SeedReviewPending(void);
// 0: waiting/inactive, 1: accepted, 2: declined. Creation follows name entry.
s32 LocalWorldShuffle_PollSeedReview(void);
void LocalWorldShuffle_ChooseSeedReview(s32 accept);
s32 LocalWorldShuffle_HasReviewedSeed(void);
s32 LocalWorldShuffle_CommitReviewedSeed(const u8* playerName);
void LocalWorldShuffle_ReopenSeedReview(void);
u32 LocalWorldShuffle_ReviewedSeedNumber(void);
u8 LocalWorldShuffle_ReviewedSeedRating(void);
const char* LocalWorldShuffle_ReviewDescription(u8 rating);
void LocalWorldShuffle_CancelSeedReview(void);
const char* LocalWorldShuffle_RatingName(u8 rating);
s32 LocalWorldShuffle_NpcEnabled(void);
s32 LocalWorldShuffle_NpcCollected(u16 check, s32 vanillaCollected);
s32 LocalWorldShuffle_OfferNpcPrize(Actor* actor, PlayState* play, u16 check, s32 originalGetItemId, f32 xzRange, f32 yRange);
void LocalWorldShuffle_AcceptNpcPrize(Actor* actor, PlayState* play, u16 check);
// Inert outside the isolated generation context.
s32 LocalWorldShuffle_LogicAgeAllowed(u16 check, s32 adult);
s32 LocalWorldShuffle_ApplyLogicItem(u16 item);
s32 LocalWorldShuffle_CanCollect(Actor* actor, PlayState* play, s16 originalParams);
s32 LocalWorldShuffle_CheckCollected(u16 check);
s32 LocalWorldShuffle_ValidateData(const LocalWorldShuffleSaveData* data);
s32 LocalWorldShuffle_HasCheck(Actor* actor, PlayState* play, s16 originalParams);
s32 LocalWorldShuffle_IsCollected(Actor* actor, PlayState* play, s16 originalParams);
void LocalWorldShuffle_Collect(Actor* actor, PlayState* play, s16 originalParams);
GetItemEntry LocalWorldShuffle_GetEntry(Actor* actor, PlayState* play, s16 originalParams);
s32 LocalWorldShuffle_OfferPickup(Actor* actor, PlayState* play, s16 originalParams);
s32 LocalWorldShuffle_DrawPickup(Actor* actor, PlayState* play, s16 originalParams);
#ifdef __cplusplus
}
#endif
#endif
