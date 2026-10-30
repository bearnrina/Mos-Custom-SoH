#include "global.h"
#include "vt.h"
#include "soh/cvar_prefixes.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include "objects/gameplay_keep/gameplay_keep.h"
#include <objects/object_d_hsblock/object_d_hsblock.h>

#define FLAGS ACTOR_FLAG_UPDATE_CULLING_DISABLED

// Keep the existing EnAObj layout intact; this actor allocation owns the extra state.
#define ARROW_SIGN_BOARD_PARAM 0xFE

typedef struct {
    EnAObj base;
    u8 broken;
    u8 boardState; // 0: falling, 1: resting, 2: repairing
    u8 ocarinaFlag;
    u8 bladeHistoryValid;
    u8 swordHitLatched;
    s16 bladeAnimation;
    f32 bladeFrame;
    Vec3f previousBladeBase;
    Vec3f previousBladeTip;
} EnAObjArrowSign;

static const ALIGN_ASSET(2) char sArrowBoardDL[] = "__OTR__custom/arrow_sign/boardDL";
static const ALIGN_ASSET(2) char sArrowPostDL[] = "__OTR__custom/arrow_sign/postDL";

static void EnAObj_ArrowBoard(EnAObj* this, PlayState* play);
static void EnAObj_RepairArrowSign(EnAObj* this, PlayState* play);


void EnAObj_Init(Actor* thisx, PlayState* play);
void EnAObj_Destroy(Actor* thisx, PlayState* play);
void EnAObj_Update(Actor* thisx, PlayState* play);
void EnAObj_Draw(Actor* thisx, PlayState* play);

void EnAObj_WaitFinishedTalking(EnAObj* this, PlayState* play);
void EnAObj_WaitTalk(EnAObj* this, PlayState* play);
void EnAObj_BlockRot(EnAObj* this, PlayState* play);
void EnAObj_BoulderFragment(EnAObj* this, PlayState* play);
void EnAObj_Block(EnAObj* this, PlayState* play);

void EnAObj_SetupWaitTalk(EnAObj* this, s16 type);
void EnAObj_SetupBlockRot(EnAObj* this, s16 type);
void EnAObj_SetupBoulderFragment(EnAObj* this, s16 type);
void EnAObj_SetupBlock(EnAObj* this, s16 type);

const ActorInit En_A_Obj_InitVars = {
    ACTOR_EN_A_OBJ,
    ACTORCAT_PROP,
    FLAGS,
    OBJECT_GAMEPLAY_KEEP,
    sizeof(EnAObjArrowSign),
    (ActorFunc)EnAObj_Init,
    (ActorFunc)EnAObj_Destroy,
    (ActorFunc)EnAObj_Update,
    (ActorFunc)EnAObj_Draw,
    NULL,
};

static ColliderCylinderInit sCylinderInit = {
    {
        COLTYPE_NONE,
        AT_NONE,
        AC_ON | AC_TYPE_ALL,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_2,
        COLSHAPE_CYLINDER,
    },
    {
        ELEMTYPE_UNK2,
        { 0x00000000, 0x00, 0x00 },
        { 0xFFCFFFFF, 0x00, 0x00 },
        TOUCH_NONE,
        BUMP_ON,
        OCELEM_ON,
    },
    { 25, 60, 0, { 0, 0, 0 } },
};

// extern CollisionHeader D_06000730; // gHookshotTargetCol ?

static CollisionHeader* sColHeaders[] = {
    &gLargerCubeCol,       // A_OBJ_GRASS_CLUMP, A_OBJ_TREE_STUMP
    &gLargerCubeCol,       // A_OBJ_BLOCK_LARGE, A_OBJ_BLOCK_HUGE
    &gSmallerFlatBlockCol, // unused
    &gLargerFlatBlockCol,  // A_OBJ_BLOCK_SMALL_ROT, A_OBJ_BLOCK_LARGE_ROT
    &gSmallerCubeCol,      // unused
    //&D_06000730,           // A_OBJ_UNKNOWN_6 // OTRTODO
};

