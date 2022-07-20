#include "ImGuiImpl.h"
#include "Cvar.h"

#include "chaos_commands.h"
#include "chaos_commands_macros.h"

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

uint8_t TIMED_CVAR_ID = 0x11;
static std::map<uint8_t, CommandCreator> kCommands {
	CMD_ONE_SHOT(0x03, PL_NONE(), { gSaveContext.health = 0; }),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gEnemyHealthBar"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gDisableFPSView"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gForceNormalArrows"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gDisableLedgeClimb"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gFloorIsLava"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gExplodingRolls"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gFreezingRolls"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gDisableTargeting"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gMegaLetterbox"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gDisableTurning"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gJailTime"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gOnHold"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gSonicRoll"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gNaviSpam"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gScuffedLink"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gRaveMode"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gInvisPlayer"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gSlipperyFloor"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gIceDamage"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gElectricDamage"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gKnockbackDamage"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gFireDamage"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gForwardJump"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gBigHead"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gTinyHead"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gDarkenArea"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gChaosSpin"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gDisableMeleeAttacks"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gDisableEnemyDraw"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gSandstorm"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gSinkingFloor"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gCowRitual"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gFireRockRain"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gCuccoAttack"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gExplodingRupeeChallenge"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gBanItemDropPickup"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gBrokenBombchus"),
	CMD_TIMED_CVAR(TIMED_CVAR_ID++, "gAnnoyingGetItems"),
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

