#include "FileSelectEnhancements.h"

#include <ship/Context.h>

#include "soh/OTRGlobals.h"
#include "soh/ResourceManagerHelpers.h"
#include "soh/SohGui/SohModals.h"
#include "soh/SohGui/SohGui.hpp"
#include "soh/SohGui/UIWidgets.hpp"

#include <array>
#include <string>

std::array<std::string, LANGUAGE_MAX> RandomizerSettingsMenuText[RSM_MAX] = {
    {
        // English
        "Start Randomizer",
        // German
        "Randomizer starten",
        // French
        "Commencer le Randomizer",
    },
    {
        // English
        "Generate New Randomizer Seed",
        // German
        "Neuen Randomizer Seed generieren",
        // French
        "Générer une nouvelle seed pour le Randomizer",
    },
    {
        // English
        "Open Randomizer Settings",
        // German
        "Randomizer Optionen öffnen",
        // French
        "Ouvrir les paramètres du Randomizer",
    },
    {
        // English
        "Generating...",
        // German
        "Generiere...",
        // French
        "Génération en cours...",
    },
    { // English
      "No randomizer seed loaded.\nPlease generate one first"
#if defined(__WIIU__) || defined(__SWITCH__)
      ".",
#else
      ",\nor drop a spoiler log on the game window.",
#endif
      // German
      "Kein Randomizer Seed gefunden.\nBitte generiere zuerst einen"
#if defined(__WIIU__) || defined(__SWITCH__)
      ".",
#else
      ",\noder ziehe ein Spoiler Log\nauf das Spielfenster.",
#endif
      // French
      "Aucune Seed de Randomizer actuellement disponible.\nGénérez-en une dans les \"Randomizer Settings\""
#if (defined(__WIIU__) || defined(__SWITCH__))
      "."
#else
      "\nou glissez un spoilerlog sur la fenêtre du jeu."
#endif
    },
};

// Quests are hidden when their O2R is missing or their File Select "Hide" option is on
bool SohFileSelect_IsQuestHidden(u8 quest) {
    switch (quest) {
        case QUEST_NORMAL:
            return !ResourceMgr_GameHasOriginal() || CVarGetInteger(CVAR_ENHANCEMENT("FileSelect.HideNormalQuest"), 0);
        case QUEST_MASTER:
            return !ResourceMgr_GameHasMasterQuest() ||
                   CVarGetInteger(CVAR_ENHANCEMENT("FileSelect.HideMasterQuest"), 0);
        case QUEST_RANDOMIZER:
            return CVarGetInteger(CVAR_ENHANCEMENT("FileSelect.HideRandomizerQuest"), 0);
        case QUEST_BOSSRUSH:
            return CVarGetInteger(CVAR_ENHANCEMENT("FileSelect.HideBossRushQuest"), 0);
        default:
            return false;
    }
}

u8 SohFileSelect_CountVisibleQuests() {
    u8 count = 0;
    for (u8 quest = QUEST_NORMAL; quest <= QUEST_LOCAL_WORLD_SHUFFLE; ++quest) {
        if (!SohFileSelect_IsQuestHidden(quest)) {
            count++;
        }
    }

    return count;
}

const char* SohFileSelect_GetSettingText(uint8_t optionIndex, uint8_t language) {
    return RandomizerSettingsMenuText[optionIndex][language].c_str();
}

void SohFileSelect_ShowPresetMenu() {
    SohGui::ShowEscMenu();
    CVarSetString(CVAR_SETTING("Menu.ActiveHeader"), "Settings");
    CVarSetString(CVAR_SETTING("Menu.SettingsSidebarSection"), "Presets");
    CVarSetInteger(CVAR_GENERAL("HasSeenPresetModal"), 1);
}

void SohFileSelect_DismissPresetModal() {
    CVarSetInteger(CVAR_GENERAL("HasSeenPresetModal"), 1);
}

void SohFileSelect_ShowPresetModal() {
    if (CVarGetInteger(CVAR_GENERAL("HasSeenPresetModal"), 0)) {
        return;
    }
    std::shared_ptr<SohModalWindow> modal = static_pointer_cast<SohModalWindow>(
        Ship::Context::GetRawInstance()->GetWindow()->GetGui()->GetGuiWindow("Modal Window"));
    if (modal->IsPopupOpen("Choose a Preset")) {
        modal->DismissPopup();
    } else {
        modal->RegisterPopup("Choose a Preset",
                             "Start with one of Mo's presets, then customize settings in the menu.",
                             "View Presets", "Not Now", SohFileSelect_ShowPresetMenu,
                             SohFileSelect_DismissPresetModal);
    }
}
