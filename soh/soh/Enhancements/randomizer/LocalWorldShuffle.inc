// Included after LocalDungeonShuffle.inc by SeedContext.cpp; shares graph restoration helpers.
#include "LocalWorldShuffle.h"
#include "ForestSwordQuest.h"
#include "soh/SohGui/SohGui.hpp"
#include "soh/SaveManager.h"
#include "randomizer.h"
#include "soh/Enhancements/item-tables/ItemTableManager.h"
#include "soh/Enhancements/custom-message/CustomMessageManager.h"
#include "soh/Enhancements/custom-message/CustomMessageTypes.h"
#include "soh/Enhancements/game-interactor/GameInteractor.h"
#include "soh/ShipInit.hpp"
#include <tuple>
#include <array>
#include <future>
#include <chrono>

static void RegisterLocalWorldLockedChestMessage() {
    COND_ID_HOOK(OnOpenText, TEXT_LOCAL_WORLD_LOCKED_CHEST, true,
                 [](uint16_t* textId, bool* loadFromMessageTable) {
                     if (!IS_LOCAL_WORLD_SHUFFLE) return;
                     CustomMessage msg(LINK_IS_ADULT ? "The lid of the chest is&rusted shut..."
                                                     : "The chest is sealed with an&old, rusty lock...");
                     msg.Format();
                     // Use the requested wording in every language until translations are supplied.
                     CustomMessage localized(msg.GetEnglish(MF_RAW), msg.GetEnglish(MF_RAW), msg.GetEnglish(MF_RAW));
                     localized.LoadIntoFont();
                     *loadFromMessageTable = false;
                 });
}

static RegisterShipInitFunc localWorldLockedChestInit(RegisterLocalWorldLockedChestMessage);