static Gfx* sDLists[] = {
    gFlatBlockDL,
    gFlatBlockDL,
    gFlatBlockDL,
    gFlatRotBlockDL,
    gFlatRotBlockDL,
    gSmallCubeDL,
    gHookshotPostDL,
    /* gHookshotPostDL ? */ // 0x06000210, // OTRTODO!
    gGrassBladesDL,
    gTreeStumpDL,
    gSignRectangularDL,
    gSignDirectionalDL,
    gBoulderFragmentsDL,
};

void EnAObj_SetupAction(EnAObj* this, EnAObjActionFunc actionFunc) {
    this->actionFunc = actionFunc;
}

void EnAObj_Init(Actor* thisx, PlayState* play) {
    CollisionHeader* colHeader = NULL;
    s32 pad;
    EnAObj* this = (EnAObj*)thisx;
    f32 shadowScale = 6.0f;

    this->textId = (thisx->params >> 8) & 0xFF;
    thisx->params &= 0xFF;
    ((EnAObjArrowSign*)this)->broken = 0;
    ((EnAObjArrowSign*)this)->boardState = 0;
    ((EnAObjArrowSign*)this)->ocarinaFlag = 0;
    ((EnAObjArrowSign*)this)->bladeHistoryValid = false;
    ((EnAObjArrowSign*)this)->swordHitLatched = false;

    switch (thisx->params) {
        case A_OBJ_BLOCK_SMALL:
            Actor_SetScale(thisx, 0.025f);
            break;
        case A_OBJ_BLOCK_LARGE:
            Actor_SetScale(thisx, 0.05f);
            break;
        case A_OBJ_BLOCK_HUGE:
        case A_OBJ_CUBE_SMALL:
        case A_OBJ_UNKNOWN_6:
            Actor_SetScale(thisx, 0.1f);
            break;
        case A_OBJ_BLOCK_SMALL_ROT:
            Actor_SetScale(thisx, 0.005f);
            break;
        case A_OBJ_BLOCK_LARGE_ROT:
        default:
            Actor_SetScale(thisx, 0.01f);
            break;
    }

    if (thisx->params >= A_OBJ_SIGNPOST_OBLONG) {
        shadowScale = 12.0f;
    }

    ActorShape_Init(&thisx->shape, 0.0f, ActorShadow_DrawCircle, shadowScale);

    thisx->focus.pos = thisx->world.pos;
    this->dyna.bgId = BGACTOR_NEG_ONE;
    this->dyna.interactFlags = 0;
    this->dyna.transformFlags = DPM_UNK;
    thisx->uncullZoneDownward = 1200.0f;
    thisx->uncullZoneScale = 200.0f;

    switch (thisx->params) {
        case A_OBJ_BLOCK_LARGE:
        case A_OBJ_BLOCK_HUGE:
            this->dyna.bgId = 1;
            Actor_ChangeCategory(play, &play->actorCtx, thisx, ACTORCAT_BG);
            EnAObj_SetupBlock(this, thisx->params);
            break;
        case A_OBJ_BLOCK_SMALL_ROT:
        case A_OBJ_BLOCK_LARGE_ROT:
            this->dyna.bgId = 3;
            Actor_ChangeCategory(play, &play->actorCtx, thisx, ACTORCAT_BG);
            EnAObj_SetupBlockRot(this, thisx->params);
            break;
        case A_OBJ_UNKNOWN_6:
            // clang-format off
            thisx->flags |= ACTOR_FLAG_ATTENTION_ENABLED; this->dyna.bgId = 5; this->focusYoffset = 10.0f;
            // clang-format on
            thisx->gravity = -2.0f;
            EnAObj_SetupWaitTalk(this, thisx->params);
            break;
        case A_OBJ_GRASS_CLUMP:
        case A_OBJ_TREE_STUMP:
            this->dyna.bgId = 0;
            EnAObj_SetupWaitTalk(this, thisx->params);
            break;
        case A_OBJ_SIGNPOST_OBLONG:
        case A_OBJ_SIGNPOST_ARROW:
            thisx->textId = (this->textId & 0xFF) | 0x300;
            // clang-format off
            thisx->flags |= ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY; thisx->targetArrowOffset = 500.0f;
            // clang-format on
            this->focusYoffset = 45.0f;
            EnAObj_SetupWaitTalk(this, thisx->params);
            Collider_InitCylinder(play, &this->collider);
            Collider_SetCylinder(play, &this->collider, thisx, &sCylinderInit);
            thisx->colChkInfo.mass = MASS_IMMOVABLE;
            thisx->targetMode = 0;
            if (thisx->params == A_OBJ_SIGNPOST_ARROW) {
                thisx->flags |= ACTOR_FLAG_UPDATE_DURING_OCARINA;
            }
            break;
        case ARROW_SIGN_BOARD_PARAM:
            thisx->flags |= ACTOR_FLAG_UPDATE_DURING_OCARINA;
            thisx->gravity = -1.0f;
            thisx->shape.shadowScale = 8.0f;
            EnAObj_SetupAction(this, EnAObj_ArrowBoard);
            break;
        case A_OBJ_BOULDER_FRAGMENT:
            thisx->gravity = -1.5f;
            EnAObj_SetupBoulderFragment(this, thisx->params);
            break;
        default:
            thisx->gravity = -2.0f;
            EnAObj_SetupWaitTalk(this, thisx->params);
            break;
    }

    if (thisx->params <= A_OBJ_BLOCK_LARGE_ROT) { // A_OBJ_BLOCK_*
        thisx->colChkInfo.mass = MASS_IMMOVABLE;
    }

    if (this->dyna.bgId != BGACTOR_NEG_ONE) {
        CollisionHeader_GetVirtual(sColHeaders[this->dyna.bgId], &colHeader);
        this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, thisx, colHeader);
    }
}

