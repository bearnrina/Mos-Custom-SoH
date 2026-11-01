# Mo's Custom SoH

**A custom build of Ship of Harkinian for Windows.**

Mo's Custom SoH builds on [Ship of Harkinian](https://www.shipofharkinian.com), the PC port of *The Legend of Zelda: Ocarina of Time*, with additional gameplay, equipment, economy, and interface options. This is an independent fan project, **not an official Harbour Masters release**.

> **Release status:** v1.0 source preparation. Public binaries, bundled mods, and release dates have not yet been finalized.

## Features

The custom development build includes the following modifications. Individual settings and availability should be checked against the final release before publication.

### Gameplay and difficulty

- Adjustable ordinary-enemy health, with separate boss and miniboss adjustments reserved for future development.
- Adjustable day/night cycle.
- Adjustable enemy/boss speed options.
- Durable Deku Sticks setting.
- Optional death penalties for rupees, ammunition, and consumables.
- Option to suppress magic-jar drops.
- Separate child and adult area music settings.
- Adjustable healing and magic item effects.

### Items, shops, and economy

- Roc's Feather, Bombchu Bag upgrades, Child's Wallet, Tycoon Wallet, Skeleton Key, and Fishing Rod integrated into the game's shops or item progression.
- Bank Hylia deposit and withdrawal system.
- Local dungeon-item shuffle option. Complete shuffle mode integration reserved for future development.

### Controls and interface

- C-Up item equipping and configurable D-pad item controls, with mutually exclusive selection.
- Custom Navi D-pad HUD placement controls.
- Additional HUD customization and Modifier 2 HUD toggle.
- Arrow signs now have hit boxes and strikes produce sparks.

## Requirements and legal game assets

**This project does not supply a commercial game ROM or Nintendo's copyrighted game assets.** You must provide a legally acquired, supported copy of *The Legend of Zelda: Ocarina of Time* to use Ship of Harkinian. See the [ROM compatibility checker](https://ship.equipment/) and [supported hashes](docs/supportedHashes.json).

Do not upload or distribute ROM files, generated `oot.o2r`/`oot-mq.o2r` game archives, saves, or other extracted Nintendo assets as part of this repository or its releases.

## Installing and running

**Windows is the currently tested development platform.** Public installation instructions will be added once the release package and its contents have been finalized. Do not use the upstream Ship of Harkinian download links below as though they were Mo's Custom SoH releases.

