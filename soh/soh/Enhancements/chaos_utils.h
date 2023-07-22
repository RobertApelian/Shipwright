#ifndef _CHAOS_UTILS_H_
#define _CHAOS_UTILS_H_

extern "C" {
#include <z64.h>
#include "variables.h"
#include "functions.h"
#include "macros.h"
extern PlayState* gPlayState;
extern void Player_SpawnExplosion(PlayState*, Player*);
extern void Player_SetupInvincibility(Player*, s32);
extern void Player_SetupDamage(PlayState*, Player*, s32, f32, f32, s16, s32);
}

#define MAX_TURBO_SPEED 16.0f

void scale(Actor* actor, float x, float y, float z) {
    actor->scale.x *= x;
    actor->scale.y *= y;
    actor->scale.z *= z;
}

Actor* spawn_on_link(int16_t id, int16_t params) {
    Player* player = GET_PLAYER(gPlayState);
    return Actor_Spawn(&(gPlayState->actorCtx), gPlayState, id,
                player->actor.world.pos.x,
                player->actor.world.pos.y,
                player->actor.world.pos.z,
                player->actor.shape.rot.x,
                player->actor.shape.rot.y,
                player->actor.shape.rot.z,
                params,
                false);
}

#define DIST_FROM_PLAYER 100.f

void spawn_n(int16_t id, int16_t params, int32_t n) {
    if (n == 1) {
        spawn_on_link(id, params);
    } else {
        Player* player = GET_PLAYER(gPlayState);
        float sigma = (float)rand()/(float)(RAND_MAX/(M_PI / 2.f));
        float angle_step = (2.f * M_PI) / static_cast<float>(n);
        for (int32_t i = 0; i < n; ++i, sigma += angle_step) {
            Actor_Spawn(&(gPlayState->actorCtx), gPlayState, id,
                player->actor.world.pos.x + DIST_FROM_PLAYER * sin(sigma),
                player->actor.world.pos.y,
                player->actor.world.pos.z + DIST_FROM_PLAYER * cos(sigma),
                player->actor.shape.rot.x,
                player->actor.shape.rot.y,
                player->actor.shape.rot.z,
                params,
                false);
        }
    }
	if (id == 0x113) {
		CVarSetInteger("gActivateNextIK", 1);
	}
}

uint32_t g_satisified_pending_frames = 0;
bool link_is_ready() {
    Player* player = GET_PLAYER(gPlayState);

    if ((player->targetSwitchTimer & 0xFCAC2485) == 0 &&
        (player->actor.bgCheckFlags & 0x0001) &&
        (player->meleeWeaponEffectIndex & 0x000C0000) == 0 &&
        //TODO: Figure this one out 
        //(z64_event_state_1 & 0x20) == 0 &&
        (gPlayState->cameraPtrs[1] == 0)) {
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
    Player* player = GET_PLAYER(gPlayState);
    if (g_pending_freezes) {
        g_pending_freezes--;
        Player_SetupInvincibility(player, 0x14);
		Player_SetupDamage(gPlayState, player, 3, 0.0f, 0.0f, 0, 20);
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
  gPlayState->nextEntranceIndex = entrance_index;
  gPlayState->state.running = 0;
  gPlayState->state.init = (GameStateFunc)(gGameStateOverlayTable[3].init);
  gPlayState->state.size = gGameStateOverlayTable[3].instanceSize;
}

void void_out() {
  gSaveContext.respawn[0].tempSwchFlags = gPlayState->actorCtx.flags.tempSwch;
  gSaveContext.respawn[0].tempCollectFlags = gPlayState->actorCtx.flags.tempCollect;
  gSaveContext.respawnFlag = 1;
  execute_game(gSaveContext.respawn[0].entranceIndex, 0x0000);
}

void toggle_age() {
  Player* player = GET_PLAYER(gPlayState);
  int age = gSaveContext.linkAge;
  gSaveContext.linkAge = gPlayState->linkAgeOnLoad;
  gPlayState->linkAgeOnLoad = !gPlayState->linkAgeOnLoad;
  Inventory_SwapAgeEquipment();
  gSaveContext.linkAge = age;

  // TODO: check if this is the right function to call to update the c-buttons
  for (int i = 0; i < 4; ++i)
    if (gSaveContext.equips.buttonItems[i] != -1)
      Interface_LoadItemIcon1(gPlayState, i);

  Player_SetEquipmentData(gPlayState, player);
  execute_game(gSaveContext.entranceIndex, 0x0);
}

// hack during porting, command should be doing cleanup
bool previous_frame_had_forced_boots = false;
bool previous_frame_had_ohko = false;
void apply_ongoing_effects() {
    Player* player = GET_PLAYER(gPlayState);

    // TODO: replace |timers| with just a CVar
    int16_t forced_boots = CVarGetInteger("gChaosForcedBoots", 0);
    bool ohko = CVarGetInteger("gChaosOHKO", 0);
    bool no_hud = CVarGetInteger("gChaosNoHud", 0);
    bool no_z = CVarGetInteger("gChaosNoZ", 0);
    bool turbo = CVarGetInteger("gChaosTurbo", 0);
    bool invert_controls = CVarGetInteger("gChaosInvertControls", 0);

    if (forced_boots > 0) {
        // Boots are the first nibble of the |equipement| u16
        gSaveContext.equips.equipment = ((gSaveContext.equips.equipment & 0x0FFF) | (forced_boots << 12));
        Player_SetEquipmentData(gPlayState, player);
    } else if (previous_frame_had_forced_boots) {
        // Restore to kokiri boots when the time elapses
        gSaveContext.equips.equipment = ((gSaveContext.equips.equipment & 0x0FFF) | (1 << 12));
        Player_SetEquipmentData(gPlayState, player);
    }

    previous_frame_had_forced_boots = forced_boots;

    if (ohko) {
        if (gSaveContext.health == 0) {
            // Actually let us die if life is already at 0
            CVarSetInteger("gChaosOHKO", 0);
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
        gPlayState->state.input[0].press.button &= (~BTN_Z);
    }

    if (turbo) {
        float vel = player->linearVelocity * 2.0f;
        // 27 is HESS speed I think?
        player->linearVelocity = vel > MAX_TURBO_SPEED ? MAX_TURBO_SPEED : vel;
    }

    if (invert_controls) {
        gPlayState->state.input[0].cur.stick_x = -gPlayState->state.input[0].cur.stick_x;
        gPlayState->state.input[0].cur.stick_y = -gPlayState->state.input[0].cur.stick_y;
        gPlayState->state.input[0].rel.stick_x = -gPlayState->state.input[0].rel.stick_x;
        gPlayState->state.input[0].rel.stick_y = -gPlayState->state.input[0].rel.stick_y;
    }
}

#endif