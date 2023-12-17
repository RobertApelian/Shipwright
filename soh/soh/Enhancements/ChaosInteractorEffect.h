#pragma once

#ifndef ChaosInteractionEffect_h
#define ChaosInteractionEffect_h

#include "game-interactor/GameInteractionEffect.h"
#include <stdint.h>

#ifdef __cplusplus

namespace GameInteractionEffect {
    class ModifyLinkScale: public RemovableGameInteractionEffect, public ParameterizedGameInteractionEffect {
        GameInteractionEffectQueryResult CanBeApplied() override;
        void _Apply() override;
        void _Remove() override;
    };
}

#endif /* __cplusplus */
#endif /* ChaosInteractionEffect_h */
