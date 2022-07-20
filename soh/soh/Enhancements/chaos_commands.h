#ifndef _CHAOS_COMMANDS_H_
#define _CHAOS_COMMANDS_H_

#include <algorithm>
#include <functional>
#include <memory>
#include <vector>

#include <ctime>

#include "ImGuiImpl.h"

void Log(const std::string& msg) {
	SohImGui::overlay->TextDrawNotification(10.0f, true, msg.c_str());
}

class ChaosCommand {
	public:
		// Called every frame, returns true if the command is still active after the tick, false otherwise
		bool Tick() {
			if (CanTick()) {
				return DoTick();
			}
			return true;
		}

		virtual bool DoTick() = 0;
		virtual bool CanTick() { return true; }
		virtual bool CanStart() { return true; }
};

class CommandStorage {
	public:
		void Tick() {
			auto it = std::remove_if(commands_.begin(),
									 commands_.end(),
									 [](auto& c) { return !c->Tick(); });
			commands_.erase(it, commands_.end());
		}

		void AddCommand(std::unique_ptr<ChaosCommand> command) {
			commands_.push_back(std::move(command));
		}

	private:
		std::vector<std::unique_ptr<ChaosCommand>> commands_;
};

class OneShotCommand : public ChaosCommand {
	public:
		OneShotCommand(std::function<void()> f) : f_(f) {}

		bool DoTick() override {
			f_();
			return false;
		}

		std::function<void()> f_;
};

class TimedCommand : public ChaosCommand {
	public:
		TimedCommand(std::function<void()> tick_f, std::function<void()> cleanup_f, uint32_t seconds) 
			: tick_f_(tick_f), cleanup_f_(cleanup_f), seconds_(seconds) {}

		bool DoTick() override {
			if (!start_time_) {
				if (!CanStart()) return true;

				start_time_ = time(nullptr);
			}

			if ((time(nullptr) - start_time_) >= seconds_) {
				cleanup_f_();
				return false;
			}

			tick_f_();
			return true;
		}

		std::function<void()> tick_f_;
		std::function<void()> cleanup_f_;

		uint32_t seconds_;
		time_t start_time_ = 0;
};

class TimedBooleanCVarCommand : public TimedCommand {
	public:
		TimedBooleanCVarCommand(const std::string& cvar, uint32_t seconds)
			: cvar_(cvar), 
			  TimedCommand(
				[=]() { CVar_SetS32(cvar_.c_str(), 1); },
				[=]() { CVar_SetS32(cvar_.c_str(), 0); },
				seconds) {}

		bool CanStart() override {
			return !CVar_GetS32(cvar_.c_str(), 0);
		}

		std::string cvar_;
};

class OneShotBooleanCVarCommand : public OneShotCommand {
	public:
		OneShotBooleanCVarCommand(const std::string& cvar) 
			: cvar_(cvar),
			  OneShotCommand([=]() { CVar_SetS32(cvar_.c_str(), 1); }) {}

		std::string cvar_;
};

#endif