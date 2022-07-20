#include "ImGuiImpl.h"
#include "Cvar.h"

#include "chaos_commands.h"

#include <z64.h>
#include <variables.h>

#include <functional>
#include <map>
#include <vector>
#include <queue>
#include <ctime>
#include <string>

#if defined(__linux__)
#include "chaos_linux.h"
#endif

template<typename T>
T Read(const std::vector<uint8_t>& bytes, size_t start_index) {
	return *((T*)(bytes.data() + start_index));
}

struct CommandCreator {
	std::function<std::vector<uint8_t>(const std::vector<uint8_t>&)> read_payload_;
	std::function<std::unique_ptr<ChaosCommand>(const std::vector<uint8_t>&)> create_;
};

static std::map<uint8_t, CommandCreator> kCommands {
	{ 0x03, {
				[](const std::vector<uint8_t>&) { return std::vector<uint8_t>({}); },
				[](const std::vector<uint8_t>&) { 
					return std::make_unique<OneShotCommand>([]() { gSaveContext.health = 0; }); 
				}
			}
	},
	{ 0x11, {
				[](const std::vector<uint8_t>& bytes) { 
					return std::vector<uint8_t>(bytes.begin() + 1, bytes.begin() + 1 + sizeof(uint32_t)); 
				},
				[](const std::vector<uint8_t>& payload) {
					return std::make_unique<TimedBooleanCVarCommand>(
						"gInvisPlayer", Read<uint32_t>(payload, 0));
				}
			}
	}
};

static bool g_is_enabled = false;
static CommandStorage g_command_storage;

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

	auto it = kCommands.find(bytes[0]);
	if (it == kCommands.end()) {
		SohImGui::overlay->TextDrawNotification(10.0f, true, "Unrecognized command");
		return;
	}

	auto payload = it->second.read_payload_(bytes);
	g_command_storage.AddCommand(it->second.create_(payload));

	DisplayCommandMessage(bytes, message_start_index + payload.size());
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

