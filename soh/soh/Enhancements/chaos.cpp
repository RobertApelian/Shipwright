#include "ImGuiImpl.h"
#include "Cvar.h"

#include "chaos_commands.h"

#include <z64.h>
#include <variables.h>

#include <vector>
#include <queue>

#if defined(__linux__)
#include "chaos_linux.h"
#endif

static bool g_is_enabled = false;
static CommandStorage g_command_storage;

template<typename T>
T Read(const std::vector<uint8_t>& bytes, size_t start_index) {
	return *((T*)(bytes.data() + start_index));
}

void Start() {
	StartLinux();
	SohImGui::overlay->TextDrawNotification(10.0f, true, "Chaos Mode Enabled");
}

void DisplayCommandMessage(const std::vector<uint8_t>& bytes, size_t start_index) {
	std::string msg(bytes.begin() + start_index, bytes.end());
	SohImGui::overlay->TextDrawNotification(5.0f, true, msg.c_str());
}

void EnqueueCommand(const std::vector<uint8_t>& bytes) {
	size_t message_start_index = 1; // Account for the command idx itself
	switch (bytes[0]) {
		case 0x03: {
			g_command_storage.AddCommand(std::make_unique<OneShotCommand>([](){
			    gSaveContext.health = 0;
			}));
		    break;
		}
		default:
			SohImGui::overlay->TextDrawNotification(10.0f, true, "Unrecognized command");
			return;
	}

	DisplayCommandMessage(bytes, message_start_index);
}

bool ReadBytes(size_t num, std::vector<uint8_t>* buf) {
	return ReadBytesLinux(num, buf);
}

void EachFrameCallback() {
	bool new_val = CVar_GetS32("gChaosEnabled", 0);
	if (g_is_enabled && !new_val) {
		// disable
	}

	if (!g_is_enabled && new_val) {
		Start();
	}

	g_is_enabled = new_val;

	if (!g_is_enabled) return;

	// Try to read a command from the buffer. If there isn't one, try again next frame.
	// First byte is the size of the command, in bytes (excluding the size byte).
	std::vector<uint8_t> buf(1);
	if (ReadBytes(1, &buf)){
		std::vector<uint8_t> current_command_buffer;
		size_t bytes_to_read = buf[0];
		current_command_buffer.resize(bytes_to_read);

		if (!ReadBytes(bytes_to_read, &current_command_buffer)) {
			std::string msg = "Error reading command, turning off Chaos Mode";
			SohImGui::overlay->TextDrawNotification(10.0f, true, msg.c_str());
			close(fd);
			CVar_SetS32("gChaosEnabled", 0);
			g_is_enabled = false;
			return;
		}

		EnqueueCommand(current_command_buffer);
	}

	g_command_storage.Tick();
}

extern "C" {
	void Chaos_Init() {
    	CVar_RegisterS32("gChaosEnabled", 0);
	}

	void Chaos_EachFrame() {
		EachFrameCallback();
	}
}

