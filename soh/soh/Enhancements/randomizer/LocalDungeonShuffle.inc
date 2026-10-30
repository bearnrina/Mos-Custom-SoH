// Included only by SeedContext.cpp. No independent compilation unit is needed.
#include "LocalDungeonShuffle.h"
#include "soh/ResourceManagerHelpers.h"
#include <ship/Context.h>
#include <fstream>
#include "location_access.h"
#include "3drando/fill.hpp"
#include <functional>
#include <random>
#include <utility>
#include <set>

extern "C" void Save_SaveFile(void);

namespace {
// Swap the members individually. Copying Region would invalidate the pointers
// between the entrance lists, including any loaded entrance-randomizer seed.
void SwapLocalShuffleRegion(Region& a, Region& b) {
    using std::swap;
    swap(a.regionName, b.regionName);
    swap(a.scene, b.scene);
    swap(a.timePass, b.timePass);
    swap(a.areas, b.areas);
    swap(a.events, b.events);
    swap(a.locations, b.locations);
    swap(a.exits, b.exits);
    swap(a.entrances, b.entrances);
    swap(a.childDay, b.childDay);
    swap(a.childNight, b.childNight);
    swap(a.adultDay, b.adultDay);
    swap(a.adultNight, b.adultNight);
    swap(a.addedToPool, b.addedToPool);
    swap(a.randomizerRegionKey, b.randomizerRegionKey);
}

struct LocalShuffleRestore {
    std::function<void()> restore;
    ~LocalShuffleRestore() { restore(); }
};

struct LocalShuffleSlot {
    RandomizerCheck check;
    RandomizerGet logicItem;
    u8 scene;
    u8 flag;
    u8 getItemId;
};

bool IsLocalShuffleChest(const Rando::Location& location) {
    return location.GetRandomizerCheck() != RC_UNKNOWN_CHECK &&
           location.GetActorID() == ACTOR_EN_BOX &&
           location.GetCollectionCheck().type == SPOILER_CHK_CHEST &&
           location.GetArea() >= RCAREA_DEKU_TREE && location.GetArea() <= RCAREA_GANONS_CASTLE;
}
} // namespace