namespace {
// Active only on the isolated generation thread and context, never other modes.
thread_local Rando::Context* localWorldLogicContext = nullptr;
thread_local const std::set<RandomizerCheck>* localWorldLogicChecks = nullptr;
thread_local bool localWorldEnforceLogicAge = false;

// These are the techniques already supported by this mode's reachability search.
// The general glitch framework remains disabled.
constexpr RandomizerTrick localWorldReviewTricks[] = {
    RT_LENS_HW, RT_LENS_JABU_MQ, RT_LENS_BOTW, RT_LENS_SHADOW, RT_LENS_SHADOW_PLATFORM,
    RT_LENS_BONGO, RT_LENS_SHADOW_MQ, RT_LENS_SHADOW_MQ_INVISIBLE_BLADES,
    RT_LENS_SHADOW_MQ_PLATFORM, RT_LENS_SHADOW_MQ_DEADHAND, RT_LENS_SPIRIT,
    RT_LENS_SPIRIT_MQ, RT_LENS_GTG, RT_LENS_GTG_MQ, RT_LENS_GANON, RT_LENS_GANON_MQ,
    RT_DAMAGE_BOOST_SIMPLE, RT_HOVER_BOOST_SIMPLE,
};

void LocalWorldSetReviewProfile(Rando::Context& context, u8 rating) {
    for (int trick = 0; trick < RT_MAX; ++trick) {
        context.GetTrickOption(static_cast<RandomizerTrick>(trick)).Set(RO_GENERIC_OFF);
    }
    for (auto trick : localWorldReviewTricks) {
        const bool enabled = rating >= LOCAL_WORLD_RATING_TRICKY &&
            (trick != RT_HOVER_BOOST_SIMPLE || rating == LOCAL_WORLD_RATING_GLITCHES);
        context.GetTrickOption(trick).Set(enabled ? RO_GENERIC_ON : RO_GENERIC_OFF);
    }
}

bool LocalWorldLocationMatchesLayout(const Rando::Location& location, bool mq) {
    return location.GetQuest() == RCQUEST_BOTH ||
           location.GetQuest() == (mq ? RCQUEST_MQ : RCQUEST_VANILLA);
}

enum class LocalWorldItemCategory { Perishable, Reward, Quest };

LocalWorldItemCategory LocalWorldCategory(RandomizerGet item) {
    switch (item) {
        case RG_RECOVERY_HEART:
        case RG_GREEN_RUPEE:
        case RG_BLUE_RUPEE:
        case RG_RED_RUPEE:
        case RG_PURPLE_RUPEE:
        case RG_HUGE_RUPEE:
        case RG_ICE_TRAP:
        case RG_MILK:
        case RG_FISH:
        case RG_BOMBS_5:
        case RG_BOMBS_10:
        case RG_BOMBS_20:
        case RG_BOMBCHU_5:
        case RG_BOMBCHU_10:
        case RG_BOMBCHU_20:
        case RG_ARROWS_5:
        case RG_ARROWS_10:
        case RG_ARROWS_30:
        case RG_DEKU_NUTS_5:
        case RG_DEKU_NUTS_10:
        case RG_DEKU_SEEDS_30:
        case RG_DEKU_STICK_1:
        case RG_RED_POTION_REFILL:
        case RG_GREEN_POTION_REFILL:
        case RG_BLUE_POTION_REFILL:
        case RG_TREASURE_GAME_GREEN_RUPEE:
        case RG_BUY_DEKU_NUTS_5:
        case RG_BUY_ARROWS_30:
        case RG_BUY_ARROWS_50:
        case RG_BUY_BOMBS_525:
        case RG_BUY_DEKU_NUTS_10:
        case RG_BUY_DEKU_STICK_1:
        case RG_BUY_BOMBS_10:
        case RG_BUY_FISH:
        case RG_BUY_RED_POTION_30:
        case RG_BUY_GREEN_POTION:
        case RG_BUY_BLUE_POTION:
        case RG_BUY_HEART:
        case RG_BUY_BOMBCHUS_10:
        case RG_BUY_BOMBCHUS_20:
        case RG_BUY_DEKU_SEEDS_30:
        case RG_BUY_BLUE_FIRE:
        case RG_BUY_BOTTLE_BUG:
        case RG_BUY_POE:
        case RG_BUY_FAIRYS_SPIRIT:
        case RG_BUY_ARROWS_10:
        case RG_BUY_BOMBS_20:
        case RG_BUY_BOMBS_30:
        case RG_BUY_BOMBS_535:
        case RG_BUY_RED_POTION_40:
        case RG_BUY_RED_POTION_50:
            return LocalWorldItemCategory::Perishable;
        // Lensless routes are enabled in this mode; seeing hidden objects is optional.
        case RG_DEKU_SHIELD:
        case RG_EMPTY_BOTTLE:
        case RG_BOTTLE_WITH_MILK:
        case RG_BOTTLE_WITH_RED_POTION:
        case RG_BOTTLE_WITH_GREEN_POTION:
        case RG_BOTTLE_WITH_BLUE_POTION:
        case RG_BOTTLE_WITH_FAIRY:
        case RG_BOTTLE_WITH_FISH:
        case RG_BOTTLE_WITH_BLUE_FIRE:
        case RG_BOTTLE_WITH_BUGS:
        case RG_BOTTLE_WITH_POE:
        case RG_BOTTLE_WITH_BIG_POE:
        case RG_ADULT_WALLET:
        case RG_PROGRESSIVE_WALLET:
        case RG_BOTTOM_OF_THE_WELL_SMALL_KEY:
        case RG_BOTTOM_OF_THE_WELL_KEY_RING:
        case RG_GERUDO_TRAINING_GROUND_SMALL_KEY:
        case RG_GERUDO_TRAINING_GROUND_KEY_RING:
        case RG_LENS_OF_TRUTH:
        case RG_MAGIC_BEAN:
        case RG_MAGIC_BEAN_PACK:
        case RG_GOLDEN_SCALE:
        case RG_GIANT_WALLET:
        case RG_EPONAS_SONG:
        case RG_SUNS_SONG:
        case RG_SONG_OF_STORMS:
        case RG_MINUET_OF_FOREST:
        case RG_BOLERO_OF_FIRE:
        case RG_SERENADE_OF_WATER:
        case RG_REQUIEM_OF_SPIRIT:
        case RG_PRELUDE_OF_LIGHT:
        case RG_GIANTS_KNIFE:
        case RG_BIGGORON_SWORD:
        case RG_HYLIAN_SHIELD:
        case RG_STONE_OF_AGONY:
        case RG_FARORES_WIND:
        case RG_NAYRUS_LOVE:
        case RG_ICE_ARROWS:
        case RG_DOUBLE_DEFENSE:
        case RG_PROGRESSIVE_NUT_UPGRADE:
        case RG_PROGRESSIVE_STICK_UPGRADE:
        case RG_MAGIC_DOUBLE:
        case RG_PROGRESSIVE_GORONSWORD:
        case RG_DEKU_TREE_MAP:
        case RG_DODONGOS_CAVERN_MAP:
        case RG_JABU_JABUS_BELLY_MAP:
        case RG_FOREST_TEMPLE_MAP:
        case RG_FIRE_TEMPLE_MAP:
        case RG_WATER_TEMPLE_MAP:
        case RG_SPIRIT_TEMPLE_MAP:
        case RG_SHADOW_TEMPLE_MAP:
        case RG_BOTTOM_OF_THE_WELL_MAP:
        case RG_ICE_CAVERN_MAP:
        case RG_DEKU_TREE_COMPASS:
        case RG_DODONGOS_CAVERN_COMPASS:
        case RG_JABU_JABUS_BELLY_COMPASS:
        case RG_FOREST_TEMPLE_COMPASS:
        case RG_FIRE_TEMPLE_COMPASS:
        case RG_WATER_TEMPLE_COMPASS:
        case RG_SPIRIT_TEMPLE_COMPASS:
        case RG_SHADOW_TEMPLE_COMPASS:
        case RG_BOTTOM_OF_THE_WELL_COMPASS:
        case RG_ICE_CAVERN_COMPASS:
        case RG_PIECE_OF_HEART:
        case RG_HEART_CONTAINER:
        case RG_TREASURE_GAME_HEART:
        case RG_BUY_HYLIAN_SHIELD:
        case RG_FISHING_POLE:
        case RG_HINT:
        case RG_TYCOON_WALLET:
        case RG_QUIVER_INF:
        case RG_BOMB_BAG_INF:
        case RG_BULLET_BAG_INF:
        case RG_STICK_UPGRADE_INF:
        case RG_NUT_UPGRADE_INF:
        case RG_MAGIC_INF:
        case RG_BOMBCHU_INF:
        case RG_WALLET_INF:
        case RG_BIG_BOMB_BAG:
        case RG_BIGGEST_BOMB_BAG:
        case RG_BIG_QUIVER:
        case RG_BIGGEST_QUIVER:
        case RG_BIG_BULLET_BAG:
        case RG_BIGGEST_BULLET_BAG:
        case RG_DEKU_NUT_CAPACITY_30:
        case RG_DEKU_NUT_CAPACITY_40:
        case RG_DEKU_STICK_CAPACITY_20:
        case RG_DEKU_STICK_CAPACITY_30:
        case RG_GORON_MASK:
        case RG_ZORA_MASK:
        case RG_GERUDO_MASK:
            return LocalWorldItemCategory::Reward;
        default:
            return LocalWorldItemCategory::Quest;
    }
}

// Prioritize equipment and abilities, rather than counting dungeon keys as major moves.
bool LocalWorldVarietyQuest(RandomizerGet item) {
    if (LocalWorldCategory(item) != LocalWorldItemCategory::Quest) return false;
    const auto type = Rando::StaticData::RetrieveItem(item).GetItemType();
    return type == ITEMTYPE_ITEM || type == ITEMTYPE_EQUIP || type == ITEMTYPE_SONG;
}

bool LocalWorldProtectedChildQuest(RandomizerGet item) {
    if (LocalWorldCategory(item) != LocalWorldItemCategory::Quest) return false;
    switch (item) {
        case RG_KOKIRI_SWORD:
        case RG_DEKU_SHIELD:
        case RG_FAIRY_SLINGSHOT:
        case RG_PROGRESSIVE_SLINGSHOT:
        case RG_BOMB_BAG:
        case RG_PROGRESSIVE_BOMB_BAG:
        case RG_BOOMERANG:
        case RG_LENS_OF_TRUTH:
        case RG_DINS_FIRE:
        case RG_GORONS_BRACELET:
        case RG_PROGRESSIVE_STRENGTH:
        case RG_SILVER_SCALE:
        case RG_GOLDEN_SCALE:
        case RG_PROGRESSIVE_SCALE:
        case RG_MAGIC_SINGLE:
        case RG_PROGRESSIVE_MAGIC_METER:
        case RG_FAIRY_OCARINA:
        case RG_OCARINA_OF_TIME:
        case RG_PROGRESSIVE_OCARINA:
        case RG_MAGIC_BEAN:
        case RG_MAGIC_BEAN_PACK:
        case RG_EMPTY_BOTTLE:
        case RG_BOTTLE_WITH_MILK:
        case RG_BOTTLE_WITH_RED_POTION:
        case RG_BOTTLE_WITH_GREEN_POTION:
        case RG_BOTTLE_WITH_BLUE_POTION:
        case RG_BOTTLE_WITH_FAIRY:
        case RG_BOTTLE_WITH_FISH:
        case RG_BOTTLE_WITH_BLUE_FIRE:
        case RG_BOTTLE_WITH_BUGS:
        case RG_BOTTLE_WITH_POE:
        case RG_BOTTLE_WITH_BIG_POE:
        case RG_RUTOS_LETTER:
        case RG_WEIRD_EGG:
        case RG_ZELDAS_LETTER:
        case RG_CHILD_WALLET:
        case RG_ADULT_WALLET:
        case RG_GIANT_WALLET:
        case RG_PROGRESSIVE_WALLET:
        case RG_DEKU_STICK_BAG:
        case RG_DEKU_NUT_BAG:
        case RG_PROGRESSIVE_BOMBCHU_BAG:
        case RG_BRONZE_SCALE:
        case RG_POWER_BRACELET:
        case RG_CLIMB:
        case RG_CRAWL:
        case RG_OPEN_CHEST:
        case RG_SPEAK_DEKU:
        case RG_SPEAK_GORON:
        case RG_SPEAK_HYLIAN:
        case RG_SPEAK_KOKIRI:
        case RG_SPEAK_ZORA:
        case RG_ROCS_FEATHER:
        case RG_ZELDAS_LULLABY:
        case RG_EPONAS_SONG:
        case RG_SARIAS_SONG:
        case RG_SUNS_SONG:
        case RG_SONG_OF_TIME:
        case RG_SONG_OF_STORMS:
        case RG_MINUET_OF_FOREST:
        case RG_BOLERO_OF_FIRE:
        case RG_SERENADE_OF_WATER:
        case RG_REQUIEM_OF_SPIRIT:
        case RG_NOCTURNE_OF_SHADOW:
        case RG_PRELUDE_OF_LIGHT:
        case RG_BOTTOM_OF_THE_WELL_SMALL_KEY:
        case RG_BOTTOM_OF_THE_WELL_KEY_RING:
            return true;
        default:
            return false;
    }
}

// Spirit Temple uses one persistent key counter across both ages. Keys do
// not have an age-specific equipment identity; never age-gate a shared key.
bool LocalWorldSharedQuest(RandomizerGet item) {
    return item == RG_SPIRIT_TEMPLE_SMALL_KEY || item == RG_SPIRIT_TEMPLE_KEY_RING;
}

bool LocalWorldItemAgeAllowed(RandomizerGet item, bool adult) {
    return LocalWorldCategory(item) != LocalWorldItemCategory::Quest || LocalWorldSharedQuest(item) ||
           (LocalWorldProtectedChildQuest(item) ? !adult : adult);
}

bool LocalWorldChildDistrict(const Rando::Location& location) {
    // Child dungeons belong to their parent districts; availability is checked
    // separately by the child-only logic search. No Hyrule Field or Lake Hylia.
    switch (location.GetArea()) {
        case RCAREA_MARKET:
        case RCAREA_HYRULE_CASTLE:
        case RCAREA_DEATH_MOUNTAIN_TRAIL:
        case RCAREA_GORON_CITY:
        case RCAREA_DEATH_MOUNTAIN_CRATER:
        case RCAREA_KAKARIKO_VILLAGE:
        case RCAREA_GRAVEYARD:
        case RCAREA_ZORAS_RIVER:
        case RCAREA_ZORAS_DOMAIN:
        case RCAREA_ZORAS_FOUNTAIN:
        case RCAREA_KOKIRI_FOREST:
        case RCAREA_LOST_WOODS:
        case RCAREA_SACRED_FOREST_MEADOW:
        case RCAREA_LON_LON_RANCH:
        case RCAREA_DEKU_TREE:
        case RCAREA_DODONGOS_CAVERN:
        case RCAREA_JABU_JABUS_BELLY:
        case RCAREA_BOTTOM_OF_THE_WELL:
            return location.GetScene() != SCENE_GROTTOS;
        default:
            return false;
    }
}

bool LocalWorldNpcMinigame(RandomizerCheck check) {
    switch (check) {
        case RC_MARKET_SHOOTING_GALLERY_REWARD:
        case RC_KAK_SHOOTING_GALLERY_REWARD:
        case RC_MARKET_BOMBCHU_BOWLING_FIRST_PRIZE:
        case RC_MARKET_BOMBCHU_BOWLING_SECOND_PRIZE:
        case RC_LW_OCARINA_MEMORY_GAME:
        case RC_LW_TARGET_IN_WOODS:
        case RC_LH_CHILD_FISHING:
        case RC_LH_ADULT_FISHING:
        case RC_GF_HBA_1000_POINTS:
        case RC_GF_HBA_1500_POINTS:
        case RC_LLR_TALONS_CHICKENS:
        case RC_ZR_FROGS_ZELDAS_LULLABY:
        case RC_ZR_FROGS_EPONAS_SONG:
        case RC_ZR_FROGS_SARIAS_SONG:
        case RC_ZR_FROGS_SUNS_SONG:
        case RC_ZR_FROGS_SONG_OF_TIME:
        case RC_ZR_FROGS_IN_THE_RAIN:
        case RC_ZR_FROGS_OCARINA_GAME:
            return true;
        default:
            return false;
    }
}

bool LocalWorldMinigame(RandomizerCheck check) {
    return LocalWorldNpcMinigame(check) || check == RC_GRAVEYARD_DAMPE_RACE_FREESTANDING_POH ||
           check == RC_GRAVEYARD_DAMPE_GRAVEDIGGING_TOUR;
}

// Repeat prizes have no collected bit: the saved permutation remains reusable.
// Milk stays in its bottle-dependent slot; other games must not require a bottle.
constexpr std::array<u16, LOCAL_WORLD_NPC_REPEAT_PRIZE_COUNT> localWorldNativeRepeatPrizes = {
    GI_RUPEE_GREEN, GI_RUPEE_BLUE, GI_RUPEE_RED, GI_RUPEE_PURPLE, GI_BOMBCHUS_10, GI_BOMBS_1, GI_MILK
};

// Separate base equipment from capacity rewards and exact upgrade identities.
// Legacy physical placements retain their version 3 item IDs when NPC prizes are off.
RandomizerGet LocalWorldNativeItem(const Rando::Location& location, bool npcPrizesEnabled = false) {
    if (npcPrizesEnabled) {
        switch (location.GetRandomizerCheck()) {
            case RC_DEKU_TREE_SLINGSHOT_CHEST:
            case RC_DEKU_TREE_MQ_SLINGSHOT_CHEST: return RG_FAIRY_SLINGSHOT;
            case RC_DODONGOS_CAVERN_BOMB_BAG_CHEST:
            case RC_DODONGOS_CAVERN_MQ_BOMB_BAG_CHEST: return RG_BOMB_BAG;
            case RC_FOREST_TEMPLE_BOW_CHEST:
            case RC_FOREST_TEMPLE_MQ_BOW_CHEST: return RG_FAIRY_BOW;
            default: break;
        }
    }
    switch (location.GetRandomizerCheck()) {
        case RC_MARKET_SHOOTING_GALLERY_REWARD: return RG_BIG_BULLET_BAG;
        case RC_LW_TARGET_IN_WOODS: return RG_BIGGEST_BULLET_BAG;
        case RC_KAK_SHOOTING_GALLERY_REWARD: return RG_BIG_QUIVER;
        case RC_GF_HBA_1500_POINTS: return RG_BIGGEST_QUIVER;
        case RC_MARKET_BOMBCHU_BOWLING_FIRST_PRIZE: return RG_BIG_BOMB_BAG;
        case RC_LH_ADULT_FISHING: return RG_GOLDEN_SCALE;
        case RC_ZD_DIVING_MINIGAME: return RG_SILVER_SCALE; // Fixed progression, not a shuffled NPC prize.
        // This source grants the adult-use upgrade, not another child bracelet.
        case RC_SPIRIT_TEMPLE_SILVER_GAUNTLETS_CHEST: return RG_SILVER_GAUNTLETS;
        case RC_GANONS_CASTLE_SHADOW_TRIAL_GOLDEN_GAUNTLETS_CHEST:
        case RC_GANONS_CASTLE_MQ_SPIRIT_TRIAL_GOLDEN_GAUNTLETS_CHEST: return RG_GOLDEN_GAUNTLETS;
        case RC_GC_DARUNIAS_JOY: return RG_GORONS_BRACELET;
        case RC_GRAVEYARD_HOOKSHOT_CHEST: return RG_HOOKSHOT;
        case RC_WATER_TEMPLE_LONGSHOT_CHEST:
        case RC_WATER_TEMPLE_MQ_LONGSHOT_CHEST: return RG_LONGSHOT;
        default: return location.GetVanillaItem();
    }
}

bool LocalWorldMajorItemRegion(const Rando::Location& location, RandomizerGet item) {
    const auto area = location.GetArea();
    switch (item) {
        case RG_FAIRY_SLINGSHOT:
        case RG_PROGRESSIVE_SLINGSHOT:
            return area == RCAREA_DEKU_TREE || area == RCAREA_KOKIRI_FOREST || area == RCAREA_LOST_WOODS;
        case RG_BOMB_BAG:
        case RG_PROGRESSIVE_BOMB_BAG:
            return area == RCAREA_DODONGOS_CAVERN || area == RCAREA_DEATH_MOUNTAIN_TRAIL || area == RCAREA_GORON_CITY;
        case RG_BOOMERANG:
            return area == RCAREA_JABU_JABUS_BELLY || area == RCAREA_ZORAS_DOMAIN || area == RCAREA_ZORAS_FOUNTAIN;
        case RG_FAIRY_BOW:
        case RG_PROGRESSIVE_BOW:
            return area == RCAREA_FOREST_TEMPLE || area == RCAREA_SACRED_FOREST_MEADOW || area == RCAREA_LOST_WOODS;
        case RG_LONGSHOT:
            return area == RCAREA_WATER_TEMPLE || area == RCAREA_LAKE_HYLIA;
        case RG_MEGATON_HAMMER:
            return area == RCAREA_FIRE_TEMPLE || area == RCAREA_DEATH_MOUNTAIN_CRATER;
        case RG_MIRROR_SHIELD:
            return area == RCAREA_SPIRIT_TEMPLE || area == RCAREA_DESERT_COLOSSUS;
        default:
            return true;
    }
}

bool LocalWorldPlacementAllowed(RandomizerCheck check, RandomizerGet item, bool childAccessible, bool adultAccessible) {
    const auto& location = *Rando::StaticData::GetLocation(check);
    const auto category = LocalWorldCategory(item);
    const auto nativeCategory = LocalWorldCategory(LocalWorldNativeItem(location));
    // Quest items may use any eligible chest/permanent pickup. Allow displaced
    // Rewards/Perishables in native Quest slots while retaining their other pools.
    // NPC minigames remain category-restricted below.
    if (category != nativeCategory && category != LocalWorldItemCategory::Quest &&
        nativeCategory != LocalWorldItemCategory::Quest) return false;
    if (!LocalWorldMajorItemRegion(location, item)) return false;
    if (LocalWorldCategory(item) == LocalWorldItemCategory::Quest &&
        !LocalWorldProtectedChildQuest(item) && !LocalWorldSharedQuest(item) && !adultAccessible) return false;
    if (LocalWorldSharedQuest(item) && !childAccessible && !adultAccessible) return false;
    if (LocalWorldMinigame(check) &&
        (LocalWorldCategory(item) == LocalWorldItemCategory::Quest ||
         LocalWorldCategory(item) != LocalWorldCategory(LocalWorldNativeItem(location)))) return false;
    if (LocalWorldProtectedChildQuest(item) && (!childAccessible || !LocalWorldChildDistrict(location))) return false;
    if (item == RG_ICE_TRAP && location.GetActorID() != ACTOR_EN_BOX) return false;
    return true;
}

bool LocalWorldFixedStoryItem(RandomizerGet item) {
    switch (item) {
        case RG_RUTOS_LETTER:
        case RG_MASTER_SWORD:
        case RG_FAIRY_OCARINA:
        case RG_OCARINA_OF_TIME:
        case RG_PROGRESSIVE_OCARINA:
        case RG_KOKIRI_EMERALD:
        case RG_GORON_RUBY:
        case RG_ZORA_SAPPHIRE:
        case RG_FOREST_MEDALLION:
        case RG_FIRE_MEDALLION:
        case RG_WATER_MEDALLION:
        case RG_SPIRIT_MEDALLION:
        case RG_SHADOW_MEDALLION:
        case RG_LIGHT_MEDALLION:
            return true;
        default:
            return false;
    }
}

bool IsLocalWorldBiggoronItem(RandomizerGet item) {
    switch (item) {
        case RG_POCKET_EGG:
        case RG_COJIRO:
        case RG_ODD_MUSHROOM:
        case RG_ODD_POTION:
        case RG_POACHERS_SAW:
        case RG_BROKEN_SWORD:
        case RG_PRESCRIPTION:
        case RG_EYEBALL_FROG:
        case RG_EYEDROPS:
        case RG_CLAIM_CHECK:
        case RG_BIGGORON_SWORD:
        case RG_PROGRESSIVE_GORONSWORD:
            return true;
        default:
            return false;
    }
}

bool IsLocalWorldLocation(const Rando::Location& location, bool forestSwordQuest, bool npcPrizesEnabled) {
    if (LocalWorldFixedStoryItem(location.GetVanillaItem()) ||
        location.GetRCType() == RCTYPE_ADULT_TRADE || IsLocalWorldBiggoronItem(location.GetVanillaItem()) ||
        location.GetVanillaItem() == RG_GREG_RUPEE ||
        location.GetRandomizerCheck() == RC_GF_CHEST || // Child variant is an Odd Mushroom trade source.
        location.GetRCType() == RCTYPE_GF_KEY ||
        location.GetVanillaItem() == RG_GERUDO_FORTRESS_SMALL_KEY ||
        location.GetRandomizerCheck() == RC_UNKNOWN_CHECK || location.GetVanillaItem() == RG_NONE ||
        location.GetScene() == SCENE_TREASURE_BOX_SHOP || location.GetScene() == SCENE_GROTTOS ||
        (forestSwordQuest && location.GetRandomizerCheck() == RC_KF_KOKIRI_SWORD_CHEST)) {
        return false; // Keep grottos, chest-game prizes and the fixed Forest Sword wallet out of the pool.
    }
    if (LocalWorldNpcMinigame(location.GetRandomizerCheck())) {
        return npcPrizesEnabled && LocalWorldCategory(LocalWorldNativeItem(location)) != LocalWorldItemCategory::Quest;
    }
    const auto collection = location.GetCollectionCheck();
    if (location.GetActorID() == ACTOR_EN_BOX) {
        return collection.type == SPOILER_CHK_CHEST;
    }
    if (location.GetActorID() == ACTOR_EN_ITEM00 || location.GetActorID() == ACTOR_ITEM_B_HEART) {
        return collection.type == SPOILER_CHK_COLLECTABLE && collection.flag > 0 && collection.flag < 32;
    }
    // These permanent pickups are listed as event checks rather than actor checks.
    return location.GetRandomizerCheck() == RC_LH_UNDERWATER_ITEM ||
           location.GetRandomizerCheck() == RC_LH_SUN;
}
} // namespace

