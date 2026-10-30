#pragma once

#include <z64.h>

// Custom enhancement text IDs, separate from the randomizer shop messages.
#define TEXT_ROCS_FEATHER_SHOP_DESC 0x9420
#define TEXT_ROCS_FEATHER_SHOP_PROMPT 0x9421
#define TEXT_ROCS_FEATHER_SHOP_PICKUP 0x9422

uint8_t Enhancement_GetNextNayrusItem();
uint8_t Enhancement_GetPrevNayrusItem();
