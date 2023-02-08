#include "SetCutscenes.h"

namespace Ship {
void* SetCutscenes::GetPointer() {
    if (cutscene == nullptr) {
        return nullptr;
    }
    return cutscene->GetPointer();
}

size_t SetCutscenes::GetPointerSize() {
    if (cutscene == nullptr) {
        return 0;
    }
	return cutscene->GetPointerSize();
}
} // namespace Ship