For information on the original project, visit [Ship of Harkinian](https://www.shipofharkinian.com).

## Controls

The original Ship of Harkinian default keyboard layout is:

| N64 | A | B | Z | Start | Analog stick | C buttons | D-pad |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Keyboard | X | C | Z | Space | WASD | Arrow keys | TFGH |

Common shortcuts include `Esc` (menu), `F11` (fullscreen), and `Ctrl+R` (reset). Custom controls and HUD options may change how C-Up and the D-pad behave; consult the in-game settings and tooltips.

## Source and development

This repository is based on [Harbour Masters' Shipwright / Ship of Harkinian](https://github.com/HarbourMasters/Shipwright), which uses [libultraship](https://github.com/Kenix3/libultraship/) and work from the *Ocarina of Time* decompilation project. The upstream [build instructions](docs/BUILDING.md) are provided for reference; they have not yet been independently validated for this customized public source package.

Custom assets for Ship of Harkinian are generally packaged as `.otr`/`.o2r` archives and placed in the `mods` directory. Only redistribute third-party mods or assets when their licenses or creators permit it. The three credited GameBanana modifications have been authorized for redistribution by their original authors.

## Credits and third-party notices

- **Mo:** Custom build direction and modifications.
- **Harbour Masters and Ship of Harkinian contributors:** Original PC port and associated systems.
- **libultraship contributors:** Supporting platform library.
- **Ocarina of Time decompilation contributors:** Foundational reverse-engineering work.
- **Third-party mod authors:** The following three modifications are included with redistribution permission from their original authors:
  - **CGWolf95** — [HD Roc's Feather](https://gamebanana.com/mods/662992), custom Roc's Feather visual asset.
  - **Neirn** — Original MM Jumps plugin author; **denoflions** — credited contributor. [MM Jump Flips](https://gamebanana.com/mods/688588) is submitted on GameBanana by **VioLink** as redistributor.
  - **CGWolf95** — [Fairy Talk Icon (OoT)](https://gamebanana.com/mods/614571), custom fairy interaction icon.

These assets remain the work of their original creators. Inclusion does not imply that they endorse Mo's Custom SoH. The permission to redistribute these three assets does not extend to unrelated third-party content.

Retain all existing copyright notices, license files, and attribution documents from upstream and third-party components. See [upstream credits](docs/CREDITS.md).

## Planned features
PLANNED FEATURES V 1.1
Adjustable drop percentage- enhancements- difficulty- drops
Adjustable hover boot duration & slide
Adjustable iron boots run/sink speed
Assign equipment to d-pad
Make speed modifier affect climbing and block pushing
Make quick swimming faster
Add 'give all' option to general, equipment, quest items, & dungeon items in save editor
Stone of agony allows talking to gossip stones
Quick plant magic beans
Clock on pause menu
Void fall causes instant death
Add deku nut tree drops
Fix random enemy sizes
Adjustable bunny hood speed
Add night only option to gaurd vision
Condense all menu text and pop-up text
Skippable cutscenes

PLANNED FEATURES V 1.2 and beyond
Map Mania- Buyable maps- all 34 regions maps are buyable
Compassanity- Stone of agony functions as map compass (all in-game compasses are replaced by perishable items)
Adjustable enemy drop rate/selection
Adjustable enemy detection range
Adjustable grass/pot/etc drop rate/selection
Add hint options from randomizer
Dash strike attack
Change Jabu Jabu's exterior hit sound from stone to flesh
Universal day/night cycle
Fish anywhere (turns off bottle catches)
Improve horse mechanics
Restore lake hylia and zoras domain after water temple
Gaurds patrol hyrule castle field
Slot Skins- toggle between multiple alternate assets for items, weapons & equipment in start menu

SHUFFLE MODE (enhancements - extra modes)
Items
Local Dungeon Item Shuffle- shuffle all items within a given dungeon
Global Dungeon Item Shuffle- shuffle all c-items between dungeons
Combine Compass/Map- combine the map/compass pickup into one, thus freeing a chest
World Item Shuffle- shuffle all items world items (no dungeons, grottos, etc.)
-
Enemies
Local Dungeon Enemy Shuffle
Global Dungeon Enemy Shuffle
World Enemy Shuffle
Boss Shuffle
-
Dungeons
Local Dungeon Room Shuffle
-
Locations
Local Town Entrance Shuffle- shuffle all entrances within a given town
Hyrule Field Entrance Shuffle- shuffle all entrances within hyrule field (excludes castle town and lon lon ranch)

GLITCHES & SPEEDRUNNING TECHNIQUES
Easy bomb sliding
Easy supersliding
Easy megaflip
Easy bomb hovering
Easy ISG
Easy seam walking
Easy door of time skip
Easy gainer

UPGRADED SYSTEMS
Features		Upgrades
Song of storms		water bean soil
Plant beans in soil	add different seeds/plants that produce different effects in the future.
Happy masks		extended quest for other 4
Useless chests		add treasure system to be sold at the treasure shop
Fishing guild		collect fishes/loaches
Gerudo guild		collect bounties

BUILD FORKS
Gregforce Quest- nearly complete
3D Castletown- rebuild temple of time courtyard and castletown into 3d enviroments
Castle Stealth Sequences- add 2 more castle stealth sequences; 1 for each stone
Light Temple- brand new dungeon

Future feature ideas and changes are not guarantees of inclusion.

## Disclaimer

This is an unofficial fan modification. It is not affiliated with, endorsed by, or sponsored by Nintendo or Harbour Masters. *The Legend of Zelda* and *Ocarina of Time* are trademarks of their respective owners.
