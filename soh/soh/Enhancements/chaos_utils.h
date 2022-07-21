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
  gGlobalCtx->state.init = gGameStateOverlayTable[3].init;
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

#endif