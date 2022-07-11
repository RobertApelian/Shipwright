/*
 * File: z_player.c
 * Overlay: ovl_player_actor
 * Description: Link
 */

#include "ultra64.h"
#include "global.h"

#include "overlays/actors/ovl_Bg_Heavy_Block/z_bg_heavy_block.h"
#include "overlays/actors/ovl_Door_Shutter/z_door_shutter.h"
#include "overlays/actors/ovl_En_Boom/z_en_boom.h"
#include "overlays/actors/ovl_En_Arrow/z_en_arrow.h"
#include "overlays/actors/ovl_En_Box/z_en_box.h"
#include "overlays/actors/ovl_En_Door/z_en_door.h"
#include "overlays/actors/ovl_En_Elf/z_en_elf.h"
#include "overlays/actors/ovl_En_Fish/z_en_fish.h"
#include "overlays/actors/ovl_En_Horse/z_en_horse.h"
#include "overlays/effects/ovl_Effect_Ss_Fhg_Flash/z_eff_ss_fhg_flash.h"
#include "objects/gameplay_keep/gameplay_keep.h"
#include "objects/object_link_child/object_link_child.h"
#include "textures/icon_item_24_static/icon_item_24_static.h"

#include "overlays/actors/ovl_En_Bom/z_en_bom.h"
#include "overlays/actors/ovl_Bg_Hidan_Curtain/z_bg_hidan_curtain.h"

typedef struct {
    /* 0x00 */ u8 itemId;
    /* 0x01 */ u8 field; // various bit-packed data
    /* 0x02 */ s8 gi;    // defines the draw id and chest opening animation
    /* 0x03 */ u8 textId;
    /* 0x04 */ u16 objectId;
} GetItemEntry; // size = 0x06

#define GET_ITEM(itemId, objectId, drawId, textId, field, chestAnim) \
    { itemId, field, (chestAnim != CHEST_ANIM_SHORT ? 1 : -1) * (drawId + 1), textId, objectId }

#define CHEST_ANIM_SHORT 0
#define CHEST_ANIM_LONG 1

#define GET_ITEM_NONE \
    { ITEM_NONE, 0, 0, 0, OBJECT_INVALID }

typedef enum {
    /* 0x00 */ KNOB_ANIM_ADULT_L,
    /* 0x01 */ KNOB_ANIM_CHILD_L,
    /* 0x02 */ KNOB_ANIM_ADULT_R,
    /* 0x03 */ KNOB_ANIM_CHILD_R
} KnobDoorAnim;

typedef struct {
    /* 0x00 */ u8 itemId;
    /* 0x02 */ s16 actorId;
} ExplosiveInfo; // size = 0x04

typedef struct {
    /* 0x00 */ s16 actorId;
    /* 0x02 */ u8 itemId;
    /* 0x03 */ u8 actionParam;
    /* 0x04 */ u8 textId;
} BottleCatchInfo; // size = 0x06

typedef struct {
    /* 0x00 */ s16 actorId;
    /* 0x02 */ s16 actorParams;
} BottleDropInfo; // size = 0x04

typedef struct {
    /* 0x00 */ s8 damage;
    /* 0x01 */ u8 unk_01;
    /* 0x02 */ u8 unk_02;
    /* 0x03 */ u8 unk_03;
    /* 0x04 */ u16 sfxId;
} FallImpactInfo; // size = 0x06

typedef struct {
    /* 0x00 */ Vec3f pos;
    /* 0x0C */ s16 yaw;
} SpecialRespawnInfo; // size = 0x10

typedef struct {
    /* 0x00 */ u16 sfxId;
    /* 0x02 */ s16 field;
} PlayerAnimSfxEntry; // size = 0x04

typedef struct {
    /* 0x00 */ u16 unk_00;
    /* 0x02 */ s16 unk_02;
} struct_808551A4; // size = 0x04

typedef struct {
    /* 0x00 */ LinkAnimationHeader* anim;
    /* 0x04 */ u8 unk_04;
} ItemChangeAnimInfo; // size = 0x08

typedef struct {
    /* 0x00 */ LinkAnimationHeader* bottleSwingAnim;
    /* 0x04 */ LinkAnimationHeader* bottleCatchAnim;
    /* 0x08 */ u8 unk_08;
    /* 0x09 */ u8 unk_09;
} BottleSwingAnimInfo; // size = 0x0C

typedef struct {
    /* 0x00 */ LinkAnimationHeader* unk_00;
    /* 0x04 */ LinkAnimationHeader* unk_04;
    /* 0x08 */ LinkAnimationHeader* unk_08;
    /* 0x0C */ u8 unk_0C;
    /* 0x0D */ u8 unk_0D;
} MeleeAttackAnimInfo; // size = 0x10

typedef struct {
    /* 0x00 */ LinkAnimationHeader* anim;
    /* 0x04 */ f32 unk_04;
    /* 0x04 */ f32 unk_08;
} struct_80854578; // size = 0x0C

typedef struct {
    /* 0x00 */ s8 type;
    /* 0x04 */ union {
        void* ptr;
        void (*func)(GlobalContext*, Player*, CsCmdActorAction*);
    };
} struct_80854B18; // size = 0x08

typedef struct {
    /* 0x00 */ s16 unk_00;
    /* 0x02 */ s16 unk_02;
    /* 0x04 */ s16 unk_04;
    /* 0x06 */ s16 unk_06;
    /* 0x08 */ s16 unk_08;
} struct_80858AC8; // size = 0x0A

void Player_Draw(Actor* thisx, GlobalContext* globalCtx2);

void Player_DoNothing(GlobalContext* globalCtx, Player* this);
void Player_DoNothing2(GlobalContext* globalCtx, Player* this);
void Player_SetupBowOrSlingshot(GlobalContext* globalCtx, Player* this);
void Player_SetupDekuStick(GlobalContext* globalCtx, Player* this);
void Player_SetupExplosive(GlobalContext* globalCtx, Player* this);
void Player_SetupHookshot(GlobalContext* globalCtx, Player* this);
void Player_SetupBoomerang(GlobalContext* globalCtx, Player* this);
void Player_ChangeItem(GlobalContext* globalCtx, Player* this, s8 actionParam);
s32 Player_SetupStartZTargetDefend(Player* this, GlobalContext* globalCtx);
s32 Player_SetupStartZTargetDefend2(Player* this, GlobalContext* globalCtx);
s32 Player_StartChangeItem(Player* this, GlobalContext* globalCtx);
s32 Player_StandingDefend(Player* this, GlobalContext* globalCtx);
s32 Player_EndDefend(Player* this, GlobalContext* globalCtx);
s32 Player_HoldFpsItem(Player* this, GlobalContext* globalCtx);
s32 Player_ReadyFpsItemToShoot(Player* this, GlobalContext* globalCtx);
s32 Player_AimFpsItem(Player* this, GlobalContext* globalCtx);
s32 Player_EndAimFpsItem(Player* this, GlobalContext* globalCtx);
s32 Player_HoldActor(Player* this, GlobalContext* globalCtx);
s32 Player_HoldBoomerang(Player* this, GlobalContext* globalCtx);
s32 Player_SetupAimBoomerang(Player* this, GlobalContext* globalCtx);
s32 Player_AimBoomerang(Player* this, GlobalContext* globalCtx);
s32 Player_ThrowBoomerang(Player* this, GlobalContext* globalCtx);
s32 Player_WaitForThrownBoomerang(Player* this, GlobalContext* globalCtx);
s32 Player_CatchBoomerang(Player* this, GlobalContext* globalCtx);
void Player_UseItem(GlobalContext* globalCtx, Player* this, s32 item);
void Player_SetupStandingStillType(Player* this, GlobalContext* globalCtx);
s32 Player_SetupWallJumpBehavior(Player* this, GlobalContext* globalCtx);
s32 Player_SetupOpenDoor(Player* this, GlobalContext* globalCtx);
s32 Player_SetupItemCutsceneOrFirstPerson(Player* this, GlobalContext* globalCtx);
s32 Player_SetupCUpBehavior(Player* this, GlobalContext* globalCtx);
s32 Player_SetupSpeakOrCheck(Player* this, GlobalContext* globalCtx);
s32 Player_SetupJumpSlashOrRoll(Player* this, GlobalContext* globalCtx);
s32 Player_SetupRollOrPutAway(Player* this, GlobalContext* globalCtx);
s32 Player_SetupDefend(Player* this, GlobalContext* globalCtx);
s32 Player_SetupStartChargeSpinAttack(Player* this, GlobalContext* globalCtx);
s32 Player_SetupThrowDekuNut(GlobalContext* globalCtx, Player* this);
void Player_SpawnNoMomentum(GlobalContext* globalCtx, Player* this);
void Player_SpawnWalkingSlow(GlobalContext* globalCtx, Player* this);
void Player_SpawnWalkingPreserveMomentum(GlobalContext* globalCtx, Player* this);
s32 Player_SetupMountHorse(Player* this, GlobalContext* globalCtx);
s32 Player_SetupGetItemOrHoldBehavior(Player* this, GlobalContext* globalCtx);
s32 Player_SetupPutDownOrThrowActor(Player* this, GlobalContext* globalCtx);
s32 Player_SetupSpecialWallInteraction(Player* this, GlobalContext* globalCtx);
void Player_UnfriendlyZTargetStandingStill(Player* this, GlobalContext* globalCtx);
void Player_FriendlyZTargetStandingStill(Player* this, GlobalContext* globalCtx);
void Player_StandingStill(Player* this, GlobalContext* globalCtx);
void Player_EndSidewalk(Player* this, GlobalContext* globalCtx);
void Player_FriendlyBackwalk(Player* this, GlobalContext* globalCtx);
void Player_HaltFriendlyBackwalk(Player* this, GlobalContext* globalCtx);
void Player_EndHaltFriendlyBackwalk(Player* this, GlobalContext* globalCtx);
void Player_Sidewalk(Player* this, GlobalContext* globalCtx);
void Player_Turn(Player* this, GlobalContext* globalCtx);
void Player_Run(Player* this, GlobalContext* globalCtx);
void Player_ZTargetingRun(Player* this, GlobalContext* globalCtx);
void func_8084279C(Player* this, GlobalContext* globalCtx);
void Player_UnfriendlyBackwalk(Player* this, GlobalContext* globalCtx);
void Player_EndUnfriendlyBackwalk(Player* this, GlobalContext* globalCtx);
void Player_AimShieldCrouched(Player* this, GlobalContext* globalCtx);
void Player_DeflectAttackWithShield(Player* this, GlobalContext* globalCtx);
void func_8084370C(Player* this, GlobalContext* globalCtx);
void Player_StartKnockback(Player* this, GlobalContext* globalCtx);
void Player_DownFromKnockback(Player* this, GlobalContext* globalCtx);
void Player_GetUpFromKnockback(Player* this, GlobalContext* globalCtx);
void Player_Die(Player* this, GlobalContext* globalCtx);
void Player_UpdateMidair(Player* this, GlobalContext* globalCtx);
void Player_Rolling(Player* this, GlobalContext* globalCtx);
void Player_FallingDive(Player* this, GlobalContext* globalCtx);
void Player_JumpSlash(Player* this, GlobalContext* globalCtx);
void Player_ChargeSpinAttack(Player* this, GlobalContext* globalCtx);
void Player_WalkChargingSpinAttack(Player* this, GlobalContext* globalCtx);
void Player_SidewalkChargingSpinAttack(Player* this, GlobalContext* globalCtx);
void Player_JumpUpToLedge(Player* this, GlobalContext* globalCtx);
void Player_RunMiniCutsceneFunc(Player* this, GlobalContext* globalCtx);
void Player_MiniCsMovement(Player* this, GlobalContext* globalCtx);
void Player_OpenDoor(Player* this, GlobalContext* globalCtx);
void Player_LiftActor(Player* this, GlobalContext* globalCtx);
void Player_ThrowStonePillar(Player* this, GlobalContext* globalCtx);
void Player_LiftSilverBoulder(Player* this, GlobalContext* globalCtx);
void Player_ThrowSilverBoulder(Player* this, GlobalContext* globalCtx);
void Player_FailToLiftActor(Player* this, GlobalContext* globalCtx);
void Player_SetupPutDownActor(Player* this, GlobalContext* globalCtx);
void Player_StartThrowActor(Player* this, GlobalContext* globalCtx);
void Player_SpawnNoUpdateOrDraw(GlobalContext* globalCtx, Player* this);
void Player_SetupSpawnFromBlueWarp(GlobalContext* globalCtx, Player* this);
void Player_SpawnFromTimeTravel(GlobalContext* globalCtx, Player* this);
void Player_SpawnOpeningDoor(GlobalContext* globalCtx, Player* this);
void Player_SpawnExitingGrotto(GlobalContext* globalCtx, Player* this);
void Player_SpawnWithKnockback(GlobalContext* globalCtx, Player* this);
void Player_SetupSpawnFromWarpSong(GlobalContext* globalCtx, Player* this);
void Player_SetupSpawnFromFaroresWind(GlobalContext* globalCtx, Player* this);
void Player_FirstPersonAiming(Player* this, GlobalContext* globalCtx);
void Player_TalkWithActor(Player* this, GlobalContext* globalCtx);
void Player_GrabPushPullWall(Player* this, GlobalContext* globalCtx);
void Player_PushWall(Player* this, GlobalContext* globalCtx);
void Player_PullWall(Player* this, GlobalContext* globalCtx);
void Player_GrabLedge(Player* this, GlobalContext* globalCtx);
void Player_ClimbOntoLedge(Player* this, GlobalContext* globalCtx);
void Player_ClimbingWallOrDownLedge(Player* this, GlobalContext* globalCtx);
void Player_UpdateCommon(Player* this, GlobalContext* globalCtx, Input* input);
void Player_EndClimb(Player* this, GlobalContext* globalCtx);
void Player_InsideCrawlspace(Player* this, GlobalContext* globalCtx);
void Player_ExitCrawlspace(Player* this, GlobalContext* globalCtx);
void Player_RideHorse(Player* this, GlobalContext* globalCtx);
void Player_DismountHorse(Player* this, GlobalContext* globalCtx);
void Player_UpdateSwimIdle(Player* this, GlobalContext* globalCtx);
void Player_SpawnSwimming(Player* this, GlobalContext* globalCtx);
void Player_Swim(Player* this, GlobalContext* globalCtx);
void Player_ZTargetSwimming(Player* this, GlobalContext* globalCtx);
void Player_Dive(Player* this, GlobalContext* globalCtx);
void Player_GetItemInWater(Player* this, GlobalContext* globalCtx);
void Player_DamagedSwim(Player* this, GlobalContext* globalCtx);
void Player_Drown(Player* this, GlobalContext* globalCtx);
void Player_PlayOcarina(Player* this, GlobalContext* globalCtx);
void Player_ThrowDekuNut(Player* this, GlobalContext* globalCtx);
void Player_GetItem(Player* this, GlobalContext* globalCtx);
void Player_EndTimeTravel(Player* this, GlobalContext* globalCtx);
void Player_DrinkFromBottle(Player* this, GlobalContext* globalCtx);
void Player_SwingBottle(Player* this, GlobalContext* globalCtx);
void Player_HealWithFairy(Player* this, GlobalContext* globalCtx);
void Player_DropItemFromBottle(Player* this, GlobalContext* globalCtx);
void Player_PresentExchangeItem(Player* this, GlobalContext* globalCtx);
void Player_SlipOnSlope(Player* this, GlobalContext* globalCtx);
void Player_SetDrawAndStartCutsceneAfterTimer(Player* this, GlobalContext* globalCtx);
void Player_SpawnFromWarpSong(Player* this, GlobalContext* globalCtx);
void Player_SpawnFromBlueWarp(Player* this, GlobalContext* globalCtx);
void Player_EnterGrotto(Player* this, GlobalContext* globalCtx);
void Player_SetupOpenDoorFromSpawn(Player* this, GlobalContext* globalCtx);
void Player_JumpFromGrotto(Player* this, GlobalContext* globalCtx);
void Player_ShootingGalleryPlay(Player* this, GlobalContext* globalCtx);
void Player_FrozenInIce(Player* this, GlobalContext* globalCtx);
void Player_SetupElectricShock(Player* this, GlobalContext* globalCtx);
s32 Player_CheckNoDebugModeCombo(Player* this, GlobalContext* globalCtx);
void Player_BowStringMoveAfterShot(Player* this);
void Player_BunnyHoodPhysics(Player* this);
s32 Player_SetupStartMeleeWeaponAttack(Player* this, GlobalContext* globalCtx);
void Player_MeleeWeaponAttack(Player* this, GlobalContext* globalCtx);
void Player_MeleeWeaponRebound(Player* this, GlobalContext* globalCtx);
void Player_ChooseFaroresWindOption(Player* this, GlobalContext* globalCtx);
void Player_SpawnFromFaroresWind(Player* this, GlobalContext* globalCtx);
void Player_UpdateMagicSpell(Player* this, GlobalContext* globalCtx);
void Player_MoveAlongHookshotPath(Player* this, GlobalContext* globalCtx);
void Player_CastFishingRod(Player* this, GlobalContext* globalCtx);
void Player_ReleaseCaughtFish(Player* this, GlobalContext* globalCtx);
void Player_AnimPlaybackType0(GlobalContext* globalCtx, Player* this, void* anim);
void Player_AnimPlaybackType1(GlobalContext* globalCtx, Player* this, void* anim);
void Player_AnimPlaybackType13(GlobalContext* globalCtx, Player* this, void* anim);
void Player_AnimPlaybackType2(GlobalContext* globalCtx, Player* this, void* anim);
void Player_AnimPlaybackType3(GlobalContext* globalCtx, Player* this, void* anim);
void Player_AnimPlaybackType4(GlobalContext* globalCtx, Player* this, void* anim);
void Player_AnimPlaybackType5(GlobalContext* globalCtx, Player* this, void* anim);
void Player_AnimPlaybackType6(GlobalContext* globalCtx, Player* this, void* anim);
void Player_AnimPlaybackType7(GlobalContext* globalCtx, Player* this, void* anim);
void Player_AnimPlaybackType8(GlobalContext* globalCtx, Player* this, void* anim);
void Player_AnimPlaybackType9(GlobalContext* globalCtx, Player* this, void* anim);
void Player_AnimPlaybackType14(GlobalContext* globalCtx, Player* this, void* anim);
void Player_AnimPlaybackType15(GlobalContext* globalCtx, Player* this, void* anim);
void Player_AnimPlaybackType10(GlobalContext* globalCtx, Player* this, void* anim);
void Player_AnimPlaybackType11(GlobalContext* globalCtx, Player* this, void* anim);
void Player_AnimPlaybackType16(GlobalContext* globalCtx, Player* this, void* anim);
void Player_AnimPlaybackType12(GlobalContext* globalCtx, Player* this, void* anim);
void Player_AnimPlaybackType17(GlobalContext* globalCtx, Player* this, void* arg2);
void Player_CutsceneSetupSwimIdle(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneSurfaceFromDive(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneIdle(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneTurnAroundSurprisedShort(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneSetupIdle(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneWait(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneTurnAroundSurprisedLong(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneSetupEnterWarp(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneEnterWarp(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneSetupFightStance(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneFightStance(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneUnk3Update(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneUnk4Update(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneSetupSwordPedestal(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneSwordPedestal(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneSetupWarpToSages(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneWarpToSages(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneKnockedToGround(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneSetupStartPlayOcarina(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneDrawAndBrandishSword(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneCloseEyes(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneOpenEyes(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneSetupGetItemInWater(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneSetupSleeping(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneSleeping(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneSetupSleepingRestless(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneAwaken(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneGetOffBed(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneSetupBlownBackward(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneBlownBackward(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneRaisedByWarp(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneSetupIdle3(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneIdle3(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneSetupStop(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneSetDraw(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneInspectGroundCarefully(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneStartPassOcarina(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneDrawSwordChild(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneTurnAroundSlowly(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneDesperateLookAtZeldasCrystal(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneStepBackCautiously(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneSetupSpinAttackIdle(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneSpinAttackIdle(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneInspectWeapon(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_SetupDoNothing4(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_DoNothing5(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneSetupKnockedToGroundDamaged(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneKnockedToGroundDamaged(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneSetupGetSwordBack(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneSwordKnockedFromHand(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_LearnOcarinaSong(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneGetSwordBack(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneGanonKillCombo(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneEnd(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneSetPosAndYaw(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_CutsceneUnk6Update(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2);
void Player_StartCutscene(Player* this, GlobalContext* globalCtx);
s32 Player_IsDroppingFish(GlobalContext* globalCtx);
s32 Player_StartFishing(GlobalContext* globalCtx);
s32 Player_SetupRestrainedByEnemy(GlobalContext* globalCtx, Player* this);
s32 Player_SetupPlayerCutscene(GlobalContext* globalCtx, Actor* actor, s32 csMode);
void Player_SetupStandingStillMorph(Player* this, GlobalContext* globalCtx);
s32 Player_InflictDamage(GlobalContext* globalCtx, s32 damage);
s32 Player_InflictDamageModified(GlobalContext* globalCtx, s32 damage, u8 modified);
void Player_StartTalkingWithActor(GlobalContext* globalCtx, Actor* actor);

// .bss part 1
static s32 sPrevSkelAnimeMoveFlags;
static s32 sCurrentMask;
static Vec3f sWallIntersectPos;
static Input* sControlInput;

// .data

static u8 D_80853410[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

static PlayerAgeProperties sAgeProperties[] = {
    {
        56.0f,
        90.0f,
        1.0f,
        111.0f,
        70.0f,
        79.4f,
        59.0f,
        41.0f,
        19.0f,
        36.0f,
        44.8f,
        56.0f,
        68.0f,
        70.0f,
        18.0f,
        15.0f,
        70.0f,
        { 9, 4671, 359 },
        {
            { 8, 4694, 380 },
            { 9, 6122, 359 },
            { 8, 4694, 380 },
            { 9, 6122, 359 },
        },
        {
            { 9, 6122, 359 },
            { 9, 7693, 380 },
            { 9, 6122, 359 },
            { 9, 7693, 380 },
        },
        {
            { 8, 4694, 380 },
            { 9, 6122, 359 },
        },
        {
            { -1592, 4694, 380 },
            { -1591, 6122, 359 },
        },
        0,
        0x80,
        &gPlayerAnim_002718,
        &gPlayerAnim_002720,
        &gPlayerAnim_002838,
        &gPlayerAnim_002E70,
        &gPlayerAnim_002E78,
        { &gPlayerAnim_002E80, &gPlayerAnim_002E88, &gPlayerAnim_002D90, &gPlayerAnim_002D98 },
        { &gPlayerAnim_002D70, &gPlayerAnim_002D78 },
        { &gPlayerAnim_002E50, &gPlayerAnim_002E58 },
        { &gPlayerAnim_002E68, &gPlayerAnim_002E60 },
    },
    {
        40.0f,
        60.0f,
        11.0f / 17.0f,
        71.0f,
        50.0f,
        47.0f,
        39.0f,
        27.0f,
        19.0f,
        22.0f,
        29.6f,
        32.0f,
        48.0f,
        70.0f * (11.0f / 17.0f),
        14.0f,
        12.0f,
        55.0f,
        { -24, 3565, 876 },
        {
            { -24, 3474, 862 },
            { -24, 4977, 937 },
            { 8, 4694, 380 },
            { 9, 6122, 359 },
        },
        {
            { -24, 4977, 937 },
            { -24, 6495, 937 },
            { 9, 6122, 359 },
            { 9, 7693, 380 },
        },
        {
            { 8, 4694, 380 },
            { 9, 6122, 359 },
        },
        {
            { -1592, 4694, 380 },
            { -1591, 6122, 359 },
        },
        0x20,
        0,
        &gPlayerAnim_002318,
        &gPlayerAnim_002360,
        &gPlayerAnim_0023A8,
        &gPlayerAnim_0023E0,
        &gPlayerAnim_0023E8,
        { &gPlayerAnim_0023F0, &gPlayerAnim_0023F8, &gPlayerAnim_002D90, &gPlayerAnim_002D98 },
        { &gPlayerAnim_002D70, &gPlayerAnim_002D78 },
        { &gPlayerAnim_0023C0, &gPlayerAnim_0023C8 },
        { &gPlayerAnim_0023D8, &gPlayerAnim_0023D0 },
    },
};

static u32 sDebugModeFlag = false;
static f32 sAnalogStickDistance = 0.0f;
static s16 sAnalogStickAngle = 0;
static s16 sCameraOffsetAnalogStickAngle = 0;
static s32 D_808535E0 = 0;
static s32 sFloorSpecialProperty = 0;
static f32 sWaterSpeedScale = 1.0f;
static f32 sInvertedWaterSpeedScale = 1.0f;
static u32 sTouchedWallFlags = 0;
static u32 sConveyorSpeedIndex = 0;
static s16 sIsFloorConveyor = 0;
static s16 sConveyorYaw = 0;
static f32 sPlayerYDistToFloor = 0.0f;
static s32 sFloorProperty = 0;
static s32 sYawToTouchedWall = 0;
static s32 sYawToTouchedWall2 = 0;
static s16 sAngleToFloorX = 0;
static s32 sUsingItemAlreadyInHand = 0;
static s32 sUsingItemAlreadyInHand2 = 0;

static u16 sInterruptableSfx[] = {
    NA_SE_VO_LI_SWEAT,
    NA_SE_VO_LI_SNEEZE,
    NA_SE_VO_LI_RELAX,
    NA_SE_VO_LI_FALL_L,
};

static GetItemEntry sGetItemTable[] = {
    GET_ITEM(ITEM_BOMBS_5, OBJECT_GI_BOMB_1, GID_BOMB, 0x32, 0x59, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_NUTS_5, OBJECT_GI_NUTS, GID_NUTS, 0x34, 0x0C, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_BOMBCHU, OBJECT_GI_BOMB_2, GID_BOMBCHU, 0x33, 0x80, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_BOW, OBJECT_GI_BOW, GID_BOW, 0x31, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_SLINGSHOT, OBJECT_GI_PACHINKO, GID_SLINGSHOT, 0x30, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_BOOMERANG, OBJECT_GI_BOOMERANG, GID_BOOMERANG, 0x35, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_STICK, OBJECT_GI_STICK, GID_STICK, 0x37, 0x0D, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_HOOKSHOT, OBJECT_GI_HOOKSHOT, GID_HOOKSHOT, 0x36, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_LONGSHOT, OBJECT_GI_HOOKSHOT, GID_LONGSHOT, 0x4F, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_LENS, OBJECT_GI_GLASSES, GID_LENS, 0x39, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_LETTER_ZELDA, OBJECT_GI_LETTER, GID_LETTER_ZELDA, 0x69, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_OCARINA_TIME, OBJECT_GI_OCARINA, GID_OCARINA_TIME, 0x3A, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_HAMMER, OBJECT_GI_HAMMER, GID_HAMMER, 0x38, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_COJIRO, OBJECT_GI_NIWATORI, GID_COJIRO, 0x02, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_BOTTLE, OBJECT_GI_BOTTLE, GID_BOTTLE, 0x42, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_POTION_RED, OBJECT_GI_LIQUID, GID_POTION_RED, 0x43, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_POTION_GREEN, OBJECT_GI_LIQUID, GID_POTION_GREEN, 0x44, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_POTION_BLUE, OBJECT_GI_LIQUID, GID_POTION_BLUE, 0x45, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_FAIRY, OBJECT_GI_BOTTLE, GID_BOTTLE, 0x46, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_MILK_BOTTLE, OBJECT_GI_MILK, GID_MILK, 0x98, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_LETTER_RUTO, OBJECT_GI_BOTTLE_LETTER, GID_LETTER_RUTO, 0x99, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_BEAN, OBJECT_GI_BEAN, GID_BEAN, 0x48, 0x80, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_MASK_SKULL, OBJECT_GI_SKJ_MASK, GID_MASK_SKULL, 0x10, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_MASK_SPOOKY, OBJECT_GI_REDEAD_MASK, GID_MASK_SPOOKY, 0x11, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_CHICKEN, OBJECT_GI_NIWATORI, GID_CHICKEN, 0x48, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_MASK_KEATON, OBJECT_GI_KI_TAN_MASK, GID_MASK_KEATON, 0x12, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_MASK_BUNNY, OBJECT_GI_RABIT_MASK, GID_MASK_BUNNY, 0x13, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_MASK_TRUTH, OBJECT_GI_TRUTH_MASK, GID_MASK_TRUTH, 0x17, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_POCKET_EGG, OBJECT_GI_EGG, GID_EGG, 0x01, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_POCKET_CUCCO, OBJECT_GI_NIWATORI, GID_CHICKEN, 0x48, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_ODD_MUSHROOM, OBJECT_GI_MUSHROOM, GID_ODD_MUSHROOM, 0x03, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_ODD_POTION, OBJECT_GI_POWDER, GID_ODD_POTION, 0x04, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_SAW, OBJECT_GI_SAW, GID_SAW, 0x05, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_SWORD_BROKEN, OBJECT_GI_BROKENSWORD, GID_SWORD_BROKEN, 0x08, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_PRESCRIPTION, OBJECT_GI_PRESCRIPTION, GID_PRESCRIPTION, 0x09, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_FROG, OBJECT_GI_FROG, GID_FROG, 0x0D, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_EYEDROPS, OBJECT_GI_EYE_LOTION, GID_EYEDROPS, 0x0E, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_CLAIM_CHECK, OBJECT_GI_TICKETSTONE, GID_CLAIM_CHECK, 0x0A, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_SWORD_KOKIRI, OBJECT_GI_SWORD_1, GID_SWORD_KOKIRI, 0xA4, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_SWORD_BGS, OBJECT_GI_LONGSWORD, GID_SWORD_BGS, 0x4B, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_SHIELD_DEKU, OBJECT_GI_SHIELD_1, GID_SHIELD_DEKU, 0x4C, 0xA0, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_SHIELD_HYLIAN, OBJECT_GI_SHIELD_2, GID_SHIELD_HYLIAN, 0x4D, 0xA0, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_SHIELD_MIRROR, OBJECT_GI_SHIELD_3, GID_SHIELD_MIRROR, 0x4E, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_TUNIC_GORON, OBJECT_GI_CLOTHES, GID_TUNIC_GORON, 0x50, 0xA0, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_TUNIC_ZORA, OBJECT_GI_CLOTHES, GID_TUNIC_ZORA, 0x51, 0xA0, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_BOOTS_IRON, OBJECT_GI_BOOTS_2, GID_BOOTS_IRON, 0x53, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_BOOTS_HOVER, OBJECT_GI_HOVERBOOTS, GID_BOOTS_HOVER, 0x54, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_QUIVER_40, OBJECT_GI_ARROWCASE, GID_QUIVER_40, 0x56, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_QUIVER_50, OBJECT_GI_ARROWCASE, GID_QUIVER_50, 0x57, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_BOMB_BAG_20, OBJECT_GI_BOMBPOUCH, GID_BOMB_BAG_20, 0x58, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_BOMB_BAG_30, OBJECT_GI_BOMBPOUCH, GID_BOMB_BAG_30, 0x59, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_BOMB_BAG_40, OBJECT_GI_BOMBPOUCH, GID_BOMB_BAG_40, 0x5A, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_GAUNTLETS_SILVER, OBJECT_GI_GLOVES, GID_GAUNTLETS_SILVER, 0x5B, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_GAUNTLETS_GOLD, OBJECT_GI_GLOVES, GID_GAUNTLETS_GOLD, 0x5C, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_SCALE_SILVER, OBJECT_GI_SCALE, GID_SCALE_SILVER, 0xCD, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_SCALE_GOLDEN, OBJECT_GI_SCALE, GID_SCALE_GOLDEN, 0xCE, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_STONE_OF_AGONY, OBJECT_GI_MAP, GID_STONE_OF_AGONY, 0x68, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_GERUDO_CARD, OBJECT_GI_GERUDO, GID_GERUDO_CARD, 0x7B, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_OCARINA_FAIRY, OBJECT_GI_OCARINA_0, GID_OCARINA_FAIRY, 0x4A, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_SEEDS, OBJECT_GI_SEED, GID_SEEDS, 0xDC, 0x50, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_HEART_CONTAINER, OBJECT_GI_HEARTS, GID_HEART_CONTAINER, 0xC6, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_HEART_PIECE_2, OBJECT_GI_HEARTS, GID_HEART_PIECE, 0xC2, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_KEY_BOSS, OBJECT_GI_BOSSKEY, GID_KEY_BOSS, 0xC7, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_COMPASS, OBJECT_GI_COMPASS, GID_COMPASS, 0x67, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_DUNGEON_MAP, OBJECT_GI_MAP, GID_DUNGEON_MAP, 0x66, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_KEY_SMALL, OBJECT_GI_KEY, GID_KEY_SMALL, 0x60, 0x80, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_MAGIC_SMALL, OBJECT_GI_MAGICPOT, GID_MAGIC_SMALL, 0x52, 0x6F, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_MAGIC_LARGE, OBJECT_GI_MAGICPOT, GID_MAGIC_LARGE, 0x52, 0x6E, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_WALLET_ADULT, OBJECT_GI_PURSE, GID_WALLET_ADULT, 0x5E, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_WALLET_GIANT, OBJECT_GI_PURSE, GID_WALLET_GIANT, 0x5F, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_WEIRD_EGG, OBJECT_GI_EGG, GID_EGG, 0x9A, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_HEART, OBJECT_GI_HEART, GID_HEART, 0x55, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_ARROWS_SMALL, OBJECT_GI_ARROW, GID_ARROWS_SMALL, 0xE6, 0x48, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_ARROWS_MEDIUM, OBJECT_GI_ARROW, GID_ARROWS_MEDIUM, 0xE6, 0x49, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_ARROWS_LARGE, OBJECT_GI_ARROW, GID_ARROWS_LARGE, 0xE6, 0x4A, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_RUPEE_GREEN, OBJECT_GI_RUPY, GID_RUPEE_GREEN, 0x6F, 0x00, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_RUPEE_BLUE, OBJECT_GI_RUPY, GID_RUPEE_BLUE, 0xCC, 0x01, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_RUPEE_RED, OBJECT_GI_RUPY, GID_RUPEE_RED, 0xF0, 0x02, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_HEART_CONTAINER, OBJECT_GI_HEARTS, GID_HEART_CONTAINER, 0xC6, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_MILK, OBJECT_GI_MILK, GID_MILK, 0x98, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_MASK_GORON, OBJECT_GI_GOLONMASK, GID_MASK_GORON, 0x14, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_MASK_ZORA, OBJECT_GI_ZORAMASK, GID_MASK_ZORA, 0x15, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_MASK_GERUDO, OBJECT_GI_GERUDOMASK, GID_MASK_GERUDO, 0x16, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_BRACELET, OBJECT_GI_BRACELET, GID_BRACELET, 0x79, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_RUPEE_PURPLE, OBJECT_GI_RUPY, GID_RUPEE_PURPLE, 0xF1, 0x14, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_RUPEE_GOLD, OBJECT_GI_RUPY, GID_RUPEE_GOLD, 0xF2, 0x13, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_SWORD_BGS, OBJECT_GI_LONGSWORD, GID_SWORD_BGS, 0x0C, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_ARROW_FIRE, OBJECT_GI_M_ARROW, GID_ARROW_FIRE, 0x70, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_ARROW_ICE, OBJECT_GI_M_ARROW, GID_ARROW_ICE, 0x71, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_ARROW_LIGHT, OBJECT_GI_M_ARROW, GID_ARROW_LIGHT, 0x72, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_SKULL_TOKEN, OBJECT_GI_SUTARU, GID_SKULL_TOKEN, 0xB4, 0x80, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_DINS_FIRE, OBJECT_GI_GODDESS, GID_DINS_FIRE, 0xAD, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_FARORES_WIND, OBJECT_GI_GODDESS, GID_FARORES_WIND, 0xAE, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_NAYRUS_LOVE, OBJECT_GI_GODDESS, GID_NAYRUS_LOVE, 0xAF, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_BULLET_BAG_30, OBJECT_GI_DEKUPOUCH, GID_BULLET_BAG, 0x07, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_BULLET_BAG_40, OBJECT_GI_DEKUPOUCH, GID_BULLET_BAG, 0x07, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_STICKS_5, OBJECT_GI_STICK, GID_STICK, 0x37, 0x0D, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_STICKS_10, OBJECT_GI_STICK, GID_STICK, 0x37, 0x0D, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_NUTS_5, OBJECT_GI_NUTS, GID_NUTS, 0x34, 0x0C, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_NUTS_10, OBJECT_GI_NUTS, GID_NUTS, 0x34, 0x0C, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_BOMB, OBJECT_GI_BOMB_1, GID_BOMB, 0x32, 0x59, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_BOMBS_10, OBJECT_GI_BOMB_1, GID_BOMB, 0x32, 0x59, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_BOMBS_20, OBJECT_GI_BOMB_1, GID_BOMB, 0x32, 0x59, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_BOMBS_30, OBJECT_GI_BOMB_1, GID_BOMB, 0x32, 0x59, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_SEEDS_30, OBJECT_GI_SEED, GID_SEEDS, 0xDC, 0x50, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_BOMBCHUS_5, OBJECT_GI_BOMB_2, GID_BOMBCHU, 0x33, 0x80, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_BOMBCHUS_20, OBJECT_GI_BOMB_2, GID_BOMBCHU, 0x33, 0x80, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_FISH, OBJECT_GI_FISH, GID_FISH, 0x47, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_BUG, OBJECT_GI_INSECT, GID_BUG, 0x7A, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_BLUE_FIRE, OBJECT_GI_FIRE, GID_BLUE_FIRE, 0x5D, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_POE, OBJECT_GI_GHOST, GID_POE, 0x97, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_BIG_POE, OBJECT_GI_GHOST, GID_BIG_POE, 0xF9, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_KEY_SMALL, OBJECT_GI_KEY, GID_KEY_SMALL, 0xF3, 0x80, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_RUPEE_GREEN, OBJECT_GI_RUPY, GID_RUPEE_GREEN, 0xF4, 0x00, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_RUPEE_BLUE, OBJECT_GI_RUPY, GID_RUPEE_BLUE, 0xF5, 0x01, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_RUPEE_RED, OBJECT_GI_RUPY, GID_RUPEE_RED, 0xF6, 0x02, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_RUPEE_PURPLE, OBJECT_GI_RUPY, GID_RUPEE_PURPLE, 0xF7, 0x14, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_HEART_PIECE_2, OBJECT_GI_HEARTS, GID_HEART_PIECE, 0xFA, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_STICK_UPGRADE_20, OBJECT_GI_STICK, GID_STICK, 0x90, 0x80, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_STICK_UPGRADE_30, OBJECT_GI_STICK, GID_STICK, 0x91, 0x80, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_NUT_UPGRADE_30, OBJECT_GI_NUTS, GID_NUTS, 0xA7, 0x80, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_NUT_UPGRADE_40, OBJECT_GI_NUTS, GID_NUTS, 0xA8, 0x80, CHEST_ANIM_SHORT),
    GET_ITEM(ITEM_BULLET_BAG_50, OBJECT_GI_DEKUPOUCH, GID_BULLET_BAG_50, 0x6C, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_ARROW_ICE, OBJECT_GI_M_ARROW, GID_ARROW_ICE, 0x3C, 0x80, CHEST_ANIM_LONG), // Ice Traps
    GET_ITEM_NONE,

    GET_ITEM(ITEM_MEDALLION_LIGHT, OBJECT_GI_MEDAL, GID_MEDALLION_LIGHT, 0x40, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_MEDALLION_FOREST, OBJECT_GI_MEDAL, GID_MEDALLION_FOREST, 0x3E, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_MEDALLION_FIRE, OBJECT_GI_MEDAL, GID_MEDALLION_FIRE, 0x3C, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_MEDALLION_WATER, OBJECT_GI_MEDAL, GID_MEDALLION_WATER, 0x3D, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_MEDALLION_SHADOW, OBJECT_GI_MEDAL, GID_MEDALLION_SHADOW, 0x41, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_MEDALLION_SPIRIT, OBJECT_GI_MEDAL, GID_MEDALLION_SPIRIT, 0x3F, 0x80, CHEST_ANIM_LONG),

    GET_ITEM(ITEM_KOKIRI_EMERALD, OBJECT_GI_JEWEL, GID_KOKIRI_EMERALD, 0x80, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_GORON_RUBY, OBJECT_GI_JEWEL, GID_GORON_RUBY, 0x81, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_ZORA_SAPPHIRE, OBJECT_GI_JEWEL, GID_ZORA_SAPPHIRE, 0x82, 0x80, CHEST_ANIM_LONG),

    GET_ITEM(ITEM_SONG_LULLABY, OBJECT_GI_MELODY, GID_SONG_ZELDA, 0xD4, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_SONG_SUN, OBJECT_GI_MELODY, GID_SONG_SUN, 0xD3, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_SONG_EPONA, OBJECT_GI_MELODY, GID_SONG_EPONA, 0xD2, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_SONG_STORMS, OBJECT_GI_MELODY, GID_SONG_STORM, 0xD6, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_SONG_TIME, OBJECT_GI_MELODY, GID_SONG_TIME, 0xD5, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_SONG_SARIA, OBJECT_GI_MELODY, GID_SONG_SARIA, 0xD1, 0x80, CHEST_ANIM_LONG),

    GET_ITEM(ITEM_SONG_MINUET, OBJECT_GI_MELODY, GID_SONG_MINUET, 0x73, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_SONG_BOLERO, OBJECT_GI_MELODY, GID_SONG_BOLERO, 0x74, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_SONG_SERENADE, OBJECT_GI_MELODY, GID_SONG_SERENADE, 0x75, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_SONG_NOCTURNE, OBJECT_GI_MELODY, GID_SONG_NOCTURNE, 0x77, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_SONG_REQUIEM, OBJECT_GI_MELODY, GID_SONG_REQUIEM, 0x76, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_SONG_PRELUDE, OBJECT_GI_MELODY, GID_SONG_PRELUDE, 0x78, 0x80, CHEST_ANIM_LONG),

    GET_ITEM(ITEM_SINGLE_MAGIC, OBJECT_GI_MAGICPOT, GID_MAGIC_SMALL, 0xE4, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_DOUBLE_MAGIC, OBJECT_GI_MAGICPOT, GID_MAGIC_LARGE, 0xE8, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_DOUBLE_DEFENSE, OBJECT_GI_HEARTS, GID_HEART_CONTAINER, 0xE9, 0x80, CHEST_ANIM_LONG),

    GET_ITEM(ITEM_BOTTLE_WITH_RED_POTION, OBJECT_GI_LIQUID, GID_POTION_RED, 0x43, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_BOTTLE_WITH_GREEN_POTION, OBJECT_GI_LIQUID, GID_POTION_GREEN, 0x44, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_BOTTLE_WITH_BLUE_POTION, OBJECT_GI_LIQUID, GID_POTION_BLUE, 0x45, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_BOTTLE_WITH_FAIRY, OBJECT_GI_BOTTLE, GID_BOTTLE, 0x46, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_BOTTLE_WITH_FISH, OBJECT_GI_FISH, GID_FISH, 0x47, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_BOTTLE_WITH_BLUE_FIRE, OBJECT_GI_FIRE, GID_BLUE_FIRE, 0x5D, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_BOTTLE_WITH_BUGS, OBJECT_GI_INSECT, GID_BUG, 0x7A, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_BOTTLE_WITH_POE, OBJECT_GI_GHOST, GID_POE, 0x97, 0x80, CHEST_ANIM_LONG),
    GET_ITEM(ITEM_BOTTLE_WITH_BIG_POE, OBJECT_GI_GHOST, GID_BIG_POE, 0xF9, 0x80, CHEST_ANIM_LONG),

    GET_ITEM_NONE,
    GET_ITEM_NONE,
};

#define GET_PLAYER_ANIM(group, type) sPlayerAnimations[group * PLAYER_ANIMTYPE_MAX + type]

static LinkAnimationHeader* sPlayerAnimations[PLAYER_ANIMGROUP_MAX * PLAYER_ANIMTYPE_MAX] = {
    /* PLAYER_ANIMGROUP_STANDING_STILL */
    &gPlayerAnim_003240, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_003238, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_003238, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002BE0, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_003240, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_003240, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_WALKING */
    &gPlayerAnim_003290, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_003268, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_003268, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002BF8, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_003290, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_003290, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_RUNNING */
    &gPlayerAnim_003140, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_002B38, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_003138, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002B40, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_003140, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_003140, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_RUNNING_DAMAGED */
    &gPlayerAnim_002E98, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_0029E8, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_002E98, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_0029F0, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_002E98, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_002E98, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_IRON_BOOTS */
    &gPlayerAnim_002FB0, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_002FA8, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_002FB0, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002A40, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_002FB0, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_002FB0, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_FIGHTING_LEFT_OF_ENEMY */
    &gPlayerAnim_003220, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_002590, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_002590, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002BC0, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_003220, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_003220, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_FIGHTING_RIGHT_OF_ENEMY */
    &gPlayerAnim_003230, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_0025D0, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_0025D0, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002BD0, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_003230, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_003230, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_START_FIGHTING */
    &gPlayerAnim_002BB0, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_0031F8, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_0031F8, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002BB0, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_002BB0, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_002BB0, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_8 */
    &gPlayerAnim_003088, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_002A70, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_002A70, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_003088, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_003088, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_003088, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_OPEN_DOOR_ADULT_LEFT */
    &gPlayerAnim_002750, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_002748, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_002748, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002750, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_002750, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_002750, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_OPEN_DOOR_CHILD_LEFT */
    &gPlayerAnim_002330, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_002330, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_002330, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002330, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_002330, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_002330, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_OPEN_DOOR_ADULT_RIGHT */
    &gPlayerAnim_002760, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_002758, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_002758, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002760, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_002760, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_002760, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_OPEN_DOOR_CHILD_RIGHT */
    &gPlayerAnim_002338, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_002338, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_002338, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002338, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_002338, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_002338, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_HOLDING_OBJECT */
    &gPlayerAnim_002E08, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_002E00, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_002E00, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002E08, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_002E08, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_002E08, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_TALL_JUMP_LANDING */
    &gPlayerAnim_003028, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_003020, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_003020, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_003028, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_003028, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_003028, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_SHORT_JUMP_LANDING */
    &gPlayerAnim_003170, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_003168, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_003168, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_003170, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_003170, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_003170, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_ROLLING */
    &gPlayerAnim_003038, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_003030, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_003030, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002A68, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_003038, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_003038, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_ROLL_BONKING */
    &gPlayerAnim_002FC0, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_002FB8, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_002FB8, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002FC8, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_002FC0, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_002FC0, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_END_WALK_ON_LEFT_FOOT */
    &gPlayerAnim_003278, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_003270, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_003270, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002BE8, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_003278, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_003278, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_END_WALK_ON_RIGHT_FOOT */
    &gPlayerAnim_003288, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_003280, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_003280, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002BF0, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_003288, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_003288, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_START_DEFENDING */
    &gPlayerAnim_002EB8, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_002EA0, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_002EA0, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002EB8, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_0026C8, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_002EB8, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_DEFENDING */
    &gPlayerAnim_002ED8, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_002ED0, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_002ED0, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002ED8, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_0026D0, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_002ED8, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_END_DEFENDING */
    &gPlayerAnim_002EB0, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_002EA8, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_002EA8, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002EB0, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_002EB0, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_002EB0, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_END_SIDEWALKING */
    &gPlayerAnim_003190, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_003188, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_003188, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002B68, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_003190, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_003190, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_SIDEWALKING_LEFT */
    &gPlayerAnim_003178, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_002568, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_002568, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002B58, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_003178, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_003178, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_SIDEWALKING_RIGHT */
    &gPlayerAnim_003180, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_002570, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_002570, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002B60, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_003180, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_003180, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_SHUFFLE_TURN */
    &gPlayerAnim_002D60, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_002D58, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_002D58, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002D60, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_002D60, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_002D60, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_END_FIGHTING_LEFT_OF_ENEMY */
    &gPlayerAnim_002BB8, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_003218, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_003218, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002BB8, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_002BB8, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_002BB8, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_END_FIGHTING_RIGHT_OF_ENEMY */
    &gPlayerAnim_002BC8, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_003228, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_003228, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002BC8, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_002BC8, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_002BC8, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_THROWING_OBJECT */
    &gPlayerAnim_0031C8, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_0031C0, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_0031C0, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_0031C8, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_0031C8, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_0031C8, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_PUTTING_DOWN_OBJECT */
    &gPlayerAnim_003118, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_003110, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_003110, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_003118, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_003118, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_003118, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_BACKWALKING */
    &gPlayerAnim_002DE8, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_002DE8, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_002DE8, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002DE8, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_002DE8, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_002DE8, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_START_CHECKING_OR_SPEAKING */
    &gPlayerAnim_002E30, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_002E18, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_002E18, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002E30, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_002E30, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_002E30, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_CHECKING_OR_SPEAKING */
    &gPlayerAnim_002E40, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_002E38, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_002E38, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002E40, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_002E40, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_002E40, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_END_CHECKING_OR_SPEAKING */
    &gPlayerAnim_002E28, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_002E20, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_002E20, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002E28, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_002E28, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_002E28, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_END_PULL_OBJECT */
    &gPlayerAnim_0030C8, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_0030C0, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_0030C0, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_0030C8, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_0030C8, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_0030C8, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_PULL_OBJECT */
    &gPlayerAnim_0030D8, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_0030D0, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_0030D0, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_0030D8, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_0030D8, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_0030D8, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_PUSH_OBJECT */
    &gPlayerAnim_0030B8, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_0030B0, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_0030B0, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_0030B8, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_0030B8, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_0030B8, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_KNOCKED_FROM_CLIMBING */
    &gPlayerAnim_002F20, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_002F18, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_002F18, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002F20, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_002F20, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_002F20, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_HANGING_FROM_LEDGE */
    &gPlayerAnim_002FF0, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_002FE8, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_002FE8, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002FF0, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_002FF0, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_002FF0, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_CLIMBING_IDLE */
    &gPlayerAnim_003010, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_003008, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_003008, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_003010, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_003010, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_003010, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_CLIMBING */
    &gPlayerAnim_003000, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_002FF8, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_002FF8, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_003000, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_003000, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_003000, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_SLIDING_DOWN_SLOPE */
    &gPlayerAnim_002EF0, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_002EE8, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_002EE8, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_002EF8, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_002EF0, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_002EF0, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_END_SLIDING_DOWN_SLOPE */
    &gPlayerAnim_0031E0, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_0031D8, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_0031D8, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_0031E8, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_0031E0, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_0031E0, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    /* PLAYER_ANIMGROUP_RELAX */
    &gPlayerAnim_003468, // PLAYER_ANIMTYPE_DEFAULT
    &gPlayerAnim_003438, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
    &gPlayerAnim_003438, // PLAYER_ANIMTYPE_HOLDING_SHIELD
    &gPlayerAnim_003468, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
    &gPlayerAnim_003468, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
    &gPlayerAnim_003468, // PLAYER_ANIMTYPE_USED_EXPLOSIVE
};

static LinkAnimationHeader* D_80853D4C[][3] = {
    { &gPlayerAnim_002A28, &gPlayerAnim_002A38, &gPlayerAnim_002A30 },
    { &gPlayerAnim_002950, &gPlayerAnim_002960, &gPlayerAnim_002958 },
    { &gPlayerAnim_0029D0, &gPlayerAnim_0029E0, &gPlayerAnim_0029D8 },
    { &gPlayerAnim_002988, &gPlayerAnim_002998, &gPlayerAnim_002990 },
};

static LinkAnimationHeader* sIdleAnims[][2] = {
    { &gPlayerAnim_003248, &gPlayerAnim_003200 }, { &gPlayerAnim_003258, &gPlayerAnim_003210 },
    { &gPlayerAnim_003250, &gPlayerAnim_003208 }, { &gPlayerAnim_003250, &gPlayerAnim_003208 },
    { &gPlayerAnim_003430, &gPlayerAnim_0033F0 }, { &gPlayerAnim_003430, &gPlayerAnim_0033F0 },
    { &gPlayerAnim_003430, &gPlayerAnim_0033F0 }, { &gPlayerAnim_0033F8, &gPlayerAnim_0033D0 },
    { &gPlayerAnim_003400, &gPlayerAnim_0033D8 }, { &gPlayerAnim_003420, &gPlayerAnim_003420 },
    { &gPlayerAnim_003408, &gPlayerAnim_0033E0 }, { &gPlayerAnim_003410, &gPlayerAnim_0033E8 },
    { &gPlayerAnim_003418, &gPlayerAnim_003418 }, { &gPlayerAnim_003428, &gPlayerAnim_003428 }
};

static PlayerAnimSfxEntry D_80853DEC[] = {
    { NA_SE_VO_LI_SNEEZE, -0x2008 },
};

static PlayerAnimSfxEntry D_80853DF0[] = {
    { NA_SE_VO_LI_SWEAT, -0x2012 },
};

static PlayerAnimSfxEntry D_80853DF4[] = {
    { NA_SE_VO_LI_BREATH_REST, -0x200D },
};

static PlayerAnimSfxEntry D_80853DF8[] = {
    { NA_SE_VO_LI_BREATH_REST, -0x200A },
};

static PlayerAnimSfxEntry D_80853DFC[] = {
    { NA_SE_PL_CALM_HIT, 0x82C }, { NA_SE_PL_CALM_HIT, 0x830 },  { NA_SE_PL_CALM_HIT, 0x834 },
    { NA_SE_PL_CALM_HIT, 0x838 }, { NA_SE_PL_CALM_HIT, -0x83C },
};

static PlayerAnimSfxEntry D_80853E10[] = {
    { 0, 0x4019 }, { 0, 0x401E }, { 0, 0x402C }, { 0, 0x4030 }, { 0, 0x4034 }, { 0, -0x4038 },
};

static PlayerAnimSfxEntry D_80853E28[] = {
    { NA_SE_IT_SHIELD_POSTURE, 0x810 },
    { NA_SE_IT_SHIELD_POSTURE, 0x814 },
    { NA_SE_IT_SHIELD_POSTURE, -0x846 },
};

static PlayerAnimSfxEntry D_80853E34[] = {
    { NA_SE_IT_HAMMER_SWING, 0x80A },
    { NA_SE_VO_LI_AUTO_JUMP, 0x200A },
    { NA_SE_IT_SWORD_SWING, 0x816 },
    { NA_SE_VO_LI_SWORD_N, -0x2016 },
};

static PlayerAnimSfxEntry D_80853E44[] = {
    { NA_SE_IT_SWORD_SWING, 0x827 },
    { NA_SE_VO_LI_SWORD_N, -0x2027 },
};

static PlayerAnimSfxEntry D_80853E4C[] = {
    { NA_SE_VO_LI_RELAX, -0x2014 },
};

static PlayerAnimSfxEntry* D_80853E50[] = {
    D_80853DEC, D_80853DF0, D_80853DF4, D_80853DF8, D_80853DFC, D_80853E10,
    D_80853E28, D_80853E34, D_80853E44, D_80853E4C, NULL,
};

static u8 D_80853E7C[] = {
    0, 0, 1, 1, 2, 2, 2, 2, 10, 10, 10, 10, 10, 10, 3, 3, 4, 4, 8, 8, 5, 5, 6, 6, 7, 7, 9, 9, 0,
};

// Used to map item IDs to action params
static s8 sItemActionParams[] = {
    PLAYER_AP_STICK,
    PLAYER_AP_NUT,
    PLAYER_AP_BOMB,
    PLAYER_AP_BOW,
    PLAYER_AP_BOW_FIRE,
    PLAYER_AP_DINS_FIRE,
    PLAYER_AP_SLINGSHOT,
    PLAYER_AP_OCARINA_FAIRY,
    PLAYER_AP_OCARINA_TIME,
    PLAYER_AP_BOMBCHU,
    PLAYER_AP_HOOKSHOT,
    PLAYER_AP_LONGSHOT,
    PLAYER_AP_BOW_ICE,
    PLAYER_AP_FARORES_WIND,
    PLAYER_AP_BOOMERANG,
    PLAYER_AP_LENS,
    PLAYER_AP_BEAN,
    PLAYER_AP_HAMMER,
    PLAYER_AP_BOW_LIGHT,
    PLAYER_AP_NAYRUS_LOVE,
    PLAYER_AP_BOTTLE,
    PLAYER_AP_BOTTLE_POTION_RED,
    PLAYER_AP_BOTTLE_POTION_GREEN,
    PLAYER_AP_BOTTLE_POTION_BLUE,
    PLAYER_AP_BOTTLE_FAIRY,
    PLAYER_AP_BOTTLE_FISH,
    PLAYER_AP_BOTTLE_MILK,
    PLAYER_AP_BOTTLE_LETTER,
    PLAYER_AP_BOTTLE_FIRE,
    PLAYER_AP_BOTTLE_BUG,
    PLAYER_AP_BOTTLE_BIG_POE,
    PLAYER_AP_BOTTLE_MILK_HALF,
    PLAYER_AP_BOTTLE_POE,
    PLAYER_AP_WEIRD_EGG,
    PLAYER_AP_CHICKEN,
    PLAYER_AP_LETTER_ZELDA,
    PLAYER_AP_MASK_KEATON,
    PLAYER_AP_MASK_SKULL,
    PLAYER_AP_MASK_SPOOKY,
    PLAYER_AP_MASK_BUNNY,
    PLAYER_AP_MASK_GORON,
    PLAYER_AP_MASK_ZORA,
    PLAYER_AP_MASK_GERUDO,
    PLAYER_AP_MASK_TRUTH,
    PLAYER_AP_SWORD_MASTER,
    PLAYER_AP_POCKET_EGG,
    PLAYER_AP_POCKET_CUCCO,
    PLAYER_AP_COJIRO,
    PLAYER_AP_ODD_MUSHROOM,
    PLAYER_AP_ODD_POTION,
    PLAYER_AP_SAW,
    PLAYER_AP_SWORD_BROKEN,
    PLAYER_AP_PRESCRIPTION,
    PLAYER_AP_FROG,
    PLAYER_AP_EYEDROPS,
    PLAYER_AP_CLAIM_CHECK,
    PLAYER_AP_BOW_FIRE,
    PLAYER_AP_BOW_ICE,
    PLAYER_AP_BOW_LIGHT,
    PLAYER_AP_SWORD_KOKIRI,
    PLAYER_AP_SWORD_MASTER,
    PLAYER_AP_SWORD_BGS,
    PLAYER_AP_SHIELD_DEKU,
    PLAYER_AP_SHIELD_HYLIAN,
    PLAYER_AP_SHIELD_MIRROR,
    PLAYER_AP_TUNIC_KOKIRI,
    PLAYER_AP_TUNIC_GORON,
    PLAYER_AP_TUNIC_ZORA,
    PLAYER_AP_BOOTS_KOKIRI,
    PLAYER_AP_BOOTS_IRON,
    PLAYER_AP_BOOTS_HOVER,
};

static u8 sMaskMemory;

static s32 (*sUpperBodyItemFuncs[])(Player* this, GlobalContext* globalCtx) = {
    Player_SetupStartZTargetDefend,  // PLAYER_AP_NONE,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_LAST_USED,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_FISHING_POLE,
    Player_SetupStartZTargetDefend2, // PLAYER_AP_SWORD_MASTER,
    Player_SetupStartZTargetDefend2, // PLAYER_AP_SWORD_KOKIRI,
    Player_SetupStartZTargetDefend2, // PLAYER_AP_SWORD_BGS,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_STICK,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_HAMMER,
    Player_HoldFpsItem,              // PLAYER_AP_BOW,
    Player_HoldFpsItem,              // PLAYER_AP_BOW_FIRE,
    Player_HoldFpsItem,              // PLAYER_AP_BOW_ICE,
    Player_HoldFpsItem,              // PLAYER_AP_BOW_LIGHT,
    Player_HoldFpsItem,              // PLAYER_AP_BOW_0C,
    Player_HoldFpsItem,              // PLAYER_AP_BOW_0D,
    Player_HoldFpsItem,              // PLAYER_AP_BOW_0E,
    Player_HoldFpsItem,              // PLAYER_AP_SLINGSHOT,
    Player_HoldFpsItem,              // PLAYER_AP_HOOKSHOT,
    Player_HoldFpsItem,              // PLAYER_AP_LONGSHOT,
    Player_HoldActor,                // PLAYER_AP_BOMB,
    Player_HoldActor,                // PLAYER_AP_BOMBCHU,
    Player_HoldBoomerang,            // PLAYER_AP_BOOMERANG,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_MAGIC_SPELL_15,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_MAGIC_SPELL_16,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_MAGIC_SPELL_17,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_FARORES_WIND,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_NAYRUS_LOVE,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_DINS_FIRE,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_NUT,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_OCARINA_FAIRY,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_OCARINA_TIME,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_BOTTLE,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_BOTTLE_FISH,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_BOTTLE_FIRE,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_BOTTLE_BUG,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_BOTTLE_POE,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_BOTTLE_BIG_POE,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_BOTTLE_LETTER,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_BOTTLE_POTION_RED,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_BOTTLE_POTION_BLUE,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_BOTTLE_POTION_GREEN,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_BOTTLE_MILK,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_BOTTLE_MILK_HALF,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_BOTTLE_FAIRY,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_LETTER_ZELDA,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_WEIRD_EGG,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_CHICKEN,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_BEAN,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_POCKET_EGG,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_POCKET_CUCCO,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_COJIRO,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_ODD_MUSHROOM,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_ODD_POTION,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_SAW,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_SWORD_BROKEN,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_PRESCRIPTION,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_FROG,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_EYEDROPS,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_CLAIM_CHECK,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_MASK_KEATON,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_MASK_SKULL,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_MASK_SPOOKY,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_MASK_BUNNY,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_MASK_GORON,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_MASK_ZORA,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_MASK_GERUDO,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_MASK_TRUTH,
    Player_SetupStartZTargetDefend,  // PLAYER_AP_LENS,
};

static void (*sItemChangeFuncs[])(GlobalContext* globalCtx, Player* this) = {
    Player_DoNothing,           // PLAYER_AP_NONE,
    Player_DoNothing,           // PLAYER_AP_LAST_USED,
    Player_DoNothing,           // PLAYER_AP_FISHING_POLE,
    Player_DoNothing,           // PLAYER_AP_SWORD_MASTER,
    Player_DoNothing,           // PLAYER_AP_SWORD_KOKIRI,
    Player_DoNothing,           // PLAYER_AP_SWORD_BGS,
    Player_SetupDekuStick,      // PLAYER_AP_STICK,
    Player_DoNothing2,          // PLAYER_AP_HAMMER,
    Player_SetupBowOrSlingshot, // PLAYER_AP_BOW,
    Player_SetupBowOrSlingshot, // PLAYER_AP_BOW_FIRE,
    Player_SetupBowOrSlingshot, // PLAYER_AP_BOW_ICE,
    Player_SetupBowOrSlingshot, // PLAYER_AP_BOW_LIGHT,
    Player_SetupBowOrSlingshot, // PLAYER_AP_BOW_0C,
    Player_SetupBowOrSlingshot, // PLAYER_AP_BOW_0D,
    Player_SetupBowOrSlingshot, // PLAYER_AP_BOW_0E,
    Player_SetupBowOrSlingshot, // PLAYER_AP_SLINGSHOT,
    Player_SetupHookshot,       // PLAYER_AP_HOOKSHOT,
    Player_SetupHookshot,       // PLAYER_AP_LONGSHOT,
    Player_SetupExplosive,      // PLAYER_AP_BOMB,
    Player_SetupExplosive,      // PLAYER_AP_BOMBCHU,
    Player_SetupBoomerang,      // PLAYER_AP_BOOMERANG,
    Player_DoNothing,           // PLAYER_AP_MAGIC_SPELL_15,
    Player_DoNothing,           // PLAYER_AP_MAGIC_SPELL_16,
    Player_DoNothing,           // PLAYER_AP_MAGIC_SPELL_17,
    Player_DoNothing,           // PLAYER_AP_FARORES_WIND,
    Player_DoNothing,           // PLAYER_AP_NAYRUS_LOVE,
    Player_DoNothing,           // PLAYER_AP_DINS_FIRE,
    Player_DoNothing,           // PLAYER_AP_NUT,
    Player_DoNothing,           // PLAYER_AP_OCARINA_FAIRY,
    Player_DoNothing,           // PLAYER_AP_OCARINA_TIME,
    Player_DoNothing,           // PLAYER_AP_BOTTLE,
    Player_DoNothing,           // PLAYER_AP_BOTTLE_FISH,
    Player_DoNothing,           // PLAYER_AP_BOTTLE_FIRE,
    Player_DoNothing,           // PLAYER_AP_BOTTLE_BUG,
    Player_DoNothing,           // PLAYER_AP_BOTTLE_POE,
    Player_DoNothing,           // PLAYER_AP_BOTTLE_BIG_POE,
    Player_DoNothing,           // PLAYER_AP_BOTTLE_LETTER,
    Player_DoNothing,           // PLAYER_AP_BOTTLE_POTION_RED,
    Player_DoNothing,           // PLAYER_AP_BOTTLE_POTION_BLUE,
    Player_DoNothing,           // PLAYER_AP_BOTTLE_POTION_GREEN,
    Player_DoNothing,           // PLAYER_AP_BOTTLE_MILK,
    Player_DoNothing,           // PLAYER_AP_BOTTLE_MILK_HALF,
    Player_DoNothing,           // PLAYER_AP_BOTTLE_FAIRY,
    Player_DoNothing,           // PLAYER_AP_LETTER_ZELDA,
    Player_DoNothing,           // PLAYER_AP_WEIRD_EGG,
    Player_DoNothing,           // PLAYER_AP_CHICKEN,
    Player_DoNothing,           // PLAYER_AP_BEAN,
    Player_DoNothing,           // PLAYER_AP_POCKET_EGG,
    Player_DoNothing,           // PLAYER_AP_POCKET_CUCCO,
    Player_DoNothing,           // PLAYER_AP_COJIRO,
    Player_DoNothing,           // PLAYER_AP_ODD_MUSHROOM,
    Player_DoNothing,           // PLAYER_AP_ODD_POTION,
    Player_DoNothing,           // PLAYER_AP_SAW,
    Player_DoNothing,           // PLAYER_AP_SWORD_BROKEN,
    Player_DoNothing,           // PLAYER_AP_PRESCRIPTION,
    Player_DoNothing,           // PLAYER_AP_FROG,
    Player_DoNothing,           // PLAYER_AP_EYEDROPS,
    Player_DoNothing,           // PLAYER_AP_CLAIM_CHECK,
    Player_DoNothing,           // PLAYER_AP_MASK_KEATON,
    Player_DoNothing,           // PLAYER_AP_MASK_SKULL,
    Player_DoNothing,           // PLAYER_AP_MASK_SPOOKY,
    Player_DoNothing,           // PLAYER_AP_MASK_BUNNY,
    Player_DoNothing,           // PLAYER_AP_MASK_GORON,
    Player_DoNothing,           // PLAYER_AP_MASK_ZORA,
    Player_DoNothing,           // PLAYER_AP_MASK_GERUDO,
    Player_DoNothing,           // PLAYER_AP_MASK_TRUTH,
    Player_DoNothing,           // PLAYER_AP_LENS,
};

typedef enum {
    /*  0 */ PLAYER_ITEM_CHANGE_DEFAULT,
    /*  1 */ PLAYER_ITEM_CHANGE_SHIELD_TO_1HAND,
    /*  2 */ PLAYER_ITEM_CHANGE_SHIELD_TO_2HAND,
    /*  3 */ PLAYER_ITEM_CHANGE_SHIELD,
    /*  4 */ PLAYER_ITEM_CHANGE_2HAND_TO_1HAND,
    /*  5 */ PLAYER_ITEM_CHANGE_1HAND,
    /*  6 */ PLAYER_ITEM_CHANGE_2HAND,
    /*  7 */ PLAYER_ITEM_CHANGE_2HAND_TO_2HAND,
    /*  8 */ PLAYER_ITEM_CHANGE_DEFAULT_2,
    /*  9 */ PLAYER_ITEM_CHANGE_1HAND_TO_BOMB,
    /* 10 */ PLAYER_ITEM_CHANGE_2HAND_TO_BOMB,
    /* 11 */ PLAYER_ITEM_CHANGE_BOMB,
    /* 12 */ PLAYER_ITEM_CHANGE_UNK_12,
    /* 13 */ PLAYER_ITEM_CHANGE_LEFT_HAND,
    /* 14 */ PLAYER_ITEM_CHANGE_MAX
} PlayersItemChangeAnimsIndex;

static ItemChangeAnimInfo sItemChangeAnimsInfo[PLAYER_ITEM_CHANGE_MAX] = {
    /* PLAYER_ITEM_CHANGE_DEFAULT */ { &gPlayerAnim_002F50, 12 },
    /* PLAYER_ITEM_CHANGE_SHIELD_TO_1HAND */ { &gPlayerAnim_003080, 6 },
    /* PLAYER_ITEM_CHANGE_SHIELD_TO_2HAND */ { &gPlayerAnim_002C68, 8 },
    /* PLAYER_ITEM_CHANGE_SHIELD */ { &gPlayerAnim_003090, 8 },
    /* PLAYER_ITEM_CHANGE_2HAND_TO_1HAND */ { &gPlayerAnim_002A20, 8 },
    /* PLAYER_ITEM_CHANGE_1HAND */ { &gPlayerAnim_002F30, 10 },
    /* PLAYER_ITEM_CHANGE_2HAND */ { &gPlayerAnim_002C58, 7 },
    /* PLAYER_ITEM_CHANGE_2HAND_TO_2HAND */ { &gPlayerAnim_002C60, 11 },
    /* PLAYER_ITEM_CHANGE_DEFAULT_2 */ { &gPlayerAnim_002F50, 12 },
    /* PLAYER_ITEM_CHANGE_1HAND_TO_BOMB */ { &gPlayerAnim_003078, 4 },
    /* PLAYER_ITEM_CHANGE_2HAND_TO_BOMB */ { &gPlayerAnim_003058, 4 },
    /* PLAYER_ITEM_CHANGE_BOMB */ { &gPlayerAnim_002F38, 4 },
    /* PLAYER_ITEM_CHANGE_UNK_12 */ { &gPlayerAnim_0024E0, 5 },
    /* PLAYER_ITEM_CHANGE_LEFT_HAND */ { &gPlayerAnim_002F48, 13 },
};

static s8 sAnimtypeToItemChangeAnims[PLAYER_ANIMTYPE_MAX][PLAYER_ANIMTYPE_MAX] = {
    // From: PLAYER_ANIMTYPE_DEFAULT, to:
    {
        PLAYER_ITEM_CHANGE_DEFAULT_2, // PLAYER_ANIMTYPE_DEFAULT
        -PLAYER_ITEM_CHANGE_1HAND,    // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
        -PLAYER_ITEM_CHANGE_SHIELD,   // PLAYER_ANIMTYPE_HOLDING_SHIELD
        -PLAYER_ITEM_CHANGE_2HAND,    // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
        PLAYER_ITEM_CHANGE_DEFAULT_2, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
        PLAYER_ITEM_CHANGE_BOMB       // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    },
    // From: PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON, to:
    {
        PLAYER_ITEM_CHANGE_1HAND,            // PLAYER_ANIMTYPE_DEFAULT
        PLAYER_ITEM_CHANGE_DEFAULT,          // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
        -PLAYER_ITEM_CHANGE_SHIELD_TO_1HAND, // PLAYER_ANIMTYPE_HOLDING_SHIELD
        PLAYER_ITEM_CHANGE_2HAND_TO_1HAND,   // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
        PLAYER_ITEM_CHANGE_1HAND,            // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
        PLAYER_ITEM_CHANGE_1HAND_TO_BOMB     // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    },
    // From: PLAYER_ANIMTYPE_HOLDING_SHIELD, to:
    {
        PLAYER_ITEM_CHANGE_SHIELD,          // PLAYER_ANIMTYPE_DEFAULT
        PLAYER_ITEM_CHANGE_SHIELD_TO_1HAND, // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
        PLAYER_ITEM_CHANGE_DEFAULT,         // PLAYER_ANIMTYPE_HOLDING_SHIELD
        PLAYER_ITEM_CHANGE_SHIELD_TO_2HAND, // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
        PLAYER_ITEM_CHANGE_SHIELD,          // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
        PLAYER_ITEM_CHANGE_1HAND_TO_BOMB    // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    },
    // From: PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON, to:
    {
        PLAYER_ITEM_CHANGE_2HAND,            // PLAYER_ANIMTYPE_DEFAULT
        -PLAYER_ITEM_CHANGE_2HAND_TO_1HAND,  // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
        -PLAYER_ITEM_CHANGE_SHIELD_TO_2HAND, // PLAYER_ANIMTYPE_HOLDING_SHIELD
        PLAYER_ITEM_CHANGE_2HAND_TO_2HAND,   // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
        PLAYER_ITEM_CHANGE_2HAND,            // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
        PLAYER_ITEM_CHANGE_2HAND_TO_BOMB     // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    },
    // From: PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND, to:
    {
        PLAYER_ITEM_CHANGE_DEFAULT_2, // PLAYER_ANIMTYPE_DEFAULT
        -PLAYER_ITEM_CHANGE_1HAND,    // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
        -PLAYER_ITEM_CHANGE_SHIELD,   // PLAYER_ANIMTYPE_HOLDING_SHIELD
        -PLAYER_ITEM_CHANGE_2HAND,    // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
        PLAYER_ITEM_CHANGE_DEFAULT_2, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
        PLAYER_ITEM_CHANGE_BOMB       // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    },
    // From: PLAYER_ANIMTYPE_USED_EXPLOSIVE, to:
    {
        PLAYER_ITEM_CHANGE_DEFAULT_2, // PLAYER_ANIMTYPE_DEFAULT
        -PLAYER_ITEM_CHANGE_1HAND,    // PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON
        -PLAYER_ITEM_CHANGE_SHIELD,   // PLAYER_ANIMTYPE_HOLDING_SHIELD
        -PLAYER_ITEM_CHANGE_2HAND,    // PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON
        PLAYER_ITEM_CHANGE_DEFAULT_2, // PLAYER_ANIMTYPE_HOLDING_ITEM_IN_LEFT_HAND
        PLAYER_ITEM_CHANGE_BOMB       // PLAYER_ANIMTYPE_USED_EXPLOSIVE
    },
};

static ExplosiveInfo sExplosiveInfos[] = {
    { ITEM_BOMB, ACTOR_EN_BOM },
    { ITEM_BOMBCHU, ACTOR_EN_BOM_CHU },
};

static MeleeAttackAnimInfo sMeleeAttackAnims[] = {
    /* PLAYER_MELEEATKTYPE_FORWARD_SLASH_1H */
    { &gPlayerAnim_002A80, &gPlayerAnim_002A90, &gPlayerAnim_002A88, 1, 4 },
    /* PLAYER_MELEEATKTYPE_FORWARD_SLASH_2H */
    { &gPlayerAnim_0028C0, &gPlayerAnim_0028C8, &gPlayerAnim_002498, 1, 4 },
    /* PLAYER_MELEEATKTYPE_FORWARD_COMBO_1H */
    { &gPlayerAnim_002A98, &gPlayerAnim_002AA0, &gPlayerAnim_002540, 0, 5 },
    /* PLAYER_MELEEATKTYPE_FORWARD_COMBO_2H */
    { &gPlayerAnim_0028D0, &gPlayerAnim_0028D8, &gPlayerAnim_0024A0, 1, 7 },
    /* PLAYER_MELEEATKTYPE_LEFT_SLASH_1H */
    { &gPlayerAnim_002968, &gPlayerAnim_002970, &gPlayerAnim_0024C0, 1, 4 },
    /* PLAYER_MELEEATKTYPE_LEFT_SLASH_2H */
    { &gPlayerAnim_002880, &gPlayerAnim_002888, &gPlayerAnim_002478, 0, 5 },
    /* PLAYER_MELEEATKTYPE_LEFT_COMBO_1H */
    { &gPlayerAnim_002978, &gPlayerAnim_002980, &gPlayerAnim_0024C8, 2, 8 },
    /* PLAYER_MELEEATKTYPE_LEFT_COMBO_2H */
    { &gPlayerAnim_002890, &gPlayerAnim_002898, &gPlayerAnim_002480, 3, 8 },
    /* PLAYER_MELEEATKTYPE_RIGHT_SLASH_1H */
    { &gPlayerAnim_0029A0, &gPlayerAnim_0029A8, &gPlayerAnim_0024D0, 0, 4 },
    /* PLAYER_MELEEATKTYPE_RIGHT_SLASH_2H */
    { &gPlayerAnim_0028A0, &gPlayerAnim_0028A8, &gPlayerAnim_002488, 0, 5 },
    /* PLAYER_MELEEATKTYPE_RIGHT_COMBO_1H */
    { &gPlayerAnim_0029B0, &gPlayerAnim_0029B8, &gPlayerAnim_0024D8, 0, 6 },
    /* PLAYER_MELEEATKTYPE_RIGHT_COMBO_2H */
    { &gPlayerAnim_0028B0, &gPlayerAnim_0028B8, &gPlayerAnim_002490, 1, 5 },
    /* PLAYER_MELEEATKTYPE_STAB_1H */
    { &gPlayerAnim_002AA8, &gPlayerAnim_002AB0, &gPlayerAnim_002548, 0, 3 },
    /* PLAYER_MELEEATKTYPE_STAB_2H */
    { &gPlayerAnim_0028E0, &gPlayerAnim_0028E8, &gPlayerAnim_0024A8, 0, 3 },
    /* PLAYER_MELEEATKTYPE_STAB_COMBO_1H */
    { &gPlayerAnim_002AB8, &gPlayerAnim_002AC0, &gPlayerAnim_002550, 1, 9 },
    /* PLAYER_MELEEATKTYPE_STAB_COMBO_2H */
    { &gPlayerAnim_0028F0, &gPlayerAnim_0028F8, &gPlayerAnim_0024B0, 1, 8 },
    /* PLAYER_MELEEATKTYPE_FLIPSLASH_START */
    { &gPlayerAnim_002A60, &gPlayerAnim_002A50, &gPlayerAnim_002A50, 1, 10 },
    /* PLAYER_MELEEATKTYPE_JUMPSLASH_START */
    { &gPlayerAnim_002900, &gPlayerAnim_002910, &gPlayerAnim_002910, 1, 11 },
    /* PLAYER_MELEEATKTYPE_FLIPSLASH_FINISH */
    { &gPlayerAnim_002A50, &gPlayerAnim_002A58, &gPlayerAnim_002A58, 1, 2 },
    /* PLAYER_MELEEATKTYPE_JUMPSLASH_FINISH */
    { &gPlayerAnim_002910, &gPlayerAnim_002908, &gPlayerAnim_002908, 1, 2 },
    /* PLAYER_MELEEATKTYPE_BACKSLASH_RIGHT */
    { &gPlayerAnim_002B80, &gPlayerAnim_002B88, &gPlayerAnim_002B88, 1, 5 },
    /* PLAYER_MELEEATKTYPE_BACKSLASH_LEFT */
    { &gPlayerAnim_002B70, &gPlayerAnim_002B78, &gPlayerAnim_002B78, 1, 4 },
    /* PLAYER_MELEEATKTYPE_HAMMER_FORWARD */
    { &gPlayerAnim_002C40, &gPlayerAnim_002C50, &gPlayerAnim_002C48, 3, 10 },
    /* PLAYER_MELEEATKTYPE_HAMMER_SIDE */
    { &gPlayerAnim_002C70, &gPlayerAnim_002C80, &gPlayerAnim_002C78, 2, 11 },
    /* PLAYER_MELEEATKTYPE_SPIN_ATTACK_1H */
    { &gPlayerAnim_002B28, &gPlayerAnim_002B30, &gPlayerAnim_002560, 0, 12 },
    /* PLAYER_MELEEATKTYPE_SPIN_ATTACK_2H */
    { &gPlayerAnim_002940, &gPlayerAnim_002948, &gPlayerAnim_0024B8, 0, 15 },
    /* PLAYER_MELEEATKTYPE_BIG_SPIN_1H */
    { &gPlayerAnim_0029C0, &gPlayerAnim_0029C8, &gPlayerAnim_002560, 0, 16 },
    /* PLAYER_MELEEATKTYPE_BIG_SPIN_2H */
    { &gPlayerAnim_0029C0, &gPlayerAnim_0029C8, &gPlayerAnim_0024B8, 0, 16 },
};

static LinkAnimationHeader* sSpinAttackAnims2[] = {
    &gPlayerAnim_002AE8,
    &gPlayerAnim_002920,
};

static LinkAnimationHeader* sSpinAttackAnims1[] = {
    &gPlayerAnim_002AE0,
    &gPlayerAnim_002920,
};

static LinkAnimationHeader* sSpinAttackChargeAnims[] = {
    &gPlayerAnim_002AF0,
    &gPlayerAnim_002928,
};

static LinkAnimationHeader* sCancelSpinAttackChargeAnims[] = {
    &gPlayerAnim_002AF8,
    &gPlayerAnim_002930,
};

static LinkAnimationHeader* sSpinAttackChargeWalkAnims[] = {
    &gPlayerAnim_002B00,
    &gPlayerAnim_002938,
};

static LinkAnimationHeader* sSpinAttackChargeSidewalkAnims[] = {
    &gPlayerAnim_002AD8,
    &gPlayerAnim_002918,
};

static u8 D_80854380[2] = { 0x18, 0x19 };
static u8 D_80854384[2] = { 0x1A, 0x1B };

static u16 sUseItemButtons[] = { BTN_B, BTN_CLEFT, BTN_CDOWN, BTN_CRIGHT, BTN_DUP, BTN_DDOWN, BTN_DLEFT, BTN_DRIGHT };

static u8 sMagicSpellCosts[] = { 12, 24, 24, 12, 24, 12 };

static u16 sFpsItemReadySfx[] = { NA_SE_IT_BOW_DRAW, NA_SE_IT_SLING_DRAW, NA_SE_IT_HOOKSHOT_READY };

static u8 sMagicArrowCosts[] = { 4, 4, 8 };

static LinkAnimationHeader* sRightDefendStandingAnims[] = {
    &gPlayerAnim_0025C0,
    &gPlayerAnim_0025C8,
};

static LinkAnimationHeader* sLeftDefendStandingAnims[] = {
    &gPlayerAnim_002580,
    &gPlayerAnim_002588,
};

static LinkAnimationHeader* sLeftStandingDeflectWithShieldAnims[] = {
    &gPlayerAnim_002510,
    &gPlayerAnim_002518,
};

static LinkAnimationHeader* sRightStandingDeflectWithShieldAnims[] = {
    &gPlayerAnim_002510,
    &gPlayerAnim_002520,
};

static LinkAnimationHeader* sDeflectWithShieldAnims[] = {
    &gPlayerAnim_002EC0,
    &gPlayerAnim_002A08,
};

static LinkAnimationHeader* sReadyFpsItemWhileWalkingAnims[] = {
    &gPlayerAnim_0026F0,
    &gPlayerAnim_002CC8,
};

static LinkAnimationHeader* sReadyFpsItemAnims[] = {
    &gPlayerAnim_0026C0,
    &gPlayerAnim_002CC0,
};

// return type can't be void due to regalloc in Player_CheckNoDebugModeCombo
s32 Player_StopMovement(Player* this) {
    this->actor.speedXZ = 0.0f;
    this->linearVelocity = 0.0f;
}

// return type can't be void due to regalloc in func_8083F72C
s32 Player_ClearAttentionModeAndStopMoving(Player* this) {
    Player_StopMovement(this);
    this->attentionMode = 0;
}

s32 Player_CheckActorTalkRequested(GlobalContext* globalCtx) {
    Player* this = GET_PLAYER(globalCtx);

    return CHECK_FLAG_ALL(this->actor.flags, ACTOR_FLAG_8);
}

void Player_PlayAnimOnce(GlobalContext* globalCtx, Player* this, LinkAnimationHeader* anim) {
    LinkAnimation_PlayOnce(globalCtx, &this->skelAnime, anim);
}

void Player_PlayAnimLoop(GlobalContext* globalCtx, Player* this, LinkAnimationHeader* anim) {
    LinkAnimation_PlayLoop(globalCtx, &this->skelAnime, anim);
}

void Player_PlayAnimLoopSlowed(GlobalContext* globalCtx, Player* this, LinkAnimationHeader* anim) {
    LinkAnimation_PlayLoopSetSpeed(globalCtx, &this->skelAnime, anim, 2.0f / 3.0f);
}

void Player_PlayAnimOnceSlowed(GlobalContext* globalCtx, Player* this, LinkAnimationHeader* anim) {
    LinkAnimation_PlayOnceSetSpeed(globalCtx, &this->skelAnime, anim, 2.0f / 3.0f);
}

void Player_AddRootYawToShapeYaw(Player* this) {
    this->actor.shape.rot.y += this->skelAnime.jointTable[1].y;
    this->skelAnime.jointTable[1].y = 0;
}

void Player_InactivateMeleeWeapon(Player* this) {
    this->stateFlags2 &= ~PLAYER_STATE2_RELEASING_SPIN_ATTACK;
    this->swordState = 0;
    this->swordInfo[0].active = this->swordInfo[1].active = this->swordInfo[2].active = 0;
}

void Player_ResetSubCam(GlobalContext* globalCtx, Player* this) {
    Camera* camera;

    if (this->subCamId != SUBCAM_NONE) {
        camera = globalCtx->cameraPtrs[this->subCamId];
        if ((camera != NULL) && (camera->csId == 1100)) {
            OnePointCutscene_EndCutscene(globalCtx, this->subCamId);
            this->subCamId = SUBCAM_NONE;
        }
    }

    this->stateFlags2 &= ~(PLAYER_STATE2_DIVING | PLAYER_STATE2_ENABLE_DIVE_CAMERA_AND_TIMER);
}

void Player_DetatchHeldActor(GlobalContext* globalCtx, Player* this) {
    Actor* heldActor = this->heldActor;

    if ((heldActor != NULL) && !Player_HoldsHookshot(this)) {
        this->actor.child = NULL;
        this->heldActor = NULL;
        this->interactRangeActor = NULL;
        heldActor->parent = NULL;
        this->stateFlags1 &= ~PLAYER_STATE1_HOLDING_ACTOR;
    }

    if (Player_GetExplosiveHeld(this) >= 0) {
        Player_ChangeItem(globalCtx, this, PLAYER_AP_NONE);
        this->heldItemId = ITEM_NONE_FE;
    }
}

void Player_ResetAttributes(GlobalContext* globalCtx, Player* this) {
    if ((this->stateFlags1 & PLAYER_STATE1_HOLDING_ACTOR) && (this->heldActor == NULL)) {
        if (this->interactRangeActor != NULL) {
            if (this->getItemId == GI_NONE) {
                this->stateFlags1 &= ~PLAYER_STATE1_HOLDING_ACTOR;
                this->interactRangeActor = NULL;
            }
        } else {
            this->stateFlags1 &= ~PLAYER_STATE1_HOLDING_ACTOR;
        }
    }

    Player_InactivateMeleeWeapon(this);
    this->attentionMode = 0;

    Player_ResetSubCam(globalCtx, this);
    func_8005B1A4(Gameplay_GetCamera(globalCtx, 0));

    this->stateFlags1 &= ~(PLAYER_STATE1_HANGING_FROM_LEDGE_SLIP | PLAYER_STATE1_CLIMBING_ONTO_LEDGE |
                           PLAYER_STATE1_IN_FIRST_PERSON_MODE | PLAYER_STATE1_CLIMBING);
    this->stateFlags2 &=
        ~(PLAYER_STATE2_MOVING_PUSH_PULL_WALL | PLAYER_STATE2_RESTRAINED_BY_ENEMY | PLAYER_STATE2_INSIDE_CRAWLSPACE);

    this->actor.shape.rot.x = 0;
    this->actor.shape.yOffset = 0.0f;

    this->slashCounter = this->comboTimer = 0;
}

s32 Player_UnequipItem(GlobalContext* globalCtx, Player* this) {
    if (this->heldItemActionParam >= PLAYER_AP_FISHING_POLE) {
        Player_UseItem(globalCtx, this, ITEM_NONE);
        return 1;
    } else {
        return 0;
    }
}

void Player_ResetAttributesAndHeldActor(GlobalContext* globalCtx, Player* this) {
    Player_ResetAttributes(globalCtx, this);
    Player_DetatchHeldActor(globalCtx, this);
}

s32 Player_MashTimerThresholdExceeded(Player* this, s32 arg1, s32 arg2) {
    s16 temp = this->analogStickAngle - sAnalogStickAngle;

    this->genericTimer += arg1 + (s16)(ABS(temp) * fabsf(sAnalogStickDistance) * 2.5415802156203426e-06f);

    if (CHECK_BTN_ANY(sControlInput->press.button, BTN_A | BTN_B)) {
        this->genericTimer += 5;
    }

    return this->genericTimer > arg2;
}

void Player_SetFreezeFlashTimer(GlobalContext* globalCtx) {
    if (globalCtx->actorCtx.freezeFlashTimer == 0) {
        globalCtx->actorCtx.freezeFlashTimer = 1;
    }
}

void Player_RequestRumble(Player* this, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    if (this->actor.category == ACTORCAT_PLAYER) {
        func_800AA000(arg4, arg1, arg2, arg3);
    }
}

void Player_PlayVoiceSfxForAge(Player* this, u16 sfxId) {
    if (this->actor.category == ACTORCAT_PLAYER) {
        func_8002F7DC(&this->actor, sfxId + this->ageProperties->unk_92);
    } else {
        func_800F4190(&this->actor.projectedPos, sfxId);
    }
}

void Player_StopInterruptableSfx(Player* this) {
    u16* entry = &sInterruptableSfx[0];
    s32 i;

    for (i = 0; i < 4; i++) {
        Audio_StopSfxById((u16)(*entry + this->ageProperties->unk_92));
        entry++;
    }
}

u16 Player_GetMoveSfx(Player* this, u16 sfxId) {
    return sfxId + this->surfaceMaterial;
}

void Player_PlayMoveSfx(Player* this, u16 sfxId) {
    func_8002F7DC(&this->actor, Player_GetMoveSfx(this, sfxId));
}

u16 Player_GetMoveSfxForAge(Player* this, u16 sfxId) {
    return sfxId + this->surfaceMaterial + this->ageProperties->unk_94;
}

void Player_PlayMoveSfxForAge(Player* this, u16 sfxId) {
    func_8002F7DC(&this->actor, Player_GetMoveSfxForAge(this, sfxId));
}

void Player_PlayWalkSfx(Player* this, f32 arg1) {
    s32 sfxId;

    if (this->currentBoots == PLAYER_BOOTS_IRON) {
        sfxId = NA_SE_PL_WALK_HEAVYBOOTS;
    } else {
        sfxId = Player_GetMoveSfxForAge(this, NA_SE_PL_WALK_GROUND);
    }

    func_800F4010(&this->actor.projectedPos, sfxId, arg1);
}

void Player_PlayJumpSfx(Player* this) {
    s32 sfxId;

    if (this->currentBoots == PLAYER_BOOTS_IRON) {
        sfxId = NA_SE_PL_JUMP_HEAVYBOOTS;
    } else {
        sfxId = Player_GetMoveSfxForAge(this, NA_SE_PL_JUMP);
    }

    func_8002F7DC(&this->actor, sfxId);
}

void Player_PlayLandingSfx(Player* this) {
    s32 sfxId;

    if (this->currentBoots == PLAYER_BOOTS_IRON) {
        sfxId = NA_SE_PL_LAND_HEAVYBOOTS;
    } else {
        sfxId = Player_GetMoveSfxForAge(this, NA_SE_PL_LAND);
    }

    func_8002F7DC(&this->actor, sfxId);
}

void Player_PlayReactableSfx(Player* this, u16 sfxId) {
    func_8002F7DC(&this->actor, sfxId);
    this->stateFlags2 |= PLAYER_STATE2_MAKING_REACTABLE_NOISE;
}

void Player_PlayAnimSfx(Player* this, PlayerAnimSfxEntry* entry) {
    s32 data;
    s32 flags;
    u32 cont;
    s32 pad;

    do {
        data = ABS(entry->field);
        flags = data & 0x7800;
        if (LinkAnimation_OnFrame(&this->skelAnime, fabsf(data & 0x7FF))) {
            if (flags == 0x800) {
                func_8002F7DC(&this->actor, entry->sfxId);
            } else if (flags == 0x1000) {
                Player_PlayMoveSfx(this, entry->sfxId);
            } else if (flags == 0x1800) {
                Player_PlayMoveSfxForAge(this, entry->sfxId);
            } else if (flags == 0x2000) {
                Player_PlayVoiceSfxForAge(this, entry->sfxId);
            } else if (flags == 0x2800) {
                Player_PlayLandingSfx(this);
            } else if (flags == 0x3000) {
                Player_PlayWalkSfx(this, 6.0f);
            } else if (flags == 0x3800) {
                Player_PlayJumpSfx(this);
            } else if (flags == 0x4000) {
                Player_PlayWalkSfx(this, 0.0f);
            } else if (flags == 0x4800) {
                func_800F4010(&this->actor.projectedPos, this->ageProperties->unk_94 + NA_SE_PL_WALK_LADDER, 0.0f);
            }
        }
        cont = (entry->field >= 0);
        entry++;
    } while (cont);
}

void Player_ChangeAnimMorphToLastFrame(GlobalContext* globalCtx, Player* this, LinkAnimationHeader* anim) {
    LinkAnimation_Change(globalCtx, &this->skelAnime, anim, 1.0f, 0.0f, Animation_GetLastFrame(anim), ANIMMODE_ONCE,
                         -6.0f);
}

void Player_ChangeAnimSlowedMorphToLastFrame(GlobalContext* globalCtx, Player* this, LinkAnimationHeader* anim) {
    LinkAnimation_Change(globalCtx, &this->skelAnime, anim, 2.0f / 3.0f, 0.0f, Animation_GetLastFrame(anim),
                         ANIMMODE_ONCE, -6.0f);
}

void Player_ChangeAnimShortMorphLoop(GlobalContext* globalCtx, Player* this, LinkAnimationHeader* anim) {
    LinkAnimation_Change(globalCtx, &this->skelAnime, anim, 1.0f, 0.0f, 0.0f, ANIMMODE_LOOP, -6.0f);
}

void Player_ChangeAnimOnce(GlobalContext* globalCtx, Player* this, LinkAnimationHeader* anim) {
    LinkAnimation_Change(globalCtx, &this->skelAnime, anim, 1.0f, 0.0f, 0.0f, ANIMMODE_ONCE, 0.0f);
}

void Player_ChangeAnimLongMorphLoop(GlobalContext* globalCtx, Player* this, LinkAnimationHeader* anim) {
    LinkAnimation_Change(globalCtx, &this->skelAnime, anim, 1.0f, 0.0f, 0.0f, ANIMMODE_LOOP, -16.0f);
}

s32 Player_LoopAnimContinuously(GlobalContext* globalCtx, Player* this, LinkAnimationHeader* anim) {
    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        Player_PlayAnimLoop(globalCtx, this, anim);
        return 1;
    } else {
        return 0;
    }
}

void Player_AnimUpdatePrevTranslRot(Player* this) {
    this->skelAnime.prevTransl = this->skelAnime.baseTransl;
    this->skelAnime.prevRot = this->actor.shape.rot.y;
}

void Player_AnimUpdatePrevTranslRotApplyAgeScale(Player* this) {
    Player_AnimUpdatePrevTranslRot(this);
    this->skelAnime.prevTransl.x *= this->ageProperties->unk_08;
    this->skelAnime.prevTransl.y *= this->ageProperties->unk_08;
    this->skelAnime.prevTransl.z *= this->ageProperties->unk_08;
}

void Player_ClearRootLimbPosY(Player* this) {
    this->skelAnime.jointTable[1].y = 0;
}

void Player_EndAnimMovement(Player* this) {
    if (this->skelAnime.moveFlags != 0) {
        Player_AddRootYawToShapeYaw(this);
        this->skelAnime.jointTable[0].x = this->skelAnime.baseTransl.x;
        this->skelAnime.jointTable[0].z = this->skelAnime.baseTransl.z;
        if (this->skelAnime.moveFlags & 8) {
            if (this->skelAnime.moveFlags & 2) {
                this->skelAnime.jointTable[0].y = this->skelAnime.prevTransl.y;
            }
        } else {
            this->skelAnime.jointTable[0].y = this->skelAnime.baseTransl.y;
        }
        Player_AnimUpdatePrevTranslRot(this);
        this->skelAnime.moveFlags = 0;
    }
}

void Player_UpdateAnimMovement(Player* this, s32 flags) {
    Vec3f pos;

    this->skelAnime.moveFlags = flags;
    this->skelAnime.prevTransl = this->skelAnime.baseTransl;
    SkelAnime_UpdateTranslation(&this->skelAnime, &pos, this->actor.shape.rot.y);

    if (flags & 1) {
        if (!LINK_IS_ADULT) {
            pos.x *= 0.64f;
            pos.z *= 0.64f;
        }
        this->actor.world.pos.x += pos.x * this->actor.scale.x;
        this->actor.world.pos.z += pos.z * this->actor.scale.z;
    }

    if (flags & 2) {
        if (!(flags & 4)) {
            pos.y *= this->ageProperties->unk_08;
        }
        this->actor.world.pos.y += pos.y * this->actor.scale.y;
    }

    Player_AddRootYawToShapeYaw(this);
}

void Player_SetupAnimMovement(GlobalContext* globalCtx, Player* this, s32 flags) {
    if (flags & 0x200) {
        Player_AnimUpdatePrevTranslRotApplyAgeScale(this);
    } else if ((flags & 0x100) || (this->skelAnime.moveFlags != 0)) {
        Player_AnimUpdatePrevTranslRot(this);
    } else {
        this->skelAnime.prevTransl = this->skelAnime.jointTable[0];
        this->skelAnime.prevRot = this->actor.shape.rot.y;
    }

    this->skelAnime.moveFlags = flags;
    Player_StopMovement(this);
    AnimationContext_DisableQueue(globalCtx);
}

void Player_PlayAnimOnceWithMovementSetSpeed(GlobalContext* globalCtx, Player* this, LinkAnimationHeader* anim,
                                             s32 flags, f32 playbackSpeed) {
    LinkAnimation_PlayOnceSetSpeed(globalCtx, &this->skelAnime, anim, playbackSpeed);
    Player_SetupAnimMovement(globalCtx, this, flags);
}

void Player_PlayAnimOnceWithMovement(GlobalContext* globalCtx, Player* this, LinkAnimationHeader* anim, s32 flags) {
    Player_PlayAnimOnceWithMovementSetSpeed(globalCtx, this, anim, flags, 1.0f);
}

void Player_PlayAnimOnceWithMovementSlowed(GlobalContext* globalCtx, Player* this, LinkAnimationHeader* anim,
                                           s32 flags) {
    Player_PlayAnimOnceWithMovementSetSpeed(globalCtx, this, anim, flags, 2.0f / 3.0f);
}

void Player_PlayAnimOnceWithMovementPresetFlagsSlowed(GlobalContext* globalCtx, Player* this,
                                                      LinkAnimationHeader* anim) {
    Player_PlayAnimOnceWithMovementSlowed(globalCtx, this, anim, 0x1C);
}

void Player_PlayAnimLoopWithMovementSetSpeed(GlobalContext* globalCtx, Player* this, LinkAnimationHeader* anim,
                                             s32 flags, f32 playbackSpeed) {
    LinkAnimation_PlayLoopSetSpeed(globalCtx, &this->skelAnime, anim, playbackSpeed);
    Player_SetupAnimMovement(globalCtx, this, flags);
}

void Player_PlayAnimLoopWithMovement(GlobalContext* globalCtx, Player* this, LinkAnimationHeader* anim, s32 flags) {
    Player_PlayAnimLoopWithMovementSetSpeed(globalCtx, this, anim, flags, 1.0f);
}

void Player_PlayAnimLoopWithMovementSlowed(GlobalContext* globalCtx, Player* this, LinkAnimationHeader* anim,
                                           s32 flags) {
    Player_PlayAnimLoopWithMovementSetSpeed(globalCtx, this, anim, flags, 2.0f / 3.0f);
}

void Player_PlayAnimLoopWithMovementPresetFlagsSlowed(GlobalContext* globalCtx, Player* this,
                                                      LinkAnimationHeader* anim) {
    Player_PlayAnimLoopWithMovementSlowed(globalCtx, this, anim, 0x1C);
}

void Player_StoreAnalogStickInput(GlobalContext* globalCtx, Player* this) {
    s8 scaledStickAngle;
    s8 scaledCamOffsetStickAngle;

    this->analogStickDistance = sAnalogStickDistance;
    this->analogStickAngle = sAnalogStickAngle;

    func_80077D10(&sAnalogStickDistance, &sAnalogStickAngle, sControlInput);

    sCameraOffsetAnalogStickAngle = Camera_GetInputDirYaw(GET_ACTIVE_CAM(globalCtx)) + sAnalogStickAngle;

    this->inputFrameCounter = (this->inputFrameCounter + 1) % 4;

    if (sAnalogStickDistance < 55.0f) {
        scaledCamOffsetStickAngle = -1;
        scaledStickAngle = -1;
    } else {
        scaledStickAngle = (u16)(sAnalogStickAngle + 0x2000) >> 9;
        scaledCamOffsetStickAngle =
            (u16)((s16)(sCameraOffsetAnalogStickAngle - this->actor.shape.rot.y) + 0x2000) >> 14;
    }

    this->analogStickInputs[this->inputFrameCounter] = scaledStickAngle;
    this->relativeAnalogStickInputs[this->inputFrameCounter] = scaledCamOffsetStickAngle;
}

void Player_PlayAnimOnceWithWaterInfluence(GlobalContext* globalCtx, Player* this, LinkAnimationHeader* linkAnim) {
    LinkAnimation_PlayOnceSetSpeed(globalCtx, &this->skelAnime, linkAnim, sWaterSpeedScale);
}

s32 Player_IsSwimming(Player* this) {
    return (this->stateFlags1 & PLAYER_STATE1_SWIMMING) && (this->currentBoots != PLAYER_BOOTS_IRON);
}

s32 Player_IsAimingBoomerang(Player* this) {
    return (this->stateFlags1 & PLAYER_STATE1_AIMING_BOOMERANG);
}

void Player_SetGetItemDrawIdPlusOne(Player* this, GlobalContext* globalCtx) {
    GetItemEntry* giEntry = &sGetItemTable[this->getItemId - 1];

    this->giDrawIdPlusOne = ABS(giEntry->gi);
}

static LinkAnimationHeader* Player_GetStandingStillAnim(Player* this) {
    return GET_PLAYER_ANIM(PLAYER_ANIMGROUP_STANDING_STILL, this->modelAnimType);
}

s32 Player_IsPlayingIdleAnim(Player* this) {
    LinkAnimationHeader** entry;
    s32 i;

    if (Player_GetStandingStillAnim(this) != this->skelAnime.animation) {
        for (i = 0, entry = &sIdleAnims[0][0]; i < 28; i++, entry++) {
            if (this->skelAnime.animation == *entry) {
                return i + 1;
            }
        }
        return 0;
    }

    return -1;
}

void Player_PlayIdleAnimSfx(Player* this, s32 arg1) {
    if (D_80853E7C[arg1] != 0) {
        Player_PlayAnimSfx(this, D_80853E50[D_80853E7C[arg1] - 1]);
    }
}

LinkAnimationHeader* Player_GetRunningAnim(Player* this) {
    if (this->runDamageTimer != 0) {
        return GET_PLAYER_ANIM(PLAYER_ANIMGROUP_RUNNING_DAMAGED, this->modelAnimType);
    } else if (!(this->stateFlags1 & (PLAYER_STATE1_SWIMMING | PLAYER_STATE1_IN_CUTSCENE)) &&
               (this->currentBoots == PLAYER_BOOTS_IRON)) {
        return GET_PLAYER_ANIM(PLAYER_ANIMGROUP_IRON_BOOTS, this->modelAnimType);
    } else {
        return GET_PLAYER_ANIM(PLAYER_ANIMGROUP_RUNNING, this->modelAnimType);
    }
}

s32 Player_IsAimingReadyBoomerang(Player* this) {
    return Player_IsAimingBoomerang(this) && (this->fpsItemTimer != 0);
}

LinkAnimationHeader* Player_GetFightingRightAnim(Player* this) {
    if (Player_IsAimingReadyBoomerang(this)) {
        return &gPlayerAnim_002638;
    } else {
        return GET_PLAYER_ANIM(PLAYER_ANIMGROUP_FIGHTING_RIGHT_OF_ENEMY, this->modelAnimType);
    }
}

LinkAnimationHeader* Player_GetFightingLeftAnim(Player* this) {
    if (Player_IsAimingReadyBoomerang(this)) {
        return &gPlayerAnim_002630;
    } else {
        return GET_PLAYER_ANIM(PLAYER_ANIMGROUP_FIGHTING_LEFT_OF_ENEMY, this->modelAnimType);
    }
}

LinkAnimationHeader* Player_GetEndSidewalkAnim(Player* this) {
    if (Actor_PlayerIsAimingReadyFpsItem(this)) {
        return &gPlayerAnim_0026E8;
    } else {
        return GET_PLAYER_ANIM(PLAYER_ANIMGROUP_END_SIDEWALKING, this->modelAnimType);
    }
}

LinkAnimationHeader* Player_GetSidewalkRightAnim(Player* this) {
    if (Player_IsAimingReadyBoomerang(this)) {
        return &gPlayerAnim_002620;
    } else {
        return GET_PLAYER_ANIM(PLAYER_ANIMGROUP_SIDEWALKING_RIGHT, this->modelAnimType);
    }
}

LinkAnimationHeader* Player_GetSidewalkLeftAnim(Player* this) {
    if (Player_IsAimingReadyBoomerang(this)) {
        return &gPlayerAnim_002618;
    } else {
        return GET_PLAYER_ANIM(PLAYER_ANIMGROUP_SIDEWALKING_LEFT, this->modelAnimType);
    }
}

void Player_SetUpperActionFunc(Player* this, PlayerUpperActionFunc arg1) {
    this->upperActionFunc = arg1;
    this->fpsItemShootState = 0;
    this->upperInterpWeight = 0.0f;
    Player_StopInterruptableSfx(this);
}

void Player_SetupChangeItemAnim(GlobalContext* globalCtx, Player* this, s8 actionParam) {
    LinkAnimationHeader* current = this->skelAnime.animation;
    LinkAnimationHeader** iter = sPlayerAnimations + this->modelAnimType;
    u32 i;

    this->stateFlags1 &= ~(PLAYER_STATE1_AIMING_FPS_ITEM | PLAYER_STATE1_AIMING_BOOMERANG);

    for (i = 0; i < PLAYER_ANIMGROUP_MAX; i++) {
        if (current == *iter) {
            break;
        }
        iter += PLAYER_ANIMTYPE_MAX;
    }

    Player_ChangeItem(globalCtx, this, actionParam);

    if (i < PLAYER_ANIMGROUP_MAX) {
        this->skelAnime.animation = GET_PLAYER_ANIM(i, this->modelAnimType);
    }
}

s8 Player_ItemToActionParam(s32 item) {
    if (item >= ITEM_NONE_FE) {
        return PLAYER_AP_NONE;
    } else if (item == ITEM_LAST_USED) {
        return PLAYER_AP_LAST_USED;
    } else if (item == ITEM_FISHING_POLE) {
        return PLAYER_AP_FISHING_POLE;
    } else {
        return sItemActionParams[item];
    }
}

void Player_DoNothing(GlobalContext* globalCtx, Player* this) {
}

void Player_SetupDekuStick(GlobalContext* globalCtx, Player* this) {
    this->unk_85C = 1.0f;
}

void Player_DoNothing2(GlobalContext* globalCtx, Player* this) {
}

void Player_SetupBowOrSlingshot(GlobalContext* globalCtx, Player* this) {
    this->stateFlags1 |= PLAYER_STATE1_AIMING_FPS_ITEM;

    if (this->heldItemActionParam != PLAYER_AP_SLINGSHOT) {
        this->fpsItemType = -1;
    } else {
        this->fpsItemType = -2;
    }
}

void Player_SetupExplosive(GlobalContext* globalCtx, Player* this) {
    s32 explosiveType;
    ExplosiveInfo* explosiveInfo;
    Actor* spawnedActor;

    if (this->stateFlags1 & PLAYER_STATE1_HOLDING_ACTOR) {
        Player_UnequipItem(globalCtx, this);
        return;
    }

    explosiveType = Player_GetExplosiveHeld(this);
    explosiveInfo = &sExplosiveInfos[explosiveType];

    spawnedActor = Actor_SpawnAsChild(&globalCtx->actorCtx, &this->actor, globalCtx, explosiveInfo->actorId,
                                      this->actor.world.pos.x, this->actor.world.pos.y, this->actor.world.pos.z, 0,
                                      this->actor.shape.rot.y, 0, 0);
    if (spawnedActor != NULL) {
        if ((explosiveType != 0) && (globalCtx->bombchuBowlingStatus != 0)) {
            globalCtx->bombchuBowlingStatus--;
            if (globalCtx->bombchuBowlingStatus == 0) {
                globalCtx->bombchuBowlingStatus = -1;
            }
        } else {
            Inventory_ChangeAmmo(explosiveInfo->itemId, -1);
        }

        this->interactRangeActor = spawnedActor;
        this->heldActor = spawnedActor;
        this->getItemId = GI_NONE;
        this->leftHandRot.y = spawnedActor->shape.rot.y - this->actor.shape.rot.y;
        this->stateFlags1 |= PLAYER_STATE1_HOLDING_ACTOR;
    }
}

void Player_SetupHookshot(GlobalContext* globalCtx, Player* this) {
    this->stateFlags1 |= PLAYER_STATE1_AIMING_FPS_ITEM;
    this->fpsItemType = -3;

    this->heldActor =
        Actor_SpawnAsChild(&globalCtx->actorCtx, &this->actor, globalCtx, ACTOR_ARMS_HOOK, this->actor.world.pos.x,
                           this->actor.world.pos.y, this->actor.world.pos.z, 0, this->actor.shape.rot.y, 0, 0);
}

void Player_SetupBoomerang(GlobalContext* globalCtx, Player* this) {
    this->stateFlags1 |= PLAYER_STATE1_AIMING_BOOMERANG;
}

void Player_ChangeItem(GlobalContext* globalCtx, Player* this, s8 actionParam) {
    this->fpsItemType = 0;
    this->unk_85C = 0.0f;
    this->unk_858 = 0.0f;

    this->heldItemActionParam = this->itemActionParam = actionParam;
    this->modelGroup = this->nextModelGroup;

    this->stateFlags1 &= ~(PLAYER_STATE1_AIMING_FPS_ITEM | PLAYER_STATE1_AIMING_BOOMERANG);

    sItemChangeFuncs[actionParam](globalCtx, this);

    Player_SetModelGroup(this, this->modelGroup);
}

void Player_MeleeAttack(Player* this, s32 newSwordState) {
    u16 itemSfx;
    u16 voiceSfx;

    if (this->swordState == 0) {
        if ((this->heldItemActionParam == PLAYER_AP_SWORD_BGS) && (gSaveContext.swordHealth > 0.0f)) {
            itemSfx = NA_SE_IT_HAMMER_SWING;
        } else {
            itemSfx = NA_SE_IT_SWORD_SWING;
        }

        voiceSfx = NA_SE_VO_LI_SWORD_N;
        if (this->heldItemActionParam == PLAYER_AP_HAMMER) {
            itemSfx = NA_SE_IT_HAMMER_SWING;
        } else if (this->swordAnimation >= 0x18) {
            itemSfx = 0;
            voiceSfx = NA_SE_VO_LI_SWORD_L;
        } else if (this->slashCounter >= 3) {
            itemSfx = NA_SE_IT_SWORD_SWING_HARD;
            voiceSfx = NA_SE_VO_LI_SWORD_L;
        }

        if (itemSfx != 0) {
            Player_PlayReactableSfx(this, itemSfx);
        }

        if ((this->swordAnimation < 0x10) || (this->swordAnimation >= 0x14)) {
            Player_PlayVoiceSfxForAge(this, voiceSfx);
        }
    }

    this->swordState = newSwordState;
}

s32 Player_IsFriendlyZTargeting(Player* this) {
    if (this->stateFlags1 & (PLAYER_STATE1_FORCE_STRAFING | PLAYER_STATE1_Z_TARGETING_FRIENDLY | PLAYER_STATE1_30)) {
        return 1;
    } else {
        return 0;
    }
}

s32 Player_SetupStartUnfriendlyZTargeting(Player* this) {
    if ((this->targetActor != NULL) && CHECK_FLAG_ALL(this->targetActor->flags, ACTOR_FLAG_0 | ACTOR_FLAG_2)) {
        this->stateFlags1 |= PLAYER_STATE1_Z_TARGETING_UNFRIENDLY;
        return 1;
    }

    if (this->stateFlags1 & PLAYER_STATE1_Z_TARGETING_UNFRIENDLY) {
        this->stateFlags1 &= ~PLAYER_STATE1_Z_TARGETING_UNFRIENDLY;
        if (this->linearVelocity == 0.0f) {
            this->currentYaw = this->actor.shape.rot.y;
        }
    }

    return 0;
}

s32 Player_IsZTargeting(Player* this) {
    return Player_IsUnfriendlyZTargeting(this) || Player_IsFriendlyZTargeting(this);
}

s32 Player_IsZTargetingSetupStartUnfriendly(Player* this) {
    return Player_SetupStartUnfriendlyZTargeting(this) || Player_IsFriendlyZTargeting(this);
}

void Player_ResetLeftRightBlendWeight(Player* this) {
    this->leftRightBlendWeight = this->leftRightBlendWeightTarget = 0.0f;
}

s32 Player_IsItemValid(Player* this, s32 item) {
    if ((item < ITEM_NONE_FE) && (Player_ItemToActionParam(item) == this->itemActionParam)) {
        return 1;
    } else {
        return 0;
    }
}

s32 Player_IsWearableMaskValid(s32 item1, s32 actionParam) {
    if ((item1 < ITEM_NONE_FE) && (Player_ItemToActionParam(item1) == actionParam)) {
        return 1;
    } else {
        return 0;
    }
}

s32 Player_GetButtonItem(GlobalContext* globalCtx, s32 index) {
    if (index >= ((CVar_GetS32("gDpadEquips", 0) != 0) ? 8 : 4)) {
        return ITEM_NONE;
    } else if (globalCtx->bombchuBowlingStatus != 0) {
        return (globalCtx->bombchuBowlingStatus > 0) ? ITEM_BOMBCHU : ITEM_NONE;
    } else if (index == 0) {
        return B_BTN_ITEM;
    } else if (index == 1) {
        return C_BTN_ITEM(0);
    } else if (index == 2) {
        return C_BTN_ITEM(1);
    } else if (index == 3) {
        return C_BTN_ITEM(2);
    } else if (index == 4) {
        return DPAD_ITEM(0);
    } else if (index == 5) {
        return DPAD_ITEM(1);
    } else if (index == 6) {
        return DPAD_ITEM(2);
    } else if (index == 7) {
        return DPAD_ITEM(3);
    }
}

void Player_SetupUseItem(Player* this, GlobalContext* globalCtx) {
    s32 maskActionParam;
    s32 item;
    s32 i;

    if (this->currentMask != PLAYER_MASK_NONE) {
        if (CVar_GetS32("gMMBunnyHood", 0) != 0) {
            s32 maskItem = this->currentMask - PLAYER_MASK_KEATON + ITEM_MASK_KEATON;
            bool hasOnDpad = false;
            if (CVar_GetS32("gDpadEquips", 0) != 0) {
                for (int buttonIndex = 4; buttonIndex < 8; buttonIndex++) {
                    hasOnDpad |= gSaveContext.equips.buttonItems[buttonIndex] == maskItem;
                }
            }

            if (gSaveContext.equips.buttonItems[0] != maskItem && gSaveContext.equips.buttonItems[1] != maskItem &&
                gSaveContext.equips.buttonItems[2] != maskItem && gSaveContext.equips.buttonItems[3] != maskItem &&
                !hasOnDpad) {
                this->currentMask = sMaskMemory = PLAYER_MASK_NONE;
                Player_PlayReactableSfx(this, NA_SE_PL_CHANGE_ARMS);
            }
        } else {
            maskActionParam = this->currentMask - 1 + PLAYER_AP_MASK_KEATON;
            bool hasOnDpad = false;
            if (CVar_GetS32("gDpadEquips", 0) != 0) {
                for (int buttonIndex = 0; buttonIndex < 4; buttonIndex++) {
                    hasOnDpad |= Player_IsWearableMaskValid(DPAD_ITEM(buttonIndex), maskActionParam);
                }
            }
            if (!Player_IsWearableMaskValid(C_BTN_ITEM(0), maskActionParam) && !Player_IsWearableMaskValid(C_BTN_ITEM(1), maskActionParam) &&
                !Player_IsWearableMaskValid(C_BTN_ITEM(2), maskActionParam) && !hasOnDpad) {
                this->currentMask = PLAYER_MASK_NONE;
            }
        }
    }

    if (!(this->stateFlags1 & (PLAYER_STATE1_HOLDING_ACTOR | PLAYER_STATE1_IN_CUTSCENE)) &&
        !Player_IsShootingHookshot(this)) {
        if (this->itemActionParam >= PLAYER_AP_FISHING_POLE) {
            bool hasOnDpad = false;
            if (CVar_GetS32("gDpadEquips", 0) != 0) {
                for (int buttonIndex = 0; buttonIndex < 4; buttonIndex++) {
                    hasOnDpad |= Player_IsItemValid(this, DPAD_ITEM(buttonIndex));
                }
            }
            if (!Player_IsItemValid(this, B_BTN_ITEM) && !Player_IsItemValid(this, C_BTN_ITEM(0)) &&
                !Player_IsItemValid(this, C_BTN_ITEM(1)) && !Player_IsItemValid(this, C_BTN_ITEM(2)) && !hasOnDpad) {
                Player_UseItem(globalCtx, this, ITEM_NONE);
                return;
            }
        }

        for (i = 0; i < ARRAY_COUNT(sUseItemButtons); i++) {
            if (CHECK_BTN_ALL(sControlInput->press.button, sUseItemButtons[i])) {
                break;
            }
        }

        item = Player_GetButtonItem(globalCtx, i);
        if (item >= ITEM_NONE_FE) {
            for (i = 0; i < ARRAY_COUNT(sUseItemButtons); i++) {
                if (CHECK_BTN_ALL(sControlInput->cur.button, sUseItemButtons[i])) {
                    break;
                }
            }

            item = Player_GetButtonItem(globalCtx, i);
            if ((item < ITEM_NONE_FE) && (Player_ItemToActionParam(item) == this->heldItemActionParam)) {
                sUsingItemAlreadyInHand2 = true;
            }
        } else {
            this->heldItemButton = i;
            Player_UseItem(globalCtx, this, item);
        }
    }
}

void Player_SetupStartChangeItem(Player* this, GlobalContext* globalCtx) {
    LinkAnimationHeader* anim;
    f32 frameCount;
    f32 startFrame;
    f32 endFrame;
    f32 playSpeed;
    s32 itemChangeAnim;
    s8 actionParam;
    s32 nextAnimType;

    actionParam = Player_ItemToActionParam(this->heldItemId);
    Player_SetUpperActionFunc(this, Player_StartChangeItem);

    nextAnimType = gPlayerModelTypes[this->nextModelGroup][PLAYER_MODELGROUPENTRY_ANIM];
    itemChangeAnim = sAnimtypeToItemChangeAnims[gPlayerModelTypes[this->modelGroup][PLAYER_MODELGROUPENTRY_ANIM]][nextAnimType];
    if ((actionParam == PLAYER_AP_BOTTLE) || (actionParam == PLAYER_AP_BOOMERANG) ||
        ((actionParam == PLAYER_AP_NONE) &&
         ((this->heldItemActionParam == PLAYER_AP_BOTTLE) || (this->heldItemActionParam == PLAYER_AP_BOOMERANG)))) {
        itemChangeAnim = (actionParam == PLAYER_AP_NONE) ? -PLAYER_ITEM_CHANGE_LEFT_HAND : PLAYER_ITEM_CHANGE_LEFT_HAND;
    }

    this->itemChangeAnim = ABS(itemChangeAnim);

    anim = sItemChangeAnimsInfo[this->itemChangeAnim].anim;
    if ((anim == &gPlayerAnim_002F30) && (this->currentShield == PLAYER_SHIELD_NONE)) {
        anim = &gPlayerAnim_002F40;
    }

    frameCount = Animation_GetLastFrame(anim);
    endFrame = frameCount;

    if (itemChangeAnim >= 0) {
        playSpeed = 1.2f;
        startFrame = 0.0f;
    } else {
        endFrame = 0.0f;
        playSpeed = -1.2f;
        startFrame = frameCount;
    }

    if (actionParam != PLAYER_AP_NONE) {
        playSpeed *= 2.0f;
    }

    LinkAnimation_Change(globalCtx, &this->skelAnimeUpper, anim, playSpeed, startFrame, endFrame, ANIMMODE_ONCE, 0.0f);

    this->stateFlags1 &= ~PLAYER_STATE1_START_CHANGE_ITEM;
}

void Player_SetupItem(Player* this, GlobalContext* globalCtx) {
    if ((this->actor.category == ACTORCAT_PLAYER) && !(this->stateFlags1 & PLAYER_STATE1_START_CHANGE_ITEM) &&
        ((this->heldItemActionParam == this->itemActionParam) || (this->stateFlags1 & PLAYER_STATE1_SHIELDING)) &&
        (gSaveContext.health != 0) && (globalCtx->csCtx.state == CS_STATE_IDLE) && (this->csMode == 0) &&
        (globalCtx->shootingGalleryStatus == 0) && (globalCtx->activeCamera == MAIN_CAM) &&
        (globalCtx->sceneLoadFlag != 0x14) && (gSaveContext.timer1State != 10)) {
        Player_SetupUseItem(this, globalCtx);
    }

    if (this->stateFlags1 & PLAYER_STATE1_START_CHANGE_ITEM) {
        Player_SetupStartChangeItem(this, globalCtx);
    }
}

s32 Player_GetFpsItemAmmo(GlobalContext* globalCtx, Player* this, s32* itemPtr, s32* typePtr) {
    if (LINK_IS_ADULT) {
        *itemPtr = ITEM_BOW;
        if (this->stateFlags1 & PLAYER_STATE1_RIDING_HORSE) {
            *typePtr = ARROW_NORMAL_HORSE;
        } else {
            *typePtr = this->heldItemActionParam - 6;
        }
    } else {
        *itemPtr = ITEM_SLINGSHOT;
        *typePtr = ARROW_SEED;
    }

    if (gSaveContext.minigameState == 1) {
        return globalCtx->interfaceCtx.hbaAmmo;
    } else if (globalCtx->shootingGalleryStatus != 0) {
        return globalCtx->shootingGalleryStatus;
    } else {
        return AMMO(*itemPtr);
    }
}

s32 Player_SetupReadyFpsItemToShoot(Player* this, GlobalContext* globalCtx) {
    s32 item;
    s32 arrowType;
    s32 magicArrowType;

    if ((this->heldItemActionParam >= PLAYER_AP_BOW_FIRE) && (this->heldItemActionParam <= PLAYER_AP_BOW_0E) &&
        (gSaveContext.unk_13F0 != 0)) {
        func_80078884(NA_SE_SY_ERROR);
    } else {
        Player_SetUpperActionFunc(this, Player_ReadyFpsItemToShoot);

        this->stateFlags1 |= PLAYER_STATE1_READY_TO_SHOOT;
        this->fpsItemTimer = 14;

        if (this->fpsItemType >= 0) {
            func_8002F7DC(&this->actor, sFpsItemReadySfx[ABS(this->fpsItemType) - 1]);

            if (!Player_HoldsHookshot(this) && (Player_GetFpsItemAmmo(globalCtx, this, &item, &arrowType) > 0)) {
                // Chaos
                if (CVar_GetS32("gForceNormalArrows", 0)) {
                    arrowType = ARROW_NORMAL;
                }

                magicArrowType = arrowType - ARROW_FIRE;
                
                if (this->fpsItemType >= 0) {
                    if ((magicArrowType >= 0) && (magicArrowType <= 2) &&
                        !func_80087708(globalCtx, sMagicArrowCosts[magicArrowType], 0)) {
                        arrowType = ARROW_NORMAL;
                    }

                    this->heldActor = Actor_SpawnAsChild(
                        &globalCtx->actorCtx, &this->actor, globalCtx, ACTOR_EN_ARROW, this->actor.world.pos.x,
                        this->actor.world.pos.y, this->actor.world.pos.z, 0, this->actor.shape.rot.y, 0, arrowType);
                }
            }
        }

        return 1;
    }

    return 0;
}

void Player_ChangeItemWithSfx(GlobalContext* globalCtx, Player* this) {
    if (this->heldItemActionParam != PLAYER_AP_NONE) {
        if (Player_GetSwordItemAP(this, this->heldItemActionParam) >= 0) {
            Player_PlayReactableSfx(this, NA_SE_IT_SWORD_PUTAWAY);
        } else {
            Player_PlayReactableSfx(this, NA_SE_PL_CHANGE_ARMS);
        }
    }

    Player_UseItem(globalCtx, this, this->heldItemId);

    if (Player_GetSwordItemAP(this, this->heldItemActionParam) >= 0) {
        Player_PlayReactableSfx(this, NA_SE_IT_SWORD_PICKOUT);
    } else if (this->heldItemActionParam != PLAYER_AP_NONE) {
        Player_PlayReactableSfx(this, NA_SE_PL_CHANGE_ARMS);
    }
}

void Player_SetupHeldItemUpperActionFunc(GlobalContext* globalCtx, Player* this) {
    if (Player_StartChangeItem == this->upperActionFunc) {
        Player_ChangeItemWithSfx(globalCtx, this);
    }

    Player_SetUpperActionFunc(this, sUpperBodyItemFuncs[this->heldItemActionParam]);
    this->fpsItemTimer = 0;
    this->idleCounter = 0;
    Player_DetatchHeldActor(globalCtx, this);
    this->stateFlags1 &= ~PLAYER_STATE1_START_CHANGE_ITEM;
}

LinkAnimationHeader* Player_GetStandingDefendAnim(GlobalContext* globalCtx, Player* this) {
    Player_SetUpperActionFunc(this, Player_StandingDefend);
    Player_DetatchHeldActor(globalCtx, this);

    if (this->leftRightBlendWeight < 0.5f) {
        return sRightDefendStandingAnims[Player_HoldsTwoHandedWeapon(this) &&
                                         !(CVar_GetS32("gShieldTwoHanded", 0) &&
                                           (this->heldItemActionParam != PLAYER_AP_STICK))];
    } else {
        return sLeftDefendStandingAnims[Player_HoldsTwoHandedWeapon(this) &&
                                        !(CVar_GetS32("gShieldTwoHanded", 0) &&
                                          (this->heldItemActionParam != PLAYER_AP_STICK))];
    }
}

s32 Player_StartZTargetDefend(GlobalContext* globalCtx, Player* this) {
    LinkAnimationHeader* anim;
    f32 frame;

    if (!(this->stateFlags1 & (PLAYER_STATE1_SHIELDING | PLAYER_STATE1_RIDING_HORSE | PLAYER_STATE1_IN_CUTSCENE)) &&
        (globalCtx->shootingGalleryStatus == 0) && (this->heldItemActionParam == this->itemActionParam) &&
        (this->currentShield != PLAYER_SHIELD_NONE) && !Player_IsChildWithHylianShield(this) && Player_IsZTargeting(this) &&
        CHECK_BTN_ALL(sControlInput->cur.button, BTN_R)) {

        anim = Player_GetStandingDefendAnim(globalCtx, this);
        frame = Animation_GetLastFrame(anim);
        LinkAnimation_Change(globalCtx, &this->skelAnimeUpper, anim, 1.0f, frame, frame, ANIMMODE_ONCE, 0.0f);
        func_8002F7DC(&this->actor, NA_SE_IT_SHIELD_POSTURE);

        return 1;
    } else {
        return 0;
    }
}

s32 Player_SetupStartZTargetDefend(Player* this, GlobalContext* globalCtx) {
    if (Player_StartZTargetDefend(globalCtx, this)) {
        return 1;
    } else {
        return 0;
    }
}

void Player_SetupEndDefend(Player* this) {
    Player_SetUpperActionFunc(this, Player_EndDefend);

    if (this->itemActionParam < 0) {
        Player_SetHeldItem(this);
    }

    Animation_Reverse(&this->skelAnimeUpper);
    func_8002F7DC(&this->actor, NA_SE_IT_SHIELD_REMOVE);
}

void Player_SetupChangeItem(GlobalContext* globalCtx, Player* this) {
    ItemChangeAnimInfo* ptr = &sItemChangeAnimsInfo[this->itemChangeAnim];
    f32 itemChangeFrame;

    itemChangeFrame = ptr->unk_04;
    itemChangeFrame = (this->skelAnimeUpper.playSpeed < 0.0f) ? itemChangeFrame - 1.0f : itemChangeFrame;

    if (LinkAnimation_OnFrame(&this->skelAnimeUpper, itemChangeFrame)) {
        Player_ChangeItemWithSfx(globalCtx, this);
    }

    Player_SetupStartUnfriendlyZTargeting(this);
}

s32 func_8083499C(Player* this, GlobalContext* globalCtx) {
    if (this->stateFlags1 & PLAYER_STATE1_START_CHANGE_ITEM) {
        Player_SetupStartChangeItem(this, globalCtx);
    } else {
        return 0;
    }

    return 1;
}

s32 Player_SetupStartZTargetDefend2(Player* this, GlobalContext* globalCtx) {
    if (Player_StartZTargetDefend(globalCtx, this) || func_8083499C(this, globalCtx)) {
        return 1;
    } else {
        return 0;
    }
}

s32 Player_StartChangeItem(Player* this, GlobalContext* globalCtx) {
    if (LinkAnimation_Update(globalCtx, &this->skelAnimeUpper) ||
        ((Player_ItemToActionParam(this->heldItemId) == this->heldItemActionParam) &&
         (sUsingItemAlreadyInHand =
              (sUsingItemAlreadyInHand || ((this->modelAnimType != PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON) &&
                                           (globalCtx->shootingGalleryStatus == 0)))))) {
        Player_SetUpperActionFunc(this, sUpperBodyItemFuncs[this->heldItemActionParam]);
        this->fpsItemTimer = 0;
        this->idleCounter = 0;
        sUsingItemAlreadyInHand2 = sUsingItemAlreadyInHand;
        return this->upperActionFunc(this, globalCtx);
    }

    if (Player_IsPlayingIdleAnim(this) != 0) {
        Player_SetupChangeItem(globalCtx, this);
        Player_PlayAnimOnce(globalCtx, this, Player_GetStandingStillAnim(this));
        this->idleCounter = 0;
    } else {
        Player_SetupChangeItem(globalCtx, this);
    }

    return 1;
}

s32 Player_StandingDefend(Player* this, GlobalContext* globalCtx) {
    LinkAnimation_Update(globalCtx, &this->skelAnimeUpper);

    if (!CHECK_BTN_ALL(sControlInput->cur.button, BTN_R)) {
        Player_SetupEndDefend(this);
        return 1;
    } else {
        this->stateFlags1 |= PLAYER_STATE1_SHIELDING;
        Player_SetModelsForHoldingShield(this);
        return 1;
    }
}

s32 Player_EndDeflectAttackStanding(Player* this, GlobalContext* globalCtx) {
    LinkAnimationHeader* anim;
    f32 frame;

    if (LinkAnimation_Update(globalCtx, &this->skelAnimeUpper)) {
        anim = Player_GetStandingDefendAnim(globalCtx, this);
        frame = Animation_GetLastFrame(anim);
        LinkAnimation_Change(globalCtx, &this->skelAnimeUpper, anim, 1.0f, frame, frame, ANIMMODE_ONCE, 0.0f);
    }

    this->stateFlags1 |= PLAYER_STATE1_SHIELDING;
    Player_SetModelsForHoldingShield(this);

    return 1;
}

s32 Player_EndDefend(Player* this, GlobalContext* globalCtx) {
    sUsingItemAlreadyInHand = sUsingItemAlreadyInHand2;

    if (sUsingItemAlreadyInHand || LinkAnimation_Update(globalCtx, &this->skelAnimeUpper)) {
        Player_SetUpperActionFunc(this, sUpperBodyItemFuncs[this->heldItemActionParam]);
        LinkAnimation_PlayLoop(globalCtx, &this->skelAnimeUpper, GET_PLAYER_ANIM(PLAYER_ANIMGROUP_STANDING_STILL, this->modelAnimType));
        this->idleCounter = 0;
        this->upperActionFunc(this, globalCtx);
        return 0;
    }

    return 1;
}

s32 Player_SetupUseFpsItem(Player* this, GlobalContext* globalCtx) {
    LinkAnimationHeader* anim;

    if (this->heldItemActionParam != PLAYER_AP_BOOMERANG) {
        if (!Player_SetupReadyFpsItemToShoot(this, globalCtx)) {
            return 0;
        }

        if (!Player_HoldsHookshot(this)) {
            anim = &gPlayerAnim_0026A0;
        } else {
            anim = &gPlayerAnim_002CA0;
        }
        LinkAnimation_PlayOnce(globalCtx, &this->skelAnimeUpper, anim);
    } else {
        Player_SetUpperActionFunc(this, Player_SetupAimBoomerang);
        this->fpsItemTimer = 10;
        LinkAnimation_PlayOnce(globalCtx, &this->skelAnimeUpper, &gPlayerAnim_002628);
    }

    if (this->stateFlags1 & PLAYER_STATE1_RIDING_HORSE) {
        Player_PlayAnimLoop(globalCtx, this, &gPlayerAnim_003380);
    } else if ((this->actor.bgCheckFlags & 1) && !Player_SetupStartUnfriendlyZTargeting(this)) {
        Player_PlayAnimLoop(globalCtx, this, GET_PLAYER_ANIM(PLAYER_ANIMGROUP_STANDING_STILL, this->modelAnimType));
    }

    return 1;
}

s32 Player_CheckShootingGalleryShootInput(GlobalContext* globalCtx) {
    return (globalCtx->shootingGalleryStatus > 0) && CHECK_BTN_ALL(sControlInput->press.button, BTN_B);
}

s32 func_80834E7C(GlobalContext* globalCtx) {
    u16 buttonsToCheck = BTN_A | BTN_B | BTN_CUP | BTN_CLEFT | BTN_CRIGHT | BTN_CDOWN;
    if (CVar_GetS32("gDpadEquips", 0) != 0) {
        buttonsToCheck |= BTN_DUP | BTN_DDOWN | BTN_DLEFT | BTN_DRIGHT;
    }
    return (globalCtx->shootingGalleryStatus != 0) &&
           ((globalCtx->shootingGalleryStatus < 0) || CHECK_BTN_ANY(sControlInput->cur.button, buttonsToCheck));
}

s32 Player_SetupAimAttention(Player* this, GlobalContext* globalCtx) {
    if ((this->attentionMode == PLAYER_ATTENTIONMODE_NONE) || (this->attentionMode == PLAYER_ATTENTIONMODE_AIMING)) {
        if (Player_IsZTargeting(this) || (Camera_CheckValidMode(Gameplay_GetCamera(globalCtx, 0), 7) == 0)) {
            return 1;
        }
        this->attentionMode = PLAYER_ATTENTIONMODE_AIMING;
    }

    return 0;
}

s32 Player_CanUseFpsItem(Player* this, GlobalContext* globalCtx) {
    if ((this->doorType == PLAYER_DOORTYPE_NONE) && !(this->stateFlags1 & PLAYER_STATE1_AWAITING_THROWN_BOOMERANG)) {
        if (sUsingItemAlreadyInHand || Player_CheckShootingGalleryShootInput(globalCtx)) {
            if (Player_SetupUseFpsItem(this, globalCtx)) {
                return Player_SetupAimAttention(this, globalCtx);
            }
        }
    }

    return 0;
}

s32 Player_EndHookshotMove(Player* this) {
    if (this->actor.child != NULL) {
        if (this->heldActor == NULL) {
            this->heldActor = this->actor.child;
            Player_RequestRumble(this, 255, 10, 250, 0);
            func_8002F7DC(&this->actor, NA_SE_IT_HOOKSHOT_RECEIVE);
        }

        return 1;
    }

    return 0;
}

s32 Player_HoldFpsItem(Player* this, GlobalContext* globalCtx) {
    if (this->fpsItemType >= 0) {
        this->fpsItemType = -this->fpsItemType;
    }

    if ((!Player_HoldsHookshot(this) || Player_EndHookshotMove(this)) && !Player_StartZTargetDefend(globalCtx, this) &&
        !Player_CanUseFpsItem(this, globalCtx)) {
        return 0;
    } else if (this->rideActor != NULL) {
        this->attentionMode = 2; // OTRTODO: THIS IS A BAD IDEA BUT IT FIXES THE HORSE FIRST PERSON?
    }

    return 1;
}

s32 Player_UpdateShotFpsItem(GlobalContext* globalCtx, Player* this) {
    s32 item;
    s32 arrowType;

    if (this->heldActor != NULL) {
        if (!Player_HoldsHookshot(this)) {
            Player_GetFpsItemAmmo(globalCtx, this, &item, &arrowType);

            if (gSaveContext.minigameState == 1) {
                globalCtx->interfaceCtx.hbaAmmo--;
            } else if (globalCtx->shootingGalleryStatus != 0) {
                globalCtx->shootingGalleryStatus--;
            } else {
                Inventory_ChangeAmmo(item, -1);
            }

            if (globalCtx->shootingGalleryStatus == 1) {
                globalCtx->shootingGalleryStatus = -10;
            }

            Player_RequestRumble(this, 150, 10, 150, 0);
        } else {
            Player_RequestRumble(this, 255, 20, 150, 0);
        }

        this->fpsItemShotTimer = 4;
        this->heldActor->parent = NULL;
        this->actor.child = NULL;
        this->heldActor = NULL;

        return 1;
    }

    return 0;
}

static u16 sFpsItemNoAmmoSfx[] = { NA_SE_IT_BOW_FLICK, NA_SE_IT_SLING_FLICK };

s32 Player_ReadyFpsItemToShoot(Player* this, GlobalContext* globalCtx) {
    s32 holdingHookshot;

    if (!Player_HoldsHookshot(this)) {
        holdingHookshot = 0;
    } else {
        holdingHookshot = 1;
    }

    Math_ScaledStepToS(&this->upperBodyRot.z, 1200, 400);
    this->lookFlags |= 0x100;

    if ((this->fpsItemShootState == 0) && (Player_IsPlayingIdleAnim(this) == 0) &&
        (this->skelAnime.animation == &gPlayerAnim_0026E8)) {
        LinkAnimation_PlayOnce(globalCtx, &this->skelAnimeUpper, sReadyFpsItemWhileWalkingAnims[holdingHookshot]);
        this->fpsItemShootState = -1;
    } else if (LinkAnimation_Update(globalCtx, &this->skelAnimeUpper)) {
        LinkAnimation_PlayLoop(globalCtx, &this->skelAnimeUpper, sReadyFpsItemAnims[holdingHookshot]);
        this->fpsItemShootState = 1;
    } else if (this->fpsItemShootState == 1) {
        this->fpsItemShootState = 2;
    }

    if (this->fpsItemTimer > 10) {
        this->fpsItemTimer--;
    }

    Player_SetupAimAttention(this, globalCtx);

    if ((this->fpsItemShootState > 0) &&
        ((this->fpsItemType < 0) || (!sUsingItemAlreadyInHand2 && !func_80834E7C(globalCtx)))) {
        Player_SetUpperActionFunc(this, Player_AimFpsItem);
        if (this->fpsItemType >= 0) {
            if (holdingHookshot == 0) {
                if (!Player_UpdateShotFpsItem(globalCtx, this)) {
                    func_8002F7DC(&this->actor, sFpsItemNoAmmoSfx[ABS(this->fpsItemType) - 1]);
                }
            } else if (this->actor.bgCheckFlags & 1) {
                Player_UpdateShotFpsItem(globalCtx, this);
            }
        }
        this->fpsItemTimer = 10;
        Player_StopMovement(this);
    } else {
        this->stateFlags1 |= PLAYER_STATE1_READY_TO_SHOOT;
    }

    return 1;
}

s32 Player_AimFpsItem(Player* this, GlobalContext* globalCtx) {
    LinkAnimation_Update(globalCtx, &this->skelAnimeUpper);

    if (Player_HoldsHookshot(this) && !Player_EndHookshotMove(this)) {
        return 1;
    }

    if (!Player_StartZTargetDefend(globalCtx, this) &&
        (sUsingItemAlreadyInHand || ((this->fpsItemType < 0) && sUsingItemAlreadyInHand2) || Player_CheckShootingGalleryShootInput(globalCtx))) {
        this->fpsItemType = ABS(this->fpsItemType);

        if (Player_SetupReadyFpsItemToShoot(this, globalCtx)) {
            if (Player_HoldsHookshot(this)) {
                this->fpsItemShootState = 1;
            } else {
                LinkAnimation_PlayOnce(globalCtx, &this->skelAnimeUpper, &gPlayerAnim_0026B8);
            }
        }
    } else {
        if (this->fpsItemTimer != 0) {
            this->fpsItemTimer--;
        }

        if (Player_IsZTargeting(this) || (this->attentionMode != 0) ||
            (this->stateFlags1 & PLAYER_STATE1_IN_FIRST_PERSON_MODE)) {
            if (this->fpsItemTimer == 0) {
                this->fpsItemTimer++;
            }
            return 1;
        }

        if (Player_HoldsHookshot(this)) {
            Player_SetUpperActionFunc(this, Player_HoldFpsItem);
        } else {
            Player_SetUpperActionFunc(this, Player_EndAimFpsItem);
            LinkAnimation_PlayOnce(globalCtx, &this->skelAnimeUpper, &gPlayerAnim_0026B0);
        }

        this->fpsItemTimer = 0;
    }

    return 1;
}

s32 Player_EndAimFpsItem(Player* this, GlobalContext* globalCtx) {
    if (!(this->actor.bgCheckFlags & 1) || LinkAnimation_Update(globalCtx, &this->skelAnimeUpper)) {
        Player_SetUpperActionFunc(this, Player_HoldFpsItem);
    }

    return 1;
}

void Player_SetZTargetFriendlyYaw(Player* this) {
    this->stateFlags1 |= PLAYER_STATE1_Z_TARGETING_FRIENDLY;

    if (!(this->skelAnime.moveFlags & 0x80) && (this->actor.bgCheckFlags & 0x200) && (sYawToTouchedWall < 0x2000)) {
        this->currentYaw = this->actor.shape.rot.y = this->actor.wallYaw + 0x8000;
    }

    this->targetYaw = this->actor.shape.rot.y;
}

s32 Player_InterruptHoldingActor(GlobalContext* globalCtx, Player* this, Actor* heldActor) {
    if (heldActor == NULL) {
        Player_ResetAttributesAndHeldActor(globalCtx, this);
        Player_SetupStandingStillType(this, globalCtx);
        return 1;
    }

    return 0;
}

void Player_SetupHoldActorUpperAction(Player* this, GlobalContext* globalCtx) {
    if (!Player_InterruptHoldingActor(globalCtx, this, this->heldActor)) {
        Player_SetUpperActionFunc(this, Player_HoldActor);
        LinkAnimation_PlayLoop(globalCtx, &this->skelAnimeUpper, &gPlayerAnim_002E10);
    }
}

s32 Player_HoldActor(Player* this, GlobalContext* globalCtx) {
    Actor* heldActor = this->heldActor;

    if (heldActor == NULL) {
        Player_SetupHeldItemUpperActionFunc(globalCtx, this);
    }

    if (Player_StartZTargetDefend(globalCtx, this)) {
        return 1;
    }

    if (this->stateFlags1 & PLAYER_STATE1_HOLDING_ACTOR) {
        if (LinkAnimation_Update(globalCtx, &this->skelAnimeUpper)) {
            LinkAnimation_PlayLoop(globalCtx, &this->skelAnimeUpper, &gPlayerAnim_002E10);
        }

        if ((heldActor->id == ACTOR_EN_NIW) && (this->actor.velocity.y <= 0.0f)) {
            this->actor.minVelocityY = -2.0f;
            this->actor.gravity = -0.5f;
            this->fallStartHeight = this->actor.world.pos.y;
        }

        return 1;
    }

    return Player_SetupStartZTargetDefend(this, globalCtx);
}

void Player_SetLeftHandDlists(Player* this, Gfx** dLists) {
    this->leftHandDLists = &dLists[gSaveContext.linkAge];
}

s32 Player_HoldBoomerang(Player* this, GlobalContext* globalCtx) {
    if (Player_StartZTargetDefend(globalCtx, this)) {
        return 1;
    }

    if (this->stateFlags1 & PLAYER_STATE1_AWAITING_THROWN_BOOMERANG) {
        Player_SetUpperActionFunc(this, Player_WaitForThrownBoomerang);
    } else if (Player_CanUseFpsItem(this, globalCtx)) {
        return 1;
    }

    return 0;
}

s32 Player_SetupAimBoomerang(Player* this, GlobalContext* globalCtx) {
    if (LinkAnimation_Update(globalCtx, &this->skelAnimeUpper)) {
        Player_SetUpperActionFunc(this, Player_AimBoomerang);
        LinkAnimation_PlayLoop(globalCtx, &this->skelAnimeUpper, &gPlayerAnim_002638);
    }

    Player_SetupAimAttention(this, globalCtx);

    return 1;
}

s32 Player_AimBoomerang(Player* this, GlobalContext* globalCtx) {
    LinkAnimationHeader* animSeg = this->skelAnime.animation;

    if ((Player_GetFightingRightAnim(this) == animSeg) || (Player_GetFightingLeftAnim(this) == animSeg) ||
        (Player_GetSidewalkRightAnim(this) == animSeg) || (Player_GetSidewalkLeftAnim(this) == animSeg)) {
        AnimationContext_SetCopyAll(globalCtx, this->skelAnime.limbCount, this->skelAnimeUpper.jointTable,
                                    this->skelAnime.jointTable);
    } else {
        LinkAnimation_Update(globalCtx, &this->skelAnimeUpper);
    }

    Player_SetupAimAttention(this, globalCtx);

    if (!sUsingItemAlreadyInHand2) {
        Player_SetUpperActionFunc(this, Player_ThrowBoomerang);
        LinkAnimation_PlayOnce(globalCtx, &this->skelAnimeUpper,
                               (this->leftRightBlendWeight < 0.5f) ? &gPlayerAnim_002608 : &gPlayerAnim_002600);
    }

    return 1;
}

s32 Player_ThrowBoomerang(Player* this, GlobalContext* globalCtx) {
    if (LinkAnimation_Update(globalCtx, &this->skelAnimeUpper)) {
        Player_SetUpperActionFunc(this, Player_WaitForThrownBoomerang);
        this->fpsItemTimer = 0;
    } else if (LinkAnimation_OnFrame(&this->skelAnimeUpper, 6.0f)) {
        f32 posX = (Math_SinS(this->actor.shape.rot.y) * 10.0f) + this->actor.world.pos.x;
        f32 posZ = (Math_CosS(this->actor.shape.rot.y) * 10.0f) + this->actor.world.pos.z;
        s32 yaw = (this->targetActor != NULL) ? this->actor.shape.rot.y + 14000 : this->actor.shape.rot.y;
        EnBoom* boomerang =
            (EnBoom*)Actor_Spawn(&globalCtx->actorCtx, globalCtx, ACTOR_EN_BOOM, posX, this->actor.world.pos.y + 30.0f,
                                 posZ, this->actor.focus.rot.x, yaw, 0, 0);

        this->boomerangActor = &boomerang->actor;
        if (boomerang != NULL) {
            boomerang->moveTo = this->targetActor;
            boomerang->returnTimer = 20;
            this->stateFlags1 |= PLAYER_STATE1_AWAITING_THROWN_BOOMERANG;
            if (!Player_IsUnfriendlyZTargeting(this)) {
                Player_SetZTargetFriendlyYaw(this);
            }
            this->fpsItemShotTimer = 4;
            func_8002F7DC(&this->actor, NA_SE_IT_BOOMERANG_THROW);
            Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_SWORD_N);
        }
    }

    return 1;
}

s32 Player_WaitForThrownBoomerang(Player* this, GlobalContext* globalCtx) {
    if (Player_StartZTargetDefend(globalCtx, this)) {
        return 1;
    }

    if (!(this->stateFlags1 & PLAYER_STATE1_AWAITING_THROWN_BOOMERANG)) {
        Player_SetUpperActionFunc(this, Player_CatchBoomerang);
        LinkAnimation_PlayOnce(globalCtx, &this->skelAnimeUpper, &gPlayerAnim_0025F8);
        Player_SetLeftHandDlists(this, gPlayerLeftHandBoomerangDLs);
        func_8002F7DC(&this->actor, NA_SE_PL_CATCH_BOOMERANG);
        Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_SWORD_N);
        return 1;
    }

    return 0;
}

s32 Player_CatchBoomerang(Player* this, GlobalContext* globalCtx) {
    if (!Player_HoldBoomerang(this, globalCtx) && LinkAnimation_Update(globalCtx, &this->skelAnimeUpper)) {
        Player_SetUpperActionFunc(this, Player_HoldBoomerang);
    }

    return 1;
}

s32 Player_SetActionFunc(GlobalContext* globalCtx, Player* this, PlayerActionFunc func, s32 flags) {
    if (func == this->actionFunc) {
        return 0;
    }

    if (Player_PlayOcarina == this->actionFunc) {
        Audio_OcaSetInstrument(0);
        this->stateFlags2 &= ~(PLAYER_STATE2_ATTEMPT_PLAY_OCARINA_FOR_ACTOR | PLAYER_STATE2_PLAYING_OCARINA_FOR_ACTOR);
    } else if (Player_UpdateMagicSpell == this->actionFunc) {
        Player_ResetSubCam(globalCtx, this);
    }

    this->actionFunc = func;

    if ((this->itemActionParam != this->heldItemActionParam) &&
        (!(flags & 1) || !(this->stateFlags1 & PLAYER_STATE1_SHIELDING))) {
        Player_SetHeldItem(this);
    }

    if (!(flags & 1) && (!(this->stateFlags1 & PLAYER_STATE1_HOLDING_ACTOR))) {
        Player_SetupHeldItemUpperActionFunc(globalCtx, this);
        this->stateFlags1 &= ~PLAYER_STATE1_SHIELDING;
    }

    Player_EndAnimMovement(this);
    this->stateFlags1 &= ~(PLAYER_STATE1_END_HOOKSHOT_MOVE | PLAYER_STATE1_TALKING | PLAYER_STATE1_TAKING_DAMAGE |
                           PLAYER_STATE1_SKIP_OTHER_ACTORS_UPDATE | PLAYER_STATE1_IN_CUTSCENE |
                           PLAYER_STATE1_FALLING_INTO_GROTTO_OR_VOID);
    this->stateFlags2 &=
        ~(PLAYER_STATE2_BACKFLIPPING_OR_SIDEHOPPING | PLAYER_STATE2_PLAYING_OCARINA_GENERAL | PLAYER_STATE2_IDLING);
    this->stateFlags3 &=
        ~(PLAYER_STATE3_MIDAIR | PLAYER_STATE3_ENDING_MELEE_ATTACK | PLAYER_STATE3_MOVING_ALONG_HOOKSHOT_PATH);
    this->genericVar = 0;
    this->genericTimer = 0;
    this->idleCounter = 0;
    Player_StopInterruptableSfx(this);

    return 1;
}

void Player_SetActionFuncPreserveMoveFlags(GlobalContext* globalCtx, Player* this, PlayerActionFunc func, s32 flags) {
    s32 flagsToRestore;

    flagsToRestore = this->skelAnime.moveFlags;
    this->skelAnime.moveFlags = 0;
    Player_SetActionFunc(globalCtx, this, func, flags);
    this->skelAnime.moveFlags = flagsToRestore;
}

void Player_SetActionFuncPreserveItemAP(GlobalContext* globalCtx, Player* this, PlayerActionFunc func, s32 flags) {
    s32 temp;

    if (this->itemActionParam >= 0) {
        temp = this->itemActionParam;
        this->itemActionParam = this->heldItemActionParam;
        Player_SetActionFunc(globalCtx, this, func, flags);
        this->itemActionParam = temp;
        Player_SetModels(this, Player_ActionToModelGroup(this, this->itemActionParam));
    }
}

void Player_ChangeCameraSetting(GlobalContext* globalCtx, s16 camSetting) {
    if (!func_800C0CB8(globalCtx)) {
        if (camSetting == CAM_SET_SCENE_TRANSITION) {
            Interface_ChangeAlpha(2);
        }
    } else {
        Camera_ChangeSetting(Gameplay_GetCamera(globalCtx, 0), camSetting);
    }
}

void Player_SetCameraTurnAround(GlobalContext* globalCtx, s32 arg1) {
    Player_ChangeCameraSetting(globalCtx, CAM_SET_TURN_AROUND);
    Camera_SetCameraData(Gameplay_GetCamera(globalCtx, 0), 4, 0, 0, arg1, 0, 0);
}

void Player_PutAwayHookshot(Player* this) {
    if (Player_HoldsHookshot(this)) {
        Actor* heldActor = this->heldActor;

        if (heldActor != NULL) {
            Actor_Kill(heldActor);
            this->actor.child = NULL;
            this->heldActor = NULL;
        }
    }
}

void Player_UseItem(GlobalContext* globalCtx, Player* this, s32 item) {
    s8 actionParam;
    s32 temp;
    s32 nextAnimType;

    actionParam = Player_ItemToActionParam(item);

    if (((this->heldItemActionParam == this->itemActionParam) &&
         (!(this->stateFlags1 & PLAYER_STATE1_SHIELDING) || (Player_ActionToSword(actionParam) != 0) ||
          (actionParam == PLAYER_AP_NONE))) ||
        ((this->itemActionParam < 0) &&
         ((Player_ActionToSword(actionParam) != 0) || (actionParam == PLAYER_AP_NONE)))) {

        if ((actionParam == PLAYER_AP_NONE) || !(this->stateFlags1 & PLAYER_STATE1_SWIMMING) ||
            ((this->actor.bgCheckFlags & 1) &&
             ((actionParam == PLAYER_AP_HOOKSHOT) || (actionParam == PLAYER_AP_LONGSHOT))) ||
            ((actionParam >= PLAYER_AP_SHIELD_DEKU) && (actionParam <= PLAYER_AP_BOOTS_HOVER))) {

            if ((globalCtx->bombchuBowlingStatus == 0) &&
                (((actionParam == PLAYER_AP_STICK) && (AMMO(ITEM_STICK) == 0)) ||
                 ((actionParam == PLAYER_AP_BEAN) && (AMMO(ITEM_BEAN) == 0)) ||
                 (temp = Player_ActionToExplosive(this, actionParam),
                  ((temp >= 0) && ((AMMO(sExplosiveInfos[temp].itemId) == 0) ||
                                   (globalCtx->actorCtx.actorLists[ACTORCAT_EXPLOSIVE].length >= 3)))))) {
                func_80078884(NA_SE_SY_ERROR);
                return;
            }

            if (actionParam >= PLAYER_AP_BOOTS_KOKIRI) {
                u16 bootsValue = actionParam - PLAYER_AP_BOOTS_KOKIRI + 1;
                if (CUR_EQUIP_VALUE(EQUIP_BOOTS) == bootsValue) {
                    Inventory_ChangeEquipment(EQUIP_BOOTS, PLAYER_BOOTS_KOKIRI + 1);
                } else {
                    Inventory_ChangeEquipment(EQUIP_BOOTS, bootsValue);
                }
                Player_SetEquipmentData(globalCtx, this);
                Player_PlayReactableSfx(this, CUR_EQUIP_VALUE(EQUIP_BOOTS) == PLAYER_BOOTS_IRON + 1
                                                  ? NA_SE_PL_WALK_HEAVYBOOTS
                                                  : NA_SE_PL_CHANGE_ARMS);
                return;
            }

            if (actionParam >= PLAYER_AP_TUNIC_KOKIRI) {
                u16 tunicValue = actionParam - PLAYER_AP_TUNIC_KOKIRI + 1;
                if (CUR_EQUIP_VALUE(EQUIP_TUNIC) == tunicValue) {
                    Inventory_ChangeEquipment(EQUIP_TUNIC, PLAYER_TUNIC_KOKIRI + 1);
                } else {
                    Inventory_ChangeEquipment(EQUIP_TUNIC, tunicValue);
                }
                Player_SetEquipmentData(globalCtx, this);
                Player_PlayReactableSfx(this, NA_SE_PL_CHANGE_ARMS);
                return;
            }

            if (actionParam >= PLAYER_AP_SHIELD_DEKU) {
                // Changing shields through action commands is unimplemented
                return;
            }

            if (actionParam == PLAYER_AP_LENS) {
                if (func_80087708(globalCtx, 0, 3)) {
                    if (globalCtx->actorCtx.unk_03 != 0) {
                        func_800304B0(globalCtx);
                    } else {
                        globalCtx->actorCtx.unk_03 = 1;
                    }
                    func_80078884((globalCtx->actorCtx.unk_03 != 0) ? NA_SE_SY_GLASSMODE_ON : NA_SE_SY_GLASSMODE_OFF);
                } else {
                    func_80078884(NA_SE_SY_ERROR);
                }
                return;
            }

            if (actionParam == PLAYER_AP_NUT) {
                if (AMMO(ITEM_NUT) != 0) {
                    Player_SetupThrowDekuNut(globalCtx, this);
                } else {
                    func_80078884(NA_SE_SY_ERROR);
                }
                return;
            }

            temp = Player_ActionToMagicSpell(this, actionParam);
            if (temp >= 0) {
                if (((actionParam == PLAYER_AP_FARORES_WIND) && (gSaveContext.respawn[RESPAWN_MODE_TOP].data > 0)) ||
                    ((gSaveContext.unk_13F4 != 0) && (gSaveContext.unk_13F0 == 0) &&
                     (gSaveContext.magic >= sMagicSpellCosts[temp]))) {
                    this->itemActionParam = actionParam;
                    this->attentionMode = 4;
                } else {
                    func_80078884(NA_SE_SY_ERROR);
                }
                return;
            }

            if (actionParam >= PLAYER_AP_MASK_KEATON) {
                if (this->currentMask != PLAYER_MASK_NONE) {
                    this->currentMask = PLAYER_MASK_NONE;
                } else {
                    this->currentMask = actionParam - PLAYER_AP_MASK_KEATON + 1;
                }
                sMaskMemory = this->currentMask;
                Player_PlayReactableSfx(this, NA_SE_PL_CHANGE_ARMS);
                return;
            }

            if (((actionParam >= PLAYER_AP_OCARINA_FAIRY) && (actionParam <= PLAYER_AP_OCARINA_TIME)) ||
                (actionParam >= PLAYER_AP_BOTTLE_FISH)) {
                if (!Player_IsUnfriendlyZTargeting(this) ||
                    ((actionParam >= PLAYER_AP_BOTTLE_POTION_RED) && (actionParam <= PLAYER_AP_BOTTLE_FAIRY))) {
                    func_8002D53C(globalCtx, &globalCtx->actorCtx.titleCtx);
                    this->attentionMode = 4;
                    this->itemActionParam = actionParam;
                }
                return;
            }

            if ((actionParam != this->heldItemActionParam) ||
                ((this->heldActor == 0) && (Player_ActionToExplosive(this, actionParam) >= 0))) {
                this->nextModelGroup = Player_ActionToModelGroup(this, actionParam);
                nextAnimType = gPlayerModelTypes[this->nextModelGroup][PLAYER_MODELGROUPENTRY_ANIM];
                if ((this->heldItemActionParam >= 0) && (Player_ActionToMagicSpell(this, actionParam) < 0) &&
                    (item != this->heldItemId) &&
                    (sAnimtypeToItemChangeAnims[gPlayerModelTypes[this->modelGroup][PLAYER_MODELGROUPENTRY_ANIM]]
                                               [nextAnimType] != PLAYER_ITEM_CHANGE_DEFAULT)) {
                    this->heldItemId = item;
                    this->stateFlags1 |= PLAYER_STATE1_START_CHANGE_ITEM;
                } else {
                    Player_PutAwayHookshot(this);
                    Player_DetatchHeldActor(globalCtx, this);
                    Player_SetupChangeItemAnim(globalCtx, this, actionParam);
                }
                return;
            }

            sUsingItemAlreadyInHand = sUsingItemAlreadyInHand2 = true;
        }
    }
}

void Player_SetupDie(GlobalContext* globalCtx, Player* this, LinkAnimationHeader* anim) {
    s32 isSwimming = Player_IsSwimming(this);

    Player_ResetAttributesAndHeldActor(globalCtx, this);

    Player_SetActionFunc(globalCtx, this, isSwimming ? Player_Drown : Player_Die, 0);

    this->stateFlags1 |= PLAYER_STATE1_IN_DEATH_CUTSCENE;

    Player_PlayAnimOnce(globalCtx, this, anim);
    if (anim == &gPlayerAnim_002878) {
        this->skelAnime.endFrame = 84.0f;
    }

    Player_ClearAttentionModeAndStopMoving(this);
    Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_DOWN);

    if (this->actor.category == ACTORCAT_PLAYER) {
        func_800F47BC();

        if (Inventory_ConsumeFairy(globalCtx)) {
            globalCtx->gameOverCtx.state = GAMEOVER_REVIVE_START;
            this->genericVar = 1;
        } else {
            globalCtx->gameOverCtx.state = GAMEOVER_DEATH_START;
            func_800F6AB0(0);
            Audio_PlayFanfare(NA_BGM_GAME_OVER);
            gSaveContext.seqId = (u8)NA_BGM_DISABLED;
            gSaveContext.natureAmbienceId = NATURE_ID_DISABLED;
        }

        OnePointCutscene_Init(globalCtx, 9806, isSwimming ? 120 : 60, &this->actor, MAIN_CAM);
        ShrinkWindow_SetVal(0x20);
    }
}

s32 Player_CanUseItem(Player* this) {
    return (!(Player_RunMiniCutsceneFunc == this->actionFunc) ||
            ((this->stateFlags1 & PLAYER_STATE1_START_CHANGE_ITEM) &&
             ((this->heldItemId == ITEM_LAST_USED) || (this->heldItemId == ITEM_NONE)))) &&
           (!(Player_StartChangeItem == this->upperActionFunc) ||
            (Player_ItemToActionParam(this->heldItemId) == this->heldItemActionParam));
}

s32 Player_SetupCurrentUpperAction(Player* this, GlobalContext* globalCtx) {
    if (!(this->stateFlags1 & PLAYER_STATE1_RIDING_HORSE) && (this->actor.parent != NULL) &&
        Player_HoldsHookshot(this)) {
        Player_SetActionFunc(globalCtx, this, Player_MoveAlongHookshotPath, 1);
        this->stateFlags3 |= PLAYER_STATE3_MOVING_ALONG_HOOKSHOT_PATH;
        Player_PlayAnimOnce(globalCtx, this, &gPlayerAnim_002C90);
        Player_SetupAnimMovement(globalCtx, this, 0x9B);
        Player_ClearAttentionModeAndStopMoving(this);
        this->currentYaw = this->actor.shape.rot.y;
        this->actor.bgCheckFlags &= ~1;
        this->hoverBootsTimer = 0;
        this->lookFlags |= 0x43;
        Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_LASH);
        return 1;
    }

    if (Player_CanUseItem(this)) {
        Player_SetupItem(this, globalCtx);
        if (Player_ThrowDekuNut == this->actionFunc) {
            return 1;
        }
    }

    if (!this->upperActionFunc(this, globalCtx)) {
        return 0;
    }

    if (this->upperInterpWeight != 0.0f) {
        if ((Player_IsPlayingIdleAnim(this) == 0) || (this->linearVelocity != 0.0f)) {
            AnimationContext_SetCopyFalse(globalCtx, this->skelAnime.limbCount, this->skelAnimeUpper.jointTable,
                                          this->skelAnime.jointTable, D_80853410);
        }
        Math_StepToF(&this->upperInterpWeight, 0.0f, 0.25f);
        AnimationContext_SetInterp(globalCtx, this->skelAnime.limbCount, this->skelAnime.jointTable,
                                   this->skelAnimeUpper.jointTable, 1.0f - this->upperInterpWeight);
    } else if ((Player_IsPlayingIdleAnim(this) == 0) || (this->linearVelocity != 0.0f)) {
        AnimationContext_SetCopyTrue(globalCtx, this->skelAnime.limbCount, this->skelAnime.jointTable,
                                     this->skelAnimeUpper.jointTable, D_80853410);
    } else {
        AnimationContext_SetCopyAll(globalCtx, this->skelAnime.limbCount, this->skelAnime.jointTable,
                                    this->skelAnimeUpper.jointTable);
    }

    return 1;
}

s32 Player_SetupMiniCsFunc(GlobalContext* globalCtx, Player* this, PlayerMiniCsFunc func) {
    this->miniCsFunc = func;
    Player_SetActionFunc(globalCtx, this, Player_RunMiniCutsceneFunc, 0);
    this->stateFlags2 |= PLAYER_STATE2_ALWAYS_DISABLE_MOVE_ROTATION;
    return Player_UnequipItem(globalCtx, this);
}

void Player_UpdateYaw(Player* this, GlobalContext* globalCtx) {
    s16 previousYaw = this->actor.shape.rot.y;

    if (!(this->stateFlags2 &
          (PLAYER_STATE2_DISABLE_MOVE_ROTATION_WHILE_Z_TARGETING | PLAYER_STATE2_ALWAYS_DISABLE_MOVE_ROTATION))) {
        if ((this->targetActor != NULL) &&
            ((globalCtx->actorCtx.targetCtx.unk_4B != 0) || (this->actor.category != ACTORCAT_PLAYER))) {
            Math_ScaledStepToS(&this->actor.shape.rot.y,
                               Math_Vec3f_Yaw(&this->actor.world.pos, &this->targetActor->focus.pos), 4000);
        } else if ((this->stateFlags1 & PLAYER_STATE1_Z_TARGETING_FRIENDLY) &&
                   !(this->stateFlags2 & (PLAYER_STATE2_DISABLE_MOVE_ROTATION_WHILE_Z_TARGETING |
                                          PLAYER_STATE2_ALWAYS_DISABLE_MOVE_ROTATION))) {
            Math_ScaledStepToS(&this->actor.shape.rot.y, this->targetYaw, 4000);
        }
    } else if (!(this->stateFlags2 & PLAYER_STATE2_ALWAYS_DISABLE_MOVE_ROTATION)) {
        Math_ScaledStepToS(&this->actor.shape.rot.y, this->currentYaw, 2000);
    }

    this->unk_87C = this->actor.shape.rot.y - previousYaw;
}

s32 Player_StepAngleWithOffset(s16* pValue, s16 target, s16 step, s16 angleMinMax, s16 referenceAngle,
                               s16 angleDiffMinMax) {
    s16 angleDiff;
    s16 clampedAngleDiff;
    s16 originalAngle;

    angleDiff = clampedAngleDiff = referenceAngle - *pValue;
    clampedAngleDiff = CLAMP(clampedAngleDiff, -angleDiffMinMax, angleDiffMinMax);
    *pValue += (s16)(angleDiff - clampedAngleDiff);

    Math_ScaledStepToS(pValue, target, step);

    originalAngle = *pValue;
    if (*pValue < -angleMinMax) {
        *pValue = -angleMinMax;
    } else if (*pValue > angleMinMax) {
        *pValue = angleMinMax;
    }
    return originalAngle - *pValue;
}

s32 Player_UpdateLookAngles(Player* this, s32 syncUpperRotToFocusRot) {
    s16 yawDiff;
    s16 lookYaw;

    lookYaw = this->actor.shape.rot.y;
    if (syncUpperRotToFocusRot != 0) {
        lookYaw = this->actor.focus.rot.y;
        this->upperBodyRot.x = this->actor.focus.rot.x;
        this->lookFlags |= 0x41;
    } else {
        Player_StepAngleWithOffset(&this->upperBodyRot.x,
                                   Player_StepAngleWithOffset(&this->headRot.x, this->actor.focus.rot.x, 600, 10000,
                                                              this->actor.focus.rot.x, 0),
                                   200, 4000, this->headRot.x, 10000);
        yawDiff = this->actor.focus.rot.y - lookYaw;
        Player_StepAngleWithOffset(&yawDiff, 0, 200, 24000, this->upperBodyRot.y, 8000);
        lookYaw = this->actor.focus.rot.y - yawDiff;
        Player_StepAngleWithOffset(&this->headRot.y, yawDiff - this->upperBodyRot.y, 200, 8000, yawDiff, 8000);
        Player_StepAngleWithOffset(&this->upperBodyRot.y, yawDiff, 200, 8000, this->headRot.y, 8000);
        this->lookFlags |= PLAYER_LOOKFLAGS_OVERRIDE_UPPERBODY_ROT_Y | PLAYER_LOOKFLAGS_OVERRIDE_UPPERBODY_ROT_X |
                           PLAYER_LOOKFLAGS_OVERRIDE_HEAD_ROT_Y | PLAYER_LOOKFLAGS_OVERRIDE_HEAD_ROT_X |
                           PLAYER_LOOKFLAGS_OVERRIDE_FOCUS_ROT_X;
    }

    return lookYaw;
}

void Player_SetupZTargeting(Player* this, GlobalContext* globalCtx) {
    s32 isRangeCheckDisabled = 0;
    s32 zTrigPressed = CHECK_BTN_ALL(sControlInput->cur.button, BTN_Z);
    Actor* actorToTarget;
    s32 pad;
    s32 holdTarget;
    s32 actorRequestingTalk;

    if (!zTrigPressed) {
        this->stateFlags1 &= ~PLAYER_STATE1_30;
    }

    if ((globalCtx->csCtx.state != CS_STATE_IDLE) || (this->csMode != 0) ||
        (this->stateFlags1 & (PLAYER_STATE1_IN_DEATH_CUTSCENE | PLAYER_STATE1_IN_CUTSCENE)) ||
        (this->stateFlags3 & PLAYER_STATE3_MOVING_ALONG_HOOKSHOT_PATH)) {
        this->targetSwitchTimer = 0;
    } else if ((zTrigPressed && !CVar_GetS32("gDisableTargeting", 0)) ||
               (this->stateFlags2 & PLAYER_STATE2_USING_SWITCH_Z_TARGETING) || (this->forcedTargetActor != NULL)) {
        if (this->targetSwitchTimer <= 5) {
            this->targetSwitchTimer = 5;
        } else {
            this->targetSwitchTimer--;
        }
    } else if (this->stateFlags1 & PLAYER_STATE1_Z_TARGETING_FRIENDLY) {
        this->targetSwitchTimer = 0;
    } else if (this->targetSwitchTimer != 0) {
        this->targetSwitchTimer--;
    }

    if (this->targetSwitchTimer >= 6) {
        isRangeCheckDisabled = 1;
    }

    actorRequestingTalk = Player_CheckActorTalkRequested(globalCtx);
    if (actorRequestingTalk || (this->targetSwitchTimer != 0) ||
        (this->stateFlags1 & (PLAYER_STATE1_CHARGING_SPIN_ATTACK | PLAYER_STATE1_AWAITING_THROWN_BOOMERANG))) {
        if (!actorRequestingTalk) {
            if (!(this->stateFlags1 & PLAYER_STATE1_AWAITING_THROWN_BOOMERANG) &&
                ((this->heldItemActionParam != PLAYER_AP_FISHING_POLE) || (this->fpsItemType == 0)) &&
                CHECK_BTN_ALL(sControlInput->press.button, BTN_Z)) {

                if (this->actor.category == ACTORCAT_PLAYER) {
                    actorToTarget = globalCtx->actorCtx.targetCtx.arrowPointedActor;
                } else {
                    actorToTarget = &GET_PLAYER(globalCtx)->actor;
                }

                holdTarget = (gSaveContext.zTargetSetting != 0) || (this->actor.category != ACTORCAT_PLAYER);
                this->stateFlags1 |= PLAYER_STATE1_UNUSED_Z_TARGETING_FLAG;

                if ((actorToTarget != NULL) && !(actorToTarget->flags & ACTOR_FLAG_27)) {
                    if ((actorToTarget == this->targetActor) && (this->actor.category == ACTORCAT_PLAYER)) {
                        actorToTarget = globalCtx->actorCtx.targetCtx.unk_94;
                    }

                    if (actorToTarget != this->targetActor) {
                        if (!holdTarget) {
                            this->stateFlags2 |= PLAYER_STATE2_USING_SWITCH_Z_TARGETING;
                        }
                        this->targetActor = actorToTarget;
                        this->targetSwitchTimer = 15;
                        this->stateFlags2 &= ~(PLAYER_STATE2_CAN_SPEAK_OR_CHECK | PLAYER_STATE2_NAVI_REQUESTING_TALK);
                    } else {
                        if (!holdTarget) {
                            func_8008EDF0(this);
                        }
                    }

                    this->stateFlags1 &= ~PLAYER_STATE1_30;
                } else {
                    if (!(this->stateFlags1 & (PLAYER_STATE1_Z_TARGETING_FRIENDLY | PLAYER_STATE1_30))) {
                        Player_SetZTargetFriendlyYaw(this);
                    }
                }
            }

            if (this->targetActor != NULL) {
                if ((this->actor.category == ACTORCAT_PLAYER) && (this->targetActor != this->forcedTargetActor) &&
                    func_8002F0C8(this->targetActor, this, isRangeCheckDisabled)) {
                    func_8008EDF0(this);
                    this->stateFlags1 |= PLAYER_STATE1_30;
                } else if (this->targetActor != NULL) {
                    this->targetActor->targetPriority = 40;
                }
            } else if (this->forcedTargetActor != NULL) {
                this->targetActor = this->forcedTargetActor;
            }
        }

        if (this->targetActor != NULL) {
            this->stateFlags1 &= ~(PLAYER_STATE1_FORCE_STRAFING | PLAYER_STATE1_Z_TARGETING_FRIENDLY);
            if ((this->stateFlags1 & PLAYER_STATE1_HOLDING_ACTOR) ||
                !CHECK_FLAG_ALL(this->targetActor->flags, ACTOR_FLAG_0 | ACTOR_FLAG_2)) {
                this->stateFlags1 |= PLAYER_STATE1_FORCE_STRAFING;
            }
        } else {
            if (this->stateFlags1 & PLAYER_STATE1_Z_TARGETING_FRIENDLY) {
                this->stateFlags2 &= ~PLAYER_STATE2_USING_SWITCH_Z_TARGETING;
            } else {
                func_8008EE08(this);
            }
        }
    } else {
        func_8008EE08(this);
    }
}

s32 Player_CalculateTargetVelocityAndYaw(GlobalContext* globalCtx, Player* this, f32* targetVelocity, s16* targetYaw,
                                         f32 arg4) {
    f32 baseSpeedScale;
    f32 slope;
    f32 slopeSpeedScale;
    f32 speedLimit;

    if ((this->attentionMode != 0) || (globalCtx->sceneLoadFlag == 0x14) ||
        (this->stateFlags1 & PLAYER_STATE1_EXITING_SCENE)) {
        *targetVelocity = 0.0f;
        *targetYaw = this->actor.shape.rot.y;
    } else {
        *targetVelocity = sAnalogStickDistance;
        *targetYaw = sAnalogStickAngle;

        if (arg4 != 0.0f) {
            *targetVelocity -= 20.0f;
            if (*targetVelocity < 0.0f) {
                *targetVelocity = 0.0f;
            } else {
                baseSpeedScale = 1.0f - Math_CosS(*targetVelocity * 450.0f);
                *targetVelocity = ((baseSpeedScale * baseSpeedScale) * 30.0f) + 7.0f;
            }
        } else {
            *targetVelocity *= 0.8f;
        }

        if (sAnalogStickDistance != 0.0f) {
            slope = Math_SinS(this->angleToFloorX);
            speedLimit = this->speedLimit;
            slopeSpeedScale = CLAMP(slope, 0.0f, 0.6f);

            if (this->shapeOffsetY != 0.0f) {
                speedLimit = speedLimit - (this->shapeOffsetY * 0.008f);
                if (speedLimit < 2.0f) {
                    speedLimit = 2.0f;
                }
            }

            *targetVelocity = (*targetVelocity * 0.14f) - (8.0f * slopeSpeedScale * slopeSpeedScale);
            *targetVelocity = CLAMP(*targetVelocity, 0.0f, speedLimit);

            return 1;
        }
    }

    return 0;
}

s32 Player_StepLinearVelocityToZero(Player* this) {
    return Math_StepToF(&this->linearVelocity, 0.0f, REG(43) / 100.0f);
}

s32 Player_GetTargetVelocityAndYaw(Player* this, f32* arg1, s16* arg2, f32 arg3, GlobalContext* globalCtx) {
    if (!Player_CalculateTargetVelocityAndYaw(globalCtx, this, arg1, arg2, arg3)) {
        *arg2 = this->actor.shape.rot.y;

        if (this->targetActor != NULL) {
            if ((globalCtx->actorCtx.targetCtx.unk_4B != 0) &&
                !(this->stateFlags2 & PLAYER_STATE2_ALWAYS_DISABLE_MOVE_ROTATION)) {
                *arg2 = Math_Vec3f_Yaw(&this->actor.world.pos, &this->targetActor->focus.pos);
                return 0;
            }
        } else if (Player_IsFriendlyZTargeting(this)) {
            *arg2 = this->targetYaw;
        }

        return 0;
    } else {
        *arg2 += Camera_GetInputDirYaw(GET_ACTIVE_CAM(globalCtx));
        return 1;
    }
}

static s8 sTargetEnemyStandStillSubActions[] = { 13, 2, 4, 9, 10, 11, 8, -7 };
static s8 sFriendlyTargetingStandStillSubActions[] = { 13, 1, 2, 5, 3, 4, 9, 10, 11, 7, 8, -6 };
static s8 sEndSidewalkSubActions[] = { 13, 1, 2, 3, 4, 9, 10, 11, 8, 7, -6 };
static s8 sFriendlyBackwalkSubActions[] = { 13, 2, 4, 9, 10, 11, 8, -7 };
static s8 sSidewalkSubActions[] = { 13, 2, 4, 9, 10, 11, 12, 8, -7 };
static s8 sTurnSubActions[] = { -7 };
static s8 sStandStillSubActions[] = { 0, 11, 1, 2, 3, 5, 4, 9, 8, 7, -6 };
static s8 sRunSubActions[] = { 0, 11, 1, 2, 3, 12, 5, 4, 9, 8, 7, -6 };
static s8 sTargetRunSubActions[] = { 13, 1, 2, 3, 12, 5, 4, 9, 10, 11, 8, 7, -6 };
static s8 sEndBackwalkSubActions[] = { 10, 8, -7 };
static s8 sSwimSubActions[] = { 0, 12, 5, -4 };

static s32 (*sSubActions[])(Player* this, GlobalContext* globalCtx) = {
    Player_SetupCUpBehavior,               // 0
    Player_SetupOpenDoor,                  // 1
    Player_SetupGetItemOrHoldBehavior,     // 2
    Player_SetupMountHorse,                // 3
    Player_SetupSpeakOrCheck,              // 4
    Player_SetupSpecialWallInteraction,    // 5
    Player_SetupRollOrPutAway,             // 6
    Player_SetupStartMeleeWeaponAttack,    // 7
    Player_SetupStartChargeSpinAttack,     // 8
    Player_SetupPutDownOrThrowActor,       // 9
    Player_SetupJumpSlashOrRoll,           // 10
    Player_SetupDefend,                    // 11
    Player_SetupWallJumpBehavior,          // 12
    Player_SetupItemCutsceneOrFirstPerson, // 13
};

s32 Player_SetupSubAction(GlobalContext* globalCtx, Player* this, s8* subActionIndex, s32 arg3) {
    s32 i;

    if (!(this->stateFlags1 &
          (PLAYER_STATE1_EXITING_SCENE | PLAYER_STATE1_IN_DEATH_CUTSCENE | PLAYER_STATE1_IN_CUTSCENE))) {
        if (arg3 != 0) {
            D_808535E0 = Player_SetupCurrentUpperAction(this, globalCtx);
            if (Player_ThrowDekuNut == this->actionFunc) {
                return 1;
            }
        }

        if (Player_IsShootingHookshot(this)) {
            this->lookFlags |= PLAYER_LOOKFLAGS_OVERRIDE_UPPERBODY_ROT_X | PLAYER_LOOKFLAGS_OVERRIDE_FOCUS_ROT_X;
            return 1;
        }

        if (!(this->stateFlags1 & PLAYER_STATE1_START_CHANGE_ITEM) &&
            (Player_StartChangeItem != this->upperActionFunc)) {
            while (*subActionIndex >= 0) {
                if (sSubActions[*subActionIndex](this, globalCtx)) {
                    return 1;
                }
                subActionIndex++;
            }

            if (sSubActions[-(*subActionIndex)](this, globalCtx)) {
                return 1;
            }
        }
    }

    return 0;
}

// Checks if action is interrupted within a certain number of frames from the end of the current animation
// Returns -1 is action is not interrupted at all, 0 if interrupted by a sub-action, 1 if interrupted by the player
// moving
s32 Player_IsActionInterrupted(GlobalContext* globalCtx, Player* this, SkelAnime* skelAnime, f32 framesFromEnd) {
    f32 targetVelocity;
    s16 targetYaw;

    if ((skelAnime->endFrame - framesFromEnd) <= skelAnime->curFrame) {
        if (Player_SetupSubAction(globalCtx, this, sStandStillSubActions, 1)) {
            return 0;
        }

        if (Player_GetTargetVelocityAndYaw(this, &targetVelocity, &targetYaw, 0.018f, globalCtx)) {
            return 1;
        }
    }

    return -1;
}

void Player_SetupSpinAttackActor(GlobalContext* globalCtx, Player* this, s32 spinAttackParams) {
    if (spinAttackParams != 0) {
        this->unk_858 = 0.0f;
    } else {
        this->unk_858 = 0.5f;
    }

    this->stateFlags1 |= PLAYER_STATE1_CHARGING_SPIN_ATTACK;

    if (this->actor.category == ACTORCAT_PLAYER) {
        Actor_Spawn(&globalCtx->actorCtx, globalCtx, ACTOR_EN_M_THUNDER, this->bodyPartsPos[PLAYER_BODYPART_WAIST].x,
                    this->bodyPartsPos[PLAYER_BODYPART_WAIST].y, this->bodyPartsPos[PLAYER_BODYPART_WAIST].z, 0, 0, 0,
                    Player_GetSwordHeld(this) | spinAttackParams);
    }
}

s32 Player_CanQuickspin(Player* this) {
    s8 stickInputsArray[4];
    s8* analogStickInput;
    s8* stickInput;
    s8 inputDiff1;
    s8 inputDiff2;
    s32 i;

    if ((this->heldItemActionParam == PLAYER_AP_STICK) || Player_HoldsBrokenKnife(this)) {
        return 0;
    }

    analogStickInput = &this->analogStickInputs[0];
    stickInput = &stickInputsArray[0];
    // Check all four stored frames of input to see if stick distance traveled is ever less than 55.0f from the center
    for (i = 0; i < 4; i++, analogStickInput++, stickInput++) {
        if ((*stickInput = *analogStickInput) < 0) {
            return 0;
        }
        // Multiply each stored stickInput by 2
        *stickInput *= 2;
    }

    // Get diff between first two frames of stick input
    inputDiff1 = stickInputsArray[0] - stickInputsArray[1];
    // Return false if the difference is too small (< ~28 degrees) for the player to be spinning the analog stick
    if (ABS(inputDiff1) < 10) {
        return 0;
    }

    stickInput = &stickInputsArray[1];
    // *stickInput will be the second, then third frame of stick input in this loop
    for (i = 1; i < 3; i++, stickInput++) {
        // Get diff between current input frame and next input frame
        inputDiff2 = *stickInput - *(stickInput + 1);
        // If the difference too small, or stick has changed directions, return false
        if ((ABS(inputDiff2) < 10) || (inputDiff2 * inputDiff1 < 0)) {
            return 0;
        }
    }

    return 1;
}

void Player_SetupSpinAttackAnims(GlobalContext* globalCtx, Player* this) {
    LinkAnimationHeader* anim;

    if ((this->swordAnimation >= 4) && (this->swordAnimation < 8)) {
        anim = sSpinAttackAnims1[Player_HoldsTwoHandedWeapon(this)];
    } else {
        anim = sSpinAttackAnims2[Player_HoldsTwoHandedWeapon(this)];
    }

    Player_InactivateMeleeWeapon(this);
    LinkAnimation_Change(globalCtx, &this->skelAnime, anim, 1.0f, 8.0f, Animation_GetLastFrame(anim), ANIMMODE_ONCE,
                         -9.0f);
    Player_SetupSpinAttackActor(globalCtx, this, 0x200);
}

void Player_StartChargeSpinAttack(GlobalContext* globalCtx, Player* this) {
    Player_SetActionFunc(globalCtx, this, Player_ChargeSpinAttack, 1);
    Player_SetupSpinAttackAnims(globalCtx, this);
}

static s8 sMeleeWeaponAttackDirections[] = { 12, 4, 4, 8 };
static s8 sHammerAttackDirections[] = { 22, 23, 22, 23 };

s32 Player_GetMeleeAttackAnim(Player* this) {
    s32 relativeStickInput = this->relativeAnalogStickInputs[this->inputFrameCounter];
    s32 attackAnim;

    if (this->heldItemActionParam == PLAYER_AP_HAMMER) {
        if (relativeStickInput < 0) {
            relativeStickInput = 0;
        }
        attackAnim = sHammerAttackDirections[relativeStickInput];
        this->slashCounter = 0;
    } else {
        if (Player_CanQuickspin(this)) {
            attackAnim = 24;
        } else {
            if (relativeStickInput < 0) {
                if (Player_IsZTargeting(this)) {
                    attackAnim = 0;
                } else {
                    attackAnim = 4;
                }
            } else {
                attackAnim = sMeleeWeaponAttackDirections[relativeStickInput];
                if (attackAnim == 12) {
                    this->stateFlags2 |= PLAYER_STATE2_ENABLE_FORWARD_SLIDE_FROM_ATTACK;
                    if (!Player_IsZTargeting(this)) {
                        attackAnim = 0;
                    }
                }
            }
            if (this->heldItemActionParam == PLAYER_AP_STICK) {
                attackAnim = 0;
            }
        }
        if (Player_HoldsTwoHandedWeapon(this)) {
            attackAnim++;
        }
    }

    return attackAnim;
}

void Player_SetupMeleeWeaponToucherFlags(Player* this, s32 quadIndex, u32 flags) {
    this->swordQuads[quadIndex].info.toucher.dmgFlags = flags;

    if (flags == 2) {
        this->swordQuads[quadIndex].info.toucherFlags = TOUCH_ON | TOUCH_NEAREST | TOUCH_SFX_WOOD;
    } else {
        this->swordQuads[quadIndex].info.toucherFlags = TOUCH_ON | TOUCH_NEAREST;
    }
}

static u32 sMeleeWeaponDmgFlags[][2] = {
    { 0x00000200, 0x08000000 }, { 0x00000100, 0x02000000 }, { 0x00000400, 0x04000000 },
    { 0x00000002, 0x08000000 }, { 0x00000040, 0x40000000 },
};

void Player_StartMeleeWeaponAttack(GlobalContext* globalCtx, Player* this, s32 arg2) {
    s32 pad;
    u32 flags;
    s32 temp;

    Player_SetActionFunc(globalCtx, this, Player_MeleeWeaponAttack, 0);
    this->comboTimer = 8;
    if ((arg2 < 18) || (arg2 >= 20)) {
        Player_InactivateMeleeWeapon(this);
    }

    if ((arg2 != this->swordAnimation) || !(this->slashCounter < 3)) {
        this->slashCounter = 0;
    }

    this->slashCounter++;
    if (this->slashCounter >= 3) {
        arg2 += 2;
    }

    this->swordAnimation = arg2;

    Player_PlayAnimOnceSlowed(globalCtx, this, sMeleeAttackAnims[arg2].unk_00);
    if ((arg2 != 16) && (arg2 != 17)) {
        Player_SetupAnimMovement(globalCtx, this, 0x209);
    }

    this->currentYaw = this->actor.shape.rot.y;

    if (Player_HoldsBrokenKnife(this)) {
        temp = 1;
    } else {
        temp = Player_GetSwordHeld(this) - 1;
    }

    if ((arg2 >= 16) && (arg2 < 20)) {
        flags = sMeleeWeaponDmgFlags[temp][1];
    } else {
        flags = sMeleeWeaponDmgFlags[temp][0];
    }

    Player_SetupMeleeWeaponToucherFlags(this, 0, flags);
    Player_SetupMeleeWeaponToucherFlags(this, 1, flags);
}

void Player_SetupInvincibility(Player* this, s32 timer) {
    if (this->invincibilityTimer >= 0) {
        this->invincibilityTimer = timer;
        this->damageFlashTimer = 0;
    }
}

void Player_SetupInvincibilityNoDamageFlash(Player* this, s32 timer) {
    if (this->invincibilityTimer > timer) {
        this->invincibilityTimer = timer;
    }
    this->damageFlashTimer = 0;
}

s32 Player_Damage_modified(GlobalContext* globalCtx, Player* this, s32 damage, u8 modified) {
    if ((this->invincibilityTimer != 0) || (this->actor.category != ACTORCAT_PLAYER)) {
        return 1;
    }

    s32 modifiedDamage = damage;
    if (modified) {
        modifiedDamage *= CVar_GetS32("gDamageMul", 1);
    }

    return Health_ChangeBy(globalCtx, modifiedDamage);
}

s32 Player_Damage(GlobalContext* globalCtx, Player* this, s32 damage) {
    return Player_Damage_modified(globalCtx, this, damage, true);
}

void Player_SetLedgeGrabPosition(Player* this) {
    this->skelAnime.prevTransl = this->skelAnime.jointTable[0];
    Player_UpdateAnimMovement(this, 3);
}

void Player_SetupFallFromLedge(Player* this, GlobalContext* globalCtx) {
    Player_SetActionFunc(globalCtx, this, Player_UpdateMidair, 0);
    Player_PlayAnimLoop(globalCtx, this, &gPlayerAnim_003040);
    this->genericTimer = 1;
    if (this->attentionMode != 3) {
        this->attentionMode = 0;
    }
}

static LinkAnimationHeader* sLinkDamageAnims[] = {
    &gPlayerAnim_002F80, &gPlayerAnim_002F78, &gPlayerAnim_002DE0, &gPlayerAnim_002DD8,
    &gPlayerAnim_002F70, &gPlayerAnim_002528, &gPlayerAnim_002DC8, &gPlayerAnim_0024F0,
};

void Player_SetupDamage(GlobalContext* globalCtx, Player* this, s32 damageReaction, f32 knockbackVelXZ,
                        f32 knockbackVelY, s16 damageYaw, s32 invincibilityTimer) {
    LinkAnimationHeader* anim = NULL;
    LinkAnimationHeader** damageAnims;

    if (this->stateFlags1 & PLAYER_STATE1_HANGING_FROM_LEDGE_SLIP) {
        Player_SetLedgeGrabPosition(this);
    }

    this->runDamageTimer = 0;

    func_8002F7DC(&this->actor, NA_SE_PL_DAMAGE);

    if (!Player_Damage(globalCtx, this, 0 - this->actor.colChkInfo.damage)) {
        this->stateFlags2 &= ~PLAYER_STATE2_RESTRAINED_BY_ENEMY;
        if (!(this->actor.bgCheckFlags & 1) && !(this->stateFlags1 & PLAYER_STATE1_SWIMMING)) {
            Player_SetupFallFromLedge(this, globalCtx);
        }
        return;
    }

    Player_SetupInvincibility(this, invincibilityTimer);

    if (damageReaction == PLAYER_DMGREACTION_FROZEN) {
        Player_SetActionFunc(globalCtx, this, Player_FrozenInIce, 0);

        anim = &gPlayerAnim_002FD0;

        Player_ClearAttentionModeAndStopMoving(this);
        Player_RequestRumble(this, 255, 10, 40, 0);

        func_8002F7DC(&this->actor, NA_SE_PL_FREEZE_S);
        Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_FREEZE);
    } else if (damageReaction == PLAYER_DMGREACTION_ELECTRIC_SHOCKED) {
        Player_SetActionFunc(globalCtx, this, Player_SetupElectricShock, 0);

        Player_RequestRumble(this, 255, 80, 150, 0);

        Player_PlayAnimLoopSlowed(globalCtx, this, &gPlayerAnim_002F00);
        Player_ClearAttentionModeAndStopMoving(this);

        this->genericTimer = 20;
    } else {
        damageYaw -= this->actor.shape.rot.y;
        if (this->stateFlags1 & PLAYER_STATE1_SWIMMING) {
            Player_SetActionFunc(globalCtx, this, Player_DamagedSwim, 0);
            Player_RequestRumble(this, 180, 20, 50, 0);

            this->linearVelocity = 4.0f;
            this->actor.velocity.y = 0.0f;

            anim = &gPlayerAnim_003320;

            Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_DAMAGE_S);
        } else if ((damageReaction == PLAYER_DMGREACTION_KNOCKBACK) || (damageReaction == PLAYER_DMGREACTION_HOP) ||
                   !(this->actor.bgCheckFlags & 1) ||
                   (this->stateFlags1 & (PLAYER_STATE1_HANGING_FROM_LEDGE_SLIP | PLAYER_STATE1_CLIMBING_ONTO_LEDGE |
                                         PLAYER_STATE1_CLIMBING))) {
            Player_SetActionFunc(globalCtx, this, Player_StartKnockback, 0);

            this->stateFlags3 |= PLAYER_STATE3_MIDAIR;

            Player_RequestRumble(this, 255, 20, 150, 0);
            Player_ClearAttentionModeAndStopMoving(this);

            if (damageReaction == PLAYER_DMGREACTION_HOP) {
                this->genericTimer = 4;

                this->actor.speedXZ = 3.0f;
                this->linearVelocity = 3.0f;
                this->actor.velocity.y = 6.0f;

                Player_ChangeAnimOnce(globalCtx, this,
                                      GET_PLAYER_ANIM(PLAYER_ANIMGROUP_RUNNING_DAMAGED, this->modelAnimType));
                Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_DAMAGE_S);
            } else {
                this->actor.speedXZ = knockbackVelXZ;
                this->linearVelocity = knockbackVelXZ;
                this->actor.velocity.y = knockbackVelY;

                if (ABS(damageYaw) > 0x4000) {
                    anim = &gPlayerAnim_002F58;
                } else {
                    anim = &gPlayerAnim_002DB0;
                }

                if ((this->actor.category != ACTORCAT_PLAYER) && (this->actor.colChkInfo.health == 0)) {
                    Player_PlayVoiceSfxForAge(this, NA_SE_VO_BL_DOWN);
                } else {
                    Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_FALL_L);
                }
            }

            this->hoverBootsTimer = 0;
            this->actor.bgCheckFlags &= ~1;
        } else {
            if ((this->linearVelocity > 4.0f) && !Player_IsUnfriendlyZTargeting(this)) {
                this->runDamageTimer = 20;
                Player_RequestRumble(this, 120, 20, 10, 0);
                Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_DAMAGE_S);
                return;
            }

            damageAnims = sLinkDamageAnims;

            Player_SetActionFunc(globalCtx, this, func_8084370C, 0);
            Player_ResetLeftRightBlendWeight(this);

            if (this->actor.colChkInfo.damage < 5) {
                Player_RequestRumble(this, 120, 20, 10, 0);
            } else {
                Player_RequestRumble(this, 180, 20, 100, 0);
                this->linearVelocity = 23.0f;
                damageAnims += 4;
            }

            if (ABS(damageYaw) <= 0x4000) {
                damageAnims += 2;
            }

            if (Player_IsUnfriendlyZTargeting(this)) {
                damageAnims += 1;
            }

            anim = *damageAnims;

            Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_DAMAGE_S);
        }

        this->actor.shape.rot.y += damageYaw;
        this->currentYaw = this->actor.shape.rot.y;
        this->actor.world.rot.y = this->actor.shape.rot.y;
        if (ABS(damageYaw) > 0x4000) {
            this->actor.shape.rot.y += 0x8000;
        }
    }

    Player_ResetAttributesAndHeldActor(globalCtx, this);

    this->stateFlags1 |= PLAYER_STATE1_TAKING_DAMAGE;

    if (anim != NULL) {
        Player_PlayAnimOnceSlowed(globalCtx, this, anim);
    }
}

s32 Player_GetHurtFloorType(s32 arg0) {
    s32 temp = arg0 - 2;

    if ((temp >= 0) && (temp < 2)) {
        return temp;
    } else {
        return -1;
    }
}

s32 Player_IsFloorSinkingSand(s32 arg0) {
    return (arg0 == 4) || (arg0 == 7) || (arg0 == 12);
}

void Player_BurnDekuShield(Player* this, GlobalContext* globalCtx) {
    if (this->currentShield == PLAYER_SHIELD_DEKU && (CVar_GetS32("gFireproofDekuShield", 0) == 0)) {
        Actor_Spawn(&globalCtx->actorCtx, globalCtx, ACTOR_ITEM_SHIELD, this->actor.world.pos.x,
                    this->actor.world.pos.y, this->actor.world.pos.z, 0, 0, 0, 1);
        Inventory_DeleteEquipment(globalCtx, EQUIP_SHIELD);
        Message_StartTextbox(globalCtx, 0x305F, NULL);
    }
}

void Player_StartBurning(Player* this) {
    s32 i;

    // clang-format off
    for (i = 0; i < 18; i++) { this->flameTimers[i] = Rand_S16Offset(0, 200); }
    // clang-format on

    this->isBurning = true;
}

void Player_PlayFallSfxAndCheckBurning(Player* this) {
    if (this->actor.colChkInfo.acHitEffect == 1 || CVar_GetS32("gFireDamage", 0)) {
        Player_StartBurning(this);
    }
    Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_FALL_L);
}

void Player_RoundUpInvincibilityTimer(Player* this) {
    if ((this->invincibilityTimer >= 0) && (this->invincibilityTimer < 20)) {
        this->invincibilityTimer = 20;
    }
}

s32 Player_UpdateDamage(Player* this, GlobalContext* globalCtx) {
    s32 pad;
    s32 sinkingGroundVoidOut = false;
    s32 attackHitShield;

    if (this->voidRespawnCounter != 0) {
        if (!Player_InBlockingCsMode(globalCtx, this)) {
            Player_InflictDamageModified(globalCtx, -16 * CVar_GetS32("gVoidDamageMul", 1), false);
            this->voidRespawnCounter = 0;
        }
    } else {
        sinkingGroundVoidOut = ((Player_GetHeight(this) - 8.0f) < (this->shapeOffsetY * this->actor.scale.y));

        if (sinkingGroundVoidOut || (this->actor.bgCheckFlags & 0x100) || (sFloorSpecialProperty == 9) ||
            (this->stateFlags2 & PLAYER_STATE2_FORCE_VOID_OUT)) {
            Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_DAMAGE_S);

            if (sinkingGroundVoidOut) {
                Gameplay_TriggerRespawn(globalCtx);
                func_800994A0(globalCtx);
            } else {
                // Special case for getting crushed in Forest Temple's Checkboard Ceiling Hall or Shadow Temple's
                // Falling Spike Trap Room, to respawn the player in a specific place
                if (((globalCtx->sceneNum == SCENE_BMORI1) && (globalCtx->roomCtx.curRoom.num == 15)) ||
                    ((globalCtx->sceneNum == SCENE_HAKADAN) && (globalCtx->roomCtx.curRoom.num == 10))) {
                    static SpecialRespawnInfo checkboardCeilingRespawn = { { 1992.0f, 403.0f, -3432.0f }, 0 };
                    static SpecialRespawnInfo fallingSpikeTrapRespawn = { { 1200.0f, -1343.0f, 3850.0f }, 0 };
                    SpecialRespawnInfo* respawnInfo;

                    if (globalCtx->sceneNum == SCENE_BMORI1) {
                        respawnInfo = &checkboardCeilingRespawn;
                    } else {
                        respawnInfo = &fallingSpikeTrapRespawn;
                    }

                    Gameplay_SetupRespawnPoint(globalCtx, RESPAWN_MODE_DOWN, 0xDFF);
                    gSaveContext.respawn[RESPAWN_MODE_DOWN].pos = respawnInfo->pos;
                    gSaveContext.respawn[RESPAWN_MODE_DOWN].yaw = respawnInfo->yaw;
                }

                Gameplay_TriggerVoidOut(globalCtx);
            }

            Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_TAKEN_AWAY);
            globalCtx->unk_11DE9 = 1;
            func_80078884(NA_SE_OC_ABYSS);
        } else if ((this->damageEffect != 0) && ((this->damageEffect >= 2) || (this->invincibilityTimer == 0))) {
            u8 sp5C[] = { 2, 1, 1 };

            Player_PlayFallSfxAndCheckBurning(this);

            if (this->damageEffect == 3) {
                this->shockTimer = 40;
            }

            this->actor.colChkInfo.damage += this->damageAmount;
            Player_SetupDamage(globalCtx, this, sp5C[this->damageEffect - 1], this->knockbackVelXZ, this->knockbackVelY,
                               this->damageYaw, 20);
        } else {
            attackHitShield = (this->shieldQuad.base.acFlags & AC_BOUNCED) != 0;

            //! @bug The second set of conditions here seems intended as a way for Link to "block" hits by rolling.
            // However, `Collider.atFlags` is a byte so the flag check at the end is incorrect and cannot work.
            // Additionally, `Collider.atHit` can never be set while already colliding as AC, so it's also bugged.
            // This behavior was later fixed in MM, most likely by removing both the `atHit` and `atFlags` checks.
            if (attackHitShield ||
                ((this->invincibilityTimer < 0) && (this->cylinder.base.acFlags & AC_HIT) &&
                 (this->cylinder.info.atHit != NULL) && (this->cylinder.info.atHit->atFlags & 0x20000000))) {

                Player_RequestRumble(this, 180, 20, 100, 0);

                if (!Player_IsChildWithHylianShield(this)) {
                    if (this->invincibilityTimer >= 0) {
                        LinkAnimationHeader* anim;
                        s32 sp54 = Player_AimShieldCrouched == this->actionFunc;

                        if (!Player_IsSwimming(this)) {
                            Player_SetActionFunc(globalCtx, this, Player_DeflectAttackWithShield, 0);
                        }

                        if (!(this->genericVar = sp54)) {
                            Player_SetUpperActionFunc(this, Player_EndDeflectAttackStanding);

                            if (this->leftRightBlendWeight < 0.5f) {
                                anim = sRightStandingDeflectWithShieldAnims[Player_HoldsTwoHandedWeapon(this) &&
                                                                            !(CVar_GetS32("gShieldTwoHanded", 0) &&
                                                                              (this->heldItemActionParam !=
                                                                               PLAYER_AP_STICK))];
                            } else {
                                anim = sLeftStandingDeflectWithShieldAnims[Player_HoldsTwoHandedWeapon(this) &&
                                                                           !(CVar_GetS32("gShieldTwoHanded", 0) &&
                                                                             (this->heldItemActionParam !=
                                                                              PLAYER_AP_STICK))];
                            }
                            LinkAnimation_PlayOnce(globalCtx, &this->skelAnimeUpper, anim);
                        } else {
                            Player_PlayAnimOnce(
                                globalCtx, this,
                                sDeflectWithShieldAnims[Player_HoldsTwoHandedWeapon(this) &&
                                                        !(CVar_GetS32("gShieldTwoHanded", 0) &&
                                                          (this->heldItemActionParam != PLAYER_AP_STICK))]);
                        }
                    }

                    if (!(this->stateFlags1 & (PLAYER_STATE1_HANGING_FROM_LEDGE_SLIP |
                                               PLAYER_STATE1_CLIMBING_ONTO_LEDGE | PLAYER_STATE1_CLIMBING))) {
                        this->linearVelocity = -18.0f;
                        this->currentYaw = this->actor.shape.rot.y;
                    }
                }

                if (attackHitShield && (this->shieldQuad.info.acHitInfo->toucher.effect == PLAYER_HITEFFECTAC_FIRE)) {
                    Player_BurnDekuShield(this, globalCtx);
                }

                return 0;
            }

            if ((this->deathTimer != 0) || (this->invincibilityTimer > 0) ||
                (this->stateFlags1 & PLAYER_STATE1_TAKING_DAMAGE) || (this->csMode != 0) ||
                (this->swordQuads[0].base.atFlags & AT_HIT) || (this->swordQuads[1].base.atFlags & AT_HIT)) {
                return 0;
            }

            if (this->cylinder.base.acFlags & AC_HIT) {
                Actor* ac = this->cylinder.base.ac;
                s32 damageReaction;

                if (ac->flags & ACTOR_FLAG_24) {
                    func_8002F7DC(&this->actor, NA_SE_PL_BODY_HIT);
                }

                u8 damageOverride = CVar_GetS32("gIceDamage", 0) || CVar_GetS32("gElectricDamage", 0) ||
                                    CVar_GetS32("gKnockbackDamage", 0);

                if (!damageOverride) {
                    if (this->stateFlags1 & PLAYER_STATE1_SWIMMING) {
                        damageReaction = PLAYER_DMGREACTION_DEFAULT;
                    } else if (this->actor.colChkInfo.acHitEffect == PLAYER_HITEFFECTAC_ICE) {
                        damageReaction = PLAYER_DMGREACTION_FROZEN;
                    } else if (this->actor.colChkInfo.acHitEffect == PLAYER_HITEFFECTAC_ELECTRIC) {
                        damageReaction = PLAYER_DMGREACTION_ELECTRIC_SHOCKED;
                    } else if (this->actor.colChkInfo.acHitEffect == PLAYER_HITEFFECTAC_POWERFUL_HIT) {
                        damageReaction = PLAYER_DMGREACTION_KNOCKBACK;
                    } else {
                        Player_PlayFallSfxAndCheckBurning(this);
                        damageReaction = PLAYER_DMGREACTION_DEFAULT;
                    }
                } else {
                    if (CVar_GetS32("gIceDamage", 0)) {
                        damageReaction = PLAYER_DMGREACTION_FROZEN;
                    }
                    if (CVar_GetS32("gElectricDamage", 0)) {
                        damageReaction = PLAYER_DMGREACTION_ELECTRIC_SHOCKED;
                    }
                    if (CVar_GetS32("gKnockbackDamage", 0)) {
                        damageReaction = PLAYER_DMGREACTION_KNOCKBACK;
                    }
                }

                Player_SetupDamage(globalCtx, this, damageReaction, 4.0f, 5.0f,
                                   Actor_WorldYawTowardActor(ac, &this->actor), 20);
            } else if (this->invincibilityTimer != 0) {
                return 0;
            } else {
                static u8 D_808544F4[] = { 120, 60 };
                s32 hurtFloorType = Player_GetHurtFloorType(sFloorSpecialProperty);

                if (((this->actor.wallPoly != NULL) &&
                     SurfaceType_IsWallDamage(&globalCtx->colCtx, this->actor.wallPoly, this->actor.wallBgId)) ||
                    ((hurtFloorType >= 0) &&
                     SurfaceType_IsWallDamage(&globalCtx->colCtx, this->actor.floorPoly, this->actor.floorBgId) &&
                     (this->hurtFloorTimer >= D_808544F4[hurtFloorType])) ||
                    ((hurtFloorType >= 0) && ((this->currentTunic != PLAYER_TUNIC_GORON && CVar_GetS32("gSuperTunic", 0) == 0) ||
                                     (this->hurtFloorTimer >= D_808544F4[hurtFloorType]))) ||
                    (CVar_GetS32("gFloorIsLava", 0) && this->actor.bgCheckFlags & BGCHECKFLAG_GROUND)) {
                    this->hurtFloorTimer = 0;
                    this->actor.colChkInfo.damage = 4;
                    Player_SetupDamage(globalCtx, this, 0, 4.0f, 5.0f, this->actor.shape.rot.y, 20);
                } else {
                    return 0;
                }
            }
        }
    }

    return 1;
}

void Player_SetupJumpWithSfx(Player* this, LinkAnimationHeader* anim, f32 arg2, GlobalContext* globalCtx, u16 sfxId) {
    Player_SetActionFunc(globalCtx, this, Player_UpdateMidair, 1);

    if (anim != NULL) {
        Player_PlayAnimOnceSlowed(globalCtx, this, anim);
    }

    this->actor.velocity.y = arg2 * sWaterSpeedScale;
    this->hoverBootsTimer = 0;
    this->actor.bgCheckFlags &= ~1;

    Player_PlayJumpSfx(this);
    Player_PlayVoiceSfxForAge(this, sfxId);

    this->stateFlags1 |= PLAYER_STATE1_JUMPING;
}

void Player_SetupJump(Player* this, LinkAnimationHeader* anim, f32 arg2, GlobalContext* globalCtx) {
    Player_SetupJumpWithSfx(this, anim, arg2, globalCtx, NA_SE_VO_LI_SWORD_N);
}

s32 Player_SetupWallJumpBehavior(Player* this, GlobalContext* globalCtx) {
    s32 canJumpToLedge;
    LinkAnimationHeader* anim;
    f32 wallHeight;
    f32 yVel;
    f32 wallPolyNormalX;
    f32 wallPolyNormalZ;
    f32 wallDist;

    if (!(this->stateFlags1 & PLAYER_STATE1_HOLDING_ACTOR) && (this->touchedWallJumpType >= 2) &&
        (!(this->stateFlags1 & PLAYER_STATE1_SWIMMING) || (this->ageProperties->unk_14 > this->wallHeight))) {
        canJumpToLedge = 0;

        if (Player_IsSwimming(this)) {
            if (this->actor.yDistToWater < 50.0f) {
                if ((this->touchedWallJumpType < 2) || (this->wallHeight > this->ageProperties->unk_10)) {
                    return 0;
                }
            } else if ((this->currentBoots != PLAYER_BOOTS_IRON) || (this->touchedWallJumpType > 2)) {
                return 0;
            }
        } else if (!(this->actor.bgCheckFlags & 1) || ((this->ageProperties->unk_14 <= this->wallHeight) &&
                                                       (this->stateFlags1 & PLAYER_STATE1_SWIMMING))) {
            return 0;
        }

        if ((this->actor.wallBgId != BGCHECK_SCENE) && (sTouchedWallFlags & 0x40)) {
            if (this->wallTouchTimer >= 6) {
                this->stateFlags2 |= PLAYER_STATE2_CAN_CLIMB_PUSH_PULL_WALL;
                if (CHECK_BTN_ALL(sControlInput->press.button, BTN_A)) {
                    canJumpToLedge = 1;
                }
            }
        } else if ((this->wallTouchTimer >= 6) || CHECK_BTN_ALL(sControlInput->press.button, BTN_A)) {
            canJumpToLedge = 1;
        }

        // Chaos
        if (CVar_GetS32("gDisableLedgeClimb", 0)) {
            canJumpToLedge = 0;
        }

        if (canJumpToLedge != 0) {
            Player_SetActionFunc(globalCtx, this, Player_JumpUpToLedge, 0);

            this->stateFlags1 |= PLAYER_STATE1_JUMPING;

            wallHeight = this->wallHeight;

            if (this->ageProperties->unk_14 <= wallHeight) {
                anim = &gPlayerAnim_002D48;
                this->linearVelocity = 1.0f;
            } else {
                wallPolyNormalX = COLPOLY_GET_NORMAL(this->actor.wallPoly->normal.x);
                wallPolyNormalZ = COLPOLY_GET_NORMAL(this->actor.wallPoly->normal.z);
                wallDist = this->wallDistance + 0.5f;

                this->stateFlags1 |= PLAYER_STATE1_CLIMBING_ONTO_LEDGE;

                if (Player_IsSwimming(this)) {
                    anim = &gPlayerAnim_0032E8;
                    wallHeight -= (60.0f * this->ageProperties->unk_08);
                    this->stateFlags1 &= ~PLAYER_STATE1_SWIMMING;
                } else if (this->ageProperties->unk_18 <= wallHeight) {
                    anim = &gPlayerAnim_002D40;
                    wallHeight -= (59.0f * this->ageProperties->unk_08);
                } else {
                    anim = &gPlayerAnim_002D38;
                    wallHeight -= (41.0f * this->ageProperties->unk_08);
                }

                this->actor.shape.yOffset -= wallHeight * 100.0f;

                this->actor.world.pos.x -= wallDist * wallPolyNormalX;
                this->actor.world.pos.y += this->wallHeight;
                this->actor.world.pos.z -= wallDist * wallPolyNormalZ;

                Player_ClearAttentionModeAndStopMoving(this);
            }

            this->actor.bgCheckFlags |= 1;

            LinkAnimation_PlayOnceSetSpeed(globalCtx, &this->skelAnime, anim, 1.3f);
            AnimationContext_DisableQueue(globalCtx);

            this->actor.shape.rot.y = this->currentYaw = this->actor.wallYaw + 0x8000;

            return 1;
        }
    } else if ((this->actor.bgCheckFlags & 1) && (this->touchedWallJumpType == 1) && (this->wallTouchTimer >= 3)) {
        yVel = (this->wallHeight * 0.08f) + 5.5f;
        Player_SetupJump(this, &gPlayerAnim_002FE0, yVel, globalCtx);
        this->linearVelocity = 2.5f;

        return 1;
    }

    return 0;
}

void Player_SetupMiniCsMovement(GlobalContext* globalCtx, Player* this, f32 arg2, s16 arg3) {
    Player_SetActionFunc(globalCtx, this, Player_MiniCsMovement, 0);
    Player_ResetAttributes(globalCtx, this);

    this->genericVar = 1;
    this->genericTimer = 1;

    this->csStartPos.x = (Math_SinS(arg3) * arg2) + this->actor.world.pos.x;
    this->csStartPos.z = (Math_CosS(arg3) * arg2) + this->actor.world.pos.z;

    Player_PlayAnimOnce(globalCtx, this, Player_GetStandingStillAnim(this));
}

void Player_SetupSwimIdle(GlobalContext* globalCtx, Player* this) {
    Player_SetActionFunc(globalCtx, this, Player_UpdateSwimIdle, 0);
    Player_ChangeAnimLongMorphLoop(globalCtx, this, &gPlayerAnim_003328);
}

void Player_SetupEnterGrotto(GlobalContext* globalCtx, Player* this) {
    Player_SetActionFunc(globalCtx, this, Player_EnterGrotto, 0);

    this->stateFlags1 |= PLAYER_STATE1_IN_CUTSCENE | PLAYER_STATE1_FALLING_INTO_GROTTO_OR_VOID;

    Camera_ChangeSetting(Gameplay_GetCamera(globalCtx, 0), CAM_SET_FREE0);
}

s32 Player_ShouldEnterGrotto(GlobalContext* globalCtx, Player* this) {
    if ((globalCtx->sceneLoadFlag == 0) && (this->stateFlags1 & PLAYER_STATE1_FALLING_INTO_GROTTO_OR_VOID)) {
        Player_SetupEnterGrotto(globalCtx, this);
        Player_PlayAnimLoop(globalCtx, this, &gPlayerAnim_003040);
        Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_FALL_S);
        func_800788CC(NA_SE_OC_SECRET_WARP_IN);
        return 1;
    }

    return 0;
}

s16 sReturnEntranceGroupData[] = {
    0x045B, // DMT from Magic Fairy Fountain
    0x0482, // DMC from Double Defense Fairy Fountain
    0x0340, // Hyrule Castle from Dins Fire Fairy Fountain
    0x044B, // Kakariko from Potion Shop
    0x02A2, // Market (child day) from Potion Shop
    0x0201, // Kakariko from Bazaar
    0x03B8, // Market (child day) from Bazaar
    0x04EE, // Kakariko from House of Skulltulas
    0x03C0, // Back Alley (day) from Bombchu Shop
    0x0463, // Kakariko from Shooting Gallery
    0x01CD, // Market (child day) from Shooting Gallery
    0x0394, // Zoras Fountain from Farores Wind Fairy Fountain
    0x0340, // Hyrule Castle from Dins Fire Fairy Fountain
    0x057C, // Desert Colossus from Nayrus Love Fairy Fountain
};

/**
 * The values are indices into `sReturnEntranceGroupData` marking the start of each group
 */
u8 sReturnEntranceGroupIndices[] = {
    11, // ENTR_RETURN_YOUSEI_IZUMI_YOKO
    9,  // ENTR_RETURN_SYATEKIJYOU
    3,  // ENTR_RETURN_2
    5,  // ENTR_RETURN_SHOP1
    7,  // ENTR_RETURN_4
    0,  // ENTR_RETURN_DAIYOUSEI_IZUMI
};

s32 Player_SetupExit(GlobalContext* globalCtx, Player* this, CollisionPoly* poly, u32 bgId) {
    s32 exitIndex;
    s32 floorSpecialProperty;
    s32 yDistToExit;
    f32 linearVel;
    s32 yaw;

    if (this->actor.category == ACTORCAT_PLAYER) {
        exitIndex = 0;

        if (!(this->stateFlags1 & PLAYER_STATE1_IN_DEATH_CUTSCENE) && (globalCtx->sceneLoadFlag == 0) &&
            (this->csMode == 0) && !(this->stateFlags1 & PLAYER_STATE1_EXITING_SCENE) &&
            (((poly != NULL) && (exitIndex = SurfaceType_GetSceneExitIndex(&globalCtx->colCtx, poly, bgId), exitIndex != 0)) ||
             (Player_IsFloorSinkingSand(sFloorSpecialProperty) && (this->floorProperty == 12)))) {

            yDistToExit = this->sceneExitPosY - (s32)this->actor.world.pos.y;

            if (!(this->stateFlags1 &
                  (PLAYER_STATE1_RIDING_HORSE | PLAYER_STATE1_SWIMMING | PLAYER_STATE1_IN_CUTSCENE)) &&
                !(this->actor.bgCheckFlags & 1) && (yDistToExit < 100) && (sPlayerYDistToFloor > 100.0f)) {
                return 0;
            }

            if (exitIndex == 0) {
                Gameplay_TriggerVoidOut(globalCtx);
                func_800994A0(globalCtx);
            } else {
                globalCtx->nextEntranceIndex = globalCtx->setupExitList[exitIndex - 1];
                if (globalCtx->nextEntranceIndex == 0x7FFF) {
                    gSaveContext.respawnFlag = 2;
                    globalCtx->nextEntranceIndex = gSaveContext.respawn[RESPAWN_MODE_RETURN].entranceIndex;
                    globalCtx->fadeTransition = 3;
                    gSaveContext.nextTransition = 3;
                } else if (globalCtx->nextEntranceIndex >= 0x7FF9) {
                    globalCtx->nextEntranceIndex =
                        sReturnEntranceGroupData[sReturnEntranceGroupIndices[globalCtx->nextEntranceIndex - 0x7FF9] + globalCtx->curSpawn];
                    func_800994A0(globalCtx);
                } else {
                    if (SurfaceType_GetSlope(&globalCtx->colCtx, poly, bgId) == 2) {
                        gSaveContext.respawn[RESPAWN_MODE_DOWN].entranceIndex = globalCtx->nextEntranceIndex;
                        Gameplay_TriggerVoidOut(globalCtx);
                        gSaveContext.respawnFlag = -2;
                    }
                    gSaveContext.unk_13C3 = 1;
                    func_800994A0(globalCtx);
                }
                globalCtx->sceneLoadFlag = 0x14;
            }

            if (!(this->stateFlags1 & (PLAYER_STATE1_RIDING_HORSE | PLAYER_STATE1_IN_CUTSCENE)) &&
                !(this->stateFlags2 & PLAYER_STATE2_INSIDE_CRAWLSPACE) && !Player_IsSwimming(this) &&
                (floorSpecialProperty = func_80041D4C(&globalCtx->colCtx, poly, bgId), (floorSpecialProperty != 10)) &&
                ((yDistToExit < 100) || (this->actor.bgCheckFlags & 1))) {

                if (floorSpecialProperty == 11) {
                    func_800788CC(NA_SE_OC_SECRET_HOLE_OUT);
                    func_800F6964(5);
                    gSaveContext.seqId = (u8)NA_BGM_DISABLED;
                    gSaveContext.natureAmbienceId = NATURE_ID_DISABLED;
                } else {
                    linearVel = this->linearVelocity;

                    if (linearVel < 0.0f) {
                        this->actor.world.rot.y += 0x8000;
                        linearVel = -linearVel;
                    }

                    if (linearVel > R_RUN_SPEED_LIMIT / 100.0f) {
                        gSaveContext.entranceSpeed = R_RUN_SPEED_LIMIT / 100.0f;
                    } else {
                        gSaveContext.entranceSpeed = linearVel;
                    }

                    if (sConveyorSpeedIndex != 0) {
                        yaw = sConveyorYaw;
                    } else {
                        yaw = this->actor.world.rot.y;
                    }
                    Player_SetupMiniCsMovement(globalCtx, this, 400.0f, yaw);
                }
            } else {
                if (!(this->actor.bgCheckFlags & 1)) {
                    Player_StopMovement(this);
                }
            }

            this->stateFlags1 |= PLAYER_STATE1_EXITING_SCENE | PLAYER_STATE1_IN_CUTSCENE;

            Player_ChangeCameraSetting(globalCtx, 0x2F);

            return 1;
        } else {
            if (globalCtx->sceneLoadFlag == 0) {

                if ((this->actor.world.pos.y < -4000.0f) ||
                    (((this->floorProperty == 5) || (this->floorProperty == 12)) &&
                     ((sPlayerYDistToFloor < 100.0f) || (this->fallDistance > 400.0f) ||
                      ((globalCtx->sceneNum != SCENE_HAKADAN) && (this->fallDistance > 200.0f)))) ||
                    ((globalCtx->sceneNum == SCENE_GANON_FINAL) && (this->fallDistance > 320.0f))) {

                    if (this->actor.bgCheckFlags & 1) {
                        if (this->floorProperty == 5) {
                            Gameplay_TriggerRespawn(globalCtx);
                        } else {
                            Gameplay_TriggerVoidOut(globalCtx);
                        }
                        globalCtx->fadeTransition = 4;
                        func_80078884(NA_SE_OC_ABYSS);
                    } else {
                        Player_SetupEnterGrotto(globalCtx, this);
                        this->genericTimer = 9999;
                        if (this->floorProperty == 5) {
                            this->genericVar = -1;
                        } else {
                            this->genericVar = 1;
                        }
                    }
                }

                this->sceneExitPosY = this->actor.world.pos.y;
            }
        }
    }

    return 0;
}

void Player_GetWorldPosRelativeToPlayer(Player* this, Vec3f* arg1, Vec3f* arg2, Vec3f* arg3) {
    f32 cos = Math_CosS(this->actor.shape.rot.y);
    f32 sin = Math_SinS(this->actor.shape.rot.y);

    arg3->x = arg1->x + ((arg2->x * cos) + (arg2->z * sin));
    arg3->y = arg1->y + arg2->y;
    arg3->z = arg1->z + ((arg2->z * cos) - (arg2->x * sin));
}

Actor* Player_SpawnFairy(GlobalContext* globalCtx, Player* this, Vec3f* arg2, Vec3f* arg3, s32 type) {
    Vec3f pos;

    Player_GetWorldPosRelativeToPlayer(this, arg2, arg3, &pos);

    return Actor_Spawn(&globalCtx->actorCtx, globalCtx, ACTOR_EN_ELF, pos.x, pos.y, pos.z, 0, 0, 0, type);
}

f32 Player_RaycastFloorWithOffset(GlobalContext* globalCtx, Player* this, Vec3f* raycastPosOffset, Vec3f* raycastPos, CollisionPoly** colPoly, s32* bgId) {
    Player_GetWorldPosRelativeToPlayer(this, &this->actor.world.pos, raycastPosOffset, raycastPos);

    return BgCheck_EntityRaycastFloor3(&globalCtx->colCtx, colPoly, bgId, raycastPos);
}

f32 Player_RaycastFloorWithOffset2(GlobalContext* globalCtx, Player* this, Vec3f* raycastPosOffset, Vec3f* raycastPos) {
    CollisionPoly* colPoly;
    s32 polyBgId;

    return Player_RaycastFloorWithOffset(globalCtx, this, raycastPosOffset, raycastPos, &colPoly, &polyBgId);
}

s32 Player_WallLineTestWithOffset(GlobalContext* globalCtx, Player* this, Vec3f* posOffset, CollisionPoly** wallPoly, s32* bgId, Vec3f* posResult) {
    Vec3f sp44;
    Vec3f sp38;

    sp44.x = this->actor.world.pos.x;
    sp44.y = this->actor.world.pos.y + posOffset->y;
    sp44.z = this->actor.world.pos.z;

    Player_GetWorldPosRelativeToPlayer(this, &this->actor.world.pos, posOffset, &sp38);

    return BgCheck_EntityLineTest1(&globalCtx->colCtx, &sp44, &sp38, posResult, wallPoly, true, false, false, true, bgId);
}

s32 Player_SetupOpenDoor(Player* this, GlobalContext* globalCtx) {
    DoorShutter* doorShutter;
    EnDoor* door; // Can also be DoorKiller*
    s32 doorDirection;
    f32 cos;
    f32 sin;
    Actor* doorActor;
    f32 doorOpeningPosOffset;
    s32 pad3;
    s32 frontRoom;
    Actor* attachedActor;
    LinkAnimationHeader* anim;
    CollisionPoly* floorPoly;
    Vec3f raycastPos;

    if ((this->doorType != PLAYER_DOORTYPE_NONE) &&
        (!(this->stateFlags1 & PLAYER_STATE1_HOLDING_ACTOR) ||
         ((this->heldActor != NULL) && (this->heldActor->id == ACTOR_EN_RU1)))) {
        if (CHECK_BTN_ALL(sControlInput->press.button, BTN_A) || (Player_SetupOpenDoorFromSpawn == this->actionFunc)) {
            doorActor = this->doorActor;

            if (this->doorType <= PLAYER_DOORTYPE_AJAR) {
                doorActor->textId = 0xD0;
                Player_StartTalkingWithActor(globalCtx, doorActor);
                return 0;
            }

            doorDirection = this->doorDirection;
            cos = Math_CosS(doorActor->shape.rot.y);
            sin = Math_SinS(doorActor->shape.rot.y);

            if (this->doorType == PLAYER_DOORTYPE_SLIDING) {
                doorShutter = (DoorShutter*)doorActor;

                this->currentYaw = doorShutter->dyna.actor.home.rot.y;
                if (doorDirection > 0) {
                    this->currentYaw -= 0x8000;
                }
                this->actor.shape.rot.y = this->currentYaw;

                if (this->linearVelocity <= 0.0f) {
                    this->linearVelocity = 0.1f;
                }

                Player_SetupMiniCsMovement(globalCtx, this, 50.0f, this->actor.shape.rot.y);

                this->genericVar = 0;
                this->csDoorType = this->doorType;
                this->stateFlags1 |= PLAYER_STATE1_IN_CUTSCENE;

                this->csStartPos.x = this->actor.world.pos.x + ((doorDirection * 20.0f) * sin);
                this->csStartPos.z = this->actor.world.pos.z + ((doorDirection * 20.0f) * cos);
                this->csEndPos.x = this->actor.world.pos.x + ((doorDirection * -120.0f) * sin);
                this->csEndPos.z = this->actor.world.pos.z + ((doorDirection * -120.0f) * cos);

                doorShutter->unk_164 = 1;
                Player_ClearAttentionModeAndStopMoving(this);

                if (this->doorTimer != 0) {
                    this->genericTimer = 0;
                    Player_ChangeAnimMorphToLastFrame(globalCtx, this, Player_GetStandingStillAnim(this));
                    this->skelAnime.endFrame = 0.0f;
                } else {
                    this->linearVelocity = 0.1f;
                }

                if (doorShutter->dyna.actor.category == ACTORCAT_DOOR) {
                    this->unk_46A = globalCtx->transiActorCtx.list[(u16)doorShutter->dyna.actor.params >> 10]
                                        .sides[(doorDirection > 0) ? 0 : 1]
                                        .effects;

                    func_800304B0(globalCtx);
                }
            } else {
                // This actor can be either EnDoor or DoorKiller.
                // Don't try to access any struct vars other than `animStyle` and `playerIsOpening`! These two variables
                // are common across the two actors' structs however most other variables are not!
                door = (EnDoor*)doorActor;

                door->animStyle = (doorDirection < 0.0f) ? (LINK_IS_ADULT ? KNOB_ANIM_ADULT_L : KNOB_ANIM_CHILD_L)
                                                         : (LINK_IS_ADULT ? KNOB_ANIM_ADULT_R : KNOB_ANIM_CHILD_R);

                if (door->animStyle == KNOB_ANIM_ADULT_L) {
                    anim = GET_PLAYER_ANIM(PLAYER_ANIMGROUP_OPEN_DOOR_ADULT_LEFT, this->modelAnimType);
                } else if (door->animStyle == KNOB_ANIM_CHILD_L) {
                    anim = GET_PLAYER_ANIM(PLAYER_ANIMGROUP_OPEN_DOOR_CHILD_LEFT, this->modelAnimType);
                } else if (door->animStyle == KNOB_ANIM_ADULT_R) {
                    anim = GET_PLAYER_ANIM(PLAYER_ANIMGROUP_OPEN_DOOR_ADULT_RIGHT, this->modelAnimType);
                } else {
                    anim = GET_PLAYER_ANIM(PLAYER_ANIMGROUP_OPEN_DOOR_CHILD_RIGHT, this->modelAnimType);
                }

                Player_SetActionFunc(globalCtx, this, Player_OpenDoor, 0);
                Player_UnequipItem(globalCtx, this);

                if (doorDirection < 0) {
                    this->actor.shape.rot.y = doorActor->shape.rot.y;
                } else {
                    this->actor.shape.rot.y = doorActor->shape.rot.y - 0x8000;
                }

                this->currentYaw = this->actor.shape.rot.y;

                doorOpeningPosOffset = (doorDirection * 22.0f);
                this->actor.world.pos.x = doorActor->world.pos.x + doorOpeningPosOffset * sin;
                this->actor.world.pos.z = doorActor->world.pos.z + doorOpeningPosOffset * cos;

                Player_PlayAnimOnceWithWaterInfluence(globalCtx, this, anim);

                if (this->doorTimer != 0) {
                    this->skelAnime.endFrame = 0.0f;
                }

                Player_ClearAttentionModeAndStopMoving(this);
                Player_SetupAnimMovement(globalCtx, this, 0x28F);

                if (doorActor->parent != NULL) {
                    doorDirection = -doorDirection;
                }

                door->playerIsOpening = 1;

                if (this->doorType != PLAYER_DOORTYPE_FAKE) {
                    this->stateFlags1 |= PLAYER_STATE1_IN_CUTSCENE;
                    func_800304B0(globalCtx);

                    if (((doorActor->params >> 7) & 7) == 3) {
                        raycastPos.x = doorActor->world.pos.x - (doorOpeningPosOffset * sin);
                        raycastPos.y = doorActor->world.pos.y + 10.0f;
                        raycastPos.z = doorActor->world.pos.z - (doorOpeningPosOffset * cos);

                        BgCheck_EntityRaycastFloor1(&globalCtx->colCtx, &floorPoly, &raycastPos);

                        if (Player_SetupExit(globalCtx, this, floorPoly, BGCHECK_SCENE)) {
                            gSaveContext.entranceSpeed = 2.0f;
                            gSaveContext.entranceSound = NA_SE_OC_DOOR_OPEN;
                        }
                    } else {
                        Camera_ChangeDoorCam(Gameplay_GetCamera(globalCtx, 0), doorActor,
                                             globalCtx->transiActorCtx.list[(u16)doorActor->params >> 10]
                                                 .sides[(doorDirection > 0) ? 0 : 1]
                                                 .effects,
                                             0, 38.0f * sInvertedWaterSpeedScale, 26.0f * sInvertedWaterSpeedScale,
                                             10.0f * sInvertedWaterSpeedScale);
                    }
                }
            }

            if ((this->doorType != PLAYER_DOORTYPE_FAKE) && (doorActor->category == ACTORCAT_DOOR)) {
                frontRoom = globalCtx->transiActorCtx.list[(u16)doorActor->params >> 10]
                                .sides[(doorDirection > 0) ? 0 : 1]
                                .room;

                if ((frontRoom >= 0) && (frontRoom != globalCtx->roomCtx.curRoom.num)) {
                    func_8009728C(globalCtx, &globalCtx->roomCtx, frontRoom);
                }
            }

            doorActor->room = globalCtx->roomCtx.curRoom.num;

            if (((attachedActor = doorActor->child) != NULL) || ((attachedActor = doorActor->parent) != NULL)) {
                attachedActor->room = globalCtx->roomCtx.curRoom.num;
            }

            return 1;
        }
    }

    return 0;
}

void Player_SetupUnfriendlyZTargetStandStill(Player* this, GlobalContext* globalCtx) {
    LinkAnimationHeader* anim;

    Player_SetActionFunc(globalCtx, this, Player_UnfriendlyZTargetStandingStill, 1);

    if (this->leftRightBlendWeight < 0.5f) {
        anim = Player_GetFightingRightAnim(this);
        this->leftRightBlendWeight = 0.0f;
    } else {
        anim = Player_GetFightingLeftAnim(this);
        this->leftRightBlendWeight = 1.0f;
    }

    this->leftRightBlendWeightTarget = this->leftRightBlendWeight;
    Player_PlayAnimLoop(globalCtx, this, anim);
    this->currentYaw = this->actor.shape.rot.y;
}

void Player_SetupFriendlyZTargetingStandStill(Player* this, GlobalContext* globalCtx) {
    Player_SetActionFunc(globalCtx, this, Player_FriendlyZTargetStandingStill, 1);
    Player_ChangeAnimMorphToLastFrame(globalCtx, this, Player_GetStandingStillAnim(this));
    this->currentYaw = this->actor.shape.rot.y;
}

void Player_SetupStandingStillType(Player* this, GlobalContext* globalCtx) {
    if (Player_IsUnfriendlyZTargeting(this)) {
        Player_SetupUnfriendlyZTargetStandStill(this, globalCtx);
    } else if (Player_IsFriendlyZTargeting(this)) {
        Player_SetupFriendlyZTargetingStandStill(this, globalCtx);
    } else {
        Player_SetupStandingStillMorph(this, globalCtx);
    }
}

void Player_ReturnToStandStill(Player* this, GlobalContext* globalCtx) {
    PlayerActionFunc func;

    if (Player_IsUnfriendlyZTargeting(this)) {
        func = Player_UnfriendlyZTargetStandingStill;
    } else if (Player_IsFriendlyZTargeting(this)) {
        func = Player_FriendlyZTargetStandingStill;
    } else {
        func = Player_StandingStill;
    }

    Player_SetActionFunc(globalCtx, this, func, 1);
}

void Player_SetupReturnToStandStill(Player* this, GlobalContext* globalCtx) {
    Player_ReturnToStandStill(this, globalCtx);
    if (Player_IsUnfriendlyZTargeting(this)) {
        this->genericTimer = 1;
    }
}

void Player_SetupReturnToStandStillSetAnim(Player* this, LinkAnimationHeader* anim, GlobalContext* globalCtx) {
    Player_SetupReturnToStandStill(this, globalCtx);
    Player_PlayAnimOnceWithWaterInfluence(globalCtx, this, anim);
}

s32 Player_CanHoldActor(Player* this) {
    return (this->interactRangeActor != NULL) && (this->heldActor == NULL);
}

void Player_SetupHoldActor(GlobalContext* globalCtx, Player* this) {
    if (Player_CanHoldActor(this)) {
        Actor* interactRangeActor = this->interactRangeActor;
        s32 interactActorId = interactRangeActor->id;

        if (interactActorId == ACTOR_BG_TOKI_SWD) {
            this->interactRangeActor->parent = &this->actor;
            Player_SetActionFunc(globalCtx, this, Player_SetDrawAndStartCutsceneAfterTimer, 0);
            this->stateFlags1 |= PLAYER_STATE1_IN_CUTSCENE;
            sMaskMemory = PLAYER_MASK_NONE;
        } else {
            LinkAnimationHeader* anim;

            if (interactActorId == ACTOR_BG_HEAVY_BLOCK) {
                Player_SetActionFunc(globalCtx, this, Player_ThrowStonePillar, 0);
                this->stateFlags1 |= PLAYER_STATE1_IN_CUTSCENE;
                anim = &gPlayerAnim_002F98;
            } else if ((interactActorId == ACTOR_EN_ISHI) && ((interactRangeActor->params & 0xF) == 1)) {
                Player_SetActionFunc(globalCtx, this, Player_LiftSilverBoulder, 0);
                anim = &gPlayerAnim_0032B0;
            } else if (((interactActorId == ACTOR_EN_BOMBF) || (interactActorId == ACTOR_EN_KUSA)) &&
                       (Player_GetStrength() <= PLAYER_STR_NONE)) {
                Player_SetActionFunc(globalCtx, this, Player_FailToLiftActor, 0);
                this->actor.world.pos.x =
                    (Math_SinS(interactRangeActor->yawTowardsPlayer) * 20.0f) + interactRangeActor->world.pos.x;
                this->actor.world.pos.z =
                    (Math_CosS(interactRangeActor->yawTowardsPlayer) * 20.0f) + interactRangeActor->world.pos.z;
                this->currentYaw = this->actor.shape.rot.y = interactRangeActor->yawTowardsPlayer + 0x8000;
                anim = &gPlayerAnim_003060;
            } else {
                Player_SetActionFunc(globalCtx, this, Player_LiftActor, 0);
                anim = GET_PLAYER_ANIM(PLAYER_ANIMGROUP_HOLDING_OBJECT, this->modelAnimType);
            }

            Player_PlayAnimOnce(globalCtx, this, anim);
        }
    } else {
        Player_SetupStandingStillType(this, globalCtx);
        this->stateFlags1 &= ~PLAYER_STATE1_HOLDING_ACTOR;
    }
}

void Player_SetupTalkWithActor(GlobalContext* globalCtx, Player* this) {
    Player_SetActionFuncPreserveMoveFlags(globalCtx, this, Player_TalkWithActor, 0);

    this->stateFlags1 |= PLAYER_STATE1_TALKING | PLAYER_STATE1_IN_CUTSCENE;

    if (this->actor.textId != 0) {
        Message_StartTextbox(globalCtx, this->actor.textId, this->talkActor);
        this->targetActor = this->talkActor;
    }
}

void Player_SetupRideHorse(GlobalContext* globalCtx, Player* this) {
    Player_SetActionFuncPreserveMoveFlags(globalCtx, this, Player_RideHorse, 0);
}

void Player_SetupGrabPushPullWall(GlobalContext* globalCtx, Player* this) {
    Player_SetActionFunc(globalCtx, this, Player_GrabPushPullWall, 0);
}

void Player_SetupClimbingWallOrDownLedge(GlobalContext* globalCtx, Player* this) {
    s32 preservedTimer = this->genericTimer;
    s32 preservedVar = this->genericVar;

    Player_SetActionFuncPreserveMoveFlags(globalCtx, this, Player_ClimbingWallOrDownLedge, 0);
    this->actor.velocity.y = 0.0f;

    this->genericTimer = preservedTimer;
    this->genericVar = preservedVar;
}

void Player_SetupInsideCrawlspace(GlobalContext* globalCtx, Player* this) {
    Player_SetActionFuncPreserveMoveFlags(globalCtx, this, Player_InsideCrawlspace, 0);
}

void Player_SetupGetItem(GlobalContext* globalCtx, Player* this) {
    Player_SetActionFuncPreserveMoveFlags(globalCtx, this, Player_GetItem, 0);

    this->stateFlags1 |= PLAYER_STATE1_GETTING_ITEM | PLAYER_STATE1_IN_CUTSCENE;

    if (this->getItemId == GI_HEART_CONTAINER_2) {
        this->genericTimer = 20;
    } else if (this->getItemId >= 0) {
        this->genericTimer = 1;
    } else {
        this->getItemId = -this->getItemId;
    }
}

s32 Player_StartJump(Player* this, GlobalContext* globalCtx) {
    s16 yawDiff;
    LinkAnimationHeader* anim;
    f32 yVel;

    yawDiff = this->currentYaw - this->actor.shape.rot.y;

    if ((ABS(yawDiff) < 0x1000) && (this->linearVelocity > 4.0f)) {
        anim = &gPlayerAnim_003148;
    } else {
        anim = &gPlayerAnim_002FE0;
    }

    if (this->linearVelocity > (IREG(66) / 100.0f)) {
        yVel = IREG(67) / 100.0f;
    } else {
        yVel = (IREG(68) / 100.0f) + ((IREG(69) * this->linearVelocity) / 1000.0f);
    }

    Player_SetupJumpWithSfx(this, anim, yVel, globalCtx, NA_SE_VO_LI_AUTO_JUMP);
    this->genericTimer = 1;

    return 1;
}

void Player_SetupGrabLedge(GlobalContext* globalCtx, Player* this, CollisionPoly* arg2, f32 arg3, LinkAnimationHeader* arg4) {
    f32 sp24 = COLPOLY_GET_NORMAL(arg2->normal.x);
    f32 sp20 = COLPOLY_GET_NORMAL(arg2->normal.z);

    Player_SetActionFunc(globalCtx, this, Player_GrabLedge, 0);
    Player_ResetAttributesAndHeldActor(globalCtx, this);
    Player_PlayAnimOnce(globalCtx, this, arg4);

    this->actor.world.pos.x -= (arg3 + 1.0f) * sp24;
    this->actor.world.pos.z -= (arg3 + 1.0f) * sp20;
    this->actor.shape.rot.y = this->currentYaw = Math_Atan2S(sp20, sp24);

    Player_ClearAttentionModeAndStopMoving(this);
    Player_AnimUpdatePrevTranslRot(this);
}

s32 Player_SetupGrabLedgeInsteadOfFalling(Player* this, GlobalContext* globalCtx) {
    CollisionPoly* colPoly;
    s32 polyBgId;
    Vec3f pos;
    Vec3f colPolyPos;
    f32 dist;

    if ((this->actor.yDistToWater < -80.0f) && (ABS(this->angleToFloorX) < 2730) && (ABS(this->angleToFloorY) < 2730)) {
        pos.x = this->actor.prevPos.x - this->actor.world.pos.x;
        pos.z = this->actor.prevPos.z - this->actor.world.pos.z;

        dist = sqrtf(SQ(pos.x) + SQ(pos.z));
        if (dist != 0.0f) {
            dist = 5.0f / dist;
        } else {
            dist = 0.0f;
        }

        pos.x = this->actor.prevPos.x + (pos.x * dist);
        pos.y = this->actor.world.pos.y;
        pos.z = this->actor.prevPos.z + (pos.z * dist);

        if (BgCheck_EntityLineTest1(&globalCtx->colCtx, &this->actor.world.pos, &pos, &colPolyPos, &colPoly, true, false, false,
                                    true, &polyBgId) &&
            ((ABS(colPoly->normal.y) < 600) || (CVar_GetS32("gClimbEverything", 0) != 0))) {
            f32 nx = COLPOLY_GET_NORMAL(colPoly->normal.x);
            f32 ny = COLPOLY_GET_NORMAL(colPoly->normal.y);
            f32 nz = COLPOLY_GET_NORMAL(colPoly->normal.z);
            f32 distToPoly;
            s32 shouldClimbDownAdjacentWall;

            distToPoly = Math3D_UDistPlaneToPos(nx, ny, nz, colPoly->dist, &this->actor.world.pos);

            shouldClimbDownAdjacentWall = sFloorProperty == 6;
            if (!shouldClimbDownAdjacentWall && (func_80041DB8(&globalCtx->colCtx, colPoly, polyBgId) & 8)) {
                shouldClimbDownAdjacentWall = 1;
            }

            Player_SetupGrabLedge(globalCtx, this, colPoly, distToPoly, shouldClimbDownAdjacentWall ? &gPlayerAnim_002D88 : &gPlayerAnim_002F10);

            if (shouldClimbDownAdjacentWall) {
                Player_SetupMiniCsFunc(globalCtx, this, Player_SetupClimbingWallOrDownLedge);

                this->currentYaw += 0x8000;
                this->actor.shape.rot.y = this->currentYaw;

                this->stateFlags1 |= PLAYER_STATE1_CLIMBING;
                Player_SetupAnimMovement(globalCtx, this, 0x9F);

                this->genericTimer = -1;
                this->genericVar = shouldClimbDownAdjacentWall;
            } else {
                this->stateFlags1 |= PLAYER_STATE1_HANGING_FROM_LEDGE_SLIP;
                this->stateFlags1 &= ~PLAYER_STATE1_Z_TARGETING_FRIENDLY;
            }

            func_8002F7DC(&this->actor, NA_SE_PL_SLIPDOWN);
            Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_HANG);
            return 1;
        }
    }

    return 0;
}

void Player_SetupClimbOntoLedge(Player* this, LinkAnimationHeader* anim, GlobalContext* globalCtx) {
    Player_SetActionFunc(globalCtx, this, Player_ClimbOntoLedge, 0);
    LinkAnimation_PlayOnceSetSpeed(globalCtx, &this->skelAnime, anim, 1.3f);
}

static Vec3f sWaterRaycastOffset = { 0.0f, 0.0f, 100.0f };

void Player_SetupMidairBehavior(Player* this, GlobalContext* globalCtx) {
    s32 yawDiff;
    CollisionPoly* floorPoly;
    s32 floorBgId;
    WaterBox* waterbox;
    Vec3f raycastPos;
    f32 floorPosY;
    f32 waterPosY;

    this->fallDistance = this->fallStartHeight - (s32)this->actor.world.pos.y;

    if (!(this->stateFlags1 & (PLAYER_STATE1_SWIMMING | PLAYER_STATE1_IN_CUTSCENE)) &&
        !(this->actor.bgCheckFlags & 1)) {
        if (!Player_ShouldEnterGrotto(globalCtx, this)) {
            if (sFloorProperty == 8) {
                this->actor.world.pos.x = this->actor.prevPos.x;
                this->actor.world.pos.z = this->actor.prevPos.z;
                return;
            }

            if (!(this->stateFlags3 & PLAYER_STATE3_MIDAIR) && !(this->skelAnime.moveFlags & 0x80) &&
                (Player_UpdateMidair != this->actionFunc) && (Player_FallingDive != this->actionFunc)) {

                if ((sFloorProperty == 7) || (this->swordState != 0)) {
                    Math_Vec3f_Copy(&this->actor.world.pos, &this->actor.prevPos);
                    Player_StopMovement(this);
                    return;
                }

                if (this->hoverBootsTimer != 0) {
                    this->actor.velocity.y = 1.0f;
                    sFloorProperty = 9;
                    return;
                }

                yawDiff = (s16)(this->currentYaw - this->actor.shape.rot.y);

                Player_SetActionFunc(globalCtx, this, Player_UpdateMidair, 1);
                Player_ResetAttributes(globalCtx, this);

                this->surfaceMaterial = this->prevSurfaceMaterial;

                if ((this->actor.bgCheckFlags & 4) && !(this->stateFlags1 & PLAYER_STATE1_SWIMMING) &&
                    (sFloorProperty != 6) && (sFloorProperty != 9) && (sPlayerYDistToFloor > 20.0f) &&
                    (this->swordState == 0) && (ABS(yawDiff) < 0x2000) && (this->linearVelocity > 3.0f)) {

                    if ((sFloorProperty == 11) && !(this->stateFlags1 & PLAYER_STATE1_HOLDING_ACTOR)) {

                        floorPosY = Player_RaycastFloorWithOffset(globalCtx, this, &sWaterRaycastOffset, &raycastPos, &floorPoly, &floorBgId);
                        waterPosY = this->actor.world.pos.y;

                        if (WaterBox_GetSurface1(globalCtx, &globalCtx->colCtx, raycastPos.x, raycastPos.z, &waterPosY, &waterbox) &&
                            ((waterPosY - floorPosY) > 50.0f)) {
                            Player_SetupJump(this, &gPlayerAnim_003158, 6.0f, globalCtx);
                            Player_SetActionFunc(globalCtx, this, Player_FallingDive, 0);
                            return;
                        }
                    }

                    Player_StartJump(this, globalCtx);
                    return;
                }

                if ((sFloorProperty == 9) || (sPlayerYDistToFloor <= this->ageProperties->unk_34) ||
                    !Player_SetupGrabLedgeInsteadOfFalling(this, globalCtx)) {
                    Player_PlayAnimLoop(globalCtx, this, &gPlayerAnim_003040);
                    return;
                }
            }
        }
    } else {
        this->fallStartHeight = this->actor.world.pos.y;
    }
}

s32 Player_SetupCameraMode(GlobalContext* globalCtx, Player* this) {
    s32 cameraMode;

    if (!(CVar_GetS32("gDisableFPSView", 0))) {
        if (this->attentionMode == PLAYER_ATTENTIONMODE_AIMING) {
            if (Actor_PlayerIsAimingFpsItem(this)) {
                if (LINK_IS_ADULT) {
                    cameraMode = CAM_MODE_BOWARROW;
                } else {
                    cameraMode = CAM_MODE_SLINGSHOT;
                }
            } else {
                cameraMode = CAM_MODE_BOOMERANG;
            }
        } else {
            cameraMode = CAM_MODE_FIRSTPERSON;
        }

        return Camera_ChangeMode(Gameplay_GetCamera(globalCtx, 0), cameraMode);
    }
}

s32 Player_SetupCutscene(GlobalContext* globalCtx, Player* this) {
    if (this->attentionMode == PLAYER_ATTENTIONMODE_CUTSCENE) {
        Player_SetActionFunc(globalCtx, this, Player_StartCutscene, 0);
        if (this->unk_46A != 0) {
            this->stateFlags1 |= PLAYER_STATE1_IN_CUTSCENE;
        }
        Player_InactivateMeleeWeapon(this);
        return 1;
    } else {
        return 0;
    }
}

void Player_LoadGetItemObject(Player* this, s16 objectId) {
    s32 pad;
    size_t size;

    if (objectId != OBJECT_INVALID) {
        this->giObjectLoading = true;
        osCreateMesgQueue(&this->giObjectLoadQueue, &this->giObjectLoadMsg, 1);

        size = gObjectTable[objectId].vromEnd - gObjectTable[objectId].vromStart;

        LOG_HEX("size", size);
        ASSERT(size <= 1024 * 8);

        DmaMgr_SendRequest2(&this->giObjectDmaRequest, (uintptr_t)this->giObjectSegment,
                            gObjectTable[objectId].vromStart, size, 0, &this->giObjectLoadQueue, OS_MESG_PTR(NULL),
                            __FILE__, __LINE__);
    }
}

void Player_SetupMagicSpell(GlobalContext* globalCtx, Player* this, s32 magicSpell) {
    Player_SetActionFuncPreserveItemAP(globalCtx, this, Player_UpdateMagicSpell, 0);

    this->genericVar = magicSpell - 3;
    func_80087708(globalCtx, sMagicSpellCosts[magicSpell], 4);

    LinkAnimation_PlayOnceSetSpeed(globalCtx, &this->skelAnime, &gPlayerAnim_002D28, 0.83f);

    if (magicSpell == 5) {
        this->subCamId = OnePointCutscene_Init(globalCtx, 1100, -101, NULL, MAIN_CAM);
    } else {
        Player_SetCameraTurnAround(globalCtx, 10);
    }
}

void Player_ResetLookAngles(Player* this) {
    this->actor.focus.rot.x = this->actor.focus.rot.z = this->headRot.x = this->headRot.y = this->headRot.z =
        this->upperBodyRot.x = this->upperBodyRot.y = this->upperBodyRot.z = 0;

    this->actor.focus.rot.y = this->actor.shape.rot.y;
}

static u8 sExchangeGetItemIDs[] = {
    GI_LETTER_ZELDA, GI_WEIRD_EGG,    GI_CHICKEN,     GI_BEAN,        GI_POCKET_EGG,   GI_POCKET_CUCCO,
    GI_COJIRO,       GI_ODD_MUSHROOM, GI_ODD_POTION,  GI_SAW,         GI_SWORD_BROKEN, GI_PRESCRIPTION,
    GI_FROG,         GI_EYEDROPS,     GI_CLAIM_CHECK, GI_MASK_SKULL,  GI_MASK_SPOOKY,  GI_MASK_KEATON,
    GI_MASK_BUNNY,   GI_MASK_TRUTH,   GI_MASK_GORON,  GI_MASK_ZORA,   GI_MASK_GERUDO,  GI_LETTER_RUTO,
    GI_LETTER_RUTO,  GI_LETTER_RUTO,  GI_LETTER_RUTO, GI_LETTER_RUTO, GI_LETTER_RUTO,
};

static LinkAnimationHeader* sExchangeItemAnims[] = {
    &gPlayerAnim_002F88,
    &gPlayerAnim_002690,
    &gPlayerAnim_003198,
};

s32 Player_SetupItemCutsceneOrFirstPerson(Player* this, GlobalContext* globalCtx) {
    s32 item;
    s32 sp28;
    GetItemEntry* giEntry;
    Actor* targetActor;

    if ((this->attentionMode != 0) && (Player_IsSwimming(this) || (this->actor.bgCheckFlags & 1) ||
                                       (this->stateFlags1 & PLAYER_STATE1_RIDING_HORSE))) {

        if (!Player_SetupCutscene(globalCtx, this)) {
            if (this->attentionMode == 4) {
                item = Player_ActionToMagicSpell(this, this->itemActionParam);
                if (item >= 0) {
                    if ((item != 3) || (gSaveContext.respawn[RESPAWN_MODE_TOP].data <= 0)) {
                        Player_SetupMagicSpell(globalCtx, this, item);
                    } else {
                        Player_SetActionFunc(globalCtx, this, Player_ChooseFaroresWindOption, 1);
                        this->stateFlags1 |= PLAYER_STATE1_SKIP_OTHER_ACTORS_UPDATE | PLAYER_STATE1_IN_CUTSCENE;
                        Player_PlayAnimOnce(globalCtx, this, Player_GetStandingStillAnim(this));
                        Player_SetCameraTurnAround(globalCtx, 4);
                    }

                    Player_ClearAttentionModeAndStopMoving(this);
                    return 1;
                }

                item = this->itemActionParam - PLAYER_AP_LETTER_ZELDA;
                if ((item >= 0) ||
                    (sp28 = Player_ActionToBottle(this, this->itemActionParam) - 1,
                     ((sp28 >= 0) && (sp28 < 6) &&
                      ((this->itemActionParam > PLAYER_AP_BOTTLE_POE) ||
                       ((this->talkActor != NULL) &&
                        (((this->itemActionParam == PLAYER_AP_BOTTLE_POE) && (this->exchangeItemId == EXCH_ITEM_POE)) ||
                         (this->exchangeItemId == EXCH_ITEM_BLUE_FIRE))))))) {

                    if ((globalCtx->actorCtx.titleCtx.delayTimer == 0) && (globalCtx->actorCtx.titleCtx.alpha == 0)) {
                        Player_SetActionFuncPreserveItemAP(globalCtx, this, Player_PresentExchangeItem, 0);

                        if (item >= 0) {
                            giEntry = &sGetItemTable[sExchangeGetItemIDs[item] - 1];
                            Player_LoadGetItemObject(this, giEntry->objectId);
                        }

                        this->stateFlags1 |=
                            PLAYER_STATE1_TALKING | PLAYER_STATE1_SKIP_OTHER_ACTORS_UPDATE | PLAYER_STATE1_IN_CUTSCENE;

                        if (item >= 0) {
                            item = item + 1;
                        } else {
                            item = sp28 + 0x18;
                        }

                        targetActor = this->talkActor;

                        if ((targetActor != NULL) &&
                            ((this->exchangeItemId == item) || (this->exchangeItemId == EXCH_ITEM_BLUE_FIRE) ||
                             ((this->exchangeItemId == EXCH_ITEM_POE) &&
                              (this->itemActionParam == PLAYER_AP_BOTTLE_BIG_POE)) ||
                             ((this->exchangeItemId == EXCH_ITEM_BEAN) &&
                              (this->itemActionParam == PLAYER_AP_BOTTLE_BUG))) &&
                            ((this->exchangeItemId != EXCH_ITEM_BEAN) || (this->itemActionParam == PLAYER_AP_BEAN))) {
                            if (this->exchangeItemId == EXCH_ITEM_BEAN) {
                                Inventory_ChangeAmmo(ITEM_BEAN, -1);
                                Player_SetActionFuncPreserveItemAP(globalCtx, this, func_8084279C, 0);
                                this->stateFlags1 |= PLAYER_STATE1_IN_CUTSCENE;
                                this->genericTimer = 0x50;
                                this->genericVar = -1;
                            }
                            targetActor->flags |= ACTOR_FLAG_8;
                            this->targetActor = this->talkActor;
                        } else if (item == EXCH_ITEM_LETTER_RUTO) {
                            this->genericVar = 1;
                            this->actor.textId = 0x4005;
                            Player_SetCameraTurnAround(globalCtx, 1);
                        } else {
                            this->genericVar = 2;
                            this->actor.textId = 0xCF;
                            Player_SetCameraTurnAround(globalCtx, 4);
                        }

                        this->actor.flags |= ACTOR_FLAG_8;
                        this->exchangeItemId = item;

                        if (this->genericVar < 0) {
                            Player_ChangeAnimMorphToLastFrame(
                                globalCtx, this,
                                GET_PLAYER_ANIM(PLAYER_ANIMGROUP_START_CHECKING_OR_SPEAKING, this->modelAnimType));
                        } else {
                            Player_PlayAnimOnce(globalCtx, this, sExchangeItemAnims[this->genericVar]);
                        }

                        Player_ClearAttentionModeAndStopMoving(this);
                    }
                    return 1;
                }

                item = Player_ActionToBottle(this, this->itemActionParam);
                if (item >= 0) {
                    if (item == 0xC) {
                        Player_SetActionFuncPreserveItemAP(globalCtx, this, Player_HealWithFairy, 0);
                        Player_PlayAnimOnceSlowed(globalCtx, this, &gPlayerAnim_002650);
                        Player_SetCameraTurnAround(globalCtx, 3);
                    } else if ((item > 0) && (item < 4)) {
                        Player_SetActionFuncPreserveItemAP(globalCtx, this, Player_DropItemFromBottle, 0);
                        Player_PlayAnimOnceSlowed(globalCtx, this, &gPlayerAnim_002688);
                        Player_SetCameraTurnAround(globalCtx, (item == 1) ? 1 : 5);
                    } else {
                        Player_SetActionFuncPreserveItemAP(globalCtx, this, Player_DrinkFromBottle, 0);
                        Player_ChangeAnimSlowedMorphToLastFrame(globalCtx, this, &gPlayerAnim_002668);
                        Player_SetCameraTurnAround(globalCtx, 2);
                    }
                } else {
                    Player_SetActionFuncPreserveItemAP(globalCtx, this, Player_PlayOcarina, 0);
                    Player_PlayAnimOnceSlowed(globalCtx, this, &gPlayerAnim_0030A0);
                    this->stateFlags2 |= PLAYER_STATE2_PLAYING_OCARINA_GENERAL;
                    Player_SetCameraTurnAround(globalCtx, (this->ocarinaActor != NULL) ? 0x5B : 0x5A);
                    if (this->ocarinaActor != NULL) {
                        this->stateFlags2 |= PLAYER_STATE2_PLAYING_OCARINA_FOR_ACTOR;
                        Camera_SetParam(Gameplay_GetCamera(globalCtx, 0), 8, this->ocarinaActor);
                    }
                }
            } else if (Player_SetupCameraMode(globalCtx, this)) {
                if (!(this->stateFlags1 & PLAYER_STATE1_RIDING_HORSE)) {
                    Player_SetActionFunc(globalCtx, this, Player_FirstPersonAiming, 1);
                    this->genericTimer = 13;
                    Player_ResetLookAngles(this);
                }
                this->stateFlags1 |= PLAYER_STATE1_IN_FIRST_PERSON_MODE;
                func_80078884(NA_SE_SY_CAMERA_ZOOM_UP);
                Player_StopMovement(this);
                return 1;
            } else {
                this->attentionMode = 0;
                func_80078884(NA_SE_SY_ERROR);
                return 0;
            }

            this->stateFlags1 |= PLAYER_STATE1_SKIP_OTHER_ACTORS_UPDATE | PLAYER_STATE1_IN_CUTSCENE;
        }

        Player_ClearAttentionModeAndStopMoving(this);
        return 1;
    }

    return 0;
}

s32 Player_SetupSpeakOrCheck(Player* this, GlobalContext* globalCtx) {
    Actor* talkActor = this->talkActor;
    Actor* targetActor = this->targetActor;
    Actor* naviActor = NULL;
    s32 naviHasText = 0;
    s32 targetActorHasText;

    targetActorHasText = (targetActor != NULL) && (CHECK_FLAG_ALL(targetActor->flags, ACTOR_FLAG_0 | ACTOR_FLAG_18) || (targetActor->naviEnemyId != 0xFF));

    if (targetActorHasText || (this->naviTextId != 0)) {
        naviHasText = (this->naviTextId < 0) && ((ABS(this->naviTextId) & 0xFF00) != 0x200);
        if (naviHasText || !targetActorHasText) {
            naviActor = this->naviActor;
            if (naviHasText) {
                targetActor = NULL;
                talkActor = NULL;
            }
        } else {
            naviActor = targetActor;
        }
    }

    if ((talkActor != NULL) || (naviActor != NULL)) {
        if ((targetActor == NULL) || (targetActor == talkActor) || (targetActor == naviActor)) {
            if (!(this->stateFlags1 & PLAYER_STATE1_HOLDING_ACTOR) ||
                ((this->heldActor != NULL) && (naviHasText || (talkActor == this->heldActor) || (naviActor == this->heldActor) ||
                                               ((talkActor != NULL) && (talkActor->flags & ACTOR_FLAG_16))))) {
                if ((this->actor.bgCheckFlags & 1) || (this->stateFlags1 & PLAYER_STATE1_RIDING_HORSE) ||
                    (Player_IsSwimming(this) && !(this->stateFlags2 & PLAYER_STATE2_DIVING))) {

                    if (talkActor != NULL) {
                        this->stateFlags2 |= PLAYER_STATE2_CAN_SPEAK_OR_CHECK;
                        if (CHECK_BTN_ALL(sControlInput->press.button, BTN_A) || (talkActor->flags & ACTOR_FLAG_16)) {
                            naviActor = NULL;
                        } else if (naviActor == NULL) {
                            return 0;
                        }
                    }

                    if (naviActor != NULL) {
                        if (!naviHasText) {
                            this->stateFlags2 |= PLAYER_STATE2_NAVI_REQUESTING_TALK;
                        }

                        if (!CHECK_BTN_ALL(sControlInput->press.button, BTN_CUP) && !naviHasText) {
                            return 0;
                        }

                        talkActor = naviActor;
                        this->talkActor = NULL;

                        if (naviHasText || !targetActorHasText) {
                            if (this->naviTextId >= 0) {
                                naviActor->textId = this->naviTextId;
                            } else {
                                naviActor->textId = -this->naviTextId;
                            }
                        } else {
                            if (naviActor->naviEnemyId != 0xFF) {
                                naviActor->textId = naviActor->naviEnemyId + 0x600;
                            }
                        }
                    }

                    this->currentMask = sCurrentMask;
                    Player_StartTalkingWithActor(globalCtx, talkActor);
                    return 1;
                }
            }
        }
    }

    return 0;
}

s32 Player_ForceFirstPerson(Player* this, GlobalContext* globalCtx) {
    if (!(this->stateFlags1 & (PLAYER_STATE1_HOLDING_ACTOR | PLAYER_STATE1_RIDING_HORSE)) &&
        Camera_CheckValidMode(Gameplay_GetCamera(globalCtx, 0), 6)) {
        if ((this->actor.bgCheckFlags & 1) ||
            (Player_IsSwimming(this) && (this->actor.yDistToWater < this->ageProperties->unk_2C))) {
            this->attentionMode = 1;
            return 1;
        }
    }

    return 0;
}

s32 Player_SetupCUpBehavior(Player* this, GlobalContext* globalCtx) {
    if (this->attentionMode != 0) {
        Player_SetupItemCutsceneOrFirstPerson(this, globalCtx);
        return 1;
    }

    if ((this->targetActor != NULL) && (CHECK_FLAG_ALL(this->targetActor->flags, ACTOR_FLAG_0 | ACTOR_FLAG_18) ||
                                        (this->targetActor->naviEnemyId != 0xFF))) {
        this->stateFlags2 |= PLAYER_STATE2_NAVI_REQUESTING_TALK;
    } else if ((this->naviTextId == 0) && !Player_IsUnfriendlyZTargeting(this) &&
               CHECK_BTN_ALL(sControlInput->press.button, BTN_CUP) && (YREG(15) != 0x10) && (YREG(15) != 0x20) &&
               !Player_ForceFirstPerson(this, globalCtx)) {
        func_80078884(NA_SE_SY_ERROR);
    }

    return 0;
}

void Player_SetupJumpSlash(GlobalContext* globalCtx, Player* this, s32 arg2, f32 xzVelocity, f32 yVelocity) {
    Player_StartMeleeWeaponAttack(globalCtx, this, arg2);
    Player_SetActionFunc(globalCtx, this, Player_JumpSlash, 0);

    this->stateFlags3 |= PLAYER_STATE3_MIDAIR;

    this->currentYaw = this->actor.shape.rot.y;
    this->linearVelocity = xzVelocity;
    this->actor.velocity.y = yVelocity;

    this->actor.bgCheckFlags &= ~1;
    this->hoverBootsTimer = 0;

    Player_PlayJumpSfx(this);
    Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_SWORD_L);
}

s32 Player_CanJumpSlash(Player* this) {
    if (!(this->stateFlags1 & PLAYER_STATE1_SHIELDING) && (Player_GetSwordHeld(this) != 0)) {
        if (sUsingItemAlreadyInHand ||
            ((this->actor.category != ACTORCAT_PLAYER) && CHECK_BTN_ALL(sControlInput->press.button, BTN_B))) {
            return 1;
        }
    }

    return 0;
}

s32 Player_SetupMidairJumpSlash(Player* this, GlobalContext* globalCtx) {
    if (Player_CanJumpSlash(this) && (sFloorSpecialProperty != 7)) {
        Player_SetupJumpSlash(globalCtx, this, 17, 3.0f, 4.5f);
        return 1;
    }

    return 0;
}

void Player_SetupRolling(Player* this, GlobalContext* globalCtx) {
    EnBom* bomb;

    // Chaos
    if (CVar_GetS32("gExplodingRolls", 0)) {
        bomb = (EnBom*)Actor_Spawn(&globalCtx->actorCtx, globalCtx, ACTOR_EN_BOM, this->actor.world.pos.x,
                                   this->actor.world.pos.y, this->actor.world.pos.z, 0, 0, 6, BOMB_BODY);
        if (bomb != NULL) {
            bomb->timer = 0;
        }

        Player_Damage(globalCtx, this, -16);
        Player_SetupDamage(globalCtx, this, PLAYER_DMGREACTION_KNOCKBACK, 0.0f, 0.0f, 0, 20);
    }
    if (CVar_GetS32("gFreezingRolls", 0)) {
        this->actor.colChkInfo.damage = 0;
        Player_SetupDamage(globalCtx, this, PLAYER_DMGREACTION_FROZEN, 0.0f, 0.0f, 0, 20);
    }
    else {
        Player_SetActionFunc(globalCtx, this, Player_Rolling, 0);

        LinkAnimation_PlayOnceSetSpeed(globalCtx, &this->skelAnime,
                                       GET_PLAYER_ANIM(PLAYER_ANIMGROUP_ROLLING, this->modelAnimType),
                                       1.25f * sWaterSpeedScale);
    }

}

s32 Player_CanRoll(Player* this, GlobalContext* globalCtx) {
    if ((this->relativeAnalogStickInputs[this->inputFrameCounter] == 0) && (sFloorSpecialProperty != 7)) {
        Player_SetupRolling(this, globalCtx);
        return 1;
    }

    return 0;
}

void Player_SetupBackflipOrSidehop(Player* this, GlobalContext* globalCtx, s32 relativeStickInput) {
    Player_SetupJumpWithSfx(this, D_80853D4C[relativeStickInput][0], !(relativeStickInput & 1) ? 5.8f : 3.5f, globalCtx, NA_SE_VO_LI_SWORD_N);

    if (relativeStickInput) {}

    this->genericTimer = 1;
    this->genericVar = relativeStickInput;

    this->currentYaw = this->actor.shape.rot.y + (relativeStickInput << 0xE);
    this->linearVelocity = !(relativeStickInput & 1) ? 6.0f : 8.5f;

    this->stateFlags2 |= PLAYER_STATE2_BACKFLIPPING_OR_SIDEHOPPING;

    func_8002F7DC(&this->actor, ((relativeStickInput << 0xE) == 0x8000) ? NA_SE_PL_ROLL : NA_SE_PL_SKIP);
}

s32 Player_SetupJumpSlashOrRoll(Player* this, GlobalContext* globalCtx) {
    s32 relativeStickInput;

    if (CHECK_BTN_ALL(sControlInput->press.button, BTN_A) && (globalCtx->roomCtx.curRoom.unk_03 != 2) &&
        (sFloorSpecialProperty != 7) &&
        (SurfaceType_GetSlope(&globalCtx->colCtx, this->actor.floorPoly, this->actor.floorBgId) != 1)) {
        relativeStickInput = this->relativeAnalogStickInputs[this->inputFrameCounter];

        if (relativeStickInput <= 0) {
            if (Player_IsZTargeting(this)) {
                if (this->actor.category != ACTORCAT_PLAYER) {
                    if (relativeStickInput < 0) {
                        Player_SetupJump(this, &gPlayerAnim_002FE0, REG(69) / 100.0f, globalCtx);
                    } else {
                        Player_SetupRolling(this, globalCtx);
                    }
                } else {
                    if (Player_GetSwordHeld(this) && Player_CanUseItem(this)) {
                        Player_SetupJumpSlash(globalCtx, this, 17, 5.0f, 5.0f);
                    } else {
                        Player_SetupRolling(this, globalCtx);
                    }
                }
                return 1;
            }
        } else {
            Player_SetupBackflipOrSidehop(this, globalCtx, relativeStickInput);
            return 1;
        }
    }

    return 0;
}

void Player_EndRun(Player* this, GlobalContext* globalCtx) {
    LinkAnimationHeader* anim;
    f32 frame;

    frame = this->walkFrame - 3.0f;
    if (frame < 0.0f) {
        frame += 29.0f;
    }

    if (frame < 14.0f) {
        anim = GET_PLAYER_ANIM(PLAYER_ANIMGROUP_END_WALK_ON_LEFT_FOOT, this->modelAnimType);
        frame = 11.0f - frame;
        if (frame < 0.0f) {
            frame = 1.375f * -frame;
        }
        frame /= 11.0f;
    } else {
        anim = GET_PLAYER_ANIM(PLAYER_ANIMGROUP_END_WALK_ON_RIGHT_FOOT, this->modelAnimType);
        frame = 26.0f - frame;
        if (frame < 0.0f) {
            frame = 2 * -frame;
        }
        frame /= 12.0f;
    }

    LinkAnimation_Change(globalCtx, &this->skelAnime, anim, 1.0f, 0.0f, Animation_GetLastFrame(anim), ANIMMODE_ONCE,
                         4.0f * frame);
    this->currentYaw = this->actor.shape.rot.y;
}

void Player_SetupEndRun(Player* this, GlobalContext* globalCtx) {
    Player_ReturnToStandStill(this, globalCtx);
    Player_EndRun(this, globalCtx);
}

void Player_SetupStandingStillNoMorph(Player* this, GlobalContext* globalCtx) {
    Player_SetActionFunc(globalCtx, this, Player_StandingStill, 1);
    Player_PlayAnimOnce(globalCtx, this, Player_GetStandingStillAnim(this));
    this->currentYaw = this->actor.shape.rot.y;
}

void Player_ClearLookAndAttention(Player* this, GlobalContext* globalCtx) {
    if (!(this->stateFlags3 & PLAYER_STATE3_MOVING_ALONG_HOOKSHOT_PATH)) {
        Player_ResetLookAngles(this);
        if (this->stateFlags1 & PLAYER_STATE1_SWIMMING) {
            Player_SetupSwimIdle(globalCtx, this);
        } else {
            Player_SetupStandingStillType(this, globalCtx);
        }
        if (this->attentionMode < PLAYER_ATTENTIONMODE_ITEM_CUTSCENE) {
            this->attentionMode = PLAYER_ATTENTIONMODE_NONE;
        }
    }

    this->stateFlags1 &= ~(PLAYER_STATE1_HANGING_FROM_LEDGE_SLIP | PLAYER_STATE1_CLIMBING_ONTO_LEDGE |
                           PLAYER_STATE1_IN_FIRST_PERSON_MODE);
}

s32 Player_SetupRollOrPutAway(Player* this, GlobalContext* globalCtx) {
    if (CVar_GetS32("gSonicRoll", 0)) {
        Player_SetupRolling(this, globalCtx);
    } else if (!Player_SetupStartUnfriendlyZTargeting(this) && (D_808535E0 == 0) &&
               !(this->stateFlags1 & PLAYER_STATE1_RIDING_HORSE) && CHECK_BTN_ALL(sControlInput->press.button, BTN_A)) {
        if (Player_CanRoll(this, globalCtx)) {
            return 1;
        }
        if ((this->putAwayTimer == 0) && (this->heldItemActionParam >= PLAYER_AP_SWORD_MASTER)) {
            Player_UseItem(globalCtx, this, ITEM_NONE);
        } else {
            this->stateFlags2 ^= PLAYER_STATE2_NAVI_IS_ACTIVE;
        }
    }

    return 0;
}

s32 Player_SetupDefend(Player* this, GlobalContext* globalCtx) {
    LinkAnimationHeader* anim;
    f32 frame;

    if ((globalCtx->shootingGalleryStatus == 0) && (this->currentShield != PLAYER_SHIELD_NONE) &&
        CHECK_BTN_ALL(sControlInput->cur.button, BTN_R) &&
        (Player_IsChildWithHylianShield(this) || (!Player_IsFriendlyZTargeting(this) && (this->targetActor == NULL)))) {

        Player_InactivateMeleeWeapon(this);
        Player_DetatchHeldActor(globalCtx, this);

        if (Player_SetActionFunc(globalCtx, this, Player_AimShieldCrouched, 0)) {
            this->stateFlags1 |= PLAYER_STATE1_SHIELDING;

            if (!Player_IsChildWithHylianShield(this)) {
                Player_SetModelsForHoldingShield(this);
                anim = GET_PLAYER_ANIM(PLAYER_ANIMGROUP_START_DEFENDING, this->modelAnimType);
            } else {
                anim = &gPlayerAnim_002400;
            }

            if (anim != this->skelAnime.animation) {
                if (Player_IsUnfriendlyZTargeting(this)) {
                    this->unk_86C = 1.0f;
                } else {
                    this->unk_86C = 0.0f;
                    Player_ResetLeftRightBlendWeight(this);
                }
                this->upperBodyRot.x = this->upperBodyRot.y = this->upperBodyRot.z = 0;
            }

            frame = Animation_GetLastFrame(anim);
            LinkAnimation_Change(globalCtx, &this->skelAnime, anim, 1.0f, frame, frame, ANIMMODE_ONCE, 0.0f);

            if (Player_IsChildWithHylianShield(this)) {
                Player_SetupAnimMovement(globalCtx, this, 4);
            }

            func_8002F7DC(&this->actor, NA_SE_IT_SHIELD_POSTURE);
        }

        return 1;
    }

    return 0;
}

s32 Player_SetupTurnAroundRunning(Player* this, f32* targetVelocity, s16* targetYaw) {
    s16 yawDiff = this->currentYaw - *targetYaw;

    if (ABS(yawDiff) > DEGF_TO_BINANG(135.0f)) {
        if (Player_StepLinearVelocityToZero(this)) {
            *targetVelocity = 0.0f;
            *targetYaw = this->currentYaw;
        } else {
            return 1;
        }
    }

    return 0;
}

void Player_SetupDeactivateComboTimer(Player* this) {
    if ((this->comboTimer > 0) && !CHECK_BTN_ALL(sControlInput->cur.button, BTN_B)) {
        this->comboTimer = -this->comboTimer;
    }
}

s32 Player_SetupStartChargeSpinAttack(Player* this, GlobalContext* globalCtx) {
    if (CHECK_BTN_ALL(sControlInput->cur.button, BTN_B)) {
        if (!(this->stateFlags1 & PLAYER_STATE1_SHIELDING) && (Player_GetSwordHeld(this) != 0) &&
            (this->comboTimer == 1) && (this->heldItemActionParam != PLAYER_AP_STICK)) {
            if ((this->heldItemActionParam != PLAYER_AP_SWORD_BGS) || (gSaveContext.swordHealth > 0.0f)) {
                Player_StartChargeSpinAttack(globalCtx, this);
                return 1;
            }
        }
    } else {
        Player_SetupDeactivateComboTimer(this);
    }

    return 0;
}

s32 Player_SetupThrowDekuNut(GlobalContext* globalCtx, Player* this) {
    if ((globalCtx->roomCtx.curRoom.unk_03 != 2) && (this->actor.bgCheckFlags & 1) && (AMMO(ITEM_NUT) != 0)) {
        Player_SetActionFunc(globalCtx, this, Player_ThrowDekuNut, 0);
        Player_PlayAnimOnce(globalCtx, this, &gPlayerAnim_003048);
        this->attentionMode = 0;
        return 1;
    }

    return 0;
}

static BottleSwingAnimInfo sBottleSwingAnims[] = {
    { &gPlayerAnim_002648, &gPlayerAnim_002640, 2, 3 },
    { &gPlayerAnim_002680, &gPlayerAnim_002678, 5, 3 },
};

s32 Player_CanSwingBottleOrCastFishingRod(GlobalContext* globalCtx, Player* this) {
    Vec3f checkPos;

    if (sUsingItemAlreadyInHand) {
        if (Player_GetBottleHeld(this) >= 0) {
            Player_SetActionFunc(globalCtx, this, Player_SwingBottle, 0);

            if (this->actor.yDistToWater > 12.0f) {
                this->genericTimer = 1;
            }

            Player_PlayAnimOnceSlowed(globalCtx, this, sBottleSwingAnims[this->genericTimer].bottleSwingAnim);

            func_8002F7DC(&this->actor, NA_SE_IT_SWORD_SWING);
            Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_AUTO_JUMP);
            return 1;
        }

        if (this->heldItemActionParam == PLAYER_AP_FISHING_POLE) {
            checkPos = this->actor.world.pos;
            checkPos.y += 50.0f;

            if (CVar_GetS32("gHoverFishing", 0)
                    ? 0
                    : !(this->actor.bgCheckFlags & 1) || (this->actor.world.pos.z > 1300.0f) ||
                          BgCheck_SphVsFirstPoly(&globalCtx->colCtx, &checkPos, 20.0f)) {
                func_80078884(NA_SE_SY_ERROR);
                return 0;
            }

            Player_SetActionFunc(globalCtx, this, Player_CastFishingRod, 0);
            this->fpsItemType = 1;
            Player_StopMovement(this);
            Player_PlayAnimOnce(globalCtx, this, &gPlayerAnim_002C30);
            return 1;
        } else {
            return 0;
        }
    }

    return 0;
}

void Player_SetupRun(Player* this, GlobalContext* globalCtx) {
    PlayerActionFunc func;

    if (Player_IsZTargeting(this)) {
        func = Player_ZTargetingRun;
    } else {
        func = Player_Run;
    }

    Player_SetActionFunc(globalCtx, this, func, 1);
    Player_ChangeAnimShortMorphLoop(globalCtx, this, GET_PLAYER_ANIM(PLAYER_ANIMGROUP_RUNNING, this->modelAnimType));

    this->walkAngleToFloorX = 0;
    this->unk_864 = this->walkFrame = 0.0f;
}

void Player_SetupZTargetRunning(Player* this, GlobalContext* globalCtx, s16 arg2) {
    this->actor.shape.rot.y = this->currentYaw = arg2;
    Player_SetupRun(this, globalCtx);
}

s32 Player_SetupDefaultSpawnBehavior(GlobalContext* globalCtx, Player* this, f32 arg2) {
    WaterBox* waterbox;
    f32 posY;

    posY = this->actor.world.pos.y;
    if (WaterBox_GetSurface1(globalCtx, &globalCtx->colCtx, this->actor.world.pos.x, this->actor.world.pos.z, &posY,
                             &waterbox) != 0) {
        posY -= this->actor.world.pos.y;
        if (this->ageProperties->unk_24 <= posY) {
            Player_SetActionFunc(globalCtx, this, Player_SpawnSwimming, 0);
            Player_ChangeAnimLongMorphLoop(globalCtx, this, &gPlayerAnim_0032F0);
            this->stateFlags1 |= PLAYER_STATE1_SWIMMING | PLAYER_STATE1_IN_CUTSCENE;
            this->genericTimer = 20;
            this->linearVelocity = 2.0f;
            Player_SetBootData(globalCtx, this);
            return 0;
        }
    }

    Player_SetupMiniCsMovement(globalCtx, this, arg2, this->actor.shape.rot.y);
    this->stateFlags1 |= PLAYER_STATE1_IN_CUTSCENE;
    return 1;
}

void Player_SpawnNoMomentum(GlobalContext* globalCtx, Player* this) {
    if (Player_SetupDefaultSpawnBehavior(globalCtx, this, 180.0f)) {
        this->genericTimer = -20;
    }
}

void Player_SpawnWalkingSlow(GlobalContext* globalCtx, Player* this) {
    this->linearVelocity = 2.0f;
    gSaveContext.entranceSpeed = 2.0f;
    if (Player_SetupDefaultSpawnBehavior(globalCtx, this, 120.0f)) {
        this->genericTimer = -15;
    }
}

void Player_SpawnWalkingPreserveMomentum(GlobalContext* globalCtx, Player* this) {
    if (gSaveContext.entranceSpeed < 0.1f) {
        gSaveContext.entranceSpeed = 0.1f;
    }

    this->linearVelocity = gSaveContext.entranceSpeed;

    if (Player_SetupDefaultSpawnBehavior(globalCtx, this, 800.0f)) {
        this->genericTimer = -80 / this->linearVelocity;
        if (this->genericTimer < -20) {
            this->genericTimer = -20;
        }
    }
}

void Player_SetupFriendlyBackwalk(Player* this, s16 yaw, GlobalContext* globalCtx) {
    Player_SetActionFunc(globalCtx, this, Player_FriendlyBackwalk, 1);
    LinkAnimation_CopyJointToMorph(globalCtx, &this->skelAnime);
    this->unk_864 = this->walkFrame = 0.0f;
    this->currentYaw = yaw;
}

void Player_SetupFriendlySidewalk(Player* this, GlobalContext* globalCtx) {
    Player_SetActionFunc(globalCtx, this, Player_EndSidewalk, 1);
    Player_ChangeAnimShortMorphLoop(globalCtx, this, GET_PLAYER_ANIM(PLAYER_ANIMGROUP_WALKING, this->modelAnimType));
}

void Player_SetupUnfriendlyBackwalk(Player* this, s16 yaw, GlobalContext* globalCtx) {
    Player_SetActionFunc(globalCtx, this, Player_UnfriendlyBackwalk, 1);
    LinkAnimation_Change(globalCtx, &this->skelAnime, &gPlayerAnim_0024F8, 2.2f, 0.0f,
                         Animation_GetLastFrame(&gPlayerAnim_0024F8), ANIMMODE_ONCE, -6.0f);
    this->linearVelocity = 8.0f;
    this->currentYaw = yaw;
}

void Player_SetupSidewalk(Player* this, GlobalContext* globalCtx) {
    Player_SetActionFunc(globalCtx, this, Player_Sidewalk, 1);
    Player_ChangeAnimShortMorphLoop(globalCtx, this,
                                    GET_PLAYER_ANIM(PLAYER_ANIMGROUP_SIDEWALKING_RIGHT, this->modelAnimType));
    this->walkFrame = 0.0f;
}

void Player_SetupEndUnfriendlyBackwalk(Player* this, GlobalContext* globalCtx) {
    Player_SetActionFunc(globalCtx, this, Player_EndUnfriendlyBackwalk, 1);
    LinkAnimation_PlayOnceSetSpeed(globalCtx, &this->skelAnime, &gPlayerAnim_0024E8, 2.0f);
}

void Player_SetupTurn(GlobalContext* globalCtx, Player* this, s16 yaw) {
    this->currentYaw = yaw;
    Player_SetActionFunc(globalCtx, this, Player_Turn, 1);
    this->unk_87E = 1200;
    this->unk_87E *= sWaterSpeedScale;
    LinkAnimation_Change(globalCtx, &this->skelAnime,
                         GET_PLAYER_ANIM(PLAYER_ANIMGROUP_SHUFFLE_TURN, this->modelAnimType),
                         1.0f, 0.0f, 0.0f, ANIMMODE_LOOP, -6.0f);
}

void Player_EndUnfriendlyZTarget(Player* this, GlobalContext* globalCtx) {
    LinkAnimationHeader* anim;

    Player_SetActionFunc(globalCtx, this, Player_StandingStill, 1);

    if (this->leftRightBlendWeight < 0.5f) {
        anim = GET_PLAYER_ANIM(PLAYER_ANIMGROUP_END_FIGHTING_RIGHT_OF_ENEMY, this->modelAnimType);
    } else {
        anim = GET_PLAYER_ANIM(PLAYER_ANIMGROUP_END_FIGHTING_LEFT_OF_ENEMY, this->modelAnimType);
    }
    Player_PlayAnimOnce(globalCtx, this, anim);

    this->currentYaw = this->actor.shape.rot.y;
}

void Player_SetupUnfriendlyZTarget(Player* this, GlobalContext* globalCtx) {
    Player_SetActionFunc(globalCtx, this, Player_UnfriendlyZTargetStandingStill, 1);
    Player_ChangeAnimMorphToLastFrame(globalCtx, this,
                                      GET_PLAYER_ANIM(PLAYER_ANIMGROUP_START_FIGHTING, this->modelAnimType));
    this->genericTimer = 1;
}

void Player_SetupEndUnfriendlyZTarget(Player* this, GlobalContext* globalCtx) {
    if (this->linearVelocity != 0.0f) {
        Player_SetupRun(this, globalCtx);
    } else {
        Player_EndUnfriendlyZTarget(this, globalCtx);
    }
}

void Player_EndMiniCsMovement(Player* this, GlobalContext* globalCtx) {
    if (this->linearVelocity != 0.0f) {
        Player_SetupRun(this, globalCtx);
    } else {
        Player_SetupStandingStillType(this, globalCtx);
    }
}

s32 Player_SetupSpawnSplash(GlobalContext* globalCtx, Player* this, f32 yVelocity, s32 splashScale) {
    f32 yVelEnteringWater = fabsf(yVelocity);
    WaterBox* waterbox;
    f32 waterSurfaceY;
    Vec3f splashPos;
    s32 splashType;

    if (yVelEnteringWater > 2.0f) {
        splashPos.x = this->bodyPartsPos[PLAYER_BODYPART_WAIST].x;
        splashPos.z = this->bodyPartsPos[PLAYER_BODYPART_WAIST].z;
        waterSurfaceY = this->actor.world.pos.y;
        if (WaterBox_GetSurface1(globalCtx, &globalCtx->colCtx, splashPos.x, splashPos.z, &waterSurfaceY, &waterbox)) {
            if ((waterSurfaceY - this->actor.world.pos.y) < 100.0f) {
                splashType = (yVelEnteringWater <= 10.0f) ? 0 : 1;
                splashPos.y = waterSurfaceY;
                EffectSsGSplash_Spawn(globalCtx, &splashPos, NULL, NULL, splashType, splashScale);
                return 1;
            }
        }
    }

    return 0;
}

void Player_StartJumpOutOfWater(GlobalContext* globalCtx, Player* this, f32 yVelocity) {
    this->stateFlags1 |= PLAYER_STATE1_JUMPING;
    this->stateFlags1 &= ~PLAYER_STATE1_SWIMMING;

    Player_ResetSubCam(globalCtx, this);
    if (Player_SetupSpawnSplash(globalCtx, this, yVelocity, 500)) {
        func_8002F7DC(&this->actor, NA_SE_EV_JUMP_OUT_WATER);
    }

    Player_SetBootData(globalCtx, this);
}

s32 Player_SetupDive(GlobalContext* globalCtx, Player* this, Input* arg2) {
    if (!(this->stateFlags1 & PLAYER_STATE1_GETTING_ITEM) && !(this->stateFlags2 & PLAYER_STATE2_DIVING)) {
        if ((arg2 == NULL) || (CHECK_BTN_ALL(arg2->press.button, BTN_A) && (ABS(this->shapePitchOffset) < 12000) &&
                               (this->currentBoots != PLAYER_BOOTS_IRON))) {

            Player_SetActionFunc(globalCtx, this, Player_Dive, 0);
            Player_PlayAnimOnce(globalCtx, this, &gPlayerAnim_003308);

            this->shapePitchOffset = 0;
            this->stateFlags2 |= PLAYER_STATE2_DIVING;
            this->actor.velocity.y = 0.0f;

            if (arg2 != NULL) {
                this->stateFlags2 |= PLAYER_STATE2_ENABLE_DIVE_CAMERA_AND_TIMER;
                func_8002F7DC(&this->actor, NA_SE_PL_DIVE_BUBBLE);
            }

            return 1;
        }
    }

    if ((this->stateFlags1 & PLAYER_STATE1_GETTING_ITEM) || (this->stateFlags2 & PLAYER_STATE2_DIVING)) {
        if (this->actor.velocity.y > 0.0f) {
            if (this->actor.yDistToWater < this->ageProperties->unk_30) {

                this->stateFlags2 &= ~PLAYER_STATE2_DIVING;

                if (arg2 != NULL) {
                    Player_SetActionFunc(globalCtx, this, Player_GetItemInWater, 1);

                    if (this->stateFlags1 & PLAYER_STATE1_GETTING_ITEM) {
                        this->stateFlags1 |=
                            PLAYER_STATE1_GETTING_ITEM | PLAYER_STATE1_HOLDING_ACTOR | PLAYER_STATE1_IN_CUTSCENE;
                    }

                    this->genericTimer = 2;
                }

                Player_ResetSubCam(globalCtx, this);
                Player_ChangeAnimMorphToLastFrame(
                    globalCtx, this,
                    (this->stateFlags1 & PLAYER_STATE1_HOLDING_ACTOR) ? &gPlayerAnim_003318 : &gPlayerAnim_003300);

                if (Player_SetupSpawnSplash(globalCtx, this, this->actor.velocity.y, 500)) {
                    func_8002F7DC(&this->actor, NA_SE_PL_FACE_UP);
                }

                return 1;
            }
        }
    }

    return 0;
}

void Player_RiseFromDive(GlobalContext* globalCtx, Player* this) {
    Player_PlayAnimLoop(globalCtx, this, &gPlayerAnim_0032F0);
    this->shapePitchOffset = 16000;
    this->genericTimer = 1;
}

void func_8083D36C(GlobalContext* globalCtx, Player* this) {
    if ((this->currentBoots != PLAYER_BOOTS_IRON) || !(this->actor.bgCheckFlags & 1)) {
        Player_ResetAttributesAndHeldActor(globalCtx, this);

        if ((this->currentBoots != PLAYER_BOOTS_IRON) && (this->stateFlags2 & PLAYER_STATE2_DIVING)) {
            this->stateFlags2 &= ~PLAYER_STATE2_DIVING;
            Player_SetupDive(globalCtx, this, 0);
            this->genericVar = 1;
        } else if (Player_FallingDive == this->actionFunc) {
            Player_SetActionFunc(globalCtx, this, Player_Dive, 0);
            Player_RiseFromDive(globalCtx, this);
        } else {
            Player_SetActionFunc(globalCtx, this, Player_UpdateSwimIdle, 1);
            Player_ChangeAnimMorphToLastFrame(
                globalCtx, this, (this->actor.bgCheckFlags & 1) ? &gPlayerAnim_003330 : &gPlayerAnim_0032E0);
        }
    }

    if (!(this->stateFlags1 & PLAYER_STATE1_SWIMMING) || (this->actor.yDistToWater < this->ageProperties->unk_2C)) {
        if (Player_SetupSpawnSplash(globalCtx, this, this->actor.velocity.y, 500)) {
            func_8002F7DC(&this->actor, NA_SE_EV_DIVE_INTO_WATER);

            if (this->fallDistance > 800.0f) {
                Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_CLIMB_END);
            }
        }
    }

    this->stateFlags1 |= PLAYER_STATE1_SWIMMING;
    this->stateFlags2 |= PLAYER_STATE2_DIVING;
    this->stateFlags1 &= ~(PLAYER_STATE1_JUMPING | PLAYER_STATE1_FREEFALLING);
    this->rippleTimer = 0.0f;

    Player_SetBootData(globalCtx, this);
}

void func_8083D53C(GlobalContext* globalCtx, Player* this) {
    if (this->actor.yDistToWater < this->ageProperties->unk_2C) {
        Audio_SetBaseFilter(0);
        this->unk_840 = 0;
    } else {
        Audio_SetBaseFilter(0x20);
        if (this->unk_840 < 300) {
            this->unk_840++;
        }
    }

    if ((Player_JumpUpToLedge != this->actionFunc) && (Player_ClimbOntoLedge != this->actionFunc)) {
        if (this->ageProperties->unk_2C < this->actor.yDistToWater) {
            if (!(this->stateFlags1 & PLAYER_STATE1_SWIMMING) ||
                (!((this->currentBoots == PLAYER_BOOTS_IRON) && (this->actor.bgCheckFlags & 1)) &&
                 (Player_DamagedSwim != this->actionFunc) && (Player_Drown != this->actionFunc) &&
                 (Player_UpdateSwimIdle != this->actionFunc) && (Player_Swim != this->actionFunc) &&
                 (Player_ZTargetSwimming != this->actionFunc) && (Player_Dive != this->actionFunc) &&
                 (Player_GetItemInWater != this->actionFunc) && (Player_SpawnSwimming != this->actionFunc))) {
                func_8083D36C(globalCtx, this);
                return;
            }
        } else if ((this->stateFlags1 & PLAYER_STATE1_SWIMMING) &&
                   (this->actor.yDistToWater < this->ageProperties->unk_24)) {
            if ((this->skelAnime.moveFlags == 0) && (this->currentBoots != PLAYER_BOOTS_IRON)) {
                Player_SetupTurn(globalCtx, this, this->actor.shape.rot.y);
            }
            Player_StartJumpOutOfWater(globalCtx, this, this->actor.velocity.y);
        }
    }
}

void func_8083D6EC(GlobalContext* globalCtx, Player* this) {
    Vec3f ripplePos;
    f32 temp1;
    f32 temp2;
    f32 temp3;
    f32 temp4;

    this->actor.minVelocityY = -20.0f;
    this->actor.gravity = REG(68) / 100.0f;

    if (Player_IsFloorSinkingSand(sFloorSpecialProperty)) {
        temp1 = fabsf(this->linearVelocity) * 20.0f;
        temp3 = 0.0f;

        if (sFloorSpecialProperty == 4) {
            if (this->shapeOffsetY > 1300.0f) {
                temp2 = this->shapeOffsetY;
            } else {
                temp2 = 1300.0f;
            }
            if (this->currentBoots == PLAYER_BOOTS_HOVER) {
                temp1 += temp1;
            } else if (this->currentBoots == PLAYER_BOOTS_IRON) {
                temp1 *= 0.3f;
            }
        } else {
            temp2 = 20000.0f;
            if (this->currentBoots != PLAYER_BOOTS_HOVER) {
                temp1 += temp1;
            } else if ((sFloorSpecialProperty == 7) || (this->currentBoots == PLAYER_BOOTS_IRON)) {
                temp1 = 0;
            }
        }

        if (this->currentBoots != PLAYER_BOOTS_HOVER) {
            temp3 = (temp2 - this->shapeOffsetY) * 0.02f;
            temp3 = CLAMP(temp3, 0.0f, 300.0f);
            if (this->currentBoots == PLAYER_BOOTS_IRON) {
                temp3 += temp3;
            }
        }

        this->shapeOffsetY += temp3 - temp1;
        this->shapeOffsetY = CLAMP(this->shapeOffsetY, 0.0f, temp2);

        this->actor.gravity -= this->shapeOffsetY * 0.004f;
    } else {
        this->shapeOffsetY = 0.0f;
    }

    if (this->actor.bgCheckFlags & 0x20) {
        if (this->actor.yDistToWater < 50.0f) {
            temp4 = fabsf(this->bodyPartsPos[PLAYER_BODYPART_WAIST].x - this->prevWaistPos.x) +
                    fabsf(this->bodyPartsPos[PLAYER_BODYPART_WAIST].y - this->prevWaistPos.y) +
                    fabsf(this->bodyPartsPos[PLAYER_BODYPART_WAIST].z - this->prevWaistPos.z);
            if (temp4 > 4.0f) {
                temp4 = 4.0f;
            }
            this->rippleTimer += temp4;

            if (this->rippleTimer > 15.0f) {
                this->rippleTimer = 0.0f;

                ripplePos.x = (Rand_ZeroOne() * 10.0f) + this->actor.world.pos.x;
                ripplePos.y = this->actor.world.pos.y + this->actor.yDistToWater;
                ripplePos.z = (Rand_ZeroOne() * 10.0f) + this->actor.world.pos.z;
                EffectSsGRipple_Spawn(globalCtx, &ripplePos, 100, 500, 0);

                if ((this->linearVelocity > 4.0f) && !Player_IsSwimming(this) &&
                    ((this->actor.world.pos.y + this->actor.yDistToWater) <
                     this->bodyPartsPos[PLAYER_BODYPART_WAIST].y)) {
                    Player_SetupSpawnSplash(globalCtx, this, 20.0f,
                                  (fabsf(this->linearVelocity) * 50.0f) + (this->actor.yDistToWater * 5.0f));
                }
            }
        }

        if (this->actor.yDistToWater > 40.0f) {
            s32 numBubbles = 0;
            s32 i;

            if ((this->actor.velocity.y > -1.0f) || (this->actor.bgCheckFlags & 1)) {
                if (Rand_ZeroOne() < 0.2f) {
                    numBubbles = 1;
                }
            } else {
                numBubbles = this->actor.velocity.y * -2.0f;
            }

            for (i = 0; i < numBubbles; i++) {
                EffectSsBubble_Spawn(globalCtx, &this->actor.world.pos, 20.0f, 10.0f, 20.0f, 0.13f);
            }
        }
    }
}

s32 Player_LookAtTargetActor(Player* this, s32 flag) {
    Actor* targetActor = this->targetActor;
    Vec3f sp30;
    s16 sp2E;
    s16 sp2C;

    sp30.x = this->actor.world.pos.x;
    sp30.y = this->bodyPartsPos[PLAYER_BODYPART_HEAD].y + 3.0f;
    sp30.z = this->actor.world.pos.z;
    sp2E = Math_Vec3f_Pitch(&sp30, &targetActor->focus.pos);
    sp2C = Math_Vec3f_Yaw(&sp30, &targetActor->focus.pos);
    Math_SmoothStepToS(&this->actor.focus.rot.y, sp2C, 4, 10000, 0);
    Math_SmoothStepToS(&this->actor.focus.rot.x, sp2E, 4, 10000, 0);
    this->lookFlags |= 2;

    return Player_UpdateLookAngles(this, flag);
}

static Vec3f D_8085456C = { 0.0f, 100.0f, 40.0f };

void func_8083DC54(Player* this, GlobalContext* globalCtx) {
    s16 sp46;
    s16 temp2;
    f32 temp1;
    Vec3f sp34;

    if (this->targetActor != NULL) {
        if (Actor_PlayerIsAimingReadyFpsItem(this) || Player_IsAimingReadyBoomerang(this)) {
            Player_LookAtTargetActor(this, 1);
        } else {
            Player_LookAtTargetActor(this, 0);
        }
        return;
    }

    if (sFloorSpecialProperty == 11) {
        Math_SmoothStepToS(&this->actor.focus.rot.x, -20000, 10, 4000, 800);
    } else {
        sp46 = 0;
        temp1 = Player_RaycastFloorWithOffset2(globalCtx, this, &D_8085456C, &sp34);
        if (temp1 > BGCHECK_Y_MIN) {
            temp2 = Math_Atan2S(40.0f, this->actor.world.pos.y - temp1);
            sp46 = CLAMP(temp2, -4000, 4000);
        }
        this->actor.focus.rot.y = this->actor.shape.rot.y;
        Math_SmoothStepToS(&this->actor.focus.rot.x, sp46, 14, 4000, 30);
    }

    Player_UpdateLookAngles(this, Actor_PlayerIsAimingReadyFpsItem(this) || Player_IsAimingReadyBoomerang(this));
}

void func_8083DDC8(Player* this, GlobalContext* globalCtx) {
    s16 temp1;
    s16 temp2;

    if (!Actor_PlayerIsAimingReadyFpsItem(this) && !Player_IsAimingReadyBoomerang(this) && (this->linearVelocity > 5.0f)) {
        temp1 = this->linearVelocity * 200.0f;
        temp2 = (s16)(this->currentYaw - this->actor.shape.rot.y) * this->linearVelocity * 0.1f;
        temp1 = CLAMP(temp1, -4000, 4000);
        temp2 = CLAMP(-temp2, -4000, 4000);
        Math_ScaledStepToS(&this->upperBodyRot.x, temp1, 900);
        this->headRot.x = -(f32)this->upperBodyRot.x * 0.5f;
        Math_ScaledStepToS(&this->headRot.z, temp2, 300);
        Math_ScaledStepToS(&this->upperBodyRot.z, temp2, 200);
        this->lookFlags |= 0x168;
    } else {
        func_8083DC54(this, globalCtx);
    }
}

void Player_SetRunVelocityAndYaw(Player* this, f32 arg1, s16 arg2) {
    Math_AsymStepToF(&this->linearVelocity, arg1, REG(19) / 100.0f, 1.5f);
    Math_ScaledStepToS(&this->currentYaw, arg2, REG(27));
}

void func_8083DFE0(Player* this, f32* arg1, s16* arg2) {
    s16 yawDiff = this->currentYaw - *arg2;

    if (this->swordState == 0) {
        float maxSpeed = R_RUN_SPEED_LIMIT / 100.0f;
        if (CVar_GetS32("gMMBunnyHood", 0) != 0 && this->currentMask == PLAYER_MASK_BUNNY) {
            maxSpeed *= 1.5f;
        }
        this->linearVelocity = CLAMP(this->linearVelocity, -maxSpeed, maxSpeed);
    }

    if (ABS(yawDiff) > 0x6000) {
        if (Math_StepToF(&this->linearVelocity, 0.0f, 1.0f)) {
            this->currentYaw = *arg2;
        }
    } else {
        Math_AsymStepToF(&this->linearVelocity, *arg1, 0.05f, 0.1f);
        Math_ScaledStepToS(&this->currentYaw, *arg2, 200);
    }
}

static struct_80854578 D_80854578[] = {
    { &gPlayerAnim_003398, 35.17f, 6.6099997f },
    { &gPlayerAnim_0033A8, -34.16f, 7.91f },
};

s32 Player_SetupMountHorse(Player* this, GlobalContext* globalCtx) {
    EnHorse* rideActor = (EnHorse*)this->rideActor;
    f32 unk_04;
    f32 unk_08;
    f32 sp38;
    f32 sp34;
    s32 temp;

    if ((rideActor != NULL) && CHECK_BTN_ALL(sControlInput->press.button, BTN_A)) {
        sp38 = Math_CosS(rideActor->actor.shape.rot.y);
        sp34 = Math_SinS(rideActor->actor.shape.rot.y);

        Player_SetupMiniCsFunc(globalCtx, this, Player_SetupRideHorse);

        this->stateFlags1 |= PLAYER_STATE1_RIDING_HORSE;
        this->actor.bgCheckFlags &= ~0x20;

        if (this->mountSide < 0) {
            temp = 0;
        } else {
            temp = 1;
        }

        unk_04 = D_80854578[temp].unk_04;
        unk_08 = D_80854578[temp].unk_08;
        this->actor.world.pos.x =
            rideActor->actor.world.pos.x + rideActor->riderPos.x + ((unk_04 * sp38) + (unk_08 * sp34));
        this->actor.world.pos.z =
            rideActor->actor.world.pos.z + rideActor->riderPos.z + ((unk_08 * sp38) - (unk_04 * sp34));

        this->rideOffsetY = rideActor->actor.world.pos.y - this->actor.world.pos.y;
        this->currentYaw = this->actor.shape.rot.y = rideActor->actor.shape.rot.y;

        Actor_MountHorse(globalCtx, this, &rideActor->actor);
        Player_PlayAnimOnce(globalCtx, this, D_80854578[temp].anim);
        Player_SetupAnimMovement(globalCtx, this, 0x9B);
        this->actor.parent = this->rideActor;
        Player_ClearAttentionModeAndStopMoving(this);
        func_800304B0(globalCtx);
        return 1;
    }

    return 0;
}

void func_8083E298(CollisionPoly* arg0, Vec3f* arg1, s16* arg2) {
    arg1->x = COLPOLY_GET_NORMAL(arg0->normal.x);
    arg1->y = COLPOLY_GET_NORMAL(arg0->normal.y);
    arg1->z = COLPOLY_GET_NORMAL(arg0->normal.z);

    *arg2 = Math_Atan2S(arg1->z, arg1->x);
}

static LinkAnimationHeader* D_80854590[] = {
    &gPlayerAnim_002EE0,
    &gPlayerAnim_0031D0,
};

s32 func_8083E318(GlobalContext* globalCtx, Player* this, CollisionPoly* arg2) {
    s32 pad;
    s16 sp4A;
    Vec3f sp3C;
    s16 sp3A;
    f32 temp1;
    f32 temp2;
    s16 temp3;

    if (!Player_InBlockingCsMode(globalCtx, this) && (Player_SlipOnSlope != this->actionFunc) &&
        (SurfaceType_GetSlope(&globalCtx->colCtx, arg2, this->actor.floorBgId) == 1)) {
        sp4A = Math_Atan2S(this->actor.velocity.z, this->actor.velocity.x);
        func_8083E298(arg2, &sp3C, &sp3A);
        temp3 = sp3A - sp4A;

        if (ABS(temp3) > 16000) {
            temp1 = (1.0f - sp3C.y) * 40.0f;
            temp2 = (temp1 * temp1) * 0.015f;
            if (temp2 < 1.2f) {
                temp2 = 1.2f;
            }
            this->pushedYaw = sp3A;
            Math_StepToF(&this->pushedSpeed, temp1, temp2);
        } else {
            Player_SetActionFunc(globalCtx, this, Player_SlipOnSlope, 0);
            Player_ResetAttributesAndHeldActor(globalCtx, this);
            if (sAngleToFloorX >= 0) {
                this->genericVar = 1;
            }
            Player_ChangeAnimShortMorphLoop(globalCtx, this, D_80854590[this->genericVar]);
            this->linearVelocity = sqrtf(SQ(this->actor.velocity.x) + SQ(this->actor.velocity.z));
            this->currentYaw = sp4A;
            return 1;
        }
    }

    return 0;
}

// unknown data (unused)
static s32 D_80854598[] = {
    0xFFDB0871, 0xF8310000, 0x00940470, 0xF3980000, 0xFFB504A9, 0x0C9F0000, 0x08010402,
};

void func_8083E4C4(GlobalContext* globalCtx, Player* this, GetItemEntry* giEntry) {
    s32 sp1C = giEntry->field & 0x1F;

    if (!(giEntry->field & 0x80)) {
        Item_DropCollectible(globalCtx, &this->actor.world.pos, sp1C | 0x8000);
        if ((sp1C != 4) && (sp1C != 8) && (sp1C != 9) && (sp1C != 0xA) && (sp1C != 0) && (sp1C != 1) && (sp1C != 2) &&
            (sp1C != 0x14) && (sp1C != 0x13)) {
            Item_Give(globalCtx, giEntry->itemId);
        }
    } else {
        Item_Give(globalCtx, giEntry->itemId);
    }

    func_80078884((this->getItemId < 0) ? NA_SE_SY_GET_BOXITEM : NA_SE_SY_GET_ITEM);
}

s32 Player_SetupGetItemOrHoldBehavior(Player* this, GlobalContext* globalCtx) {
    Actor* interactedActor;

    if (iREG(67) || (((interactedActor = this->interactRangeActor) != NULL) &&
                     func_8002D53C(globalCtx, &globalCtx->actorCtx.titleCtx))) {
        if (iREG(67) || (this->getItemId > GI_NONE)) {
            if (iREG(67)) {
                this->getItemId = iREG(68);
            }

            if (this->getItemId < GI_MAX) {
                GetItemEntry* giEntry = &sGetItemTable[this->getItemId - 1];

                if ((interactedActor != &this->actor) && !iREG(67)) {
                    interactedActor->parent = &this->actor;
                }

                iREG(67) = false;

                if (gSaveContext.n64ddFlag && this->getItemId == GI_ICE_TRAP) {
                    this->stateFlags1 &= ~(PLAYER_STATE1_GETTING_ITEM | PLAYER_STATE1_HOLDING_ACTOR);
                    this->actor.colChkInfo.damage = 0;
                    Player_SetupDamage(globalCtx, this, 3, 0.0f, 0.0f, 0, 20);
                    return;
                }

                s32 drop = giEntry->objectId;

                if ((globalCtx->sceneNum == SCENE_BOWLING) || !(CVar_GetS32("gFastDrops", 0) &&
                    ((drop == OBJECT_GI_BOMB_1) || (drop == OBJECT_GI_NUTS) || (drop == OBJECT_GI_STICK) ||
                    (drop == OBJECT_GI_SEED) || (drop == OBJECT_GI_MAGICPOT) || (drop == OBJECT_GI_ARROW))) &&
                    (Item_CheckObtainability(giEntry->itemId) == ITEM_NONE)) {
                    Player_DetatchHeldActor(globalCtx, this);
                    Player_LoadGetItemObject(this, giEntry->objectId);

                    if (!(this->stateFlags2 & PLAYER_STATE2_DIVING) || (this->currentBoots == PLAYER_BOOTS_IRON)) {
                        Player_SetupMiniCsFunc(globalCtx, this, Player_SetupGetItem);
                        Player_PlayAnimOnceSlowed(globalCtx, this, &gPlayerAnim_002788);
                        Player_SetCameraTurnAround(globalCtx, 9);
                    }

                    this->stateFlags1 |=
                        PLAYER_STATE1_GETTING_ITEM | PLAYER_STATE1_HOLDING_ACTOR | PLAYER_STATE1_IN_CUTSCENE;
                    Player_ClearAttentionModeAndStopMoving(this);
                    return 1;
                }

                func_8083E4C4(globalCtx, this, giEntry);
                this->getItemId = GI_NONE;
            }
        } else if (CHECK_BTN_ALL(sControlInput->press.button, BTN_A) &&
                   !(this->stateFlags1 & PLAYER_STATE1_HOLDING_ACTOR) && !(this->stateFlags2 & PLAYER_STATE2_DIVING)) {
            if (this->getItemId != GI_NONE) {
                GetItemEntry* giEntry = &sGetItemTable[-this->getItemId - 1];
                EnBox* chest = (EnBox*)interactedActor;
                if (CVar_GetS32("gFastChests", 0) != 0) {
                    giEntry->gi = -1 * abs(giEntry->gi);
                }

                if (giEntry->itemId != ITEM_NONE) {
                    if (((Item_CheckObtainability(giEntry->itemId) == ITEM_NONE) && (giEntry->field & 0x40)) ||
                        ((Item_CheckObtainability(giEntry->itemId) != ITEM_NONE) && (giEntry->field & 0x20))) {
                        this->getItemId = -GI_RUPEE_BLUE;
                        giEntry = &sGetItemTable[GI_RUPEE_BLUE - 1];
                    }
                }

                Player_SetupMiniCsFunc(globalCtx, this, Player_SetupGetItem);
                this->stateFlags1 |=
                    PLAYER_STATE1_GETTING_ITEM | PLAYER_STATE1_HOLDING_ACTOR | PLAYER_STATE1_IN_CUTSCENE;
                Player_LoadGetItemObject(this, giEntry->objectId);
                this->actor.world.pos.x =
                    chest->dyna.actor.world.pos.x - (Math_SinS(chest->dyna.actor.shape.rot.y) * 29.4343f);
                this->actor.world.pos.z =
                    chest->dyna.actor.world.pos.z - (Math_CosS(chest->dyna.actor.shape.rot.y) * 29.4343f);
                this->currentYaw = this->actor.shape.rot.y = chest->dyna.actor.shape.rot.y;
                Player_ClearAttentionModeAndStopMoving(this);

                if ((giEntry->itemId != ITEM_NONE) && (giEntry->gi >= 0) &&
                    (Item_CheckObtainability(giEntry->itemId) == ITEM_NONE)) {
                    Player_PlayAnimOnceSlowed(globalCtx, this, this->ageProperties->unk_98);
                    Player_SetupAnimMovement(globalCtx, this, 0x28F);
                    chest->unk_1F4 = 1;
                    Camera_ChangeSetting(Gameplay_GetCamera(globalCtx, 0), CAM_SET_SLOW_CHEST_CS);
                }
                else {
                    Player_PlayAnimOnce(globalCtx, this, &gPlayerAnim_002DF8);
                    chest->unk_1F4 = -1;
                }

                return 1;
            }

            if ((this->heldActor == NULL) || Player_HoldsHookshot(this)) {
                if ((interactedActor->id == ACTOR_BG_TOKI_SWD) && LINK_IS_ADULT) {
                    s32 sp24 = this->itemActionParam;

                    this->itemActionParam = PLAYER_AP_NONE;
                    this->modelAnimType = PLAYER_ANIMTYPE_DEFAULT;
                    this->heldItemActionParam = this->itemActionParam;
                    Player_SetupMiniCsFunc(globalCtx, this, Player_SetupHoldActor);

                    if (sp24 == PLAYER_AP_SWORD_MASTER) {
                        this->nextModelGroup = Player_ActionToModelGroup(this, PLAYER_AP_LAST_USED);
                        Player_ChangeItem(globalCtx, this, PLAYER_AP_LAST_USED);
                    }
                    else {
                        Player_UseItem(globalCtx, this, ITEM_LAST_USED);
                    }
                }
                else {
                    s32 strength = Player_GetStrength();

                    if ((interactedActor->id == ACTOR_EN_ISHI) && ((interactedActor->params & 0xF) == 1) &&
                        (strength < PLAYER_STR_SILVER_G)) {
                        return 0;
                    }

                    Player_SetupMiniCsFunc(globalCtx, this, Player_SetupHoldActor);
                }

                Player_ClearAttentionModeAndStopMoving(this);
                this->stateFlags1 |= PLAYER_STATE1_HOLDING_ACTOR;
                return 1;
            }
        }
    }

    return 0;
}

void func_8083EA94(Player* this, GlobalContext* globalCtx) {
    Player_SetActionFunc(globalCtx, this, Player_StartThrowActor, 1);
    Player_PlayAnimOnce(globalCtx, this, GET_PLAYER_ANIM(PLAYER_ANIMGROUP_THROWING_OBJECT, this->modelAnimType));
}

s32 func_8083EAF0(Player* this, Actor* actor) {
    if ((actor != NULL) && !(actor->flags & ACTOR_FLAG_23) &&
        ((this->linearVelocity < 1.1f) || (actor->id == ACTOR_EN_BOM_CHU))) {
        return 0;
    }

    return 1;
}

s32 Player_SetupPutDownOrThrowActor(Player* this, GlobalContext* globalCtx) {
    u16 buttonsToCheck = BTN_A | BTN_B | BTN_CLEFT | BTN_CRIGHT | BTN_CDOWN;
    if (CVar_GetS32("gDpadEquips", 0) != 0) {
        buttonsToCheck |= BTN_DUP | BTN_DDOWN | BTN_DLEFT | BTN_DRIGHT;
    }
    if ((this->stateFlags1 & PLAYER_STATE1_HOLDING_ACTOR) && (this->heldActor != NULL) &&
        CHECK_BTN_ANY(sControlInput->press.button, buttonsToCheck)) {
        if (!Player_InterruptHoldingActor(globalCtx, this, this->heldActor)) {
            if (!func_8083EAF0(this, this->heldActor)) {
                Player_SetActionFunc(globalCtx, this, Player_SetupPutDownActor, 1);
                Player_PlayAnimOnce(globalCtx, this,
                                    GET_PLAYER_ANIM(PLAYER_ANIMGROUP_PUTTING_DOWN_OBJECT, this->modelAnimType));
            } else {
                func_8083EA94(this, globalCtx);
            }
        }
        return 1;
    }

    return 0;
}

s32 func_8083EC18(Player* this, GlobalContext* globalCtx, u32 arg2) {
    if (this->wallHeight >= 79.0f) {
        if (!(this->stateFlags1 & PLAYER_STATE1_SWIMMING) || (this->currentBoots == PLAYER_BOOTS_IRON) ||
            (this->actor.yDistToWater < this->ageProperties->unk_2C)) {
            s32 sp8C = (arg2 & 8) ? 2 : 0;

            if ((sp8C != 0) || (arg2 & 2) ||
                func_80041E4C(&globalCtx->colCtx, this->actor.wallPoly, this->actor.wallBgId)) {
                f32 phi_f20;
                CollisionPoly* sp84 = this->actor.wallPoly;
                f32 sp80;
                f32 sp7C;
                f32 phi_f12;
                f32 phi_f14;

                phi_f20 = phi_f12 = 0.0f;

                if (sp8C != 0) {
                    sp80 = this->actor.world.pos.x;
                    sp7C = this->actor.world.pos.z;
                } else {
                    Vec3f sp50[3];
                    s32 i;
                    f32 sp48;
                    Vec3f* sp44 = &sp50[0];
                    s32 pad;

                    CollisionPoly_GetVerticesByBgId(sp84, this->actor.wallBgId, &globalCtx->colCtx, sp50);

                    sp80 = phi_f12 = sp44->x;
                    sp7C = phi_f14 = sp44->z;
                    phi_f20 = sp44->y;
                    for (i = 1; i < 3; i++) {
                        sp44++;
                        if (sp80 > sp44->x) {
                            sp80 = sp44->x;
                        } else if (phi_f12 < sp44->x) {
                            phi_f12 = sp44->x;
                        }

                        if (sp7C > sp44->z) {
                            sp7C = sp44->z;
                        } else if (phi_f14 < sp44->z) {
                            phi_f14 = sp44->z;
                        }

                        if (phi_f20 > sp44->y) {
                            phi_f20 = sp44->y;
                        }
                    }

                    sp80 = (sp80 + phi_f12) * 0.5f;
                    sp7C = (sp7C + phi_f14) * 0.5f;

                    phi_f12 = ((this->actor.world.pos.x - sp80) * COLPOLY_GET_NORMAL(sp84->normal.z)) -
                              ((this->actor.world.pos.z - sp7C) * COLPOLY_GET_NORMAL(sp84->normal.x));
                    sp48 = this->actor.world.pos.y - phi_f20;

                    phi_f20 = ((f32)(s32)((sp48 / 15.000000223517418) + 0.5) * 15.000000223517418) - sp48;
                    phi_f12 = fabsf(phi_f12);
                }

                if (phi_f12 < 8.0f) {
                    f32 sp3C = COLPOLY_GET_NORMAL(sp84->normal.x);
                    f32 sp38 = COLPOLY_GET_NORMAL(sp84->normal.z);
                    f32 sp34 = this->wallDistance;
                    LinkAnimationHeader* sp30;

                    Player_SetupMiniCsFunc(globalCtx, this, Player_SetupClimbingWallOrDownLedge);
                    this->stateFlags1 |= PLAYER_STATE1_CLIMBING;
                    this->stateFlags1 &= ~PLAYER_STATE1_SWIMMING;

                    if ((sp8C != 0) || (arg2 & 2)) {
                        if ((this->genericVar = sp8C) != 0) {
                            if (this->actor.bgCheckFlags & 1) {
                                sp30 = &gPlayerAnim_002D80;
                            } else {
                                sp30 = &gPlayerAnim_002D68;
                            }
                            sp34 = (this->ageProperties->unk_38 - 1.0f) - sp34;
                        } else {
                            sp30 = this->ageProperties->unk_A4;
                            sp34 = sp34 - 1.0f;
                        }
                        this->genericTimer = -2;
                        this->actor.world.pos.y += phi_f20;
                        this->actor.shape.rot.y = this->currentYaw = this->actor.wallYaw + 0x8000;
                    } else {
                        sp30 = this->ageProperties->unk_A8;
                        this->genericTimer = -4;
                        this->actor.shape.rot.y = this->currentYaw = this->actor.wallYaw;
                    }

                    this->actor.world.pos.x = (sp34 * sp3C) + sp80;
                    this->actor.world.pos.z = (sp34 * sp38) + sp7C;
                    Player_ClearAttentionModeAndStopMoving(this);
                    Math_Vec3f_Copy(&this->actor.prevPos, &this->actor.world.pos);
                    Player_PlayAnimOnce(globalCtx, this, sp30);
                    Player_SetupAnimMovement(globalCtx, this, 0x9F);

                    return 1;
                }
            }
        }
    }

    return 0;
}

void func_8083F070(Player* this, LinkAnimationHeader* anim, GlobalContext* globalCtx) {
    Player_SetActionFuncPreserveMoveFlags(globalCtx, this, Player_EndClimb, 0);
    LinkAnimation_PlayOnceSetSpeed(globalCtx, &this->skelAnime, anim, (4.0f / 3.0f));
}

s32 func_8083F0C8(Player* this, GlobalContext* globalCtx, u32 arg2) {
    CollisionPoly* wallPoly;
    Vec3f wallVertices[3];
    f32 tempX;
    f32 temp;
    f32 tempZ;
    f32 maxWallZ;
    s32 i;

    if (!LINK_IS_ADULT && !(this->stateFlags1 & PLAYER_STATE1_SWIMMING) && (arg2 & 0x30)) {
        wallPoly = this->actor.wallPoly;
        CollisionPoly_GetVerticesByBgId(wallPoly, this->actor.wallBgId, &globalCtx->colCtx, wallVertices);

        // compute min and max x/z of wall vertices
        tempX = temp = wallVertices[0].x;
        tempZ = maxWallZ = wallVertices[0].z;
        for (i = 1; i < 3; i++) {
            if (tempX > wallVertices[i].x) {
                tempX = wallVertices[i].x;
            } else if (temp < wallVertices[i].x) {
                temp = wallVertices[i].x;
            }

            if (tempZ > wallVertices[i].z) {
                tempZ = wallVertices[i].z;
            } else if (maxWallZ < wallVertices[i].z) {
                maxWallZ = wallVertices[i].z;
            }
        }

        // average min and max x/z of wall vertices
        tempX = (tempX + temp) * 0.5f;
        tempZ = (tempZ + maxWallZ) * 0.5f;

        temp = ((this->actor.world.pos.x - tempX) * COLPOLY_GET_NORMAL(wallPoly->normal.z)) -
               ((this->actor.world.pos.z - tempZ) * COLPOLY_GET_NORMAL(wallPoly->normal.x));

        if (fabsf(temp) < 8.0f) {
            this->stateFlags2 |= PLAYER_STATE2_CAN_ENTER_CRAWLSPACE;

            if (CHECK_BTN_ALL(sControlInput->press.button, BTN_A)) {
                f32 wallPolyNormX = COLPOLY_GET_NORMAL(wallPoly->normal.x);
                f32 wallPolyNormZ = COLPOLY_GET_NORMAL(wallPoly->normal.z);
                f32 wallDistance = this->wallDistance;

                Player_SetupMiniCsFunc(globalCtx, this, Player_SetupInsideCrawlspace);
                this->stateFlags2 |= PLAYER_STATE2_INSIDE_CRAWLSPACE;
                this->actor.shape.rot.y = this->currentYaw = this->actor.wallYaw + 0x8000;
                this->actor.world.pos.x = tempX + (wallDistance * wallPolyNormX);
                this->actor.world.pos.z = tempZ + (wallDistance * wallPolyNormZ);
                Player_ClearAttentionModeAndStopMoving(this);
                this->actor.prevPos = this->actor.world.pos;
                Player_PlayAnimOnce(globalCtx, this, &gPlayerAnim_002708);
                Player_SetupAnimMovement(globalCtx, this, 0x9D);

                return 1;
            }
        }
    }

    return 0;
}

s32 func_8083F360(GlobalContext* globalCtx, Player* this, f32 arg1, f32 arg2, f32 arg3, f32 arg4) {
    CollisionPoly* wallPoly;
    s32 sp78;
    Vec3f sp6C;
    Vec3f sp60;
    Vec3f sp54;
    f32 yawCos;
    f32 yawSin;
    s32 temp;
    f32 temp1;
    f32 temp2;

    yawCos = Math_CosS(this->actor.shape.rot.y);
    yawSin = Math_SinS(this->actor.shape.rot.y);

    sp6C.x = this->actor.world.pos.x + (arg4 * yawSin);
    sp6C.z = this->actor.world.pos.z + (arg4 * yawCos);
    sp60.x = this->actor.world.pos.x + (arg3 * yawSin);
    sp60.z = this->actor.world.pos.z + (arg3 * yawCos);
    sp60.y = sp6C.y = this->actor.world.pos.y + arg1;

    if (BgCheck_EntityLineTest1(&globalCtx->colCtx, &sp6C, &sp60, &sp54, &this->actor.wallPoly, true, false, false,
                                true, &sp78)) {
        wallPoly = this->actor.wallPoly;

        this->actor.bgCheckFlags |= 0x200;
        this->actor.wallBgId = sp78;

        sTouchedWallFlags = func_80041DB8(&globalCtx->colCtx, wallPoly, sp78);

        temp1 = COLPOLY_GET_NORMAL(wallPoly->normal.x);
        temp2 = COLPOLY_GET_NORMAL(wallPoly->normal.z);
        temp = Math_Atan2S(-temp2, -temp1);
        Math_ScaledStepToS(&this->actor.shape.rot.y, temp, 800);

        this->currentYaw = this->actor.shape.rot.y;
        this->actor.world.pos.x = sp54.x - (Math_SinS(this->actor.shape.rot.y) * arg2);
        this->actor.world.pos.z = sp54.z - (Math_CosS(this->actor.shape.rot.y) * arg2);

        return 1;
    }

    this->actor.bgCheckFlags &= ~0x200;

    return 0;
}

s32 func_8083F524(GlobalContext* globalCtx, Player* this) {
    return func_8083F360(globalCtx, this, 26.0f, this->ageProperties->unk_38 + 5.0f, 30.0f, 0.0f);
}

s32 func_8083F570(Player* this, GlobalContext* globalCtx) {
    s16 temp;

    if ((this->linearVelocity != 0.0f) && (this->actor.bgCheckFlags & 8) && (sTouchedWallFlags & 0x30)) {

        temp = this->actor.shape.rot.y - this->actor.wallYaw;
        if (this->linearVelocity < 0.0f) {
            temp += 0x8000;
        }

        if (ABS(temp) > 0x4000) {
            Player_SetActionFunc(globalCtx, this, Player_ExitCrawlspace, 0);

            if (this->linearVelocity > 0.0f) {
                this->actor.shape.rot.y = this->actor.wallYaw + 0x8000;
                Player_PlayAnimOnce(globalCtx, this, &gPlayerAnim_002700);
                Player_SetupAnimMovement(globalCtx, this, 0x9D);
                OnePointCutscene_Init(globalCtx, 9601, 999, NULL, MAIN_CAM);
            } else {
                this->actor.shape.rot.y = this->actor.wallYaw;
                LinkAnimation_Change(globalCtx, &this->skelAnime, &gPlayerAnim_002708, -1.0f,
                                     Animation_GetLastFrame(&gPlayerAnim_002708), 0.0f, ANIMMODE_ONCE, 0.0f);
                Player_SetupAnimMovement(globalCtx, this, 0x9D);
                OnePointCutscene_Init(globalCtx, 9602, 999, NULL, MAIN_CAM);
            }

            this->currentYaw = this->actor.shape.rot.y;
            Player_StopMovement(this);

            return 1;
        }
    }

    return 0;
}

void func_8083F72C(Player* this, LinkAnimationHeader* anim, GlobalContext* globalCtx) {
    if (!Player_SetupMiniCsFunc(globalCtx, this, Player_SetupGrabPushPullWall)) {
        Player_SetActionFunc(globalCtx, this, Player_GrabPushPullWall, 0);
    }

    Player_PlayAnimOnce(globalCtx, this, anim);
    Player_ClearAttentionModeAndStopMoving(this);

    this->actor.shape.rot.y = this->currentYaw = this->actor.wallYaw + 0x8000;
}

s32 Player_SetupSpecialWallInteraction(Player* this, GlobalContext* globalCtx) {
    DynaPolyActor* wallPolyActor;

    if (!(this->stateFlags1 & PLAYER_STATE1_HOLDING_ACTOR) && (this->actor.bgCheckFlags & 0x200) &&
        (sYawToTouchedWall < 0x3000)) {

        if (((this->linearVelocity > 0.0f) && func_8083EC18(this, globalCtx, sTouchedWallFlags)) ||
            func_8083F0C8(this, globalCtx, sTouchedWallFlags)) {
            return 1;
        }

        if (!Player_IsSwimming(this) &&
            ((this->linearVelocity == 0.0f) || !(this->stateFlags2 & PLAYER_STATE2_CAN_CLIMB_PUSH_PULL_WALL)) &&
            (sTouchedWallFlags & 0x40) && (this->actor.bgCheckFlags & 1) && (this->wallHeight >= 39.0f)) {

            this->stateFlags2 |= PLAYER_STATE2_CAN_GRAB_PUSH_PULL_WALL;

            if (CHECK_BTN_ALL(sControlInput->cur.button, BTN_A)) {

                if ((this->actor.wallBgId != BGCHECK_SCENE) &&
                    ((wallPolyActor = DynaPoly_GetActor(&globalCtx->colCtx, this->actor.wallBgId)) != NULL)) {

                    if (wallPolyActor->actor.id == ACTOR_BG_HEAVY_BLOCK) {
                        if (Player_GetStrength() < PLAYER_STR_GOLD_G) {
                            return 0;
                        }

                        Player_SetupMiniCsFunc(globalCtx, this, Player_SetupHoldActor);
                        this->stateFlags1 |= PLAYER_STATE1_HOLDING_ACTOR;
                        this->interactRangeActor = &wallPolyActor->actor;
                        this->getItemId = GI_NONE;
                        this->currentYaw = this->actor.wallYaw + 0x8000;
                        Player_ClearAttentionModeAndStopMoving(this);

                        return 1;
                    }

                    this->pushPullActor = &wallPolyActor->actor;
                } else {
                    this->pushPullActor = NULL;
                }

                func_8083F72C(this, &gPlayerAnim_0030F8, globalCtx);

                return 1;
            }
        }
    }

    return 0;
}

s32 func_8083F9D0(GlobalContext* globalCtx, Player* this) {
    if ((this->actor.bgCheckFlags & 0x200) && ((this->stateFlags2 & PLAYER_STATE2_MOVING_PUSH_PULL_WALL) ||
                                               CHECK_BTN_ALL(sControlInput->cur.button, BTN_A))) {
        DynaPolyActor* wallPolyActor = NULL;

        if (this->actor.wallBgId != BGCHECK_SCENE) {
            wallPolyActor = DynaPoly_GetActor(&globalCtx->colCtx, this->actor.wallBgId);
        }

        if (&wallPolyActor->actor == this->pushPullActor) {
            if (this->stateFlags2 & PLAYER_STATE2_MOVING_PUSH_PULL_WALL) {
                return 1;
            } else {
                return 0;
            }
        }
    }

    Player_ReturnToStandStill(this, globalCtx);
    Player_PlayAnimOnce(globalCtx, this, &gPlayerAnim_003100);
    this->stateFlags2 &= ~PLAYER_STATE2_MOVING_PUSH_PULL_WALL;
    return 1;
}

void func_8083FAB8(Player* this, GlobalContext* globalCtx) {
    Player_SetActionFunc(globalCtx, this, Player_PushWall, 0);
    this->stateFlags2 |= PLAYER_STATE2_MOVING_PUSH_PULL_WALL;
    Player_PlayAnimOnce(globalCtx, this, &gPlayerAnim_0030F0);
}

void func_8083FB14(Player* this, GlobalContext* globalCtx) {
    Player_SetActionFunc(globalCtx, this, Player_PullWall, 0);
    this->stateFlags2 |= PLAYER_STATE2_MOVING_PUSH_PULL_WALL;
    Player_PlayAnimOnce(globalCtx, this, GET_PLAYER_ANIM(PLAYER_ANIMGROUP_END_PULL_OBJECT, this->modelAnimType));
}

void func_8083FB7C(Player* this, GlobalContext* globalCtx) {
    this->stateFlags1 &= ~(PLAYER_STATE1_CLIMBING | PLAYER_STATE1_SWIMMING);
    Player_SetupFallFromLedge(this, globalCtx);
    this->linearVelocity = -0.4f;
}

s32 func_8083FBC0(Player* this, GlobalContext* globalCtx) {
    if (!CHECK_BTN_ALL(sControlInput->press.button, BTN_A) && (this->actor.bgCheckFlags & 0x200) &&
        ((sTouchedWallFlags & 8) || (sTouchedWallFlags & 2) ||
         func_80041E4C(&globalCtx->colCtx, this->actor.wallPoly, this->actor.wallBgId))) {
        return 0;
    }

    func_8083FB7C(this, globalCtx);
    Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_AUTO_JUMP);
    return 1;
}

s32 func_8083FC68(Player* this, f32 arg1, s16 arg2) {
    f32 sp1C = (s16)(arg2 - this->actor.shape.rot.y);
    f32 temp;

    if (this->targetActor != NULL) {
        Player_LookAtTargetActor(this, Actor_PlayerIsAimingReadyFpsItem(this) || Player_IsAimingReadyBoomerang(this));
    }

    temp = fabsf(sp1C) / 32768.0f;

    if (arg1 > (((temp * temp) * 50.0f) + 6.0f)) {
        return 1;
    } else if (arg1 > (((1.0f - temp) * 10.0f) + 6.8f)) {
        return -1;
    }

    return 0;
}

s32 func_8083FD78(Player* this, f32* arg1, s16* arg2, GlobalContext* globalCtx) {
    s16 sp2E = *arg2 - this->targetYaw;
    u16 sp2C = ABS(sp2E);

    if ((Actor_PlayerIsAimingReadyFpsItem(this) || Player_IsAimingReadyBoomerang(this)) && (this->targetActor == NULL)) {
        *arg1 *= Math_SinS(sp2C);

        if (*arg1 != 0.0f) {
            *arg2 = (((sp2E >= 0) ? 1 : -1) << 0xE) + this->actor.shape.rot.y;
        } else {
            *arg2 = this->actor.shape.rot.y;
        }

        if (this->targetActor != NULL) {
            Player_LookAtTargetActor(this, 1);
        } else {
            Math_SmoothStepToS(&this->actor.focus.rot.x, sControlInput->rel.stick_y * 240.0f, 14, 4000, 30);
            Player_UpdateLookAngles(this, 1);
        }
    } else {
        if (this->targetActor != NULL) {
            return func_8083FC68(this, *arg1, *arg2);
        } else {
            func_8083DC54(this, globalCtx);
            if ((*arg1 != 0.0f) && (sp2C < 6000)) {
                return 1;
            } else if (*arg1 > Math_SinS((0x4000 - (sp2C >> 1))) * 200.0f) {
                return -1;
            }
        }
    }

    return 0;
}

s32 func_8083FFB8(Player* this, f32* arg1, s16* arg2) {
    s16 temp1 = *arg2 - this->actor.shape.rot.y;
    u16 temp2 = ABS(temp1);
    f32 temp3 = Math_CosS(temp2);

    *arg1 *= temp3;

    if (*arg1 != 0.0f) {
        if (temp3 > 0) {
            return 1;
        } else {
            return -1;
        }
    }

    return 0;
}

s32 func_80840058(Player* this, f32* arg1, s16* arg2, GlobalContext* globalCtx) {
    func_8083DC54(this, globalCtx);

    if ((*arg1 != 0.0f) || (ABS(this->unk_87C) > 400)) {
        s16 temp1 = *arg2 - Camera_GetInputDirYaw(GET_ACTIVE_CAM(globalCtx));
        u16 temp2 = (ABS(temp1) - 0x2000) & 0xFFFF;

        if ((temp2 < 0x4000) || (this->unk_87C != 0)) {
            return -1;
        } else {
            return 1;
        }
    }

    return 0;
}

void func_80840138(Player* this, f32 arg1, s16 arg2) {
    s16 temp = arg2 - this->actor.shape.rot.y;

    if (arg1 > 0.0f) {
        if (temp < 0) {
            this->leftRightBlendWeightTarget = 0.0f;
        } else {
            this->leftRightBlendWeightTarget = 1.0f;
        }
    }

    Math_StepToF(&this->leftRightBlendWeight, this->leftRightBlendWeightTarget, 0.3f);
}

void func_808401B0(GlobalContext* globalCtx, Player* this) {
    LinkAnimation_BlendToJoint(globalCtx, &this->skelAnime, Player_GetFightingRightAnim(this), this->walkFrame,
                               Player_GetFightingLeftAnim(this), this->walkFrame, this->leftRightBlendWeight,
                               this->blendTable);
}

s32 func_8084021C(f32 arg0, f32 arg1, f32 arg2, f32 arg3) {
    f32 temp;

    if ((arg3 == 0.0f) && (arg1 > 0.0f)) {
        arg3 = arg2;
    }

    temp = (arg0 + arg1) - arg3;

    if (((temp * arg1) >= 0.0f) && (((temp - arg1) * arg1) < 0.0f)) {
        return 1;
    }

    return 0;
}

void Player_SetupWalkSfx(Player* this, f32 frameStep) {
    f32 updateScale = R_UPDATE_RATE * 0.5f;

    frameStep *= updateScale;
    if (frameStep < -7.25) {
        frameStep = -7.25;
    } else if (frameStep > 7.25f) {
        frameStep = 7.25f;
    }

    if ((this->currentBoots == PLAYER_BOOTS_HOVER) && !(this->actor.bgCheckFlags & 1) && (this->hoverBootsTimer != 0)) {
        func_8002F8F0(&this->actor, NA_SE_PL_HOBBERBOOTS_LV - SFX_FLAG);
    } else if (func_8084021C(this->walkFrame, frameStep, 29.0f, 10.0f) ||
               func_8084021C(this->walkFrame, frameStep, 29.0f, 24.0f)) {
        Player_PlayWalkSfx(this, this->linearVelocity);
        if (this->linearVelocity > 4.0f) {
            this->stateFlags2 |= PLAYER_STATE2_MAKING_REACTABLE_NOISE;
        }
    }

    this->walkFrame += frameStep;

    if (this->walkFrame < 0.0f) {
        this->walkFrame += 29.0f;
    } else if (this->walkFrame >= 29.0f) {
        this->walkFrame -= 29.0f;
    }
}

void Player_UnfriendlyZTargetStandingStill(Player* this, GlobalContext* globalCtx) {
    f32 sp44;
    s16 sp42;
    s32 temp1;
    u32 temp2;
    s16 temp3;
    s32 temp4;

    if (this->stateFlags3 & PLAYER_STATE3_ENDING_MELEE_ATTACK) {
        if (Player_GetSwordHeld(this)) {
            this->stateFlags2 |=
                PLAYER_STATE2_DISABLE_MOVE_ROTATION_WHILE_Z_TARGETING | PLAYER_STATE2_ALWAYS_DISABLE_MOVE_ROTATION;
        } else {
            this->stateFlags3 &= ~PLAYER_STATE3_ENDING_MELEE_ATTACK;
        }
    }

    if (this->genericTimer != 0) {
        if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
            Player_EndAnimMovement(this);
            Player_PlayAnimLoop(globalCtx, this, Player_GetFightingRightAnim(this));
            this->genericTimer = 0;
            this->stateFlags3 &= ~PLAYER_STATE3_ENDING_MELEE_ATTACK;
        }
        Player_ResetLeftRightBlendWeight(this);
    } else {
        func_808401B0(globalCtx, this);
    }

    Player_StepLinearVelocityToZero(this);

    if (!Player_SetupSubAction(globalCtx, this, sTargetEnemyStandStillSubActions, 1)) {
        if (!Player_SetupStartUnfriendlyZTargeting(this) && (!Player_IsFriendlyZTargeting(this) || (Player_StandingDefend != this->upperActionFunc))) {
            Player_SetupEndUnfriendlyZTarget(this, globalCtx);
            return;
        }

        Player_GetTargetVelocityAndYaw(this, &sp44, &sp42, 0.0f, globalCtx);

        temp1 = func_8083FC68(this, sp44, sp42);

        if (temp1 > 0) {
            Player_SetupZTargetRunning(this, globalCtx, sp42);
            return;
        }

        if (temp1 < 0) {
            Player_SetupUnfriendlyBackwalk(this, sp42, globalCtx);
            return;
        }

        if (sp44 > 4.0f) {
            Player_SetupSidewalk(this, globalCtx);
            return;
        }

        Player_SetupWalkSfx(this, (this->linearVelocity * 0.3f) + 1.0f);
        func_80840138(this, sp44, sp42);

        temp2 = this->walkFrame;
        if ((temp2 < 6) || ((temp2 - 0xE) < 6)) {
            Math_StepToF(&this->linearVelocity, 0.0f, 1.5f);
            return;
        }

        temp3 = sp42 - this->currentYaw;
        temp4 = ABS(temp3);

        if (temp4 > 0x4000) {
            if (Math_StepToF(&this->linearVelocity, 0.0f, 1.5f)) {
                this->currentYaw = sp42;
            }
            return;
        }

        Math_AsymStepToF(&this->linearVelocity, sp44 * 0.3f, 2.0f, 1.5f);

        if (!(this->stateFlags3 & PLAYER_STATE3_ENDING_MELEE_ATTACK)) {
            Math_ScaledStepToS(&this->currentYaw, sp42, temp4 * 0.1f);
        }
    }
}

void Player_FriendlyZTargetStandingStill(Player* this, GlobalContext* globalCtx) {
    f32 sp3C;
    s16 sp3A;
    s32 temp1;
    s16 temp2;
    s32 temp3;

    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        Player_EndAnimMovement(this);
        Player_PlayAnimOnce(globalCtx, this, Player_GetStandingStillAnim(this));
    }

    Player_StepLinearVelocityToZero(this);

    if (!Player_SetupSubAction(globalCtx, this, sFriendlyTargetingStandStillSubActions, 1)) {
        if (Player_SetupStartUnfriendlyZTargeting(this)) {
            Player_SetupUnfriendlyZTarget(this, globalCtx);
            return;
        }

        if (!Player_IsFriendlyZTargeting(this)) {
            Player_SetActionFuncPreserveMoveFlags(globalCtx, this, Player_StandingStill, 1);
            this->currentYaw = this->actor.shape.rot.y;
            return;
        }

        if (Player_StandingDefend == this->upperActionFunc) {
            Player_SetupUnfriendlyZTarget(this, globalCtx);
            return;
        }

        Player_GetTargetVelocityAndYaw(this, &sp3C, &sp3A, 0.0f, globalCtx);

        temp1 = func_8083FD78(this, &sp3C, &sp3A, globalCtx);

        if (temp1 > 0) {
            Player_SetupZTargetRunning(this, globalCtx, sp3A);
            return;
        }

        if (temp1 < 0) {
            Player_SetupFriendlyBackwalk(this, sp3A, globalCtx);
            return;
        }

        if (sp3C > 4.9f) {
            Player_SetupSidewalk(this, globalCtx);
            Player_ResetLeftRightBlendWeight(this);
            return;
        }
        if (sp3C != 0.0f) {
            Player_SetupFriendlySidewalk(this, globalCtx);
            return;
        }

        temp2 = sp3A - this->actor.shape.rot.y;
        temp3 = ABS(temp2);

        if (temp3 > 800) {
            Player_SetupTurn(globalCtx, this, sp3A);
        }
    }
}

void Player_SetupIdleAnim(GlobalContext* globalCtx, Player* this) {
    LinkAnimationHeader* anim;
    LinkAnimationHeader** animPtr;
    s32 heathIsCritical;
    s32 sp38;
    s32 sp34;

    if ((this->targetActor != NULL) ||
        (!(heathIsCritical = HealthMeter_IsCritical()) && ((this->idleCounter = (this->idleCounter + 1) & 1) != 0))) {
        this->stateFlags2 &= ~PLAYER_STATE2_IDLING;
        anim = Player_GetStandingStillAnim(this);
    } else {
        this->stateFlags2 |= PLAYER_STATE2_IDLING;
        if (this->stateFlags1 & PLAYER_STATE1_HOLDING_ACTOR) {
            anim = Player_GetStandingStillAnim(this);
        } else {
            sp38 = globalCtx->roomCtx.curRoom.unk_02;
            if (heathIsCritical) {
                if (this->idleCounter >= 0) {
                    sp38 = 7;
                    this->idleCounter = -1;
                } else {
                    sp38 = 8;
                }
            } else {
                sp34 = Rand_ZeroOne() * 5.0f;
                if (sp34 < 4) {
                    if (((sp34 != 0) && (sp34 != 3)) || ((this->rightHandType == PLAYER_MODELTYPE_RH_SHIELD) &&
                                                         ((sp34 == 3) || Player_GetSwordHeld(this)))) {
                        if ((sp34 == 1) && Player_HoldsTwoHandedWeapon(this) && CVar_GetS32("gTwoHandedIdle", 1) == 1) {
                            sp34 = 4;
                        }
                        sp38 = sp34 + 9;
                    }
                }
            }
            animPtr = &sIdleAnims[sp38][0];
            if (this->modelAnimType != PLAYER_ANIMTYPE_HOLDING_ONE_HAND_WEAPON) {
                animPtr = &sIdleAnims[sp38][1];
            }
            anim = *animPtr;
        }
    }

    LinkAnimation_Change(globalCtx, &this->skelAnime, anim, (2.0f / 3.0f) * sWaterSpeedScale, 0.0f,
                         Animation_GetLastFrame(anim), ANIMMODE_ONCE, -6.0f);
}

void Player_StandingStill(Player* this, GlobalContext* globalCtx) {
    s32 idleAnimOffset;
    s32 animDone;
    f32 targetVelocity;
    s16 targetYaw;
    s16 targetYawDiff;

    idleAnimOffset = Player_IsPlayingIdleAnim(this);
    animDone = LinkAnimation_Update(globalCtx, &this->skelAnime);

    if (idleAnimOffset > 0) {
        Player_PlayIdleAnimSfx(this, idleAnimOffset - 1);
    }

    if (animDone != 0) {
        if (this->genericTimer != 0) {
            if (DECR(this->genericTimer) == 0) {
                this->skelAnime.endFrame = this->skelAnime.animLength - 1.0f;
            }
            this->skelAnime.jointTable[0].y =
                (this->skelAnime.jointTable[0].y + ((this->genericTimer & 1) * 0x50)) - 0x28;
        } else {
            Player_EndAnimMovement(this);
            Player_SetupIdleAnim(globalCtx, this);
        }
    }

    Player_StepLinearVelocityToZero(this);

    if (this->genericTimer == 0) {
        if (!Player_SetupSubAction(globalCtx, this, sStandStillSubActions, 1)) {
            if (Player_SetupStartUnfriendlyZTargeting(this)) {
                Player_SetupUnfriendlyZTarget(this, globalCtx);
                return;
            }

            if (Player_IsFriendlyZTargeting(this)) {
                Player_SetupFriendlyZTargetingStandStill(this, globalCtx);
                return;
            }

            Player_GetTargetVelocityAndYaw(this, &targetVelocity, &targetYaw, 0.018f, globalCtx);

            if (targetVelocity != 0.0f) {
                Player_SetupZTargetRunning(this, globalCtx, targetYaw);
                return;
            }

            targetYawDiff = targetYaw - this->actor.shape.rot.y;
            if (ABS(targetYawDiff) > 800) {
                Player_SetupTurn(globalCtx, this, targetYaw);
                return;
            }

            Math_ScaledStepToS(&this->actor.shape.rot.y, targetYaw, 1200);
            this->currentYaw = this->actor.shape.rot.y;
            if (Player_GetStandingStillAnim(this) == this->skelAnime.animation) {
                func_8083DC54(this, globalCtx);
            }
        }
    }
}

void Player_EndSidewalk(Player* this, GlobalContext* globalCtx) {
    f32 frames;
    f32 coeff;
    f32 sp44;
    s16 sp42;
    s32 temp1;
    s16 temp2;
    s32 temp3;
    s32 direction;

    this->skelAnime.mode = 0;
    LinkAnimation_SetUpdateFunction(&this->skelAnime);

    this->skelAnime.animation = Player_GetEndSidewalkAnim(this);

    if (this->skelAnime.animation == &gPlayerAnim_0026E8) {
        frames = 24.0f;
        coeff = -(MREG(95) / 100.0f);
    } else {
        frames = 29.0f;
        coeff = MREG(95) / 100.0f;
    }

    this->skelAnime.animLength = frames;
    this->skelAnime.endFrame = frames - 1.0f;

    if ((s16)(this->currentYaw - this->actor.shape.rot.y) >= 0) {
        direction = 1;
    } else {
        direction = -1;
    }

    this->skelAnime.playSpeed = direction * (this->linearVelocity * coeff);

    LinkAnimation_Update(globalCtx, &this->skelAnime);

    if (LinkAnimation_OnFrame(&this->skelAnime, 0.0f) || LinkAnimation_OnFrame(&this->skelAnime, frames * 0.5f)) {
        Player_PlayWalkSfx(this, this->linearVelocity);
    }

    if (!Player_SetupSubAction(globalCtx, this, sEndSidewalkSubActions, 1)) {
        if (Player_SetupStartUnfriendlyZTargeting(this)) {
            Player_SetupUnfriendlyZTarget(this, globalCtx);
            return;
        }

        if (!Player_IsFriendlyZTargeting(this)) {
            Player_SetupStandingStillMorph(this, globalCtx);
            return;
        }

        Player_GetTargetVelocityAndYaw(this, &sp44, &sp42, 0.0f, globalCtx);
        temp1 = func_8083FD78(this, &sp44, &sp42, globalCtx);

        if (temp1 > 0) {
            Player_SetupZTargetRunning(this, globalCtx, sp42);
            return;
        }

        if (temp1 < 0) {
            Player_SetupFriendlyBackwalk(this, sp42, globalCtx);
            return;
        }

        if (sp44 > 4.9f) {
            Player_SetupSidewalk(this, globalCtx);
            Player_ResetLeftRightBlendWeight(this);
            return;
        }

        if ((sp44 == 0.0f) && (this->linearVelocity == 0.0f)) {
            Player_SetupFriendlyZTargetingStandStill(this, globalCtx);
            return;
        }

        temp2 = sp42 - this->currentYaw;
        temp3 = ABS(temp2);

        if (temp3 > 0x4000) {
            if (Math_StepToF(&this->linearVelocity, 0.0f, 1.5f)) {
                this->currentYaw = sp42;
            }
            return;
        }

        Math_AsymStepToF(&this->linearVelocity, sp44 * 0.4f, 1.5f, 1.5f);
        Math_ScaledStepToS(&this->currentYaw, sp42, temp3 * 0.1f);
    }
}

void func_80841138(Player* this, GlobalContext* globalCtx) {
    f32 temp1;
    f32 temp2;

    if (this->unk_864 < 1.0f) {
        temp1 = R_UPDATE_RATE * 0.5f;
        Player_SetupWalkSfx(this, REG(35) / 1000.0f);
        LinkAnimation_LoadToJoint(globalCtx, &this->skelAnime,
                                  GET_PLAYER_ANIM(PLAYER_ANIMGROUP_BACKWALKING, this->modelAnimType), this->walkFrame);
        this->unk_864 += 1 * temp1;
        if (this->unk_864 >= 1.0f) {
            this->unk_864 = 1.0f;
        }
        temp1 = this->unk_864;
    } else {
        temp2 = this->linearVelocity - (REG(48) / 100.0f);
        if (temp2 < 0.0f) {
            temp1 = 1.0f;
            Player_SetupWalkSfx(this, (REG(35) / 1000.0f) + ((REG(36) / 1000.0f) * this->linearVelocity));
            LinkAnimation_LoadToJoint(globalCtx, &this->skelAnime,
                                      GET_PLAYER_ANIM(PLAYER_ANIMGROUP_BACKWALKING, this->modelAnimType),
                                      this->walkFrame);
        } else {
            temp1 = (REG(37) / 1000.0f) * temp2;
            if (temp1 < 1.0f) {
                Player_SetupWalkSfx(this, (REG(35) / 1000.0f) + ((REG(36) / 1000.0f) * this->linearVelocity));
            } else {
                temp1 = 1.0f;
                Player_SetupWalkSfx(this, 1.2f + ((REG(38) / 1000.0f) * temp2));
            }
            LinkAnimation_LoadToMorph(globalCtx, &this->skelAnime,
                                      GET_PLAYER_ANIM(PLAYER_ANIMGROUP_BACKWALKING, this->modelAnimType),
                                      this->walkFrame);
            LinkAnimation_LoadToJoint(globalCtx, &this->skelAnime, &gPlayerAnim_002DD0,
                                      this->walkFrame * (16.0f / 29.0f));
        }
    }

    if (temp1 < 1.0f) {
        LinkAnimation_InterpJointMorph(globalCtx, &this->skelAnime, 1.0f - temp1);
    }
}

void func_8084140C(Player* this, GlobalContext* globalCtx) {
    Player_SetActionFunc(globalCtx, this, Player_HaltFriendlyBackwalk, 1);
    Player_ChangeAnimMorphToLastFrame(globalCtx, this, &gPlayerAnim_002DA0);
}

s32 func_80841458(Player* this, f32* arg1, s16* arg2, GlobalContext* globalCtx) {
    if (this->linearVelocity > 6.0f) {
        func_8084140C(this, globalCtx);
        return 1;
    }

    if (*arg1 != 0.0f) {
        if (Player_StepLinearVelocityToZero(this)) {
            *arg1 = 0.0f;
            *arg2 = this->currentYaw;
        } else {
            return 1;
        }
    }

    return 0;
}

void Player_FriendlyBackwalk(Player* this, GlobalContext* globalCtx) {
    f32 sp34;
    s16 sp32;
    s32 sp2C;
    s16 sp2A;

    func_80841138(this, globalCtx);

    if (!Player_SetupSubAction(globalCtx, this, sFriendlyBackwalkSubActions, 1)) {
        if (!Player_IsZTargetingSetupStartUnfriendly(this)) {
            Player_SetupZTargetRunning(this, globalCtx, this->currentYaw);
            return;
        }

        Player_GetTargetVelocityAndYaw(this, &sp34, &sp32, 0.0f, globalCtx);
        sp2C = func_8083FD78(this, &sp34, &sp32, globalCtx);

        if (sp2C >= 0) {
            if (!func_80841458(this, &sp34, &sp32, globalCtx)) {
                if (sp2C != 0) {
                    Player_SetupRun(this, globalCtx);
                } else if (sp34 > 4.9f) {
                    Player_SetupSidewalk(this, globalCtx);
                } else {
                    Player_SetupFriendlySidewalk(this, globalCtx);
                }
            }
        } else {
            sp2A = sp32 - this->currentYaw;

            Math_AsymStepToF(&this->linearVelocity, sp34 * 1.5f, 1.5f, 2.0f);
            Math_ScaledStepToS(&this->currentYaw, sp32, sp2A * 0.1f);

            if ((sp34 == 0.0f) && (this->linearVelocity == 0.0f)) {
                Player_SetupFriendlyZTargetingStandStill(this, globalCtx);
            }
        }
    }
}

void func_808416C0(Player* this, GlobalContext* globalCtx) {
    Player_SetActionFunc(globalCtx, this, Player_EndHaltFriendlyBackwalk, 1);
    Player_PlayAnimOnce(globalCtx, this, &gPlayerAnim_002DA8);
}

void Player_HaltFriendlyBackwalk(Player* this, GlobalContext* globalCtx) {
    s32 sp34;
    f32 sp30;
    s16 sp2E;

    sp34 = LinkAnimation_Update(globalCtx, &this->skelAnime);
    Player_StepLinearVelocityToZero(this);

    if (!Player_SetupSubAction(globalCtx, this, sFriendlyBackwalkSubActions, 1)) {
        Player_GetTargetVelocityAndYaw(this, &sp30, &sp2E, 0.0f, globalCtx);

        if (this->linearVelocity == 0.0f) {
            this->currentYaw = this->actor.shape.rot.y;

            if (func_8083FD78(this, &sp30, &sp2E, globalCtx) > 0) {
                Player_SetupRun(this, globalCtx);
            } else if ((sp30 != 0.0f) || (sp34 != 0)) {
                func_808416C0(this, globalCtx);
            }
        }
    }
}

void Player_EndHaltFriendlyBackwalk(Player* this, GlobalContext* globalCtx) {
    s32 sp1C;

    sp1C = LinkAnimation_Update(globalCtx, &this->skelAnime);

    if (!Player_SetupSubAction(globalCtx, this, sFriendlyBackwalkSubActions, 1)) {
        if (sp1C != 0) {
            Player_SetupFriendlyZTargetingStandStill(this, globalCtx);
        }
    }
}

void func_80841860(GlobalContext* globalCtx, Player* this) {
    f32 frame;
    LinkAnimationHeader* sidewalkLeftAnim = GET_PLAYER_ANIM(PLAYER_ANIMGROUP_SIDEWALKING_LEFT, this->modelAnimType);
    LinkAnimationHeader* sidewalkRightAnim = GET_PLAYER_ANIM(PLAYER_ANIMGROUP_SIDEWALKING_RIGHT, this->modelAnimType);

    this->skelAnime.animation = sidewalkLeftAnim;

    Player_SetupWalkSfx(this, (REG(30) / 1000.0f) + ((REG(32) / 1000.0f) * this->linearVelocity));

    frame = this->walkFrame * (16.0f / 29.0f);
    LinkAnimation_BlendToJoint(globalCtx, &this->skelAnime, sidewalkRightAnim, frame, sidewalkLeftAnim, frame,
                               this->leftRightBlendWeight, this->blendTable);
}

void Player_Sidewalk(Player* this, GlobalContext* globalCtx) {
    f32 sp3C;
    s16 sp3A;
    s32 temp1;
    s16 temp2;
    s32 temp3;

    func_80841860(globalCtx, this);

    if (!Player_SetupSubAction(globalCtx, this, sSidewalkSubActions, 1)) {
        if (!Player_IsZTargetingSetupStartUnfriendly(this)) {
            Player_SetupRun(this, globalCtx);
            return;
        }

        Player_GetTargetVelocityAndYaw(this, &sp3C, &sp3A, 0.0f, globalCtx);

        if (Player_IsFriendlyZTargeting(this)) {
            temp1 = func_8083FD78(this, &sp3C, &sp3A, globalCtx);
        } else {
            temp1 = func_8083FC68(this, sp3C, sp3A);
        }

        if (temp1 > 0) {
            Player_SetupRun(this, globalCtx);
            return;
        }

        if (temp1 < 0) {
            if (Player_IsFriendlyZTargeting(this)) {
                Player_SetupFriendlyBackwalk(this, sp3A, globalCtx);
            } else {
                Player_SetupUnfriendlyBackwalk(this, sp3A, globalCtx);
            }
            return;
        }

        if ((this->linearVelocity < 3.6f) && (sp3C < 4.0f)) {
            if (!Player_IsUnfriendlyZTargeting(this) && Player_IsFriendlyZTargeting(this)) {
                Player_SetupFriendlySidewalk(this, globalCtx);
            } else {
                Player_SetupStandingStillType(this, globalCtx);
            }
            return;
        }

        func_80840138(this, sp3C, sp3A);

        temp2 = sp3A - this->currentYaw;
        temp3 = ABS(temp2);

        if (temp3 > 0x4000) {
            if (Math_StepToF(&this->linearVelocity, 0.0f, 3.0f) != 0) {
                this->currentYaw = sp3A;
            }
            return;
        }

        sp3C *= 0.9f;
        Math_AsymStepToF(&this->linearVelocity, sp3C, 2.0f, 3.0f);
        Math_ScaledStepToS(&this->currentYaw, sp3A, temp3 * 0.1f);
    }
}

void Player_Turn(Player* this, GlobalContext* globalCtx) {
    f32 sp34;
    s16 sp32;

    LinkAnimation_Update(globalCtx, &this->skelAnime);

    if (Player_HoldsTwoHandedWeapon(this)) {
        AnimationContext_SetLoadFrame(globalCtx, Player_GetStandingStillAnim(this), 0, this->skelAnime.limbCount,
                                      this->skelAnime.morphTable);
        AnimationContext_SetCopyTrue(globalCtx, this->skelAnime.limbCount, this->skelAnime.jointTable,
                                     this->skelAnime.morphTable, D_80853410);
    }

    Player_GetTargetVelocityAndYaw(this, &sp34, &sp32, 0.018f, globalCtx);

    if (!Player_SetupSubAction(globalCtx, this, sTurnSubActions, 1)) {
        if (sp34 != 0.0f) {
            this->actor.shape.rot.y = sp32;
            Player_SetupRun(this, globalCtx);
        } else if (Math_ScaledStepToS(&this->actor.shape.rot.y, sp32, this->unk_87E)) {
            Player_SetupStandingStillNoMorph(this, globalCtx);
        }

        this->currentYaw = this->actor.shape.rot.y;
    }
}

void Player_BlendWalkAnims(Player* this, s32 blendToMorph, GlobalContext* globalCtx) {
    LinkAnimationHeader* anim;
    s16 targetPitch;
    f32 blendWeight;

    if (ABS(sAngleToFloorX) < 3640) {
        targetPitch = 0;
    } else {
        targetPitch = CLAMP(sAngleToFloorX, -10922, 10922);
    }

    Math_ScaledStepToS(&this->walkAngleToFloorX, targetPitch, 400);

    if ((this->modelAnimType == PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON) ||
        ((this->walkAngleToFloorX == 0) && (this->shapeOffsetY <= 0.0f))) {
        if (blendToMorph == 0) {
            LinkAnimation_LoadToJoint(globalCtx, &this->skelAnime,
                                      GET_PLAYER_ANIM(PLAYER_ANIMGROUP_WALKING, this->modelAnimType), this->walkFrame);
        } else {
            LinkAnimation_LoadToMorph(globalCtx, &this->skelAnime,
                                      GET_PLAYER_ANIM(PLAYER_ANIMGROUP_WALKING, this->modelAnimType), this->walkFrame);
        }
        return;
    }

    if (this->walkAngleToFloorX != 0) {
        blendWeight = this->walkAngleToFloorX / 10922.0f;
    } else {
        blendWeight = this->shapeOffsetY * 0.0006f;
    }

    blendWeight *= fabsf(this->linearVelocity) * 0.5f;

    if (blendWeight > 1.0f) {
        blendWeight = 1.0f;
    }

    if (blendWeight < 0.0f) {
        anim = &gPlayerAnim_002E48;
        blendWeight = -blendWeight;
    } else {
        anim = &gPlayerAnim_002E90;
    }

    if (blendToMorph == 0) {
        LinkAnimation_BlendToJoint(globalCtx, &this->skelAnime,
                                   GET_PLAYER_ANIM(PLAYER_ANIMGROUP_WALKING, this->modelAnimType), this->walkFrame,
                                   anim, this->walkFrame, blendWeight, this->blendTable);
    } else {
        LinkAnimation_BlendToMorph(globalCtx, &this->skelAnime,
                                   GET_PLAYER_ANIM(PLAYER_ANIMGROUP_WALKING, this->modelAnimType), this->walkFrame,
                                   anim, this->walkFrame, blendWeight, this->blendTable);
    }
}

void func_80841EE4(Player* this, GlobalContext* globalCtx) {
    f32 temp1;
    f32 temp2;

    if (this->unk_864 < 1.0f) {
        temp1 = R_UPDATE_RATE * 0.5f;

        Player_SetupWalkSfx(this, REG(35) / 1000.0f);
        LinkAnimation_LoadToJoint(globalCtx, &this->skelAnime,
                                  GET_PLAYER_ANIM(PLAYER_ANIMGROUP_WALKING, this->modelAnimType), this->walkFrame);

        this->unk_864 += 1 * temp1;
        if (this->unk_864 >= 1.0f) {
            this->unk_864 = 1.0f;
        }

        temp1 = this->unk_864;
    } else {
        temp2 = this->linearVelocity - (REG(48) / 100.0f);

        if (temp2 < 0.0f) {
            temp1 = 1.0f;
            Player_SetupWalkSfx(this, (REG(35) / 1000.0f) + ((REG(36) / 1000.0f) * this->linearVelocity));

            Player_BlendWalkAnims(this, 0, globalCtx);
        } else {
            temp1 = (REG(37) / 1000.0f) * temp2;
            if (temp1 < 1.0f) {
                Player_SetupWalkSfx(this, (REG(35) / 1000.0f) + ((REG(36) / 1000.0f) * this->linearVelocity));
            } else {
                temp1 = 1.0f;
                Player_SetupWalkSfx(this, 1.2f + ((REG(38) / 1000.0f) * temp2));
            }

            Player_BlendWalkAnims(this, 1, globalCtx);

            LinkAnimation_LoadToJoint(globalCtx, &this->skelAnime, Player_GetRunningAnim(this),
                                      this->walkFrame * (20.0f / 29.0f));
        }
    }

    if (temp1 < 1.0f) {
        LinkAnimation_InterpJointMorph(globalCtx, &this->skelAnime, 1.0f - temp1);
    }
}

void Player_Run(Player* this, GlobalContext* globalCtx) {
    f32 sp2C;
    s16 sp2A;

    this->stateFlags2 |= PLAYER_STATE2_DISABLE_MOVE_ROTATION_WHILE_Z_TARGETING;
    func_80841EE4(this, globalCtx);

    if (!Player_SetupSubAction(globalCtx, this, sRunSubActions, 1)) {
        if (Player_IsZTargetingSetupStartUnfriendly(this)) {
            Player_SetupRun(this, globalCtx);
            return;
        }

        Player_GetTargetVelocityAndYaw(this, &sp2C, &sp2A, 0.018f, globalCtx);

        if (!Player_SetupTurnAroundRunning(this, &sp2C, &sp2A)) {
            if (CVar_GetS32("gMMBunnyHood", 0) != 0 && this->currentMask == PLAYER_MASK_BUNNY) {
                sp2C *= 1.5f;
            }
            Player_SetRunVelocityAndYaw(this, sp2C, sp2A);
            func_8083DDC8(this, globalCtx);

            if ((this->linearVelocity == 0.0f) && (sp2C == 0.0f)) {
                Player_SetupEndRun(this, globalCtx);
            }
        }
    }
}

void Player_ZTargetingRun(Player* this, GlobalContext* globalCtx) {
    f32 sp2C;
    s16 sp2A;

    this->stateFlags2 |= PLAYER_STATE2_DISABLE_MOVE_ROTATION_WHILE_Z_TARGETING;
    func_80841EE4(this, globalCtx);

    if (!Player_SetupSubAction(globalCtx, this, sTargetRunSubActions, 1)) {
        if (!Player_IsZTargetingSetupStartUnfriendly(this)) {
            Player_SetupRun(this, globalCtx);
            return;
        }

        Player_GetTargetVelocityAndYaw(this, &sp2C, &sp2A, 0.0f, globalCtx);

        if (!Player_SetupTurnAroundRunning(this, &sp2C, &sp2A)) {
            if ((Player_IsFriendlyZTargeting(this) && (sp2C != 0.0f) && (func_8083FD78(this, &sp2C, &sp2A, globalCtx) <= 0)) ||
                (!Player_IsFriendlyZTargeting(this) && (func_8083FC68(this, sp2C, sp2A) <= 0))) {
                Player_SetupStandingStillType(this, globalCtx);
                return;
            }

            Player_SetRunVelocityAndYaw(this, sp2C, sp2A);
            func_8083DDC8(this, globalCtx);

            if ((this->linearVelocity == 0) && (sp2C == 0)) {
                Player_SetupStandingStillType(this, globalCtx);
            }
        }
    }
}

void Player_UnfriendlyBackwalk(Player* this, GlobalContext* globalCtx) {
    s32 sp34;
    f32 sp30;
    s16 sp2E;

    sp34 = LinkAnimation_Update(globalCtx, &this->skelAnime);

    if (!Player_SetupSubAction(globalCtx, this, sSidewalkSubActions, 1)) {
        if (!Player_IsZTargetingSetupStartUnfriendly(this)) {
            Player_SetupRun(this, globalCtx);
            return;
        }

        Player_GetTargetVelocityAndYaw(this, &sp30, &sp2E, 0.0f, globalCtx);

        if ((this->skelAnime.morphWeight == 0.0f) && (this->skelAnime.curFrame > 5.0f)) {
            Player_StepLinearVelocityToZero(this);

            if ((this->skelAnime.curFrame > 10.0f) && (func_8083FC68(this, sp30, sp2E) < 0)) {
                Player_SetupUnfriendlyBackwalk(this, sp2E, globalCtx);
                return;
            }

            if (sp34 != 0) {
                Player_SetupEndUnfriendlyBackwalk(this, globalCtx);
            }
        }
    }
}

void Player_EndUnfriendlyBackwalk(Player* this, GlobalContext* globalCtx) {
    s32 sp34;
    f32 sp30;
    s16 sp2E;

    sp34 = LinkAnimation_Update(globalCtx, &this->skelAnime);

    Player_StepLinearVelocityToZero(this);

    if (!Player_SetupSubAction(globalCtx, this, sEndBackwalkSubActions, 1)) {
        Player_GetTargetVelocityAndYaw(this, &sp30, &sp2E, 0.0f, globalCtx);

        if (this->linearVelocity == 0.0f) {
            this->currentYaw = this->actor.shape.rot.y;

            if (func_8083FC68(this, sp30, sp2E) > 0) {
                Player_SetupRun(this, globalCtx);
                return;
            }

            if ((sp30 != 0.0f) || (sp34 != 0)) {
                Player_SetupStandingStillType(this, globalCtx);
            }
        }
    }
}

void func_8084260C(Vec3f* src, Vec3f* dest, f32 arg2, f32 arg3, f32 arg4) {
    dest->x = (Rand_ZeroOne() * arg3) + src->x;
    dest->y = (Rand_ZeroOne() * arg4) + (src->y + arg2);
    dest->z = (Rand_ZeroOne() * arg3) + src->z;
}

static Vec3f D_808545B4 = { 0.0f, 0.0f, 0.0f };
static Vec3f D_808545C0 = { 0.0f, 0.0f, 0.0f };

s32 Player_SetupSpawnDustAtFeet(GlobalContext* globalCtx, Player* this) {
    Vec3f sp2C;

    if ((this->surfaceMaterial == 0) || (this->surfaceMaterial == 1)) {
        func_8084260C(&this->actor.shape.feetPos[FOOT_LEFT], &sp2C,
                      this->actor.floorHeight - this->actor.shape.feetPos[FOOT_LEFT].y, 7.0f, 5.0f);
        func_800286CC(globalCtx, &sp2C, &D_808545B4, &D_808545C0, 50, 30);
        func_8084260C(&this->actor.shape.feetPos[FOOT_RIGHT], &sp2C,
                      this->actor.floorHeight - this->actor.shape.feetPos[FOOT_RIGHT].y, 7.0f, 5.0f);
        func_800286CC(globalCtx, &this->actor.shape.feetPos[FOOT_RIGHT], &D_808545B4, &D_808545C0, 50, 30);
        return 1;
    }

    return 0;
}

void func_8084279C(Player* this, GlobalContext* globalCtx) {
    Player_LoopAnimContinuously(globalCtx, this,
                                GET_PLAYER_ANIM(PLAYER_ANIMGROUP_CHECKING_OR_SPEAKING, this->modelAnimType));

    if (DECR(this->genericTimer) == 0) {
        if (!Player_SetupItemCutsceneOrFirstPerson(this, globalCtx)) {
            Player_SetupReturnToStandStillSetAnim(
                this, GET_PLAYER_ANIM(PLAYER_ANIMGROUP_END_CHECKING_OR_SPEAKING, this->modelAnimType), globalCtx);
        }

        this->actor.flags &= ~ACTOR_FLAG_8;
        func_8005B1A4(Gameplay_GetCamera(globalCtx, 0));
    }
}

s32 func_8084285C(Player* this, f32 arg1, f32 arg2, f32 arg3) {
    if ((arg1 <= this->skelAnime.curFrame) && (this->skelAnime.curFrame <= arg3)) {
        Player_MeleeAttack(this, (arg2 <= this->skelAnime.curFrame) ? 1 : -1);
        return 1;
    }

    Player_InactivateMeleeWeapon(this);
    return 0;
}

s32 func_808428D8(Player* this, GlobalContext* globalCtx) {
    if (!Player_IsChildWithHylianShield(this) && Player_GetSwordHeld(this) && sUsingItemAlreadyInHand) {
        Player_PlayAnimOnce(globalCtx, this, &gPlayerAnim_002EC8);
        this->genericVar = 1;
        this->swordAnimation = 0xC;
        this->currentYaw = this->actor.shape.rot.y + this->upperBodyRot.y;
        return 1;
    }

    return 0;
}

s32 func_80842964(Player* this, GlobalContext* globalCtx) {
    return Player_SetupItemCutsceneOrFirstPerson(this, globalCtx) || Player_SetupSpeakOrCheck(this, globalCtx) ||
           Player_SetupGetItemOrHoldBehavior(this, globalCtx);
}

void func_808429B4(GlobalContext* globalCtx, s32 speed, s32 y, s32 countdown) {
    s32 quakeIdx = Quake_Add(Gameplay_GetCamera(globalCtx, 0), 3);

    Quake_SetSpeed(quakeIdx, speed);
    Quake_SetQuakeValues(quakeIdx, y, 0, 0, 0);
    Quake_SetCountdown(quakeIdx, countdown);
}

void func_80842A28(GlobalContext* globalCtx, Player* this) {
    func_808429B4(globalCtx, 27767, 7, 20);
    globalCtx->actorCtx.unk_02 = 4;
    Player_RequestRumble(this, 255, 20, 150, 0);
    func_8002F7DC(&this->actor, NA_SE_IT_HAMMER_HIT);
}

void func_80842A88(GlobalContext* globalCtx, Player* this) {
    Inventory_ChangeAmmo(ITEM_STICK, -1);
    Player_UseItem(globalCtx, this, ITEM_NONE);
}

s32 func_80842AC4(GlobalContext* globalCtx, Player* this) {
    if ((this->heldItemActionParam == PLAYER_AP_STICK) && (this->unk_85C > 0.5f)) {
        if (AMMO(ITEM_STICK) != 0) {
            EffectSsStick_Spawn(globalCtx, &this->bodyPartsPos[PLAYER_BODYPART_R_HAND],
                                this->actor.shape.rot.y + 0x8000);
            this->unk_85C = 0.5f;
            func_80842A88(globalCtx, this);
            func_8002F7DC(&this->actor, NA_SE_IT_WOODSTICK_BROKEN);
        }

        return 1;
    }

    return 0;
}

s32 func_80842B7C(GlobalContext* globalCtx, Player* this) {
    if (this->heldItemActionParam == PLAYER_AP_SWORD_BGS) {
        if (!gSaveContext.bgsFlag && (gSaveContext.swordHealth > 0.0f)) {
            if ((gSaveContext.swordHealth -= 1.0f) <= 0.0f) {
                EffectSsStick_Spawn(globalCtx, &this->bodyPartsPos[PLAYER_BODYPART_R_HAND],
                                    this->actor.shape.rot.y + 0x8000);
                func_800849EC(globalCtx);
                func_8002F7DC(&this->actor, NA_SE_IT_MAJIN_SWORD_BROKEN);
            }
        }

        return 1;
    }

    return 0;
}

void func_80842CF0(GlobalContext* globalCtx, Player* this) {
    func_80842AC4(globalCtx, this);
    func_80842B7C(globalCtx, this);
}

static LinkAnimationHeader* D_808545CC[] = {
    &gPlayerAnim_002B10,
    &gPlayerAnim_002B20,
    &gPlayerAnim_002B08,
    &gPlayerAnim_002B18,
};

void func_80842D20(GlobalContext* globalCtx, Player* this) {
    s32 pad;
    s32 sp28;

    if (Player_AimShieldCrouched != this->actionFunc) {
        Player_ResetAttributes(globalCtx, this);
        Player_SetActionFunc(globalCtx, this, Player_MeleeWeaponRebound, 0);

        if (Player_IsUnfriendlyZTargeting(this)) {
            sp28 = 2;
        } else {
            sp28 = 0;
        }

        Player_PlayAnimOnceSlowed(globalCtx, this, D_808545CC[Player_HoldsTwoHandedWeapon(this) + sp28]);
    }

    Player_RequestRumble(this, 180, 20, 100, 0);
    this->linearVelocity = -18.0f;
    func_80842CF0(globalCtx, this);
}

s32 func_80842DF4(GlobalContext* globalCtx, Player* this) {
    f32 phi_f2;
    CollisionPoly* sp78;
    s32 sp74;
    Vec3f sp68;
    Vec3f sp5C;
    Vec3f sp50;
    s32 temp1;
    s32 sp48;

    if (this->swordState > 0) {
        if (this->swordAnimation < 0x18) {
            if (!(this->swordQuads[0].base.atFlags & AT_BOUNCED) && !(this->swordQuads[1].base.atFlags & AT_BOUNCED)) {
                if (this->skelAnime.curFrame >= 2.0f) {

                    phi_f2 = Math_Vec3f_DistXYZAndStoreDiff(&this->swordInfo[0].tip, &this->swordInfo[0].base, &sp50);
                    if (phi_f2 != 0.0f) {
                        phi_f2 = (phi_f2 + 10.0f) / phi_f2;
                    }

                    sp68.x = this->swordInfo[0].tip.x + (sp50.x * phi_f2);
                    sp68.y = this->swordInfo[0].tip.y + (sp50.y * phi_f2);
                    sp68.z = this->swordInfo[0].tip.z + (sp50.z * phi_f2);

                    if (BgCheck_EntityLineTest1(&globalCtx->colCtx, &sp68, &this->swordInfo[0].tip, &sp5C, &sp78, true,
                                                false, false, true, &sp74) &&
                        !SurfaceType_IsIgnoredByEntities(&globalCtx->colCtx, sp78, sp74) &&
                        (func_80041D4C(&globalCtx->colCtx, sp78, sp74) != 6) &&
                        (func_8002F9EC(globalCtx, &this->actor, sp78, sp74, &sp5C) == 0)) {

                        if (this->heldItemActionParam == PLAYER_AP_HAMMER) {
                            Player_SetFreezeFlashTimer(globalCtx);
                            func_80842A28(globalCtx, this);
                            func_80842D20(globalCtx, this);
                            return 1;
                        }

                        if (this->linearVelocity >= 0.0f) {
                            sp48 = func_80041F10(&globalCtx->colCtx, sp78, sp74);

                            if (sp48 == 0xA) {
                                CollisionCheck_SpawnShieldParticlesWood(globalCtx, &sp5C, &this->actor.projectedPos);
                            } else {
                                CollisionCheck_SpawnShieldParticles(globalCtx, &sp5C);
                                if (sp48 == 0xB) {
                                    func_8002F7DC(&this->actor, NA_SE_IT_WALL_HIT_SOFT);
                                } else {
                                    func_8002F7DC(&this->actor, NA_SE_IT_WALL_HIT_HARD);
                                }
                            }

                            func_80842CF0(globalCtx, this);
                            this->linearVelocity = -14.0f;
                            Player_RequestRumble(this, 180, 20, 100, 0);
                        }
                    }
                }
            } else {
                func_80842D20(globalCtx, this);
                Player_SetFreezeFlashTimer(globalCtx);
                return 1;
            }
        }

        temp1 = (this->swordQuads[0].base.atFlags & AT_HIT) || (this->swordQuads[1].base.atFlags & AT_HIT);

        if (temp1) {
            if (this->swordAnimation < 0x18) {
                Actor* at = this->swordQuads[temp1 ? 1 : 0].base.at;

                if ((at != NULL) && (at->id != ACTOR_EN_KANBAN)) {
                    Player_SetFreezeFlashTimer(globalCtx);
                }
            }

            if ((func_80842AC4(globalCtx, this) == 0) && (this->heldItemActionParam != PLAYER_AP_HAMMER)) {
                func_80842B7C(globalCtx, this);

                if (this->actor.colChkInfo.atHitEffect == 1) {
                    this->actor.colChkInfo.damage = 8;
                    Player_SetupDamage(globalCtx, this, 4, 0.0f, 0.0f, this->actor.shape.rot.y, 20);
                    return 1;
                }
            }
        }
    }

    return 0;
}

void Player_AimShieldCrouched(Player* this, GlobalContext* globalCtx) {
    f32 sp54;
    f32 sp50;
    s16 sp4E;
    s16 sp4C;
    s16 sp4A;
    s16 sp48;
    s16 sp46;
    f32 sp40;

    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        if (!Player_IsChildWithHylianShield(this)) {
            Player_PlayAnimLoop(globalCtx, this, GET_PLAYER_ANIM(PLAYER_ANIMGROUP_DEFENDING, this->modelAnimType));
        }
        this->genericTimer = 1;
        this->genericVar = 0;
    }

    if (!Player_IsChildWithHylianShield(this)) {
        this->stateFlags1 |= PLAYER_STATE1_SHIELDING;
        Player_SetupCurrentUpperAction(this, globalCtx);
        this->stateFlags1 &= ~PLAYER_STATE1_SHIELDING;
    }

    Player_StepLinearVelocityToZero(this);

    if (this->genericTimer != 0) {
        sp54 = sControlInput->rel.stick_y * 100;
        sp50 = sControlInput->rel.stick_x * -120;
        sp4E = this->actor.shape.rot.y - Camera_GetInputDirYaw(GET_ACTIVE_CAM(globalCtx));

        sp40 = Math_CosS(sp4E);
        sp4C = (Math_SinS(sp4E) * sp50) + (sp54 * sp40);
        sp40 = Math_CosS(sp4E);
        sp4A = (sp50 * sp40) - (Math_SinS(sp4E) * sp54);

        if (sp4C > 3500) {
            sp4C = 3500;
        }

        sp48 = ABS(sp4C - this->actor.focus.rot.x) * 0.25f;
        if (sp48 < 100) {
            sp48 = 100;
        }

        sp46 = ABS(sp4A - this->upperBodyRot.y) * 0.25f;
        if (sp46 < 50) {
            sp46 = 50;
        }

        Math_ScaledStepToS(&this->actor.focus.rot.x, sp4C, sp48);
        this->upperBodyRot.x = this->actor.focus.rot.x;
        Math_ScaledStepToS(&this->upperBodyRot.y, sp4A, sp46);

        if (this->genericVar != 0) {
            if (!func_80842DF4(globalCtx, this)) {
                if (this->skelAnime.curFrame < 2.0f) {
                    Player_MeleeAttack(this, 1);
                }
            } else {
                this->genericTimer = 1;
                this->genericVar = 0;
            }
        } else if (!func_80842964(this, globalCtx)) {
            if (Player_SetupDefend(this, globalCtx)) {
                func_808428D8(this, globalCtx);
            } else {
                this->stateFlags1 &= ~PLAYER_STATE1_SHIELDING;
                Player_InactivateMeleeWeapon(this);

                if (Player_IsChildWithHylianShield(this)) {
                    Player_SetupReturnToStandStill(this, globalCtx);
                    LinkAnimation_Change(globalCtx, &this->skelAnime, &gPlayerAnim_002400, 1.0f,
                                         Animation_GetLastFrame(&gPlayerAnim_002400), 0.0f, ANIMMODE_ONCE, 0.0f);
                    Player_SetupAnimMovement(globalCtx, this, 4);
                } else {
                    if (this->itemActionParam < 0) {
                        Player_SetHeldItem(this);
                    }
                    Player_SetupReturnToStandStillSetAnim(
                        this, GET_PLAYER_ANIM(PLAYER_ANIMGROUP_END_DEFENDING, this->modelAnimType), globalCtx);
                }

                func_8002F7DC(&this->actor, NA_SE_IT_SHIELD_REMOVE);
                return;
            }
        } else {
            return;
        }
    }

    this->stateFlags1 |= PLAYER_STATE1_SHIELDING;
    Player_SetModelsForHoldingShield(this);

    this->lookFlags |= 0xC1;
}

void Player_DeflectAttackWithShield(Player* this, GlobalContext* globalCtx) {
    s32 temp;
    LinkAnimationHeader* anim;
    f32 frames;

    Player_StepLinearVelocityToZero(this);

    if (this->genericVar == 0) {
        D_808535E0 = Player_SetupCurrentUpperAction(this, globalCtx);
        if ((Player_StandingDefend == this->upperActionFunc) ||
            (Player_IsActionInterrupted(globalCtx, this, &this->skelAnimeUpper, 4.0f) > 0)) {
            Player_SetActionFunc(globalCtx, this, Player_UnfriendlyZTargetStandingStill, 1);
        }
    } else {
        temp = Player_IsActionInterrupted(globalCtx, this, &this->skelAnime, 4.0f);
        if ((temp != 0) && ((temp > 0) || LinkAnimation_Update(globalCtx, &this->skelAnime))) {
            Player_SetActionFunc(globalCtx, this, Player_AimShieldCrouched, 1);
            this->stateFlags1 |= PLAYER_STATE1_SHIELDING;
            Player_SetModelsForHoldingShield(this);
            anim = GET_PLAYER_ANIM(PLAYER_ANIMGROUP_START_DEFENDING, this->modelAnimType);
            frames = Animation_GetLastFrame(anim);
            LinkAnimation_Change(globalCtx, &this->skelAnime, anim, 1.0f, frames, frames, ANIMMODE_ONCE, 0.0f);
        }
    }
}

void func_8084370C(Player* this, GlobalContext* globalCtx) {
    s32 sp1C;

    Player_StepLinearVelocityToZero(this);

    sp1C = Player_IsActionInterrupted(globalCtx, this, &this->skelAnime, 16.0f);
    if ((sp1C != 0) && (LinkAnimation_Update(globalCtx, &this->skelAnime) || (sp1C > 0))) {
        Player_SetupStandingStillType(this, globalCtx);
    }
}

void Player_StartKnockback(Player* this, GlobalContext* globalCtx) {
    this->stateFlags2 |=
        PLAYER_STATE2_DISABLE_MOVE_ROTATION_WHILE_Z_TARGETING | PLAYER_STATE2_ALWAYS_DISABLE_MOVE_ROTATION;

    Player_RoundUpInvincibilityTimer(this);

    if (!(this->stateFlags1 & PLAYER_STATE1_IN_CUTSCENE) && (this->genericTimer == 0) && (this->damageEffect != 0)) {
        s16 temp = this->actor.shape.rot.y - this->damageYaw;

        this->currentYaw = this->actor.shape.rot.y = this->damageYaw;
        this->linearVelocity = this->knockbackVelXZ;

        if (ABS(temp) > 0x4000) {
            this->actor.shape.rot.y = this->damageYaw + 0x8000;
        }

        if (this->actor.velocity.y < 0.0f) {
            this->actor.gravity = 0.0f;
            this->actor.velocity.y = 0.0f;
        }
    }

    if (LinkAnimation_Update(globalCtx, &this->skelAnime) && (this->actor.bgCheckFlags & 1)) {
        if (this->genericTimer != 0) {
            this->genericTimer--;
            if (this->genericTimer == 0) {
                Player_SetupStandingStillMorph(this, globalCtx);
            }
        } else if ((this->stateFlags1 & PLAYER_STATE1_IN_CUTSCENE) ||
                   (!(this->cylinder.base.acFlags & AC_HIT) && (this->damageEffect == 0))) {
            if (this->stateFlags1 & PLAYER_STATE1_IN_CUTSCENE) {
                this->genericTimer++;
            } else {
                Player_SetActionFunc(globalCtx, this, Player_DownFromKnockback, 0);
                this->stateFlags1 |= PLAYER_STATE1_TAKING_DAMAGE;
            }

            Player_PlayAnimOnce(globalCtx, this,
                                (this->currentYaw != this->actor.shape.rot.y) ? &gPlayerAnim_002F60
                                                                              : &gPlayerAnim_002DB8);
            Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_FREEZE);
        }
    }

    if (this->actor.bgCheckFlags & 2) {
        Player_PlayMoveSfx(this, NA_SE_PL_BOUND);
    }
}

void Player_DownFromKnockback(Player* this, GlobalContext* globalCtx) {
    this->stateFlags2 |=
        PLAYER_STATE2_DISABLE_MOVE_ROTATION_WHILE_Z_TARGETING | PLAYER_STATE2_ALWAYS_DISABLE_MOVE_ROTATION;
    Player_RoundUpInvincibilityTimer(this);

    Player_StepLinearVelocityToZero(this);

    if (LinkAnimation_Update(globalCtx, &this->skelAnime) && (this->linearVelocity == 0.0f)) {
        if (this->stateFlags1 & PLAYER_STATE1_IN_CUTSCENE) {
            this->genericTimer++;
        } else {
            Player_SetActionFunc(globalCtx, this, Player_GetUpFromKnockback, 0);
            this->stateFlags1 |= PLAYER_STATE1_TAKING_DAMAGE;
        }

        Player_PlayAnimOnceSlowed(
            globalCtx, this, (this->currentYaw != this->actor.shape.rot.y) ? &gPlayerAnim_002F68 : &gPlayerAnim_002DC0);
        this->currentYaw = this->actor.shape.rot.y;
    }
}

static PlayerAnimSfxEntry D_808545DC[] = {
    { 0, 0x4014 },
    { 0, -0x401E },
};

void Player_GetUpFromKnockback(Player* this, GlobalContext* globalCtx) {
    s32 sp24;

    this->stateFlags2 |= PLAYER_STATE2_DISABLE_MOVE_ROTATION_WHILE_Z_TARGETING;
    Player_RoundUpInvincibilityTimer(this);

    if (this->stateFlags1 & PLAYER_STATE1_IN_CUTSCENE) {
        LinkAnimation_Update(globalCtx, &this->skelAnime);
    } else {
        sp24 = Player_IsActionInterrupted(globalCtx, this, &this->skelAnime, 16.0f);
        if ((sp24 != 0) && (LinkAnimation_Update(globalCtx, &this->skelAnime) || (sp24 > 0))) {
            Player_SetupStandingStillType(this, globalCtx);
        }
    }

    Player_PlayAnimSfx(this, D_808545DC);
}

static Vec3f D_808545E4 = { 0.0f, 0.0f, 5.0f };

void func_80843AE8(GlobalContext* globalCtx, Player* this) {
    if (this->genericTimer != 0) {
        if (this->genericTimer > 0) {
            this->genericTimer--;
            if (this->genericTimer == 0) {
                if (this->stateFlags1 & PLAYER_STATE1_SWIMMING) {
                    LinkAnimation_Change(globalCtx, &this->skelAnime, &gPlayerAnim_003328, 1.0f, 0.0f,
                                         Animation_GetLastFrame(&gPlayerAnim_003328), ANIMMODE_ONCE, -16.0f);
                } else {
                    LinkAnimation_Change(globalCtx, &this->skelAnime, &gPlayerAnim_002878, 1.0f, 99.0f,
                                         Animation_GetLastFrame(&gPlayerAnim_002878), ANIMMODE_ONCE, 0.0f);
                }
                if (CVar_GetS32("gFairyReviveEffect", 0)) {
                    if (CVar_GetS32("gFairyRevivePercentRestore", 0)) {
                        gSaveContext.healthAccumulator =
                            (gSaveContext.healthCapacity * CVar_GetS32("gFairyReviveHealth", 100) / 100 + 15) / 16 * 16;
                    } else {
                        gSaveContext.healthAccumulator = CVar_GetS32("gFairyReviveHealth", 20) * 16;
                    }
                } else {
                    gSaveContext.healthAccumulator = 0x140;
                }
                this->genericTimer = -1;
            }
        } else if (gSaveContext.healthAccumulator == 0) {
            this->stateFlags1 &= ~PLAYER_STATE1_IN_DEATH_CUTSCENE;
            if (this->stateFlags1 & PLAYER_STATE1_SWIMMING) {
                Player_SetupSwimIdle(globalCtx, this);
            } else {
                Player_SetupStandingStillMorph(this, globalCtx);
            }
            this->deathTimer = 20;
            Player_SetupInvincibilityNoDamageFlash(this, -20);
            func_800F47FC();
        }
    } else if (this->genericVar != 0) {
        this->genericTimer = 60;
        Player_SpawnFairy(globalCtx, this, &this->actor.world.pos, &D_808545E4, FAIRY_REVIVE_DEATH);
        func_8002F7DC(&this->actor, NA_SE_EV_FIATY_HEAL - SFX_FLAG);
        OnePointCutscene_Init(globalCtx, 9908, 125, &this->actor, MAIN_CAM);
    } else if (globalCtx->gameOverCtx.state == GAMEOVER_DEATH_WAIT_GROUND) {
        globalCtx->gameOverCtx.state = GAMEOVER_DEATH_DELAY_MENU;
        sMaskMemory = PLAYER_MASK_NONE;
    }
}

static PlayerAnimSfxEntry D_808545F0[] = {
    { NA_SE_PL_BOUND, 0x103C },
    { 0, 0x408C },
    { 0, 0x40A4 },
    { 0, -0x40AA },
};

void Player_Die(Player* this, GlobalContext* globalCtx) {
    if (this->currentTunic != PLAYER_TUNIC_GORON && CVar_GetS32("gSuperTunic", 0) == 0) {
        if ((globalCtx->roomCtx.curRoom.unk_02 == 3) || (sFloorSpecialProperty == 9) ||
            ((Player_GetHurtFloorType(sFloorSpecialProperty) >= 0) &&
             !SurfaceType_IsWallDamage(&globalCtx->colCtx, this->actor.floorPoly, this->actor.floorBgId))) {
            Player_StartBurning(this);
        }
    }

    Player_StepLinearVelocityToZero(this);

    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        if (this->actor.category == ACTORCAT_PLAYER) {
            func_80843AE8(globalCtx, this);
        }
        return;
    }

    if (this->skelAnime.animation == &gPlayerAnim_002878) {
        Player_PlayAnimSfx(this, D_808545F0);
    } else if (this->skelAnime.animation == &gPlayerAnim_002F08) {
        if (LinkAnimation_OnFrame(&this->skelAnime, 88.0f)) {
            Player_PlayMoveSfx(this, NA_SE_PL_BOUND);
        }
    }
}

void func_80843E14(Player* this, u16 sfxId) {
    Player_PlayVoiceSfxForAge(this, sfxId);

    if ((this->heldActor != NULL) && (this->heldActor->id == ACTOR_EN_RU1)) {
        Audio_PlayActorSound2(this->heldActor, NA_SE_VO_RT_FALL);
    }
}

static FallImpactInfo D_80854600[] = {
    { -8, 180, 40, 100, NA_SE_VO_LI_LAND_DAMAGE_S },
    { -16, 255, 140, 150, NA_SE_VO_LI_LAND_DAMAGE_S },
};

s32 func_80843E64(GlobalContext* globalCtx, Player* this) {
    s32 sp34;

    if ((sFloorSpecialProperty == 6) || (sFloorSpecialProperty == 9)) {
        sp34 = 0;
    } else {
        sp34 = this->fallDistance;
    }

    Math_StepToF(&this->linearVelocity, 0.0f, 1.0f);

    this->stateFlags1 &= ~(PLAYER_STATE1_JUMPING | PLAYER_STATE1_FREEFALLING);

    if (sp34 >= 400) {
        s32 impactIndex;
        FallImpactInfo* impactInfo;

        if (this->fallDistance < 800) {
            impactIndex = 0;
        } else {
            impactIndex = 1;
        }

        impactInfo = &D_80854600[impactIndex];

        if (Player_InflictDamageModified(globalCtx, impactInfo->damage * CVar_GetS32("gFallDamageMul", 1), false)) {
            return -1;
        }

        Player_SetupInvincibility(this, 40);
        func_808429B4(globalCtx, 32967, 2, 30);
        Player_RequestRumble(this, impactInfo->unk_01, impactInfo->unk_02, impactInfo->unk_03, 0);
        func_8002F7DC(&this->actor, NA_SE_PL_BODY_HIT);
        Player_PlayVoiceSfxForAge(this, impactInfo->sfxId);

        return impactIndex + 1;
    }

    if (sp34 > 200) {
        sp34 *= 2;

        if (sp34 > 255) {
            sp34 = 255;
        }

        Player_RequestRumble(this, (u8)sp34, (u8)(sp34 * 0.1f), (u8)sp34, 0);

        if (sFloorSpecialProperty == 6) {
            Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_CLIMB_END);
        }
    }

    Player_PlayLandingSfx(this);

    return 0;
}

void func_8084409C(GlobalContext* globalCtx, Player* this, f32 speedXZ, f32 velocityY) {
    Actor* heldActor = this->heldActor;

    if (!Player_InterruptHoldingActor(globalCtx, this, heldActor)) {
        heldActor->world.rot.y = this->actor.shape.rot.y;
        heldActor->speedXZ = speedXZ;
        heldActor->velocity.y = velocityY;
        Player_SetupHeldItemUpperActionFunc(globalCtx, this);
        func_8002F7DC(&this->actor, NA_SE_PL_THROW);
        Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_SWORD_N);
    }
}

void Player_UpdateMidair(Player* this, GlobalContext* globalCtx) {
    f32 sp4C;
    s16 sp4A;

    if (gSaveContext.respawn[RESPAWN_MODE_TOP].data > 40) {
        this->actor.gravity = 0.0f;
    } else if (Player_IsUnfriendlyZTargeting(this)) {
        this->actor.gravity = -1.2f;
    }

    Player_GetTargetVelocityAndYaw(this, &sp4C, &sp4A, 0.0f, globalCtx);

    if (!(this->actor.bgCheckFlags & 1)) {
        if (this->stateFlags1 & PLAYER_STATE1_HOLDING_ACTOR) {
            Actor* heldActor = this->heldActor;

            u16 buttonsToCheck = BTN_A | BTN_B | BTN_CLEFT | BTN_CRIGHT | BTN_CDOWN;
            if (CVar_GetS32("gDpadEquips", 0) != 0) {
                buttonsToCheck |= BTN_DUP | BTN_DDOWN | BTN_DLEFT | BTN_DRIGHT;
            }
            if (!Player_InterruptHoldingActor(globalCtx, this, heldActor) && (heldActor->id == ACTOR_EN_NIW) &&
                CHECK_BTN_ANY(sControlInput->press.button, buttonsToCheck)) {
                func_8084409C(globalCtx, this, this->linearVelocity + 2.0f, this->actor.velocity.y + 2.0f);
            }
        }

        LinkAnimation_Update(globalCtx, &this->skelAnime);

        if (!(this->stateFlags2 & PLAYER_STATE2_BACKFLIPPING_OR_SIDEHOPPING)) {
            func_8083DFE0(this, &sp4C, &sp4A);
        }

        Player_SetupCurrentUpperAction(this, globalCtx);

        if (((this->stateFlags2 & PLAYER_STATE2_BACKFLIPPING_OR_SIDEHOPPING) && (this->genericVar == 2)) ||
            !Player_SetupMidairJumpSlash(this, globalCtx)) {
            if (this->actor.velocity.y < 0.0f) {
                if (this->genericTimer >= 0) {
                    if ((this->actor.bgCheckFlags & 8) || (this->genericTimer == 0) || (this->fallDistance > 0)) {
                        if ((sPlayerYDistToFloor > 800.0f) || (this->stateFlags1 & PLAYER_STATE1_END_HOOKSHOT_MOVE)) {
                            func_80843E14(this, NA_SE_VO_LI_FALL_S);
                            this->stateFlags1 &= ~PLAYER_STATE1_END_HOOKSHOT_MOVE;
                        }

                        LinkAnimation_Change(globalCtx, &this->skelAnime, &gPlayerAnim_003020, 1.0f, 0.0f, 0.0f,
                                             ANIMMODE_ONCE, 8.0f);
                        this->genericTimer = -1;
                    }
                } else {
                    if ((this->genericTimer == -1) && (this->fallDistance > 120.0f) && (sPlayerYDistToFloor > 280.0f)) {
                        this->genericTimer = -2;
                        func_80843E14(this, NA_SE_VO_LI_FALL_L);
                    }

                    if ((this->actor.bgCheckFlags & 0x200) &&
                        !(this->stateFlags2 & PLAYER_STATE2_BACKFLIPPING_OR_SIDEHOPPING) &&
                        !(this->stateFlags1 & (PLAYER_STATE1_HOLDING_ACTOR | PLAYER_STATE1_SWIMMING)) &&
                        (this->linearVelocity > 0.0f)) {
                        if ((this->wallHeight >= 150.0f) &&
                            (this->relativeAnalogStickInputs[this->inputFrameCounter] == 0)) {
                            func_8083EC18(this, globalCtx, sTouchedWallFlags);
                        } else if ((this->touchedWallJumpType >= 2) && (this->wallHeight < 150.0f) &&
                                   (((this->actor.world.pos.y - this->actor.floorHeight) + this->wallHeight) >
                                    (70.0f * this->ageProperties->unk_08))) {
                            AnimationContext_DisableQueue(globalCtx);
                            if (this->stateFlags1 & PLAYER_STATE1_END_HOOKSHOT_MOVE) {
                                Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_HOOKSHOT_HANG);
                            } else {
                                Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_HANG);
                            }
                            this->actor.world.pos.y += this->wallHeight;
                            Player_SetupGrabLedge(globalCtx, this, this->actor.wallPoly, this->wallDistance,
                                GET_PLAYER_ANIM(PLAYER_ANIMGROUP_HANGING_FROM_LEDGE, this->modelAnimType));
                            this->actor.shape.rot.y = this->currentYaw += 0x8000;
                            this->stateFlags1 |= PLAYER_STATE1_HANGING_FROM_LEDGE_SLIP;
                        }
                    }
                }
            }
        }
    } else {
        LinkAnimationHeader* anim = GET_PLAYER_ANIM(PLAYER_ANIMGROUP_TALL_JUMP_LANDING, this->modelAnimType);
        s32 sp3C;

        if (this->stateFlags2 & PLAYER_STATE2_BACKFLIPPING_OR_SIDEHOPPING) {
            if (Player_IsUnfriendlyZTargeting(this)) {
                anim = D_80853D4C[this->genericVar][2];
            } else {
                anim = D_80853D4C[this->genericVar][1];
            }
        } else if (this->skelAnime.animation == &gPlayerAnim_003148) {
            anim = &gPlayerAnim_003150;
        } else if (Player_IsUnfriendlyZTargeting(this)) {
            anim = &gPlayerAnim_002538;
            Player_ResetLeftRightBlendWeight(this);
        } else if (this->fallDistance <= 80) {
            anim = GET_PLAYER_ANIM(PLAYER_ANIMGROUP_SHORT_JUMP_LANDING, this->modelAnimType);
        } else if ((this->fallDistance < 800) && (this->relativeAnalogStickInputs[this->inputFrameCounter] == 0) &&
                   !(this->stateFlags1 & PLAYER_STATE1_HOLDING_ACTOR)) {
            Player_SetupRolling(this, globalCtx);
            return;
        }

        sp3C = func_80843E64(globalCtx, this);

        if (sp3C > 0) {
            Player_SetupReturnToStandStillSetAnim(
                this, GET_PLAYER_ANIM(PLAYER_ANIMGROUP_TALL_JUMP_LANDING, this->modelAnimType), globalCtx);
            this->skelAnime.endFrame = 8.0f;
            if (sp3C == 1) {
                this->genericTimer = 10;
            } else {
                this->genericTimer = 20;
            }
        } else if (sp3C == 0) {
            Player_SetupReturnToStandStillSetAnim(this, anim, globalCtx);
        }
    }
}

static PlayerAnimSfxEntry D_8085460C[] = {
    { NA_SE_VO_LI_SWORD_N, 0x2001 },
    { NA_SE_PL_WALK_GROUND, 0x1806 },
    { NA_SE_PL_ROLL, 0x806 },
    { 0, -0x2812 },
};

void Player_Rolling(Player* this, GlobalContext* globalCtx) {
    Actor* cylinderOc;
    s32 temp;
    s32 sp44;
    DynaPolyActor* wallPolyActor;
    s32 pad;
    f32 targetVelocity;
    s16 targetYaw;

    this->stateFlags2 |= PLAYER_STATE2_DISABLE_MOVE_ROTATION_WHILE_Z_TARGETING;

    cylinderOc = NULL;
    sp44 = LinkAnimation_Update(globalCtx, &this->skelAnime);

    if (LinkAnimation_OnFrame(&this->skelAnime, 8.0f)) {
        Player_SetupInvincibilityNoDamageFlash(this, -10);
    }

    if (func_80842964(this, globalCtx) == 0) {
        if (this->genericTimer != 0) {
            if (!CVar_GetS32("gSonicRoll", 0)) {
                Math_StepToF(&this->linearVelocity, 0.0f, 2.0f);
            }

            temp = Player_IsActionInterrupted(globalCtx, this, &this->skelAnime, 5.0f);
            if ((temp != 0) && ((temp > 0) || sp44)) {
                Player_SetupReturnToStandStill(this, globalCtx);
            }
        } else {
            if (this->linearVelocity >= 7.0f) {
                if (((this->actor.bgCheckFlags & 0x200) && (sYawToTouchedWall2 < 0x2000)) ||
                    ((this->cylinder.base.ocFlags1 & OC1_HIT) &&
                     (cylinderOc = this->cylinder.base.oc,
                      ((cylinderOc->id == ACTOR_EN_WOOD02) &&
                       (ABS((s16)(this->actor.world.rot.y - cylinderOc->yawTowardsPlayer)) > 0x6000))))) {

                    if (cylinderOc != NULL) {
                        cylinderOc->home.rot.y = 1;
                    } else if (this->actor.wallBgId != BGCHECK_SCENE) {
                        wallPolyActor = DynaPoly_GetActor(&globalCtx->colCtx, this->actor.wallBgId);
                        if ((wallPolyActor != NULL) && (wallPolyActor->actor.id == ACTOR_OBJ_KIBAKO2)) {
                            wallPolyActor->actor.home.rot.z = 1;
                        }
                    }

                    Player_PlayAnimOnce(globalCtx, this,
                                        GET_PLAYER_ANIM(PLAYER_ANIMGROUP_ROLL_BONKING, this->modelAnimType));
                    this->linearVelocity = -this->linearVelocity;
                    func_808429B4(globalCtx, 33267, 3, 12);
                    Player_RequestRumble(this, 255, 20, 150, 0);
                    func_8002F7DC(&this->actor, NA_SE_PL_BODY_HIT);
                    Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_CLIMB_END);
                    this->genericTimer = 1;
                    return;
                }
            }

            if ((this->skelAnime.curFrame < 15.0f) || !Player_SetupStartMeleeWeaponAttack(this, globalCtx)) {
                if (this->skelAnime.curFrame >= 20.0f) {
                    if (CVar_GetS32("gSonicRoll", 0)) {
                        Player_SetupRolling(this, globalCtx);
                    } else {
                        Player_SetupReturnToStandStill(this, globalCtx);
                    }
                    return;
                }

                Player_GetTargetVelocityAndYaw(this, &targetVelocity, &targetYaw, 0.018f, globalCtx);

                targetVelocity *= 1.5f;
                if ((targetVelocity < 3.0f) || (this->relativeAnalogStickInputs[this->inputFrameCounter] != 0)) {
                    targetVelocity = 3.0f;
                }

                if (CVar_GetS32("gSonicRoll", 0)) {
                    targetVelocity *= 4.5f;
                    Player_SetRunVelocityAndYaw(this, targetVelocity, targetYaw);
                    if (this->linearVelocity < 30.0f) {
                        this->linearVelocity = 30.0f;
                    }
                }
                else {
                    Player_SetRunVelocityAndYaw(this, targetVelocity, this->actor.shape.rot.y);
                }

                if (Player_SetupSpawnDustAtFeet(globalCtx, this)) {
                    func_8002F8F0(&this->actor, NA_SE_PL_ROLL_DUST - SFX_FLAG);
                }

                Player_PlayAnimSfx(this, D_8085460C);
            }
        }
    }
}

void Player_FallingDive(Player* this, GlobalContext* globalCtx) {
    this->stateFlags2 |= PLAYER_STATE2_DISABLE_MOVE_ROTATION_WHILE_Z_TARGETING;

    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        Player_PlayAnimLoop(globalCtx, this, &gPlayerAnim_003160);
    }

    Math_StepToF(&this->linearVelocity, 0.0f, 0.05f);

    if (this->actor.bgCheckFlags & 1) {
        this->actor.colChkInfo.damage = 0x10;
        Player_SetupDamage(globalCtx, this, 1, 4.0f, 5.0f, this->actor.shape.rot.y, 20);
    }
}

void Player_JumpSlash(Player* this, GlobalContext* globalCtx) {
    f32 sp2C;
    s16 sp2A;

    this->stateFlags2 |= PLAYER_STATE2_DISABLE_MOVE_ROTATION_WHILE_Z_TARGETING;

    this->actor.gravity = -1.2f;
    LinkAnimation_Update(globalCtx, &this->skelAnime);

    if (!func_80842DF4(globalCtx, this)) {
        func_8084285C(this, 6.0f, 7.0f, 99.0f);

        if (!(this->actor.bgCheckFlags & 1)) {
            Player_GetTargetVelocityAndYaw(this, &sp2C, &sp2A, 0.0f, globalCtx);
            func_8083DFE0(this, &sp2C, &this->currentYaw);
            return;
        }

        if (func_80843E64(globalCtx, this) >= 0) {
            this->swordAnimation += 2;
            Player_StartMeleeWeaponAttack(globalCtx, this, this->swordAnimation);
            this->slashCounter = 3;
            Player_PlayLandingSfx(this);
        }
    }
}

s32 func_80844BE4(Player* this, GlobalContext* globalCtx) {
    s32 temp;

    if (Player_SetupCutscene(globalCtx, this)) {
        this->stateFlags2 |= PLAYER_STATE2_RELEASING_SPIN_ATTACK;
    } else {
        if (!CHECK_BTN_ALL(sControlInput->cur.button, BTN_B)) {
            if ((this->unk_858 >= 0.85f) || Player_CanQuickspin(this)) {
                temp = D_80854384[Player_HoldsTwoHandedWeapon(this)];
            } else {
                temp = D_80854380[Player_HoldsTwoHandedWeapon(this)];
            }

            Player_StartMeleeWeaponAttack(globalCtx, this, temp);
            Player_SetupInvincibilityNoDamageFlash(this, -8);

            this->stateFlags2 |= PLAYER_STATE2_RELEASING_SPIN_ATTACK;
            if (this->relativeAnalogStickInputs[this->inputFrameCounter] == 0) {
                this->stateFlags2 |= PLAYER_STATE2_ENABLE_FORWARD_SLIDE_FROM_ATTACK;
            }
        } else {
            return 0;
        }
    }

    return 1;
}

void func_80844CF8(Player* this, GlobalContext* globalCtx) {
    Player_SetActionFunc(globalCtx, this, Player_WalkChargingSpinAttack, 1);
}

void func_80844D30(Player* this, GlobalContext* globalCtx) {
    Player_SetActionFunc(globalCtx, this, Player_SidewalkChargingSpinAttack, 1);
}

void func_80844D68(Player* this, GlobalContext* globalCtx) {
    Player_ReturnToStandStill(this, globalCtx);
    Player_InactivateMeleeWeapon(this);
    Player_ChangeAnimMorphToLastFrame(globalCtx, this, sCancelSpinAttackChargeAnims[Player_HoldsTwoHandedWeapon(this)]);
    this->currentYaw = this->actor.shape.rot.y;
}

void func_80844DC8(Player* this, GlobalContext* globalCtx) {
    Player_SetActionFunc(globalCtx, this, Player_ChargeSpinAttack, 1);
    this->walkFrame = 0.0f;
    Player_PlayAnimLoop(globalCtx, this, sSpinAttackChargeAnims[Player_HoldsTwoHandedWeapon(this)]);
    this->genericTimer = 1;
}

void func_80844E3C(Player* this) {
    Math_StepToF(&this->unk_858, 1.0f, 0.02f);
}

void Player_ChargeSpinAttack(Player* this, GlobalContext* globalCtx) {
    f32 sp34;
    s16 sp32;
    s32 temp;

    this->stateFlags1 |= PLAYER_STATE1_CHARGING_SPIN_ATTACK;

    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        Player_EndAnimMovement(this);
        Player_SetZTargetFriendlyYaw(this);
        this->stateFlags1 &= ~PLAYER_STATE1_Z_TARGETING_FRIENDLY;
        Player_PlayAnimLoop(globalCtx, this, sSpinAttackChargeAnims[Player_HoldsTwoHandedWeapon(this)]);
        this->genericTimer = -1;
    }

    Player_StepLinearVelocityToZero(this);

    if (!func_80842964(this, globalCtx) && (this->genericTimer != 0)) {
        func_80844E3C(this);

        if (this->genericTimer < 0) {
            if (this->unk_858 >= 0.1f) {
                this->slashCounter = 0;
                this->genericTimer = 1;
            } else if (!CHECK_BTN_ALL(sControlInput->cur.button, BTN_B)) {
                func_80844D68(this, globalCtx);
            }
        } else if (!func_80844BE4(this, globalCtx)) {
            Player_GetTargetVelocityAndYaw(this, &sp34, &sp32, 0.0f, globalCtx);

            temp = func_80840058(this, &sp34, &sp32, globalCtx);
            if (temp > 0) {
                func_80844CF8(this, globalCtx);
            } else if (temp < 0) {
                func_80844D30(this, globalCtx);
            }
        }
    }
}

void Player_WalkChargingSpinAttack(Player* this, GlobalContext* globalCtx) {
    s16 temp1;
    s32 temp2;
    f32 sp5C;
    f32 sp58;
    f32 sp54;
    s16 sp52;
    s32 temp4;
    s16 temp5;
    s32 sp44;

    temp1 = this->currentYaw - this->actor.shape.rot.y;
    temp2 = ABS(temp1);

    sp5C = fabsf(this->linearVelocity);
    sp58 = sp5C * 1.5f;

    this->stateFlags1 |= PLAYER_STATE1_CHARGING_SPIN_ATTACK;

    if (sp58 < 1.5f) {
        sp58 = 1.5f;
    }

    sp58 = ((temp2 < 0x4000) ? -1.0f : 1.0f) * sp58;

    Player_SetupWalkSfx(this, sp58);

    sp58 = CLAMP(sp5C * 0.5f, 0.5f, 1.0f);

    LinkAnimation_BlendToJoint(globalCtx, &this->skelAnime, sSpinAttackChargeAnims[Player_HoldsTwoHandedWeapon(this)],
                               0.0f, sSpinAttackChargeWalkAnims[Player_HoldsTwoHandedWeapon(this)],
                               this->walkFrame * (21.0f / 29.0f), sp58, this->blendTable);

    if (!func_80842964(this, globalCtx) && !func_80844BE4(this, globalCtx)) {
        func_80844E3C(this);
        Player_GetTargetVelocityAndYaw(this, &sp54, &sp52, 0.0f, globalCtx);

        temp4 = func_80840058(this, &sp54, &sp52, globalCtx);

        if (temp4 < 0) {
            func_80844D30(this, globalCtx);
            return;
        }

        if (temp4 == 0) {
            sp54 = 0.0f;
            sp52 = this->currentYaw;
        }

        temp5 = sp52 - this->currentYaw;
        sp44 = ABS(temp5);

        if (sp44 > 0x4000) {
            if (Math_StepToF(&this->linearVelocity, 0.0f, 1.0f)) {
                this->currentYaw = sp52;
            }
            return;
        }

        Math_AsymStepToF(&this->linearVelocity, sp54 * 0.2f, 1.0f, 0.5f);
        Math_ScaledStepToS(&this->currentYaw, sp52, sp44 * 0.1f);

        if ((sp54 == 0.0f) && (this->linearVelocity == 0.0f)) {
            func_80844DC8(this, globalCtx);
        }
    }
}

void Player_SidewalkChargingSpinAttack(Player* this, GlobalContext* globalCtx) {
    f32 sp5C;
    f32 sp58;
    f32 sp54;
    s16 sp52;
    s32 temp4;
    s16 temp5;
    s32 sp44;

    sp5C = fabsf(this->linearVelocity);

    this->stateFlags1 |= PLAYER_STATE1_CHARGING_SPIN_ATTACK;

    if (sp5C == 0.0f) {
        sp5C = ABS(this->unk_87C) * 0.0015f;
        if (sp5C < 400.0f) {
            sp5C = 0.0f;
        }
        Player_SetupWalkSfx(this, ((this->unk_87C >= 0) ? 1 : -1) * sp5C);
    } else {
        sp58 = sp5C * 1.5f;
        if (sp58 < 1.5f) {
            sp58 = 1.5f;
        }
        Player_SetupWalkSfx(this, sp58);
    }

    sp58 = CLAMP(sp5C * 0.5f, 0.5f, 1.0f);

    LinkAnimation_BlendToJoint(globalCtx, &this->skelAnime, sSpinAttackChargeAnims[Player_HoldsTwoHandedWeapon(this)],
                               0.0f, sSpinAttackChargeSidewalkAnims[Player_HoldsTwoHandedWeapon(this)],
                               this->walkFrame * (21.0f / 29.0f), sp58, this->blendTable);

    if (!func_80842964(this, globalCtx) && !func_80844BE4(this, globalCtx)) {
        func_80844E3C(this);
        Player_GetTargetVelocityAndYaw(this, &sp54, &sp52, 0.0f, globalCtx);

        temp4 = func_80840058(this, &sp54, &sp52, globalCtx);

        if (temp4 > 0) {
            func_80844CF8(this, globalCtx);
            return;
        }

        if (temp4 == 0) {
            sp54 = 0.0f;
            sp52 = this->currentYaw;
        }

        temp5 = sp52 - this->currentYaw;
        sp44 = ABS(temp5);

        if (sp44 > 0x4000) {
            if (Math_StepToF(&this->linearVelocity, 0.0f, 1.0f)) {
                this->currentYaw = sp52;
            }
            return;
        }

        Math_AsymStepToF(&this->linearVelocity, sp54 * 0.2f, 1.0f, 0.5f);
        Math_ScaledStepToS(&this->currentYaw, sp52, sp44 * 0.1f);

        if ((sp54 == 0.0f) && (this->linearVelocity == 0.0f) && (sp5C == 0.0f)) {
            func_80844DC8(this, globalCtx);
        }
    }
}

void Player_JumpUpToLedge(Player* this, GlobalContext* globalCtx) {
    s32 sp3C;
    f32 temp1;
    s32 temp2;
    f32 temp3;

    this->stateFlags2 |= PLAYER_STATE2_DISABLE_MOVE_ROTATION_WHILE_Z_TARGETING;
    sp3C = LinkAnimation_Update(globalCtx, &this->skelAnime);

    if (this->skelAnime.animation == &gPlayerAnim_002D48) {
        this->linearVelocity = 1.0f;

        if (LinkAnimation_OnFrame(&this->skelAnime, 8.0f)) {
            temp1 = this->wallHeight;

            if (temp1 > this->ageProperties->unk_0C) {
                temp1 = this->ageProperties->unk_0C;
            }

            if (this->stateFlags1 & PLAYER_STATE1_SWIMMING) {
                temp1 *= 0.085f;
            } else {
                temp1 *= 0.072f;
            }

            if (!LINK_IS_ADULT) {
                temp1 += 1.0f;
            }

            Player_SetupJumpWithSfx(this, NULL, temp1, globalCtx, NA_SE_VO_LI_AUTO_JUMP);
            this->genericTimer = -1;
            return;
        }
    } else {
        temp2 = Player_IsActionInterrupted(globalCtx, this, &this->skelAnime, 4.0f);

        if (temp2 == 0) {
            this->stateFlags1 &= ~(PLAYER_STATE1_CLIMBING_ONTO_LEDGE | PLAYER_STATE1_JUMPING);
            return;
        }

        if ((sp3C != 0) || (temp2 > 0)) {
            Player_SetupStandingStillNoMorph(this, globalCtx);
            this->stateFlags1 &= ~(PLAYER_STATE1_CLIMBING_ONTO_LEDGE | PLAYER_STATE1_JUMPING);
            return;
        }

        temp3 = 0.0f;

        if (this->skelAnime.animation == &gPlayerAnim_0032E8) {
            if (LinkAnimation_OnFrame(&this->skelAnime, 30.0f)) {
                Player_StartJumpOutOfWater(globalCtx, this, 10.0f);
            }
            temp3 = 50.0f;
        } else if (this->skelAnime.animation == &gPlayerAnim_002D40) {
            temp3 = 30.0f;
        } else if (this->skelAnime.animation == &gPlayerAnim_002D38) {
            temp3 = 16.0f;
        }

        if (LinkAnimation_OnFrame(&this->skelAnime, temp3)) {
            Player_PlayLandingSfx(this);
            Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_CLIMB_END);
        }

        if ((this->skelAnime.animation == &gPlayerAnim_002D38) || (this->skelAnime.curFrame > 5.0f)) {
            if (this->genericTimer == 0) {
                Player_PlayJumpSfx(this);
                this->genericTimer = 1;
            }
            Math_StepToF(&this->actor.shape.yOffset, 0.0f, 150.0f);
        }
    }
}

void Player_RunMiniCutsceneFunc(Player* this, GlobalContext* globalCtx) {
    this->stateFlags2 |=
        PLAYER_STATE2_DISABLE_MOVE_ROTATION_WHILE_Z_TARGETING | PLAYER_STATE2_ALWAYS_DISABLE_MOVE_ROTATION;
    LinkAnimation_Update(globalCtx, &this->skelAnime);

    if (((this->stateFlags1 & PLAYER_STATE1_HOLDING_ACTOR) && (this->heldActor != NULL) &&
         (this->getItemId == GI_NONE)) ||
        !Player_SetupCurrentUpperAction(this, globalCtx)) {
        this->miniCsFunc(globalCtx, this);
    }
}

s32 func_80845964(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2, f32 arg3, s16 arg4, s32 arg5) {
    if ((arg5 != 0) && (this->linearVelocity == 0.0f)) {
        return LinkAnimation_Update(globalCtx, &this->skelAnime);
    }

    if (arg5 != 2) {
        f32 sp34 = R_UPDATE_RATE * 0.5f;
        f32 selfDistX = arg2->endPos.x - this->actor.world.pos.x;
        f32 selfDistZ = arg2->endPos.z - this->actor.world.pos.z;
        f32 sp28 = sqrtf(SQ(selfDistX) + SQ(selfDistZ)) / sp34;
        s32 sp24 = (arg2->endFrame - globalCtx->csCtx.frames) + 1;

        arg4 = Math_Atan2S(selfDistZ, selfDistX);

        if (arg5 == 1) {
            f32 distX = arg2->endPos.x - arg2->startPos.x;
            f32 distZ = arg2->endPos.z - arg2->startPos.z;
            s32 temp = (((sqrtf(SQ(distX) + SQ(distZ)) / sp34) / (arg2->endFrame - arg2->startFrame)) / 1.5f) * 4.0f;

            if (temp >= sp24) {
                arg4 = this->actor.shape.rot.y;
                arg3 = 0.0f;
            } else {
                arg3 = sp28 / ((sp24 - temp) + 1);
            }
        } else {
            arg3 = sp28 / sp24;
        }
    }

    this->stateFlags2 |= PLAYER_STATE2_DISABLE_MOVE_ROTATION_WHILE_Z_TARGETING;
    func_80841EE4(this, globalCtx);
    Player_SetRunVelocityAndYaw(this, arg3, arg4);

    if ((arg3 == 0.0f) && (this->linearVelocity == 0.0f)) {
        Player_EndRun(this, globalCtx);
    }

    return 0;
}

s32 func_80845BA0(GlobalContext* arg0, Player* arg1, f32* arg2, s32 arg3) {
    f32 dx = arg1->csStartPos.x - arg1->actor.world.pos.x;
    f32 dz = arg1->csStartPos.z - arg1->actor.world.pos.z;
    s32 sp2C = sqrtf(SQ(dx) + SQ(dz));
    s16 yaw = Math_Vec3f_Yaw(&arg1->actor.world.pos, &arg1->csStartPos);

    if (sp2C < arg3) {
        *arg2 = 0.0f;
        yaw = arg1->actor.shape.rot.y;
    }

    if (func_80845964(arg0, arg1, NULL, *arg2, yaw, 2)) {
        return 0;
    }

    return sp2C;
}

s32 func_80845C68(GlobalContext* globalCtx, s32 arg1) {
    if (arg1 == 0) {
        Gameplay_SetupRespawnPoint(globalCtx, RESPAWN_MODE_DOWN, 0xDFF);
    }
    gSaveContext.respawn[RESPAWN_MODE_DOWN].data = 0;
    return arg1;
}

void Player_MiniCsMovement(Player* this, GlobalContext* globalCtx) {
    f32 sp3C;
    s32 temp;
    f32 sp34;
    s32 sp30;
    s32 pad;

    if (!Player_SetupItemCutsceneOrFirstPerson(this, globalCtx)) {
        if (this->genericTimer == 0) {
            LinkAnimation_Update(globalCtx, &this->skelAnime);

            if (DECR(this->doorTimer) == 0) {
                this->linearVelocity = 0.1f;
                this->genericTimer = 1;
            }
        } else if (this->genericVar == 0) {
            sp3C = 5.0f * sWaterSpeedScale;

            if (func_80845BA0(globalCtx, this, &sp3C, -1) < 30) {
                this->genericVar = 1;
                this->stateFlags1 |= PLAYER_STATE1_IN_CUTSCENE;

                this->csStartPos.x = this->csEndPos.x;
                this->csStartPos.z = this->csEndPos.z;
            }
        } else {
            sp34 = 5.0f;
            sp30 = 20;

            if (this->stateFlags1 & PLAYER_STATE1_EXITING_SCENE) {
                sp34 = gSaveContext.entranceSpeed;

                if (sConveyorSpeedIndex != 0) {
                    this->csStartPos.x = (Math_SinS(sConveyorYaw) * 400.0f) + this->actor.world.pos.x;
                    this->csStartPos.z = (Math_CosS(sConveyorYaw) * 400.0f) + this->actor.world.pos.z;
                }
            } else if (this->genericTimer < 0) {
                this->genericTimer++;

                sp34 = gSaveContext.entranceSpeed;
                sp30 = -1;
            }

            temp = func_80845BA0(globalCtx, this, &sp34, sp30);

            if ((this->genericTimer == 0) ||
                ((temp == 0) && (this->linearVelocity == 0.0f) && (Gameplay_GetCamera(globalCtx, 0)->unk_14C & 0x10))) {

                func_8005B1A4(Gameplay_GetCamera(globalCtx, 0));
                func_80845C68(globalCtx, gSaveContext.respawn[RESPAWN_MODE_DOWN].data);

                if (!Player_SetupSpeakOrCheck(this, globalCtx)) {
                    Player_EndMiniCsMovement(this, globalCtx);
                }
            }
        }
    }

    if (this->stateFlags1 & PLAYER_STATE1_HOLDING_ACTOR) {
        Player_SetupCurrentUpperAction(this, globalCtx);
    }
}

void Player_OpenDoor(Player* this, GlobalContext* globalCtx) {
    s32 sp2C;

    this->stateFlags2 |= PLAYER_STATE2_DISABLE_MOVE_ROTATION_WHILE_Z_TARGETING;
    sp2C = LinkAnimation_Update(globalCtx, &this->skelAnime);

    Player_SetupCurrentUpperAction(this, globalCtx);

    if (sp2C) {
        if (this->genericTimer == 0) {
            if (DECR(this->doorTimer) == 0) {
                this->genericTimer = 1;
                this->skelAnime.endFrame = this->skelAnime.animLength - 1.0f;
            }
        } else {
            Player_SetupStandingStillNoMorph(this, globalCtx);
            if (globalCtx->roomCtx.prevRoom.num >= 0) {
                func_80097534(globalCtx, &globalCtx->roomCtx);
            }
            func_8005B1A4(Gameplay_GetCamera(globalCtx, 0));
            Gameplay_SetupRespawnPoint(globalCtx, 0, 0xDFF);
        }
        return;
    }

    if (!(this->stateFlags1 & PLAYER_STATE1_IN_CUTSCENE) && LinkAnimation_OnFrame(&this->skelAnime, 15.0f)) {
        globalCtx->func_11D54(this, globalCtx);
    }
}

void Player_LiftActor(Player* this, GlobalContext* globalCtx) {
    Player_StepLinearVelocityToZero(this);

    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        Player_SetupStandingStillType(this, globalCtx);
        Player_SetupHoldActorUpperAction(this, globalCtx);
        return;
    }

    if (LinkAnimation_OnFrame(&this->skelAnime, 4.0f)) {
        Actor* interactRangeActor = this->interactRangeActor;

        if (!Player_InterruptHoldingActor(globalCtx, this, interactRangeActor)) {
            this->heldActor = interactRangeActor;
            this->actor.child = interactRangeActor;
            interactRangeActor->parent = &this->actor;
            interactRangeActor->bgCheckFlags &= 0xFF00;
            this->leftHandRot.y = interactRangeActor->shape.rot.y - this->actor.shape.rot.y;
        }
        return;
    }

    Math_ScaledStepToS(&this->leftHandRot.y, 0, 4000);
}

static PlayerAnimSfxEntry D_8085461C[] = {
    { NA_SE_VO_LI_SWORD_L, 0x2031 },
    { NA_SE_VO_LI_SWORD_N, -0x20E6 },
};

void Player_ThrowStonePillar(Player* this, GlobalContext* globalCtx) {
    if (LinkAnimation_Update(globalCtx, &this->skelAnime) && (this->genericTimer++ > 20)) {
        if (!Player_SetupItemCutsceneOrFirstPerson(this, globalCtx)) {
            Player_SetupReturnToStandStillSetAnim(this, &gPlayerAnim_002FA0, globalCtx);
        }
        return;
    }

    if (LinkAnimation_OnFrame(&this->skelAnime, 41.0f)) {
        BgHeavyBlock* heavyBlock = (BgHeavyBlock*)this->interactRangeActor;

        this->heldActor = &heavyBlock->dyna.actor;
        this->actor.child = &heavyBlock->dyna.actor;
        heavyBlock->dyna.actor.parent = &this->actor;
        func_8002DBD0(&heavyBlock->dyna.actor, &heavyBlock->unk_164, &this->leftHandPos);
        return;
    }

    if (LinkAnimation_OnFrame(&this->skelAnime, 229.0f)) {
        Actor* heldActor = this->heldActor;

        heldActor->speedXZ = Math_SinS(heldActor->shape.rot.x) * 40.0f;
        heldActor->velocity.y = Math_CosS(heldActor->shape.rot.x) * 40.0f;
        heldActor->gravity = -2.0f;
        heldActor->minVelocityY = -30.0f;
        Player_DetatchHeldActor(globalCtx, this);
        return;
    }

    Player_PlayAnimSfx(this, D_8085461C);
}

void Player_LiftSilverBoulder(Player* this, GlobalContext* globalCtx) {
    Player_StepLinearVelocityToZero(this);

    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        Player_PlayAnimLoop(globalCtx, this, &gPlayerAnim_0032C0);
        this->genericTimer = 1;
        return;
    }

    u16 buttonsToCheck = BTN_A | BTN_B | BTN_CLEFT | BTN_CRIGHT | BTN_CDOWN;
    if (CVar_GetS32("gDpadEquips", 0) != 0) {
        buttonsToCheck |= BTN_DUP | BTN_DDOWN | BTN_DLEFT | BTN_DRIGHT;
    }
    if (this->genericTimer == 0) {
        if (LinkAnimation_OnFrame(&this->skelAnime, 27.0f)) {
            Actor* interactRangeActor = this->interactRangeActor;

            this->heldActor = interactRangeActor;
            this->actor.child = interactRangeActor;
            interactRangeActor->parent = &this->actor;
            return;
        }

        if (LinkAnimation_OnFrame(&this->skelAnime, 25.0f)) {
            Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_SWORD_L);
            return;
        }

    } else if (CHECK_BTN_ANY(sControlInput->press.button, buttonsToCheck)) {
        Player_SetActionFunc(globalCtx, this, Player_ThrowSilverBoulder, 1);
        Player_PlayAnimOnce(globalCtx, this, &gPlayerAnim_0032B8);
    }
}

void Player_ThrowSilverBoulder(Player* this, GlobalContext* globalCtx) {
    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        Player_SetupStandingStillType(this, globalCtx);
        return;
    }

    if (LinkAnimation_OnFrame(&this->skelAnime, 6.0f)) {
        Actor* heldActor = this->heldActor;

        heldActor->world.rot.y = this->actor.shape.rot.y;
        heldActor->speedXZ = 10.0f;
        heldActor->velocity.y = 20.0f;
        Player_SetupHeldItemUpperActionFunc(globalCtx, this);
        func_8002F7DC(&this->actor, NA_SE_PL_THROW);
        Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_SWORD_N);
    }
}

void Player_FailToLiftActor(Player* this, GlobalContext* globalCtx) {
    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        Player_PlayAnimLoop(globalCtx, this, &gPlayerAnim_003070);
        this->genericTimer = 15;
        return;
    }

    if (this->genericTimer != 0) {
        this->genericTimer--;
        if (this->genericTimer == 0) {
            Player_SetupReturnToStandStillSetAnim(this, &gPlayerAnim_003068, globalCtx);
            this->stateFlags1 &= ~PLAYER_STATE1_HOLDING_ACTOR;
            Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_DAMAGE_S);
        }
    }
}

void Player_SetupPutDownActor(Player* this, GlobalContext* globalCtx) {
    Player_StepLinearVelocityToZero(this);

    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        Player_SetupStandingStillType(this, globalCtx);
        return;
    }

    if (LinkAnimation_OnFrame(&this->skelAnime, 4.0f)) {
        Actor* heldActor = this->heldActor;

        if (!Player_InterruptHoldingActor(globalCtx, this, heldActor)) {
            heldActor->velocity.y = 0.0f;
            heldActor->speedXZ = 0.0f;
            Player_SetupHeldItemUpperActionFunc(globalCtx, this);
            if (heldActor->id == ACTOR_EN_BOM_CHU) {
                Player_ForceFirstPerson(this, globalCtx);
            }
        }
    }
}

void Player_StartThrowActor(Player* this, GlobalContext* globalCtx) {
    f32 sp34;
    s16 sp32;

    Player_StepLinearVelocityToZero(this);

    if (LinkAnimation_Update(globalCtx, &this->skelAnime) ||
        ((this->skelAnime.curFrame >= 8.0f) && Player_GetTargetVelocityAndYaw(this, &sp34, &sp32, 0.018f, globalCtx))) {
        Player_SetupStandingStillType(this, globalCtx);
        return;
    }

    if (LinkAnimation_OnFrame(&this->skelAnime, 3.0f)) {
        func_8084409C(globalCtx, this, this->linearVelocity + 8.0f, 12.0f);
    }
}

static ColliderCylinderInit D_80854624 = {
    {
        COLTYPE_HIT5,
        AT_NONE,
        AC_ON | AC_TYPE_ENEMY,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_PLAYER,
        COLSHAPE_CYLINDER,
    },
    {
        ELEMTYPE_UNK1,
        { 0x00000000, 0x00, 0x00 },
        { 0xFFCFFFFF, 0x00, 0x00 },
        TOUCH_NONE,
        BUMP_ON,
        OCELEM_ON,
    },
    { 12, 60, 0, { 0, 0, 0 } },
};

static ColliderQuadInit D_80854650 = {
    {
        COLTYPE_NONE,
        AT_ON | AT_TYPE_PLAYER,
        AC_NONE,
        OC1_NONE,
        OC2_TYPE_PLAYER,
        COLSHAPE_QUAD,
    },
    {
        ELEMTYPE_UNK2,
        { 0x00000100, 0x00, 0x01 },
        { 0xFFCFFFFF, 0x00, 0x00 },
        TOUCH_ON | TOUCH_SFX_NORMAL,
        BUMP_NONE,
        OCELEM_NONE,
    },
    { { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } } },
};

static ColliderQuadInit D_808546A0 = {
    {
        COLTYPE_METAL,
        AT_ON | AT_TYPE_PLAYER,
        AC_ON | AC_HARD | AC_TYPE_ENEMY,
        OC1_NONE,
        OC2_TYPE_PLAYER,
        COLSHAPE_QUAD,
    },
    {
        ELEMTYPE_UNK2,
        { 0x00100000, 0x00, 0x00 },
        { 0xDFCFFFFF, 0x00, 0x00 },
        TOUCH_ON | TOUCH_SFX_NORMAL,
        BUMP_ON,
        OCELEM_NONE,
    },
    { { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } } },
};

void func_8084663C(Actor* thisx, GlobalContext* globalCtx) {
}

void Player_SpawnNoUpdateOrDraw(GlobalContext* globalCtx, Player* this) {
    this->actor.update = func_8084663C;
    this->actor.draw = NULL;
}

void Player_SetupSpawnFromBlueWarp(GlobalContext* globalCtx, Player* this) {
    Player_SetActionFunc(globalCtx, this, Player_SpawnFromBlueWarp, 0);
    if ((globalCtx->sceneNum == SCENE_SPOT06) && (gSaveContext.sceneSetupIndex >= 4)) {
        this->genericVar = 1;
    }
    this->stateFlags1 |= PLAYER_STATE1_IN_CUTSCENE;
    LinkAnimation_Change(globalCtx, &this->skelAnime, &gPlayerAnim_003298, 2.0f / 3.0f, 0.0f, 24.0f, ANIMMODE_ONCE,
                         0.0f);
    this->actor.world.pos.y += 800.0f;
}

static u8 D_808546F0[] = { ITEM_SWORD_MASTER, ITEM_SWORD_KOKIRI };

void func_80846720(GlobalContext* globalCtx, Player* this, s32 arg2) {
    s32 item = D_808546F0[(void)0, gSaveContext.linkAge];
    s32 actionParam = sItemActionParams[item];

    Player_PutAwayHookshot(this);
    Player_DetatchHeldActor(globalCtx, this);

    this->heldItemId = item;
    this->nextModelGroup = Player_ActionToModelGroup(this, actionParam);

    Player_ChangeItem(globalCtx, this, actionParam);
    Player_SetupHeldItemUpperActionFunc(globalCtx, this);

    if (arg2 != 0) {
        func_8002F7DC(&this->actor, NA_SE_IT_SWORD_PICKOUT);
    }
}

static Vec3f D_808546F4 = { -1.0f, 69.0f, 20.0f };

void Player_SpawnFromTimeTravel(GlobalContext* globalCtx, Player* this) {
    Player_SetActionFunc(globalCtx, this, Player_EndTimeTravel, 0);
    this->stateFlags1 |= PLAYER_STATE1_IN_CUTSCENE;
    Math_Vec3f_Copy(&this->actor.world.pos, &D_808546F4);
    this->currentYaw = this->actor.shape.rot.y = -0x8000;
    LinkAnimation_Change(globalCtx, &this->skelAnime, this->ageProperties->unk_A0, 2.0f / 3.0f, 0.0f, 0.0f,
                         ANIMMODE_ONCE, 0.0f);
    Player_SetupAnimMovement(globalCtx, this, 0x28F);
    if (LINK_IS_ADULT) {
        func_80846720(globalCtx, this, 0);
    }
    this->genericTimer = 20;
}

void Player_SpawnOpeningDoor(GlobalContext* globalCtx, Player* this) {
    Player_SetActionFunc(globalCtx, this, Player_SetupOpenDoorFromSpawn, 0);
    Player_SetupAnimMovement(globalCtx, this, 0x9B);
}

void Player_SpawnExitingGrotto(GlobalContext* globalCtx, Player* this) {
    Player_SetupJump(this, &gPlayerAnim_002FE0, 12.0f, globalCtx);
    Player_SetActionFunc(globalCtx, this, Player_JumpFromGrotto, 0);
    this->stateFlags1 |= PLAYER_STATE1_IN_CUTSCENE;
    this->fallStartHeight = this->actor.world.pos.y;
    OnePointCutscene_Init(globalCtx, 5110, 40, &this->actor, MAIN_CAM);
}

void Player_SpawnWithKnockback(GlobalContext* globalCtx, Player* this) {
    Player_SetupDamage(globalCtx, this, 1, 2.0f, 2.0f, this->actor.shape.rot.y + 0x8000, 0);
}

void Player_SetupSpawnFromWarpSong(GlobalContext* globalCtx, Player* this) {
    Player_SetActionFunc(globalCtx, this, Player_SpawnFromWarpSong, 0);
    this->actor.draw = NULL;
    this->stateFlags1 |= PLAYER_STATE1_IN_CUTSCENE;
}

static s16 D_80854700[] = { ACTOR_MAGIC_WIND, ACTOR_MAGIC_DARK, ACTOR_MAGIC_FIRE };

Actor* func_80846A00(GlobalContext* globalCtx, Player* this, s32 arg2) {
    return Actor_Spawn(&globalCtx->actorCtx, globalCtx, D_80854700[arg2], this->actor.world.pos.x,
                       this->actor.world.pos.y, this->actor.world.pos.z, 0, 0, 0, 0);
}

void Player_SetupSpawnFromFaroresWind(GlobalContext* globalCtx, Player* this) {
    this->actor.draw = NULL;
    Player_SetActionFunc(globalCtx, this, Player_SpawnFromFaroresWind, 0);
    this->stateFlags1 |= PLAYER_STATE1_IN_CUTSCENE;
}

static InitChainEntry D_80854708[] = {
    ICHAIN_F32(targetArrowOffset, 500, ICHAIN_STOP),
};

static EffectBlureInit2 D_8085470C = {
    0, 8, 0, { 255, 255, 255, 255 }, { 255, 255, 255, 64 }, { 255, 255, 255, 0 }, { 255, 255, 255, 0 }, 4,
    0, 2, 0, { 0, 0, 0, 0 },         { 0, 0, 0, 0 },
};

static Vec3s D_80854730 = { -57, 3377, 0 };

void Player_InitCommon(Player* this, GlobalContext* globalCtx, FlexSkeletonHeader* skelHeader) {
    this->ageProperties = &sAgeProperties[gSaveContext.linkAge];
    Actor_ProcessInitChain(&this->actor, D_80854708);
    this->swordEffectIndex = TOTAL_EFFECT_COUNT;
    this->currentYaw = this->actor.world.rot.y;
    Player_SetupHeldItemUpperActionFunc(globalCtx, this);

    SkelAnime_InitLink(globalCtx, &this->skelAnime, skelHeader,
                       GET_PLAYER_ANIM(PLAYER_ANIMGROUP_STANDING_STILL, this->modelAnimType), 9, this->jointTable,
                       this->morphTable, PLAYER_LIMB_MAX);
    this->skelAnime.baseTransl = D_80854730;
    SkelAnime_InitLink(globalCtx, &this->skelAnimeUpper, skelHeader, Player_GetStandingStillAnim(this), 9,
                       this->jointTableUpper, this->morphTableUpper, PLAYER_LIMB_MAX);
    this->skelAnimeUpper.baseTransl = D_80854730;

    Effect_Add(globalCtx, &this->swordEffectIndex, EFFECT_BLURE2, 0, 0, &D_8085470C);
    ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawFeet, this->ageProperties->unk_04);
    this->subCamId = SUBCAM_NONE;
    Collider_InitCylinder(globalCtx, &this->cylinder);
    Collider_SetCylinder(globalCtx, &this->cylinder, &this->actor, &D_80854624);
    Collider_InitQuad(globalCtx, &this->swordQuads[0]);
    Collider_SetQuad(globalCtx, &this->swordQuads[0], &this->actor, &D_80854650);
    Collider_InitQuad(globalCtx, &this->swordQuads[1]);
    Collider_SetQuad(globalCtx, &this->swordQuads[1], &this->actor, &D_80854650);
    Collider_InitQuad(globalCtx, &this->shieldQuad);
    Collider_SetQuad(globalCtx, &this->shieldQuad, &this->actor, &D_808546A0);
}

static void (*D_80854738[])(GlobalContext* globalCtx, Player* this) = {
    Player_SpawnNoUpdateOrDraw,       Player_SpawnFromTimeTravel,
    Player_SetupSpawnFromBlueWarp,    Player_SpawnOpeningDoor,
    Player_SpawnExitingGrotto,        Player_SetupSpawnFromWarpSong,
    Player_SetupSpawnFromFaroresWind, Player_SpawnWithKnockback,
    Player_SpawnWalkingSlow,          Player_SpawnWalkingSlow,
    Player_SpawnWalkingSlow,          Player_SpawnWalkingSlow,
    Player_SpawnWalkingSlow,          Player_SpawnNoMomentum,
    Player_SpawnWalkingSlow,          Player_SpawnWalkingPreserveMomentum,
};

static Vec3f D_80854778 = { 0.0f, 50.0f, 0.0f };

void Player_Init(Actor* thisx, GlobalContext* globalCtx2) {
    Player* this = (Player*)thisx;
    GlobalContext* globalCtx = globalCtx2;
    SceneTableEntry* scene = globalCtx->loadedScene;
    u32 titleFileSize;
    s32 initMode;
    s32 sp50;
    s32 sp4C;

    globalCtx->shootingGalleryStatus = globalCtx->bombchuBowlingStatus = 0;

    globalCtx->playerInit = Player_InitCommon;
    globalCtx->playerUpdate = Player_UpdateCommon;
    globalCtx->isPlayerDroppingFish = Player_IsDroppingFish;
    globalCtx->startPlayerFishing = Player_StartFishing;
    globalCtx->grabPlayer = Player_SetupRestrainedByEnemy;
    globalCtx->startPlayerCutscene = Player_SetupPlayerCutscene;
    globalCtx->func_11D54 = Player_SetupStandingStillMorph;
    globalCtx->damagePlayer = Player_InflictDamage;
    globalCtx->talkWithPlayer = Player_StartTalkingWithActor;

    thisx->room = -1;
    this->ageProperties = &sAgeProperties[gSaveContext.linkAge];
    this->itemActionParam = this->heldItemActionParam = -1;
    this->heldItemId = ITEM_NONE;

    Player_UseItem(globalCtx, this, ITEM_NONE);
    Player_SetEquipmentData(globalCtx, this);
    this->prevBoots = this->currentBoots;
    if (CVar_GetS32("gMMBunnyHood", 0)) {
        if (INV_CONTENT(ITEM_TRADE_CHILD) == ITEM_SOLD_OUT) {
            sMaskMemory = PLAYER_MASK_NONE;
        }
        this->currentMask = sMaskMemory;
        for (uint16_t cSlotIndex = 0; cSlotIndex < ARRAY_COUNT(gSaveContext.equips.cButtonSlots); cSlotIndex++) {
            if (gSaveContext.equips.cButtonSlots[cSlotIndex] == SLOT_TRADE_CHILD &&
                (gItemAgeReqs[gSaveContext.equips.buttonItems[cSlotIndex + 1]] != 9 && LINK_IS_ADULT &&
                 !CVar_GetS32("gNoRestrictAge", 0))) {
                gSaveContext.equips.cButtonSlots[cSlotIndex] = SLOT_NONE;
                gSaveContext.equips.buttonItems[cSlotIndex + 1] = ITEM_NONE;
            }
        }
    }
    Player_InitCommon(this, globalCtx, gPlayerSkelHeaders[((void)0, gSaveContext.linkAge)]);
    this->giObjectSegment = (void*)(((uintptr_t)ZELDA_ARENA_MALLOC_DEBUG(0x3008) + 8) & ~0xF);

    sp50 = gSaveContext.respawnFlag;

    if (sp50 != 0) {
        if (sp50 == -3) {
            thisx->params = gSaveContext.respawn[RESPAWN_MODE_RETURN].playerParams;
        } else {
            if ((sp50 == 1) || (sp50 == -1)) {
                this->voidRespawnCounter = -2;
            }

            if (sp50 < 0) {
                sp4C = 0;
            } else {
                sp4C = sp50 - 1;
                Math_Vec3f_Copy(&thisx->world.pos, &gSaveContext.respawn[sp50 - 1].pos);
                Math_Vec3f_Copy(&thisx->home.pos, &thisx->world.pos);
                Math_Vec3f_Copy(&thisx->prevPos, &thisx->world.pos);
                this->fallStartHeight = thisx->world.pos.y;
                this->currentYaw = thisx->shape.rot.y = gSaveContext.respawn[sp4C].yaw;
                thisx->params = gSaveContext.respawn[sp4C].playerParams;
            }

            globalCtx->actorCtx.flags.tempSwch = gSaveContext.respawn[sp4C].tempSwchFlags & 0xFFFFFF;
            globalCtx->actorCtx.flags.tempCollect = gSaveContext.respawn[sp4C].tempCollectFlags;
        }
    }

    if ((sp50 == 0) || (sp50 < -1)) {
        titleFileSize = scene->titleFile.vromEnd - scene->titleFile.vromStart;
        if (gSaveContext.showTitleCard) {
            if ((gSaveContext.sceneSetupIndex < 4) &&
                (gEntranceTable[((void)0, gSaveContext.entranceIndex) + ((void)0, gSaveContext.sceneSetupIndex)].field &
                 0x4000) &&
                ((globalCtx->sceneNum != SCENE_DDAN) || (gSaveContext.eventChkInf[11] & 1)) &&
                ((globalCtx->sceneNum != SCENE_NIGHT_SHOP) || (gSaveContext.eventChkInf[2] & 0x20))) {
                TitleCard_InitPlaceName(globalCtx, &globalCtx->actorCtx.titleCtx, this->giObjectSegment, 160, 120, 144,
                                        24, 20);
            }
        }
        gSaveContext.showTitleCard = true;
    }

    if (func_80845C68(globalCtx, (sp50 == 2) ? 1 : 0) == 0) {
        gSaveContext.respawn[RESPAWN_MODE_DOWN].playerParams = (thisx->params & 0xFF) | 0xD00;
    }

    gSaveContext.respawn[RESPAWN_MODE_DOWN].data = 1;

    if (globalCtx->sceneNum <= SCENE_GANONTIKA_SONOGO) {
        gSaveContext.infTable[26] |= gBitFlags[globalCtx->sceneNum];
    }

    initMode = (thisx->params & 0xF00) >> 8;
    if ((initMode == 5) || (initMode == 6)) {
        if (gSaveContext.cutsceneIndex >= 0xFFF0) {
            initMode = 13;
        }
    }

    D_80854738[initMode](globalCtx, this);

    if (initMode != 0) {
        if ((gSaveContext.gameMode == 0) || (gSaveContext.gameMode == 3)) {
            this->naviActor = Player_SpawnFairy(globalCtx, this, &thisx->world.pos, &D_80854778, FAIRY_NAVI);
            if (gSaveContext.dogParams != 0) {
                gSaveContext.dogParams |= 0x8000;
            }
        }
    }

    if (gSaveContext.nayrusLoveTimer != 0) {
        gSaveContext.unk_13F0 = 3;
        func_80846A00(globalCtx, this, 1);
        this->stateFlags3 &= ~PLAYER_STATE3_RESTORE_NAYRUS_LOVE;
    }

    if (gSaveContext.entranceSound != 0) {
        Audio_PlayActorSound2(&this->actor, ((void)0, gSaveContext.entranceSound));
        gSaveContext.entranceSound = 0;
    }

    Map_SavePlayerInitialInfo(globalCtx);
    MREG(64) = 0;
}

void func_808471F4(s16* pValue) {
    s16 step;

    step = (ABS(*pValue) * 100.0f) / 1000.0f;
    step = CLAMP(step, 400, 4000);

    Math_ScaledStepToS(pValue, 0, step);
}

void func_80847298(Player* this) {
    s16 sp26;

    if (!(this->lookFlags & 2)) {
        sp26 = this->actor.focus.rot.y - this->actor.shape.rot.y;
        func_808471F4(&sp26);
        this->actor.focus.rot.y = this->actor.shape.rot.y + sp26;
    }

    if (!(this->lookFlags & 1)) {
        func_808471F4(&this->actor.focus.rot.x);
    }

    if (!(this->lookFlags & 8)) {
        func_808471F4(&this->headRot.x);
    }

    if (!(this->lookFlags & 0x40)) {
        func_808471F4(&this->upperBodyRot.x);
    }

    if (!(this->lookFlags & 4)) {
        func_808471F4(&this->actor.focus.rot.z);
    }

    if (!(this->lookFlags & 0x10)) {
        func_808471F4(&this->headRot.y);
    }

    if (!(this->lookFlags & 0x20)) {
        func_808471F4(&this->headRot.z);
    }

    if (!(this->lookFlags & 0x80)) {
        if (this->upperBodyYawOffset != 0) {
            func_808471F4(&this->upperBodyYawOffset);
        } else {
            func_808471F4(&this->upperBodyRot.y);
        }
    }

    if (!(this->lookFlags & 0x100)) {
        func_808471F4(&this->upperBodyRot.z);
    }

    this->lookFlags = 0;
}

static f32 D_80854784[] = { 120.0f, 240.0f, 360.0f };

static u8 sDiveDoActions[] = { DO_ACTION_1, DO_ACTION_2, DO_ACTION_3, DO_ACTION_4,
                               DO_ACTION_5, DO_ACTION_6, DO_ACTION_7, DO_ACTION_8 };

void func_808473D4(GlobalContext* globalCtx, Player* this) {
    if ((Message_GetState(&globalCtx->msgCtx) == TEXT_STATE_NONE) && (this->actor.category == ACTORCAT_PLAYER)) {
        Actor* heldActor = this->heldActor;
        Actor* interactRangeActor = this->interactRangeActor;
        s32 sp24;
        s32 sp20 = this->relativeAnalogStickInputs[this->inputFrameCounter];
        s32 sp1C = Player_IsSwimming(this);
        s32 doAction = DO_ACTION_NONE;

        if (!Player_InBlockingCsMode(globalCtx, this)) {
            if (this->stateFlags1 & PLAYER_STATE1_IN_FIRST_PERSON_MODE) {
                doAction = DO_ACTION_RETURN;
            } else if ((this->heldItemActionParam == PLAYER_AP_FISHING_POLE) && (this->fpsItemType != 0)) {
                if (this->fpsItemType == 2) {
                    doAction = DO_ACTION_REEL;
                }
            } else if ((Player_PlayOcarina != this->actionFunc) &&
                       !(this->stateFlags2 & PLAYER_STATE2_INSIDE_CRAWLSPACE)) {
                if ((this->doorType != PLAYER_DOORTYPE_NONE) &&
                    (!(this->stateFlags1 & PLAYER_STATE1_HOLDING_ACTOR) ||
                     ((heldActor != NULL) && (heldActor->id == ACTOR_EN_RU1)))) {
                    doAction = DO_ACTION_OPEN;
                } else if ((!(this->stateFlags1 & PLAYER_STATE1_HOLDING_ACTOR) || (heldActor == NULL)) &&
                           (interactRangeActor != NULL) &&
                           ((!sp1C && (this->getItemId == GI_NONE)) ||
                            ((this->getItemId < 0) && !(this->stateFlags1 & PLAYER_STATE1_SWIMMING)))) {
                    if (this->getItemId < 0) {
                        doAction = DO_ACTION_OPEN;
                    } else if ((interactRangeActor->id == ACTOR_BG_TOKI_SWD) && LINK_IS_ADULT) {
                        doAction = DO_ACTION_DROP;
                    } else {
                        doAction = DO_ACTION_GRAB;
                    }
                } else if (!sp1C && (this->stateFlags2 & PLAYER_STATE2_CAN_GRAB_PUSH_PULL_WALL)) {
                    doAction = DO_ACTION_GRAB;
                } else if ((this->stateFlags2 & PLAYER_STATE2_CAN_CLIMB_PUSH_PULL_WALL) ||
                           (!(this->stateFlags1 & PLAYER_STATE1_RIDING_HORSE) && (this->rideActor != NULL))) {
                    doAction = DO_ACTION_CLIMB;
                } else if ((this->stateFlags1 & PLAYER_STATE1_RIDING_HORSE) &&
                           !EN_HORSE_CHECK_4((EnHorse*)this->rideActor) && (Player_DismountHorse != this->actionFunc)) {
                    if ((this->stateFlags2 & PLAYER_STATE2_CAN_SPEAK_OR_CHECK) && (this->talkActor != NULL)) {
                        if (this->talkActor->category == ACTORCAT_NPC) {
                            doAction = DO_ACTION_SPEAK;
                        } else {
                            doAction = DO_ACTION_CHECK;
                        }
                    } else if (!Actor_PlayerIsAimingReadyFpsItem(this) && !(this->stateFlags1 & PLAYER_STATE1_IN_FIRST_PERSON_MODE)) {
                        doAction = DO_ACTION_FASTER;
                    }
                } else if ((this->stateFlags2 & PLAYER_STATE2_CAN_SPEAK_OR_CHECK) && (this->talkActor != NULL)) {
                    if (this->talkActor->category == ACTORCAT_NPC) {
                        doAction = DO_ACTION_SPEAK;
                    } else {
                        doAction = DO_ACTION_CHECK;
                    }
                } else if ((this->stateFlags1 & (PLAYER_STATE1_HANGING_FROM_LEDGE_SLIP | PLAYER_STATE1_CLIMBING)) ||
                           ((this->stateFlags1 & PLAYER_STATE1_RIDING_HORSE) &&
                            (this->stateFlags2 & PLAYER_STATE2_CAN_DISMOUNT_HORSE))) {
                    doAction = DO_ACTION_DOWN;
                } else if (this->stateFlags2 & PLAYER_STATE2_CAN_ENTER_CRAWLSPACE) {
                    doAction = DO_ACTION_ENTER;
                } else if ((this->stateFlags1 & PLAYER_STATE1_HOLDING_ACTOR) && (this->getItemId == GI_NONE) &&
                           (heldActor != NULL)) {
                    if ((this->actor.bgCheckFlags & 1) || (heldActor->id == ACTOR_EN_NIW)) {
                        if (func_8083EAF0(this, heldActor) == 0) {
                            doAction = DO_ACTION_DROP;
                        } else {
                            doAction = DO_ACTION_THROW;
                        }
                    }
                } else if (!(this->stateFlags1 & PLAYER_STATE1_SWIMMING) && Player_CanHoldActor(this) &&
                           (this->getItemId < GI_MAX)) {
                    doAction = DO_ACTION_GRAB;
                } else if (this->stateFlags2 & PLAYER_STATE2_ENABLE_DIVE_CAMERA_AND_TIMER) {
                    sp24 = (D_80854784[CUR_UPG_VALUE(UPG_SCALE)] - this->actor.yDistToWater) / 40.0f;
                    sp24 = CLAMP(sp24, 0, 7);
                    doAction = sDiveDoActions[sp24];
                } else if (sp1C && !(this->stateFlags2 & PLAYER_STATE2_DIVING)) {
                    doAction = DO_ACTION_DIVE;
                } else if (!sp1C && (!(this->stateFlags1 & PLAYER_STATE1_SHIELDING) || Player_IsZTargeting(this) ||
                                     !Player_IsChildWithHylianShield(this))) {
                    if ((!(this->stateFlags1 & PLAYER_STATE1_CLIMBING_ONTO_LEDGE) && (sp20 <= 0) &&
                         (Player_IsUnfriendlyZTargeting(this) ||
                          ((sFloorSpecialProperty != 7) &&
                           (Player_IsFriendlyZTargeting(this) ||
                            ((globalCtx->roomCtx.curRoom.unk_03 != 2) &&
                             !(this->stateFlags1 & PLAYER_STATE1_SHIELDING) && (sp20 == 0))))))) {
                        doAction = DO_ACTION_ATTACK;
                    } else if ((globalCtx->roomCtx.curRoom.unk_03 != 2) && Player_IsZTargeting(this) && (sp20 > 0)) {
                        doAction = DO_ACTION_JUMP;
                    } else if ((this->heldItemActionParam >= PLAYER_AP_SWORD_MASTER) ||
                               ((this->stateFlags2 & PLAYER_STATE2_NAVI_IS_ACTIVE) &&
                                (globalCtx->actorCtx.targetCtx.arrowPointedActor == NULL))) {
                        doAction = DO_ACTION_PUTAWAY;
                    }
                }
            }
        }

        if (doAction != DO_ACTION_PUTAWAY) {
            this->putAwayTimer = 20;
        } else if (this->putAwayTimer != 0) {
            doAction = DO_ACTION_NONE;
            this->putAwayTimer--;
        }

        Interface_SetDoAction(globalCtx, doAction);

        if (this->stateFlags2 & PLAYER_STATE2_NAVI_REQUESTING_TALK) {
            if (this->targetActor != NULL) {
                Interface_SetNaviCall(globalCtx, 0x1E);
            } else {
                Interface_SetNaviCall(globalCtx, 0x1D);
            }
            Interface_SetNaviCall(globalCtx, 0x1E);
        } else {
            Interface_SetNaviCall(globalCtx, 0x1F);
        }
    }
}

s32 func_80847A78(Player* this) {
    s32 cond;

    if ((this->currentBoots == PLAYER_BOOTS_HOVER) && (this->hoverBootsTimer != 0)) {
        this->hoverBootsTimer--;
    } else {
        this->hoverBootsTimer = 0;
    }

    cond = (this->currentBoots == PLAYER_BOOTS_HOVER) &&
           ((this->actor.yDistToWater >= 0.0f) || (Player_GetHurtFloorType(sFloorSpecialProperty) >= 0) ||
            Player_IsFloorSinkingSand(sFloorSpecialProperty));

    if (cond && (this->actor.bgCheckFlags & 1) && (this->hoverBootsTimer != 0)) {
        this->actor.bgCheckFlags &= ~1;
    }

    if (this->actor.bgCheckFlags & 1) {
        if (!cond) {
            this->hoverBootsTimer = 19;
        }
        return 0;
    }

    sFloorSpecialProperty = 0;
    this->angleToFloorX = this->angleToFloorY = sAngleToFloorX = 0;

    return 1;
}

static Vec3f D_80854798 = { 0.0f, 18.0f, 0.0f };

void func_80847BA0(GlobalContext* globalCtx, Player* this) {
    u8 spC7 = 0;
    CollisionPoly* spC0;
    Vec3f spB4;
    f32 spB0;
    f32 spAC;
    f32 spA8;
    u32 spA4;

    sFloorProperty = this->floorProperty;

    if (this->stateFlags2 & PLAYER_STATE2_INSIDE_CRAWLSPACE) {
        spB0 = 10.0f;
        spAC = 15.0f;
        spA8 = 30.0f;
    } else {
        spB0 = this->ageProperties->unk_38;
        spAC = 26.0f;
        spA8 = this->ageProperties->unk_00;
    }

    if (this->stateFlags1 & (PLAYER_STATE1_IN_CUTSCENE | PLAYER_STATE1_FALLING_INTO_GROTTO_OR_VOID)) {
        if (this->stateFlags1 & PLAYER_STATE1_FALLING_INTO_GROTTO_OR_VOID) {
            this->actor.bgCheckFlags &= ~1;
            spA4 = 0x38;
        } else if ((this->stateFlags1 & PLAYER_STATE1_EXITING_SCENE) &&
                   ((this->sceneExitPosY - (s32)this->actor.world.pos.y) >= 100)) {
            spA4 = 0x39;
        } else if (!(this->stateFlags1 & PLAYER_STATE1_EXITING_SCENE) &&
                   ((Player_OpenDoor == this->actionFunc) || (Player_MiniCsMovement == this->actionFunc))) {
            this->actor.bgCheckFlags &= ~0x208;
            spA4 = 0x3C;
        } else {
            spA4 = 0x3F;
        }
    } else {
        spA4 = 0x3F;
    }

    if (this->stateFlags3 & PLAYER_STATE3_IGNORE_CEILING_FLOOR_AND_WATER) {
        spA4 &= ~6;
    }

    if (spA4 & 4) {
        this->stateFlags3 |= PLAYER_STATE3_CHECKING_FLOOR_AND_WATER_COLLISION;
    }

    Math_Vec3f_Copy(&spB4, &this->actor.world.pos);
    Actor_UpdateBgCheckInfo(globalCtx, &this->actor, spAC, spB0, spA8, spA4);

    if (this->actor.bgCheckFlags & 0x10) {
        this->actor.velocity.y = 0.0f;
    }

    sPlayerYDistToFloor = this->actor.world.pos.y - this->actor.floorHeight;
    sConveyorSpeedIndex = 0;

    spC0 = this->actor.floorPoly;

    if (spC0 != NULL) {
        this->floorProperty = func_80041EA4(&globalCtx->colCtx, spC0, this->actor.floorBgId);
        this->prevSurfaceMaterial = this->surfaceMaterial;

        if (this->actor.bgCheckFlags & 0x20) {
            if (this->actor.yDistToWater < 20.0f) {
                this->surfaceMaterial = 4;
            } else {
                this->surfaceMaterial = 5;
            }
        } else {
            if (this->stateFlags2 & PLAYER_STATE2_SPAWN_DUST_AT_FEET) {
                this->surfaceMaterial = 1;
            } else {
                this->surfaceMaterial = SurfaceType_GetSfx(&globalCtx->colCtx, spC0, this->actor.floorBgId);
            }
        }

        if (this->actor.category == ACTORCAT_PLAYER) {
            Audio_SetCodeReverb(SurfaceType_GetEcho(&globalCtx->colCtx, spC0, this->actor.floorBgId));

            if (this->actor.floorBgId == BGCHECK_SCENE) {
                func_80074CE8(globalCtx,
                              SurfaceType_GetLightSettingIndex(&globalCtx->colCtx, spC0, this->actor.floorBgId));
            } else {
                func_80043508(&globalCtx->colCtx, this->actor.floorBgId);
            }
        }

        sConveyorSpeedIndex = SurfaceType_GetConveyorSpeed(&globalCtx->colCtx, spC0, this->actor.floorBgId);
        if (sConveyorSpeedIndex != 0) {
            sIsFloorConveyor = SurfaceType_IsConveyor(&globalCtx->colCtx, spC0, this->actor.floorBgId);
            if (((sIsFloorConveyor == 0) && (this->actor.yDistToWater > 20.0f) &&
                 (this->currentBoots != PLAYER_BOOTS_IRON)) ||
                ((sIsFloorConveyor != 0) && (this->actor.bgCheckFlags & 1))) {
                sConveyorYaw = SurfaceType_GetConveyorDirection(&globalCtx->colCtx, spC0, this->actor.floorBgId) << 10;
            } else {
                sConveyorSpeedIndex = 0;
            }
        }
    }

    Player_SetupExit(globalCtx, this, spC0, this->actor.floorBgId);

    this->actor.bgCheckFlags &= ~0x200;

    if (this->actor.bgCheckFlags & 8) {
        CollisionPoly* spA0;
        s32 sp9C;
        s16 sp9A;
        s32 pad;

        D_80854798.y = 18.0f;
        D_80854798.z = this->ageProperties->unk_38 + 10.0f;

        if (!(this->stateFlags2 & PLAYER_STATE2_INSIDE_CRAWLSPACE) &&
            Player_WallLineTestWithOffset(globalCtx, this, &D_80854798, &spA0, &sp9C, &sWallIntersectPos)) {
            this->actor.bgCheckFlags |= 0x200;
            if (this->actor.wallPoly != spA0) {
                this->actor.wallPoly = spA0;
                this->actor.wallBgId = sp9C;
                this->actor.wallYaw = Math_Atan2S(spA0->normal.z, spA0->normal.x);
            }
        }

        sp9A = this->actor.shape.rot.y - (s16)(this->actor.wallYaw + 0x8000);

        sTouchedWallFlags = func_80041DB8(&globalCtx->colCtx, this->actor.wallPoly, this->actor.wallBgId);

        sYawToTouchedWall = ABS(sp9A);

        sp9A = this->currentYaw - (s16)(this->actor.wallYaw + 0x8000);

        sYawToTouchedWall2 = ABS(sp9A);

        spB0 = sYawToTouchedWall2 * 0.00008f;
        if (!(this->actor.bgCheckFlags & 1) || spB0 >= 1.0f) {
            this->speedLimit = R_RUN_SPEED_LIMIT / 100.0f;
        } else {
            spAC = (R_RUN_SPEED_LIMIT / 100.0f * spB0);
            this->speedLimit = spAC;
            if (spAC < 0.1f) {
                this->speedLimit = 0.1f;
            }
        }

        if ((this->actor.bgCheckFlags & 0x200) && (sYawToTouchedWall < 0x3000)) {
            CollisionPoly* wallPoly = this->actor.wallPoly;

            if ((ABS(wallPoly->normal.y) < 600) || (CVar_GetS32("gClimbEverything", 0) != 0)) {
                f32 sp8C = COLPOLY_GET_NORMAL(wallPoly->normal.x);
                f32 sp88 = COLPOLY_GET_NORMAL(wallPoly->normal.y);
                f32 sp84 = COLPOLY_GET_NORMAL(wallPoly->normal.z);
                f32 wallHeight;
                CollisionPoly* sp7C;
                CollisionPoly* sp78;
                s32 sp74;
                Vec3f sp68;
                f32 sp64;
                f32 sp60;
                s32 temp3;

                this->wallDistance = Math3D_UDistPlaneToPos(sp8C, sp88, sp84, wallPoly->dist, &this->actor.world.pos);

                spB0 = this->wallDistance + 10.0f;
                sp68.x = this->actor.world.pos.x - (spB0 * sp8C);
                sp68.z = this->actor.world.pos.z - (spB0 * sp84);
                sp68.y = this->actor.world.pos.y + this->ageProperties->unk_0C;

                sp64 = BgCheck_EntityRaycastFloor1(&globalCtx->colCtx, &sp7C, &sp68);
                wallHeight = sp64 - this->actor.world.pos.y;
                this->wallHeight = wallHeight;

                if ((this->wallHeight < 18.0f) ||
                    BgCheck_EntityCheckCeiling(&globalCtx->colCtx, &sp60, &this->actor.world.pos,
                                               (sp64 - this->actor.world.pos.y) + 20.0f, &sp78, &sp74, &this->actor)) {
                    this->wallHeight = 399.96002f;
                } else {
                    D_80854798.y = (sp64 + 5.0f) - this->actor.world.pos.y;

                    if (Player_WallLineTestWithOffset(globalCtx, this, &D_80854798, &sp78, &sp74, &sWallIntersectPos) &&
                        (temp3 = this->actor.wallYaw - Math_Atan2S(sp78->normal.z, sp78->normal.x),
                         ABS(temp3) < 0x4000) &&
                        !func_80041E18(&globalCtx->colCtx, sp78, sp74)) {
                        this->wallHeight = 399.96002f;
                    } else if (func_80041DE4(&globalCtx->colCtx, wallPoly, this->actor.wallBgId) == 0) {
                        if (this->ageProperties->unk_1C <= this->wallHeight) {
                            if (ABS(sp7C->normal.y) > 28000) {
                                if (this->ageProperties->unk_14 <= this->wallHeight) {
                                    spC7 = 4;
                                } else if (this->ageProperties->unk_18 <= this->wallHeight) {
                                    spC7 = 3;
                                } else {
                                    spC7 = 2;
                                }
                            }
                        } else {
                            spC7 = 1;
                        }
                    }
                }
            }
        }
    } else {
        this->speedLimit = R_RUN_SPEED_LIMIT / 100.0f;
        this->wallTouchTimer = 0;
        this->wallHeight = 0.0f;
    }

    if (spC7 == this->touchedWallJumpType) {
        if ((this->linearVelocity != 0.0f) && (this->wallTouchTimer < 100)) {
            this->wallTouchTimer++;
        }
    } else {
        this->touchedWallJumpType = spC7;
        this->wallTouchTimer = 0;
    }

    if (this->actor.bgCheckFlags & 1) {
        sFloorSpecialProperty = func_80041D4C(&globalCtx->colCtx, spC0, this->actor.floorBgId);

        if (!func_80847A78(this)) {
            f32 sp58;
            f32 sp54;
            f32 sp50;
            f32 sp4C;
            s32 pad2;
            f32 sp44;
            s32 pad3;

            if (this->actor.floorBgId != BGCHECK_SCENE) {
                func_800434C8(&globalCtx->colCtx, this->actor.floorBgId);
            }

            sp58 = COLPOLY_GET_NORMAL(spC0->normal.x);
            sp54 = 1.0f / COLPOLY_GET_NORMAL(spC0->normal.y);
            sp50 = COLPOLY_GET_NORMAL(spC0->normal.z);

            sp4C = Math_SinS(this->currentYaw);
            sp44 = Math_CosS(this->currentYaw);

            this->angleToFloorX = Math_Atan2S(1.0f, (-(sp58 * sp4C) - (sp50 * sp44)) * sp54);
            this->angleToFloorY = Math_Atan2S(1.0f, (-(sp58 * sp44) - (sp50 * sp4C)) * sp54);

            sp4C = Math_SinS(this->actor.shape.rot.y);
            sp44 = Math_CosS(this->actor.shape.rot.y);

            sAngleToFloorX = Math_Atan2S(1.0f, (-(sp58 * sp4C) - (sp50 * sp44)) * sp54);

            func_8083E318(globalCtx, this, spC0);
        }
    } else {
        func_80847A78(this);
    }

    if (this->prevFloorSpecialProperty == sFloorSpecialProperty) {
        this->hurtFloorTimer++;
    } else {
        this->prevFloorSpecialProperty = sFloorSpecialProperty;
        this->hurtFloorTimer = 0;
    }
}

void Player_UpdateCamAndSeqModes(GlobalContext* globalCtx, Player* this) {
    u8 seqMode;
    s32 pad;
    Actor* targetActor;
    s32 camMode;

    if (this->actor.category == ACTORCAT_PLAYER) {
        seqMode = SEQ_MODE_DEFAULT;

        if (this->csMode != 0) {
            Camera_ChangeMode(Gameplay_GetCamera(globalCtx, 0), CAM_MODE_NORMAL);
        } else if (!(this->stateFlags1 & PLAYER_STATE1_IN_FIRST_PERSON_MODE)) {
            if ((this->actor.parent != NULL) && (this->stateFlags3 & PLAYER_STATE3_MOVING_ALONG_HOOKSHOT_PATH)) {
                camMode = CAM_MODE_HOOKSHOT;
                Camera_SetParam(Gameplay_GetCamera(globalCtx, 0), 8, this->actor.parent);
            } else if (Player_StartKnockback == this->actionFunc) {
                camMode = CAM_MODE_STILL;
            } else if (this->stateFlags2 & PLAYER_STATE2_ENABLE_PUSH_PULL_CAM) {
                camMode = CAM_MODE_PUSHPULL;
            } else if ((targetActor = this->targetActor) != NULL) {
                if (CHECK_FLAG_ALL(this->actor.flags, ACTOR_FLAG_8)) {
                    camMode = CAM_MODE_TALK;
                } else if (this->stateFlags1 & PLAYER_STATE1_FORCE_STRAFING) {
                    if (this->stateFlags1 & PLAYER_STATE1_AWAITING_THROWN_BOOMERANG) {
                        camMode = CAM_MODE_FOLLOWBOOMERANG;
                    } else {
                        camMode = CAM_MODE_FOLLOWTARGET;
                    }
                } else {
                    camMode = CAM_MODE_BATTLE;
                }
                Camera_SetParam(Gameplay_GetCamera(globalCtx, 0), 8, targetActor);
            } else if (this->stateFlags1 & PLAYER_STATE1_CHARGING_SPIN_ATTACK) {
                camMode = CAM_MODE_CHARGE;
            } else if (this->stateFlags1 & PLAYER_STATE1_AWAITING_THROWN_BOOMERANG) {
                camMode = CAM_MODE_FOLLOWBOOMERANG;
                Camera_SetParam(Gameplay_GetCamera(globalCtx, 0), 8, this->boomerangActor);
            } else if (this->stateFlags1 &
                       (PLAYER_STATE1_HANGING_FROM_LEDGE_SLIP | PLAYER_STATE1_CLIMBING_ONTO_LEDGE)) {
                if (Player_IsFriendlyZTargeting(this)) {
                    camMode = CAM_MODE_HANGZ;
                } else {
                    camMode = CAM_MODE_HANG;
                }
            } else if (this->stateFlags1 & (PLAYER_STATE1_Z_TARGETING_FRIENDLY | PLAYER_STATE1_30)) {
                if ((Actor_PlayerIsAimingReadyFpsItem(this) || Player_IsAimingReadyBoomerang(this)) && !CVar_GetS32("gDisableFPSView", 0)) {
                    camMode = CAM_MODE_BOWARROWZ;
                } else if (this->stateFlags1 & PLAYER_STATE1_CLIMBING) {
                    camMode = CAM_MODE_CLIMBZ;
                } else {
                    camMode = CAM_MODE_TARGET;
                }
            } else if (this->stateFlags1 & (PLAYER_STATE1_JUMPING | PLAYER_STATE1_CLIMBING)) {
                if ((Player_JumpUpToLedge == this->actionFunc) || (this->stateFlags1 & PLAYER_STATE1_CLIMBING)) {
                    camMode = CAM_MODE_CLIMB;
                } else {
                    camMode = CAM_MODE_JUMP;
                }
            } else if (this->stateFlags1 & PLAYER_STATE1_FREEFALLING) {
                camMode = CAM_MODE_FREEFALL;
            } else if ((this->swordState != 0) && (this->swordAnimation >= 0) && (this->swordAnimation < 0x18)) {
                camMode = CAM_MODE_STILL;
            } else {
                camMode = CAM_MODE_NORMAL;
                if ((this->linearVelocity == 0.0f) &&
                    (!(this->stateFlags1 & PLAYER_STATE1_RIDING_HORSE) || (this->rideActor->speedXZ == 0.0f))) {
                    // not moving
                    seqMode = SEQ_MODE_STILL;
                }
            }

            Camera_ChangeMode(Gameplay_GetCamera(globalCtx, 0), camMode);
        } else {
            // First person mode
            seqMode = SEQ_MODE_STILL;
        }

        if (globalCtx->actorCtx.targetCtx.bgmEnemy != NULL) {
            seqMode = SEQ_MODE_ENEMY;
            Audio_SetBgmEnemyVolume(sqrtf(globalCtx->actorCtx.targetCtx.bgmEnemy->xyzDistToPlayerSq));
        }

        if (globalCtx->sceneNum != SCENE_TURIBORI) {
            Audio_SetSequenceMode(seqMode);
        }
    }
}

static Vec3f D_808547A4 = { 0.0f, 0.5f, 0.0f };
static Vec3f D_808547B0 = { 0.0f, 0.5f, 0.0f };

static Color_RGBA8 D_808547BC = { 255, 255, 100, 255 };
static Color_RGBA8 D_808547C0 = { 255, 50, 0, 0 };

void func_80848A04(GlobalContext* globalCtx, Player* this) {
    f32 temp;

    if (this->unk_85C == 0.0f) {
        Player_UseItem(globalCtx, this, 0xFF);
        return;
    }

    temp = 1.0f;
    if (DECR(this->fpsItemType) == 0) {
        Inventory_ChangeAmmo(ITEM_STICK, -1);
        this->fpsItemType = 1;
        temp = 0.0f;
        this->unk_85C = temp;
    } else if (this->fpsItemType > 200) {
        temp = (210 - this->fpsItemType) / 10.0f;
    } else if (this->fpsItemType < 20) {
        temp = this->fpsItemType / 20.0f;
        this->unk_85C = temp;
    }

    func_8002836C(globalCtx, &this->swordInfo[0].tip, &D_808547A4, &D_808547B0, &D_808547BC, &D_808547C0, temp * 200.0f,
                  0, 8);
}

void func_80848B44(GlobalContext* globalCtx, Player* this) {
    Vec3f shockPos;
    Vec3f* randBodyPart;
    s32 shockScale;

    this->shockTimer--;
    this->unk_892 += this->shockTimer;

    if (this->unk_892 > 20) {
        shockScale = this->shockTimer * 2;
        this->unk_892 -= 20;

        if (shockScale > 40) {
            shockScale = 40;
        }

        randBodyPart = this->bodyPartsPos + (s32)Rand_ZeroFloat(ARRAY_COUNT(this->bodyPartsPos) - 0.1f);
        shockPos.x = (Rand_CenteredFloat(5.0f) + randBodyPart->x) - this->actor.world.pos.x;
        shockPos.y = (Rand_CenteredFloat(5.0f) + randBodyPart->y) - this->actor.world.pos.y;
        shockPos.z = (Rand_CenteredFloat(5.0f) + randBodyPart->z) - this->actor.world.pos.z;

        EffectSsFhgFlash_SpawnShock(globalCtx, &this->actor, &shockPos, shockScale, FHGFLASH_SHOCK_PLAYER);
        func_8002F8F0(&this->actor, NA_SE_PL_SPARK - SFX_FLAG);
    }
}

void func_80848C74(GlobalContext* globalCtx, Player* this) {
    s32 spawnedFlame;
    u8* timerPtr;
    s32 timerStep;
    f32 flameScale;
    f32 flameIntensity;
    s32 dmgCooldown;
    s32 i;
    s32 sp58;
    s32 sp54;

    if (this->currentTunic == PLAYER_TUNIC_GORON || CVar_GetS32("gSuperTunic", 0) != 0) {
        sp54 = 20;
    } else {
        sp54 = (s32)(this->linearVelocity * 0.4f) + 1;
    }

    spawnedFlame = false;
    timerPtr = this->flameTimers;

    if (this->stateFlags2 & PLAYER_STATE2_MAKING_REACTABLE_NOISE) {
        sp58 = 100;
    } else {
        sp58 = 0;
    }

    Player_BurnDekuShield(this, globalCtx);

    for (i = 0; i < PLAYER_BODYPART_MAX; i++, timerPtr++) {
        timerStep = sp58 + sp54;

        if (*timerPtr <= timerStep) {
            *timerPtr = 0;
        } else {
            spawnedFlame = true;
            *timerPtr -= timerStep;

            if (*timerPtr > 20.0f) {
                flameIntensity = (*timerPtr - 20.0f) * 0.01f;
                flameScale = CLAMP(flameIntensity, 0.19999999f, 0.2f);
            } else {
                flameScale = *timerPtr * 0.01f;
            }

            flameIntensity = (*timerPtr - 25.0f) * 0.02f;
            flameIntensity = CLAMP(flameIntensity, 0.0f, 1.0f);
            EffectSsFireTail_SpawnFlameOnPlayer(globalCtx, flameScale, i, flameIntensity);
        }
    }

    if (spawnedFlame) {
        func_8002F7DC(&this->actor, NA_SE_EV_TORCH - SFX_FLAG);

        if (globalCtx->sceneNum == SCENE_JYASINBOSS) {
            dmgCooldown = 0;
        } else {
            dmgCooldown = 7;
        }

        if ((dmgCooldown & globalCtx->gameplayFrames) == 0) {
            Player_InflictDamage(globalCtx, -1);
        }
    } else {
        this->isBurning = false;
    }
}

void func_80848EF8(Player* this, GlobalContext* globalCtx) {
    if (CHECK_QUEST_ITEM(QUEST_STONE_OF_AGONY)) {
        f32 temp = 200000.0f - (this->stoneOfAgonyActorDistSq * 5.0f);

        if (temp < 0.0f) {
            temp = 0.0f;
        }

        this->stoneOfAgonyRumbleTimer += temp;

        /*Prevent it on horse, while jumping and on title screen.
        If you fly around no stone of agony for you! */
        if (CVar_GetS32("gVisualAgony", 0) != 0 && !this->stateFlags1) {
            s16 Top_Margins = (CVar_GetS32("gHUDMargin_T", 0) * -1);
            s16 Left_Margins = CVar_GetS32("gHUDMargin_L", 0);
            s16 Right_Margins = CVar_GetS32("gHUDMargin_R", 0);
            s16 X_Margins_VSOA;
            s16 Y_Margins_VSOA;
            if (CVar_GetS32("gVSOAUseMargins", 0) != 0) {
                if (CVar_GetS32("gVSOAPosType", 0) == 0) {
                    X_Margins_VSOA = Left_Margins;
                };
                Y_Margins_VSOA = Top_Margins;
            } else {
                X_Margins_VSOA = 0;
                Y_Margins_VSOA = 0;
            }
            s16 PosX_VSOA_ori = OTRGetRectDimensionFromLeftEdge(26) + X_Margins_VSOA;
            s16 PosY_VSOA_ori = 60 + Y_Margins_VSOA;
            s16 PosX_VSOA;
            s16 PosY_VSOA;
            if (CVar_GetS32("gVSOAPosType", 0) != 0) {
                PosY_VSOA = CVar_GetS32("gVSOAPosY", 0) + Y_Margins_VSOA;
                if (CVar_GetS32("gVSOAPosType", 0) == 1) { // Anchor Left
                    if (CVar_GetS32("gVSOAUseMargins", 0) != 0) {
                        X_Margins_VSOA = Left_Margins;
                    };
                    PosX_VSOA = OTRGetDimensionFromLeftEdge(CVar_GetS32("gVSOAPosX", 0) + X_Margins_VSOA);
                } else if (CVar_GetS32("gVSOAPosType", 0) == 2) { // Anchor Right
                    if (CVar_GetS32("gVSOAUseMargins", 0) != 0) {
                        X_Margins_VSOA = Right_Margins;
                    };
                    PosX_VSOA = OTRGetDimensionFromRightEdge(CVar_GetS32("gVSOAPosX", 0) + X_Margins_VSOA);
                } else if (CVar_GetS32("gVSOAPosType", 0) == 3) { // Anchor None
                    PosX_VSOA = CVar_GetS32("gVSOAPosX", 0);
                } else if (CVar_GetS32("gVSOAPosType", 0) == 4) { // Hidden
                    PosX_VSOA = -9999;
                }
            } else {
                PosY_VSOA = PosY_VSOA_ori;
                PosX_VSOA = PosX_VSOA_ori;
            }

            int rectLeft = PosX_VSOA; // Left X Pos
            int rectTop = PosY_VSOA;  // Top Y Pos
            int rectWidth = 24;       // Texture Width
            int rectHeight = 24;      // Texture Heigh
            int DefaultIconA = 50;    // Default icon alpha (55 on 255)

            OPEN_DISPS(globalCtx->state.gfxCtx);
            gDPPipeSync(OVERLAY_DISP++);

            if (CVar_GetS32("gHudColors", 1) == 2) {
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, CVar_GetS32("gCCVSOAPrimR", 255),
                                CVar_GetS32("gCCVSOAPrimG", 255), CVar_GetS32("gCCVSOAPrimB", 255), DefaultIconA);
            } else {
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, DefaultIconA);
            }

            gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0,
                              PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
            if (this->stoneOfAgonyRumbleTimer > 4000000.0f) {
                if (CVar_GetS32("gHudColors", 1) == 2) {
                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, CVar_GetS32("gCCVSOAPrimR", 255),
                                    CVar_GetS32("gCCVSOAPrimG", 255), CVar_GetS32("gCCVSOAPrimB", 255), 255);
                } else {
                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, 255);
                }
            } else {
                if (CVar_GetS32("gHudColors", 1) == 2) {
                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, CVar_GetS32("gCCVSOAPrimR", 255),
                                    CVar_GetS32("gCCVSOAPrimG", 255), CVar_GetS32("gCCVSOAPrimB", 255), DefaultIconA);
                } else {
                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, DefaultIconA);
                }
            }
            if (temp == 0 || temp <= 0.1f) {
                /*Fail check, it is used to draw off the icon when
                link is standing out range but do not refresh stoneOfAgonyRumbleTimer.
                Also used to make a default value in my case.*/
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, DefaultIconA);
            }
            gDPSetEnvColor(OVERLAY_DISP++, 0, 0, 0, 255);
            gDPSetOtherMode(OVERLAY_DISP++,
                            G_AD_DISABLE | G_CD_DISABLE | G_CK_NONE | G_TC_FILT | G_TF_POINT | G_TT_IA16 | G_TL_TILE |
                                G_TD_CLAMP | G_TP_NONE | G_CYC_1CYCLE | G_PM_NPRIMITIVE,
                            G_AC_NONE | G_ZS_PRIM | G_RM_XLU_SURF | G_RM_XLU_SURF2);
            gDPLoadTextureBlock(OVERLAY_DISP++, gStoneOfAgonyIconTex, G_IM_FMT_RGBA, G_IM_SIZ_32b, 24, 24, 0,
                                G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                G_TX_NOLOD, G_TX_NOLOD);
            gDPSetOtherMode(OVERLAY_DISP++,
                            G_AD_DISABLE | G_CD_DISABLE | G_CK_NONE | G_TC_FILT | G_TF_BILERP | G_TT_IA16 | G_TL_TILE |
                                G_TD_CLAMP | G_TP_NONE | G_CYC_1CYCLE | G_PM_NPRIMITIVE,
                            G_AC_NONE | G_ZS_PRIM | G_RM_XLU_SURF | G_RM_XLU_SURF2);
            gSPWideTextureRectangle(OVERLAY_DISP++, rectLeft << 2, rectTop << 2, (rectLeft + rectWidth) << 2,
                                    (rectTop + rectHeight) << 2, G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
            CLOSE_DISPS(globalCtx->state.gfxCtx);
        }

        if (this->stoneOfAgonyRumbleTimer > 4000000.0f) {
            this->stoneOfAgonyRumbleTimer = 0.0f;
            if (CVar_GetS32("gVisualAgony", 0) != 0 && !this->stateFlags1) {
                // This audio is placed here and not in previous CVar check to prevent ears ra.. :)
                Audio_PlaySoundGeneral(NA_SE_SY_MESSAGE_WOMAN, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E0);
            }
            Player_RequestRumble(this, 120, 20, 10, 0);
        }
    }
}

static s8 D_808547C4[] = {
    0,  3,  3,  5,   4,   8,   9,   13, 14, 15, 16, 17, 18, -22, 23, 24, 25,  26, 27,  28,  29, 31, 32, 33, 34, -35,
    30, 36, 38, -39, -40, -41, 42,  43, 45, 46, 0,  0,  0,  67,  48, 47, -50, 51, -52, -53, 54, 55, 56, 57, 58, 59,
    60, 61, 62, 63,  64,  -65, -66, 68, 11, 69, 70, 71, 8,  8,   72, 73, 78,  79, 80,  89,  90, 91, 92, 77, 19, 94,
};

static Vec3f D_80854814 = { 0.0f, 0.0f, 200.0f };

static f32 D_80854820[] = { 2.0f, 4.0f, 7.0f };
static f32 D_8085482C[] = { 0.5f, 1.0f, 3.0f };

void Player_SetupSwim(GlobalContext* globalCtx, Player* this, s16 yaw);

void Player_UpdateCommon(Player* this, GlobalContext* globalCtx, Input* input) {
    s32 pad;

    sControlInput = input;

    if (this->voidRespawnCounter < 0) {
        this->voidRespawnCounter++;
        if (this->voidRespawnCounter == 0) {
            this->voidRespawnCounter = 1;
            func_80078884(NA_SE_OC_REVENGE);
        }
    }

    static u8 inJail = false;
    static EnAObj* jail1 = NULL;
    static EnAObj* jail2 = NULL;
    static EnAObj* jail3 = NULL;
    static EnAObj* jail4 = NULL;
    static EnAObj* jail5 = NULL;

    #define PLAYER_JAIL_DIST 150

    if (CVar_GetS32("gJailTime", 0)) {
        if (!inJail) {
            jail1 = (EnAObj*)Actor_Spawn(&globalCtx->actorCtx, globalCtx, ACTOR_EN_A_OBJ,
                                         this->actor.world.pos.x + PLAYER_JAIL_DIST, this->actor.world.pos.y,
                                         this->actor.world.pos.z, 0, 0, 0, 5);
            jail2 = (EnAObj*)Actor_Spawn(&globalCtx->actorCtx, globalCtx, ACTOR_EN_A_OBJ,
                                         this->actor.world.pos.x - PLAYER_JAIL_DIST, this->actor.world.pos.y,
                                         this->actor.world.pos.z, 0, 0, 0, 5);
            jail3 =
                (EnAObj*)Actor_Spawn(&globalCtx->actorCtx, globalCtx, ACTOR_EN_A_OBJ, this->actor.world.pos.x,
                                     this->actor.world.pos.y, this->actor.world.pos.z + PLAYER_JAIL_DIST, 0, 0, 0, 5);
            jail4 =
                (EnAObj*)Actor_Spawn(&globalCtx->actorCtx, globalCtx, ACTOR_EN_A_OBJ, this->actor.world.pos.x,
                                     this->actor.world.pos.y, this->actor.world.pos.z - PLAYER_JAIL_DIST, 0, 0, 0, 5);
            jail5 =
                (EnAObj*)Actor_Spawn(&globalCtx->actorCtx, globalCtx, ACTOR_EN_A_OBJ, this->actor.world.pos.x,
                                     this->actor.world.pos.y - PLAYER_JAIL_DIST, this->actor.world.pos.z, 0, 0, 0, 5);
            inJail = true;
        }
    } else {
        if (inJail) {
            Actor_Kill(jail1);
            Actor_Kill(jail2);
            Actor_Kill(jail3);
            Actor_Kill(jail4);
            Actor_Kill(jail5);
            inJail = false;
        }
    }

    if (CVar_GetS32("gNaviSpam", 0)) {
        if (!(globalCtx->state.frames % 10)) {
            Audio_PlaySoundGeneral(NA_SE_VO_NAVY_CALL, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
        }
        if (!(globalCtx->state.frames % 24)) {
            Audio_PlaySoundGeneral(NA_SE_VO_NAVY_HELLO, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
        }
    }

    if (CVar_GetS32("gInvisPlayer", 0)) {
        this->actor.draw = NULL;
    }
    else {
        this->actor.draw = Player_Draw;
    }

    #define SCREEN_COLOR_RATE 15

    static u8 colorMode = 0;

    if (CVar_GetS32("gRaveMode", 0)) {
        switch (colorMode) {
            case 0:
                D_801614B0.r = 255;
                D_801614B0.g += SCREEN_COLOR_RATE;
                D_801614B0.b = 0;
                D_801614B0.a = 255;

                if (D_801614B0.g == 255) {
                    colorMode = 1;
                }
                break;

            case 1:
                D_801614B0.r -= SCREEN_COLOR_RATE;
                D_801614B0.g = 255;
                D_801614B0.b = 0;
                D_801614B0.a = 255;

                if (D_801614B0.r == 0) {
                    colorMode = 2;
                }
                break;
        
            case 2:
                D_801614B0.r = 0;
                D_801614B0.g = 255;
                D_801614B0.b += SCREEN_COLOR_RATE;
                D_801614B0.a = 255;

                if (D_801614B0.b == 255) {
                    colorMode = 3;
                }
                break;

            case 3:
                D_801614B0.r = 0;
                D_801614B0.g -= SCREEN_COLOR_RATE;
                D_801614B0.b = 255;
                D_801614B0.a = 255;

                if (D_801614B0.g == 0) {
                    colorMode = 4;
                }
                break;

            case 4:
                D_801614B0.r += SCREEN_COLOR_RATE;
                D_801614B0.g = 0;
                D_801614B0.b = 255;
                D_801614B0.a = 255;

                if (D_801614B0.r == 255) {
                    colorMode = 5;
                }
                break;

            case 5:
                D_801614B0.r = 255;
                D_801614B0.g = 0;
                D_801614B0.b -= SCREEN_COLOR_RATE;
                D_801614B0.a = 255;

                if (D_801614B0.b == 0) {
                    colorMode = 0;
                }
                break;
        }
    }
    else {
        D_801614B0.r = 0;
        D_801614B0.g = 0;
        D_801614B0.b = 0;
        D_801614B0.a = 0;
        colorMode = 0;
    }

    Math_Vec3f_Copy(&this->actor.prevPos, &this->actor.home.pos);

    if (this->fpsItemShotTimer != 0) {
        this->fpsItemShotTimer--;
    }

    if (this->endTalkTimer != 0) {
        this->endTalkTimer--;
    }

    if (this->deathTimer != 0) {
        this->deathTimer--;
    }

    if (this->invincibilityTimer < 0) {
        this->invincibilityTimer++;
    } else if (this->invincibilityTimer > 0) {
        this->invincibilityTimer--;
    }

    if (this->runDamageTimer != 0) {
        this->runDamageTimer--;
    }

    func_808473D4(globalCtx, this);
    Player_SetupZTargeting(this, globalCtx);


    if ((this->heldItemActionParam == PLAYER_AP_STICK) && (this->fpsItemType != 0)) {
        func_80848A04(globalCtx, this);
    } else if ((this->heldItemActionParam == PLAYER_AP_FISHING_POLE) && (this->fpsItemType < 0)) {
        this->fpsItemType++;
    }

    if (this->shockTimer != 0) {
        func_80848B44(globalCtx, this);
    }

    if (this->isBurning) {
        func_80848C74(globalCtx, this);
    }

    if ((this->stateFlags3 & PLAYER_STATE3_RESTORE_NAYRUS_LOVE) && (gSaveContext.nayrusLoveTimer != 0) &&
        (gSaveContext.unk_13F0 == 0)) {
        gSaveContext.unk_13F0 = 3;
        func_80846A00(globalCtx, this, 1);
        this->stateFlags3 &= ~PLAYER_STATE3_RESTORE_NAYRUS_LOVE;
    }

    if (this->stateFlags2 & PLAYER_STATE2_PAUSE_MOST_UPDATING || CVar_GetS32("gOnHold", 0)) {
        if (!(this->actor.bgCheckFlags & 1)) {
            Player_StopMovement(this);
            Actor_MoveForward(&this->actor);
        }

        func_80847BA0(globalCtx, this);
    } else {
        f32 temp_f0;
        f32 phi_f12;

        if (this->currentBoots != this->prevBoots) {
            if (this->currentBoots == PLAYER_BOOTS_IRON) {
                if (this->stateFlags1 & PLAYER_STATE1_SWIMMING) {
                    Player_ResetSubCam(globalCtx, this);
                    if (this->ageProperties->unk_2C < this->actor.yDistToWater) {
                        this->stateFlags2 |= PLAYER_STATE2_DIVING;
                    }
                }
            } else {
                if (this->stateFlags1 & PLAYER_STATE1_SWIMMING) {
                    if ((this->prevBoots == PLAYER_BOOTS_IRON) || (this->actor.bgCheckFlags & 1)) {
                        func_8083D36C(globalCtx, this);
                        this->stateFlags2 &= ~PLAYER_STATE2_DIVING;
                    }
                }
            }

            this->prevBoots = this->currentBoots;
        }

        if ((this->actor.parent == NULL) && (this->stateFlags1 & PLAYER_STATE1_RIDING_HORSE)) {
            this->actor.parent = this->rideActor;
            Player_SetupRideHorse(globalCtx, this);
            this->stateFlags1 |= PLAYER_STATE1_RIDING_HORSE;
            Player_PlayAnimOnce(globalCtx, this, &gPlayerAnim_0033B8);
            Player_SetupAnimMovement(globalCtx, this, 0x9B);
            this->genericTimer = 99;
        }

        if (this->comboTimer == 0) {
            this->slashCounter = 0;
        } else if (this->comboTimer < 0) {
            this->comboTimer++;
        } else {
            this->comboTimer--;
        }

        Math_ScaledStepToS(&this->shapePitchOffset, 0, 400);
        func_80032CB4(this->unk_3A8, 20, 80, 6);

        this->actor.shape.face = this->unk_3A8[0] + ((globalCtx->gameplayFrames & 32) ? 0 : 3);

        if (this->currentMask == PLAYER_MASK_BUNNY) {
            Player_BunnyHoodPhysics(this);
        }

        if (Actor_PlayerIsAimingFpsItem(this) != 0) {
            Player_BowStringMoveAfterShot(this);
        }

        if (!(this->skelAnime.moveFlags & 0x80)) {
            if (((this->actor.bgCheckFlags & 1) && (((sFloorSpecialProperty == 5) &&
                 (this->currentBoots != PLAYER_BOOTS_IRON)) ||
                  ((this->currentBoots == PLAYER_BOOTS_HOVER)) || CVar_GetS32("gSlipperyFloor", 0)) &&
                 !(this->stateFlags1 & (PLAYER_STATE1_SWIMMING | PLAYER_STATE1_IN_CUTSCENE)))) {
                f32 sp70 = this->linearVelocity;
                s16 sp6E = this->currentYaw;
                s16 yawDiff = this->actor.world.rot.y - sp6E;
                s32 pad;

                if ((ABS(yawDiff) > 0x6000) && (this->actor.speedXZ != 0.0f)) {
                    sp70 = 0.0f;
                    sp6E += 0x8000;
                }

                if (Math_StepToF(&this->actor.speedXZ, sp70, CVar_GetS32("gSlipperyFloor", 0) ? 0.15f: 0.35f) &&
                    (sp70 == 0.0f)) {
                    this->actor.world.rot.y = this->currentYaw;
                }

                if (this->linearVelocity != 0.0f) {
                    s32 phi_v0;

                    phi_v0 = (fabsf(this->linearVelocity) * 700.0f) - (fabsf(this->actor.speedXZ) * 100.0f);
                    phi_v0 = CLAMP(phi_v0, 0, 1350);

                    Math_ScaledStepToS(&this->actor.world.rot.y, sp6E, phi_v0);
                }

                if ((this->linearVelocity == 0.0f) && (this->actor.speedXZ != 0.0f)) {
                    func_800F4138(&this->actor.projectedPos, 0xD0, this->actor.speedXZ);
                }
            } else {
                this->actor.speedXZ = this->linearVelocity;
                this->actor.world.rot.y = this->currentYaw;
            }

            func_8002D868(&this->actor);

            if ((this->pushedSpeed != 0.0f) && !Player_InCsMode(globalCtx) &&
                !(this->stateFlags1 & (PLAYER_STATE1_HANGING_FROM_LEDGE_SLIP | PLAYER_STATE1_CLIMBING_ONTO_LEDGE |
                                       PLAYER_STATE1_CLIMBING)) &&
                (Player_JumpUpToLedge != this->actionFunc) && (Player_UpdateMagicSpell != this->actionFunc)) {
                this->actor.velocity.x += this->pushedSpeed * Math_SinS(this->pushedYaw);
                this->actor.velocity.z += this->pushedSpeed * Math_CosS(this->pushedYaw);
            }

            func_8002D7EC(&this->actor);
            func_80847BA0(globalCtx, this);
        } else {
            sFloorSpecialProperty = 0;
            this->floorProperty = 0;

            if (!(this->stateFlags1 & PLAYER_STATE1_EXITING_SCENE) &&
                (this->stateFlags1 & PLAYER_STATE1_RIDING_HORSE)) {
                EnHorse* rideActor = (EnHorse*)this->rideActor;
                CollisionPoly* sp5C;
                s32 sp58;
                Vec3f sp4C;

                if (!(rideActor->actor.bgCheckFlags & 1)) {
                    Player_RaycastFloorWithOffset(globalCtx, this, &D_80854814, &sp4C, &sp5C, &sp58);
                } else {
                    sp5C = rideActor->actor.floorPoly;
                    sp58 = rideActor->actor.floorBgId;
                }

                if ((sp5C != NULL) && Player_SetupExit(globalCtx, this, sp5C, sp58)) {
                    if (DREG(25) != 0) {
                        DREG(25) = 0;
                    } else {
                        AREG(6) = 1;
                    }
                }
            }

            sConveyorSpeedIndex = 0;
            this->pushedSpeed = 0.0f;
        }

        if ((sConveyorSpeedIndex != 0) && (this->currentBoots != PLAYER_BOOTS_IRON)) {
            f32 sp48;

            sConveyorSpeedIndex--;

            if (sIsFloorConveyor == 0) {
                sp48 = D_80854820[sConveyorSpeedIndex];

                if (!(this->stateFlags1 & PLAYER_STATE1_SWIMMING)) {
                    sp48 *= 0.25f;
                }
            } else {
                sp48 = D_8085482C[sConveyorSpeedIndex];
            }

            Math_StepToF(&this->pushedSpeed, sp48, sp48 * 0.1f);

            Math_ScaledStepToS(&this->pushedYaw, sConveyorYaw,
                               ((this->stateFlags1 & PLAYER_STATE1_SWIMMING) ? 400.0f : 800.0f) * sp48);
        } else if (this->pushedSpeed != 0.0f) {
            Math_StepToF(&this->pushedSpeed, 0.0f, (this->stateFlags1 & PLAYER_STATE1_SWIMMING) ? 0.5f : 1.0f);
        }

        if (!Player_InBlockingCsMode(globalCtx, this) && !(this->stateFlags2 & PLAYER_STATE2_INSIDE_CRAWLSPACE)) {
            func_8083D53C(globalCtx, this);

            if ((this->actor.category == ACTORCAT_PLAYER) && (gSaveContext.health == 0)) {
                if (this->stateFlags1 & (PLAYER_STATE1_HANGING_FROM_LEDGE_SLIP | PLAYER_STATE1_CLIMBING_ONTO_LEDGE |
                                         PLAYER_STATE1_CLIMBING)) {
                    Player_ResetAttributes(globalCtx, this);
                    Player_SetupFallFromLedge(this, globalCtx);
                } else if ((this->actor.bgCheckFlags & 1) || (this->stateFlags1 & PLAYER_STATE1_SWIMMING)) {
                    Player_SetupDie(globalCtx, this,
                                    Player_IsSwimming(this)   ? &gPlayerAnim_003310
                                    : (this->shockTimer != 0) ? &gPlayerAnim_002F08
                                                              : &gPlayerAnim_002878);
                }
            } else {
                if ((this->actor.parent == NULL) && ((globalCtx->sceneLoadFlag == 0x14) || (this->deathTimer != 0) ||
                                                     !Player_UpdateDamage(this, globalCtx))) {
                    Player_SetupMidairBehavior(this, globalCtx);
                } else {
                    this->fallStartHeight = this->actor.world.pos.y;
                }
                func_80848EF8(this, globalCtx);
            }
        }

        if ((globalCtx->csCtx.state != CS_STATE_IDLE) && (this->csMode != 6) &&
            !(this->stateFlags1 & PLAYER_STATE1_RIDING_HORSE) &&
            !(this->stateFlags2 & PLAYER_STATE2_RESTRAINED_BY_ENEMY) && (this->actor.category == ACTORCAT_PLAYER)) {
            CsCmdActorAction* linkActionCsCmd = globalCtx->csCtx.linkAction;

            if ((linkActionCsCmd != NULL) && (D_808547C4[linkActionCsCmd->action] != 0)) {
                func_8002DF54(globalCtx, NULL, 6);
                Player_StopMovement(this);
            } else if ((this->csMode == 0) && !(this->stateFlags2 & PLAYER_STATE2_DIVING) &&
                       (globalCtx->csCtx.state != CS_STATE_UNSKIPPABLE_INIT)) {
                func_8002DF54(globalCtx, NULL, 0x31);
                Player_StopMovement(this);
            }
        }

        if (this->csMode != 0) {
            if ((this->csMode != 7) ||
                !(this->stateFlags1 & (PLAYER_STATE1_HANGING_FROM_LEDGE_SLIP | PLAYER_STATE1_CLIMBING_ONTO_LEDGE |
                                       PLAYER_STATE1_CLIMBING | PLAYER_STATE1_TAKING_DAMAGE))) {
                this->attentionMode = 3;
            } else if (Player_StartCutscene != this->actionFunc) {
                Player_CutsceneEnd(globalCtx, this, NULL);
            }
        } else {
            this->prevCsMode = 0;
        }

        func_8083D6EC(globalCtx, this);

        if ((this->targetActor == NULL) && (this->naviTextId == 0)) {
            this->stateFlags2 &= ~(PLAYER_STATE2_CAN_SPEAK_OR_CHECK | PLAYER_STATE2_NAVI_REQUESTING_TALK);
        }

        this->stateFlags1 &= ~(PLAYER_STATE1_SWINGING_BOTTLE | PLAYER_STATE1_READY_TO_SHOOT |
                               PLAYER_STATE1_CHARGING_SPIN_ATTACK | PLAYER_STATE1_SHIELDING);
        this->stateFlags2 &=
            ~(PLAYER_STATE2_CAN_GRAB_PUSH_PULL_WALL | PLAYER_STATE2_CAN_CLIMB_PUSH_PULL_WALL |
              PLAYER_STATE2_MAKING_REACTABLE_NOISE | PLAYER_STATE2_DISABLE_MOVE_ROTATION_WHILE_Z_TARGETING |
              PLAYER_STATE2_ALWAYS_DISABLE_MOVE_ROTATION | PLAYER_STATE2_ENABLE_PUSH_PULL_CAM |
              PLAYER_STATE2_SPAWN_DUST_AT_FEET | PLAYER_STATE2_IDLE_WHILE_CLIMBING | PLAYER_STATE2_FROZEN_IN_ICE |
              PLAYER_STATE2_CAN_ENTER_CRAWLSPACE | PLAYER_STATE2_CAN_DISMOUNT_HORSE | PLAYER_STATE2_ENABLE_REFLECTION);
        this->stateFlags3 &= ~PLAYER_STATE3_CHECKING_FLOOR_AND_WATER_COLLISION;

        if (CVar_GetS32("gDisableTurning", 0)) {
            this->stateFlags2 |= PLAYER_STATE2_ALWAYS_DISABLE_MOVE_ROTATION;
        }

        func_80847298(this);
        Player_StoreAnalogStickInput(globalCtx, this);

        if (this->stateFlags1 & PLAYER_STATE1_SWIMMING) {
            sWaterSpeedScale = 0.5f;
        } else {
            sWaterSpeedScale = 1.0f;
        }

        sInvertedWaterSpeedScale = 1.0f / sWaterSpeedScale;
        sUsingItemAlreadyInHand = sUsingItemAlreadyInHand2 = 0;
        sCurrentMask = this->currentMask;

        if (!(this->stateFlags3 & PLAYER_STATE3_PAUSE_ACTION_FUNC)) {
            this->actionFunc(this, globalCtx);
        }

        Player_UpdateCamAndSeqModes(globalCtx, this);

        if (this->skelAnime.moveFlags & 8) {
            AnimationContext_SetMoveActor(globalCtx, &this->actor, &this->skelAnime,
                                          (this->skelAnime.moveFlags & 4) ? 1.0f : this->ageProperties->unk_08);
        }

        Player_UpdateYaw(this, globalCtx);

        if (CHECK_FLAG_ALL(this->actor.flags, ACTOR_FLAG_8)) {
            this->talkActorDistance = 0.0f;
        } else {
            this->talkActor = NULL;
            this->talkActorDistance = FLT_MAX;
            this->exchangeItemId = EXCH_ITEM_NONE;
        }

        if (!(this->stateFlags1 & PLAYER_STATE1_HOLDING_ACTOR)) {
            this->interactRangeActor = NULL;
            this->getItemDirection = 0x6000;
        }

        if (this->actor.parent == NULL) {
            this->rideActor = NULL;
        }

        this->naviTextId = 0;

        if (!(this->stateFlags2 & PLAYER_STATE2_PLAYING_OCARINA_FOR_ACTOR)) {
            this->ocarinaActor = NULL;
        }

        this->stateFlags2 &= ~PLAYER_STATE2_NEAR_OCARINA_ACTOR;
        this->stoneOfAgonyActorDistSq = FLT_MAX;

        temp_f0 = this->actor.world.pos.y - this->actor.prevPos.y;

        this->doorType = PLAYER_DOORTYPE_NONE;
        this->damageEffect = 0;
        this->forcedTargetActor = NULL;

        phi_f12 =
            ((this->bodyPartsPos[PLAYER_BODYPART_L_FOOT].y + this->bodyPartsPos[PLAYER_BODYPART_R_FOOT].y) * 0.5f) +
            temp_f0;
        temp_f0 += this->bodyPartsPos[PLAYER_BODYPART_HEAD].y + 10.0f;

        this->cylinder.dim.height = temp_f0 - phi_f12;

        if (this->cylinder.dim.height < 0) {
            phi_f12 = temp_f0;
            this->cylinder.dim.height = -this->cylinder.dim.height;
        }

        this->cylinder.dim.yShift = phi_f12 - this->actor.world.pos.y;

        if (this->stateFlags1 & PLAYER_STATE1_SHIELDING) {
            this->cylinder.dim.height = this->cylinder.dim.height * 0.8f;
        }

        Collider_UpdateCylinder(&this->actor, &this->cylinder);

        if (!(this->stateFlags2 & PLAYER_STATE2_FROZEN_IN_ICE)) {
            if (!(this->stateFlags1 & (PLAYER_STATE1_IN_DEATH_CUTSCENE | PLAYER_STATE1_HANGING_FROM_LEDGE_SLIP |
                                       PLAYER_STATE1_CLIMBING_ONTO_LEDGE | PLAYER_STATE1_RIDING_HORSE))) {
                CollisionCheck_SetOC(globalCtx, &globalCtx->colChkCtx, &this->cylinder.base);
            }

            if (!(this->stateFlags1 & (PLAYER_STATE1_IN_DEATH_CUTSCENE | PLAYER_STATE1_TAKING_DAMAGE)) &&
                (this->invincibilityTimer <= 0)) {
                CollisionCheck_SetAC(globalCtx, &globalCtx->colChkCtx, &this->cylinder.base);

                if (this->invincibilityTimer < 0) {
                    CollisionCheck_SetAT(globalCtx, &globalCtx->colChkCtx, &this->cylinder.base);
                }
            }
        }

        AnimationContext_SetNextQueue(globalCtx);
    }

    Math_Vec3f_Copy(&this->actor.home.pos, &this->actor.world.pos);
    Math_Vec3f_Copy(&this->prevWaistPos, &this->bodyPartsPos[PLAYER_BODYPART_WAIST]);

    if (this->stateFlags1 &
        (PLAYER_STATE1_IN_DEATH_CUTSCENE | PLAYER_STATE1_SKIP_OTHER_ACTORS_UPDATE | PLAYER_STATE1_IN_CUTSCENE)) {
        this->actor.colChkInfo.mass = MASS_IMMOVABLE;
    } else {
        this->actor.colChkInfo.mass = 50;
    }

    this->stateFlags3 &= ~PLAYER_STATE3_PAUSE_ACTION_FUNC;

    if (CVar_GetS32("gMegaLetterbox", 0)) {
        ShrinkWindow_SetVal(110);
    }

    if (CVar_GetS32("gPlayerGravity", 0) != 0) {
        this->actor.gravity += CVar_GetS32("gPlayerGravity", 0) * 0.1f;
    }

    Collider_ResetCylinderAC(globalCtx, &this->cylinder.base);

    Collider_ResetQuadAT(globalCtx, &this->swordQuads[0].base);
    Collider_ResetQuadAT(globalCtx, &this->swordQuads[1].base);

    Collider_ResetQuadAC(globalCtx, &this->shieldQuad.base);
    Collider_ResetQuadAT(globalCtx, &this->shieldQuad.base);
}

static Vec3f D_80854838 = { 0.0f, 0.0f, -30.0f };

void Player_Update(Actor* thisx, GlobalContext* globalCtx) {
    static Vec3f sDogSpawnPos;
    Player* this = (Player*)thisx;
    s32 dogParams;
    s32 pad;
    Input sp44;
    Actor* dog;

    if (Player_CheckNoDebugModeCombo(this, globalCtx)) {
        if (gSaveContext.dogParams < 0) {
            if (Object_GetIndex(&globalCtx->objectCtx, OBJECT_DOG) < 0) {
                gSaveContext.dogParams = 0;
            } else {
                gSaveContext.dogParams &= 0x7FFF;
                Player_GetWorldPosRelativeToPlayer(this, &this->actor.world.pos, &D_80854838, &sDogSpawnPos);
                dogParams = gSaveContext.dogParams;

                dog = Actor_Spawn(&globalCtx->actorCtx, globalCtx, ACTOR_EN_DOG, sDogSpawnPos.x, sDogSpawnPos.y,
                                  sDogSpawnPos.z, 0, this->actor.shape.rot.y, 0, dogParams | 0x8000);
                if (dog != NULL) {
                    dog->room = 0;
                }
            }
        }

        if ((this->interactRangeActor != NULL) && (this->interactRangeActor->update == NULL)) {
            this->interactRangeActor = NULL;
        }

        if ((this->heldActor != NULL) && (this->heldActor->update == NULL)) {
            Player_DetatchHeldActor(globalCtx, this);
        }

        if (this->stateFlags1 & (PLAYER_STATE1_INPUT_DISABLED | PLAYER_STATE1_IN_CUTSCENE)) {
            memset(&sp44, 0, sizeof(sp44));
        } else {
            sp44 = globalCtx->state.input[0];
            if (this->endTalkTimer != 0) {
                sp44.cur.button &= ~(BTN_A | BTN_B | BTN_CUP);
                sp44.press.button &= ~(BTN_A | BTN_B | BTN_CUP);
            }
        }

        Player_UpdateCommon(this, globalCtx, &sp44);
    }

    MREG(52) = this->actor.world.pos.x;
    MREG(53) = this->actor.world.pos.y;
    MREG(54) = this->actor.world.pos.z;
    MREG(55) = this->actor.world.rot.y;
}

static struct_80858AC8 D_80858AC8;
static Vec3s D_80858AD8[25];

static Gfx* sMaskDlists[PLAYER_MASK_MAX - 1] = {
    gLinkChildKeatonMaskDL, gLinkChildSkullMaskDL, gLinkChildSpookyMaskDL, gLinkChildBunnyHoodDL,
    gLinkChildGoronMaskDL,  gLinkChildZoraMaskDL,  gLinkChildGerudoMaskDL, gLinkChildMaskOfTruthDL,
};

static Vec3s D_80854864 = { 0, 0, 0 };

void Player_DrawGameplay(GlobalContext* globalCtx, Player* this, s32 lod, Gfx* cullDList,
                         OverrideLimbDrawOpa overrideLimbDraw) {
    static s32 D_8085486C = 255;

    OPEN_DISPS(globalCtx->state.gfxCtx);

    gSPSegment(POLY_OPA_DISP++, 0x0C, cullDList);
    gSPSegment(POLY_XLU_DISP++, 0x0C, cullDList);

    func_8008F470(globalCtx, this->skelAnime.skeleton, this->skelAnime.jointTable, this->skelAnime.dListCount, lod,
                  this->currentTunic, this->currentBoots, this->actor.shape.face, overrideLimbDraw, func_80090D20,
                  this);

    if ((overrideLimbDraw == func_80090014) && (this->currentMask != PLAYER_MASK_NONE)) {
        Mtx* sp70 = Graph_Alloc(globalCtx->state.gfxCtx, 2 * sizeof(Mtx));

        if (this->currentMask == PLAYER_MASK_BUNNY) {
            Vec3s sp68;

            gSPSegment(POLY_OPA_DISP++, 0x0B, sp70);

            sp68.x = D_80858AC8.unk_02 + 0x3E2;
            sp68.y = D_80858AC8.unk_04 + 0xDBE;
            sp68.z = D_80858AC8.unk_00 - 0x348A;
            Matrix_SetTranslateRotateYXZ(97.0f, -1203.0f, -240.0f, &sp68);
            MATRIX_TOMTX(sp70++);

            sp68.x = D_80858AC8.unk_02 - 0x3E2;
            sp68.y = -0xDBE - D_80858AC8.unk_04;
            sp68.z = D_80858AC8.unk_00 - 0x348A;
            Matrix_SetTranslateRotateYXZ(97.0f, -1203.0f, 240.0f, &sp68);
            MATRIX_TOMTX(sp70);
        }

        gSPDisplayList(POLY_OPA_DISP++, sMaskDlists[this->currentMask - 1]);
    }

    if ((this->currentBoots == PLAYER_BOOTS_HOVER) && !(this->actor.bgCheckFlags & 1) &&
        !(this->stateFlags1 & PLAYER_STATE1_RIDING_HORSE) && (this->hoverBootsTimer != 0)) {
        s32 sp5C;
        s32 hoverBootsTimer = this->hoverBootsTimer;

        if (this->hoverBootsTimer < 19) {
            if (hoverBootsTimer >= 15) {
                D_8085486C = (19 - hoverBootsTimer) * 51.0f;
            } else if (hoverBootsTimer < 19) {
                sp5C = hoverBootsTimer;

                if (sp5C > 9) {
                    sp5C = 9;
                }

                D_8085486C = (-sp5C * 4) + 36;
                D_8085486C = D_8085486C * D_8085486C;
                D_8085486C = (s32)((Math_CosS(D_8085486C) * 100.0f) + 100.0f) + 55.0f;
                D_8085486C = D_8085486C * (sp5C * (1.0f / 9.0f));
            }

            Matrix_SetTranslateRotateYXZ(this->actor.world.pos.x, this->actor.world.pos.y + 2.0f,
                                         this->actor.world.pos.z, &D_80854864);
            Matrix_Scale(4.0f, 4.0f, 4.0f, MTXMODE_APPLY);

            gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(globalCtx->state.gfxCtx),
                      G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPSegment(POLY_XLU_DISP++, 0x08,
                       Gfx_TwoTexScroll(globalCtx->state.gfxCtx, 0, 0, 0, 16, 32, 1, 0,
                                        (globalCtx->gameplayFrames * -15) % 128, 16, 32));
            gDPSetPrimColor(POLY_XLU_DISP++, 0x80, 0x80, 255, 255, 255, D_8085486C);
            gDPSetEnvColor(POLY_XLU_DISP++, 120, 90, 30, 128);
            gSPDisplayList(POLY_XLU_DISP++, gHoverBootsCircleDL);
        }
    }

    CLOSE_DISPS(globalCtx->state.gfxCtx);
}

void Player_Draw(Actor* thisx, GlobalContext* globalCtx2) {
    GlobalContext* globalCtx = globalCtx2;
    Player* this = (Player*)thisx;

    Vec3f pos;
    Vec3s rot;
    f32 scale;

    // OTRTODO: This is crashing randomly, so its temporarily been disabled
    // return;

    if (LINK_AGE_IN_YEARS == YEARS_CHILD) {
        pos.x = 2.0f;
        pos.y = -130.0f;
        pos.z = -150.0f;
        scale = 0.046f;
    } else if (CUR_EQUIP_VALUE(EQUIP_SWORD) != 2) {
        pos.x = 25.0f;
        pos.y = -228.0f;
        pos.z = 60.0f;
        scale = 0.056f;
    } else {
        pos.x = 20.0f;
        pos.y = -180.0f;
        pos.z = -40.0f;
        scale = 0.047f;
    }

    rot.y = 32300;
    rot.x = rot.z = 0;

    OPEN_DISPS(globalCtx->state.gfxCtx);

    if (!(this->stateFlags2 & PLAYER_STATE2_DISABLE_DRAW)) {
        OverrideLimbDrawOpa overrideLimbDraw = func_80090014;
        s32 lod;
        s32 pad;

        if ((this->csMode != 0) || (Player_IsUnfriendlyZTargeting(this) && 0) ||
            (this->actor.projectedPos.z < 160.0f)) {
            lod = 0;
        } else {
            lod = 1;
        }

        if (CVar_GetS32("gScuffedLink", 0)) {
            lod = 1;
        } else if (CVar_GetS32("gDisableLOD", 0) != 0) {
            lod = 0;
        }

        func_80093C80(globalCtx);
        func_80093D84(globalCtx->state.gfxCtx);

        if (this->invincibilityTimer > 0) {
            this->damageFlashTimer += CLAMP(50 - this->invincibilityTimer, 8, 40);
            POLY_OPA_DISP = Gfx_SetFog2(POLY_OPA_DISP, 255, 0, 0, 0, 0,
                                        4000 - (s32)(Math_CosS(this->damageFlashTimer * 256) * 2000.0f));
        }

        func_8002EBCC(&this->actor, globalCtx, 0);
        func_8002ED80(&this->actor, globalCtx, 0);

        if (this->attentionMode != 0) {
            Vec3f projectedHeadPos;

            SkinMatrix_Vec3fMtxFMultXYZ(&globalCtx->viewProjectionMtxF, &this->actor.focus.pos, &projectedHeadPos);
            if (projectedHeadPos.z < -4.0f) {
                overrideLimbDraw = func_800902F0;
            }
        } else if (this->stateFlags2 & PLAYER_STATE2_INSIDE_CRAWLSPACE) {
            if (this->actor.projectedPos.z < 0.0f) {
                overrideLimbDraw = func_80090440;
            }
        }

        if (this->stateFlags2 & PLAYER_STATE2_ENABLE_REFLECTION) {
            f32 sp78 = ((u16)(globalCtx->gameplayFrames * 600) * M_PI) / 0x8000;
            f32 sp74 = ((u16)(globalCtx->gameplayFrames * 1000) * M_PI) / 0x8000;

            Matrix_Push();
            this->actor.scale.y = -this->actor.scale.y;
            Matrix_SetTranslateRotateYXZ(
                this->actor.world.pos.x,
                (this->actor.floorHeight + (this->actor.floorHeight - this->actor.world.pos.y)) +
                    (this->actor.shape.yOffset * this->actor.scale.y),
                this->actor.world.pos.z, &this->actor.shape.rot);
            Matrix_Scale(this->actor.scale.x, this->actor.scale.y, this->actor.scale.z, MTXMODE_APPLY);
            Matrix_RotateX(sp78, MTXMODE_APPLY);
            Matrix_RotateY(sp74, MTXMODE_APPLY);
            Matrix_Scale(1.1f, 0.95f, 1.05f, MTXMODE_APPLY);
            Matrix_RotateY(-sp74, MTXMODE_APPLY);
            Matrix_RotateX(-sp78, MTXMODE_APPLY);
            Player_DrawGameplay(globalCtx, this, lod, gCullFrontDList, overrideLimbDraw);
            this->actor.scale.y = -this->actor.scale.y;
            Matrix_Pop();
        }

        gSPClearGeometryMode(POLY_OPA_DISP++, G_CULL_BOTH);
        gSPClearGeometryMode(POLY_XLU_DISP++, G_CULL_BOTH);

        Player_DrawGameplay(globalCtx, this, lod, gCullBackDList, overrideLimbDraw);

        if (this->invincibilityTimer > 0) {
            POLY_OPA_DISP = Gameplay_SetFog(globalCtx, POLY_OPA_DISP);
        }

        if (this->stateFlags2 & PLAYER_STATE2_FROZEN_IN_ICE) {
            f32 scale = (this->genericVar >> 1) * 22.0f;

            gSPSegment(POLY_XLU_DISP++, 0x08,
                       Gfx_TwoTexScroll(globalCtx->state.gfxCtx, 0, 0, (0 - globalCtx->gameplayFrames) % 128, 32, 32, 1,
                                        0, (globalCtx->gameplayFrames * -2) % 128, 32, 32));

            Matrix_Scale(scale, scale, scale, MTXMODE_APPLY);
            gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(globalCtx->state.gfxCtx),
                      G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gDPSetEnvColor(POLY_XLU_DISP++, 0, 50, 100, 255);
            gSPDisplayList(POLY_XLU_DISP++, gEffIceFragment3DL);
        }

        if (this->giDrawIdPlusOne > 0) {
            Player_DrawGetItem(globalCtx, this);
        }
    }

    CLOSE_DISPS(globalCtx->state.gfxCtx);
}

void Player_Destroy(Actor* thisx, GlobalContext* globalCtx) {
    Player* this = (Player*)thisx;

    Effect_Delete(globalCtx, this->swordEffectIndex);

    Collider_DestroyCylinder(globalCtx, &this->cylinder);
    Collider_DestroyQuad(globalCtx, &this->swordQuads[0]);
    Collider_DestroyQuad(globalCtx, &this->swordQuads[1]);
    Collider_DestroyQuad(globalCtx, &this->shieldQuad);

    func_800876C8(globalCtx);

    gSaveContext.linkAge = globalCtx->linkAgeOnLoad;
}

s16 func_8084ABD8(GlobalContext* globalCtx, Player* this, s32 arg2, s16 arg3) {
    s32 temp1;
    s16 temp2;
    s16 temp3;

    if (!Actor_PlayerIsAimingReadyFpsItem(this) && !Player_IsAimingReadyBoomerang(this) && (arg2 == 0)) {
        temp2 = sControlInput->rel.stick_y * 240.0f;
        Math_SmoothStepToS(&this->actor.focus.rot.x, temp2, 14, 4000, 30);

        temp2 = sControlInput->rel.stick_x * -16.0f;
        temp2 = CLAMP(temp2, -3000, 3000);
        this->actor.focus.rot.y += temp2;
    } else {
        temp1 = (this->stateFlags1 & PLAYER_STATE1_RIDING_HORSE) ? 3500 : 14000;
        temp3 = ((sControlInput->rel.stick_y >= 0) ? 1 : -1) *
                (s32)((1.0f - Math_CosS(sControlInput->rel.stick_y * 200)) * 1500.0f);
        this->actor.focus.rot.x += temp3;

        if (fabsf(sControlInput->cur.gyro_x) > 0.01f) {
            this->actor.focus.rot.x -= (sControlInput->cur.gyro_x) * 750.0f;
        }

        this->actor.focus.rot.x = CLAMP(this->actor.focus.rot.x, -temp1, temp1);

        temp1 = 19114;
        temp2 = this->actor.focus.rot.y - this->actor.shape.rot.y;
        temp3 = ((sControlInput->rel.stick_x >= 0) ? 1 : -1) *
                (s32)((1.0f - Math_CosS(sControlInput->rel.stick_x * 200)) * -1500.0f);
        temp2 += temp3;

        this->actor.focus.rot.y = CLAMP(temp2, -temp1, temp1) + this->actor.shape.rot.y;

        if (fabsf(sControlInput->cur.gyro_y) > 0.01f) {
            this->actor.focus.rot.y += (sControlInput->cur.gyro_y) * 750.0f;
        }
    }

    this->lookFlags |= 2;
    return Player_UpdateLookAngles(this, (globalCtx->shootingGalleryStatus != 0) || Actor_PlayerIsAimingReadyFpsItem(this) ||
                                             Player_IsAimingReadyBoomerang(this)) -
           arg3;
}

void Player_UpdateSwimMovement(Player* this, f32* arg1, f32 arg2, s16 arg3) {
    f32 temp1;
    f32 temp2;

    temp1 = this->skelAnime.curFrame - 10.0f;

    temp2 = (R_RUN_SPEED_LIMIT / 100.0f) * 0.8f;
    if (*arg1 > temp2) {
        *arg1 = temp2;
    }

    if ((0.0f < temp1) && (temp1 < 10.0f)) {
        temp1 *= 6.0f;
    } else {
        temp1 = 0.0f;
        arg2 = 0.0f;
    }

    Math_AsymStepToF(arg1, arg2 * 0.8f, temp1, (fabsf(*arg1) * 0.02f) + 0.05f);
    Math_ScaledStepToS(&this->currentYaw, arg3, 1600);
}

void func_8084B000(Player* this) {
    f32 phi_f18;
    f32 phi_f16;
    f32 phi_f14;
    f32 yDistToWater;

    phi_f14 = -5.0f;

    phi_f16 = this->ageProperties->unk_28;
    if (this->actor.velocity.y < 0.0f) {
        phi_f16 += 1.0f;
    }

    if (this->actor.yDistToWater < phi_f16) {
        if (this->actor.velocity.y <= 0.0f) {
            phi_f16 = 0.0f;
        } else {
            phi_f16 = this->actor.velocity.y * 0.5f;
        }
        phi_f18 = -0.1f - phi_f16;
    } else {
        if (!(this->stateFlags1 & PLAYER_STATE1_IN_DEATH_CUTSCENE) && (this->currentBoots == PLAYER_BOOTS_IRON) &&
            (this->actor.velocity.y >= -3.0f)) {
            phi_f18 = -0.2f;
        } else {
            phi_f14 = 2.0f;
            if (this->actor.velocity.y >= 0.0f) {
                phi_f16 = 0.0f;
            } else {
                phi_f16 = this->actor.velocity.y * -0.3f;
            }
            phi_f18 = phi_f16 + 0.1f;
        }

        yDistToWater = this->actor.yDistToWater;
        if (yDistToWater > 100.0f) {
            this->stateFlags2 |= PLAYER_STATE2_DIVING;
        }
    }

    this->actor.velocity.y += phi_f18;

    if (((this->actor.velocity.y - phi_f14) * phi_f18) > 0) {
        this->actor.velocity.y = phi_f14;
    }

    this->actor.gravity = 0.0f;
}

void func_8084B158(GlobalContext* globalCtx, Player* this, Input* input, f32 arg3) {
    f32 temp;

    if ((input != NULL) && CHECK_BTN_ANY(input->press.button, BTN_A | BTN_B)) {
        temp = 1.0f;
    } else {
        temp = 0.5f;
    }

    temp *= arg3;

    if (temp < 1.0f) {
        temp = 1.0f;
    }

    this->skelAnime.playSpeed = temp;
    LinkAnimation_Update(globalCtx, &this->skelAnime);
}

void Player_FirstPersonAiming(Player* this, GlobalContext* globalCtx) {
    if (this->stateFlags1 & PLAYER_STATE1_SWIMMING) {
        func_8084B000(this);
        Player_UpdateSwimMovement(this, &this->linearVelocity, 0, this->actor.shape.rot.y);
    } else {
        Player_StepLinearVelocityToZero(this);
    }

    if ((this->attentionMode == 2) && (Actor_PlayerIsAimingFpsItem(this) || Player_IsAimingBoomerang(this))) {
        Player_SetupCurrentUpperAction(this, globalCtx);
    }

    u16 buttonsToCheck = BTN_A | BTN_B | BTN_R | BTN_CUP | BTN_CLEFT | BTN_CRIGHT | BTN_CDOWN;
    if (CVar_GetS32("gDpadEquips", 0) != 0) {
        buttonsToCheck |= BTN_DUP | BTN_DDOWN | BTN_DLEFT | BTN_DRIGHT;
    }
    if ((this->csMode != 0) || (this->attentionMode == 0) || (this->attentionMode >= 4) || Player_SetupStartUnfriendlyZTargeting(this) ||
        (this->targetActor != NULL) || !Player_SetupCameraMode(globalCtx, this) ||
        (((this->attentionMode == 2) && (CHECK_BTN_ANY(sControlInput->press.button, BTN_A | BTN_B | BTN_R) ||
                                         Player_IsFriendlyZTargeting(this) || (!Actor_PlayerIsAimingReadyFpsItem(this) && !Player_IsAimingReadyBoomerang(this)))) ||
         ((this->attentionMode == 1) && CHECK_BTN_ANY(sControlInput->press.button, buttonsToCheck)))) {
        Player_ClearLookAndAttention(this, globalCtx);
        func_80078884(NA_SE_SY_CAMERA_ZOOM_UP);
    } else if ((DECR(this->genericTimer) == 0) || (this->attentionMode != 2)) {
        if (Player_IsShootingHookshot(this)) {
            this->lookFlags |= 0x43;
        } else {
            this->actor.shape.rot.y = func_8084ABD8(globalCtx, this, 0, 0);
        }
    }

    this->currentYaw = this->actor.shape.rot.y;
}

s32 func_8084B3CC(GlobalContext* globalCtx, Player* this) {
    if (globalCtx->shootingGalleryStatus != 0) {
        Player_ResetAttributesAndHeldActor(globalCtx, this);
        Player_SetActionFunc(globalCtx, this, Player_ShootingGalleryPlay, 0);

        if (!Actor_PlayerIsAimingFpsItem(this) || Player_HoldsHookshot(this)) {
            Player_UseItem(globalCtx, this, 3);
        }

        this->stateFlags1 |= PLAYER_STATE1_IN_FIRST_PERSON_MODE;
        Player_PlayAnimOnce(globalCtx, this, Player_GetStandingStillAnim(this));
        Player_StopMovement(this);
        Player_ResetLookAngles(this);
        return 1;
    }

    return 0;
}

void func_8084B498(Player* this) {
    this->itemActionParam =
        (INV_CONTENT(ITEM_OCARINA_FAIRY) == ITEM_OCARINA_FAIRY) ? PLAYER_AP_OCARINA_FAIRY : PLAYER_AP_OCARINA_TIME;
}

s32 func_8084B4D4(GlobalContext* globalCtx, Player* this) {
    if (this->stateFlags3 & PLAYER_STATE3_FORCE_PULL_OCARINA) {
        this->stateFlags3 &= ~PLAYER_STATE3_FORCE_PULL_OCARINA;
        func_8084B498(this);
        this->attentionMode = 4;
        Player_SetupItemCutsceneOrFirstPerson(this, globalCtx);
        return 1;
    }

    return 0;
}

void Player_TalkWithActor(Player* this, GlobalContext* globalCtx) {
    this->stateFlags2 |= PLAYER_STATE2_DISABLE_MOVE_ROTATION_WHILE_Z_TARGETING;
    Player_SetupCurrentUpperAction(this, globalCtx);

    if (Message_GetState(&globalCtx->msgCtx) == TEXT_STATE_CLOSING) {
        this->actor.flags &= ~ACTOR_FLAG_8;

        if (!CHECK_FLAG_ALL(this->talkActor->flags, ACTOR_FLAG_0 | ACTOR_FLAG_2)) {
            this->stateFlags2 &= ~PLAYER_STATE2_USING_SWITCH_Z_TARGETING;
        }

        func_8005B1A4(Gameplay_GetCamera(globalCtx, 0));

        if (!func_8084B4D4(globalCtx, this) && !func_8084B3CC(globalCtx, this) && !Player_SetupCutscene(globalCtx, this)) {
            if ((this->talkActor != this->interactRangeActor) || !Player_SetupGetItemOrHoldBehavior(this, globalCtx)) {
                if (this->stateFlags1 & PLAYER_STATE1_RIDING_HORSE) {
                    s32 sp24 = this->genericTimer;
                    Player_SetupRideHorse(globalCtx, this);
                    this->genericTimer = sp24;
                } else if (Player_IsSwimming(this)) {
                    Player_SetupSwimIdle(globalCtx, this);
                } else {
                    Player_SetupStandingStillMorph(this, globalCtx);
                }
            }
        }

        this->endTalkTimer = 10;
        return;
    }

    if (this->stateFlags1 & PLAYER_STATE1_RIDING_HORSE) {
        Player_RideHorse(this, globalCtx);
    } else if (Player_IsSwimming(this)) {
        Player_UpdateSwimIdle(this, globalCtx);
    } else if (!Player_IsUnfriendlyZTargeting(this) && LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        if (this->skelAnime.moveFlags != 0) {
            Player_EndAnimMovement(this);
            if ((this->talkActor->category == ACTORCAT_NPC) && (this->heldItemActionParam != PLAYER_AP_FISHING_POLE)) {
                Player_PlayAnimOnceSlowed(globalCtx, this, &gPlayerAnim_0031A0);
            } else {
                Player_PlayAnimLoop(globalCtx, this, Player_GetStandingStillAnim(this));
            }
        } else {
            Player_PlayAnimLoopSlowed(globalCtx, this, &gPlayerAnim_0031A8);
        }
    }

    if (this->targetActor != NULL) {
        this->currentYaw = this->actor.shape.rot.y = Player_LookAtTargetActor(this, 0);
    }
}

void Player_GrabPushPullWall(Player* this, GlobalContext* globalCtx) {
    f32 sp34;
    s16 sp32;
    s32 temp;

    this->stateFlags2 |= PLAYER_STATE2_CAN_GRAB_PUSH_PULL_WALL | PLAYER_STATE2_ALWAYS_DISABLE_MOVE_ROTATION |
                         PLAYER_STATE2_ENABLE_PUSH_PULL_CAM;
    func_8083F524(globalCtx, this);

    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        if (!func_8083F9D0(globalCtx, this)) {
            Player_GetTargetVelocityAndYaw(this, &sp34, &sp32, 0.0f, globalCtx);
            temp = func_8083FFB8(this, &sp34, &sp32);
            if (temp > 0) {
                func_8083FAB8(this, globalCtx);
            } else if (temp < 0) {
                func_8083FB14(this, globalCtx);
            }
        }
    }
}

void func_8084B840(GlobalContext* globalCtx, Player* this, f32 arg2) {
    if (this->actor.wallBgId != BGCHECK_SCENE) {
        DynaPolyActor* dynaPolyActor = DynaPoly_GetActor(&globalCtx->colCtx, this->actor.wallBgId);

        if (dynaPolyActor != NULL) {
            func_8002DFA4(dynaPolyActor, arg2, this->actor.world.rot.y);
        }
    }
}

static PlayerAnimSfxEntry D_80854870[] = {
    { NA_SE_PL_SLIP, 0x1003 },
    { NA_SE_PL_SLIP, -0x1015 },
};

void Player_PushWall(Player* this, GlobalContext* globalCtx) {
    f32 sp34;
    s16 sp32;
    s32 temp;

    this->stateFlags2 |= PLAYER_STATE2_CAN_GRAB_PUSH_PULL_WALL | PLAYER_STATE2_ALWAYS_DISABLE_MOVE_ROTATION |
                         PLAYER_STATE2_ENABLE_PUSH_PULL_CAM;

    if (Player_LoopAnimContinuously(globalCtx, this, &gPlayerAnim_003108)) {
        this->genericTimer = 1;
    } else if (this->genericTimer == 0) {
        if (LinkAnimation_OnFrame(&this->skelAnime, 11.0f)) {
            Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_PUSH);
        }
    }

    Player_PlayAnimSfx(this, D_80854870);
    func_8083F524(globalCtx, this);

    if (!func_8083F9D0(globalCtx, this)) {
        Player_GetTargetVelocityAndYaw(this, &sp34, &sp32, 0.0f, globalCtx);
        temp = func_8083FFB8(this, &sp34, &sp32);
        if (temp < 0) {
            func_8083FB14(this, globalCtx);
        } else if (temp == 0) {
            func_8083F72C(this, &gPlayerAnim_0030E0, globalCtx);
        } else {
            this->stateFlags2 |= PLAYER_STATE2_MOVING_PUSH_PULL_WALL;
        }
    }

    if (this->stateFlags2 & PLAYER_STATE2_MOVING_PUSH_PULL_WALL) {
        func_8084B840(globalCtx, this, 2.0f);
        this->linearVelocity = 2.0f;
    }
}

static PlayerAnimSfxEntry D_80854878[] = {
    { NA_SE_PL_SLIP, 0x1004 },
    { NA_SE_PL_SLIP, -0x1018 },
};

static Vec3f D_80854880 = { 0.0f, 26.0f, -40.0f };

void Player_PullWall(Player* this, GlobalContext* globalCtx) {
    LinkAnimationHeader* anim;
    f32 sp70;
    s16 sp6E;
    s32 temp1;
    Vec3f sp5C;
    f32 temp2;
    CollisionPoly* sp54;
    s32 sp50;
    Vec3f sp44;
    Vec3f sp38;

    anim = GET_PLAYER_ANIM(PLAYER_ANIMGROUP_PULL_OBJECT, this->modelAnimType);
    this->stateFlags2 |= PLAYER_STATE2_CAN_GRAB_PUSH_PULL_WALL | PLAYER_STATE2_ALWAYS_DISABLE_MOVE_ROTATION |
                         PLAYER_STATE2_ENABLE_PUSH_PULL_CAM;

    if (Player_LoopAnimContinuously(globalCtx, this, anim)) {
        this->genericTimer = 1;
    } else {
        if (this->genericTimer == 0) {
            if (LinkAnimation_OnFrame(&this->skelAnime, 11.0f)) {
                Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_PUSH);
            }
        } else {
            Player_PlayAnimSfx(this, D_80854878);
        }
    }

    func_8083F524(globalCtx, this);

    if (!func_8083F9D0(globalCtx, this)) {
        Player_GetTargetVelocityAndYaw(this, &sp70, &sp6E, 0.0f, globalCtx);
        temp1 = func_8083FFB8(this, &sp70, &sp6E);
        if (temp1 > 0) {
            func_8083FAB8(this, globalCtx);
        } else if (temp1 == 0) {
            func_8083F72C(this, GET_PLAYER_ANIM(PLAYER_ANIMGROUP_PUSH_OBJECT, this->modelAnimType), globalCtx);
        } else {
            this->stateFlags2 |= PLAYER_STATE2_MOVING_PUSH_PULL_WALL;
        }
    }

    if (this->stateFlags2 & PLAYER_STATE2_MOVING_PUSH_PULL_WALL) {
        temp2 = Player_RaycastFloorWithOffset2(globalCtx, this, &D_80854880, &sp5C) - this->actor.world.pos.y;
        if (fabsf(temp2) < 20.0f) {
            sp44.x = this->actor.world.pos.x;
            sp44.z = this->actor.world.pos.z;
            sp44.y = sp5C.y;
            if (!BgCheck_EntityLineTest1(&globalCtx->colCtx, &sp44, &sp5C, &sp38, &sp54, true, false, false, true,
                                         &sp50)) {
                func_8084B840(globalCtx, this, -2.0f);
                return;
            }
        }
        this->stateFlags2 &= ~PLAYER_STATE2_MOVING_PUSH_PULL_WALL;
    }
}

void Player_GrabLedge(Player* this, GlobalContext* globalCtx) {
    f32 sp3C;
    s16 sp3A;
    LinkAnimationHeader* anim;
    f32 temp;

    this->stateFlags2 |= PLAYER_STATE2_ALWAYS_DISABLE_MOVE_ROTATION;

    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        // clang-format off
        anim = (this->genericVar > 0) ? &gPlayerAnim_002F28 : GET_PLAYER_ANIM(PLAYER_ANIMGROUP_CLIMBING_IDLE, this->modelAnimType); Player_PlayAnimLoop(globalCtx, this, anim);
        // clang-format on
    } else if (this->genericVar == 0) {
        if (this->skelAnime.animation == &gPlayerAnim_002F10) {
            temp = 11.0f;
        } else {
            temp = 1.0f;
        }

        if (LinkAnimation_OnFrame(&this->skelAnime, temp)) {
            Player_PlayMoveSfx(this, NA_SE_PL_WALK_GROUND);
            if (this->skelAnime.animation == &gPlayerAnim_002F10) {
                this->genericVar = 1;
            } else {
                this->genericVar = -1;
            }
        }
    }

    Math_ScaledStepToS(&this->actor.shape.rot.y, this->currentYaw, 0x800);

    if (this->genericVar != 0) {
        Player_GetTargetVelocityAndYaw(this, &sp3C, &sp3A, 0.0f, globalCtx);
        if (this->analogStickInputs[this->inputFrameCounter] >= 0) {
            if (this->genericVar > 0) {
                anim = GET_PLAYER_ANIM(PLAYER_ANIMGROUP_KNOCKED_FROM_CLIMBING, this->modelAnimType);
            } else {
                anim = GET_PLAYER_ANIM(PLAYER_ANIMGROUP_CLIMBING, this->modelAnimType);
            }
            Player_SetupClimbOntoLedge(this, anim, globalCtx);
            return;
        }

        if (CHECK_BTN_ALL(sControlInput->cur.button, BTN_A) || (this->actor.shape.feetFloorFlags != 0)) {
            Player_SetLedgeGrabPosition(this);
            if (this->genericVar < 0) {
                this->linearVelocity = -0.8f;
            } else {
                this->linearVelocity = 0.8f;
            }
            Player_SetupFallFromLedge(this, globalCtx);
            this->stateFlags1 &= ~(PLAYER_STATE1_HANGING_FROM_LEDGE_SLIP | PLAYER_STATE1_CLIMBING_ONTO_LEDGE);
        }
    }
}

void Player_ClimbOntoLedge(Player* this, GlobalContext* globalCtx) {
    this->stateFlags2 |= PLAYER_STATE2_ALWAYS_DISABLE_MOVE_ROTATION;

    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        Player_UpdateAnimMovement(this, 1);
        Player_SetupStandingStillNoMorph(this, globalCtx);
        return;
    }

    if (LinkAnimation_OnFrame(&this->skelAnime, this->skelAnime.endFrame - 6.0f)) {
        Player_PlayLandingSfx(this);
    } else if (LinkAnimation_OnFrame(&this->skelAnime, this->skelAnime.endFrame - 34.0f)) {
        this->stateFlags1 &= ~(PLAYER_STATE1_HANGING_FROM_LEDGE_SLIP | PLAYER_STATE1_CLIMBING_ONTO_LEDGE);
        func_8002F7DC(&this->actor, NA_SE_PL_CLIMB_CLIFF);
        Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_CLIMB_END);
    }
}

void func_8084BEE4(Player* this) {
    func_8002F7DC(&this->actor, (this->genericVar != 0) ? NA_SE_PL_WALK_WALL : NA_SE_PL_WALK_LADDER);
}

void Player_ClimbingWallOrDownLedge(Player* this, GlobalContext* globalCtx) {
    static Vec3f D_8085488C = { 0.0f, 0.0f, 26.0f };
    s32 sp84;
    s32 sp80;
    f32 phi_f0;
    f32 phi_f2;
    Vec3f sp6C;
    s32 sp68;
    Vec3f sp5C;
    f32 temp_f0;
    LinkAnimationHeader* anim1;
    LinkAnimationHeader* anim2;

    sp84 = sControlInput->rel.stick_y;
    sp80 = sControlInput->rel.stick_x;

    this->fallStartHeight = this->actor.world.pos.y;
    this->stateFlags2 |= PLAYER_STATE2_ALWAYS_DISABLE_MOVE_ROTATION;

    if ((this->genericVar != 0) && (ABS(sp84) < ABS(sp80))) {
        phi_f0 = ABS(sp80) * 0.0325f;
        sp84 = 0;
    } else {
        phi_f0 = ABS(sp84) * 0.05f;
        sp80 = 0;
    }

    if (phi_f0 < 1.0f) {
        phi_f0 = 1.0f;
    } else if (phi_f0 > 3.35f) {
        phi_f0 = 3.35f;
    }

    if (this->skelAnime.playSpeed >= 0.0f) {
        phi_f2 = 1.0f;
    } else {
        phi_f2 = -1.0f;
    }

    if (CVar_GetS32("gChaosClimbSpeed", 0) != 0) {
        this->skelAnime.playSpeed = (phi_f2 * phi_f0) * (1.0f - CVar_GetS32("gChaosClimbSpeed", 0) * 0.1f);
    }
    else {
        this->skelAnime.playSpeed = phi_f2 * phi_f0 + phi_f2 * CVar_GetS32("gClimbSpeed", 0);
    }

    if (this->genericTimer >= 0) {
        if ((this->actor.wallPoly != NULL) && (this->actor.wallBgId != BGCHECK_SCENE)) {
            DynaPolyActor* wallPolyActor = DynaPoly_GetActor(&globalCtx->colCtx, this->actor.wallBgId);
            if (wallPolyActor != NULL) {
                Math_Vec3f_Diff(&wallPolyActor->actor.world.pos, &wallPolyActor->actor.prevPos, &sp6C);
                Math_Vec3f_Sum(&this->actor.world.pos, &sp6C, &this->actor.world.pos);
            }
        }

        Actor_UpdateBgCheckInfo(globalCtx, &this->actor, 26.0f, 6.0f, this->ageProperties->unk_00, 7);
        func_8083F360(globalCtx, this, 26.0f, this->ageProperties->unk_3C, 50.0f, -20.0f);
    }

    if ((this->genericTimer < 0) || !func_8083FBC0(this, globalCtx)) {
        if (LinkAnimation_Update(globalCtx, &this->skelAnime) != 0) {
            if (this->genericTimer < 0) {
                this->genericTimer = ABS(this->genericTimer) & 1;
                return;
            }

            if (sp84 != 0) {
                sp68 = this->genericVar + this->genericTimer;

                if (sp84 > 0) {
                    D_8085488C.y = this->ageProperties->unk_40;
                    temp_f0 = Player_RaycastFloorWithOffset2(globalCtx, this, &D_8085488C, &sp5C);

                    if (this->actor.world.pos.y < temp_f0) {
                        if (this->genericVar != 0) {
                            this->actor.world.pos.y = temp_f0;
                            this->stateFlags1 &= ~PLAYER_STATE1_CLIMBING;
                            Player_SetupGrabLedge(globalCtx, this, this->actor.wallPoly, this->ageProperties->unk_3C,
                                          &gPlayerAnim_003000);
                            this->currentYaw += 0x8000;
                            this->actor.shape.rot.y = this->currentYaw;
                            Player_SetupClimbOntoLedge(this, &gPlayerAnim_003000, globalCtx);
                            this->stateFlags1 |= PLAYER_STATE1_CLIMBING_ONTO_LEDGE;
                        } else {
                            func_8083F070(this, this->ageProperties->unk_CC[this->genericTimer], globalCtx);
                        }
                    } else {
                        this->skelAnime.prevTransl = this->ageProperties->unk_4A[sp68];
                        Player_PlayAnimOnce(globalCtx, this, this->ageProperties->unk_AC[sp68]);
                    }
                } else {
                    if ((this->actor.world.pos.y - this->actor.floorHeight) < 15.0f) {
                        if (this->genericVar != 0) {
                            func_8083FB7C(this, globalCtx);
                        } else {
                            if (this->genericTimer != 0) {
                                this->skelAnime.prevTransl = this->ageProperties->unk_44;
                            }
                            func_8083F070(this, this->ageProperties->unk_C4[this->genericTimer], globalCtx);
                            this->genericTimer = 1;
                        }
                    } else {
                        sp68 ^= 1;
                        this->skelAnime.prevTransl = this->ageProperties->unk_62[sp68];
                        anim1 = this->ageProperties->unk_AC[sp68];
                        LinkAnimation_Change(globalCtx, &this->skelAnime, anim1, -1.0f, Animation_GetLastFrame(anim1),
                                             0.0f, ANIMMODE_ONCE, 0.0f);
                    }
                }
                this->genericTimer ^= 1;
            } else {
                if ((this->genericVar != 0) && (sp80 != 0)) {
                    anim2 = this->ageProperties->unk_BC[this->genericTimer];

                    if (sp80 > 0) {
                        this->skelAnime.prevTransl = this->ageProperties->unk_7A[this->genericTimer];
                        Player_PlayAnimOnce(globalCtx, this, anim2);
                    } else {
                        this->skelAnime.prevTransl = this->ageProperties->unk_86[this->genericTimer];
                        LinkAnimation_Change(globalCtx, &this->skelAnime, anim2, -1.0f, Animation_GetLastFrame(anim2),
                                             0.0f, ANIMMODE_ONCE, 0.0f);
                    }
                } else {
                    this->stateFlags2 |= PLAYER_STATE2_IDLE_WHILE_CLIMBING;
                }
            }

            return;
        }
    }

    if (this->genericTimer < 0) {
        if (((this->genericTimer == -2) &&
             (LinkAnimation_OnFrame(&this->skelAnime, 14.0f) || LinkAnimation_OnFrame(&this->skelAnime, 29.0f))) ||
            ((this->genericTimer == -4) &&
             (LinkAnimation_OnFrame(&this->skelAnime, 22.0f) || LinkAnimation_OnFrame(&this->skelAnime, 35.0f) ||
              LinkAnimation_OnFrame(&this->skelAnime, 49.0f) || LinkAnimation_OnFrame(&this->skelAnime, 55.0f)))) {
            func_8084BEE4(this);
        }
        return;
    }

    if (LinkAnimation_OnFrame(&this->skelAnime, (this->skelAnime.playSpeed > 0.0f) ? 20.0f : 0.0f)) {
        func_8084BEE4(this);
    }
}

static f32 D_80854898[] = { 10.0f, 20.0f };
static f32 D_808548A0[] = { 40.0f, 50.0f };

static PlayerAnimSfxEntry D_808548A8[] = {
    { NA_SE_PL_WALK_LADDER, 0x80A },
    { NA_SE_PL_WALK_LADDER, 0x814 },
    { NA_SE_PL_WALK_LADDER, -0x81E },
};

void Player_EndClimb(Player* this, GlobalContext* globalCtx) {
    s32 temp;
    f32* sp38;
    CollisionPoly* sp34;
    s32 sp30;
    Vec3f sp24;

    this->stateFlags2 |= PLAYER_STATE2_ALWAYS_DISABLE_MOVE_ROTATION;

    temp = Player_IsActionInterrupted(globalCtx, this, &this->skelAnime, 4.0f);

    if (temp == 0) {
        this->stateFlags1 &= ~PLAYER_STATE1_CLIMBING;
        return;
    }

    if ((temp > 0) || LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        Player_SetupStandingStillNoMorph(this, globalCtx);
        this->stateFlags1 &= ~PLAYER_STATE1_CLIMBING;
        return;
    }

    sp38 = D_80854898;

    if (this->genericTimer != 0) {
        Player_PlayAnimSfx(this, D_808548A8);
        sp38 = D_808548A0;
    }

    if (LinkAnimation_OnFrame(&this->skelAnime, sp38[0]) || LinkAnimation_OnFrame(&this->skelAnime, sp38[1])) {
        sp24.x = this->actor.world.pos.x;
        sp24.y = this->actor.world.pos.y + 20.0f;
        sp24.z = this->actor.world.pos.z;
        if (BgCheck_EntityRaycastFloor3(&globalCtx->colCtx, &sp34, &sp30, &sp24) != 0.0f) {
            this->surfaceMaterial = func_80041F10(&globalCtx->colCtx, sp34, sp30);
            Player_PlayLandingSfx(this);
        }
    }
}

static PlayerAnimSfxEntry D_808548B4[] = {
    { 0, 0x3028 }, { 0, 0x3030 }, { 0, 0x3038 }, { 0, 0x3040 },  { 0, 0x3048 },
    { 0, 0x3050 }, { 0, 0x3058 }, { 0, 0x3060 }, { 0, -0x3068 },
};

void Player_InsideCrawlspace(Player* this, GlobalContext* globalCtx) {
    this->stateFlags2 |= PLAYER_STATE2_ALWAYS_DISABLE_MOVE_ROTATION;

    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        if (!(this->stateFlags1 & PLAYER_STATE1_EXITING_SCENE)) {
            if (this->skelAnime.moveFlags != 0) {
                this->skelAnime.moveFlags = 0;
                return;
            }

            if (!func_8083F570(this, globalCtx)) {
                this->linearVelocity = sControlInput->rel.stick_y * 0.03f;
            }
        }
        return;
    }

    Player_PlayAnimSfx(this, D_808548B4);
}

static PlayerAnimSfxEntry D_808548D8[] = {
    { 0, 0x300A }, { 0, 0x3012 }, { 0, 0x301A }, { 0, 0x3022 },  { 0, 0x3034 },
    { 0, 0x303C }, { 0, 0x3044 }, { 0, 0x304C }, { 0, -0x3054 },
};

void Player_ExitCrawlspace(Player* this, GlobalContext* globalCtx) {
    this->stateFlags2 |= PLAYER_STATE2_ALWAYS_DISABLE_MOVE_ROTATION;

    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        Player_SetupStandingStillNoMorph(this, globalCtx);
        this->stateFlags2 &= ~PLAYER_STATE2_INSIDE_CRAWLSPACE;
        return;
    }

    Player_PlayAnimSfx(this, D_808548D8);
}

static Vec3f D_808548FC[] = {
    { 40.0f, 0.0f, 0.0f },
    { -40.0f, 0.0f, 0.0f },
};

static Vec3f D_80854914[] = {
    { 60.0f, 20.0f, 0.0f },
    { -60.0f, 20.0f, 0.0f },
};

static Vec3f D_8085492C[] = {
    { 60.0f, -20.0f, 0.0f },
    { -60.0f, -20.0f, 0.0f },
};

s32 func_8084C89C(GlobalContext* globalCtx, Player* this, s32 arg2, f32* arg3) {
    EnHorse* rideActor = (EnHorse*)this->rideActor;
    f32 sp50;
    f32 sp4C;
    Vec3f sp40;
    Vec3f sp34;
    CollisionPoly* sp30;
    s32 sp2C;

    sp50 = rideActor->actor.world.pos.y + 20.0f;
    sp4C = rideActor->actor.world.pos.y - 20.0f;

    *arg3 = Player_RaycastFloorWithOffset2(globalCtx, this, &D_808548FC[arg2], &sp40);

    return (sp4C < *arg3) && (*arg3 < sp50) &&
           !Player_WallLineTestWithOffset(globalCtx, this, &D_80854914[arg2], &sp30, &sp2C, &sp34) &&
           !Player_WallLineTestWithOffset(globalCtx, this, &D_8085492C[arg2], &sp30, &sp2C, &sp34);
}

s32 func_8084C9BC(Player* this, GlobalContext* globalCtx) {
    EnHorse* rideActor = (EnHorse*)this->rideActor;
    s32 sp38;
    f32 sp34;

    if (this->genericTimer < 0) {
        this->genericTimer = 99;
    } else {
        sp38 = (this->mountSide < 0) ? 0 : 1;
        if (!func_8084C89C(globalCtx, this, sp38, &sp34)) {
            sp38 ^= 1;
            if (!func_8084C89C(globalCtx, this, sp38, &sp34)) {
                return 0;
            } else {
                this->mountSide = -this->mountSide;
            }
        }

        if ((globalCtx->csCtx.state == CS_STATE_IDLE) && (globalCtx->transitionMode == 0) &&
            (EN_HORSE_CHECK_1(rideActor) || EN_HORSE_CHECK_4(rideActor))) {
            this->stateFlags2 |= PLAYER_STATE2_CAN_DISMOUNT_HORSE;

            if (EN_HORSE_CHECK_1(rideActor) ||
                (EN_HORSE_CHECK_4(rideActor) && CHECK_BTN_ALL(sControlInput->press.button, BTN_A))) {
                rideActor->actor.child = NULL;
                Player_SetActionFuncPreserveMoveFlags(globalCtx, this, Player_DismountHorse, 0);
                this->rideOffsetY = sp34 - rideActor->actor.world.pos.y;
                Player_PlayAnimOnce(globalCtx, this, (this->mountSide < 0) ? &gPlayerAnim_003390 : &gPlayerAnim_0033A0);
                return 1;
            }
        }
    }

    return 0;
}

void func_8084CBF4(Player* this, f32 arg1, f32 arg2) {
    f32 temp;
    f32 dir;

    if ((this->rideOffsetY != 0.0f) && (arg2 <= this->skelAnime.curFrame)) {
        if (arg1 < fabsf(this->rideOffsetY)) {
            if (this->rideOffsetY >= 0.0f) {
                dir = 1;
            } else {
                dir = -1;
            }
            temp = dir * arg1;
        } else {
            temp = this->rideOffsetY;
        }
        this->actor.world.pos.y += temp;
        this->rideOffsetY -= temp;
    }
}

static LinkAnimationHeader* D_80854944[] = {
    &gPlayerAnim_003370,
    &gPlayerAnim_003368,
    &gPlayerAnim_003380,
    &gPlayerAnim_003358,
    &gPlayerAnim_003338,
    &gPlayerAnim_003348,
    &gPlayerAnim_003350,
    NULL,
    NULL,
};

static LinkAnimationHeader* D_80854968[] = {
    &gPlayerAnim_003388,
    &gPlayerAnim_003388,
    &gPlayerAnim_003388,
    &gPlayerAnim_003360,
    &gPlayerAnim_003340,
    &gPlayerAnim_003340,
    &gPlayerAnim_003340,
    NULL,
    NULL,
};

static LinkAnimationHeader* D_8085498C[] = {
    &gPlayerAnim_0033C8,
    &gPlayerAnim_0033B8,
    &gPlayerAnim_0033C0,
};

static u8 D_80854998[2][2] = {
    { 32, 58 },
    { 25, 42 },
};

static Vec3s D_8085499C = { -69, 7146, -266 };

static PlayerAnimSfxEntry D_808549A4[] = {
    { NA_SE_PL_CALM_HIT, 0x830 }, { NA_SE_PL_CALM_HIT, 0x83A },  { NA_SE_PL_CALM_HIT, 0x844 },
    { NA_SE_PL_CALM_PAT, 0x85C }, { NA_SE_PL_CALM_PAT, 0x86E },  { NA_SE_PL_CALM_PAT, 0x87E },
    { NA_SE_PL_CALM_PAT, 0x884 }, { NA_SE_PL_CALM_PAT, -0x888 },
};

void Player_RideHorse(Player* this, GlobalContext* globalCtx) {
    EnHorse* rideActor = (EnHorse*)this->rideActor;
    u8* arr;

    this->stateFlags2 |= PLAYER_STATE2_ALWAYS_DISABLE_MOVE_ROTATION;

    func_8084CBF4(this, 1.0f, 10.0f);

    if (this->genericTimer == 0) {
        if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
            this->skelAnime.animation = &gPlayerAnim_0033B8;
            this->genericTimer = 99;
            return;
        }

        arr = D_80854998[(this->mountSide < 0) ? 0 : 1];

        if (LinkAnimation_OnFrame(&this->skelAnime, arr[0])) {
            func_8002F7DC(&this->actor, NA_SE_PL_CLIMB_CLIFF);
            return;
        }

        if (LinkAnimation_OnFrame(&this->skelAnime, arr[1])) {
            func_8002DE74(globalCtx, this);
            func_8002F7DC(&this->actor, NA_SE_PL_SIT_ON_HORSE);
            return;
        }

        return;
    }

    func_8002DE74(globalCtx, this);
    this->skelAnime.prevTransl = D_8085499C;

    if ((rideActor->animationIdx != this->genericTimer) &&
        ((rideActor->animationIdx >= 2) || (this->genericTimer >= 2))) {
        if ((this->genericTimer = rideActor->animationIdx) < 2) {
            f32 rand = Rand_ZeroOne();
            s32 temp = 0;

            this->genericTimer = 1;

            if (rand < 0.1f) {
                temp = 2;
            } else if (rand < 0.2f) {
                temp = 1;
            }
            Player_PlayAnimOnce(globalCtx, this, D_8085498C[temp]);
        } else {
            this->skelAnime.animation = D_80854944[this->genericTimer - 2];
            Animation_SetMorph(globalCtx, &this->skelAnime, 8.0f);
            if (this->genericTimer < 4) {
                Player_SetupHeldItemUpperActionFunc(globalCtx, this);
                this->genericVar = 0;
            }
        }
    }

    if (this->genericTimer == 1) {
        if ((D_808535E0 != 0) || Player_CheckActorTalkRequested(globalCtx)) {
            Player_PlayAnimOnce(globalCtx, this, &gPlayerAnim_0033C8);
        } else if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
            this->genericTimer = 99;
        } else if (this->skelAnime.animation == &gPlayerAnim_0033B8) {
            Player_PlayAnimSfx(this, D_808549A4);
        }
    } else {
        this->skelAnime.curFrame = rideActor->curFrame;
        LinkAnimation_AnimateFrame(globalCtx, &this->skelAnime);
    }

    AnimationContext_SetCopyAll(globalCtx, this->skelAnime.limbCount, this->skelAnime.morphTable,
                                this->skelAnime.jointTable);

    if ((globalCtx->csCtx.state != CS_STATE_IDLE) || (this->csMode != 0)) {
        if (this->csMode == 7) {
            this->csMode = 0;
        }
        this->attentionMode = 0;
        this->genericVar = 0;
    } else if ((this->genericTimer < 2) || (this->genericTimer >= 4)) {
        D_808535E0 = Player_SetupCurrentUpperAction(this, globalCtx);
        if (D_808535E0 != 0) {
            this->genericVar = 0;
        }
    }

    this->actor.world.pos.x = rideActor->actor.world.pos.x + rideActor->riderPos.x;
    this->actor.world.pos.y = (rideActor->actor.world.pos.y + rideActor->riderPos.y) - 27.0f;
    this->actor.world.pos.z = rideActor->actor.world.pos.z + rideActor->riderPos.z;

    this->currentYaw = this->actor.shape.rot.y = rideActor->actor.shape.rot.y;

    if ((this->csMode != 0) || (!Player_CheckActorTalkRequested(globalCtx) &&
                                ((rideActor->actor.speedXZ != 0.0f) || !Player_SetupSpeakOrCheck(this, globalCtx)) &&
                                !Player_SetupRollOrPutAway(this, globalCtx))) {
        if (D_808535E0 == 0) {
            if (this->genericVar != 0) {
                if (LinkAnimation_Update(globalCtx, &this->skelAnimeUpper)) {
                    rideActor->stateFlags &= ~ENHORSE_FLAG_8;
                    this->genericVar = 0;
                }

                if (this->skelAnimeUpper.animation == &gPlayerAnim_0033B0) {
                    if (LinkAnimation_OnFrame(&this->skelAnimeUpper, 23.0f)) {
                        func_8002F7DC(&this->actor, NA_SE_IT_LASH);
                        Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_LASH);
                    }

                    AnimationContext_SetCopyAll(globalCtx, this->skelAnime.limbCount, this->skelAnime.jointTable,
                                                this->skelAnimeUpper.jointTable);
                } else {
                    if (LinkAnimation_OnFrame(&this->skelAnimeUpper, 10.0f)) {
                        func_8002F7DC(&this->actor, NA_SE_IT_LASH);
                        Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_LASH);
                    }

                    AnimationContext_SetCopyTrue(globalCtx, this->skelAnime.limbCount, this->skelAnime.jointTable,
                                                 this->skelAnimeUpper.jointTable, D_80853410);
                }
            } else {
                LinkAnimationHeader* anim = NULL;

                if (EN_HORSE_CHECK_3(rideActor)) {
                    anim = &gPlayerAnim_0033B0;
                } else if (EN_HORSE_CHECK_2(rideActor)) {
                    if ((this->genericTimer >= 2) && (this->genericTimer != 99)) {
                        anim = D_80854968[this->genericTimer - 2];
                    }
                }

                if (anim != NULL) {
                    LinkAnimation_PlayOnce(globalCtx, &this->skelAnimeUpper, anim);
                    this->genericVar = 1;
                }
            }
        }

        if (this->stateFlags1 & PLAYER_STATE1_IN_FIRST_PERSON_MODE) {
            if (!Player_SetupCameraMode(globalCtx, this) || CHECK_BTN_ANY(sControlInput->press.button, BTN_A) ||
                Player_IsZTargeting(this)) {
                this->attentionMode = 0;
                this->stateFlags1 &= ~PLAYER_STATE1_IN_FIRST_PERSON_MODE;
            } else {
                this->upperBodyRot.y = func_8084ABD8(globalCtx, this, 1, -5000) - this->actor.shape.rot.y;
                this->upperBodyRot.y += 5000;
                this->upperBodyYawOffset = -5000;
            }
            return;
        }

        if ((this->csMode != 0) ||
            (!func_8084C9BC(this, globalCtx) && !Player_SetupItemCutsceneOrFirstPerson(this, globalCtx))) {
            if (this->targetActor != NULL) {
                if (Actor_PlayerIsAimingReadyFpsItem(this) != 0) {
                    this->upperBodyRot.y = Player_LookAtTargetActor(this, 1) - this->actor.shape.rot.y;
                    this->upperBodyRot.y = CLAMP(this->upperBodyRot.y, -0x4AAA, 0x4AAA);
                    this->actor.focus.rot.y = this->actor.shape.rot.y + this->upperBodyRot.y;
                    this->upperBodyRot.y += 5000;
                    this->lookFlags |= 0x80;
                } else {
                    Player_LookAtTargetActor(this, 0);
                }
            } else {
                if (Actor_PlayerIsAimingReadyFpsItem(this) != 0) {
                    this->upperBodyRot.y = func_8084ABD8(globalCtx, this, 1, -5000) - this->actor.shape.rot.y;
                    this->upperBodyRot.y += 5000;
                    this->upperBodyYawOffset = -5000;
                }
            }
        }
    }
}

static PlayerAnimSfxEntry D_808549C4[] = {
    { 0, 0x2800 },
    { NA_SE_PL_GET_OFF_HORSE, 0x80A },
    { NA_SE_PL_SLIPDOWN, -0x819 },
};

void Player_DismountHorse(Player* this, GlobalContext* globalCtx) {
    this->stateFlags2 |= PLAYER_STATE2_ALWAYS_DISABLE_MOVE_ROTATION;
    func_8084CBF4(this, 1.0f, 10.0f);

    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        EnHorse* rideActor = (EnHorse*)this->rideActor;

        Player_SetupStandingStillNoMorph(this, globalCtx);
        this->stateFlags1 &= ~PLAYER_STATE1_RIDING_HORSE;
        this->actor.parent = NULL;
        AREG(6) = 0;

        if (Flags_GetEventChkInf(0x18) || (DREG(1) != 0)) {
            gSaveContext.horseData.pos.x = rideActor->actor.world.pos.x;
            gSaveContext.horseData.pos.y = rideActor->actor.world.pos.y;
            gSaveContext.horseData.pos.z = rideActor->actor.world.pos.z;
            gSaveContext.horseData.angle = rideActor->actor.shape.rot.y;
        }
    } else {
        Camera_ChangeSetting(Gameplay_GetCamera(globalCtx, 0), CAM_SET_NORMAL0);

        if (this->mountSide < 0) {
            D_808549C4[0].field = 0x2828;
        } else {
            D_808549C4[0].field = 0x281D;
        }
        Player_PlayAnimSfx(this, D_808549C4);
    }
}

static PlayerAnimSfxEntry sSwimAnimSfx[] = {
    { NA_SE_PL_SWIM, -0x800 },
};

void Player_SetupSwimMovement(Player* this, f32* arg1, f32 arg2, s16 arg3) {
    Player_UpdateSwimMovement(this, arg1, arg2, arg3);
    Player_PlayAnimSfx(this, sSwimAnimSfx);
}

void Player_SetupSwim(GlobalContext* globalCtx, Player* this, s16 arg2) {
    Player_SetActionFunc(globalCtx, this, Player_Swim, 0);
    this->actor.shape.rot.y = this->currentYaw = arg2;
    Player_ChangeAnimLongMorphLoop(globalCtx, this, &gPlayerAnim_0032F0);
}

void Player_SetupZTargetSwimming(GlobalContext* globalCtx, Player* this) {
    Player_SetActionFunc(globalCtx, this, Player_ZTargetSwimming, 0);
    Player_ChangeAnimLongMorphLoop(globalCtx, this, &gPlayerAnim_0032F0);
}

void Player_UpdateSwimIdle(Player* this, GlobalContext* globalCtx) {
    f32 sp34;
    s16 sp32;

    Player_LoopAnimContinuously(globalCtx, this, &gPlayerAnim_003328);
    func_8084B000(this);

    if (!Player_CheckActorTalkRequested(globalCtx) && !Player_SetupSubAction(globalCtx, this, sSwimSubActions, 1) &&
        !Player_SetupDive(globalCtx, this, sControlInput)) {
        if (this->attentionMode != 1) {
            this->attentionMode = 0;
        }

        if (this->currentBoots == PLAYER_BOOTS_IRON) {
            sp34 = 0.0f;
            sp32 = this->actor.shape.rot.y;

            if (this->actor.bgCheckFlags & 1) {
                Player_SetupReturnToStandStillSetAnim(
                    this, GET_PLAYER_ANIM(PLAYER_ANIMGROUP_SHORT_JUMP_LANDING, this->modelAnimType), globalCtx);
                Player_PlayLandingSfx(this);
            }
        } else {
            Player_GetTargetVelocityAndYaw(this, &sp34, &sp32, 0.0f, globalCtx);

            if (sp34 != 0.0f) {
                s16 temp = this->actor.shape.rot.y - sp32;

                if ((ABS(temp) > 0x6000) && !Math_StepToF(&this->linearVelocity, 0.0f, 1.0f)) {
                    return;
                }

                if (Player_IsZTargetingSetupStartUnfriendly(this)) {
                    Player_SetupZTargetSwimming(globalCtx, this);
                } else {
                    Player_SetupSwim(globalCtx, this, sp32);
                }
            }
        }

        Player_UpdateSwimMovement(this, &this->linearVelocity, sp34, sp32);
    }
}

void Player_SpawnSwimming(Player* this, GlobalContext* globalCtx) {
    if (!Player_SetupItemCutsceneOrFirstPerson(this, globalCtx)) {
        this->stateFlags2 |= PLAYER_STATE2_DISABLE_MOVE_ROTATION_WHILE_Z_TARGETING;

        func_8084B158(globalCtx, this, NULL, this->linearVelocity);
        func_8084B000(this);

        if (DECR(this->genericTimer) == 0) {
            Player_SetupSwimIdle(globalCtx, this);
        }
    }
}

void Player_Swim(Player* this, GlobalContext* globalCtx) {
    f32 sp34;
    s16 sp32;
    s16 temp;

    this->stateFlags2 |= PLAYER_STATE2_DISABLE_MOVE_ROTATION_WHILE_Z_TARGETING;

    func_8084B158(globalCtx, this, sControlInput, this->linearVelocity);
    func_8084B000(this);

    if (!Player_SetupSubAction(globalCtx, this, sSwimSubActions, 1) && !Player_SetupDive(globalCtx, this, sControlInput)) {
        Player_GetTargetVelocityAndYaw(this, &sp34, &sp32, 0.0f, globalCtx);

        temp = this->actor.shape.rot.y - sp32;
        if ((sp34 == 0.0f) || (ABS(temp) > 0x6000) || (this->currentBoots == PLAYER_BOOTS_IRON)) {
            Player_SetupSwimIdle(globalCtx, this);
        } else if (Player_IsZTargetingSetupStartUnfriendly(this)) {
            Player_SetupZTargetSwimming(globalCtx, this);
        }

        Player_SetupSwimMovement(this, &this->linearVelocity, sp34, sp32);
    }
}

s32 func_8084D980(GlobalContext* globalCtx, Player* this, f32* arg2, s16* arg3) {
    LinkAnimationHeader* anim;
    s16 temp1;
    s32 temp2;

    temp1 = this->currentYaw - *arg3;

    if (ABS(temp1) > 0x6000) {
        anim = &gPlayerAnim_003328;

        if (Math_StepToF(&this->linearVelocity, 0.0f, 1.0f)) {
            this->currentYaw = *arg3;
        } else {
            *arg2 = 0.0f;
            *arg3 = this->currentYaw;
        }
    } else {
        temp2 = func_8083FD78(this, arg2, arg3, globalCtx);

        if (temp2 > 0) {
            anim = &gPlayerAnim_0032F0;
        } else if (temp2 < 0) {
            anim = &gPlayerAnim_0032D8;
        } else if ((temp1 = this->actor.shape.rot.y - *arg3) > 0) {
            anim = &gPlayerAnim_0032D0;
        } else {
            anim = &gPlayerAnim_0032C8;
        }
    }

    if (anim != this->skelAnime.animation) {
        Player_ChangeAnimLongMorphLoop(globalCtx, this, anim);
        return 1;
    }

    return 0;
}

void Player_ZTargetSwimming(Player* this, GlobalContext* globalCtx) {
    f32 sp2C;
    s16 sp2A;

    func_8084B158(globalCtx, this, sControlInput, this->linearVelocity);
    func_8084B000(this);

    if (!Player_SetupSubAction(globalCtx, this, sSwimSubActions, 1) && !Player_SetupDive(globalCtx, this, sControlInput)) {
        Player_GetTargetVelocityAndYaw(this, &sp2C, &sp2A, 0.0f, globalCtx);

        if (sp2C == 0.0f) {
            Player_SetupSwimIdle(globalCtx, this);
        } else if (!Player_IsZTargetingSetupStartUnfriendly(this)) {
            Player_SetupSwim(globalCtx, this, sp2A);
        } else {
            func_8084D980(globalCtx, this, &sp2C, &sp2A);
        }

        Player_SetupSwimMovement(this, &this->linearVelocity, sp2C, sp2A);
    }
}

void func_8084DBC4(GlobalContext* globalCtx, Player* this, f32 arg2) {
    f32 sp2C;
    s16 sp2A;

    Player_GetTargetVelocityAndYaw(this, &sp2C, &sp2A, 0.0f, globalCtx);
    Player_UpdateSwimMovement(this, &this->linearVelocity, sp2C * 0.5f, sp2A);
    Player_UpdateSwimMovement(this, &this->actor.velocity.y, arg2, this->currentYaw);
}

void Player_Dive(Player* this, GlobalContext* globalCtx) {
    f32 sp2C;

    this->stateFlags2 |= PLAYER_STATE2_DISABLE_MOVE_ROTATION_WHILE_Z_TARGETING;
    this->actor.gravity = 0.0f;
    Player_SetupCurrentUpperAction(this, globalCtx);

    if (!Player_SetupItemCutsceneOrFirstPerson(this, globalCtx)) {
        if (this->currentBoots == PLAYER_BOOTS_IRON) {
            Player_SetupSwimIdle(globalCtx, this);
            return;
        }

        if (this->genericVar == 0) {
            if (this->genericTimer == 0) {
                if (LinkAnimation_Update(globalCtx, &this->skelAnime) ||
                    ((this->skelAnime.curFrame >= 22.0f) && !CHECK_BTN_ALL(sControlInput->cur.button, BTN_A))) {
                    Player_RiseFromDive(globalCtx, this);
                } else if (LinkAnimation_OnFrame(&this->skelAnime, 20.0f) != 0) {
                    this->actor.velocity.y = -2.0f;
                }

                Player_StepLinearVelocityToZero(this);
                return;
            }

            func_8084B158(globalCtx, this, sControlInput, this->actor.velocity.y);
            this->shapePitchOffset = 16000;

            if (CHECK_BTN_ALL(sControlInput->cur.button, BTN_A) &&
                !Player_SetupGetItemOrHoldBehavior(this, globalCtx) && !(this->actor.bgCheckFlags & 1) &&
                (this->actor.yDistToWater < D_80854784[CUR_UPG_VALUE(UPG_SCALE)])) {
                func_8084DBC4(globalCtx, this, -2.0f);
            } else {
                this->genericVar++;
                Player_ChangeAnimLongMorphLoop(globalCtx, this, &gPlayerAnim_003328);
            }
        } else if (this->genericVar == 1) {
            LinkAnimation_Update(globalCtx, &this->skelAnime);
            func_8084B000(this);

            if (this->shapePitchOffset < 10000) {
                this->genericVar++;
                this->genericTimer = this->actor.yDistToWater;
                Player_ChangeAnimLongMorphLoop(globalCtx, this, &gPlayerAnim_0032F0);
            }
        } else if (!Player_SetupDive(globalCtx, this, sControlInput)) {
            sp2C = (this->genericTimer * 0.018f) + 4.0f;

            if (this->stateFlags1 & PLAYER_STATE1_HOLDING_ACTOR) {
                sControlInput = NULL;
            }

            func_8084B158(globalCtx, this, sControlInput, fabsf(this->actor.velocity.y));
            Math_ScaledStepToS(&this->shapePitchOffset, -10000, 800);

            if (sp2C > 8.0f) {
                sp2C = 8.0f;
            }

            func_8084DBC4(globalCtx, this, sp2C);
        }
    }
}

void func_8084DF6C(GlobalContext* globalCtx, Player* this) {
    this->giDrawIdPlusOne = 0;
    this->stateFlags1 &= ~(PLAYER_STATE1_GETTING_ITEM | PLAYER_STATE1_HOLDING_ACTOR);
    this->getItemId = GI_NONE;
    func_8005B1A4(Gameplay_GetCamera(globalCtx, 0));
}

void func_8084DFAC(GlobalContext* globalCtx, Player* this) {
    func_8084DF6C(globalCtx, this);
    Player_AddRootYawToShapeYaw(this);
    Player_SetupStandingStillNoMorph(this, globalCtx);
    this->currentYaw = this->actor.shape.rot.y;
}

s32 func_8084DFF4(GlobalContext* globalCtx, Player* this) {
    GetItemEntry* giEntry;
    s32 temp1;
    s32 temp2;

    if (this->getItemId == GI_NONE) {
        return 1;
    }

    if (this->genericVar == 0) {
        giEntry = &sGetItemTable[this->getItemId - 1];
        this->genericVar = 1;

        // make sure we get the BGS instead of giant's knife
        if(this->getItemId == GI_SWORD_BGS) {
            gSaveContext.bgsFlag = 1;
            gSaveContext.swordHealth = 8;       
        }

        Message_StartTextbox(globalCtx, giEntry->textId, &this->actor);
        Item_Give(globalCtx, giEntry->itemId);

        if (((this->getItemId >= GI_RUPEE_GREEN) && (this->getItemId <= GI_RUPEE_RED)) ||
            ((this->getItemId >= GI_RUPEE_PURPLE) && (this->getItemId <= GI_RUPEE_GOLD)) ||
            ((this->getItemId >= GI_RUPEE_GREEN_LOSE) && (this->getItemId <= GI_RUPEE_PURPLE_LOSE)) ||
            (this->getItemId == GI_HEART)) {
            Audio_PlaySoundGeneral(NA_SE_SY_GET_BOXITEM, &D_801333D4, 4, &D_801333E0, &D_801333E0, &D_801333E8);
        } else {
            if ((this->getItemId == GI_HEART_CONTAINER_2) || (this->getItemId == GI_HEART_CONTAINER) ||
                ((this->getItemId == GI_HEART_PIECE) &&
                 ((gSaveContext.inventory.questItems & 0xF0000000) == 0x40000000))) {
                temp1 = NA_BGM_HEART_GET | 0x900;
            } else {
                temp1 = temp2 = (this->getItemId == GI_HEART_PIECE) ? NA_BGM_SMALL_ITEM_GET : NA_BGM_ITEM_GET | 0x900;
            }
            Audio_PlayFanfare(temp1);
        }
    } else {
        if (Message_GetState(&globalCtx->msgCtx) == TEXT_STATE_CLOSING) {
            if (this->getItemId == GI_GAUNTLETS_SILVER && !gSaveContext.n64ddFlag) {
                globalCtx->nextEntranceIndex = 0x0123;
                globalCtx->sceneLoadFlag = 0x14;
                gSaveContext.nextCutsceneIndex = 0xFFF1;
                globalCtx->fadeTransition = 0xF;
                this->stateFlags1 &= ~PLAYER_STATE1_IN_CUTSCENE;
                Player_SetupPlayerCutscene(globalCtx, NULL, 8);
            }
            this->getItemId = GI_NONE;
        }
    }

    return 0;
}

void Player_GetItemInWater(Player* this, GlobalContext* globalCtx) {
    this->stateFlags2 |= PLAYER_STATE2_DISABLE_MOVE_ROTATION_WHILE_Z_TARGETING;

    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        if (!(this->stateFlags1 & PLAYER_STATE1_GETTING_ITEM) || func_8084DFF4(globalCtx, this)) {
            func_8084DF6C(globalCtx, this);
            Player_SetupSwimIdle(globalCtx, this);
            Player_ResetSubCam(globalCtx, this);
        }
    } else {
        if ((this->stateFlags1 & PLAYER_STATE1_GETTING_ITEM) && LinkAnimation_OnFrame(&this->skelAnime, 10.0f)) {
            Player_SetGetItemDrawIdPlusOne(this, globalCtx);
            Player_ResetSubCam(globalCtx, this);
            Player_SetCameraTurnAround(globalCtx, 8);
        } else if (LinkAnimation_OnFrame(&this->skelAnime, 5.0f)) {
            Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_BREATH_DRINK);
        }
    }

    func_8084B000(this);
    Player_UpdateSwimMovement(this, &this->linearVelocity, 0.0f, this->actor.shape.rot.y);
}

void Player_DamagedSwim(Player* this, GlobalContext* globalCtx) {
    func_8084B000(this);

    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        Player_SetupSwimIdle(globalCtx, this);
    }

    Player_UpdateSwimMovement(this, &this->linearVelocity, 0.0f, this->actor.shape.rot.y);
}

void Player_Drown(Player* this, GlobalContext* globalCtx) {
    func_8084B000(this);

    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        func_80843AE8(globalCtx, this);
    }

    Player_UpdateSwimMovement(this, &this->linearVelocity, 0.0f, this->actor.shape.rot.y);
}

static s16 D_808549D4[] = { 0x0600, 0x04F6, 0x0604, 0x01F1, 0x0568, 0x05F4 };

void Player_PlayOcarina(Player* this, GlobalContext* globalCtx) {
    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        Player_PlayAnimLoopSlowed(globalCtx, this, &gPlayerAnim_0030A8);
        this->genericTimer = 1;
        if (this->stateFlags2 & (PLAYER_STATE2_NEAR_OCARINA_ACTOR | PLAYER_STATE2_PLAYING_OCARINA_FOR_ACTOR)) {
            this->stateFlags2 |= PLAYER_STATE2_ATTEMPT_PLAY_OCARINA_FOR_ACTOR;
        } else {
            func_8010BD58(globalCtx, OCARINA_ACTION_FREE_PLAY);
        }
        return;
    }

    if (this->genericTimer == 0) {
        return;
    }

    if (globalCtx->msgCtx.ocarinaMode == OCARINA_MODE_04) {
        func_8005B1A4(Gameplay_GetCamera(globalCtx, 0));

        if ((this->talkActor != NULL) && (this->talkActor == this->ocarinaActor)) {
            Player_StartTalkingWithActor(globalCtx, this->talkActor);
        } else if (this->naviTextId < 0) {
            this->talkActor = this->naviActor;
            this->naviActor->textId = -this->naviTextId;
            Player_StartTalkingWithActor(globalCtx, this->talkActor);
        } else if (!Player_SetupItemCutsceneOrFirstPerson(this, globalCtx)) {
            Player_SetupReturnToStandStillSetAnim(this, &gPlayerAnim_003098, globalCtx);
        }

        this->stateFlags2 &= ~(PLAYER_STATE2_NEAR_OCARINA_ACTOR | PLAYER_STATE2_ATTEMPT_PLAY_OCARINA_FOR_ACTOR |
                               PLAYER_STATE2_PLAYING_OCARINA_FOR_ACTOR);
        this->ocarinaActor = NULL;
    } else if (globalCtx->msgCtx.ocarinaMode == OCARINA_MODE_02) {
        gSaveContext.respawn[RESPAWN_MODE_RETURN].entranceIndex = D_808549D4[globalCtx->msgCtx.lastPlayedSong];
        gSaveContext.respawn[RESPAWN_MODE_RETURN].playerParams = 0x5FF;
        gSaveContext.respawn[RESPAWN_MODE_RETURN].data = globalCtx->msgCtx.lastPlayedSong;

        this->csMode = 0;
        this->stateFlags1 &= ~PLAYER_STATE1_IN_CUTSCENE;

        Player_SetupPlayerCutscene(globalCtx, NULL, 8);
        globalCtx->mainCamera.unk_14C &= ~8;

        this->stateFlags1 |= PLAYER_STATE1_SKIP_OTHER_ACTORS_UPDATE | PLAYER_STATE1_IN_CUTSCENE;
        this->stateFlags2 |= PLAYER_STATE2_PLAYING_OCARINA_GENERAL;

        if (Actor_Spawn(&globalCtx->actorCtx, globalCtx, ACTOR_DEMO_KANKYO, 0.0f, 0.0f, 0.0f, 0, 0, 0, 0xF) == NULL) {
            Environment_WarpSongLeave(globalCtx);
        }

        gSaveContext.seqId = (u8)NA_BGM_DISABLED;
        gSaveContext.natureAmbienceId = NATURE_ID_DISABLED;
    }
}

void Player_ThrowDekuNut(Player* this, GlobalContext* globalCtx) {
    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        Player_SetupReturnToStandStillSetAnim(this, &gPlayerAnim_003050, globalCtx);
    } else if (LinkAnimation_OnFrame(&this->skelAnime, 3.0f)) {
        Inventory_ChangeAmmo(ITEM_NUT, -1);
        Actor_Spawn(&globalCtx->actorCtx, globalCtx, ACTOR_EN_ARROW, this->bodyPartsPos[PLAYER_BODYPART_R_HAND].x,
                    this->bodyPartsPos[PLAYER_BODYPART_R_HAND].y, this->bodyPartsPos[PLAYER_BODYPART_R_HAND].z, 4000,
                    this->actor.shape.rot.y, 0, ARROW_NUT);
        Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_SWORD_N);
    }

    Player_StepLinearVelocityToZero(this);
}

static PlayerAnimSfxEntry D_808549E0[] = {
    { 0, 0x3857 },
    { NA_SE_VO_LI_CLIMB_END, 0x2057 },
    { NA_SE_VO_LI_AUTO_JUMP, 0x2045 },
    { 0, -0x287B },
};

void Player_GetItem(Player* this, GlobalContext* globalCtx) {
    s32 cond;

    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        if (this->genericTimer != 0) {
            if (this->genericTimer >= 2) {
                this->genericTimer--;
            }

            if (func_8084DFF4(globalCtx, this) && (this->genericTimer == 1)) {
                cond = ((this->talkActor != NULL) && (this->exchangeItemId < 0)) ||
                       (this->stateFlags3 & PLAYER_STATE3_FORCE_PULL_OCARINA);

                if (cond || (gSaveContext.healthAccumulator == 0)) {
                    if (cond) {
                        func_8084DF6C(globalCtx, this);
                        this->exchangeItemId = EXCH_ITEM_NONE;

                        if (func_8084B4D4(globalCtx, this) == 0) {
                            Player_StartTalkingWithActor(globalCtx, this->talkActor);
                        }
                    } else {
                        func_8084DFAC(globalCtx, this);
                    }
                }
            }
        } else {
            Player_EndAnimMovement(this);

            if (this->getItemId == GI_ICE_TRAP) {
                this->stateFlags1 &= ~(PLAYER_STATE1_GETTING_ITEM | PLAYER_STATE1_HOLDING_ACTOR);

                if (this->getItemId != GI_ICE_TRAP) {
                    Actor_Spawn(&globalCtx->actorCtx, globalCtx, ACTOR_EN_CLEAR_TAG, this->actor.world.pos.x,
                                this->actor.world.pos.y + 100.0f, this->actor.world.pos.z, 0, 0, 0, 0);
                    Player_SetupStandingStillNoMorph(this, globalCtx);
                } else {
                    this->actor.colChkInfo.damage = 0;
                    Player_SetupDamage(globalCtx, this, 3, 0.0f, 0.0f, 0, 20);
                }
                return;
            }

            if (this->skelAnime.animation == &gPlayerAnim_002DF8) {
                Player_PlayAnimOnceSlowed(globalCtx, this, &gPlayerAnim_002788);
            } else {
                Player_PlayAnimOnceSlowed(globalCtx, this, &gPlayerAnim_002780);
            }

            this->genericTimer = 2;
            Player_SetCameraTurnAround(globalCtx, 9);
        }
    } else {
        if (this->genericTimer == 0) {
            if (!LINK_IS_ADULT) {
                Player_PlayAnimSfx(this, D_808549E0);
            }
            return;
        }

        if (this->skelAnime.animation == &gPlayerAnim_002788) {
            Math_ScaledStepToS(&this->actor.shape.rot.y, Camera_GetCamDirYaw(GET_ACTIVE_CAM(globalCtx)) + 0x8000, 4000);
        }

        if (LinkAnimation_OnFrame(&this->skelAnime, 21.0f)) {
            Player_SetGetItemDrawIdPlusOne(this, globalCtx);
        }
    }
}

static PlayerAnimSfxEntry D_808549F0[] = {
    { NA_SE_IT_MASTER_SWORD_SWING, -0x83C },
};

void func_8084E988(Player* this) {
    Player_PlayAnimSfx(this, D_808549F0);
}

static PlayerAnimSfxEntry D_808549F4[] = {
    { NA_SE_VO_LI_AUTO_JUMP, 0x2005 },
    { 0, -0x280F },
};

void Player_EndTimeTravel(Player* this, GlobalContext* globalCtx) {
    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        if (this->genericVar == 0) {
            if (DECR(this->genericTimer) == 0) {
                this->genericVar = 1;
                this->skelAnime.endFrame = this->skelAnime.animLength - 1.0f;
            }
        } else {
            Player_SetupStandingStillNoMorph(this, globalCtx);
        }
    } else {
        if (LINK_IS_ADULT && LinkAnimation_OnFrame(&this->skelAnime, 158.0f)) {
            Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_SWORD_N);
            return;
        }

        if (!LINK_IS_ADULT) {
            Player_PlayAnimSfx(this, D_808549F4);
        } else {
            func_8084E988(this);
        }
    }
}

static u8 D_808549FC[] = {
    0x01, 0x03, 0x02, 0x04, 0x04,
};

void Player_DrinkFromBottle(Player* this, GlobalContext* globalCtx) {
    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        if (this->genericTimer == 0) {
            if (this->itemActionParam == PLAYER_AP_BOTTLE_POE) {
                s32 rand = Rand_S16Offset(-1, 3);

                if (rand == 0) {
                    rand = 3;
                }

                if ((rand < 0) && (gSaveContext.health <= 0x10)) {
                    rand = 3;
                }

                if (rand < 0) {
                    Health_ChangeBy(globalCtx, -0x10);
                } else {
                    gSaveContext.healthAccumulator = rand * 0x10;
                }
            } else {
                s32 sp28 = D_808549FC[this->itemActionParam - PLAYER_AP_BOTTLE_POTION_RED];

                if (CVar_GetS32("gRedPotionEffect", 0) && this->itemActionParam == PLAYER_AP_BOTTLE_POTION_RED) {
                    if (CVar_GetS32("gRedPercentRestore", 0)) {
                        gSaveContext.healthAccumulator =
                            (gSaveContext.healthCapacity * CVar_GetS32("gRedPotionHealth", 100) / 100 + 15) / 16 * 16;
                    } else {
                        gSaveContext.healthAccumulator = CVar_GetS32("gRedPotionHealth", 20) * 16;
                    }
                } else if (CVar_GetS32("gBluePotionEffects", 0) &&
                           this->itemActionParam == PLAYER_AP_BOTTLE_POTION_BLUE) {
                    if (CVar_GetS32("gBlueHealthPercentRestore", 0)) {
                        gSaveContext.healthAccumulator =
                            (gSaveContext.healthCapacity * CVar_GetS32("gBluePotionHealth", 100) / 100 + 15) / 16 * 16;
                    } else {
                        gSaveContext.healthAccumulator = CVar_GetS32("gBluePotionHealth", 20) * 16;
                    }

                    if (CVar_GetS32("gBlueManaPercentRestore", 0)) {
                        if (gSaveContext.unk_13F0 != 10) {
                            Magic_Fill(globalCtx);
                        }

                        func_80087708(globalCtx,
                                      (gSaveContext.magicLevel * 48 * CVar_GetS32("gBluePotionMana", 100) / 100 + 15) /
                                          16 * 16,
                                      5);
                    } else {
                        if (gSaveContext.unk_13F0 != 10) {
                            Magic_Fill(globalCtx);
                        }

                        func_80087708(globalCtx, CVar_GetS32("gBluePotionMana", 100), 5);
                        ;
                    }
                } else if (CVar_GetS32("gGreenPotionEffect", 0) &&
                           this->itemActionParam == PLAYER_AP_BOTTLE_POTION_GREEN) {
                    if (CVar_GetS32("gGreenPercentRestore", 0)) {
                        if (gSaveContext.unk_13F0 != 10) {
                            Magic_Fill(globalCtx);
                        }

                        func_80087708(globalCtx,
                                      (gSaveContext.magicLevel * 48 * CVar_GetS32("gGreenPotionMana", 100) / 100 + 15) /
                                          16 * 16,
                                      5);
                    } else {
                        if (gSaveContext.unk_13F0 != 10) {
                            Magic_Fill(globalCtx);
                        }

                        func_80087708(globalCtx, CVar_GetS32("gGreenPotionMana", 100), 5);
                        ;
                    }
                } else if (CVar_GetS32("gMilkEffect", 0) && (this->itemActionParam == PLAYER_AP_BOTTLE_MILK ||
                                                             this->itemActionParam == PLAYER_AP_BOTTLE_MILK_HALF)) {
                    if (CVar_GetS32("gMilkPercentRestore", 0)) {
                        gSaveContext.healthAccumulator =
                            (gSaveContext.healthCapacity * CVar_GetS32("gMilkHealth", 100) / 100 + 15) / 16 * 16;
                    } else {
                        gSaveContext.healthAccumulator = CVar_GetS32("gMilkHealth", 5) * 16;
                    }
                    if (CVar_GetS32("gSeparateHalfMilkEffect", 0) &&
                        this->itemActionParam == PLAYER_AP_BOTTLE_MILK_HALF) {
                        if (CVar_GetS32("gHalfMilkPercentRestore", 0)) {
                            gSaveContext.healthAccumulator =
                                (gSaveContext.healthCapacity * CVar_GetS32("gHalfMilkHealth", 100) / 100 + 15) / 16 *
                                16;
                        } else {
                            gSaveContext.healthAccumulator = CVar_GetS32("gHalfMilkHealth", 5) * 16;
                        }
                    }
                } else {
                    if (sp28 & 1) {
                        gSaveContext.healthAccumulator = 0x140;
                    }

                    if (sp28 & 2) {
                        Magic_Fill(globalCtx);
                    }

                    if (sp28 & 4) {
                        gSaveContext.healthAccumulator = 0x50;
                    }
                }
            }

            Player_PlayAnimLoopSlowed(globalCtx, this, &gPlayerAnim_002670);
            this->genericTimer = 1;
            return;
        }

        Player_SetupStandingStillNoMorph(this, globalCtx);
        func_8005B1A4(Gameplay_GetCamera(globalCtx, 0));
    } else if (this->genericTimer == 1) {
        if ((gSaveContext.healthAccumulator == 0) && (gSaveContext.unk_13F0 != 9)) {
            Player_ChangeAnimSlowedMorphToLastFrame(globalCtx, this, &gPlayerAnim_002660);
            this->genericTimer = 2;
            Player_UpdateBottleHeld(globalCtx, this, ITEM_BOTTLE, PLAYER_AP_BOTTLE);
        }
        Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_DRINK - SFX_FLAG);
    } else if ((this->genericTimer == 2) && LinkAnimation_OnFrame(&this->skelAnime, 29.0f)) {
        Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_BREATH_DRINK);
    }
}

static BottleCatchInfo D_80854A04[] = {
    { ACTOR_EN_ELF, ITEM_FAIRY, 0x2A, 0x46 },
    { ACTOR_EN_FISH, ITEM_FISH, 0x1F, 0x47 },
    { ACTOR_EN_ICE_HONO, ITEM_BLUE_FIRE, 0x20, 0x5D },
    { ACTOR_EN_INSECT, ITEM_BUG, 0x21, 0x7A },
};

void Player_SwingBottle(Player* this, GlobalContext* globalCtx) {
    BottleSwingAnimInfo* sp24;
    BottleCatchInfo* catchInfo;
    s32 temp;
    s32 i;

    sp24 = &sBottleSwingAnims[this->genericTimer];
    Player_StepLinearVelocityToZero(this);

    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        if (this->genericVar != 0) {
            if (this->genericTimer == 0) {
                Message_StartTextbox(globalCtx, D_80854A04[this->genericVar - 1].textId, &this->actor);
                Audio_PlayFanfare(NA_BGM_ITEM_GET | 0x900);
                this->genericTimer = 1;
            } else if (Message_GetState(&globalCtx->msgCtx) == TEXT_STATE_CLOSING) {
                this->genericVar = 0;
                func_8005B1A4(Gameplay_GetCamera(globalCtx, 0));
            }
        } else {
            Player_SetupStandingStillNoMorph(this, globalCtx);
        }
    } else {
        if (this->genericVar == 0) {
            temp = this->skelAnime.curFrame - sp24->unk_08;

            if (temp >= 0) {
                if (sp24->unk_09 >= temp) {
                    if (this->genericTimer != 0) {
                        if (temp == 0) {
                            func_8002F7DC(&this->actor, NA_SE_IT_SCOOP_UP_WATER);
                        }
                    }

                    if (this->interactRangeActor != NULL) {
                        catchInfo = &D_80854A04[0];
                        for (i = 0; i < 4; i++, catchInfo++) {
                            if (this->interactRangeActor->id == catchInfo->actorId) {
                                break;
                            }
                        }

                        if (i < 4) {
                            this->genericVar = i + 1;
                            this->genericTimer = 0;
                            this->stateFlags1 |= PLAYER_STATE1_SKIP_OTHER_ACTORS_UPDATE | PLAYER_STATE1_IN_CUTSCENE;
                            this->interactRangeActor->parent = &this->actor;
                            Player_UpdateBottleHeld(globalCtx, this, catchInfo->itemId, ABS(catchInfo->actionParam));
                            Player_PlayAnimOnceSlowed(globalCtx, this, sp24->bottleCatchAnim);
                            Player_SetCameraTurnAround(globalCtx, 4);
                        }
                    }
                }
            }
        }
    }

    if (this->skelAnime.curFrame <= 7.0f) {
        this->stateFlags1 |= PLAYER_STATE1_SWINGING_BOTTLE;
    }
}

static Vec3f D_80854A1C = { 0.0f, 0.0f, 5.0f };

void Player_HealWithFairy(Player* this, GlobalContext* globalCtx) {
    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        Player_SetupStandingStillNoMorph(this, globalCtx);
        func_8005B1A4(Gameplay_GetCamera(globalCtx, 0));
        return;
    }

    if (LinkAnimation_OnFrame(&this->skelAnime, 37.0f)) {
        Player_SpawnFairy(globalCtx, this, &this->leftHandPos, &D_80854A1C, FAIRY_REVIVE_BOTTLE);
        Player_UpdateBottleHeld(globalCtx, this, ITEM_BOTTLE, PLAYER_AP_BOTTLE);
        func_8002F7DC(&this->actor, NA_SE_EV_BOTTLE_CAP_OPEN);
        func_8002F7DC(&this->actor, NA_SE_EV_FIATY_HEAL - SFX_FLAG);
    } else if (LinkAnimation_OnFrame(&this->skelAnime, 47.0f)) {
        if (CVar_GetS32("gFairyEffect", 0)) {
            if (CVar_GetS32("gFairyPercentRestore", 0)) {
                gSaveContext.healthAccumulator =
                    (gSaveContext.healthCapacity * CVar_GetS32("gFairyHealth", 100) / 100 + 15) / 16 * 16;
            } else {
                gSaveContext.healthAccumulator = CVar_GetS32("gFairyHealth", 8) * 16;
            }
        } else {
            gSaveContext.healthAccumulator = 0x140;
        }
    }
}

static BottleDropInfo D_80854A28[] = {
    { ACTOR_EN_FISH, FISH_DROPPED },
    { ACTOR_EN_ICE_HONO, 0 },
    { ACTOR_EN_INSECT, 2 },
};

static PlayerAnimSfxEntry D_80854A34[] = {
    { NA_SE_VO_LI_AUTO_JUMP, 0x2026 },
    { NA_SE_EV_BOTTLE_CAP_OPEN, -0x828 },
};

void Player_DropItemFromBottle(Player* this, GlobalContext* globalCtx) {
    Player_StepLinearVelocityToZero(this);

    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        Player_SetupStandingStillNoMorph(this, globalCtx);
        func_8005B1A4(Gameplay_GetCamera(globalCtx, 0));
        return;
    }

    if (LinkAnimation_OnFrame(&this->skelAnime, 76.0f)) {
        BottleDropInfo* dropInfo = &D_80854A28[this->itemActionParam - PLAYER_AP_BOTTLE_FISH];

        Actor_Spawn(&globalCtx->actorCtx, globalCtx, dropInfo->actorId,
                    (Math_SinS(this->actor.shape.rot.y) * 5.0f) + this->leftHandPos.x, this->leftHandPos.y,
                    (Math_CosS(this->actor.shape.rot.y) * 5.0f) + this->leftHandPos.z, 0x4000, this->actor.shape.rot.y,
                    0, dropInfo->actorParams);

        Player_UpdateBottleHeld(globalCtx, this, ITEM_BOTTLE, PLAYER_AP_BOTTLE);
        return;
    }

    Player_PlayAnimSfx(this, D_80854A34);
}

static PlayerAnimSfxEntry D_80854A3C[] = {
    { NA_SE_PL_PUT_OUT_ITEM, -0x81E },
};

void Player_PresentExchangeItem(Player* this, GlobalContext* globalCtx) {
    this->stateFlags2 |= PLAYER_STATE2_DISABLE_MOVE_ROTATION_WHILE_Z_TARGETING;

    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        if (this->genericTimer < 0) {
            Player_SetupStandingStillNoMorph(this, globalCtx);
        } else if (this->exchangeItemId == EXCH_ITEM_NONE) {
            Actor* targetActor = this->talkActor;

            this->giDrawIdPlusOne = 0;
            if (targetActor->textId != 0xFFFF) {
                this->actor.flags |= ACTOR_FLAG_8;
            }

            Player_StartTalkingWithActor(globalCtx, targetActor);
        } else {
            GetItemEntry* giEntry = &sGetItemTable[sExchangeGetItemIDs[this->exchangeItemId - 1] - 1];

            if (this->itemActionParam >= PLAYER_AP_LETTER_ZELDA) {
                if (giEntry->gi >= 0) {
                    this->giDrawIdPlusOne = giEntry->gi;
                } else {
                    this->giDrawIdPlusOne = -giEntry->gi;
                }
            }

            if (this->genericTimer == 0) {
                Message_StartTextbox(globalCtx, this->actor.textId, &this->actor);

                if ((this->itemActionParam == PLAYER_AP_CHICKEN) || (this->itemActionParam == PLAYER_AP_POCKET_CUCCO)) {
                    func_8002F7DC(&this->actor, NA_SE_EV_CHICKEN_CRY_M);
                }

                this->genericTimer = 1;
            } else if (Message_GetState(&globalCtx->msgCtx) == TEXT_STATE_CLOSING) {
                this->actor.flags &= ~ACTOR_FLAG_8;
                this->giDrawIdPlusOne = 0;

                if (this->genericVar == 1) {
                    Player_PlayAnimOnce(globalCtx, this, &gPlayerAnim_002698);
                    this->genericTimer = -1;
                } else {
                    Player_SetupStandingStillNoMorph(this, globalCtx);
                }

                func_8005B1A4(Gameplay_GetCamera(globalCtx, 0));
            }
        }
    } else if (this->genericTimer >= 0) {
        Player_PlayAnimSfx(this, D_80854A3C);
    }

    if ((this->genericVar == 0) && (this->targetActor != NULL)) {
        this->currentYaw = this->actor.shape.rot.y = Player_LookAtTargetActor(this, 0);
    }
}

void func_8084F308(Player* this, GlobalContext* globalCtx) {
    this->stateFlags2 |=
        PLAYER_STATE2_DISABLE_MOVE_ROTATION_WHILE_Z_TARGETING | PLAYER_STATE2_ALWAYS_DISABLE_MOVE_ROTATION;

    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        Player_PlayAnimLoop(globalCtx, this, &gPlayerAnim_003128);
    }

    if (Player_MashTimerThresholdExceeded(this, 0, 100)) {
        Player_SetupStandingStillType(this, globalCtx);
        this->stateFlags2 &= ~PLAYER_STATE2_RESTRAINED_BY_ENEMY;
    }
}

void Player_SlipOnSlope(Player* this, GlobalContext* globalCtx) {
    CollisionPoly* floorPoly;
    f32 sp50;
    f32 sp4C;
    f32 sp48;
    s16 sp46;
    s16 sp44;
    Vec3f sp38;

    this->stateFlags2 |=
        PLAYER_STATE2_DISABLE_MOVE_ROTATION_WHILE_Z_TARGETING | PLAYER_STATE2_ALWAYS_DISABLE_MOVE_ROTATION;
    LinkAnimation_Update(globalCtx, &this->skelAnime);
    Player_SetupSpawnDustAtFeet(globalCtx, this);
    func_800F4138(&this->actor.projectedPos, NA_SE_PL_SLIP_LEVEL - SFX_FLAG, this->actor.speedXZ);

    if (Player_SetupItemCutsceneOrFirstPerson(this, globalCtx) == 0) {
        floorPoly = this->actor.floorPoly;

        if (floorPoly == NULL) {
            Player_SetupFallFromLedge(this, globalCtx);
            return;
        }

        func_8083E298(floorPoly, &sp38, &sp46);

        sp44 = sp46;
        if (this->genericVar != 0) {
            sp44 = sp46 + 0x8000;
        }

        if (this->linearVelocity < 0) {
            sp46 += 0x8000;
        }

        sp50 = (1.0f - sp38.y) * 40.0f;
        sp50 = CLAMP(sp50, 0, 10.0f);
        sp4C = (sp50 * sp50) * 0.015f;
        sp48 = sp38.y * 0.01f;

        if (SurfaceType_GetSlope(&globalCtx->colCtx, floorPoly, this->actor.floorBgId) != 1) {
            sp50 = 0;
            sp48 = sp38.y * 10.0f;
        }

        if (sp4C < 1.0f) {
            sp4C = 1.0f;
        }

        if (Math_AsymStepToF(&this->linearVelocity, sp50, sp4C, sp48) && (sp50 == 0)) {
            LinkAnimationHeader* anim;
            if (this->genericVar == 0) {
                anim = GET_PLAYER_ANIM(PLAYER_ANIMGROUP_SLIDING_DOWN_SLOPE, this->modelAnimType);
            } else {
                anim = GET_PLAYER_ANIM(PLAYER_ANIMGROUP_END_SLIDING_DOWN_SLOPE, this->modelAnimType);
            }
            Player_SetupReturnToStandStillSetAnim(this, anim, globalCtx);
        }

        Math_SmoothStepToS(&this->currentYaw, sp46, 10, 4000, 800);
        Math_ScaledStepToS(&this->actor.shape.rot.y, sp44, 2000);
    }
}

void Player_SetDrawAndStartCutsceneAfterTimer(Player* this, GlobalContext* globalCtx) {
    if ((DECR(this->genericTimer) == 0) && Player_SetupCutscene(globalCtx, this)) {
        Player_CutsceneSetDraw(globalCtx, this, NULL);
        Player_SetActionFunc(globalCtx, this, Player_StartCutscene, 0);
        Player_StartCutscene(this, globalCtx);
    }
}

void Player_SpawnFromWarpSong(Player* this, GlobalContext* globalCtx) {
    Player_SetActionFunc(globalCtx, this, Player_SetDrawAndStartCutsceneAfterTimer, 0);
    this->genericTimer = 40;
    Actor_Spawn(&globalCtx->actorCtx, globalCtx, ACTOR_DEMO_KANKYO, 0.0f, 0.0f, 0.0f, 0, 0, 0, 0x10);
}

void Player_SpawnFromBlueWarp(Player* this, GlobalContext* globalCtx) {
    s32 pad;

    if ((this->genericVar != 0) && (globalCtx->csCtx.frames < 0x131)) {
        this->actor.gravity = 0.0f;
        this->actor.velocity.y = 0.0f;
    } else if (sPlayerYDistToFloor < 150.0f) {
        if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
            if (this->genericTimer == 0) {
                if (this->actor.bgCheckFlags & 1) {
                    this->skelAnime.endFrame = this->skelAnime.animLength - 1.0f;
                    Player_PlayLandingSfx(this);
                    this->genericTimer = 1;
                }
            } else {
                if ((globalCtx->sceneNum == SCENE_SPOT04) && Player_SetupCutscene(globalCtx, this)) {
                    return;
                }
                Player_SetupStandingStillMorph(this, globalCtx);
            }
        }
        Math_SmoothStepToF(&this->actor.velocity.y, 2.0f, 0.3f, 8.0f, 0.5f);
    }

    if ((globalCtx->sceneNum == SCENE_KENJYANOMA) && Player_SetupCutscene(globalCtx, this)) {
        return;
    }

    if ((globalCtx->csCtx.state != CS_STATE_IDLE) && (globalCtx->csCtx.linkAction != NULL)) {
        f32 sp28 = this->actor.world.pos.y;
        Player_CutsceneSetPosAndYaw(globalCtx, this, globalCtx->csCtx.linkAction);
        this->actor.world.pos.y = sp28;
    }
}

void Player_EnterGrotto(Player* this, GlobalContext* globalCtx) {
    LinkAnimation_Update(globalCtx, &this->skelAnime);

    if ((this->genericTimer++ > 8) && (globalCtx->sceneLoadFlag == 0)) {

        if (this->genericVar != 0) {
            if (globalCtx->sceneNum == 9) {
                Gameplay_TriggerRespawn(globalCtx);
                globalCtx->nextEntranceIndex = 0x0088;
            } else if (this->genericVar < 0) {
                Gameplay_TriggerRespawn(globalCtx);
            } else {
                Gameplay_TriggerVoidOut(globalCtx);
            }

            globalCtx->fadeTransition = 4;
            func_80078884(NA_SE_OC_ABYSS);
        } else {
            globalCtx->fadeTransition = 2;
            gSaveContext.nextTransition = 2;
            gSaveContext.seqId = (u8)NA_BGM_DISABLED;
            gSaveContext.natureAmbienceId = 0xFF;
        }

        globalCtx->sceneLoadFlag = 0x14;
    }
}

void Player_SetupOpenDoorFromSpawn(Player* this, GlobalContext* globalCtx) {
    Player_SetupOpenDoor(this, globalCtx);
}

void Player_JumpFromGrotto(Player* this, GlobalContext* globalCtx) {
    this->actor.gravity = -1.0f;

    LinkAnimation_Update(globalCtx, &this->skelAnime);

    if (this->actor.velocity.y < 0.0f) {
        Player_SetupFallFromLedge(this, globalCtx);
    } else if (this->actor.velocity.y < 6.0f) {
        Math_StepToF(&this->linearVelocity, 3.0f, 0.5f);
    }
}

void Player_ShootingGalleryPlay(Player* this, GlobalContext* globalCtx) {
    this->attentionMode = 2;

    Player_SetupCameraMode(globalCtx, this);
    LinkAnimation_Update(globalCtx, &this->skelAnime);
    Player_SetupCurrentUpperAction(this, globalCtx);

    this->upperBodyRot.y = func_8084ABD8(globalCtx, this, 1, 0) - this->actor.shape.rot.y;
    this->lookFlags |= 0x80;

    if (globalCtx->shootingGalleryStatus < 0) {
        globalCtx->shootingGalleryStatus++;
        if (globalCtx->shootingGalleryStatus == 0) {
            Player_ClearLookAndAttention(this, globalCtx);
        }
    }
}

void Player_FrozenInIce(Player* this, GlobalContext* globalCtx) {
    if (this->genericVar >= 0) {
        if (this->genericVar < 6) {
            this->genericVar++;
        }

        if (Player_MashTimerThresholdExceeded(this, 1, 100)) {
            this->genericVar = -1;
            EffectSsIcePiece_SpawnBurst(globalCtx, &this->actor.world.pos, this->actor.scale.x);
            func_8002F7DC(&this->actor, NA_SE_PL_ICE_BROKEN);
        } else {
            this->stateFlags2 |= PLAYER_STATE2_FROZEN_IN_ICE;
        }

        if ((globalCtx->gameplayFrames % 4) == 0) {
            Player_InflictDamage(globalCtx, -1);
        }
    } else {
        if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
            Player_SetupStandingStillType(this, globalCtx);
            Player_SetupInvincibilityNoDamageFlash(this, -20);
        }
    }
}

void Player_SetupElectricShock(Player* this, GlobalContext* globalCtx) {
    LinkAnimation_Update(globalCtx, &this->skelAnime);
    Player_RoundUpInvincibilityTimer(this);

    if (((this->genericTimer % 25) != 0) || Player_Damage(globalCtx, this, -1)) {
        if (DECR(this->genericTimer) == 0) {
            Player_SetupStandingStillType(this, globalCtx);
        }
    }

    this->shockTimer = 40;
    func_8002F8F0(&this->actor, NA_SE_VO_LI_TAKEN_AWAY - SFX_FLAG + this->ageProperties->unk_92);
}

s32 Player_CheckNoDebugModeCombo(Player* this, GlobalContext* globalCtx) {
    sControlInput = &globalCtx->state.input[0];

    if (CVar_GetS32("gDebugEnabled", 0) &&
        ((CHECK_BTN_ALL(sControlInput->cur.button, BTN_A | BTN_L | BTN_R) &&
          CHECK_BTN_ALL(sControlInput->press.button, BTN_B)) ||
         (CHECK_BTN_ALL(sControlInput->cur.button, BTN_L) && CHECK_BTN_ALL(sControlInput->press.button, BTN_DRIGHT)))) {

        sDebugModeFlag ^= 1;

        if (sDebugModeFlag) {
            Camera_ChangeMode(Gameplay_GetCamera(globalCtx, 0), CAM_MODE_BOWARROWZ);
        }
    }

    if (sDebugModeFlag) {
        f32 speed;

        if (CHECK_BTN_ALL(sControlInput->cur.button, BTN_R)) {
            speed = 100.0f;
        } else {
            speed = 20.0f;
        }

        func_8006375C(3, 2, "DEBUG MODE");

        if (!CHECK_BTN_ALL(sControlInput->cur.button, BTN_L)) {
            if (CHECK_BTN_ALL(sControlInput->cur.button, BTN_B)) {
                this->actor.world.pos.y += speed;
            } else if (CHECK_BTN_ALL(sControlInput->cur.button, BTN_A)) {
                this->actor.world.pos.y -= speed;
            }

            if (CHECK_BTN_ANY(sControlInput->cur.button, BTN_DUP | BTN_DLEFT | BTN_DDOWN | BTN_DRIGHT)) {
                s16 angle;
                s16 temp;

                angle = temp = Camera_GetInputDirYaw(GET_ACTIVE_CAM(globalCtx));

                if (CHECK_BTN_ALL(sControlInput->cur.button, BTN_DDOWN)) {
                    angle = temp + 0x8000;
                } else if (CHECK_BTN_ALL(sControlInput->cur.button, BTN_DLEFT)) {
                    angle = temp + 0x4000;
                } else if (CHECK_BTN_ALL(sControlInput->cur.button, BTN_DRIGHT)) {
                    angle = temp - 0x4000;
                }

                this->actor.world.pos.x += speed * Math_SinS(angle);
                this->actor.world.pos.z += speed * Math_CosS(angle);
            }
        }

        Player_StopMovement(this);

        this->actor.gravity = 0.0f;
        this->actor.velocity.z = 0.0f;
        this->actor.velocity.y = 0.0f;
        this->actor.velocity.x = 0.0f;

        if (CHECK_BTN_ALL(sControlInput->cur.button, BTN_L) && CHECK_BTN_ALL(sControlInput->press.button, BTN_DLEFT)) {
            Flags_SetTempClear(globalCtx, globalCtx->roomCtx.curRoom.num);
        }

        Math_Vec3f_Copy(&this->actor.home.pos, &this->actor.world.pos);

        return 0;
    }

    return 1;
}

void Player_BowStringMoveAfterShot(Player* this) {
    this->unk_858 += this->unk_85C;
    this->unk_85C -= this->unk_858 * 5.0f;
    this->unk_85C *= 0.3f;

    if (ABS(this->unk_85C) < 0.00001f) {
        this->unk_85C = 0.0f;
        if (ABS(this->unk_858) < 0.00001f) {
            this->unk_858 = 0.0f;
        }
    }
}

void Player_BunnyHoodPhysics(Player* this) {
    s32 pad;
    s16 sp2A;
    s16 sp28;
    s16 sp26;

    D_80858AC8.unk_06 -= D_80858AC8.unk_06 >> 3;
    D_80858AC8.unk_08 -= D_80858AC8.unk_08 >> 3;
    D_80858AC8.unk_06 += -D_80858AC8.unk_00 >> 2;
    D_80858AC8.unk_08 += -D_80858AC8.unk_02 >> 2;

    sp26 = this->actor.world.rot.y - this->actor.shape.rot.y;

    sp28 = (s32)(this->actor.speedXZ * -200.0f * Math_CosS(sp26) * (Rand_CenteredFloat(2.0f) + 10.0f)) & 0xFFFF;
    sp2A = (s32)(this->actor.speedXZ * 100.0f * Math_SinS(sp26) * (Rand_CenteredFloat(2.0f) + 10.0f)) & 0xFFFF;

    D_80858AC8.unk_06 += sp28 >> 2;
    D_80858AC8.unk_08 += sp2A >> 2;

    if (D_80858AC8.unk_06 > 6000) {
        D_80858AC8.unk_06 = 6000;
    } else if (D_80858AC8.unk_06 < -6000) {
        D_80858AC8.unk_06 = -6000;
    }

    if (D_80858AC8.unk_08 > 6000) {
        D_80858AC8.unk_08 = 6000;
    } else if (D_80858AC8.unk_08 < -6000) {
        D_80858AC8.unk_08 = -6000;
    }

    D_80858AC8.unk_00 += D_80858AC8.unk_06;
    D_80858AC8.unk_02 += D_80858AC8.unk_08;

    if (D_80858AC8.unk_00 < 0) {
        D_80858AC8.unk_04 = D_80858AC8.unk_00 >> 1;
    } else {
        D_80858AC8.unk_04 = 0;
    }
}

s32 Player_SetupStartMeleeWeaponAttack(Player* this, GlobalContext* globalCtx) {
    if (Player_CanSwingBottleOrCastFishingRod(globalCtx, this) == 0) {
        if (Player_CanJumpSlash(this) != 0) {
            s32 sp24 = Player_GetMeleeAttackAnim(this);

            Player_StartMeleeWeaponAttack(globalCtx, this, sp24);

            if (sp24 >= 0x18) {
                this->stateFlags2 |= PLAYER_STATE2_RELEASING_SPIN_ATTACK;
                Player_SetupSpinAttackActor(globalCtx, this, 0);
                return 1;
            }
        } else {
            return 0;
        }
    }

    return 1;
}

static Vec3f D_80854A40 = { 0.0f, 40.0f, 45.0f };

void Player_MeleeWeaponAttack(Player* this, GlobalContext* globalCtx) {
    MeleeAttackAnimInfo* sp44 = &sMeleeAttackAnims[this->swordAnimation];

    this->stateFlags2 |= PLAYER_STATE2_DISABLE_MOVE_ROTATION_WHILE_Z_TARGETING;

    if (!func_80842DF4(globalCtx, this)) {
        func_8084285C(this, 0.0f, sp44->unk_0C, sp44->unk_0D);

        if ((this->stateFlags2 & PLAYER_STATE2_ENABLE_FORWARD_SLIDE_FROM_ATTACK) &&
            (this->heldItemActionParam != PLAYER_AP_HAMMER) && LinkAnimation_OnFrame(&this->skelAnime, 0.0f)) {
            this->linearVelocity = 15.0f;
            this->stateFlags2 &= ~PLAYER_STATE2_ENABLE_FORWARD_SLIDE_FROM_ATTACK;
        }

        if (this->linearVelocity > 12.0f) {
            Player_SetupSpawnDustAtFeet(globalCtx, this);
        }

        Math_StepToF(&this->linearVelocity, 0.0f, 5.0f);
        Player_SetupDeactivateComboTimer(this);

        if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
            if (!Player_SetupStartMeleeWeaponAttack(this, globalCtx)) {
                u8 sp43 = this->skelAnime.moveFlags;
                LinkAnimationHeader* sp3C;

                if (Player_IsUnfriendlyZTargeting(this)) {
                    sp3C = sp44->unk_08;
                } else {
                    sp3C = sp44->unk_04;
                }

                Player_InactivateMeleeWeapon(this);
                this->skelAnime.moveFlags = 0;

                if ((sp3C == &gPlayerAnim_002908) && (this->modelAnimType != PLAYER_ANIMTYPE_HOLDING_TWO_HAND_WEAPON)) {
                    sp3C = &gPlayerAnim_002AC8;
                }

                Player_SetupReturnToStandStillSetAnim(this, sp3C, globalCtx);

                this->skelAnime.moveFlags = sp43;
                this->stateFlags3 |= PLAYER_STATE3_ENDING_MELEE_ATTACK;
            }
        } else if (this->heldItemActionParam == PLAYER_AP_HAMMER) {
            if ((this->swordAnimation == 0x16) || (this->swordAnimation == 0x13)) {
                static Vec3f zeroVec = { 0.0f, 0.0f, 0.0f };
                Vec3f shockwavePos;
                f32 sp2C;

                shockwavePos.y = Player_RaycastFloorWithOffset2(globalCtx, this, &D_80854A40, &shockwavePos);
                sp2C = this->actor.world.pos.y - shockwavePos.y;

                Math_ScaledStepToS(&this->actor.focus.rot.x, Math_Atan2S(45.0f, sp2C), 800);
                Player_UpdateLookAngles(this, 1);

                if ((((this->swordAnimation == 0x16) && LinkAnimation_OnFrame(&this->skelAnime, 7.0f)) ||
                     ((this->swordAnimation == 0x13) && LinkAnimation_OnFrame(&this->skelAnime, 2.0f))) &&
                    (sp2C > -40.0f) && (sp2C < 40.0f)) {
                    func_80842A28(globalCtx, this);
                    EffectSsBlast_SpawnWhiteShockwave(globalCtx, &shockwavePos, &zeroVec, &zeroVec);
                }
            }
        }
    }
}

void Player_MeleeWeaponRebound(Player* this, GlobalContext* globalCtx) {
    LinkAnimation_Update(globalCtx, &this->skelAnime);
    Player_StepLinearVelocityToZero(this);

    if (this->skelAnime.curFrame >= 6.0f) {
        Player_ReturnToStandStill(this, globalCtx);
    }
}

void Player_ChooseFaroresWindOption(Player* this, GlobalContext* globalCtx) {
    this->stateFlags2 |= PLAYER_STATE2_DISABLE_MOVE_ROTATION_WHILE_Z_TARGETING;

    LinkAnimation_Update(globalCtx, &this->skelAnime);
    Player_SetupCurrentUpperAction(this, globalCtx);

    if (this->genericTimer == 0) {
        Message_StartTextbox(globalCtx, 0x3B, &this->actor);
        this->genericTimer = 1;
        return;
    }

    if (Message_GetState(&globalCtx->msgCtx) == TEXT_STATE_CLOSING) {
        s32 respawnData = gSaveContext.respawn[RESPAWN_MODE_TOP].data;

        if (globalCtx->msgCtx.choiceIndex == 0) {
            gSaveContext.respawnFlag = 3;
            globalCtx->sceneLoadFlag = 0x14;
            globalCtx->nextEntranceIndex = gSaveContext.respawn[RESPAWN_MODE_TOP].entranceIndex;
            globalCtx->fadeTransition = 5;
            func_80088AF0(globalCtx);
            return;
        }

        if (globalCtx->msgCtx.choiceIndex == 1) {
            gSaveContext.respawn[RESPAWN_MODE_TOP].data = -respawnData;
            gSaveContext.fw.set = 0;
            func_80078914(&gSaveContext.respawn[RESPAWN_MODE_TOP].pos, NA_SE_PL_MAGIC_WIND_VANISH);
        }

        Player_SetupStandingStillMorph(this, globalCtx);
        func_8005B1A4(Gameplay_GetCamera(globalCtx, 0));
    }
}

void Player_SpawnFromFaroresWind(Player* this, GlobalContext* globalCtx) {
    s32 respawnData = gSaveContext.respawn[RESPAWN_MODE_TOP].data;

    if (this->genericTimer > 20) {
        this->actor.draw = Player_Draw;
        this->actor.world.pos.y += 60.0f;
        Player_SetupFallFromLedge(this, globalCtx);
        return;
    }

    if (this->genericTimer++ == 20) {
        gSaveContext.respawn[RESPAWN_MODE_TOP].data = respawnData + 1;
        func_80078914(&gSaveContext.respawn[RESPAWN_MODE_TOP].pos, NA_SE_PL_MAGIC_WIND_WARP);
    }
}

static LinkAnimationHeader* D_80854A58[] = {
    &gPlayerAnim_002CF8,
    &gPlayerAnim_002CE0,
    &gPlayerAnim_002D10,
};

static LinkAnimationHeader* D_80854A64[] = {
    &gPlayerAnim_002D00,
    &gPlayerAnim_002CE8,
    &gPlayerAnim_002D18,
};

static LinkAnimationHeader* D_80854A70[] = {
    &gPlayerAnim_002D08,
    &gPlayerAnim_002CF0,
    &gPlayerAnim_002D20,
};

static u8 D_80854A7C[] = { 70, 10, 10 };

static PlayerAnimSfxEntry D_80854A80[] = {
    { NA_SE_PL_SKIP, 0x814 },
    { NA_SE_VO_LI_SWORD_N, 0x2014 },
    { 0, -0x301A },
};

static PlayerAnimSfxEntry D_80854A8C[][2] = {
    {
        { 0, 0x4014 },
        { NA_SE_VO_LI_MAGIC_FROL, -0x201E },
    },
    {
        { 0, 0x4014 },
        { NA_SE_VO_LI_MAGIC_NALE, -0x202C },
    },
    {
        { NA_SE_VO_LI_MAGIC_ATTACK, 0x2014 },
        { NA_SE_IT_SWORD_SWING_HARD, -0x814 },
    },
};

void Player_UpdateMagicSpell(Player* this, GlobalContext* globalCtx) {
    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        if (this->genericVar < 0) {
            if ((this->itemActionParam == PLAYER_AP_NAYRUS_LOVE) || (gSaveContext.unk_13F0 == 0)) {
                Player_ReturnToStandStill(this, globalCtx);
                func_8005B1A4(Gameplay_GetCamera(globalCtx, 0));
            }
        } else {
            if (this->genericTimer == 0) {
                LinkAnimation_PlayOnceSetSpeed(globalCtx, &this->skelAnime, D_80854A58[this->genericVar], 0.83f);

                if (func_80846A00(globalCtx, this, this->genericVar) != NULL) {
                    this->stateFlags1 |= PLAYER_STATE1_SKIP_OTHER_ACTORS_UPDATE | PLAYER_STATE1_IN_CUTSCENE;
                    if ((this->genericVar != 0) || (gSaveContext.respawn[RESPAWN_MODE_TOP].data <= 0)) {
                        gSaveContext.unk_13F0 = 1;
                    }
                } else {
                    func_800876C8(globalCtx);
                }
            } else {
                LinkAnimation_PlayLoopSetSpeed(globalCtx, &this->skelAnime, D_80854A64[this->genericVar], 0.83f);

                if (this->genericVar == 0) {
                    this->genericTimer = -10;
                }
            }

            this->genericTimer++;
        }
    } else {
        if (this->genericTimer < 0) {
            this->genericTimer++;

            if (this->genericTimer == 0) {
                gSaveContext.respawn[RESPAWN_MODE_TOP].data = 1;
                Gameplay_SetupRespawnPoint(globalCtx, RESPAWN_MODE_TOP, 0x6FF);
                gSaveContext.fw.set = 1;
                gSaveContext.fw.pos.x = gSaveContext.respawn[RESPAWN_MODE_DOWN].pos.x;
                gSaveContext.fw.pos.y = gSaveContext.respawn[RESPAWN_MODE_DOWN].pos.y;
                gSaveContext.fw.pos.z = gSaveContext.respawn[RESPAWN_MODE_DOWN].pos.z;
                gSaveContext.fw.yaw = gSaveContext.respawn[RESPAWN_MODE_DOWN].yaw;
                gSaveContext.fw.playerParams = 0x6FF;
                gSaveContext.fw.entranceIndex = gSaveContext.respawn[RESPAWN_MODE_DOWN].entranceIndex;
                gSaveContext.fw.roomIndex = gSaveContext.respawn[RESPAWN_MODE_DOWN].roomIndex;
                gSaveContext.fw.tempSwchFlags = gSaveContext.respawn[RESPAWN_MODE_DOWN].tempSwchFlags;
                gSaveContext.fw.tempCollectFlags = gSaveContext.respawn[RESPAWN_MODE_DOWN].tempCollectFlags;
                this->genericTimer = 2;
            }
        } else if (this->genericVar >= 0) {
            if (this->genericTimer == 0) {
                Player_PlayAnimSfx(this, D_80854A80);
            } else if (this->genericTimer == 1) {
                Player_PlayAnimSfx(this, D_80854A8C[this->genericVar]);
                if ((this->genericVar == 2) && LinkAnimation_OnFrame(&this->skelAnime, 30.0f)) {
                    this->stateFlags1 &= ~(PLAYER_STATE1_SKIP_OTHER_ACTORS_UPDATE | PLAYER_STATE1_IN_CUTSCENE);
                }
            } else if (D_80854A7C[this->genericVar] < this->genericTimer++) {
                LinkAnimation_PlayOnceSetSpeed(globalCtx, &this->skelAnime, D_80854A70[this->genericVar], 0.83f);
                this->currentYaw = this->actor.shape.rot.y;
                this->genericVar = -1;
            }
        }
    }

    Player_StepLinearVelocityToZero(this);
}

void Player_MoveAlongHookshotPath(Player* this, GlobalContext* globalCtx) {
    f32 temp;

    this->stateFlags2 |= PLAYER_STATE2_DISABLE_MOVE_ROTATION_WHILE_Z_TARGETING;

    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        Player_PlayAnimLoop(globalCtx, this, &gPlayerAnim_002C98);
    }

    Math_Vec3f_Sum(&this->actor.world.pos, &this->actor.velocity, &this->actor.world.pos);

    if (Player_EndHookshotMove(this)) {
        Math_Vec3f_Copy(&this->actor.prevPos, &this->actor.world.pos);
        func_80847BA0(globalCtx, this);

        temp = this->actor.world.pos.y - this->actor.floorHeight;
        if (temp > 20.0f) {
            temp = 20.0f;
        }

        this->actor.world.rot.x = this->actor.shape.rot.x = 0;
        this->actor.world.pos.y -= temp;
        this->linearVelocity = 1.0f;
        this->actor.velocity.y = 0.0f;
        Player_SetupFallFromLedge(this, globalCtx);
        this->stateFlags2 &= ~PLAYER_STATE2_DIVING;
        this->actor.bgCheckFlags |= 1;
        this->stateFlags1 |= PLAYER_STATE1_END_HOOKSHOT_MOVE;
        return;
    }

    if ((this->skelAnime.animation != &gPlayerAnim_002C90) || (4.0f <= this->skelAnime.curFrame)) {
        this->actor.gravity = 0.0f;
        Math_ScaledStepToS(&this->actor.shape.rot.x, this->actor.world.rot.x, 0x800);
        Player_RequestRumble(this, 100, 2, 100, 0);
    }
}

void Player_CastFishingRod(Player* this, GlobalContext* globalCtx) {
    if ((this->genericTimer != 0) && ((this->unk_858 != 0.0f) || (this->unk_85C != 0.0f))) {
        f32 updateScale = R_UPDATE_RATE * 0.5f;

        this->skelAnime.curFrame += this->skelAnime.playSpeed * updateScale;
        if (this->skelAnime.curFrame >= this->skelAnime.animLength) {
            this->skelAnime.curFrame -= this->skelAnime.animLength;
        }

        LinkAnimation_BlendToJoint(globalCtx, &this->skelAnime, &gPlayerAnim_002C38, this->skelAnime.curFrame,
                                   (this->unk_858 < 0.0f) ? &gPlayerAnim_002C18 : &gPlayerAnim_002C20, 5.0f,
                                   fabsf(this->unk_858), this->blendTable);
        LinkAnimation_BlendToMorph(globalCtx, &this->skelAnime, &gPlayerAnim_002C38, this->skelAnime.curFrame,
                                   (this->unk_85C < 0.0f) ? &gPlayerAnim_002C28 : &gPlayerAnim_002C10, 5.0f,
                                   fabsf(this->unk_85C), D_80858AD8);
        LinkAnimation_InterpJointMorph(globalCtx, &this->skelAnime, 0.5f);
    } else if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        this->fpsItemType = 2;
        Player_PlayAnimLoop(globalCtx, this, &gPlayerAnim_002C38);
        this->genericTimer = 1;
    }

    Player_StepLinearVelocityToZero(this);

    if (this->fpsItemType == 0) {
        Player_SetupStandingStillMorph(this, globalCtx);
    } else if (this->fpsItemType == 3) {
        Player_SetActionFunc(globalCtx, this, Player_ReleaseCaughtFish, 0);
        Player_ChangeAnimMorphToLastFrame(globalCtx, this, &gPlayerAnim_002C00);
    }
}

void Player_ReleaseCaughtFish(Player* this, GlobalContext* globalCtx) {
    if (LinkAnimation_Update(globalCtx, &this->skelAnime) && (this->fpsItemType == 0)) {
        Player_SetupReturnToStandStillSetAnim(this, &gPlayerAnim_002C08, globalCtx);
    }
}

static void (*D_80854AA4[])(GlobalContext*, Player*, void*) = {
    NULL,
    Player_AnimPlaybackType0,
    Player_AnimPlaybackType1,
    Player_AnimPlaybackType2,
    Player_AnimPlaybackType3,
    Player_AnimPlaybackType4,
    Player_AnimPlaybackType5,
    Player_AnimPlaybackType6,
    Player_AnimPlaybackType7,
    Player_AnimPlaybackType8,
    Player_AnimPlaybackType9,
    Player_AnimPlaybackType10,
    Player_AnimPlaybackType11,
    Player_AnimPlaybackType12,
    Player_AnimPlaybackType13,
    Player_AnimPlaybackType14,
    Player_AnimPlaybackType15,
    Player_AnimPlaybackType16,
    Player_AnimPlaybackType17,
};

static PlayerAnimSfxEntry D_80854AF0[] = {
    { 0, 0x2822 },
    { NA_SE_PL_CALM_HIT, 0x82D },
    { NA_SE_PL_CALM_HIT, 0x833 },
    { NA_SE_PL_CALM_HIT, -0x840 },
};

static PlayerAnimSfxEntry D_80854B00[] = {
    { NA_SE_VO_LI_SURPRISE, 0x2003 }, { 0, 0x300F }, { 0, 0x3018 }, { 0, 0x301E }, { NA_SE_VO_LI_FALL_L, -0x201F },
};

static PlayerAnimSfxEntry D_80854B14[] = {
    { 0, -0x300A },
};

static struct_80854B18 D_80854B18[] = {
    { 0, NULL },
    { -1, Player_CutsceneSetupIdle },
    { 2, &gPlayerAnim_002790 },
    { 0, NULL },
    { 0, NULL },
    { 3, &gPlayerAnim_002740 },
    { 0, NULL },
    { 0, NULL },
    { -1, Player_CutsceneSetupIdle },
    { 2, &gPlayerAnim_002778 },
    { -1, Player_CutsceneSetupEnterWarp },
    { 3, &gPlayerAnim_002860 },
    { -1, Player_CutsceneSetupFightStance },
    { 7, &gPlayerAnim_002348 },
    { 5, &gPlayerAnim_002350 },
    { 5, &gPlayerAnim_002358 },
    { 5, &gPlayerAnim_0023B0 },
    { 7, &gPlayerAnim_0023B8 },
    { -1, Player_CutsceneSetupSwordPedestal },
    { 2, &gPlayerAnim_002728 },
    { 2, &gPlayerAnim_002738 },
    { 0, NULL },
    { -1, Player_CutsceneSetupWarpToSages },
    { 3, &gPlayerAnim_0027A8 },
    { 9, &gPlayerAnim_002DB0 },
    { 2, &gPlayerAnim_002DC0 },
    { -1, Player_CutsceneSetupStartPlayOcarina },
    { 2, &gPlayerAnim_003098 },
    { 3, &gPlayerAnim_002780 },
    { -1, Player_CutsceneSetupIdle },
    { 2, &gPlayerAnim_003088 },
    { 0, NULL },
    { 0, NULL },
    { 5, &gPlayerAnim_002320 },
    { -1, Player_CutsceneSetupSwimIdle },
    { -1, Player_CutsceneSetupGetItemInWater },
    { 5, &gPlayerAnim_002328 },
    { 16, &gPlayerAnim_002F90 },
    { -1, Player_CutsceneSetupSleepingRestless },
    { -1, Player_CutsceneSetupSleeping },
    { 6, &gPlayerAnim_002410 },
    { 6, &gPlayerAnim_002418 },
    { -1, Player_CutsceneSetupBlownBackward },
    { 5, &gPlayerAnim_002390 },
    { -1, Player_CutsceneSetupIdle3 },
    { -1, Player_CutsceneSetupStop },
    { -1, Player_CutsceneSetDraw },
    { 5, &gPlayerAnim_0023A0 },
    { 5, &gPlayerAnim_002368 },
    { -1, Player_CutsceneSetupIdle },
    { 5, &gPlayerAnim_002370 },
    { 5, &gPlayerAnim_0027B0 },
    { 5, &gPlayerAnim_0027B8 },
    { 5, &gPlayerAnim_0027C0 },
    { 3, &gPlayerAnim_002768 },
    { 3, &gPlayerAnim_0027D8 },
    { 4, &gPlayerAnim_0027E0 },
    { 3, &gPlayerAnim_002380 },
    { 3, &gPlayerAnim_002828 },
    { 6, &gPlayerAnim_002470 },
    { 6, &gPlayerAnim_0032A8 },
    { 14, &gPlayerAnim_0032A0 },
    { 3, &gPlayerAnim_0032A0 },
    { 5, &gPlayerAnim_002AE8 },
    { 16, &gPlayerAnim_002450 },
    { 15, &gPlayerAnim_002460 },
    { 15, &gPlayerAnim_002458 },
    { 3, &gPlayerAnim_002440 },
    { 3, &gPlayerAnim_002438 },
    { 3, &gPlayerAnim_002C88 },
    { 6, &gPlayerAnim_003450 },
    { 6, &gPlayerAnim_003448 },
    { 6, &gPlayerAnim_003460 },
    { 6, &gPlayerAnim_003440 },
    { 3, &gPlayerAnim_002798 },
    { 3, &gPlayerAnim_002818 },
    { 4, &gPlayerAnim_002848 },
    { 3, &gPlayerAnim_002850 },
    { 3, &gPlayerAnim_0034E0 },
    { 3, &gPlayerAnim_0034D8 },
    { 6, &gPlayerAnim_0034C8 },
    { 3, &gPlayerAnim_003470 },
    { 3, &gPlayerAnim_003478 },
    { 3, &gPlayerAnim_0034C0 },
    { 3, &gPlayerAnim_003480 },
    { 3, &gPlayerAnim_003490 },
    { 3, &gPlayerAnim_003488 },
    { 3, &gPlayerAnim_003498 },
    { 3, &gPlayerAnim_0034B0 },
    { -1, Player_CutsceneSetupSpinAttackIdle },
    { 3, &gPlayerAnim_003420 },
    { -1, Player_SetupDoNothing4 },
    { -1, Player_CutsceneSetupKnockedToGroundDamaged },
    { 3, &gPlayerAnim_003250 },
    { -1, Player_CutsceneSetupGetSwordBack },
    { 3, &gPlayerAnim_002810 },
    { 3, &gPlayerAnim_002838 },
    { 3, &gPlayerAnim_002CD0 },
    { 3, &gPlayerAnim_002CD8 },
    { 3, &gPlayerAnim_002868 },
    { 3, &gPlayerAnim_0027E8 },
    { 3, &gPlayerAnim_0027F8 },
    { 3, &gPlayerAnim_002800 },
};

static struct_80854B18 D_80854E50[] = {
    { 0, NULL },
    { -1, Player_CutsceneIdle },
    { -1, Player_CutsceneTurnAroundSurprisedShort },
    { -1, Player_CutsceneUnk3Update },
    { -1, Player_CutsceneUnk4Update },
    { 11, NULL },
    { -1, Player_CutsceneUnk6Update },
    { -1, Player_CutsceneEnd },
    { -1, Player_CutsceneWait },
    { -1, Player_CutsceneTurnAroundSurprisedLong },
    { -1, Player_CutsceneEnterWarp },
    { -1, Player_CutsceneRaisedByWarp },
    { -1, Player_CutsceneFightStance },
    { 11, NULL },
    { 11, NULL },
    { 11, NULL },
    { 18, D_80854AF0 },
    { 11, NULL },
    { -1, Player_CutsceneSwordPedestal },
    { 12, &gPlayerAnim_002730 },
    { 11, NULL },
    { 0, NULL },
    { -1, Player_CutsceneWarpToSages },
    { 11, NULL },
    { -1, Player_CutsceneKnockedToGround },
    { 11, NULL },
    { 17, &gPlayerAnim_0030A8 },
    { 11, NULL },
    { 11, NULL },
    { 11, NULL },
    { -1, Player_CutsceneDrawAndBrandishSword },
    { -1, Player_CutsceneCloseEyes },
    { -1, Player_CutsceneOpenEyes },
    { 18, D_80854B00 },
    { -1, Player_CutsceneSurfaceFromDive },
    { 11, NULL },
    { 11, NULL },
    { 11, NULL },
    { 11, NULL },
    { -1, Player_CutsceneSleeping },
    { -1, Player_CutsceneAwaken },
    { -1, Player_CutsceneGetOffBed },
    { -1, Player_CutsceneBlownBackward },
    { 13, &gPlayerAnim_002398 },
    { -1, Player_CutsceneIdle3 },
    { 0, NULL },
    { 0, NULL },
    { 11, NULL },
    { -1, Player_CutsceneStepBackCautiously },
    { -1, Player_CutsceneWait },
    { -1, Player_CutsceneDrawSwordChild },
    { 13, &gPlayerAnim_0027D0 },
    { -1, Player_CutsceneDesperateLookAtZeldasCrystal },
    { 13, &gPlayerAnim_0027C8 },
    { -1, Player_CutsceneTurnAroundSlowly },
    { 11, NULL },
    { 11, NULL },
    { 12, &gPlayerAnim_002388 },
    { -1, Player_CutsceneInspectGroundCarefully },
    { 11, NULL },
    { 18, D_80854B14 },
    { 11, NULL },
    { 11, NULL },
    { 11, NULL },
    { 11, NULL },
    { -1, Player_CutsceneStartPassOcarina },
    { 17, &gPlayerAnim_002450 },
    { 12, &gPlayerAnim_002448 },
    { 12, &gPlayerAnim_002450 },
    { 11, NULL },
    { -1, Player_LearnOcarinaSong },
    { 17, &gPlayerAnim_003468 },
    { -1, Player_LearnOcarinaSong },
    { 17, &gPlayerAnim_003468 },
    { 12, &gPlayerAnim_0027A0 },
    { 12, &gPlayerAnim_002820 },
    { 11, NULL },
    { 12, &gPlayerAnim_002858 },
    { 12, &gPlayerAnim_0034D0 },
    { 13, &gPlayerAnim_0034F0 },
    { 12, &gPlayerAnim_0034E8 },
    { 12, &gPlayerAnim_0034A8 },
    { 11, NULL },
    { 11, NULL },
    { 11, NULL },
    { 11, NULL },
    { -1, Player_CutsceneSwordKnockedFromHand },
    { 11, NULL },
    { 12, &gPlayerAnim_0034A0 },
    { -1, Player_CutsceneSpinAttackIdle },
    { -1, Player_CutsceneInspectWeapon },
    { -1, Player_DoNothing5 },
    { -1, Player_CutsceneKnockedToGroundDamaged },
    { 11, NULL },
    { 11, NULL },
    { 11, NULL },
    { -1, Player_CutsceneGetSwordBack },
    { -1, Player_CutsceneGanonKillCombo },
    { -1, Player_CutsceneGanonKillCombo },
    { 12, &gPlayerAnim_002870 },
    { 12, &gPlayerAnim_0027F0 },
    { 12, &gPlayerAnim_002808 },
    { 12, &gPlayerAnim_002450 },
};

void func_80850ED8(GlobalContext* globalCtx, Player* this, LinkAnimationHeader* anim) {
    Player_ClearRootLimbPosY(this);
    Player_ChangeAnimMorphToLastFrame(globalCtx, this, anim);
    Player_StopMovement(this);
}

void func_80850F1C(GlobalContext* globalCtx, Player* this, LinkAnimationHeader* anim) {
    Player_ClearRootLimbPosY(this);
    LinkAnimation_Change(globalCtx, &this->skelAnime, anim, (2.0f / 3.0f), 0.0f, Animation_GetLastFrame(anim),
                         ANIMMODE_ONCE, -8.0f);
    Player_StopMovement(this);
}

void func_80850F9C(GlobalContext* globalCtx, Player* this, LinkAnimationHeader* anim) {
    Player_ClearRootLimbPosY(this);
    LinkAnimation_Change(globalCtx, &this->skelAnime, anim, (2.0f / 3.0f), 0.0f, 0.0f, ANIMMODE_LOOP, -8.0f);
    Player_StopMovement(this);
}

void Player_AnimPlaybackType0(GlobalContext* globalCtx, Player* this, void* anim) {
    Player_StopMovement(this);
}

void Player_AnimPlaybackType1(GlobalContext* globalCtx, Player* this, void* anim) {
    func_80850ED8(globalCtx, this, anim);
}

void Player_AnimPlaybackType13(GlobalContext* globalCtx, Player* this, void* anim) {
    Player_ClearRootLimbPosY(this);
    Player_ChangeAnimOnce(globalCtx, this, anim);
    Player_StopMovement(this);
}

void Player_AnimPlaybackType2(GlobalContext* globalCtx, Player* this, void* anim) {
    func_80850F1C(globalCtx, this, anim);
}

void Player_AnimPlaybackType3(GlobalContext* globalCtx, Player* this, void* anim) {
    func_80850F9C(globalCtx, this, anim);
}

void Player_AnimPlaybackType4(GlobalContext* globalCtx, Player* this, void* anim) {
    Player_PlayAnimOnceWithMovementPresetFlagsSlowed(globalCtx, this, anim);
}

void Player_AnimPlaybackType5(GlobalContext* globalCtx, Player* this, void* anim) {
    Player_PlayAnimOnceWithMovement(globalCtx, this, anim, 0x9C);
}

void Player_AnimPlaybackType6(GlobalContext* globalCtx, Player* this, void* anim) {
    Player_PlayAnimLoopWithMovementPresetFlagsSlowed(globalCtx, this, anim);
}

void Player_AnimPlaybackType7(GlobalContext* globalCtx, Player* this, void* anim) {
    Player_PlayAnimLoopWithMovement(globalCtx, this, anim, 0x9C);
}

void Player_AnimPlaybackType8(GlobalContext* globalCtx, Player* this, void* anim) {
    Player_PlayAnimOnce(globalCtx, this, anim);
}

void Player_AnimPlaybackType9(GlobalContext* globalCtx, Player* this, void* anim) {
    Player_PlayAnimLoop(globalCtx, this, anim);
}

void Player_AnimPlaybackType14(GlobalContext* globalCtx, Player* this, void* anim) {
    Player_PlayAnimOnceSlowed(globalCtx, this, anim);
}

void Player_AnimPlaybackType15(GlobalContext* globalCtx, Player* this, void* anim) {
    Player_PlayAnimLoopSlowed(globalCtx, this, anim);
}

void Player_AnimPlaybackType10(GlobalContext* globalCtx, Player* this, void* anim) {
    LinkAnimation_Update(globalCtx, &this->skelAnime);
}

void Player_AnimPlaybackType11(GlobalContext* globalCtx, Player* this, void* anim) {
    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        func_80850F9C(globalCtx, this, anim);
        this->genericTimer = 1;
    }
}

void Player_AnimPlaybackType16(GlobalContext* globalCtx, Player* this, void* anim) {
    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        Player_EndAnimMovement(this);
        Player_PlayAnimLoopSlowed(globalCtx, this, anim);
    }
}

void Player_AnimPlaybackType12(GlobalContext* globalCtx, Player* this, void* anim) {
    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        Player_PlayAnimLoopWithMovementPresetFlagsSlowed(globalCtx, this, anim);
        this->genericTimer = 1;
    }
}

void Player_AnimPlaybackType17(GlobalContext* globalCtx, Player* this, void* arg2) {
    LinkAnimation_Update(globalCtx, &this->skelAnime);
    Player_PlayAnimSfx(this, arg2);
}

void func_80851314(Player* this) {
    if ((this->csTargetActor == NULL) || (this->csTargetActor->update == NULL)) {
        this->csTargetActor = NULL;
    }

    this->targetActor = this->csTargetActor;

    if (this->targetActor != NULL) {
        this->actor.shape.rot.y = Player_LookAtTargetActor(this, 0);
    }
}

void Player_CutsceneSetupSwimIdle(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    this->stateFlags1 |= PLAYER_STATE1_SWIMMING;
    this->stateFlags2 |= PLAYER_STATE2_DIVING;
    this->stateFlags1 &= ~(PLAYER_STATE1_JUMPING | PLAYER_STATE1_FREEFALLING);

    Player_PlayAnimLoop(globalCtx, this, &gPlayerAnim_0032F0);
}

void Player_CutsceneSurfaceFromDive(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    this->actor.gravity = 0.0f;

    if (this->genericVar == 0) {
        if (Player_SetupDive(globalCtx, this, NULL)) {
            this->genericVar = 1;
        } else {
            func_8084B158(globalCtx, this, NULL, fabsf(this->actor.velocity.y));
            Math_ScaledStepToS(&this->shapePitchOffset, -10000, 800);
            Player_UpdateSwimMovement(this, &this->actor.velocity.y, 4.0f, this->currentYaw);
        }
        return;
    }

    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        if (this->genericVar == 1) {
            Player_ChangeAnimLongMorphLoop(globalCtx, this, &gPlayerAnim_003328);
        } else {
            Player_PlayAnimLoop(globalCtx, this, &gPlayerAnim_003328);
        }
    }

    func_8084B000(this);
    Player_UpdateSwimMovement(this, &this->linearVelocity, 0.0f, this->actor.shape.rot.y);
}

void Player_CutsceneIdle(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    func_80851314(this);

    if (Player_IsSwimming(this)) {
        Player_CutsceneSurfaceFromDive(globalCtx, this, 0);
        return;
    }

    LinkAnimation_Update(globalCtx, &this->skelAnime);

    if (Player_IsShootingHookshot(this) || (this->stateFlags1 & PLAYER_STATE1_HOLDING_ACTOR)) {
        Player_SetupCurrentUpperAction(this, globalCtx);
        return;
    }

    if ((this->interactRangeActor != NULL) && (this->interactRangeActor->textId == 0xFFFF)) {
        Player_SetupGetItemOrHoldBehavior(this, globalCtx);
    }
}

void Player_CutsceneTurnAroundSurprisedShort(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    LinkAnimation_Update(globalCtx, &this->skelAnime);
}

void Player_CutsceneSetupIdle(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    LinkAnimationHeader* anim;

    if (Player_IsSwimming(this)) {
        Player_CutsceneSetupSwimIdle(globalCtx, this, 0);
        return;
    }

    anim = GET_PLAYER_ANIM(PLAYER_ANIMGROUP_RELAX, this->modelAnimType);

    if ((this->csAction == 6) || (this->csAction == 0x2E)) {
        Player_PlayAnimOnce(globalCtx, this, anim);
    } else {
        Player_ClearRootLimbPosY(this);
        LinkAnimation_Change(globalCtx, &this->skelAnime, anim, (2.0f / 3.0f), 0.0f, Animation_GetLastFrame(anim),
                             ANIMMODE_LOOP, -4.0f);
    }

    Player_StopMovement(this);
}

void Player_CutsceneWait(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    if (func_8084B3CC(globalCtx, this) == 0) {
        if ((this->csMode == 0x31) && (globalCtx->csCtx.state == CS_STATE_IDLE)) {
            func_8002DF54(globalCtx, NULL, 7);
            return;
        }

        if (Player_IsSwimming(this) != 0) {
            Player_CutsceneSurfaceFromDive(globalCtx, this, 0);
            return;
        }

        LinkAnimation_Update(globalCtx, &this->skelAnime);

        if (Player_IsShootingHookshot(this) || (this->stateFlags1 & PLAYER_STATE1_HOLDING_ACTOR)) {
            Player_SetupCurrentUpperAction(this, globalCtx);
        }
    }
}

static PlayerAnimSfxEntry D_80855188[] = {
    { 0, 0x302A },
    { 0, -0x3030 },
};

void Player_CutsceneTurnAroundSurprisedLong(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    LinkAnimation_Update(globalCtx, &this->skelAnime);
    Player_PlayAnimSfx(this, D_80855188);
}

void Player_CutsceneSetupEnterWarp(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    this->stateFlags1 &= ~PLAYER_STATE1_AWAITING_THROWN_BOOMERANG;

    this->currentYaw = this->actor.shape.rot.y = this->actor.world.rot.y =
        Math_Vec3f_Yaw(&this->actor.world.pos, &this->csStartPos);

    if (this->linearVelocity <= 0.0f) {
        this->linearVelocity = 0.1f;
    } else if (this->linearVelocity > 2.5f) {
        this->linearVelocity = 2.5f;
    }
}

void Player_CutsceneEnterWarp(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    f32 sp1C = 2.5f;

    func_80845BA0(globalCtx, this, &sp1C, 10);

    if (globalCtx->sceneNum == SCENE_BDAN_BOSS) {
        if (this->genericTimer == 0) {
            if (Message_GetState(&globalCtx->msgCtx) == TEXT_STATE_NONE) {
                return;
            }
        } else {
            if (Message_GetState(&globalCtx->msgCtx) != TEXT_STATE_NONE) {
                return;
            }
        }
    }

    this->genericTimer++;
    if (this->genericTimer > 20) {
        this->csMode = 0xB;
    }
}

void Player_CutsceneSetupFightStance(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    Player_SetupUnfriendlyZTarget(this, globalCtx);
}

void Player_CutsceneFightStance(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    func_80851314(this);

    if (this->genericTimer != 0) {
        if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
            Player_PlayAnimLoop(globalCtx, this, Player_GetFightingRightAnim(this));
            this->genericTimer = 0;
        }

        Player_ResetLeftRightBlendWeight(this);
    } else {
        func_808401B0(globalCtx, this);
    }
}

void Player_CutsceneUnk3Update(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    func_80845964(globalCtx, this, arg2, 0.0f, 0, 0);
}

void Player_CutsceneUnk4Update(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    func_80845964(globalCtx, this, arg2, 0.0f, 0, 1);
}

// unused
static LinkAnimationHeader* D_80855190[] = {
    &gPlayerAnim_002720,
    &gPlayerAnim_002360,
};

static Vec3f D_80855198 = { -1.0f, 70.0f, 20.0f };

void Player_CutsceneSetupSwordPedestal(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    Math_Vec3f_Copy(&this->actor.world.pos, &D_80855198);
    this->actor.shape.rot.y = -0x8000;
    Player_PlayAnimOnceSlowed(globalCtx, this, this->ageProperties->unk_9C);
    Player_SetupAnimMovement(globalCtx, this, 0x28F);
}

static struct_808551A4 D_808551A4[] = {
    { NA_SE_IT_SWORD_PUTAWAY_STN, 0 },
    { NA_SE_IT_SWORD_STICK_STN, NA_SE_VO_LI_SWORD_N },
};

static PlayerAnimSfxEntry D_808551AC[] = {
    { 0, 0x401D },
    { 0, -0x4027 },
};

void Player_CutsceneSwordPedestal(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    struct_808551A4* sp2C;
    Gfx** dLists;

    LinkAnimation_Update(globalCtx, &this->skelAnime);

    if ((LINK_IS_ADULT && LinkAnimation_OnFrame(&this->skelAnime, 70.0f)) ||
        (!LINK_IS_ADULT && LinkAnimation_OnFrame(&this->skelAnime, 87.0f))) {
        sp2C = &D_808551A4[gSaveContext.linkAge];
        this->interactRangeActor->parent = &this->actor;

        if (!LINK_IS_ADULT) {
            dLists = D_80125DE8;
        } else {
            dLists = D_80125E18;
        }
        this->leftHandDLists = &dLists[gSaveContext.linkAge];

        func_8002F7DC(&this->actor, sp2C->unk_00);
        if (!LINK_IS_ADULT) {
            Player_PlayVoiceSfxForAge(this, sp2C->unk_02);
        }
    } else if (LINK_IS_ADULT) {
        if (LinkAnimation_OnFrame(&this->skelAnime, 66.0f)) {
            Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_SWORD_L);
        }
    } else {
        Player_PlayAnimSfx(this, D_808551AC);
    }
}

void Player_CutsceneSetupWarpToSages(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    LinkAnimation_Change(globalCtx, &this->skelAnime, &gPlayerAnim_002860, -(2.0f / 3.0f), 12.0f, 12.0f, ANIMMODE_ONCE,
                         0.0f);
}

static PlayerAnimSfxEntry D_808551B4[] = {
    { 0, -0x281E },
};

void Player_CutsceneWarpToSages(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    LinkAnimation_Update(globalCtx, &this->skelAnime);

    this->genericTimer++;

    if (this->genericTimer >= 180) {
        if (this->genericTimer == 180) {
            LinkAnimation_Change(globalCtx, &this->skelAnime, &gPlayerAnim_003298, (2.0f / 3.0f), 10.0f,
                                 Animation_GetLastFrame(&gPlayerAnim_003298), ANIMMODE_ONCE, -8.0f);
        }
        Player_PlayAnimSfx(this, D_808551B4);
    }
}

void Player_CutsceneKnockedToGround(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    if (LinkAnimation_Update(globalCtx, &this->skelAnime) && (this->genericTimer == 0) &&
        (this->actor.bgCheckFlags & 1)) {
        Player_PlayAnimOnce(globalCtx, this, &gPlayerAnim_002DB8);
        this->genericTimer = 1;
    }

    if (this->genericTimer != 0) {
        Player_StepLinearVelocityToZero(this);
    }
}

void Player_CutsceneSetupStartPlayOcarina(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    func_80850F1C(globalCtx, this, &gPlayerAnim_0030A0);
    func_8084B498(this);
    Player_SetModels(this, Player_ActionToModelGroup(this, this->itemActionParam));
}

static PlayerAnimSfxEntry D_808551B8[] = {
    { NA_SE_IT_SWORD_PICKOUT, -0x80C },
};

void Player_CutsceneDrawAndBrandishSword(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    LinkAnimation_Update(globalCtx, &this->skelAnime);

    if (LinkAnimation_OnFrame(&this->skelAnime, 6.0f)) {
        func_80846720(globalCtx, this, 0);
    } else {
        Player_PlayAnimSfx(this, D_808551B8);
    }
}

void Player_CutsceneCloseEyes(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    LinkAnimation_Update(globalCtx, &this->skelAnime);
    Math_StepToS(&this->actor.shape.face, 0, 1);
}

void Player_CutsceneOpenEyes(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    LinkAnimation_Update(globalCtx, &this->skelAnime);
    Math_StepToS(&this->actor.shape.face, 2, 1);
}

void Player_CutsceneSetupGetItemInWater(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    Player_PlayAnimOnceWithMovementSlowed(globalCtx, this, &gPlayerAnim_003318, 0x98);
}

void Player_CutsceneSetupSleeping(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    Player_PlayAnimOnceWithMovement(globalCtx, this, &gPlayerAnim_002408, 0x9C);
    Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_GROAN);
}

void Player_CutsceneSleeping(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        Player_PlayAnimLoopWithMovement(globalCtx, this, &gPlayerAnim_002428, 0x9C);
    }
}

void func_80851F14(GlobalContext* globalCtx, Player* this, LinkAnimationHeader* anim, PlayerAnimSfxEntry* arg3) {
    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        Player_PlayAnimLoopSlowed(globalCtx, this, anim);
        this->genericTimer = 1;
    } else if (this->genericTimer == 0) {
        Player_PlayAnimSfx(this, arg3);
    }
}

void Player_CutsceneSetupSleepingRestless(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    this->actor.shape.shadowDraw = NULL;
    Player_AnimPlaybackType7(globalCtx, this, &gPlayerAnim_002420);
}

static PlayerAnimSfxEntry D_808551BC[] = {
    { NA_SE_VO_LI_RELAX, 0x2023 },
    { NA_SE_PL_SLIPDOWN, 0x8EC },
    { NA_SE_PL_SLIPDOWN, -0x900 },
};

void Player_CutsceneAwaken(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        Player_PlayAnimLoopWithMovement(globalCtx, this, &gPlayerAnim_002430, 0x9C);
        this->genericTimer = 1;
    } else if (this->genericTimer == 0) {
        Player_PlayAnimSfx(this, D_808551BC);
        if (LinkAnimation_OnFrame(&this->skelAnime, 240.0f)) {
            this->actor.shape.shadowDraw = ActorShadow_DrawFeet;
        }
    }
}

static PlayerAnimSfxEntry D_808551C8[] = {
    { NA_SE_PL_LAND_LADDER, 0x843 },
    { 0, 0x4854 },
    { 0, 0x485A },
    { 0, -0x4860 },
};

void Player_CutsceneGetOffBed(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    LinkAnimation_Update(globalCtx, &this->skelAnime);
    Player_PlayAnimSfx(this, D_808551C8);
}

void Player_CutsceneSetupBlownBackward(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    Player_PlayAnimOnceWithMovementSlowed(globalCtx, this, &gPlayerAnim_002340, 0x9D);
    Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_FALL_L);
}

void func_808520BC(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    f32 startX = arg2->startPos.x;
    f32 startY = arg2->startPos.y;
    f32 startZ = arg2->startPos.z;
    f32 distX = (arg2->endPos.x - startX);
    f32 distY = (arg2->endPos.y - startY);
    f32 distZ = (arg2->endPos.z - startZ);
    f32 sp4 = (f32)(globalCtx->csCtx.frames - arg2->startFrame) / (f32)(arg2->endFrame - arg2->startFrame);

    this->actor.world.pos.x = distX * sp4 + startX;
    this->actor.world.pos.y = distY * sp4 + startY;
    this->actor.world.pos.z = distZ * sp4 + startZ;
}

static PlayerAnimSfxEntry D_808551D8[] = {
    { NA_SE_PL_BOUND, 0x1014 },
    { NA_SE_PL_BOUND, -0x101E },
};

void Player_CutsceneBlownBackward(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    func_808520BC(globalCtx, this, arg2);
    LinkAnimation_Update(globalCtx, &this->skelAnime);
    Player_PlayAnimSfx(this, D_808551D8);
}

void Player_CutsceneRaisedByWarp(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    if (arg2 != NULL) {
        func_808520BC(globalCtx, this, arg2);
    }
    LinkAnimation_Update(globalCtx, &this->skelAnime);
}

void Player_CutsceneSetupIdle3(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    Player_ChangeAnimMorphToLastFrame(globalCtx, this, GET_PLAYER_ANIM(PLAYER_ANIMGROUP_RELAX, this->modelAnimType));
    Player_StopMovement(this);
}

void Player_CutsceneIdle3(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    LinkAnimation_Update(globalCtx, &this->skelAnime);
}

void Player_CutsceneSetupStop(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    Player_SetupAnimMovement(globalCtx, this, 0x98);
}

void Player_CutsceneSetDraw(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    this->actor.draw = Player_Draw;
}

void Player_CutsceneDrawSwordChild(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        Player_PlayAnimLoopWithMovementPresetFlagsSlowed(globalCtx, this, &gPlayerAnim_002378);
        this->genericTimer = 1;
    } else if (this->genericTimer == 0) {
        if (LinkAnimation_OnFrame(&this->skelAnime, 10.0f)) {
            func_80846720(globalCtx, this, 1);
        }
    }
}

static PlayerAnimSfxEntry D_808551E0[] = {
    { 0, 0x300A },
    { 0, -0x3018 },
};

void Player_CutsceneTurnAroundSlowly(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    func_80851F14(globalCtx, this, &gPlayerAnim_002770, D_808551E0);
}

static PlayerAnimSfxEntry D_808551E8[] = {
    { 0, 0x400F },
    { 0, -0x4023 },
};

void Player_CutsceneInspectGroundCarefully(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    func_80851F14(globalCtx, this, &gPlayerAnim_002830, D_808551E8);
}

void Player_CutsceneStartPassOcarina(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        Player_PlayAnimLoopSlowed(globalCtx, this, &gPlayerAnim_002468);
        this->genericTimer = 1;
    }

    if ((this->genericTimer != 0) && (globalCtx->csCtx.frames >= 900)) {
        this->rightHandType = PLAYER_MODELTYPE_LH_OPEN;
    } else {
        this->rightHandType = PLAYER_MODELTYPE_RH_FF;
    }
}

void func_80852414(GlobalContext* globalCtx, Player* this, LinkAnimationHeader* anim, PlayerAnimSfxEntry* arg3) {
    Player_AnimPlaybackType12(globalCtx, this, anim);
    if (this->genericTimer == 0) {
        Player_PlayAnimSfx(this, arg3);
    }
}

static PlayerAnimSfxEntry D_808551F0[] = {
    { 0, 0x300F },
    { 0, -0x3021 },
};

void Player_CutsceneStepBackCautiously(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    func_80852414(globalCtx, this, &gPlayerAnim_002378, D_808551F0);
}

static PlayerAnimSfxEntry D_808551F8[] = {
    { NA_SE_PL_KNOCK, -0x84E },
};

void Player_CutsceneDesperateLookAtZeldasCrystal(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    func_80852414(globalCtx, this, &gPlayerAnim_0027D0, D_808551F8);
}

void Player_CutsceneSetupSpinAttackIdle(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    Player_SetupSpinAttackAnims(globalCtx, this);
}

void Player_CutsceneSpinAttackIdle(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    sControlInput->press.button |= BTN_B;

    Player_ChargeSpinAttack(this, globalCtx);
}

void Player_CutsceneInspectWeapon(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    Player_ChargeSpinAttack(this, globalCtx);
}

void Player_SetupDoNothing4(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
}

void Player_DoNothing5(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
}

void Player_CutsceneSetupKnockedToGroundDamaged(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    this->stateFlags3 |= PLAYER_STATE3_MIDAIR;
    this->linearVelocity = 2.0f;
    this->actor.velocity.y = -1.0f;

    Player_PlayAnimOnce(globalCtx, this, &gPlayerAnim_002DB0);
    Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_FALL_L);
}

static void (*D_808551FC[])(Player* this, GlobalContext* globalCtx) = {
    Player_StartKnockback,
    Player_DownFromKnockback,
    Player_GetUpFromKnockback,
};

void Player_CutsceneKnockedToGroundDamaged(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    D_808551FC[this->genericTimer](this, globalCtx);
}

void Player_CutsceneSetupGetSwordBack(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    func_80846720(globalCtx, this, 0);
    Player_PlayAnimOnceSlowed(globalCtx, this, &gPlayerAnim_002838);
}

void Player_CutsceneSwordKnockedFromHand(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    LinkAnimation_Update(globalCtx, &this->skelAnime);

    if (LinkAnimation_OnFrame(&this->skelAnime, 10.0f)) {
        this->heldItemActionParam = this->itemActionParam = PLAYER_AP_NONE;
        this->heldItemId = ITEM_NONE;
        this->modelGroup = this->nextModelGroup = Player_ActionToModelGroup(this, PLAYER_AP_NONE);
        this->leftHandDLists = D_80125E08;
        Inventory_ChangeEquipment(EQUIP_SWORD, 2);
        gSaveContext.equips.buttonItems[0] = ITEM_SWORD_MASTER;
        Inventory_DeleteEquipment(globalCtx, 0);
    }
}

static LinkAnimationHeader* D_80855208[] = {
    &gPlayerAnim_0034B8,
    &gPlayerAnim_003458,
};

static Vec3s D_80855210[2][2] = {
    { { -200, 700, 100 }, { 800, 600, 800 } },
    { { -200, 500, 0 }, { 600, 400, 600 } },
};

void Player_LearnOcarinaSong(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    static Vec3f zeroVec = { 0.0f, 0.0f, 0.0f };
    static Color_RGBA8 primColor = { 255, 255, 255, 0 };
    static Color_RGBA8 envColor = { 0, 128, 128, 0 };
    s32 age = gSaveContext.linkAge;
    Vec3f sparklePos;
    Vec3f sp34;
    Vec3s* ptr;

    Player_AnimPlaybackType12(globalCtx, this, D_80855208[age]);

    if (this->rightHandType != PLAYER_MODELTYPE_RH_FF) {
        this->rightHandType = PLAYER_MODELTYPE_RH_FF;
        return;
    }

    ptr = D_80855210[gSaveContext.linkAge];

    sp34.x = ptr[0].x + Rand_CenteredFloat(ptr[1].x);
    sp34.y = ptr[0].y + Rand_CenteredFloat(ptr[1].y);
    sp34.z = ptr[0].z + Rand_CenteredFloat(ptr[1].z);

    SkinMatrix_Vec3fMtxFMultXYZ(&this->shieldMf, &sp34, &sparklePos);

    EffectSsKiraKira_SpawnDispersed(globalCtx, &sparklePos, &zeroVec, &zeroVec, &primColor, &envColor, 600, -10);
}

void Player_CutsceneGetSwordBack(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        Player_CutsceneEnd(globalCtx, this, arg2);
    } else if (this->genericTimer == 0) {
        Item_Give(globalCtx, ITEM_SWORD_MASTER);
        func_80846720(globalCtx, this, 0);
    } else {
        func_8084E988(this);
    }
}

void Player_CutsceneGanonKillCombo(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    if (LinkAnimation_Update(globalCtx, &this->skelAnime)) {
        func_8084285C(this, 0.0f, 99.0f, this->skelAnime.endFrame - 8.0f);
    }

    if (this->heldItemActionParam != PLAYER_AP_SWORD_MASTER) {
        func_80846720(globalCtx, this, 1);
    }
}

void Player_CutsceneEnd(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    if (Player_IsSwimming(this)) {
        Player_SetupSwimIdle(globalCtx, this);
        Player_ResetSubCam(globalCtx, this);
    } else {
        Player_ClearLookAndAttention(this, globalCtx);
        if (!Player_SetupSpeakOrCheck(this, globalCtx)) {
            Player_SetupGetItemOrHoldBehavior(this, globalCtx);
        }
    }

    this->csMode = 0;
    this->attentionMode = 0;
}

void Player_CutsceneSetPosAndYaw(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    this->actor.world.pos.x = arg2->startPos.x;
    this->actor.world.pos.y = arg2->startPos.y;
    if ((globalCtx->sceneNum == SCENE_SPOT04) && !LINK_IS_ADULT) {
        this->actor.world.pos.y -= 1.0f;
    }
    this->actor.world.pos.z = arg2->startPos.z;
    this->currentYaw = this->actor.shape.rot.y = arg2->rot.y;
}

void func_80852A54(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    f32 dx = arg2->startPos.x - (s32)this->actor.world.pos.x;
    f32 dy = arg2->startPos.y - (s32)this->actor.world.pos.y;
    f32 dz = arg2->startPos.z - (s32)this->actor.world.pos.z;
    f32 dist = sqrtf(SQ(dx) + SQ(dy) + SQ(dz));
    s16 yawDiff = arg2->rot.y - this->actor.shape.rot.y;

    if ((this->linearVelocity == 0.0f) && ((dist > 50.0f) || (ABS(yawDiff) > 0x4000))) {
        Player_CutsceneSetPosAndYaw(globalCtx, this, arg2);
    }

    this->skelAnime.moveFlags = 0;
    Player_ClearRootLimbPosY(this);
}

void func_80852B4C(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2, struct_80854B18* arg3) {
    if (arg3->type > 0) {
        D_80854AA4[arg3->type](globalCtx, this, arg3->ptr);
    } else if (arg3->type < 0) {
        arg3->func(globalCtx, this, arg2);
    }

    if ((sPrevSkelAnimeMoveFlags & 4) && !(this->skelAnime.moveFlags & 4)) {
        this->skelAnime.morphTable[0].y /= this->ageProperties->unk_08;
        sPrevSkelAnimeMoveFlags = 0;
    }
}

void func_80852C0C(GlobalContext* globalCtx, Player* this, s32 csMode) {
    if ((csMode != 1) && (csMode != 8) && (csMode != 0x31) && (csMode != 7)) {
        Player_DetatchHeldActor(globalCtx, this);
    }
}

void Player_CutsceneUnk6Update(GlobalContext* globalCtx, Player* this, CsCmdActorAction* arg2) {
    CsCmdActorAction* linkCsAction = globalCtx->csCtx.linkAction;
    s32 pad;
    s32 sp24;

    if (globalCtx->csCtx.state == CS_STATE_UNSKIPPABLE_INIT) {
        func_8002DF54(globalCtx, NULL, 7);
        this->csAction = 0;
        Player_StopMovement(this);
        return;
    }

    if (linkCsAction == NULL) {
        this->actor.flags &= ~ACTOR_FLAG_6;
        return;
    }

    if (this->csAction != linkCsAction->action) {
        sp24 = D_808547C4[linkCsAction->action];
        if (sp24 >= 0) {
            if ((sp24 == 3) || (sp24 == 4)) {
                func_80852A54(globalCtx, this, linkCsAction);
            } else {
                Player_CutsceneSetPosAndYaw(globalCtx, this, linkCsAction);
            }
        }

        sPrevSkelAnimeMoveFlags = this->skelAnime.moveFlags;

        Player_EndAnimMovement(this);
        osSyncPrintf("TOOL MODE=%d\n", sp24);
        func_80852C0C(globalCtx, this, ABS(sp24));
        func_80852B4C(globalCtx, this, linkCsAction, &D_80854B18[ABS(sp24)]);

        this->genericTimer = 0;
        this->genericVar = 0;
        this->csAction = linkCsAction->action;
    }

    sp24 = D_808547C4[this->csAction];
    func_80852B4C(globalCtx, this, linkCsAction, &D_80854E50[ABS(sp24)]);
}

void Player_StartCutscene(Player* this, GlobalContext* globalCtx) {
    if (this->csMode != this->prevCsMode) {
        sPrevSkelAnimeMoveFlags = this->skelAnime.moveFlags;

        Player_EndAnimMovement(this);
        this->prevCsMode = this->csMode;
        osSyncPrintf("DEMO MODE=%d\n", this->csMode);
        func_80852C0C(globalCtx, this, this->csMode);
        func_80852B4C(globalCtx, this, NULL, &D_80854B18[this->csMode]);
    }

    func_80852B4C(globalCtx, this, NULL, &D_80854E50[this->csMode]);
}

s32 Player_IsDroppingFish(GlobalContext* globalCtx) {
    Player* this = GET_PLAYER(globalCtx);

    return (Player_DropItemFromBottle == this->actionFunc) && (this->itemActionParam == PLAYER_AP_BOTTLE_FISH);
}

s32 Player_StartFishing(GlobalContext* globalCtx) {
    Player* this = GET_PLAYER(globalCtx);

    if (gSaveContext.linkAge == 1) {
        if (!CHECK_OWNED_EQUIP(EQUIP_SWORD, 0)) {
            gSaveContext.temporaryWeapon = true;
        }
        if (this->heldItemId == ITEM_NONE) {
            this->currentSwordItem = ITEM_SWORD_KOKIRI;
            gSaveContext.equips.buttonItems[0] = ITEM_SWORD_KOKIRI;
            Inventory_ChangeEquipment(EQUIP_SWORD, PLAYER_SWORD_KOKIRI);
        }
    }

    Player_ResetAttributesAndHeldActor(globalCtx, this);
    Player_UseItem(globalCtx, this, ITEM_FISHING_POLE);
    return 1;
}

s32 Player_SetupRestrainedByEnemy(GlobalContext* globalCtx, Player* this) {
    if (!Player_InBlockingCsMode(globalCtx, this) && (this->invincibilityTimer >= 0) &&
        !Player_IsShootingHookshot(this) && !(this->stateFlags3 & PLAYER_STATE3_MOVING_ALONG_HOOKSHOT_PATH)) {
        Player_ResetAttributesAndHeldActor(globalCtx, this);
        Player_SetActionFunc(globalCtx, this, func_8084F308, 0);
        Player_PlayAnimOnce(globalCtx, this, &gPlayerAnim_003120);
        this->stateFlags2 |= PLAYER_STATE2_RESTRAINED_BY_ENEMY;
        Player_ClearAttentionModeAndStopMoving(this);
        Player_PlayVoiceSfxForAge(this, NA_SE_VO_LI_HELD);
        return true;
    }

    return false;
}

// Sets up player cutscene
s32 Player_SetupPlayerCutscene(GlobalContext* globalCtx, Actor* actor, s32 csMode) {
    Player* this = GET_PLAYER(globalCtx);

    if (!Player_InBlockingCsMode(globalCtx, this)) {
        Player_ResetAttributesAndHeldActor(globalCtx, this);
        Player_SetActionFunc(globalCtx, this, Player_StartCutscene, 0);
        this->csMode = csMode;
        this->csTargetActor = actor;
        Player_ClearAttentionModeAndStopMoving(this);
        return 1;
    }

    return 0;
}

void Player_SetupStandingStillMorph(Player* this, GlobalContext* globalCtx) {
    Player_SetActionFunc(globalCtx, this, Player_StandingStill, 1);
    Player_ChangeAnimMorphToLastFrame(globalCtx, this, Player_GetStandingStillAnim(this));
    this->currentYaw = this->actor.shape.rot.y;
}

s32 Player_InflictDamage(GlobalContext* globalCtx, s32 damage) {
    return Player_InflictDamageModified(globalCtx, damage, true);
}

s32 Player_InflictDamageModified(GlobalContext* globalCtx, s32 damage, u8 modified) {
    Player* this = GET_PLAYER(globalCtx);

    if (!Player_InBlockingCsMode(globalCtx, this) && !Player_Damage_modified(globalCtx, this, damage, modified)) {
        this->stateFlags2 &= ~PLAYER_STATE2_RESTRAINED_BY_ENEMY;
        return 1;
    }

    return 0;
}

// Start talking with the given actor
void Player_StartTalkingWithActor(GlobalContext* globalCtx, Actor* actor) {
    Player* this = GET_PLAYER(globalCtx);
    s32 pad;

    if ((this->talkActor != NULL) || (actor == this->naviActor) ||
        CHECK_FLAG_ALL(actor->flags, ACTOR_FLAG_0 | ACTOR_FLAG_18)) {
        actor->flags |= ACTOR_FLAG_8;
    }

    this->talkActor = actor;
    this->exchangeItemId = EXCH_ITEM_NONE;

    if (actor->textId == 0xFFFF) {
        func_8002DF54(globalCtx, actor, 1);
        actor->flags |= ACTOR_FLAG_8;
        Player_UnequipItem(globalCtx, this);
    } else {
        if (this->actor.flags & ACTOR_FLAG_8) {
            this->actor.textId = 0;
        } else {
            this->actor.flags |= ACTOR_FLAG_8;
            this->actor.textId = actor->textId;
        }

        if (this->stateFlags1 & PLAYER_STATE1_RIDING_HORSE) {
            s32 sp24 = this->genericTimer;

            Player_UnequipItem(globalCtx, this);
            Player_SetupTalkWithActor(globalCtx, this);

            this->genericTimer = sp24;
        } else {
            if (Player_IsSwimming(this)) {
                Player_SetupMiniCsFunc(globalCtx, this, Player_SetupTalkWithActor);
                Player_ChangeAnimLongMorphLoop(globalCtx, this, &gPlayerAnim_003328);
            } else if ((actor->category != ACTORCAT_NPC) || (this->heldItemActionParam == PLAYER_AP_FISHING_POLE)) {
                Player_SetupTalkWithActor(globalCtx, this);

                if (!Player_IsUnfriendlyZTargeting(this)) {
                    if ((actor != this->naviActor) && (actor->xzDistToPlayer < 40.0f)) {
                        Player_PlayAnimOnceSlowed(globalCtx, this, &gPlayerAnim_002DF0);
                    } else {
                        Player_PlayAnimLoop(globalCtx, this, Player_GetStandingStillAnim(this));
                    }
                }
            } else {
                Player_SetupMiniCsFunc(globalCtx, this, Player_SetupTalkWithActor);
                Player_PlayAnimOnceSlowed(globalCtx, this,
                                          (actor->xzDistToPlayer < 40.0f) ? &gPlayerAnim_002DF0 : &gPlayerAnim_0031A0);
            }

            if (this->skelAnime.animation == &gPlayerAnim_002DF0) {
                Player_SetupAnimMovement(globalCtx, this, 0x19);
            }

            Player_ClearAttentionModeAndStopMoving(this);
        }

        this->stateFlags1 |= PLAYER_STATE1_TALKING | PLAYER_STATE1_IN_CUTSCENE;
    }

    if ((this->naviActor == this->talkActor) && ((this->talkActor->textId & 0xFF00) != 0x200)) {
        this->naviActor->flags |= ACTOR_FLAG_8;
        Player_SetCameraTurnAround(globalCtx, 0xB);
    }
}
