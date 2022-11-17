#ifndef _CHAOS_COMMANDS_MACROS_H_
#define _CHAOS_COMMANDS_MACROS_H_

#define s_add(a, b, max) (max - a < b ? max : a + b)
#define s_sub(a, b, min) (a - min < b ? min : a - b)

// Payloads
#define PL_NONE() [](const std::vector<uint8_t>& bytes) { return std::vector<uint8_t>({}); }

#define PL_BYTES(bytes_count) 									\
[](const std::vector<uint8_t>& bytes) { 						\
	return std::vector<uint8_t>( 								\
		bytes.begin() + 1, bytes.begin() + 1 + bytes_count); 	\
}

// Creators
#define CR_TIMED_CVAR(cvar, initial, applied)				\
[](const std::vector<uint8_t>& payload) {					\
	return std::make_unique<TimedCVarCommand>(				\
		cvar, Read<uint32_t>(payload, 0), initial, applied);\
}

#define CR_TIMED_BOOL_CVAR(cvar)						\
[](const std::vector<uint8_t>& payload) {				\
	return std::make_unique<TimedBooleanCVarCommand>(	\
		cvar, Read<uint32_t>(payload, 0));				\
}

#define CR_ONE_SHOT_CVAR(cvar)									\
[](const std::vector<uint8_t>& payload) {						\
	return std::make_unique<OneShotBooleanCVarCommand>(cvar);	\
}

#define CR_ONE_SHOT(body) 									\
[](const std::vector<uint8_t>& payload) { 					\
	return std::make_unique<OneShotCommand>([=]() { body });\
}

#define CR_PRED(pred, creator)									\
[](const std::vector<uint8_t>& payload) { 						\
	return std::make_unique<PredicatedCommand>(creator(payload), pred);\
}

#define CR_TIMED(tick, cleanup)						\
[](const std::vector<uint8_t>& payload) {			\
	return std::make_unique<TimedCommand>(			\
		tick, cleanup, Read<uint32_t>(payload, 0)); \
}

#define CR_ONE_SHOT_TIMED(tick, cleanup)						\
[](const std::vector<uint8_t>& payload) {			\
	return std::make_unique<OneShotTimedCommand>(			\
		tick, cleanup, Read<uint32_t>(payload, 0)); \
}

#define CR_ONE_SHOT_CLEANUP(tick, cleanup, frames)						\
[](const std::vector<uint8_t>& payload) {			\
	return std::make_unique<OneShotWithCleanupCommand>(			\
		tick, cleanup, frames); \
}

// Commands
#define CMD(id, payload, creator) { id, { payload, creator }}

#define CMD_TIMED_CVAR(id, cvar, initial, applied) CMD(id, PL_BYTES(sizeof(uint32_t)), CR_TIMED_CVAR(cvar, initial, applied))

#define CMD_TIMED_BOOL_CVAR(id, cvar) CMD(id, PL_BYTES(sizeof(uint32_t)), CR_TIMED_BOOL_CVAR(cvar))

#define CMD_ONE_SHOT(id, payload, body) CMD(id, payload, CR_ONE_SHOT(body))

#define CMD_ONE_SHOT_CVAR(id, cvar) CMD(id, PL_NONE(), CR_ONE_SHOT_CVAR(cvar))

#define CMD_TAKE_AMMO(id, item) CMD_ONE_SHOT(id, PL_BYTES(sizeof(uint32_t)), { uint32_t amt = Read<uint32_t>(payload, 0); AMMO(item) = s_sub(AMMO(item), amt, 0); })

#define CMD_GIVE_AMMO(id, item, upgrade) CMD_ONE_SHOT(id, PL_BYTES(sizeof(uint32_t)), { uint32_t amt = Read<uint32_t>(payload, 0); AMMO(item) = s_add(AMMO(item), amt, CUR_CAPACITY(upgrade)); })

#define CMD_TIMED_BOOL(id, variable) CMD(id, PL_BYTES(sizeof(uint32_t)), CR_ONE_SHOT_TIMED([&]() { variable = 1; }, [&]() { variable = 0; }))

#endif