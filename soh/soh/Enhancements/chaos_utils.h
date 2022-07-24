#ifndef _CHAOS_UTILS_H_
#define _CHAOS_UTILS_H_

extern "C" {
#include <z64.h>
#include "variables.h"
#include "functions.h"
#include "macros.h"
extern GlobalContext* gGlobalCtx;
extern void Player_SetupInvincibility(Player*, s32);
extern void Player_SetupDamage(GlobalContext*, Player*, s32, f32, f32, s16, s32);
}

#define MAX_TURBO_SPEED 16.0f

void scale(Actor* actor, float x, float y, float z) {
    actor->scale.x *= x;
    actor->scale.y *= y;
    actor->scale.z *= z;
}

void spawn_on_link(int16_t id, int16_t params) {
    Player* player = GET_PLAYER(gGlobalCtx);
    Actor_Spawn(&(gGlobalCtx->actorCtx), gGlobalCtx, id,
                player->actor.world.pos.x,
                player->actor.world.pos.y,
                player->actor.world.pos.z,
                player->actor.shape.rot.x,
                player->actor.shape.rot.y,
                player->actor.shape.rot.z,
                params);
}

uint32_t g_satisified_pending_frames = 0;
bool link_is_ready() {
    Player* player = GET_PLAYER(gGlobalCtx);

    if ((player->targetSwitchTimer & 0xFCAC2485) == 0 &&
        (player->actor.bgCheckFlags & 0x0001) &&
        (player->swordEffectIndex & 0x000C0000) == 0 &&
        //TODO: Figure this one out 
        //(z64_event_state_1 & 0x20) == 0 &&
        (gGlobalCtx->cameraPtrs[1] == 0)) {
        g_satisified_pending_frames++;
    }
    else {
        g_satisified_pending_frames = 0;
    }
    if (g_satisified_pending_frames >= 2) {
        g_satisified_pending_frames = 0;
        return false;
    }
    return true;
}

uint8_t g_pending_freezes = 0;
bool ice_trap_is_pending() {
    return g_pending_freezes > 0;
}

void push_pending_ice_trap() {
    g_pending_freezes++;
}

void give_ice_trap() {
    Player* player = GET_PLAYER(gGlobalCtx);
    if (g_pending_freezes) {
        g_pending_freezes--;
        Player_SetupInvincibility(player, 0x14);
		Player_SetupDamage(gGlobalCtx, player, 3, 0.0f, 0.0f, 0, 20);
    }
}

void execute_game(int16_t entrance_index, uint16_t cutscene_index)
{
  if (entrance_index != gSaveContext.entranceIndex ||
      cutscene_index != gSaveContext.cutsceneIndex)
  {
    gSaveContext.seqId = -1;
    gSaveContext.natureAmbienceId = -1;
  }
  gSaveContext.entranceIndex = entrance_index;
  gSaveContext.cutsceneIndex = cutscene_index;
  gSaveContext.gameMode = 0;
  if (gSaveContext.minigameState == 1)
    gSaveContext.minigameState = 3;
  gGlobalCtx->nextEntranceIndex = entrance_index;
  gGlobalCtx->state.running = 0;
  gGlobalCtx->state.init = (GameStateFunc)(gGameStateOverlayTable[3].init);
  gGlobalCtx->state.size = gGameStateOverlayTable[3].instanceSize;
}

void void_out() {
  gSaveContext.respawn[0].tempSwchFlags = gGlobalCtx->actorCtx.flags.tempSwch;
  gSaveContext.respawn[0].tempCollectFlags = gGlobalCtx->actorCtx.flags.tempCollect;
  gSaveContext.respawnFlag = 1;
  execute_game(gSaveContext.respawn[0].entranceIndex, 0x0000);
}

void toggle_age() {
  Player* player = GET_PLAYER(gGlobalCtx);
  int age = gSaveContext.linkAge;
  gSaveContext.linkAge = gGlobalCtx->linkAgeOnLoad;
  gGlobalCtx->linkAgeOnLoad = !gGlobalCtx->linkAgeOnLoad;
  Inventory_SwapAgeEquipment();
  gSaveContext.linkAge = age;

  // TODO: check if this is the right function to call to update the c-buttons
  for (int i = 0; i < 4; ++i)
    if (gSaveContext.equips.buttonItems[i] != -1)
      Interface_LoadItemIcon1(gGlobalCtx, i);

  Player_SetEquipmentData(gGlobalCtx, player);
  execute_game(gSaveContext.entranceIndex, 0x0);
}

// hack during porting, command should be doing cleanup
bool previous_frame_had_forced_boots = false;
bool previous_frame_had_ohko = false;
void apply_ongoing_effects() {
    Player* player = GET_PLAYER(gGlobalCtx);

    // TODO: replace |timers| with just a CVar
    int16_t forced_boots = CVar_GetS32("gChaosForcedBoots", 0);
    bool ohko = CVar_GetS32("gChaosOHKO", 0);
    bool no_hud = CVar_GetS32("gChaosNoHud", 0);
    bool no_z = CVar_GetS32("gChaosNoZ", 0);
    bool turbo = CVar_GetS32("gChaosTurbo", 0);
    bool invert_controls = CVar_GetS32("gChaosInvertControls", 0);

    if (forced_boots > 0) {
        // Boots are the first nibble of the |equipement| u16
        gSaveContext.equips.equipment = ((gSaveContext.equips.equipment & 0x0FFF) | (forced_boots << 12));
        Player_SetEquipmentData(gGlobalCtx, player);
    } else if (previous_frame_had_forced_boots) {
        // Restore to kokiri boots when the time elapses
        gSaveContext.equips.equipment = ((gSaveContext.equips.equipment & 0x0FFF) | (1 << 12));
        Player_SetEquipmentData(gGlobalCtx, player);
    }

    previous_frame_had_forced_boots = forced_boots;

    if (ohko) {
        if (gSaveContext.health == 0) {
            // Actually let us die if life is already at 0
            CVar_SetS32("gChaosOHKO", 0);
        } else {
            gSaveContext.health = 1;
        }
    } else if (previous_frame_had_ohko) {
        // If this was the last frame of the OHKO mode, 
        // give the player 1 heart because we're nice like that.
        gSaveContext.health = 16;
    }

    previous_frame_had_ohko = ohko;

    if (no_hud) {
        // gz's |hud_flag|
        gSaveContext.unk_13E8 = 0x001;
    }

    if (no_z) {
        gGlobalCtx->state.input[0].press.button &= (~BTN_Z);
    }

    if (turbo) {
        float vel = player->linearVelocity * 2.0f;
        // 27 is HESS speed I think?
        player->linearVelocity = vel > MAX_TURBO_SPEED ? MAX_TURBO_SPEED : vel;
    }

    if (invert_controls) {
        gGlobalCtx->state.input[0].cur.stick_x = -gGlobalCtx->state.input[0].cur.stick_x;
        gGlobalCtx->state.input[0].cur.stick_y = -gGlobalCtx->state.input[0].cur.stick_y;
        gGlobalCtx->state.input[0].rel.stick_x = -gGlobalCtx->state.input[0].rel.stick_x;
        gGlobalCtx->state.input[0].rel.stick_y = -gGlobalCtx->state.input[0].rel.stick_y;
    }
}

#endif