void EnAObj_Destroy(Actor* thisx, PlayState* play) {
    EnAObj* this = (EnAObj*)thisx;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);

    switch (this->dyna.actor.params) {
        case A_OBJ_SIGNPOST_OBLONG:
        case A_OBJ_SIGNPOST_ARROW:
            Collider_DestroyCylinder(play, &this->collider);
            break;
    }
}

void EnAObj_WaitFinishedTalking(EnAObj* this, PlayState* play) {
    if (Actor_TextboxIsClosing(&this->dyna.actor, play)) {
        EnAObj_SetupWaitTalk(this, this->dyna.actor.params);
    }
}

void EnAObj_SetupWaitTalk(EnAObj* this, s16 type) {
    EnAObj_SetupAction(this, EnAObj_WaitTalk);
}

void EnAObj_WaitTalk(EnAObj* this, PlayState* play) {
    s16 relYawTowardsPlayer;

    if (this->dyna.actor.textId != 0) {
        relYawTowardsPlayer = this->dyna.actor.yawTowardsPlayer - this->dyna.actor.shape.rot.y;
        if (ABS(relYawTowardsPlayer) < 0x2800 ||
            (this->dyna.actor.params == A_OBJ_SIGNPOST_ARROW && ABS(relYawTowardsPlayer) > 0x5800)) {
            if (Actor_ProcessTalkRequest(&this->dyna.actor, play)) {
                EnAObj_SetupAction(this, EnAObj_WaitFinishedTalking);
            } else {
                Actor_OfferTalkNearColChkInfoCylinder(&this->dyna.actor, play);
            }
        }
    }
}

void EnAObj_SetupBlockRot(EnAObj* this, s16 type) {
    this->rotateState = 0;
    this->rotateWaitTimer = 10;
    this->dyna.actor.world.rot.y = 0;
    this->dyna.actor.shape.rot = this->dyna.actor.world.rot;
    EnAObj_SetupAction(this, EnAObj_BlockRot);
}

void EnAObj_BlockRot(EnAObj* this, PlayState* play) {
    if (this->rotateState == 0) {
        if (this->dyna.interactFlags != 0) {
            this->rotateState++;
            this->rotateForTimer = 20;

            if ((s16)(this->dyna.actor.yawTowardsPlayer + 0x4000) < 0) {
                this->rotSpeedX = -0x3E8;
            } else {
                this->rotSpeedX = 0x3E8;
            }

            if (this->dyna.actor.yawTowardsPlayer < 0) {
                this->rotSpeedY = -this->rotSpeedX;
            } else {
                this->rotSpeedY = this->rotSpeedX;
            }
        }
    } else {
        if (this->rotateWaitTimer != 0) {
            this->rotateWaitTimer--;
        } else {
            this->dyna.actor.shape.rot.y += this->rotSpeedY;
            this->dyna.actor.shape.rot.x += this->rotSpeedX;
            this->rotateForTimer--;
            this->dyna.actor.gravity = -1.0f;

            if (this->rotateForTimer == 0) {
                this->dyna.actor.world.pos = this->dyna.actor.home.pos;
                this->rotateState = 0;
                this->rotateWaitTimer = 10;
                this->dyna.actor.velocity.y = 0.0f;
                this->dyna.actor.gravity = 0.0f;
                this->dyna.actor.shape.rot = this->dyna.actor.world.rot;
            }
        }
    }
}