// Each reward in an upgrade family grants the next tier, regardless of its source.
extern "C" GetItemEntry LocalWorldShuffle_ResolveUpgrade(GetItemEntry entry) {
    if (!IS_LOCAL_WORLD_SHUFFLE) return entry;
    if (entry.modIndex == MOD_RANDOMIZER && (entry.itemId == RG_MAGIC_SINGLE || entry.itemId == RG_MAGIC_DOUBLE)) {
        auto resolved = Rando::StaticData::RetrieveItem(gSaveContext.isMagicAcquired ? RG_MAGIC_DOUBLE : RG_MAGIC_SINGLE).GetGIEntry_Copy();
        resolved.getItemFrom = entry.getItemFrom;
        return resolved;
    }
    if (entry.modIndex != MOD_NONE) return entry;
    struct Family { u32 upgrade; u8 firstLevel; std::vector<s16> items; };
    static const Family families[] = {
        {UPG_SCALE, 0, {GI_SCALE_SILVER, GI_SCALE_GOLDEN}},
        {UPG_STRENGTH, 0, {GI_BRACELET, GI_GAUNTLETS_SILVER, GI_GAUNTLETS_GOLD}},
        {UPG_WALLET, 0, {GI_WALLET_ADULT, GI_WALLET_GIANT}},
        {UPG_QUIVER, 0, {GI_BOW, GI_QUIVER_40, GI_QUIVER_50}},
        {UPG_BULLET_BAG, 0, {GI_BULLET_BAG_30, GI_BULLET_BAG_40, GI_BULLET_BAG_50}},
        {UPG_BOMB_BAG, 0, {GI_BOMB_BAG_20, GI_BOMB_BAG_30, GI_BOMB_BAG_40}},
        {UPG_STICKS, 1, {GI_STICK_UPGRADE_20, GI_STICK_UPGRADE_30}},
        {UPG_NUTS, 1, {GI_NUT_UPGRADE_30, GI_NUT_UPGRADE_40}},
    };
    s16 actual = entry.getItemId;
    for (const auto& family : families) {
        if (std::find(family.items.begin(), family.items.end(), actual) == family.items.end()) continue;
        const int level = CUR_UPG_VALUE(family.upgrade);
        const size_t index = std::clamp<int>(level - family.firstLevel, 0, static_cast<int>(family.items.size()) - 1);
        actual = family.items[index];
        break;
    }
    if (actual == GI_HOOKSHOT || actual == GI_LONGSHOT) {
        actual = INV_CONTENT(ITEM_HOOKSHOT) == ITEM_NONE ? GI_HOOKSHOT : GI_LONGSHOT;
    }
    if (actual == entry.getItemId) return entry;
    auto resolved = ItemTableManager::Instance->RetrieveItemEntry(MOD_NONE, actual);
    resolved.getItemFrom = entry.getItemFrom;
    return resolved;
}

