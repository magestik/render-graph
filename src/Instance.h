#pragma once

#include <vector>

#include "VM.h"

namespace RenderGraph
{

class Framebuffer;
class Texture;

class Pass;

class Instance
{
public:

	Instance(const std::vector<uint8_t> & bytecode, const std::vector<Pass*> & passes, const std::vector<Framebuffer*> & framebuffers, const std::vector<Texture*> & textures, const std::vector<Value> & values);
	virtual ~Instance(void);

	bool resize(unsigned int width, unsigned int height);

	bool execute(void);

	unsigned int getRenderTexture(unsigned int index) const;

	void setConstant(unsigned int index, unsigned int value);
	void setConstant(unsigned int index, int value);
	void setConstant(unsigned int index, float value);
	void setConstant(unsigned int index, bool value);

	virtual unsigned int getDefaultFramebuffer(void) const = 0;

private:

	std::vector<Value> m_aValues;
	std::vector<Texture*> m_aTextures;
	std::vector<Framebuffer*> m_aFramebuffers;
	std::vector<Pass*> m_aPass;

	std::vector<uint8_t> m_bytecode;
};

class InstanceWithExternalFramebuffer : public Instance
{
public:

	InstanceWithExternalFramebuffer(const std::vector<uint8_t> & bytecode, const std::vector<Pass*> & passes, const std::vector<Framebuffer*> & framebuffers, const std::vector<Texture*> & textures, const std::vector<Value> & values, unsigned int defaultFramebuffer);
	virtual ~InstanceWithExternalFramebuffer(void) override;

	virtual unsigned int getDefaultFramebuffer(void) const override;

private:

	const unsigned int m_iDefaultFramebuffer; /*GLuint*/
};

class InstanceWithInternalFramebuffer : public Instance
{
public:

	InstanceWithInternalFramebuffer(const std::vector<uint8_t> & bytecode, const std::vector<Pass*> & passes, const std::vector<Framebuffer*> & framebuffers, const std::vector<Texture*> & textures, const std::vector<Value> & values, Framebuffer * pDefaultFramebuffer);
	virtual ~InstanceWithInternalFramebuffer(void) override;

	virtual unsigned int getDefaultFramebuffer(void) const override;

private:

	const Framebuffer * m_pDefaultFramebuffer; /*GLuint*/
};

}