void EnAObj_SetupBoulderFragment(EnAObj* this, s16 type) {
    EnAObj_SetupAction(this, EnAObj_BoulderFragment);
}

void EnAObj_BoulderFragment(EnAObj* this, PlayState* play) {
    Math_SmoothStepToF(&this->dyna.actor.speedXZ, 1.0f, 1.0f, 0.5f, 0.0f);
    this->dyna.actor.shape.rot.x += this->dyna.actor.world.rot.x >> 1;
    this->dyna.actor.shape.rot.z += this->dyna.actor.world.rot.z >> 1;

    if (this->dyna.actor.speedXZ != 0.0f && this->dyna.actor.bgCheckFlags & 0x8) {
        this->dyna.actor.world.rot.y =
            this->dyna.actor.wallYaw - this->dyna.actor.world.rot.y + this->dyna.actor.wallYaw - 0x8000;
        this->dyna.actor.bgCheckFlags &= ~0x8;
    }

    if (this->dyna.actor.bgCheckFlags & 0x2) {
        if (this->dyna.actor.velocity.y < -8.0f) {
            this->dyna.actor.velocity.y *= -0.6f;
            this->dyna.actor.speedXZ *= 0.6f;
            this->dyna.actor.bgCheckFlags &= ~0x3;
        } else {
            Actor_Kill(&this->dyna.actor);
        }
    }
}

void EnAObj_SetupBlock(EnAObj* this, s16 type) {
    this->dyna.actor.uncullZoneDownward = 1200.0f;
    this->dyna.actor.uncullZoneScale = 720.0f;
    EnAObj_SetupAction(this, EnAObj_Block);
}

void EnAObj_Block(EnAObj* this, PlayState* play) {
    this->dyna.actor.speedXZ += this->dyna.unk_150;
    this->dyna.actor.world.rot.y = this->dyna.unk_158;
    this->dyna.actor.speedXZ = CLAMP(this->dyna.actor.speedXZ, -2.5f, 2.5f);

    Math_SmoothStepToF(&this->dyna.actor.speedXZ, 0.0f, 1.0f, 1.0f, 0.0f);

    if (this->dyna.actor.speedXZ != 0.0f) {
        Audio_PlayActorSound2(&this->dyna.actor, NA_SE_EV_ROCK_SLIDE - SFX_FLAG);
    }

    this->dyna.unk_154 = 0.0f;
    this->dyna.unk_150 = 0.0f;
}

// Reattach the original board and restore its readable, targetable state.
static void EnAObj_RepairArrowSign(EnAObj* this, PlayState* play) {
    Actor* actor = &this->dyna.actor;
    ((EnAObjArrowSign*)this)->broken = 0;
    actor->flags |= ACTOR_FLAG_ATTENTION_ENABLED;
    this->collider.dim.radius = sCylinderInit.dim.radius;
    this->collider.dim.height = sCylinderInit.dim.height;
    this->collider.base.acFlags &= ~AC_HIT;
    EnAObj_SetupWaitTalk(this, actor->params);
}

