#ifndef _CHAOS_WIN_H_
#define _CHAOS_WIN_H_

#include <windows.h> 
#include <stdio.h>
#include <tchar.h>

#include <vector>

HANDLE hPipe;

void PlatformStop() {
    CloseHandle(hPipe);
}

void PlatformStart() {
    hPipe = CreateFile(TEXT("\\\\.\\pipe\\chaos"), // pipe name
                       GENERIC_READ | GENERIC_WRITE,
                       0,             // no sharing
                       NULL,          // default security attributes
                       OPEN_EXISTING, // opens existing pipe
                       0,             // default attributes
                       NULL);         // no template file
    if (hPipe == INVALID_HANDLE_VALUE) {
        // SohImGui::overlay->TextDrawNotification(10.0f, true, "Failed to open pipe");
        PlatformStop();
    }
}

bool PlatformReadBytes(size_t num, std::vector<uint8_t>* buf) {
    DWORD bytes_avail = 0;
    if (PeekNamedPipe(hPipe, NULL, 0, NULL, &bytes_avail, NULL) && !bytes_avail) {
        return false;
    }

    DWORD cbRead;
    if (ReadFile(hPipe, buf->data(), num, &cbRead, NULL)) {
        return cbRead == num;
    }

    return false;
}

#endif