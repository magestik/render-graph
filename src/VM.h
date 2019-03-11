#pragma once

#include <inttypes.h>

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