static void EnAObj_ArrowBoard(EnAObj* this, PlayState* play) {
    EnAObjArrowSign* board = (EnAObjArrowSign*)this;
    Actor* actor = &this->dyna.actor;
    Actor* parent = actor->parent;

    if (parent == NULL || parent->update == NULL) {
        Actor_Kill(actor);
        return;
    }

    // Disabling the option also repairs signs already broken in this area.
    if (!CVarGetInteger(CVAR_ENHANCEMENT("BreakableArrowSigns"), 0)) {
        EnAObj_RepairArrowSign((EnAObj*)parent, play);
        Actor_Kill(actor);
        return;
    }

    // Match the existing square sign's successful-song detection.
    if (play->msgCtx.ocarinaMode == OCARINA_MODE_01) {
        board->ocarinaFlag = 1;
    } else if (board->ocarinaFlag && play->msgCtx.ocarinaMode == OCARINA_MODE_04) {
        board->ocarinaFlag = 0;
        if (play->msgCtx.unk_E3F2 == OCARINA_SONG_LULLABY && board->boardState != 2) {
            board->boardState = 2;
            actor->gravity = 0.0f;
            actor->velocity.y = 0.0f;
            actor->speedXZ = 0.0f;
            Audio_PlayActorSound2(parent, NA_SE_SY_TRE_BOX_APPEAR);
        }
    }

    if (board->boardState == 2) {
        Math_SmoothStepToF(&actor->world.pos.x, parent->world.pos.x, 1.0f, 3.0f, 0.0f);
        Math_SmoothStepToF(&actor->world.pos.y, parent->world.pos.y + 40.0f, 1.0f, 3.0f, 0.0f);
        Math_SmoothStepToF(&actor->world.pos.z, parent->world.pos.z, 1.0f, 3.0f, 0.0f);
        Math_SmoothStepToS(&actor->shape.rot.x, parent->shape.rot.x, 1, 0x800, 0);
        Math_SmoothStepToS(&actor->shape.rot.y, parent->shape.rot.y, 1, 0x800, 0);
        Math_SmoothStepToS(&actor->shape.rot.z, parent->shape.rot.z, 1, 0x800, 0);
        if (fabsf(actor->world.pos.x - parent->world.pos.x) < 0.1f &&
            fabsf(actor->world.pos.y - parent->world.pos.y - 40.0f) < 0.1f &&
            fabsf(actor->world.pos.z - parent->world.pos.z) < 0.1f &&
            actor->shape.rot.x == parent->shape.rot.x &&
            actor->shape.rot.y == parent->shape.rot.y &&
            actor->shape.rot.z == parent->shape.rot.z) {
            EnAObj_RepairArrowSign((EnAObj*)parent, play);
            Actor_Kill(actor);
        }
    } else if (board->boardState == 0) {
        if (actor->bgCheckFlags & BGCHECKFLAG_GROUND) {
            board->boardState = 1;
            actor->gravity = 0.0f;
            actor->velocity.y = 0.0f;
            actor->speedXZ = 0.0f;
            actor->world.pos.y = actor->floorHeight + 2.0f;
            actor->shape.rot.x = 0x4000;
            actor->shape.rot.z = 0;
        } else {
            actor->shape.rot.x += 0x1000;
            actor->shape.rot.z += 0x600;
            if (actor->bgCheckFlags & BGCHECKFLAG_WALL) {
                actor->speedXZ = 0.0f;
            }
        }
    }
}

// Sword contact uses the model's narrow volumes, independently of the wide
// cylinder used for walking and talking. Units below match the normal 0.01 scale.
typedef struct {
    Vec3f normal;
    f32 distance;
} ArrowSignPlane;

typedef struct {
    Vec3f pos;
    f32 time;
} ArrowSignClipVertex;

// Convex envelope of the original tapered, triangular wooden post.
static const ArrowSignPlane sArrowPolePlanes[] = {
    { { 0.0f, -1.0f, 0.0f }, 0.0f },
    { { 0.0f, 1.0f, 0.0f }, 52.0f },
    { { 0.845488780f, 0.000539127f, -0.533992914f }, 1.244203489f },
    { { -0.845488780f, 0.000539127f, -0.533992914f }, 1.244203489f },
    { { 0.759872022f, 0.011374531f, -0.649973176f }, 1.514437500f },
    { { -0.759872022f, 0.011374531f, -0.649973176f }, 1.514437500f },
    { { 0.0f, 0.016343970f, 0.999866428f }, 1.849752893f },
};

// Exact pentagonal board outline and its two faces (four units thick).
static const ArrowSignPlane sArrowBoardPlanes[] = {
    { { -1.0f, 0.0f, 0.0f }, 16.0f },
    { { 0.0f, -1.0f, 0.0f }, -32.0f },
    { { 0.0f, 1.0f, 0.0f }, 48.0f },
    { { 0.0f, 0.0f, -1.0f }, 2.0f },
    { { 0.0f, 0.0f, 1.0f }, 2.0f },
    { { 8.0f, 11.85f, 0.0f }, 692.48f },
    { { 8.0f, -11.85f, 0.0f }, -255.52f },
};

