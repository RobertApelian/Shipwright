#include "game-interactor/GameInteractor.h"

// MARK: - State Definitions

float GameInteractor::ChaosState::CustomLinkScale = 1.0f;

// MARK: C - Bridge

float GameInteractor_CustomLinkScale() {
    return GameInteractor::ChaosState::CustomLinkScale;
}