namespace Rando {
bool Context::GenerateLocalDungeonShuffle(LocalDungeonShuffleSaveData& result) {
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
    LocalShuffleRestore restore{[&]() {
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
        // Explicit vanilla gates. Unspecified options and every trick are off.
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
            if (ResourceMgr_IsSceneMasterQuest(dungeon->GetScene())) {
                dungeon->SetMQ();
            } else {
                dungeon->ClearMQ();
            }
        }
        RegionTable_Init();
        std::array<std::vector<LocalShuffleSlot>, 12> groups;
        std::vector<RandomizerCheck> required;
        std::set<std::pair<u8, u8>> identities;
        for (const auto& location : StaticData::GetLocationTable()) {
            if (location.GetRandomizerCheck() == RC_UNKNOWN_CHECK) {
                continue;
            }
            if (location.GetQuest() != RCQUEST_BOTH) {
                auto* dungeon = simulation->GetDungeonFromScene(location.GetScene());
                if (dungeon == nullptr || (location.GetQuest() == RCQUEST_MQ) != dungeon->IsMQ()) {
                    continue;
                }
            }
            auto check = location.GetRandomizerCheck();
            simulation->GetItemLocation(check)->SetPlacedItem(location.GetVanillaItem());
            if (!IsLocalShuffleChest(location)) {
                continue;
            }
            const auto collection = location.GetCollectionCheck();
            const auto getItemId = (static_cast<u16>(location.GetActorParams()) >> 5) & 0x7F;
            if (getItemId == GI_NONE || getItemId >= GI_MAX || collection.flag > 31 ||
                !identities.emplace(collection.scene, static_cast<u8>(collection.flag)).second) {
                SPDLOG_ERROR("Local dungeon shuffle: invalid or duplicate chest {}", location.GetName());
                return false;
            }
            groups[location.GetArea() - RCAREA_DEKU_TREE].push_back({
                check, location.GetVanillaItem(), collection.scene, static_cast<u8>(collection.flag),
                static_cast<u8>(getItemId)
            });
            required.push_back(check);
        }
        if (required.empty() || required.size() > LOCAL_DUNGEON_SHUFFLE_MAX_CHESTS) {
            SPDLOG_ERROR("Local dungeon shuffle: invalid chest count {}", required.size());
            return false;
        }
        // Include boss access, not merely access to the chest containing its key.
        for (auto boss : StaticData::dungeonRewardLocations) {
            required.push_back(boss);
        }
        auto validate = [&]() {
            simulation->GetLogic()->Reset();
            Regions::AccessReset();
            simulation->LocationReset();
            GetAccessibleLocationsStruct search(0);
            search.haveTimeAccess = true;
            search.foundTempleOfTime = true;
            search.validatedStartingRegion = true;
            search.sphereZeroComplete = true;
            search.timePassChildDay = search.timePassChildNight = true;
            search.timePassAdultDay = search.timePassAdultNight = true;
            do {
                search.InitLoop();
                for (size_t i = 0; i < search.regionPool.size(); ++i) {
                    ProcessRegion(RegionTable(search.regionPool[i]), search);
                }
            } while (search.logicUpdated);
            size_t reached = 0;
            for (auto check : required) {
                reached += simulation->GetItemLocation(check)->IsAddedToPool() ? 1 : 0;
            }
            return reached;
        };
        size_t reached = validate();
        const bool baselineValid = reached == required.size();
        // A normal save can fail before the first shuffle. Record the exact
        // inaccessible checks and inventory so this is diagnosable without a debugger.
        try {
            std::ofstream report(Ship::Context::GetPathRelativeToAppDirectory("LocalDungeonShuffle-Diagnostics.txt"));
            report << "Local Dungeon Item Shuffle — vanilla baseline diagnostic\n";
            report << "Seed: " << result.seed << "\nStarting hearts: " << static_cast<int>(result.startingHearts)
                   << "\nBaseline valid: " << baselineValid << "\nRequired checks: " << required.size() << "\n\n";
            report << "Required check access (0 = blocked, 1 = reachable):\n";
            for (auto check : required) {
                report << simulation->GetItemLocation(check)->IsAddedToPool() << "  "
                       << StaticData::GetLocation(check)->GetName() << "\n";
            }
            report << "\nProgression inventory (0 = missing, 1 = owned):\n";
            const std::pair<const char*, RandomizerGet> items[] = {
                {"Kokiri Sword", RG_KOKIRI_SWORD}, {"Deku Shield", RG_DEKU_SHIELD},
                {"Slingshot", RG_FAIRY_SLINGSHOT}, {"Bomb Bag", RG_BOMB_BAG},
                {"Boomerang", RG_BOOMERANG}, {"Fairy Ocarina", RG_FAIRY_OCARINA},
                {"Ocarina of Time", RG_OCARINA_OF_TIME}, {"Song of Time", RG_SONG_OF_TIME},
                {"Zelda's Lullaby", RG_ZELDAS_LULLABY}, {"Master Sword", RG_MASTER_SWORD},
                {"Hookshot", RG_HOOKSHOT}, {"Longshot", RG_LONGSHOT}, {"Bow", RG_FAIRY_BOW},
                {"Hammer", RG_MEGATON_HAMMER}, {"Lens of Truth", RG_LENS_OF_TRUTH},
                {"Iron Boots", RG_IRON_BOOTS}, {"Hover Boots", RG_HOVER_BOOTS},
                {"Goron's Bracelet", RG_GORONS_BRACELET}, {"Silver Gauntlets", RG_SILVER_GAUNTLETS},
                {"Golden Gauntlets", RG_GOLDEN_GAUNTLETS}, {"Mirror Shield", RG_MIRROR_SHIELD},
                {"Light Arrows", RG_LIGHT_ARROWS}, {"Magic", RG_MAGIC_SINGLE},
                {"Can Open Chests", RG_OPEN_CHEST}, {"Can Climb", RG_CLIMB},
                {"Can Crawl", RG_CRAWL}, {"Can Swim", RG_BRONZE_SCALE}
            };
            for (const auto& item : items) {
                report << item.first << ": " << simulation->GetLogic()->HasItem(item.second) << "\n";
            }
            report << "\nDungeon layouts and logical small-key totals:\n";
            for (auto* dungeon : simulation->GetDungeons()->GetDungeonList()) {
                report << dungeon->GetName() << ": MQ=" << dungeon->IsMQ() << ", keys="
                       << static_cast<int>(simulation->GetLogic()->GetSmallKeyCount(dungeon->GetScene())) << "\n";
            }
        } catch (const std::exception& error) {
            SPDLOG_ERROR("Local dungeon shuffle: could not write diagnostic report: {}", error.what());
        }
        std::mt19937 random(result.seed);
        size_t repairs = 0;
        // The shared search can conservatively reject vanilla key ordering.
        // Build a starting solution instead of requiring vanilla to be one.
        // Move an unreachable progression reward to an already-reachable junk,
        // map or compass chest in the SAME dungeon; keep it only if reachability
        // increases. No extra keys, unlocked doors or assumed dungeon items.
        while (reached < required.size()) {
            std::set<RandomizerCheck> reachableChecks;
            for (const auto& group : groups) {
                for (const auto& slot : group) {
                    if (simulation->GetItemLocation(slot.check)->IsAddedToPool()) {
                        reachableChecks.insert(slot.check);
                    }
                }
            }
            struct RepairPair { size_t group; size_t donor; size_t receiver; };
            std::vector<RepairPair> candidates;
            for (size_t d = 0; d < groups.size(); ++d) {
                const auto& group = groups[d];
                for (size_t donor = 0; donor < group.size(); ++donor) {
                    if (reachableChecks.count(group[donor].check) ||
                        !StaticData::RetrieveItem(group[donor].logicItem).IsAdvancement()) {
                        continue;
                    }
                    for (size_t receiver = 0; receiver < group.size(); ++receiver) {
                        const auto& item = StaticData::RetrieveItem(group[receiver].logicItem);
                        const bool nonProgression = !item.IsAdvancement() || item.GetItemType() == ITEMTYPE_MAP ||
                                                    item.GetItemType() == ITEMTYPE_COMPASS;
                        if (reachableChecks.count(group[receiver].check) && nonProgression) {
                            candidates.push_back({d, donor, receiver});
                        }
                    }
                }
            }
            std::shuffle(candidates.begin(), candidates.end(), random);
            bool improved = false;
            for (const auto& pair : candidates) {
                auto& a = groups[pair.group][pair.donor];
                auto& b = groups[pair.group][pair.receiver];
                std::swap(a.logicItem, b.logicItem);
                std::swap(a.getItemId, b.getItemId);
                simulation->GetItemLocation(a.check)->SetPlacedItem(a.logicItem);
                simulation->GetItemLocation(b.check)->SetPlacedItem(b.logicItem);
                const size_t candidateReach = validate();
                if (candidateReach > reached) {
                    reached = candidateReach;
                    ++repairs;
                    improved = true;
                    break;
                }
                std::swap(a.logicItem, b.logicItem);
                std::swap(a.getItemId, b.getItemId);
                simulation->GetItemLocation(a.check)->SetPlacedItem(a.logicItem);
                simulation->GetItemLocation(b.check)->SetPlacedItem(b.logicItem);
            }
            if (!improved) {
                SPDLOG_ERROR("Local dungeon shuffle: unable to build a valid initial placement ({}/{} reachable)",
                             reached, required.size());
                try {
                    std::ofstream report(Ship::Context::GetPathRelativeToAppDirectory("LocalDungeonShuffle-Diagnostics.txt"),
                                         std::ios::app);
                    report << "\nInitial placement search failed: " << reached << "/" << required.size()
                           << " required checks reachable. No rewards changed.\n";
                } catch (const std::exception&) {}
                return false;
            }
            // validate() left the successful arrangement's access state in place.
        }
        SPDLOG_INFO("Local dungeon shuffle: valid initial placement after {} repairs", repairs);
        size_t accepted = 0;
        // Random swaps start from the proven vanilla solution. Each swap keeps
        // the exact multiset and is retained only if the resulting world passes.
        // Junk-to-junk swaps cannot affect progression and need no new search.
        for (auto& group : groups) {
            if (group.size() < 2) {
                continue;
            }
            std::uniform_int_distribution<size_t> choose(0, group.size() - 1);
            for (size_t attempt = 0; attempt < group.size() * 6; ++attempt) {
                const size_t a = choose(random);
                const size_t b = choose(random);
                if (a == b || group[a].getItemId == group[b].getItemId) {
                    continue;
                }
                const bool affectsProgression = StaticData::RetrieveItem(group[a].logicItem).IsAdvancement() ||
                                                StaticData::RetrieveItem(group[b].logicItem).IsAdvancement();
                std::swap(group[a].logicItem, group[b].logicItem);
                std::swap(group[a].getItemId, group[b].getItemId);
                simulation->GetItemLocation(group[a].check)->SetPlacedItem(group[a].logicItem);
                simulation->GetItemLocation(group[b].check)->SetPlacedItem(group[b].logicItem);
                if (affectsProgression && validate() != required.size()) {
                    std::swap(group[a].logicItem, group[b].logicItem);
                    std::swap(group[a].getItemId, group[b].getItemId);
                    simulation->GetItemLocation(group[a].check)->SetPlacedItem(group[a].logicItem);
                    simulation->GetItemLocation(group[b].check)->SetPlacedItem(group[b].logicItem);
                } else {
                    ++accepted;
                }
            }
        }
        if (validate() != required.size()) {
            SPDLOG_ERROR("Local dungeon shuffle: final validation failed; no rewards changed");
            return false;
        }
        result.chestCount = 0;
        for (const auto& group : groups) {
            for (const auto& slot : group) {
                result.chests[result.chestCount++] = {slot.scene, slot.flag, slot.getItemId};
            }
        }
        SPDLOG_INFO("Local dungeon shuffle: {} chests, {} accepted swaps, seed {}", result.chestCount, accepted,
                    result.seed);
        try {
            std::ofstream report(Ship::Context::GetPathRelativeToAppDirectory("LocalDungeonShuffle-Diagnostics.txt"),
                                 std::ios::app);
            report << "\nGeneration succeeded. Initial repairs: " << repairs << ", accepted random swaps: " << accepted
                   << ", stored chests: " << result.chestCount << ". Final progression validation passed.\n";
        } catch (const std::exception&) {}
        return true;
    } catch (const std::exception& error) {
        SPDLOG_ERROR("Local dungeon shuffle: generation failed: {}", error.what());
        return false;
    }
}
} // namespace Rando

