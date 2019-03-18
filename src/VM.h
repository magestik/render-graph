#pragma once

#include <inttypes.h>

#include <vector>

namespace RenderGraph
{

enum class OpCode : uint8_t
{
	NOP,

	// Stack
	PUSH,
	POP,

	// Arithmetic
	ADD,
	SUB,
	MUL,
	DIV,

	// Comparison
	EQ,
	NEQ,
	GT,
	GTE,
	LT,
	LTE,

	// Branch
	JMP,

	// Functions
	CALL,
	HALT
};

union Value
{
	unsigned int	asUInt;
	int				asInt;
	float			asFloat;
	bool			asBool;
};

static_assert(sizeof (Value) == sizeof(uint32_t), "Value union does not have the expected size");

class Parameters : public std::vector<std::pair<unsigned int, Value>>
{

};

enum CallType
{
	PASS
};

}
