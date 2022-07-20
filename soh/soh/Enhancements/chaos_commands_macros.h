#ifndef _CHAOS_COMMANDS_MACROS_H_
#define _CHAOS_COMMANDS_MACROS_H_

// Payloads
#define PL_NONE() [](const std::vector<uint8_t>& bytes) { return std::vector<uint8_t>({}); }

#define PL_BYTES(bytes_count) 									\
[](const std::vector<uint8_t>& bytes) { 						\
	return std::vector<uint8_t>( 								\
		bytes.begin() + 1, bytes.begin() + 1 + bytes_count); 	\
}

// Creators
#define CR_TIMED_CVAR(cvar)								\
[](const std::vector<uint8_t>& payload) {				\
	return std::make_unique<TimedBooleanCVarCommand>(	\
		cvar, Read<uint32_t>(payload, 0));				\
}

#define CR_ONE_SHOT(body) 									\
[](const std::vector<uint8_t>& payload) { 					\
	return std::make_unique<OneShotCommand>([=]() { body });\
}

// Commands
#define CMD(id, payload, creator) { id, { payload, creator }}

#define CMD_TIMED_CVAR(id, cvar) CMD(id, PL_BYTES(sizeof(uint32_t)), CR_TIMED_CVAR(cvar))												\

#define CMD_ONE_SHOT(id, payload, body) CMD(id, payload, CR_ONE_SHOT(body))

#endif