// Keep validation aligned with the progressive rewards actually granted in game.
extern "C" s32 LocalWorldShuffle_ApplyLogicItem(u16 item) {
    auto context = Rando::Context::GetInstance();
    if (localWorldLogicContext == nullptr || context.get() != localWorldLogicContext) return false;
    auto logic = context->GetLogic();
    switch (item) {
        case RG_FAIRY_BOW:
        case RG_FAIRY_SLINGSHOT:
        case RG_BIG_QUIVER:
        case RG_BIGGEST_QUIVER:
        case RG_BIG_BULLET_BAG:
        case RG_BIGGEST_BULLET_BAG:
        case RG_BIG_BOMB_BAG:
        case RG_BIGGEST_BOMB_BAG:
        case RG_BOMB_BAG: {
            const bool bow = item == RG_FAIRY_BOW || item == RG_BIG_QUIVER || item == RG_BIGGEST_QUIVER;
            const bool sling = item == RG_FAIRY_SLINGSHOT || item == RG_BIG_BULLET_BAG || item == RG_BIGGEST_BULLET_BAG;
            const u32 upgrade = bow ? UPG_QUIVER : sling ? UPG_BULLET_BAG : UPG_BOMB_BAG;
            const u32 inventoryItem = bow ? ITEM_BOW : sling ? ITEM_SLINGSHOT : ITEM_BOMB;
            logic->SetUpgrade(upgrade, std::min<int>(3, logic->CurrentUpgrade(upgrade) + 1));
            logic->SetInventory(inventoryItem, inventoryItem);
            return true;
        }
        case RG_ADULT_WALLET:
        case RG_GIANT_WALLET:
            logic->SetUpgrade(UPG_WALLET, std::min<int>(2, logic->CurrentUpgrade(UPG_WALLET) + 1));
            return true;
        case RG_DEKU_STICK_CAPACITY_20:
        case RG_DEKU_STICK_CAPACITY_30:
        case RG_DEKU_NUT_CAPACITY_30:
        case RG_DEKU_NUT_CAPACITY_40: {
            const u32 upgrade = item == RG_DEKU_STICK_CAPACITY_20 || item == RG_DEKU_STICK_CAPACITY_30 ? UPG_STICKS : UPG_NUTS;
            logic->SetUpgrade(upgrade, std::min<int>(3, std::max<int>(1, logic->CurrentUpgrade(upgrade)) + 1));
            return true;
        }
        case RG_SILVER_SCALE:
        case RG_GOLDEN_SCALE:
            logic->SetUpgrade(UPG_SCALE, std::min<int>(2, logic->CurrentUpgrade(UPG_SCALE) + 1));
            return true;
        case RG_MAGIC_SINGLE:
        case RG_MAGIC_DOUBLE: {
            auto* save = logic->GetSaveContext();
            save->magicLevel = std::min<int>(2, save->magicLevel + 1);
            save->isMagicAcquired = true;
            save->isDoubleMagicAcquired = save->magicLevel >= 2;
            return true;
        }
        case RG_HOOKSHOT:
        case RG_LONGSHOT:
            logic->SetInventory(ITEM_HOOKSHOT, logic->CurrentInventory(ITEM_HOOKSHOT) == ITEM_NONE ? ITEM_HOOKSHOT : ITEM_LONGSHOT);
            return true;
        case RG_GORONS_BRACELET:
        case RG_SILVER_GAUNTLETS:
        case RG_GOLDEN_GAUNTLETS: {
            const u8 level = std::min<int>(3, logic->CurrentUpgrade(UPG_STRENGTH) + 1);
            logic->SetUpgrade(UPG_STRENGTH, std::max<u8>(level, logic->CurrentUpgrade(UPG_STRENGTH)));
            logic->SetRandoInf(RAND_INF_CAN_GRAB, true);
            return true;
        }
        default:
            return false;
    }
}

extern "C" s32 LocalWorldShuffle_LogicAgeAllowed(u16 check, s32 adult) {
    auto context = Rando::Context::GetInstance();
    if (!localWorldEnforceLogicAge || context.get() != localWorldLogicContext ||
        localWorldLogicChecks == nullptr || !localWorldLogicChecks->count(static_cast<RandomizerCheck>(check))) return true;
    auto item = context->GetItemLocation(static_cast<RandomizerCheck>(check))->GetPlacedRandomizerGet();
    return LocalWorldItemAgeAllowed(item, adult != 0);
}