extern "C" s16 LocalDungeonShuffle_GetItemId(s16 scene, u8 treasureFlag, s16 vanillaItemId) {
    if (IS_RANDO) {
        return vanillaItemId;
    }
    auto& data = gSaveContext.ship.localDungeonShuffle;
    const bool beganPending = data.state == LOCAL_DUNGEON_SHUFFLE_PENDING;
    if (beganPending) {
        // Wait for the first dungeon chest: the new file's quest/layout is now final.
        bool dungeonChest = false;
        for (const auto& location : Rando::StaticData::GetLocationTable()) {
            const auto collection = location.GetCollectionCheck();
            if (IsLocalShuffleChest(location) && collection.scene == scene && collection.flag == treasureFlag) {
                dungeonChest = true;
                break;
            }
        }
        if (!dungeonChest) {
            return vanillaItemId;
        }
        auto context = Rando::Context::GetInstance();
        data.state = context && context->GenerateLocalDungeonShuffle(data) ?
                     LOCAL_DUNGEON_SHUFFLE_READY : LOCAL_DUNGEON_SHUFFLE_FAILED;
        if (data.state != LOCAL_DUNGEON_SHUFFLE_READY) {
            data.chestCount = 0;
        }
        // Persist the complete table before any chest can open. Never reroll READY files.
        if (gSaveContext.fileNum < 3) {
            Save_SaveFile();
        }
    }
    if (!beganPending && data.state == LOCAL_DUNGEON_SHUFFLE_FAILED) {
        // For a previously failed test file, run diagnostics once per seed per
        // process using a copy. Never convert opened vanilla chests retroactively.
        static std::set<u32> reportedSeeds;
        bool dungeonChest = false;
        for (const auto& location : Rando::StaticData::GetLocationTable()) {
            const auto collection = location.GetCollectionCheck();
            if (IsLocalShuffleChest(location) && collection.scene == scene && collection.flag == treasureFlag) {
                dungeonChest = true;
                break;
            }
        }
        if (dungeonChest && reportedSeeds.insert(data.seed).second) {
            if (auto context = Rando::Context::GetInstance()) {
                auto diagnosticCopy = data;
                context->GenerateLocalDungeonShuffle(diagnosticCopy);
            }
        }
    }
    if (data.state == LOCAL_DUNGEON_SHUFFLE_READY) {
        for (u16 i = 0; i < data.chestCount; ++i) {
            if (data.chests[i].scene == scene && data.chests[i].treasureFlag == treasureFlag) {
                return data.chests[i].getItemId;
            }
        }
    }
    return vanillaItemId;
}