static Vec3f EnAObj_ArrowToLocal(Actor* actor, Vec3f* world) {
    Vec3f p;
    f32 a;
    f32 b;
    f32 sine;
    f32 cosine;

    p.x = world->x - actor->world.pos.x;
    p.y = world->y - actor->world.pos.y;
    p.z = world->z - actor->world.pos.z;
    sine = Math_SinS(actor->shape.rot.z);
    cosine = Math_CosS(actor->shape.rot.z);
    a = p.x;
    b = p.y;
    p.x = cosine * a + sine * b;
    p.y = -sine * a + cosine * b;
    sine = Math_SinS(actor->shape.rot.y);
    cosine = Math_CosS(actor->shape.rot.y);
    a = p.x;
    b = p.z;
    p.x = cosine * a - sine * b;
    p.z = sine * a + cosine * b;
    sine = Math_SinS(actor->shape.rot.x);
    cosine = Math_CosS(actor->shape.rot.x);
    a = p.y;
    b = p.z;
    p.y = cosine * a + sine * b;
    p.z = -sine * a + cosine * b;
    p.x /= actor->scale.x * 100.0f;
    p.y /= actor->scale.y * 100.0f;
    p.z /= actor->scale.z * 100.0f;
    return p;
}

// Clip the actual swept blade, rather than extending sword reach or testing
// Link's distance from the sign. Interpolated time identifies the first surface.
static s32 EnAObj_ArrowClipBlade(ArrowSignClipVertex* triangle, const ArrowSignPlane* planes,
                                s32 planeCount, f32* hitTime, Vec3f* hitPos) {
    ArrowSignClipVertex buffers[2][16];
    ArrowSignClipVertex* input;
    ArrowSignClipVertex* output;
    ArrowSignClipVertex previous;
    ArrowSignClipVertex current;
    f32 previousDistance;
    f32 currentDistance;
    f32 ratio;
    s32 count = 3;
    s32 nextCount;
    s32 planeIndex;
    s32 i;
    s32 buffer = 0;

    for (i = 0; i < 3; i++) {
        buffers[0][i] = triangle[i];
    }
    for (planeIndex = 0; planeIndex < planeCount && count != 0; planeIndex++) {
        const ArrowSignPlane* plane = &planes[planeIndex];
        input = buffers[buffer];
        output = buffers[1 - buffer];
        nextCount = 0;
        previous = input[count - 1];
        previousDistance = plane->normal.x * previous.pos.x + plane->normal.y * previous.pos.y +
                           plane->normal.z * previous.pos.z - plane->distance;
        for (i = 0; i < count; i++) {
            current = input[i];
            currentDistance = plane->normal.x * current.pos.x + plane->normal.y * current.pos.y +
                              plane->normal.z * current.pos.z - plane->distance;
            if ((previousDistance <= 0.0f) != (currentDistance <= 0.0f)) {
                ratio = previousDistance / (previousDistance - currentDistance);
                output[nextCount].pos.x = previous.pos.x + ratio * (current.pos.x - previous.pos.x);
                output[nextCount].pos.y = previous.pos.y + ratio * (current.pos.y - previous.pos.y);
                output[nextCount].pos.z = previous.pos.z + ratio * (current.pos.z - previous.pos.z);
                output[nextCount].time = previous.time + ratio * (current.time - previous.time);
                nextCount++;
            }
            if (currentDistance <= 0.0f) {
                output[nextCount++] = current;
            }
            previous = current;
            previousDistance = currentDistance;
        }
        count = nextCount;
        buffer = 1 - buffer;
    }
    if (count == 0) {
        return false;
    }
    for (i = 0; i < count; i++) {
        if (buffers[buffer][i].time < *hitTime) {
            *hitTime = buffers[buffer][i].time;
            *hitPos = buffers[buffer][i].pos;
        }
    }
    return true;
}

