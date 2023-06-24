/*
GameInteractionEffects is used in conjunction with GameInteractor.

It's intended to be used in places that want to interact with the game
while having checks built-in for if said effect is able to be executed.

Effects that can last for a certain amount of time (timed effects)
have functions to both enable and disable said effect.
*/

#include "ChaosInteractorEffect.h"
#include "game-interactor/GameInteractor.h"
#include <libultraship/bridge.h>

extern "C" {
#include <z64.h>
#include "variables.h"
#include "functions.h"
#include "macros.h"
extern PlayState* gPlayState;
}

namespace GameInteractionEffect {
    // MARK: - ModifyLinkScale
    GameInteractionEffectQueryResult ModifyLinkScale::CanBeApplied() {
        if (!GameInteractor::IsSaveLoaded() || GameInteractor::IsGameplayPaused()) {
            return GameInteractionEffectQueryResult::TemporarilyNotPossible;
        } else {
            return GameInteractionEffectQueryResult::Possible;
        }
    }
    void ModifyLinkScale::_Apply() {
        GameInteractor::ChaosState::CustomLinkScale = parameters[0];
    }
    void ModifyLinkScale::_Remove() {
        GameInteractor::ChaosState::CustomLinkScale = 1.0f;
    }
}
