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

## Project scope

GREGFORCE Quest is being developed separately and is **not part of this v1.0 release**. Future feature ideas and changes are not guarantees of inclusion.

## Disclaimer

This is an unofficial fan modification. It is not affiliated with, endorsed by, or sponsored by Nintendo or Harbour Masters. *The Legend of Zelda* and *Ocarina of Time* are trademarks of their respective owners.