namespace Rando {
bool Context::GenerateLocalWorldShuffle(LocalWorldShuffleSaveData& result) {
    auto originalContext = Context::GetInstance();
    auto originalLogic = ::logic;
    auto* originalRawContext = ::ctx;
    auto graph = std::make_unique<std::array<Region, RR_MAX>>();
    for (auto& region : *graph) {
        region.scene = SCENE_ID_MAX;
        region.timePass = false;
    }
    for (size_t i = 0; i < RR_MAX; ++i) {
        SwapLocalShuffleRegion(areaTable[i], (*graph)[i]);
    }
    std::vector<EventAccess> originalGrottoEvents;
    originalGrottoEvents.swap(grottoEvents);
    std::map<RandomizerRegion, SpiritLogicData> originalSpiritData;
    originalSpiritData = Region::spiritLogicData; // Keep fixed access callbacks available during validation.
    std::shared_ptr<Context> simulation;
    auto* savedLogicContext = localWorldLogicContext;
    auto* savedLogicChecks = localWorldLogicChecks;
    const bool savedEnforceAge = localWorldEnforceLogicAge;
    LocalShuffleRestore restore{[&]() {
        localWorldLogicContext = savedLogicContext;
        localWorldLogicChecks = savedLogicChecks;
        localWorldEnforceLogicAge = savedEnforceAge;
        if (simulation) {
            auto simulationLogic = simulation->GetLogic();
            if (simulationLogic->mSaveContext != nullptr && simulationLogic->mSaveContext != &gSaveContext) {
                delete simulationLogic->mSaveContext;
                simulationLogic->mSaveContext = nullptr;
            }
            // Logic normally keeps a shared reference to Context; break the temporary cycle.
            simulationLogic->SetContext(nullptr);
        }
        mContext = originalContext;
        ::ctx = originalRawContext;
        ::logic = originalLogic;
        for (size_t i = 0; i < RR_MAX; ++i) {
            SwapLocalShuffleRegion(areaTable[i], (*graph)[i]);
        }
        originalGrottoEvents.swap(grottoEvents);
        originalSpiritData.swap(Region::spiritLogicData);
    }};

    try {
        simulation = std::make_shared<Context>();
        mContext = simulation;
        simulation->GetLogic()->SetContext(simulation);
        LocalWorldSetReviewProfile(*simulation, LOCAL_WORLD_RATING_GLITCHES);
        simulation->GetOption(RSK_FOREST).Set(RO_CLOSED_FOREST_ON);
        simulation->GetOption(RSK_DOOR_OF_TIME).Set(RO_DOOROFTIME_CLOSED);
        simulation->GetOption(RSK_GERUDO_FORTRESS).Set(RO_GF_CARPENTERS_NORMAL);
        simulation->GetOption(RSK_RAINBOW_BRIDGE).Set(RO_BRIDGE_VANILLA);
        simulation->GetOption(RSK_SHUFFLE_MAPANDCOMPASS).Set(RO_DUNGEON_ITEM_LOC_VANILLA);
        simulation->GetOption(RSK_KEYSANITY).Set(RO_DUNGEON_ITEM_LOC_VANILLA);
        simulation->GetOption(RSK_BOSS_KEYSANITY).Set(RO_DUNGEON_ITEM_LOC_VANILLA);
        simulation->GetOption(RSK_GANONS_BOSS_KEY).Set(RO_GANON_BOSS_KEY_VANILLA);
        simulation->GetOption(RSK_GERUDO_KEYS).Set(RO_GERUDO_KEYS_VANILLA);
        simulation->GetOption(RSK_SELECTED_STARTING_AGE).Set(RO_AGE_CHILD);
        simulation->GetOption(RSK_STARTING_HEARTS).Set(
            std::clamp<int>(result.startingHearts ? result.startingHearts : 3, 1, 5) - 1);
        for (auto* dungeon : simulation->GetDungeons()->GetDungeonList()) {
            if (result.masterQuest != 0) {
                dungeon->SetMQ();
            } else {
                dungeon->ClearMQ();
            }
        }
        RegionTable_Init();
        std::vector<RandomizerCheck> checks;
        std::vector<RandomizerGet> rewards;
        std::vector<RandomizerCheck> required;
        std::set<std::tuple<int, int, int>> identities;
        for (const auto& location : StaticData::GetLocationTable()) {
            if (location.GetRandomizerCheck() == RC_UNKNOWN_CHECK ||
                !LocalWorldLocationMatchesLayout(location, result.masterQuest != 0)) {
                continue;
            }
            const auto check = location.GetRandomizerCheck();
            simulation->GetItemLocation(check)->SetPlacedItem(LocalWorldNativeItem(location, result.npcPrizesEnabled != 0));
            if (IsLocalWorldLocation(location, result.forestSwordQuest != 0, result.npcPrizesEnabled != 0)) {
                if (!LocalWorldNpcMinigame(check) && location.GetActorID() != ACTOR_ID_MAX &&
                    !identities.emplace(location.GetActorID(), location.GetScene(), location.GetActorParams()).second) {
                    SPDLOG_ERROR("Local world shuffle: ambiguous source {}", location.GetName());
                    return false;
                }
                checks.push_back(check);
                rewards.push_back(LocalWorldNativeItem(location, result.npcPrizesEnabled != 0));
                // Optional pickups may be inaccessible; validate completion below.
            }
        }
        if (checks.empty() || checks.size() > LOCAL_WORLD_SHUFFLE_MAX_CHECKS) {
            return false;
        }
        if (result.forestSwordQuest) {
            // Keep the wallet in the original sword chest and the sword in the
            // unchanged Deku Theater grotto. Reuse the otherwise empty pocket
            // check only inside this temporary simulation; never persist it as a placement.
            simulation->GetOption(RSK_SHUFFLE_CHILD_WALLET).Set(RO_GENERIC_ON);
            simulation->GetItemLocation(RC_KF_KOKIRI_SWORD_CHEST)->SetPlacedItem(RG_CHILD_WALLET);
            simulation->GetItemLocation(RC_LINKS_POCKET)->SetPlacedItem(RG_KOKIRI_SWORD);
            for (auto& region : areaTable) {
                auto& locations = region.locations;
                locations.erase(std::remove_if(locations.begin(), locations.end(), [](const LocationAccess& access) {
                    return access.GetLocation() == RC_LINKS_POCKET;
                }), locations.end());
            }
            areaTable[RR_DEKU_THEATER].locations.emplace_back(RC_LINKS_POCKET, []() {
                return ::logic->CanOpenLargeChest();
            });
            required.push_back(RC_KF_KOKIRI_SWORD_CHEST);
            required.push_back(RC_LINKS_POCKET);
        }
        // Validate unchanged NPC rewards too: a reachable chest alone is not completion.
        for (auto boss : StaticData::dungeonRewardLocations) {
            required.push_back(boss);
        }
        required.push_back(RC_GANON);
        const std::set<RandomizerCheck> shuffledChecks(checks.begin(), checks.end());
        localWorldLogicContext = simulation.get();
        localWorldLogicChecks = &shuffledChecks;
        std::set<RandomizerCheck> possibleAsAdult;
        auto searchWorld = [&](bool childOnly, bool assumeInventory) {
            localWorldEnforceLogicAge = !assumeInventory;
            simulation->GetLogic()->Reset();
            Regions::AccessReset();
            simulation->LocationReset();
            if (assumeInventory) {
                // Used only to discover potential destinations at each age.
                // Real placement validation below never assumes items.
                for (const auto& location : StaticData::GetLocationTable()) {
                    if (location.GetRandomizerCheck() != RC_UNKNOWN_CHECK && location.GetVanillaItem() != RG_NONE &&
                        LocalWorldLocationMatchesLayout(location, result.masterQuest != 0)) {
                        StaticData::RetrieveItem(LocalWorldNativeItem(location, result.npcPrizesEnabled != 0)).ApplyEffect();
                    }
                }
                if (result.forestSwordQuest) StaticData::RetrieveItem(RG_CHILD_WALLET).ApplyEffect();
            }
            if (assumeInventory) {
                // Maximum access is a destination upper bound, not a playable route.
                // Freeze the assumed inventory: revisiting fixed progressive rewards
                // would otherwise apply upgrades again and overflow their levels.
                simulation->GetLogic()->CalculatingAvailableChecks = true;
                if (!childOnly) {
                    // Discover adult destinations independently of the child-to-adult
                    // route. Real validation below must still earn time travel.
                    RegionTable(RR_ROOT)->adultDay = true;
                    RegionTable(RR_ROOT)->adultNight = true;
                }
            }
            GetAccessibleLocationsStruct search(0);
            search.haveTimeAccess = true;
            // PropagateTimeTravel requires this flag. With sphere-zero complete,
            // exit processing does not turn it on: child-only searches stay child.
            search.foundTempleOfTime = !childOnly;
            search.validatedStartingRegion = true;
            search.sphereZeroComplete = true;
            search.timePassChildDay = search.timePassChildNight = true;
            search.timePassAdultDay = search.timePassAdultNight = !childOnly;
            do {
                search.InitLoop();
                for (size_t i = 0; i < search.regionPool.size(); ++i) {
                    ProcessRegion(RegionTable(search.regionPool[i]), search);
                }
            } while (search.logicUpdated);
            std::set<RandomizerCheck> reached;
            for (const auto& location : StaticData::GetLocationTable()) {
                auto check = location.GetRandomizerCheck();
                if (check != RC_UNKNOWN_CHECK && simulation->GetItemLocation(check)->IsAddedToPool()) reached.insert(check);
            }
            if (assumeInventory && !childOnly) {
                // Maximum-inventory adult destinations only. Real searches below
                // must earn every item at its assigned age through the scoped hook.
                const auto previousRegion = ::logic->CurrentRegionKey;
                const auto previousCheck = ::logic->CurrentCheckKey;
                for (size_t regionIndex = 0; regionIndex < RR_MAX; ++regionIndex) {
                    auto& region = areaTable[regionIndex];
                    ::logic->CurrentRegionKey = static_cast<RandomizerRegion>(regionIndex);
                    for (const auto& access : region.locations) {
                        ::logic->CurrentCheckKey = access.GetLocation();
                        if ((region.adultDay && access.CheckConditionAtAgeTime(::logic->IsAdult, ::logic->AtDay)) ||
                            (region.adultNight && access.CheckConditionAtAgeTime(::logic->IsAdult, ::logic->AtNight))) {
                            possibleAsAdult.insert(access.GetLocation());
                        }
                    }
                }
                ::logic->CurrentRegionKey = previousRegion;
                ::logic->CurrentCheckKey = previousCheck;
            }
            return reached;
        };
        const auto possibleAsChild = searchWorld(true, true);
        searchWorld(false, true);
        SPDLOG_INFO("Local world shuffle: seed {}, MQ {}, Forest Sword {}, {} sources; maximum child/adult access {}/{}",
                    result.seed, result.masterQuest != 0, result.forestSwordQuest != 0, checks.size(),
                    possibleAsChild.size(), possibleAsAdult.size());
        auto canPlace = [&](size_t slot, RandomizerGet item) {
            return LocalWorldPlacementAllowed(checks[slot], item, possibleAsChild.count(checks[slot]) != 0,
                                              possibleAsAdult.count(checks[slot]) != 0);
        };
        std::mt19937 random(result.seed);
        std::copy(localWorldNativeRepeatPrizes.begin(), localWorldNativeRepeatPrizes.end(), result.npcRepeatPrizes);
        if (result.npcPrizesEnabled) {
            std::shuffle(std::begin(result.npcRepeatPrizes), std::end(result.npcRepeatPrizes) - 1, random);
        }
        // Preserve the original item multiset and legal age/region domains.
        // Quest placements may exchange with chest/permanent-pickup categories;
        // Perishables and Rewards otherwise keep their own pools and have no age limit.
        const auto originalRewards = rewards;
        std::vector<std::vector<size_t>> destinations(checks.size());
        for (size_t item = 0; item < checks.size(); ++item) {
            for (size_t slot = 0; slot < checks.size(); ++slot) {
                if (canPlace(slot, originalRewards[item])) destinations[item].push_back(slot);
            }
            std::shuffle(destinations[item].begin(), destinations[item].end(), random);
            auto native = std::find(destinations[item].begin(), destinations[item].end(), item);
            if (native != destinations[item].end()) std::iter_swap(native, destinations[item].begin());
        }
        std::vector<int> itemAtSlot(checks.size(), -1);
        std::function<bool(size_t, std::vector<bool>&)> match = [&](size_t item, std::vector<bool>& visited) {
            for (auto slot : destinations[item]) {
                if (visited[slot]) continue;
                visited[slot] = true;
                if (itemAtSlot[slot] < 0 || match(static_cast<size_t>(itemAtSlot[slot]), visited)) {
                    itemAtSlot[slot] = static_cast<int>(item);
                    return true;
                }
            }
            return false;
        };
        for (size_t item = 0; item < checks.size(); ++item) {
            std::vector<bool> visited(checks.size(), false);
            if (!match(item, visited)) {
                SPDLOG_ERROR("Local world shuffle: matching failed for {} (item {}, native source '{}'), {} eligible slots, {} alternating-path slots",
                    StaticData::RetrieveItem(originalRewards[item]).GetName().GetEnglish(),
                    static_cast<unsigned>(originalRewards[item]), StaticData::GetLocation(checks[item])->GetName(),
                    destinations[item].size(), std::count(visited.begin(), visited.end(), true));
                for (size_t slot = 0; slot < checks.size(); ++slot) {
                    if (!visited[slot]) continue;
                    const int owner = itemAtSlot[slot];
                    const auto assigned = owner < 0 ? RG_NONE : originalRewards[owner];
                    SPDLOG_ERROR("Local world shuffle: contested slot '{}' [{}], child {}, adult {}, assigned '{}'",
                        StaticData::GetLocation(checks[slot])->GetName(), static_cast<unsigned>(checks[slot]),
                        possibleAsChild.count(checks[slot]) != 0, possibleAsAdult.count(checks[slot]) != 0,
                        StaticData::RetrieveItem(assigned).GetName().GetEnglish());
                }
                for (size_t slot = 0; slot < checks.size(); ++slot) {
                    if (itemAtSlot[slot] >= 0) continue;
                    SPDLOG_ERROR("Local world shuffle: unmatched slot '{}' [{}], native item {}, child {}, adult {}",
                        StaticData::GetLocation(checks[slot])->GetName(), static_cast<unsigned>(checks[slot]),
                        static_cast<unsigned>(originalRewards[slot]), possibleAsChild.count(checks[slot]) != 0,
                        possibleAsAdult.count(checks[slot]) != 0);
                }
                return false;
            }
        }
        size_t protectedItemCount = 0;
        for (size_t slot = 0; slot < checks.size(); ++slot) {
            rewards[slot] = originalRewards[itemAtSlot[slot]];
            simulation->GetItemLocation(checks[slot])->SetPlacedItem(rewards[slot]);
            protectedItemCount += LocalWorldProtectedChildQuest(rewards[slot]) ? 1 : 0;
        }
        auto storeSeed = [&](u8 rating) {
            result.state = LOCAL_WORLD_SHUFFLE_READY;
            result.seedRating = rating;
            result.placementCount = static_cast<u16>(checks.size());
            for (size_t i = 0; i < checks.size(); ++i) {
                result.placements[i] = {static_cast<u16>(checks[i]), static_cast<u16>(rewards[i]), 0,
                                        static_cast<u8>(possibleAsChild.count(checks[i]) != 0),
                                        static_cast<u8>(possibleAsAdult.count(checks[i]) != 0)};
            }
            // No warning may bypass invalid IDs, missing checks, item-pool or placement constraints.
            if (!LocalWorldShuffle_ValidateData(&result)) {
                SPDLOG_ERROR("Local world shuffle: structural validation failed before seed review");
                return false;
            }
            SPDLOG_INFO("Local world shuffle: seed {} rated {}", result.seed,
                        LocalWorldShuffle_RatingName(rating));
            return true;
        };
        const size_t requiredScore = required.size() + protectedItemCount;
        std::set<RandomizerCheck> lastChildReach, lastWorldReach;
        auto validate = [&]() {
            lastChildReach = searchWorld(true, false);
            lastWorldReach = searchWorld(false, false);
            size_t reached = 0;
            for (auto check : required) reached += lastWorldReach.count(check);
            for (size_t i = 0; i < checks.size(); ++i) {
                if (LocalWorldProtectedChildQuest(rewards[i])) reached += lastChildReach.count(checks[i]);
            }
            return reached;
        };
        auto canSwap = [&](size_t a, size_t b) {
            return canPlace(a, rewards[b]) && canPlace(b, rewards[a]);
        };
        auto swapRewards = [&](size_t a, size_t b) {
            std::swap(rewards[a], rewards[b]);
            simulation->GetItemLocation(checks[a])->SetPlacedItem(rewards[a]);
            simulation->GetItemLocation(checks[b])->SetPlacedItem(rewards[b]);
        };
        size_t reached = validate();
        // Repair conservative vanilla key-order failures without adding items or assuming access.
        while (reached < requiredScore) {
            std::vector<std::pair<size_t, size_t>> candidates;
            for (size_t a = 0; a < checks.size(); ++a) {
                const bool accessible = LocalWorldProtectedChildQuest(rewards[a]) ?
                    lastChildReach.count(checks[a]) != 0 : lastWorldReach.count(checks[a]) != 0;
                if (accessible || (LocalWorldCategory(rewards[a]) != LocalWorldItemCategory::Quest &&
                    !StaticData::RetrieveItem(rewards[a]).IsAdvancement())) {
                    continue;
                }
                for (size_t b = 0; b < checks.size(); ++b) {
                    if (rewards[a] != rewards[b] && canSwap(a, b) && lastWorldReach.count(checks[b])) {
                        candidates.emplace_back(a, b);
                    }
                }
            }
            std::shuffle(candidates.begin(), candidates.end(), random);
            bool improved = false;
            for (auto [a, b] : candidates) {
                swapRewards(a, b);
                const auto candidateReach = validate();
                if (candidateReach > reached) {
                    reached = candidateReach;
                    improved = true;
                    break;
                }
                swapRewards(a, b);
            }
            if (!improved) {
                SPDLOG_WARN("Local world shuffle: complete but unverified placement ({}/{})", reached, requiredScore);
                return storeSeed(LOCAL_WORLD_RATING_UNVERIFIED);
            }
        }
        // Build several valid candidates from the repaired layout. Major Quest items
        // get first choice of non-native destinations before the general mixing pass.
        const auto baseRewards = rewards;
        auto bestRewards = rewards;
        auto varietyScore = [&]() {
            std::pair<size_t, size_t> score{0, 0};
            for (size_t slot = 0; slot < checks.size(); ++slot) {
                if (rewards[slot] != originalRewards[slot]) {
                    ++score.second;
                    if (LocalWorldVarietyQuest(rewards[slot])) ++score.first;
                }
            }
            return score; // Prefer major moves, then total changed locations.
        };
        auto bestScore = varietyScore();
        auto applyRewards = [&](const std::vector<RandomizerGet>& layout) {
            rewards = layout;
            for (size_t slot = 0; slot < checks.size(); ++slot) {
                simulation->GetItemLocation(checks[slot])->SetPlacedItem(rewards[slot]);
            }
        };
        auto rememberBest = [&]() {
            const auto score = varietyScore();
            if (score > bestScore) {
                bestScore = score;
                bestRewards = rewards;
            }
        };
        std::vector<RandomizerGet> majorItems;
        for (auto item : originalRewards) {
            if (LocalWorldVarietyQuest(item) &&
                std::find(majorItems.begin(), majorItems.end(), item) == majorItems.end()) {
                majorItems.push_back(item);
            }
        }
        std::uniform_int_distribution<size_t> choose(0, checks.size() - 1);
        constexpr size_t candidateCount = 3;
        constexpr size_t majorAttempts = 8;
        size_t accepted = 0;
        for (size_t candidate = 0; candidate < candidateCount; ++candidate) {
            applyRewards(baseRewards);
            std::shuffle(majorItems.begin(), majorItems.end(), random);
            for (auto item : majorItems) {
                const auto current = std::find(rewards.begin(), rewards.end(), item);
                const size_t a = static_cast<size_t>(current - rewards.begin());
                std::vector<size_t> alternatives;
                for (size_t b = 0; b < checks.size(); ++b) {
                    if (a != b && rewards[b] != item && originalRewards[b] != item && canSwap(a, b)) {
                        alternatives.push_back(b);
                    }
                }
                std::shuffle(alternatives.begin(), alternatives.end(), random);
                const size_t previousMajorMoves = varietyScore().first;
                for (size_t attempt = 0; attempt < std::min(majorAttempts, alternatives.size()); ++attempt) {
                    const size_t b = alternatives[attempt];
                    swapRewards(a, b);
                    if (varietyScore().first >= previousMajorMoves && validate() == requiredScore) {
                        ++accepted;
                        rememberBest();
                        break;
                    }
                    swapRewards(a, b);
                }
            }
            // Keep the former total random-swap budget across the three candidates.
            for (size_t attempt = 0; attempt < checks.size() * 2; ++attempt) {
                size_t a = choose(random), b = choose(random);
                if (a == b || rewards[a] == rewards[b] || !canSwap(a, b)) continue;
                const size_t previousMajorMoves = varietyScore().first;
                swapRewards(a, b);
                // Never trade away major-item variety just to mix common items.
                // Every retained swap still receives full reachability validation.
                if (varietyScore().first < previousMajorMoves || validate() != requiredScore) {
                    swapRewards(a, b);
                } else {
                    ++accepted;
                    rememberBest();
                }
            }
        }
        applyRewards(bestRewards);
        SPDLOG_INFO("Local world shuffle: compared {} valid candidates; selected {} moved major Quest items, {} changed checks",
                    candidateCount, bestScore.first, bestScore.second);
        bool changed = false;
        for (size_t i = 0; i < checks.size(); ++i) {
            changed |= rewards[i] != LocalWorldNativeItem(*StaticData::GetLocation(checks[i]), result.npcPrizesEnabled != 0);
        }
        const auto finalScore = validate();
        if (finalScore != requiredScore) {
            return storeSeed(LOCAL_WORLD_RATING_UNVERIFIED);
        }
        if (!changed || accepted == 0) {
            SPDLOG_ERROR("Local world shuffle: final validation failed; changed {}, accepted {}, score {}/{}",
                         changed, accepted, finalScore, requiredScore);
            return false;
        }
        u8 rating = LOCAL_WORLD_RATING_UNVERIFIED;
        for (u8 profile : {LOCAL_WORLD_RATING_STANDARD, LOCAL_WORLD_RATING_TRICKY, LOCAL_WORLD_RATING_GLITCHES}) {
            LocalWorldSetReviewProfile(*simulation, profile);
            if (validate() == requiredScore) {
                rating = profile;
                break;
            }
        }
        SPDLOG_INFO("Local world shuffle: {} checks, {} accepted swaps, seed {}", checks.size(), accepted, result.seed);
        return storeSeed(rating);
    } catch (const std::exception& error) {
        SPDLOG_ERROR("Local world shuffle: generation failed: {}", error.what());
        return false;
    }
}
} // namespace Rando

