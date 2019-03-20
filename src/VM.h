#pragma once

#include <inttypes.h>

#include <stack>

namespace RenderGraph
{

enum class OpCode : uint8_t
{
	NOP,

	// Stack
	PUSH,
	POP,

	// Arithmetic operators
	ADD,
	SUB,
	MUL,
	DIV,
	MOD,

	NEG,
	ABS,
	FMA,

	// Comparison operators
	EQ,
	NEQ,
	GT,
	GTE,
	LT,
	LTE,

	// Logical operators
	NOT,
	AND,
	OR,

	// Branch
	JMP,
	JMPT,
	JMPF,

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

class Parameters
{
public:

	inline Parameters(std::stack<Value> & stack) : m_stack(stack)
	{
		// ...
	}

	inline Value pop()
	{
		Value v = m_stack.top();
		m_stack.pop();
		return v;
	}

	inline void push(const Value & v)
	{
		m_stack.push(v);
	}

	inline std::stack<Value>::size_type size() const
	{
		return m_stack.size();
	}

private:

	std::stack<Value> & m_stack;
};

}
