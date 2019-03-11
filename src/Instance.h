#pragma once

#include <vector>
#include <map>
#include <string>

#include "VM.h"

namespace RenderGraph
{

class Framebuffer;
class Texture;

class Pass;

class Instance
{
public:

	Instance(const std::vector<Instruction> & instructions, const std::vector<Pass*> & passes, const std::vector<Framebuffer*> & framebuffers, const std::vector<Texture*> & textures, const std::vector<Value> & values, const std::map<std::string, unsigned int> & mapTextures);
	virtual ~Instance(void);

	bool resize(unsigned int width, unsigned int height);

	bool execute(void);

	unsigned int getRenderTexture(const char * name) const;

	virtual unsigned int getDefaultFramebuffer(void) const = 0;

private:

	std::vector<Value> m_aValues;
	std::vector<Texture*> m_aTextures;
	std::vector<Framebuffer*> m_aFramebuffers;
	std::vector<Pass*> m_aPass;

	std::vector<Instruction> m_aInstruction;

	std::map<std::string, unsigned int> m_mapTextures; // TODO : remove this
};

class InstanceWithExternalFramebuffer : public Instance
{
public:

	InstanceWithExternalFramebuffer(const std::vector<Instruction> & instructions, const std::vector<Pass*> & passes, const std::vector<Framebuffer*> & framebuffers, const std::vector<Texture*> & textures, const std::vector<Value> & values, const std::map<std::string, unsigned int> & mapTextures, unsigned int defaultFramebuffer);
	virtual ~InstanceWithExternalFramebuffer(void) override;

	virtual unsigned int getDefaultFramebuffer(void) const override;

private:

	const unsigned int m_iDefaultFramebuffer; /*GLuint*/
};

class InstanceWithInternalFramebuffer : public Instance
{
public:

	InstanceWithInternalFramebuffer(const std::vector<Instruction> & instructions, const std::vector<Pass*> & passes, const std::vector<Framebuffer*> & framebuffers, const std::vector<Texture*> & textures, const std::vector<Value> & values, const std::map<std::string, unsigned int> & mapTextures, Framebuffer * pDefaultFramebuffer);
	virtual ~InstanceWithInternalFramebuffer(void) override;

	virtual unsigned int getDefaultFramebuffer(void) const override;

private:

	const Framebuffer * m_pDefaultFramebuffer; /*GLuint*/
};

}