namespace {
RandomizerCheck LocalWorldCheck(Actor* actor, PlayState* play, s16 params) {
    if (actor == nullptr || play == nullptr || !IS_LOCAL_WORLD_SHUFFLE || play->sceneNum == SCENE_GROTTOS) {
        return RC_UNKNOWN_CHECK;
    }
    if (actor->id == ACTOR_ITEM_ETCETERA && play->sceneNum == SCENE_LAKE_HYLIA) {
        if ((params & 0xFF) == 1) return RC_LH_UNDERWATER_ITEM;
        if ((params & 0xFF) == 7) return RC_LH_SUN;
        return RC_UNKNOWN_CHECK;
    }
    if (actor->id == ACTOR_EN_BOX && play->sceneNum == SCENE_GERUDOS_FORTRESS) {
        return RC_GF_CHEST; // Native child and adult chest parameters differ.
    }
    if (actor->id == ACTOR_EN_ITEM00) {
        params = static_cast<s16>(params & 0x7FFF); // Immediate-drop delivery flag is not location identity.
    }
    const auto range = Rando::StaticData::CheckFromActorMultimap.equal_range(
        std::make_tuple(static_cast<s16>(actor->id), static_cast<s16>(play->sceneNum), static_cast<s32>(params)));
    for (auto it = range.first; it != range.second; ++it) {
        const auto& location = *Rando::StaticData::GetLocation(it->second);
        if (LocalWorldNpcMinigame(location.GetRandomizerCheck())) continue;
        if (IsLocalWorldLocation(location, gSaveContext.ship.localWorldShuffle.forestSwordQuest != 0,
                                 gSaveContext.ship.localWorldShuffle.npcPrizesEnabled != 0) &&
            LocalWorldLocationMatchesLayout(location, gSaveContext.ship.localWorldShuffle.masterQuest != 0)) {
            return location.GetRandomizerCheck();
        }
    }
    return RC_UNKNOWN_CHECK;
}

LocalWorldShufflePlacement* LocalWorldPlacement(Actor* actor, PlayState* play, s16 params) {
    if (!IS_LOCAL_WORLD_SHUFFLE) return nullptr;
    const auto check = LocalWorldCheck(actor, play, params);
    auto& data = gSaveContext.ship.localWorldShuffle;
    if (check == RC_UNKNOWN_CHECK || data.state != LOCAL_WORLD_SHUFFLE_READY ||
        data.placementCount > LOCAL_WORLD_SHUFFLE_MAX_CHECKS) return nullptr;
    for (u16 i = 0; i < data.placementCount; ++i) {
        if (data.placements[i].check == check) return &data.placements[i];
    }
    return nullptr;
}
LocalWorldShufflePlacement* LocalWorldNpcPlacement(u16 check) {
    if (!IS_LOCAL_WORLD_SHUFFLE || !LocalWorldNpcMinigame(static_cast<RandomizerCheck>(check))) return nullptr;
    auto& data = gSaveContext.ship.localWorldShuffle;
    if (!data.npcPrizesEnabled || data.state != LOCAL_WORLD_SHUFFLE_READY ||
        data.placementCount > LOCAL_WORLD_SHUFFLE_MAX_CHECKS) return nullptr;
    for (u16 i = 0; i < data.placementCount; ++i) {
        if (data.placements[i].check == check) return &data.placements[i];
    }
    return nullptr;
}
} // namespace

