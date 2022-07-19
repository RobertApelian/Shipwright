#ifndef _CHAOS_COMMANDS_H_
#define _CHAOS_COMMANDS_H_

#include <algorithm>
#include <functional>
#include <memory>
#include <vector>


class ChaosCommand {
	public:
		// Called every frame, returns true if the command is still active after the tick, false otherwise
		virtual bool Tick() = 0; 
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

		bool Tick() override {
			f_();
			return false;
		}

		std::function<void()> f_;
};

#endif