static s32 EnAObj_ArrowIsStab(Player* player) {
    switch (player->meleeWeaponAnimation) {
        case PLAYER_MWA_STAB_1H:
        case PLAYER_MWA_STAB_2H:
        case PLAYER_MWA_STAB_COMBO_1H:
        case PLAYER_MWA_STAB_COMBO_2H:
            return true; // Crouch stabs also use PLAYER_MWA_STAB_1H.
        default:
            return false;
    }
}

static void EnAObj_TryBreakArrowSign(EnAObj* this, PlayState* play) {
    EnAObjArrowSign* sign = (EnAObjArrowSign*)this;
    Actor* actor = &this->dyna.actor;
    Player* player = GET_PLAYER(play);
    Actor* board;
    ArrowSignClipVertex triangles[2][3];
    Vec3f base;
    Vec3f tip;
    Vec3f polePos;
    Vec3f boardPos;
    Vec3f impactPos;
    f32 poleTime = 2.0f;
    f32 boardTime = 2.0f;
    s32 poleHit = false;
    s32 boardHit = false;
    s32 newSwing;
    s32 i;

    if (sign->broken || !CVarGetInteger(CVAR_ENHANCEMENT("BreakableArrowSigns"), 0) ||
        this->actionFunc != EnAObj_WaitTalk || player->meleeWeaponState <= 0 ||
        !player->meleeWeaponInfo[0].active || player->heldItemAction < PLAYER_IA_SWORD_MASTER ||
        player->heldItemAction > PLAYER_IA_SWORD_BIGGORON || actor->xzDistToPlayer > 200.0f) {
        sign->bladeHistoryValid = false;
        sign->swordHitLatched = false;
        return;
    }

    base = EnAObj_ArrowToLocal(actor, &player->meleeWeaponInfo[0].base);
    tip = EnAObj_ArrowToLocal(actor, &player->meleeWeaponInfo[0].tip);
    newSwing = !sign->bladeHistoryValid || sign->bladeAnimation != player->meleeWeaponAnimation ||
               player->skelAnime.curFrame < sign->bladeFrame;
    if (newSwing) {
        sign->swordHitLatched = false;
        sign->previousBladeBase = base;
        sign->previousBladeTip = tip;
    }
    triangles[0][0].pos = sign->previousBladeBase;
    triangles[0][0].time = 0.0f;
    triangles[0][1].pos = sign->previousBladeTip;
    triangles[0][1].time = 0.0f;
    triangles[0][2].pos = tip;
    triangles[0][2].time = 1.0f;
    triangles[1][0] = triangles[0][0];
    triangles[1][1] = triangles[0][2];
    triangles[1][2].pos = base;
    triangles[1][2].time = 1.0f;
    sign->previousBladeBase = base;
    sign->previousBladeTip = tip;
    sign->bladeAnimation = player->meleeWeaponAnimation;
    sign->bladeFrame = player->skelAnime.curFrame;
    sign->bladeHistoryValid = true;
    if (sign->swordHitLatched) {
        return;
    }

    for (i = 0; i < 2; i++) {
        poleHit |= EnAObj_ArrowClipBlade(triangles[i], sArrowPolePlanes, ARRAY_COUNT(sArrowPolePlanes),
                                         &poleTime, &polePos);
        boardHit |= EnAObj_ArrowClipBlade(triangles[i], sArrowBoardPlanes, ARRAY_COUNT(sArrowBoardPlanes),
                                          &boardTime, &boardPos);
    }
    if (!poleHit && !boardHit) {
        return;
    }

    // Only a stab can detach the arrow sign. A stab may hit the entire board
    // (or the post), while all other sword strikes produce wood sparks.
    if (!EnAObj_ArrowIsStab(player)) {
        sign->swordHitLatched = true;
        // The model is upright in vanilla scenes: convert the local contact back
        // through the actor's draw transform, including any scene rotation/scale.
        Matrix_Push();
        Matrix_Translate(actor->world.pos.x, actor->world.pos.y, actor->world.pos.z, MTXMODE_NEW);
        Matrix_RotateZ((actor->shape.rot.z / (f32)0x8000) * M_PI, MTXMODE_APPLY);
        Matrix_RotateY((actor->shape.rot.y / (f32)0x8000) * M_PI, MTXMODE_APPLY);
        Matrix_RotateX((actor->shape.rot.x / (f32)0x8000) * M_PI, MTXMODE_APPLY);
        Matrix_Scale(actor->scale.x * 100.0f, actor->scale.y * 100.0f, actor->scale.z * 100.0f, MTXMODE_APPLY);
        Matrix_MultVec3f(boardHit && boardTime <= poleTime ? &boardPos : &polePos, &impactPos);
        Matrix_Pop();
        // Existing wood-surface helper provides particles and the wood impact sound.
        CollisionCheck_SpawnShieldParticlesWood(play, &impactPos, &actor->projectedPos);
        return;
    }

    board = Actor_SpawnAsChild(&play->actorCtx, actor, play, ACTOR_EN_A_OBJ,
                              actor->world.pos.x, actor->world.pos.y + 40.0f, actor->world.pos.z,
                              actor->shape.rot.x, actor->shape.rot.y, actor->shape.rot.z,
                              ARROW_SIGN_BOARD_PARAM);
    if (board == NULL) {
        return; // Leave the sign intact if no actor slot is available.
    }
    sign->swordHitLatched = true;
    board->world.rot.y = actor->yawTowardsPlayer + 0x8000;
    board->speedXZ = 2.0f;
    board->velocity.y = 3.0f;
    sign->broken = 1;
    actor->flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
    this->collider.dim.radius = 4;
    this->collider.dim.height = 52;
    Audio_PlayActorSound2(actor, NA_SE_IT_SWORD_STRIKE);
}