extern "C" s32 LocalWorldShuffle_ValidateData(const LocalWorldShuffleSaveData* data) {
    if (data == nullptr || data->seedRating > LOCAL_WORLD_RATING_UNVERIFIED || data->npcPrizesEnabled > 1 || data->state != LOCAL_WORLD_SHUFFLE_READY || data->masterQuest > 1 || data->forestSwordQuest > 1 ||
        data->startingHearts < 1 || data->startingHearts > 5 || data->placementCount == 0 ||
        data->placementCount > LOCAL_WORLD_SHUFFLE_MAX_CHECKS) return false;
    if (data->npcPrizesEnabled) {
        std::array<u16, LOCAL_WORLD_NPC_REPEAT_PRIZE_COUNT> actual;
        std::copy(std::begin(data->npcRepeatPrizes), std::end(data->npcRepeatPrizes), actual.begin());
        auto expectedRepeats = localWorldNativeRepeatPrizes;
        if (actual.back() != GI_MILK) return false;
        std::sort(actual.begin(), actual.end());
        std::sort(expectedRepeats.begin(), expectedRepeats.end());
        if (actual != expectedRepeats) return false;
    }
    std::set<u16> expected, seen;
    std::map<u16, int> items;
    for (const auto& location : Rando::StaticData::GetLocationTable()) {
        if (IsLocalWorldLocation(location, data->forestSwordQuest != 0, data->npcPrizesEnabled != 0) && LocalWorldLocationMatchesLayout(location, data->masterQuest != 0)) {
            expected.insert(static_cast<u16>(location.GetRandomizerCheck()));
            ++items[static_cast<u16>(LocalWorldNativeItem(location, data->npcPrizesEnabled != 0))];
        }
    }
    if (expected.size() != data->placementCount) return false;
    for (u16 i = 0; i < data->placementCount; ++i) {
        const auto& placement = data->placements[i];
        if (!expected.count(placement.check) || !seen.insert(placement.check).second ||
            placement.item <= RG_NONE || placement.item >= RG_MAX || placement.collected > 1 ||
            placement.childAccessible > 1 || placement.adultAccessible > 1 || --items[placement.item] < 0 ||
            !LocalWorldPlacementAllowed(static_cast<RandomizerCheck>(placement.check),
                static_cast<RandomizerGet>(placement.item), placement.childAccessible != 0,
                placement.adultAccessible != 0)) return false;
    }
    return true;
}

namespace {
std::future<LocalWorldShuffleSaveData> localWorldGeneration;
}

extern "C" s32 LocalWorldShuffle_IsGenerating(void) {
    return localWorldGeneration.valid();
}

extern "C" s32 LocalWorldShuffle_Create(void) {
    // The temporary logic context must not overlap seed generation or a save worker.
    WaitForRandoGeneration();
    SaveManager::Instance->ThreadPoolWait();
    if (localWorldGeneration.valid()) return false;
    LocalWorldShuffleSaveData data{};
    data.npcPrizesEnabled = 1;
    data.masterQuest = !OTRGlobals::Instance->HasOriginal();
    data.forestSwordQuest = CVarGetInteger(CVAR_ENHANCEMENT("ForestSwordQuest"), 0) != 0;
    data.startingHearts = static_cast<u8>(std::clamp<int>(gSaveContext.healthCapacity / 0x10, 1, 5));
    data.seed = std::random_device{}();
    auto context = Rando::Context::GetInstance();
    if (!context) return false;
    try {
        localWorldGeneration = std::async(std::launch::async, [context, data]() mutable {
            data.state = context->GenerateLocalWorldShuffle(data) ? LOCAL_WORLD_SHUFFLE_READY
                                                                : LOCAL_WORLD_SHUFFLE_FAILED;
            return data;
        });
    } catch (const std::exception& error) {
        SPDLOG_ERROR("Could not start local world shuffle generation: {}", error.what());
        SohGui::RegisterPopup("Local Item Shuffle", "Could not start seed generation. No save file was created. Try again.");
        return false;
    }
    gSaveContext.ship.localDungeonShuffle = {};
    return true;
}

namespace {
enum SeedReviewChoice { REVIEW_INACTIVE, REVIEW_WAITING, REVIEW_YES, REVIEW_NO };
int localWorldReviewChoice = REVIEW_INACTIVE;
bool localWorldSeedApproved = false;
std::unique_ptr<SaveContext> localWorldBeforeReview;
std::unique_ptr<SaveContext> localWorldReviewedSave;
u16 localWorldTitleDayTime;
}

extern "C" const char* LocalWorldShuffle_RatingName(u8 rating) {
    switch (rating) {
        case LOCAL_WORLD_RATING_STANDARD: return "Standard";
        case LOCAL_WORLD_RATING_TRICKY: return "Tricky";
        case LOCAL_WORLD_RATING_GLITCHES: return "Glitches Required";
        case LOCAL_WORLD_RATING_UNVERIFIED: return "Unverified";
        default: return "Not rated";
    }
}

extern "C" void LocalWorldShuffle_CancelSeedReview(void) {
    localWorldReviewChoice = REVIEW_INACTIVE;
    localWorldSeedApproved = false;
    if (localWorldBeforeReview) gSaveContext = *localWorldBeforeReview;
    localWorldBeforeReview.reset();
    localWorldReviewedSave.reset();
}

extern "C" void LocalWorldShuffle_PrepareSeedReview(void) {
    LocalWorldShuffle_CancelSeedReview();
    localWorldBeforeReview = std::make_unique<SaveContext>(gSaveContext);
}

extern "C" void LocalWorldShuffle_BeginSeedReview(u16 titleDayTime) {
    localWorldReviewedSave = std::make_unique<SaveContext>(gSaveContext);
    localWorldTitleDayTime = titleDayTime;
    if (localWorldBeforeReview) gSaveContext = *localWorldBeforeReview;
    localWorldReviewChoice = REVIEW_INACTIVE;
    localWorldSeedApproved = false;
}

extern "C" s32 LocalWorldShuffle_PollGeneration(void) {
    if (!localWorldGeneration.valid() ||
        localWorldGeneration.wait_for(std::chrono::seconds(0)) != std::future_status::ready) return 0;
    try {
        const auto data = localWorldGeneration.get();
        if (localWorldReviewedSave && data.state == LOCAL_WORLD_SHUFFLE_READY) {
            localWorldReviewedSave->ship.localWorldShuffle = data;
            localWorldReviewChoice = REVIEW_WAITING;
            return 1;
        }
    } catch (const std::exception& error) {
        SPDLOG_ERROR("Local world shuffle generation failed: {}", error.what());
    }
    LocalWorldShuffle_CancelSeedReview();
    SohGui::RegisterPopup("Local Item Shuffle", "Could not create a valid item placement. No save file was created. Try again. See the log for details.");
    return -1;
}

extern "C" const char* LocalWorldShuffle_ReviewDescription(u8 rating) {
    switch (rating) {
        case LOCAL_WORLD_RATING_STANDARD: return "No tricks are required.";
        case LOCAL_WORLD_RATING_TRICKY: return "Advanced tricks may be required.";
        case LOCAL_WORLD_RATING_GLITCHES: return "Minor glitches may be required.";
        default: return "Warning: This seed may be unbeatable or require advanced glitches to finish.";
    }
}

extern "C" u32 LocalWorldShuffle_ReviewedSeedNumber(void) {
    return localWorldReviewedSave ? localWorldReviewedSave->ship.localWorldShuffle.seed : 0;
}

extern "C" u8 LocalWorldShuffle_ReviewedSeedRating(void) {
    return localWorldReviewedSave ? localWorldReviewedSave->ship.localWorldShuffle.seedRating : LOCAL_WORLD_RATING_UNKNOWN;
}

extern "C" s32 LocalWorldShuffle_HasReviewedSeed(void) {
    return localWorldReviewedSave && localWorldSeedApproved;
}

