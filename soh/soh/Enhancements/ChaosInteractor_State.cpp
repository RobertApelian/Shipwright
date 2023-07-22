#include "game-interactor/GameInteractor.h"

// MARK: - State Definitions

float GameInteractor::ChaosState::CustomLinkScale = 1.0f;
std::vector<DogFollower> GameInteractor::ChaosState::DogFollowers = {};

// MARK: C - Bridge

float GameInteractor_CustomLinkScale() {
    return GameInteractor::ChaosState::CustomLinkScale;
}
