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
			return DoTick();
		}

		virtual bool DoTick() = 0;
		virtual bool CanStart() { return true; }
};

class PredicatedCommand : public ChaosCommand {
	public:
		PredicatedCommand(std::unique_ptr<ChaosCommand> command, std::function<bool()> pred)
			: command_(std::move(command)), predicate_(pred) {}

		bool DoTick() {
			return command_->DoTick();
		}

		bool CanStart() {
			return predicate_() && command_->CanStart();
		}

		std::unique_ptr<ChaosCommand> command_;
		std::function<bool()> predicate_;
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
			if (!CanStart()) return true;
			f_();
			return false;
		}

		std::function<void()> f_;
};

class OneShotWithCleanupCommand : public ChaosCommand {
public:
	OneShotWithCleanupCommand(std::function<void()> tick_f, std::function<void()> cleanup_f, uint32_t frames_before_cleanup)
		: tick_f_(tick_f), cleanup_f_(cleanup_f), frames_(frames_before_cleanup) {}

	bool DoTick() override {
		if (!CanStart()) return true;

		if (!started_) {
			started_ = true;
			tick_f_();
		} else {
			if (--frames_ == 0) {
				cleanup_f_();
				return false;
			}
		}

			return true;
	}

	std::function<void()> tick_f_;
	std::function<void()> cleanup_f_;

	uint32_t frames_;
	bool started_ = false;
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

class OneShotTimedCommand : public ChaosCommand {
	public:
		OneShotTimedCommand(std::function<void()> tick_f, std::function<void()> cleanup_f, uint32_t seconds) 
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

			if (new_) {
				tick_f_();
				new_ = false;
			}
			return true;
		}

		std::function<void()> tick_f_;
		std::function<void()> cleanup_f_;

		uint32_t seconds_;
		time_t start_time_ = 0;
		bool new_ = true;
};

class TimedCVarCommand : public TimedCommand {
	public:
		TimedCVarCommand(const std::string& cvar, uint32_t seconds, int32_t initial, int32_t applied)
			: cvar_(cvar), 
			  initial_(initial),
			  applied_(applied),
			  TimedCommand(
				[=]() { CVar_SetS32(cvar_.c_str(), applied_); },
				[=]() { CVar_SetS32(cvar_.c_str(), initial_); },
				seconds) {}

		bool CanStart() override {
			return !CVar_GetS32(cvar_.c_str(), initial_);
		}

		std::string cvar_;
		int32_t initial_;
		int32_t applied_;
};

class TimedBooleanCVarCommand : public TimedCVarCommand {
	public:
		TimedBooleanCVarCommand(const std::string& cvar, uint32_t seconds)
			: TimedCVarCommand(cvar, seconds, 0, 1) {}
};

class OneShotBooleanCVarCommand : public OneShotCommand {
	public:
		OneShotBooleanCVarCommand(const std::string& cvar) 
			: cvar_(cvar),
			  OneShotCommand([=]() { CVar_SetS32(cvar_.c_str(), 1); }) {}

		std::string cvar_;
};

#endif