extern "C" void LocalWorldShuffle_ReopenSeedReview(void) {
    if (localWorldReviewedSave) localWorldReviewChoice = REVIEW_WAITING;
}

extern "C" void LocalWorldShuffle_ChooseSeedReview(s32 accept) {
    if (localWorldReviewChoice == REVIEW_WAITING) localWorldReviewChoice = accept ? REVIEW_YES : REVIEW_NO;
}

extern "C" s32 LocalWorldShuffle_SeedReviewPending(void) {
    return localWorldReviewChoice != REVIEW_INACTIVE;
}

extern "C" s32 LocalWorldShuffle_PollSeedReview(void) {
    const int choice = localWorldReviewChoice;
    if (choice != REVIEW_YES && choice != REVIEW_NO) return 0;
    if (choice == REVIEW_NO) {
        LocalWorldShuffle_CancelSeedReview();
        return 2;
    }
    if (!localWorldReviewedSave || localWorldReviewedSave->fileNum >= 3 ||
        SaveManager::Instance->SaveFile_Exist(localWorldReviewedSave->fileNum) ||
        !LocalWorldShuffle_ValidateData(&localWorldReviewedSave->ship.localWorldShuffle)) {
        LocalWorldShuffle_CancelSeedReview();
        SohGui::RegisterPopup("Local Item Shuffle", "This seed contains invalid item placements. No save file was created.");
        return 2;
    }
    localWorldSeedApproved = true;
    localWorldReviewChoice = REVIEW_INACTIVE;
    return 1;
}

extern "C" s32 LocalWorldShuffle_CommitReviewedSeed(const u8* playerName) {
    if (!LocalWorldShuffle_HasReviewedSeed() || playerName == nullptr ||
        localWorldReviewedSave->fileNum >= 3 ||
        SaveManager::Instance->SaveFile_Exist(localWorldReviewedSave->fileNum) ||
        !LocalWorldShuffle_ValidateData(&localWorldReviewedSave->ship.localWorldShuffle)) return false;
    // The name is entered after review; the exact accepted item placement remains unchanged.
    std::copy(playerName, playerName + 8, std::begin(localWorldReviewedSave->playerName));
    gSaveContext = *localWorldReviewedSave;
    localWorldBeforeReview.reset();
    localWorldReviewedSave.reset();
    localWorldReviewChoice = REVIEW_INACTIVE;
    localWorldSeedApproved = false;
    Save_SaveFile();
    SaveManager::Instance->ThreadPoolWait();
    gSaveContext.dayTime = localWorldTitleDayTime;
    return 1;
}

extern "C" s32 LocalWorldShuffle_NpcEnabled(void) {
    return IS_LOCAL_WORLD_SHUFFLE && gSaveContext.ship.localWorldShuffle.npcPrizesEnabled &&
           gSaveContext.ship.localWorldShuffle.state == LOCAL_WORLD_SHUFFLE_READY;
}
extern "C" s32 LocalWorldShuffle_NpcCollected(u16 check, s32 vanillaCollected) {
    auto* placement = LocalWorldNpcPlacement(check);
    return placement == nullptr ? vanillaCollected : placement->collected != 0;
}
extern "C" void LocalWorldShuffle_AcceptNpcPrize(Actor* actor, PlayState* play, u16 check) {
    auto* placement = LocalWorldNpcPlacement(check);
    if (placement != nullptr && actor != nullptr && play != nullptr &&
        play->sceneNum != SCENE_GROTTOS && actor->parent != nullptr) {
        placement->collected = 1; // Actor continues its native completion-flag path.
    }
}
extern "C" s32 LocalWorldShuffle_OfferNpcPrize(Actor* actor, PlayState* play, u16 check,
                                               s32 originalGetItemId, f32 xzRange, f32 yRange) {
    if (actor == nullptr || play == nullptr) return false;
    if (!LocalWorldShuffle_NpcEnabled() || play->sceneNum == SCENE_GROTTOS) {
        return Actor_OfferGetItem(actor, play, originalGetItemId, xzRange, yRange);
    }
    auto* placement = LocalWorldNpcPlacement(check);
    GetItemEntry entry;
    if (placement != nullptr && !placement->collected) {
        const auto item = static_cast<RandomizerGet>(placement->item);
        if (LocalWorldCategory(item) == LocalWorldItemCategory::Quest) return false;
        entry = LocalWorldShuffle_ResolveUpgrade(Rando::StaticData::RetrieveItem(item).GetGIEntry_Copy());
    } else {
        auto native = std::find(localWorldNativeRepeatPrizes.begin(), localWorldNativeRepeatPrizes.end(), originalGetItemId);
        if (native == localWorldNativeRepeatPrizes.end()) {
            if (placement == nullptr) return Actor_OfferGetItem(actor, play, originalGetItemId, xzRange, yRange);
            // A stale native flag must never award an already-collected unique prize again.
            native = localWorldNativeRepeatPrizes.begin() + 3; // Purple-rupee repeat slot.
        }
        const size_t index = static_cast<size_t>(native - localWorldNativeRepeatPrizes.begin());
        entry = ItemTableManager::Instance->RetrieveItemEntry(MOD_NONE,
            gSaveContext.ship.localWorldShuffle.npcRepeatPrizes[index]);
    }
    entry.getItemFrom = ITEM_FROM_NPC;
    // Preserve the native talk/cutscene/range checks, then substitute the entry.
    // The generic entry-offer bridge has different cutscene gating from this path.
    if (!Actor_OfferGetItem(actor, play, originalGetItemId, xzRange, yRange)) return false;
    auto* player = GET_PLAYER(play);
    player->getItemEntry = entry;
    player->getItemId = entry.getItemId;
    return true;
}

extern "C" s32 LocalWorldShuffle_CheckCollected(u16 check) {
    if (!IS_LOCAL_WORLD_SHUFFLE) return false;
    const auto& data = gSaveContext.ship.localWorldShuffle;
    for (u16 i = 0; i < data.placementCount && i < LOCAL_WORLD_SHUFFLE_MAX_CHECKS; ++i) {
        if (data.placements[i].check == check) return data.placements[i].collected != 0;
    }
    return false;
}
extern "C" s32 LocalWorldShuffle_HasCheck(Actor* actor, PlayState* play, s16 params) {
    return LocalWorldPlacement(actor, play, params) != nullptr;
}
extern "C" s32 LocalWorldShuffle_IsCollected(Actor* actor, PlayState* play, s16 params) {
    auto* placement = LocalWorldPlacement(actor, play, params);
    return placement != nullptr && placement->collected;
}
extern "C" void LocalWorldShuffle_Collect(Actor* actor, PlayState* play, s16 params) {
    auto* placement = LocalWorldPlacement(actor, play, params);
    if (placement != nullptr) {
        placement->collected = 1;
        const auto collection = Rando::StaticData::GetLocation(
            static_cast<RandomizerCheck>(placement->check))->GetCollectionCheck();
        if (collection.type == SPOILER_CHK_COLLECTABLE && collection.scene == play->sceneNum) {
            Flags_SetCollectible(play, collection.flag);
        }
        if (placement->check == RC_LH_UNDERWATER_ITEM) {
            Flags_SetEventChkInf(EVENTCHKINF_OBTAINED_RUTOS_LETTER);
            Flags_SetSwitch(play, 0xB);
        }
    }
    // Persist with the ordinary game save so inventory and collection flags roll back together.
}
extern "C" GetItemEntry LocalWorldShuffle_GetEntry(Actor* actor, PlayState* play, s16 params) {
    auto* placement = LocalWorldPlacement(actor, play, params);
    if (placement == nullptr) return GET_ITEM_NONE;
    auto entry = placement->item == RG_ICE_TRAP ? ItemTableManager::Instance->RetrieveItemEntry(MOD_NONE, GI_ICE_TRAP) :
        Rando::StaticData::RetrieveItem(static_cast<RandomizerGet>(placement->item)).GetGIEntry_Copy();
    if (placement->item == RG_ICE_TRAP) {
        // Vanilla deliberately leaves the ice-trap table entry empty. Preserve its
        // ID so the chest offer is negative/nonzero and selects opening, not grabbing.
        // The player then handles GI_ICE_TRAP through the ordinary freezing path.
        entry.getItemId = GI_ICE_TRAP;
    }
    entry = LocalWorldShuffle_ResolveUpgrade(entry);
    entry.getItemFrom = actor->id == ACTOR_EN_BOX ? ITEM_FROM_CHEST : ITEM_FROM_FREESTANDING;
    return entry;
}
extern "C" s32 LocalWorldShuffle_CanCollect(Actor* actor, PlayState* play, s16 params) {
    auto* placement = LocalWorldPlacement(actor, play, params);
    if (placement == nullptr) return true;
    auto item = static_cast<RandomizerGet>(placement->item);
    return LocalWorldItemAgeAllowed(item, LINK_IS_ADULT);
}
extern "C" s32 LocalWorldShuffle_OfferPickup(Actor* actor, PlayState* play, s16 params) {
    if (!LocalWorldShuffle_HasCheck(actor, play, params)) return false;
    if (LocalWorldShuffle_IsCollected(actor, play, params)) {
        Actor_Kill(actor);
    } else if (Actor_HasParent(actor, play)) {
        LocalWorldShuffle_Collect(actor, play, params);
        Actor_Kill(actor);
    } else if (LocalWorldShuffle_CanCollect(actor, play, params)) {
        GiveItemEntryFromActor(actor, play, LocalWorldShuffle_GetEntry(actor, play, params), 30.0f, 50.0f);
    }
    return true;
}
extern "C" s32 LocalWorldShuffle_DrawPickup(Actor* actor, PlayState* play, s16 params) {
    if (!LocalWorldShuffle_HasCheck(actor, play, params)) return false;
    func_8002EBCC(actor, play, 0);
    func_8002ED80(actor, play, 0);
    GetItemEntry_Draw(play, LocalWorldShuffle_GetEntry(actor, play, params));
    return true;
}
