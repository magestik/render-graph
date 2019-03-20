#include "Instance.h"

#include "Texture.h"
#include "Framebuffer.h"
#include "Pass.h"

#include "Graph.h"

#include "VM.h"

#include <math.h>

#include <assert.h>

#include <stack>

#if defined(__GNUC__) || defined(__clang__)
#	define LIKELY(condition) __builtin_expect(!!(condition), 1)
#else
#	define LIKELY(condition) condition
#endif

#define READ_BYTE() (*current++)

namespace RenderGraph
{

/**
 * @brief Constructor
 * @param bytecode
 * @param operations
 * @param framebuffers
 * @param textures
 * @param values
 */
Instance::Instance(const std::vector<uint8_t> & bytecode, const std::vector<Operation*> & operations, const std::vector<Framebuffer*> & framebuffers, const std::vector<Texture*> & textures, const std::vector<Value> & values)
	: m_aValues(values),
	  m_aTextures(textures),
	  m_aFramebuffers(framebuffers),
	  m_aOperations(operations),
	  m_bytecode(bytecode)
{
	// ...
}

/**
 * @brief Destructor
 */
Instance::~Instance(void)
{
	// ...
}

/**
 * @brief Resize textures / framebuffers
 * @param width
 * @param height
 * @return
 */
bool Instance::resize(unsigned int width, unsigned int height)
{
	//
	// Resize textures
	for (Texture * texture : m_aTextures)
	{
		texture->resize(width, height);
	}

	//
	// Resize framebuffers
	for (Framebuffer * framebuffer : m_aFramebuffers)
	{
		framebuffer->resize(width, height);
	}

	return true;
}

/**
 * @brief Render Frame
 * @return
 */
bool Instance::execute(void)
{
	std::stack<Value> stack;

	uint8_t * current = m_bytecode.data();

	bool bRunning = m_bytecode.size() > 0;

	while (bRunning)
	{
		// fetch next instruction
		assert(current < m_bytecode.data() + m_bytecode.size());
		uint8_t instruction = READ_BYTE();

		// decode instruction
		switch (OpCode(instruction))
		{
			case OpCode::NOP:
			{
				// nothing ...
			}
			break;

			case OpCode::PUSH:
			{
				uint8_t addrhi = READ_BYTE();
				uint8_t addrlo = READ_BYTE();

				bool bIsTexture = (addrhi & 0x80) != 0;
				uint16_t addr = ((addrhi << 8) | addrlo) & 0x7FFF;

				if (bIsTexture)
				{
					Value v;
					v.asUInt = m_aTextures[addr]->getNativeHandle();
					stack.push(v);
				}
				else
				{
					stack.push(m_aValues[addr]);
				}
			}
			break;

			case OpCode::POP:
			{
				uint8_t addrhi = READ_BYTE();
				uint8_t addrlo = READ_BYTE();

				bool bIsTexture = (addrhi & 0x80) != 0;
				uint16_t addr = ((addrhi << 8) | addrlo) & 0x7FFF;

				assert(!bIsTexture); // can't pop in texture

				m_aValues[addr] = stack.top();
				stack.pop();
			}
			break;

			case OpCode::ADD:
			{
				uint8_t mode = READ_BYTE();

				Value v2 = stack.top();
				stack.pop();

				Value v1 = stack.top();
				stack.pop();

				if (mode == 0) // unsigned int
				{
					Value result;
					result.asUInt = v1.asUInt + v2.asUInt;
					stack.push(result);
				}
				else if (mode == 1) // signed int
				{
					Value result;
					result.asInt = v1.asInt + v2.asInt;
					stack.push(result);
				}
				else if (mode == 2) // float
				{
					Value result;
					result.asFloat = v1.asFloat + v2.asFloat;
					stack.push(result);
				}
			}
			break;

			case OpCode::SUB:
			{
				uint8_t mode = READ_BYTE();

				Value v2 = stack.top();
				stack.pop();

				Value v1 = stack.top();
				stack.pop();

				if (mode == 0) // unsigned int
				{
					Value result;
					result.asUInt = v1.asUInt - v2.asUInt;
					stack.push(result);
				}
				else if (mode == 1) // signed int
				{
					Value result;
					result.asInt = v1.asInt - v2.asInt;
					stack.push(result);
				}
				else if (mode == 2) // float
				{
					Value result;
					result.asFloat = v1.asFloat - v2.asFloat;
					stack.push(result);
				}
			}
			break;

			case OpCode::MUL:
			{
				uint8_t mode = READ_BYTE();

				Value v2 = stack.top();
				stack.pop();

				Value v1 = stack.top();
				stack.pop();

				if (mode == 0) // unsigned int
				{
					Value result;
					result.asUInt = v1.asUInt * v2.asUInt;
					stack.push(result);
				}
				else if (mode == 1) // signed int
				{
					Value result;
					result.asInt = v1.asInt * v2.asInt;
					stack.push(result);
				}
				else if (mode == 2) // float
				{
					Value result;
					result.asFloat = v1.asFloat * v2.asFloat;
					stack.push(result);
				}
			}
			break;

			case OpCode::DIV:
			{
				uint8_t mode = READ_BYTE();

				Value v2 = stack.top();
				stack.pop();

				Value v1 = stack.top();
				stack.pop();

				if (mode == 0) // unsigned int
				{
					Value result;
					result.asUInt = v1.asUInt / v2.asUInt;
					stack.push(result);
				}
				else if (mode == 1) // signed int
				{
					Value result;
					result.asInt = v1.asInt / v2.asInt;
					stack.push(result);
				}
				else if (mode == 2) // float
				{
					Value result;
					result.asFloat = v1.asFloat / v2.asFloat;
					stack.push(result);
				}
			}
			break;

			case OpCode::MOD:
			{
				uint8_t mode = READ_BYTE();

				Value v2 = stack.top();
				stack.pop();

				Value v1 = stack.top();
				stack.pop();

				if (mode == 0) // unsigned int
				{
					Value result;
					result.asUInt = v1.asUInt % v2.asUInt;
					stack.push(result);
				}
				else if (mode == 1) // signed int
				{
					Value result;
					result.asInt = v1.asInt % v2.asInt;
					stack.push(result);
				}
				else if (mode == 2) // float
				{
					Value result;
					result.asFloat = fmodf(v1.asFloat, v2.asFloat);
					stack.push(result);
				}
			}
			break;

			case OpCode::ABS:
			{
				uint8_t mode = READ_BYTE();

				Value v1 = stack.top();
				stack.pop();

				if (mode == 0) // unsigned int
				{
					Value result;
					result.asUInt = v1.asUInt; // unsigned int >= 0
					stack.push(result);
				}
				else if (mode == 1) // signed int
				{
					Value result;
					result.asInt = (v1.asInt >= 0) ? v1.asInt : -v1.asInt;
					stack.push(result);
				}
				else if (mode == 2) // float
				{
					Value result;
					result.asFloat = (v1.asFloat >= 0.0f) ? v1.asFloat : -v1.asFloat;
					stack.push(result);
				}
			}
			break;

			case OpCode::NEG:
			{
				uint8_t mode = READ_BYTE();

				Value v1 = stack.top();
				stack.pop();

				if (mode == 0) // unsigned int
				{
					Value result;
					result.asUInt = -v1.asUInt;
					stack.push(result);
				}
				else if (mode == 1) // signed int
				{
					Value result;
					result.asInt = -v1.asInt;
					stack.push(result);
				}
				else if (mode == 2) // float
				{
					Value result;
					result.asFloat = -v1.asFloat;
					stack.push(result);
				}
			}
			break;

			case OpCode::FMA:
			{
				uint8_t mode = READ_BYTE();

				Value v3 = stack.top();
				stack.pop();

				Value v2 = stack.top();
				stack.pop();

				Value v1 = stack.top();
				stack.pop();

				if (mode == 0) // unsigned int
				{
					Value result;
					result.asUInt = v1.asUInt + v2.asUInt * v3.asUInt;
					stack.push(result);
				}
				else if (mode == 1) // signed int
				{
					Value result;
					result.asInt = v1.asInt + v2.asInt * v3.asInt;
					stack.push(result);
				}
				else if (mode == 2) // float
				{
					Value result;
					result.asFloat = v1.asFloat + v2.asFloat * v3.asFloat;
					stack.push(result);
				}
			}
			break;

			case OpCode::EQ:
			{
				uint8_t mode = READ_BYTE();

				Value v2 = stack.top();
				stack.pop();

				Value v1 = stack.top();
				stack.pop();

				if (mode == 0) // unsigned int
				{
					Value result;
					result.asBool = (v1.asUInt == v2.asUInt);
					stack.push(result);
				}
				else if (mode == 1) // signed int
				{
					Value result;
					result.asBool = (v1.asInt == v2.asInt);
					stack.push(result);
				}
				else if (mode == 2) // float
				{
					Value result;
					result.asBool = (v1.asFloat == v2.asFloat);
					stack.push(result);
				}
			}
			break;

			case OpCode::NEQ:
			{
				uint8_t mode = READ_BYTE();

				Value v2 = stack.top();
				stack.pop();

				Value v1 = stack.top();
				stack.pop();

				if (mode == 0) // unsigned int
				{
					Value result;
					result.asBool = (v1.asUInt != v2.asUInt);
					stack.push(result);
				}
				else if (mode == 1) // signed int
				{
					Value result;
					result.asBool = (v1.asInt != v2.asInt);
					stack.push(result);
				}
				else if (mode == 2) // float
				{
					Value result;
					result.asBool = (v1.asFloat != v2.asFloat);
					stack.push(result);
				}
			}
			break;

			case OpCode::GT:
			{
				uint8_t mode = READ_BYTE();

				Value v2 = stack.top();
				stack.pop();

				Value v1 = stack.top();
				stack.pop();

				if (mode == 0) // unsigned int
				{
					Value result;
					result.asBool = (v1.asUInt > v2.asUInt);
					stack.push(result);
				}
				else if (mode == 1) // signed int
				{
					Value result;
					result.asBool = (v1.asInt > v2.asInt);
					stack.push(result);
				}
				else if (mode == 2) // float
				{
					Value result;
					result.asBool = (v1.asFloat > v2.asFloat);
					stack.push(result);
				}
			}
			break;

			case OpCode::GTE:
			{
				uint8_t mode = READ_BYTE();

				Value v2 = stack.top();
				stack.pop();

				Value v1 = stack.top();
				stack.pop();

				if (mode == 0) // unsigned int
				{
					Value result;
					result.asBool = (v1.asUInt >= v2.asUInt);
					stack.push(result);
				}
				else if (mode == 1) // signed int
				{
					Value result;
					result.asBool = (v1.asInt >= v2.asInt);
					stack.push(result);
				}
				else if (mode == 2) // float
				{
					Value result;
					result.asBool = (v1.asFloat >= v2.asFloat);
					stack.push(result);
				}
			}
			break;

			case OpCode::LT:
			{
				uint8_t mode = READ_BYTE();

				Value v2 = stack.top();
				stack.pop();

				Value v1 = stack.top();
				stack.pop();

				if (mode == 0) // unsigned int
				{
					Value result;
					result.asBool = (v1.asUInt < v2.asUInt);
					stack.push(result);
				}
				else if (mode == 1) // signed int
				{
					Value result;
					result.asBool = (v1.asInt < v2.asInt);
					stack.push(result);
				}
				else if (mode == 2) // float
				{
					Value result;
					result.asBool = (v1.asFloat < v2.asFloat);
					stack.push(result);
				}
			}
			break;

			case OpCode::LTE:
			{
				uint8_t mode = READ_BYTE();

				Value v2 = stack.top();
				stack.pop();

				Value v1 = stack.top();
				stack.pop();

				if (mode == 0) // unsigned int
				{
					Value result;
					result.asBool = (v1.asUInt <= v2.asUInt);
					stack.push(result);
				}
				else if (mode == 1) // signed int
				{
					Value result;
					result.asBool = (v1.asInt <= v2.asInt);
					stack.push(result);
				}
				else if (mode == 2) // float
				{
					Value result;
					result.asBool = (v1.asFloat <= v2.asFloat);
					stack.push(result);
				}
			}
			break;

			case OpCode::NOT:
			{
				Value v1 = stack.top();
				stack.pop();

				Value result;
				result.asBool = !v1.asBool;
				stack.push(result);
			}
			break;

			case OpCode::AND:
			{
				Value v2 = stack.top();
				stack.pop();

				Value v1 = stack.top();
				stack.pop();

				Value result;
				result.asBool = v1.asBool && v2.asBool;
				stack.push(result);
			}
			break;

			case OpCode::OR:
			{
				Value v2 = stack.top();
				stack.pop();

				Value v1 = stack.top();
				stack.pop();

				Value result;
				result.asBool = v1.asBool || v2.asBool;
				stack.push(result);
			}
			break;

			case OpCode::JMP:
			{
				uint8_t addrhi = READ_BYTE();
				uint8_t addrlo = READ_BYTE();
				uint16_t addr = ((addrhi << 8) | addrlo) & 0xFFFF;

				current = m_bytecode.data() + addr;
			}
			break;

			case OpCode::JMPT:
			{
				uint8_t addrhi = READ_BYTE();
				uint8_t addrlo = READ_BYTE();
				uint16_t addr = ((addrhi << 8) | addrlo) & 0xFFFF;

				Value v = stack.top();
				stack.pop();

				if (v.asBool == true)
				{
					current = m_bytecode.data() + addr;
				}
			}
			break;

			case OpCode::JMPF:
			{
				uint8_t addrhi = READ_BYTE();
				uint8_t addrlo = READ_BYTE();
				uint16_t addr = ((addrhi << 8) | addrlo) & 0xFFFF;

				Value v = stack.top();
				stack.pop();

				if (v.asBool == false)
				{
					current = m_bytecode.data() + addr;
				}
			}
			break;

			case OpCode::CALL:
			{
				uint8_t addrhi = READ_BYTE();
				uint8_t addrlo = READ_BYTE();
				uint16_t addr = ((addrhi << 8) | addrlo) & 0xFFFF;

				Operation * op = m_aOperations[addr];

				if (LIKELY(op))
				{
					Parameters params(stack);
					bRunning = op->execute(params);
				}
				else
				{
					bRunning = false; // exit
				}
			}
			break;

			case OpCode::HALT:
			{
				bRunning = false;
			}
			break;

			default:
			{
				assert(false);
			}
		}
	}

	return true;
}

/**
 * @brief Instance::getRenderTexture
 * @param index
 * @return
 */
unsigned int Instance::getRenderTexture(unsigned int index) const
{
	return m_aTextures[index]->getNativeHandle();
}

/**
 * @brief Instance::setConstant
 * @param index
 * @param value
 */
void Instance::setConstant(unsigned int index, unsigned int value)
{
	m_aValues[index].asUInt = value;
}

/**
 * @brief Instance::setConstant
 * @param index
 * @param value
 */
void Instance::setConstant(unsigned int index, int value)
{
	m_aValues[index].asInt = value;
}

/**
 * @brief Instance::setConstant
 * @param index
 * @param value
 */
void Instance::setConstant(unsigned int index, float value)
{
	m_aValues[index].asFloat = value;
}

/**
 * @brief Instance::setConstant
 * @param index
 * @param value
 */
void Instance::setConstant(unsigned int index, bool value)
{
	m_aValues[index].asBool = value;
}

/**
 * @brief Constructor
 * @param bytecode
 * @param operations
 * @param framebuffers
 * @param textures
 * @param values
 * @param defaultFramebuffer
 */
InstanceWithExternalFramebuffer::InstanceWithExternalFramebuffer(const std::vector<uint8_t> & bytecode, const std::vector<Operation*> & operations, const std::vector<Framebuffer*> & framebuffers, const std::vector<Texture*> & textures, const std::vector<Value> & values, unsigned int /*GLuint*/ defaultFramebuffer)
	: Instance (bytecode, operations, framebuffers, textures, values),
	  m_iDefaultFramebuffer(defaultFramebuffer)
{
	// ...
}

/**
 * @brief Destructor
 */
InstanceWithExternalFramebuffer::~InstanceWithExternalFramebuffer()
{
	// ...
}

/**
 * @brief InstanceWithExternalFramebuffer::getDefaultFramebuffer
 * @return
 */
unsigned int InstanceWithExternalFramebuffer::getDefaultFramebuffer(void) const
{
	return m_iDefaultFramebuffer;
}

/**
 * @brief Constructor
 * @param bytecode
 * @param operations
 * @param framebuffers
 * @param textures
 * @param values
 * @param pDefaultFramebuffer
 */
InstanceWithInternalFramebuffer::InstanceWithInternalFramebuffer(const std::vector<uint8_t> & bytecode, const std::vector<Operation*> & operations, const std::vector<Framebuffer*> & framebuffers, const std::vector<Texture*> & textures, const std::vector<Value> & values, Framebuffer * pDefaultFramebuffer)
	: Instance (bytecode, operations, framebuffers, textures, values),
	  m_pDefaultFramebuffer(pDefaultFramebuffer)
{
	assert(nullptr != pDefaultFramebuffer);
}

/**
 * @brief Destructor
 */
InstanceWithInternalFramebuffer::~InstanceWithInternalFramebuffer()
{
	// ...
}

/**
 * @brief InstanceWithInternalFramebuffer::getDefaultFramebuffer
 * @return
 */
unsigned int InstanceWithInternalFramebuffer::getDefaultFramebuffer(void) const
{
	return m_pDefaultFramebuffer->getNativeHandle();
}

}
