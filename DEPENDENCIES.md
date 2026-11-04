# Customized dependency source

This release includes libultraship and Torch as ordinary source directories, not upstream Git submodule links. Keep the included trees when building: replacing them with upstream checkouts may discard custom behavior.

The previous public repository referenced these upstream revisions:

| Component | Upstream | Previous public submodule revision |
| --- | --- | --- |
| libultraship | https://github.com/kenix3/libultraship | 62e973aeb4a53ad4d22bb91e2d9373ecdfcd246c |
| Torch | https://github.com/HarbourMasters/Torch | 2ab12fe9660aec04e02ee89fe81baed304a1a1d6 |

The included trees are the customized sources used by this development build; the revisions above identify the former public references, not an assertion that the customized trees are unchanged at those revisions. Existing component license and attribution files are retained.