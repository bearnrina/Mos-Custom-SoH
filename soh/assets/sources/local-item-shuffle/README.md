# Local Item Shuffle subtitle

`LocalItemShuffleSubtitle.png` is the original transparent artwork generated from the user-approved title-screen mockup. It contains only the gold `LOCAL ITEM SHUFFLE` wordmark.

The production resource is `soh/assets/custom/objects/object_mag/gTitleLocalItemShuffleSubtitleTex`:

- Texture resource version 1, RGBA32, `TEX_FLAG_LOAD_AS_RAW`.
- Original dimensions: 2172 x 724 pixels. Pixels and alpha are preserved without resampling.
- Logical tile: 128 x 32, with horizontal byte scale 2172/128 and vertical pixel scale 724/32.
- File-select rendering uses a 128 x 43 rectangle to retain the original artwork aspect ratio.

The standard-resolution companion resource, `soh/assets/custom/objects/object_mag/gTitleLocalItemShuffleSubtitleStandardTex`, contains a 128 x 43 RGBA32 reduction of the original artwork, with a 128 x 32 logical tile. File select checks the resolved Zelda shield-logo texture: a width above the native 160 pixels selects the HD subtitle; the native logo selects the standard subtitle. This follows the displayed artwork even when alternate assets remain enabled with Reloaded disabled. Both use the same placement and alpha.

The normal `GenerateSohOtr` build target packs these resources into `soh.o2r`. The Release directory must receive the updated archive along with `soh.exe`.

## Artwork generation

Tool: built-in image generator. Reference: the user-approved quest-logo mockup.

Final prompt:

> Extract ONLY the gold LOCAL ITEM SHUFFLE lettering from this approved game title mockup, preserving its exact font, letters, gold metallic highlights, black outline and shadow. Remove the sky, panel, all Zelda logos, sword, shield, footer, arrows, and every other element. Output a genuinely transparent PNG containing only the single line of text LOCAL ITEM SHUFFLE. This is a production game subtitle texture. Tight horizontal framing, as little transparent padding as possible, no second line, no added icon, no redesign. The lettering should be identical to the reference. Prefer a wide canvas with the wordmark filling almost its entire width. Clean antialiased edges and readable gold serif letters.