void EnAObj_Update(Actor* thisx, PlayState* play) {
    EnAObj* this = (EnAObj*)thisx;

    if (thisx->params == A_OBJ_SIGNPOST_ARROW) {
        EnAObj_TryBreakArrowSign(this, play);
    }
    if (thisx->params != A_OBJ_SIGNPOST_ARROW || !((EnAObjArrowSign*)this)->broken) {
        this->actionFunc(this, play);
    }
    Actor_MoveXZGravity(&this->dyna.actor);

    if (this->dyna.actor.gravity != 0.0f) {
        if (this->dyna.actor.params != A_OBJ_BOULDER_FRAGMENT) {
            Actor_UpdateBgCheckInfo(play, &this->dyna.actor, 5.0f, 40.0f, 0.0f, 0x1D);
        } else {
            Actor_UpdateBgCheckInfo(play, &this->dyna.actor, 5.0f, 20.0f, 0.0f, 0x1D);
        }
    }

    this->dyna.actor.focus.pos = this->dyna.actor.world.pos;
    this->dyna.actor.focus.pos.y += this->focusYoffset;

    switch (this->dyna.actor.params) {
        case A_OBJ_SIGNPOST_OBLONG:
        case A_OBJ_SIGNPOST_ARROW:
            Collider_UpdateCylinder(&this->dyna.actor, &this->collider);
            CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
            break;
    }
}

void EnAObj_Draw(Actor* thisx, PlayState* play) {
    s32 type = thisx->params;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL_25Opa(play->state.gfxCtx);

    if (type >= A_OBJ_MAX) {
        type = A_OBJ_BOULDER_FRAGMENT;
    }

    if (thisx->params == A_OBJ_BOULDER_FRAGMENT) {
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 1, 60, 60, 60, 50);
    }

    if (thisx->params == ARROW_SIGN_BOARD_PARAM) {
        // Original board vertices are centered at local Y = 4000 (world Y = 40).
        Matrix_Translate(0.0f, -4000.0f, 0.0f, MTXMODE_APPLY);
        gSPMatrix(POLY_OPA_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_MODELVIEW | G_MTX_LOAD);
        gSPDisplayList(POLY_OPA_DISP++, sArrowBoardDL);
    } else {
        gSPMatrix(POLY_OPA_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_MODELVIEW | G_MTX_LOAD);
        if (thisx->params == A_OBJ_SIGNPOST_ARROW && ((EnAObjArrowSign*)thisx)->broken) {
            gSPDisplayList(POLY_OPA_DISP++, sArrowPostDL);
        } else {
            gSPDisplayList(POLY_OPA_DISP++, sDLists[type]);
        }
    }

    CLOSE_DISPS(play->state.gfxCtx);
}
