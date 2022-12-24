#ifndef _CHAOS_DUMMY_H_
#define _CHAOS_DUMMY_H_

#include <vector>

void PlatformStop() {
    //
}

void PlatformStart() {
    //
}

bool PlatformReadBytes(size_t num, std::vector<uint8_t>* buf) {
    return false;
}

#endif