#pragma once

#include <inttypes.h>

#include <vector>

namespace RenderGraph
{

typedef uint32_t Instruction;

enum class OpCode : uint8_t
{
	NOP,

	// Arithmetic
	ADD,
	SUB,
	MUL,
	DIV,

	// Branch
	CMP,
	JMP,

	// Functions
	PUSH,
	CALL
};

union Value
{
	unsigned int	asUInt;
	int				asInt;
	float			asFloat;
};

static_assert(sizeof (Value) == sizeof(uint32_t), "Value union does not have the expected size");

class Parameters : public std::vector<std::pair<unsigned int, Value>>
{

};

enum CallType
{
	PASS
};

enum CmpResult
{
	NEG,
	POS,
	ZERO,
};


}
