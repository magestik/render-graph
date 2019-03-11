#include "Instance.h"

#include "Texture.h"
#include "Framebuffer.h"
#include "Pass.h"

#include "Graph.h"

#include "VM.h"

#define GL_GLEXT_PROTOTYPES 1
#include <GL/glcorearb.h>

#include <assert.h>

namespace RenderGraph
{

/**
 * @brief Default constructor
 * @param instructions
 * @param passes
 * @param framebuffers
 * @param textures
 * @param mapTextures
 */
Instance::Instance(const std::vector<Instruction> & instructions, const std::vector<Pass*> & passes, const std::vector<Framebuffer*> & framebuffers, const std::vector<Texture*> & textures, const std::vector<Value> & values, const std::map<std::string, unsigned int> & mapTextures)
	: m_aValues(values),
	  m_aTextures(textures),
	  m_aFramebuffers(framebuffers),
	  m_aPass(passes),
	  m_aInstruction(instructions),
	  m_mapTextures(mapTextures)
{
	assert(passes.size() == framebuffers.size());
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
		if (framebuffer != nullptr)
		{
			framebuffer->resize(width, height);
		}
	}

	//
	// Update passes
	unsigned int count = m_aPass.size();

	for (unsigned int i = 0; i < count; ++i)
	{
		Pass * pass = m_aPass[i];
		assert(pass != nullptr);

		Framebuffer * framebuffer = m_aFramebuffers[i];

		if (framebuffer != nullptr)
		{
			pass->setFramebuffer(framebuffer->getNativeHandle(), width, height);
		}
		else
		{
			pass->setFramebuffer(getDefaultFramebuffer(), width, height);
		}
	}

	return true;
}

/**
 * @brief Render Frame
 * @return
 */
bool Instance::execute(void)
{
	unsigned int current = 0;

	bool bRunning = m_aInstruction.size() > 0;

	RenderGraph::Parameters parameters;

	CmpResult last_cmp = ZERO;

	while (bRunning)
	{
		// fetch next instruction
		assert(current < m_aInstruction.size());
		Instruction instruction = m_aInstruction[current];

		// decode instruction
		OpCode op = OpCode(instruction >> 24);
		unsigned int data = instruction & 0xFFFFFF;

		// execute instruction
		switch (op)
		{
			case OpCode::NOP:
			{
				// nothing ...
			}
			break;

			case OpCode::ADD:
			{
			}
			break;

			case OpCode::SUB:
			{
			}
			break;

			case OpCode::MUL:
			{
			}
			break;

			case OpCode::DIV:
			{
			}
			break;

			case OpCode::CMP:
			{
				uint8_t mode = (data >> 16) & 0xF;
				uint16_t addr = data & 0xFFFF;

				if (mode == 0) // unsigned int
				{
					// TODO
				}
				else if (mode == 1) // signed int
				{
					// TODO
				}
				else if (mode == 2) // float
				{
					// TODO
				}
			}
			break;

			case OpCode::JMP:
			{
				uint8_t mode = (data >> 16) & 0xF;
				uint16_t addr = data & 0xFFFF;

				if (mode == 0) // JNE
				{
					if (last_cmp != ZERO)
					{
						current += addr - 1;
					}
				}
				else if (mode == 1) // JE
				{
					if (last_cmp == ZERO)
					{
						current += addr - 1;
					}
				}
				else if (mode == 2) // JG
				{
					if (last_cmp == POS)
					{
						current += addr - 1;
					}
				}
				else if (mode == 3) // JGE
				{
					if (last_cmp == POS || last_cmp == ZERO)
					{
						current += addr - 1;
					}
				}
				else if (mode == 4) // JL
				{
					if (last_cmp == NEG)
					{
						current += addr - 1;
					}
				}
				else if (mode == 3) // JLE
				{
					if (last_cmp == NEG || last_cmp == ZERO)
					{
						current += addr - 1;
					}
				}
			}
			break;

			case OpCode::PUSH:
			{
				unsigned int index = (data >> 16) & 0xFF;
				bool bIsTexture = (data & 0x8000) != 0;
				uint16_t addr = data & 0x7FFF;

				if (bIsTexture)
				{
					Value v;
					v.asUInt = m_aTextures[addr]->getNativeHandle();
					parameters.push_back(std::pair<unsigned int, Value>(index, v));
				}
				else
				{
					parameters.push_back(std::pair<unsigned int, Value>(index, m_aValues[addr]));
				}
			}
			break;

			case OpCode::CALL:
			{
				uint8_t mode = (data >> 16) & 0xF;
				uint16_t addr = data & 0xFFFF;

				if (mode == 0) // Pass
				{
					Pass * pass = m_aPass[addr];

					pass->begin();
					pass->render(parameters);
					pass->end();
				}
				else
				{
					assert(false);
				}

				parameters.clear();
			}
			break;

			default:
			{
				assert(false);
			}
		}

		if (current >= m_aInstruction.size() - 1)
		{
			bRunning = false;
		}
		else
		{
			++current;
		}
	}

	return true;
}

/**
 * @brief Instance::getRenderTexture
 * @return
 */
unsigned int Instance::getRenderTexture(const char * name) const
{
	auto it = m_mapTextures.find(name);

	if (it != m_mapTextures.end())
	{
		return m_aTextures[it->second]->getNativeHandle();
	}

	return 0;
}

/**
 * @brief Constructor
 * @param instructions
 * @param passes
 * @param framebuffers
 * @param textures
 * @param mapTextures
 * @param defaultFramebuffer
 */
InstanceWithExternalFramebuffer::InstanceWithExternalFramebuffer(const std::vector<Instruction> & instructions, const std::vector<Pass*> & passes, const std::vector<Framebuffer*> & framebuffers, const std::vector<Texture*> & textures, const std::vector<Value> & values,  const std::map<std::string, unsigned int> & mapTextures, unsigned int /*GLuint*/ defaultFramebuffer)
	: Instance (instructions, passes, framebuffers, textures, values, mapTextures),
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
 * @param instructions
 * @param passes
 * @param framebuffers
 * @param textures
 * @param mapTextures
 * @param pDefaultFramebuffer
 */
InstanceWithInternalFramebuffer::InstanceWithInternalFramebuffer(const std::vector<Instruction> & instructions, const std::vector<Pass*> & passes, const std::vector<Framebuffer*> & framebuffers, const std::vector<Texture*> & textures, const std::vector<Value> & values,  const std::map<std::string, unsigned int> & mapTextures, Framebuffer * pDefaultFramebuffer)
	: Instance (instructions, passes, framebuffers, textures, values, mapTextures),
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
