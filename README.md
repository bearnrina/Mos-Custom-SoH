# Mo's Custom SoH

**A custom build of Ship of Harkinian for Windows — v1.1.**

Mo's Custom SoH builds on [Ship of Harkinian](https://www.shipofharkinian.com), the PC port of *The Legend of Zelda: Ocarina of Time*, with additional gameplay, equipment, economy, control, and interface options. This is an independent fan project, **not an official Harbour Masters release**.

Mo's custom release is **v1.1**. The underlying engine retains its upstream **9.2.3** version metadata. Windows is the tested platform.

## Links

These optional texture packs and assets are not bundled and require separate download. They are not required to play the build. Download and place them in the mods folder if used.

- [Ship of Harkinian](https://www.shipofharkinian.com)
- [Upstream Shipwright source](https://github.com/HarbourMasters/Shipwright)
- [libultraship](https://github.com/Kenix3/libultraship/)
- [ROM compatibility checker](https://ship.equipment/)
- [Supported ROM hashes](https://github.com/HarbourMasters/Shipwright/blob/develop/docs/supportedHashes.json)
- [OoT Reloaded — downloads and installation instructions](https://evilgames.eu/texture-packs/oot-reloaded.htm)
- [HD Roc's Feather](https://gamebanana.com/mods/662992)
- [HD Navi Icon (OoT)](https://gamebanana.com/mods/614571)
- [Alternate Skies](https://gamebanana.com/mods/579269)
- [Alternate Rupees](https://gamebanana.com/mods/694120)
- [MM Jump Flip Animation](https://gamebanana.com/mods/688588)

## Features

### Gameplay and difficulty

- Adjustable ordinary-enemy health; separate boss and miniboss health adjustments remain future work.
- Adjustable day and night lengths, plus enemy and boss speed options.
- Speed modifiers that also affect climbing, block pushing, and crawling.
- Faster swimming boosts and randomized enemy-size options.
- Durable Deku Sticks, optional death penalties for rupees/ammunition/consumables, and suppression of magic-jar drops.
- Adjustable healing and magic-item effects.
- Separate child and adult area music settings.
- Existing Ship of Harkinian enhancements, fixes, and cutscene-skipping options.

### Items, shops, and economy

- Roc's Feather, Bombchu Bag upgrades, Child's Wallet, Tycoon Wallet, Skeleton Key, and Fishing Rod integrated into shops or item progression.
- Bank Hylia deposits and withdrawals.
- Stone of Agony access to gossip-stone hints, with revised introductory dialogue.
- Save-editor Give All controls for general resources, inventory, equipment, quest items, and dungeon items.

### Local Item Shuffle

Local Item Shuffle is selected as a **game mode** from the quest-selection screen. It is no longer a checkable enhancement under Extra Modes.

The mode uses quest and settings screens styled to match the existing game modes. Its gold subtitle switches resolution to match the displayed Zelda logo.

- **Start Local Item Shuffle:** begin with an accepted seed, then enter a file name.
- **Generate New Seed:** prepare a new placement for review.
- **Review Seed:** review an accepted candidate again before starting.

Start and Review are unavailable until a seed has been accepted. Generating and reviewing a candidate does not create a save file; finishing name entry creates the accepted file.

#### Seed ratings

Every rating asks **“Do you want to continue?”** and offers **Yes / No**. Yes accepts the candidate. No discards it and returns to the Local Item Shuffle menu with Generate New Seed selected. Pressing B during review also returns to that menu.

| Rating | Review message | Completion fanfare |
| --- | --- | --- |
| Standard | No tricks are required. | Obtain Heart Container |
| Tricky | Advanced tricks may be required. | Obtain Item |
| Glitches Required | Minor glitches may be required. | Enter Zelda |
| Unverified | Warning: This seed may be unbeatable or require advanced glitches to finish. | Obtain Small Item |

**Open Treasure Chest** plays during generation. Generation runs in the background, with menu input blocked until it finishes. Failed generation uses an error sound and does not create a save.

#### Upgrades and age restrictions

Upgrade rewards advance sequentially within their families, regardless of which source is found first:

- Silver Scale → Golden Scale.
- Goron's Bracelet → Silver Gauntlets → Golden Gauntlets.
- Adult's Wallet → Giant's Wallet.
- Hookshot → Longshot.
- Bow/quiver, slingshot/bullet-bag, and bomb-bag capacity tiers.
- Deku Stick and Deku Nut capacity upgrades, plus shuffled magic-meter upgrades.

Item previews, awarded rewards, and seed validation use the progressive rules. Existing saved placements remain intact; already-owned inventory is not changed retroactively.

Some shuffled chests remain restricted to child or adult Link. Trying to open one at the wrong age displays an explanation:

- Child Link: “The chest is sealed with an old, rusty lock...”
- Adult Link: “The lid of the chest is rusted shut...”

### Mo's Custom Controls

One checkbox combines C-Up item equipping, D-pad equipment controls, and relocated Navi/first-person controls.

- Equip items to C-Up.
- Change equipment in-game with the D-pad.
- Access Navi and first-person view through D-pad Up.
- Use the custom Navi icon when its mod is installed, or standard localized Navi text when it is absent.

Consult the in-game tooltips for control combinations and HUD placement. The separate D-pad item-equipping option remains mutually exclusive with Mo's Custom Controls.

### Interface, animations, and signs

- Custom Navi D-pad HUD placement, additional HUD customization, and a Modifier 2 HUD toggle.
- Majora's Mask running-jump flips through optional animation assets.
- Arrow signs with hitboxes, hit sparks, and spin-attack knockdown behavior through the standalone arrow-sign mod.
- Revised custom-setting menu labels, tooltips, and pop-up wording.

### Mo's built-in presets

The three curated enhancement presets are built into the game and refreshed for v1.1:

- **Mo's OG+ (recommended for veterans)**
- **Mo's Nostalgia+ (newcomers start here)**
- **Mo's Vanilla+ (for purists only)**

Find them under **Settings → Presets**. The built-in versions use the author's updated, gameplay-tested selections.

## Requirements and legal game assets

**This project does not supply a commercial ROM or extracted Nintendo game-data archive.** Provide your own legally acquired, supported copy of *The Legend of Zelda: Ocarina of Time*. Use the compatibility links above to check your version.

Do not distribute ROM files, generated `oot.o2r` / `oot-mq.o2r` archives, saves, personal configurations, or other extracted Nintendo assets with this project.

## Installing and running

1. Close the game and back up existing saves and configuration before updating.
2. Extract the **Mo's Custom SoH Windows x64** download into a new writable folder.
3. Supply your own supported `oot.o2r` or `oot-mq.o2r`, or launch `soh.exe` and follow the extractor prompts using your own supported ROM. The supplied `assets` folder contains extraction descriptions, not extracted game data.
4. Launch `soh.exe`, choose a quest, and apply a curated preset if desired.

The Windows download contains the executable, custom engine archive `soh.o2r`, extraction descriptions, notices, and standalone arrow-sign mod. It excludes ROMs, generated game-data archives, saves, personal settings, temporary presets, and the three optional animation/icon/feather mods.

When migrating an existing installation, keep your backup until you have verified your saves and settings in the updated version. Upstream Ship of Harkinian downloads are separate from Mo's Custom SoH downloads.

## Optional assets and texture packs

Place compatible `.o2r` / `.otr` mod archives in `mods`, then restart after adding or removing them.

| Feature | Without its optional assets | With the assets installed |
| --- | --- | --- |
| Mo's Custom Controls | Works with standard Navi text. | Automatically uses the custom Navi icon. |
| Majora's Mask Jump Flips | Menu option is visible but unclickable; normal jumps remain. | Menu option becomes selectable. Both jump/landing headers and animation data are required. |
| Roc's Feather | Works with fallback artwork. | Uses the optional HD artwork. |
| Breakable arrow signs | Custom sign assets are unavailable. | Keep `mods/BreakableArrowSigns.o2r` installed for the custom assets. |

The old combined `MoCustomAssets.o2r` is not required for the controls or Roc's Feather. The Windows release supplies arrow signs as their own archive instead.

[OoT Reloaded](https://evilgames.eu/texture-packs/oot-reloaded.htm) is an optional texture pack by GhostlyDark. Follow its Ship of Harkinian installation instructions. It is not included in this download.

## Controls

The original Ship of Harkinian default keyboard layout is:

| N64 | A | B | Z | Start | Analog stick | C buttons | D-pad |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Keyboard | X | C | Z | Space | WASD | Arrow keys | TFGH |

Common shortcuts: `Esc` opens the menu, `F11` toggles fullscreen, and `Ctrl+R` resets. Custom controls and HUD options can change how C-Up and the D-pad behave; consult the in-game settings and tooltips.

## Source and development

This project builds on [Harbour Masters' Shipwright](https://github.com/HarbourMasters/Shipwright), [libultraship](https://github.com/Kenix3/libultraship/), and the *Ocarina of Time* decompilation project.

The source download includes the current customized libultraship and Torch source trees without Git history. Keep these included trees rather than replacing them with upstream submodule checkouts. The source package includes `SOURCE-BUILD.md` and the upstream `docs/BUILDING.md` for toolchain guidance.

The current Release executable has compiled successfully and has been gameplay-tested by the author. The exported source has not received a separate clean build; the author also confirmed that the packaged Windows download passed its fresh-folder test. SHA-256 checksums and file manifests accompany the release packages.

## Credits and third-party notices

- **Mo:** Custom build direction, modifications, preset curation, and gameplay testing.
- **Harbour Masters and Ship of Harkinian contributors:** Original PC port and associated systems.
- **libultraship contributors:** Supporting platform library.
- **Ocarina of Time decompilation contributors:** Foundational reverse-engineering work.
- **GhostlyDark and OoT Reloaded contributors:** Optional texture pack. [OoT Reloaded](https://evilgames.eu/texture-packs/oot-reloaded.htm) 
- **CGWolf95:**  Optional HD textures [HD Roc's Feather](https://gamebanana.com/mods/662992) and [Fairy Talk Icon (OoT)](https://gamebanana.com/mods/614571).
- **Neirn**, **VioLink:**  Optional animation [MM Jump Flips](https://gamebanana.com/mods/688588)

The three optional GameBanana modifications are **not bundled in v1.1**. They remain the work of their original creators. 

The Local Item Shuffle subtitle artwork was generated for this project. Its source and generation notes are in `soh/assets/sources/local-item-shuffle` in the source package.

Retain all existing copyright notices, licenses, and attribution documents from upstream and third-party components. See `docs/CREDITS.md` in the source package and `CREDITS.md` / `licenses` in the Windows package.

## Planned features

The following roadmap preserves earlier feature ideas. Completed work is described above; remaining ideas are not promises of inclusion or release dates.

v1.3 Gregforce Quest Integration
- Integrate Greg the green rupee quest into version 1.2

v1.4 Integrate Lesser Features
- Song of storms waters plants
- Easy supersliding
- Easy superslide stearing
- Super backflip ends slide

v1.5 Drop System & Difficulty Integration
- Adjustable enemy drop rate/selection
- Adjustable grass/pot/etc drop rate/selection
- Adjustable enemy detection range

v1.6 Map Mania Integration
- Map Mania- Buyable maps- all 34 regions maps are buyable
- Compassanity- Stone of agony functions as map compass (all in-game compasses are replaced by perishable items)

v1.7 Shuffle Mode Integration
- Add enemies to shuffle mode
- Add global options to shuffle mode

v1.8 Integrate Cutscenes & Restores
- Restore lake hylia and zoras domain after water temple 
- Desaturate lost woods and restore it when optional 3rd party assets are installed
- Skippable cutscenes

v1.9 Integrate Slot Skins
- Slot Skins- toggle between multiple alternate asset items, weapons & equipment in start menu

v2.0 Side Quest
- Hylian loach hunt- add 5 loaches around Hyrule catchable in bottles

v2.1 Side Quest
- Expanded happy mask quest- add quest for other 4 masks

v2.2 Advanced Features
- Extend gaurd vision to castle field and add night only option
- Gaurds patrol hyrule castle field

## Disclaimer

This is an unofficial fan modification. It is not affiliated with, endorsed by, or sponsored by Nintendo or Harbour Masters. *The Legend of Zelda* and *Ocarina of Time* are trademarks of their respective owners.
