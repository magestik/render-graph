#include "Framebuffer.h"

#include "Texture.h"

#include "Pass.h"

#include "OpenGL.h"

#include <assert.h>

namespace RenderGraph
{

/**
 * @brief Constructor
 * @param textures
 */
Framebuffer::Framebuffer(const std::vector<Texture*> & textures_, Pass * pass_) : textures(textures_), pass(pass_), currentWidth(0), currentHeight(0)
{
	framebufferId = 0;
}

/**
 * @brief Destructor
 */
Framebuffer::~Framebuffer(void)
{
	if (framebufferId != 0)
	{
		glDeleteFramebuffers(1, &framebufferId);
		framebufferId = 0;
	}
}

/**
 * @brief Resize framebuffer
 * @param width
 * @param height
 * @return
 */
bool Framebuffer::resize(unsigned int width, unsigned int height)
{
	if (framebufferId != 0)
	{
		glDeleteFramebuffers(1, &framebufferId);
		framebufferId = 0;
	}

	glGenFramebuffers(1, &framebufferId);

	assert(framebufferId != 0);

	glBindFramebuffer(GL_FRAMEBUFFER, framebufferId);

	unsigned int colorAttachmentIndex = 0;

	for (Texture * texture : textures)
	{
		assert(texture->getWidth() == width);
		assert(texture->getHeight() == height);

		if (texture->isDepthTexture())
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture->getNativeHandle(), 0);
		}
		else if (texture->isStencilTexture())
		{
			assert(false); // TODO
		}
		else if (texture->isDepthStencilTexture())
		{
			assert(false); // TODO
		}
		else
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colorAttachmentIndex, GL_TEXTURE_2D, texture->getNativeHandle(), 0);
			++colorAttachmentIndex;
		}
	}

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	currentWidth = width;
	currentHeight = height;

	pass->setFramebuffer(framebufferId, width, height);

	return true;
}

/**
 * @brief Get current width
 * @return
 */
unsigned int Framebuffer::getWidth(void) const
{
	return currentWidth;
}

/**
 * @brief Get current height
 * @return
 */
unsigned int Framebuffer::getHeight(void) const
{
	return currentHeight;
}

}
