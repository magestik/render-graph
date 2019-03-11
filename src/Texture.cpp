#include "Texture.h"

#define GL_GLEXT_PROTOTYPES 1
#include <GL/glcorearb.h>

#include <assert.h>

namespace RenderGraph
{

/**
 * @brief Constructor
 * @param format
 */
Texture::Texture(TextureFormat format_) : format(format_), currentWidth(0), currentHeight(0)
{
	textureId = 0;
}

/**
 * @brief Destructor
 */
Texture::~Texture(void)
{
	if (textureId != 0)
	{
		glDeleteTextures(1, &textureId);
		textureId = 0;
	}
}

/**
 * @brief Resize texture
 * @param width
 * @param height
 * @return
 */
bool Texture::resize(unsigned int width, unsigned int height)
{
	if (textureId != 0)
	{
		glDeleteTextures(1, &textureId);
		textureId = 0;
	}

	glGenTextures(1, &textureId);

	glBindTexture(GL_TEXTURE_2D, textureId);

#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wswitch"

	switch (format)
	{
		case UNKNOWN:
		{
			assert(false);
		}
		break;

		case R8:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
		}
		break;

		case RG8:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, width, height, 0, GL_RG, GL_UNSIGNED_BYTE, nullptr);
		}
		break;

		case RGBA8:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		}
		break;

		case R16:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, width, height, 0, GL_RED, GL_UNSIGNED_SHORT, nullptr);
		}
		break;

		case RG16:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16, width, height, 0, GL_RG, GL_UNSIGNED_SHORT, nullptr);
		}
		break;

		case RGBA16:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, width, height, 0, GL_RGBA, GL_UNSIGNED_SHORT, nullptr);
		}
		break;

		case R16F:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, width, height, 0, GL_RED, GL_FLOAT, nullptr);
		}
		break;

		case RG16F:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, width, height, 0, GL_RG, GL_FLOAT, nullptr);
		}
		break;

		case RGBA16F:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
		}
		break;

		case R32F:
		case RG32F:
		case RGBA32F:
		case R8I:
		case RG8I:
		case RGBA8I:
		case R16I:
		case RG16I:
		case RGBA16I:
		case R32I:
		case RG32I:
		case RGBA32I:
		case R8UI:
		case RG8UI:
		case RGBA8UI:
		case R16UI:
		case RG16UI:
		case RGBA16UI:
		case R32UI:
		case RG32UI:
		case RGBA32UI:
		case RGB10_A2:
		case RGB10_A2UI:
		case R11F_G11F_B10F:
		case SRGB8_ALPHA8:
		{
			assert(false); // TODO
		}
		break;

		case DEPTH_COMPONENT16:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT16, GL_FLOAT, nullptr);
		}
		break;

		case DEPTH_COMPONENT24:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT24, GL_FLOAT, nullptr);
		}
		break;

		case DEPTH_COMPONENT32F:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		}
		break;

		case DEPTH24_STENCIL8:
		case DEPTH32F_STENCIL8:
		case STENCIL_INDEX8:
		{
			assert(false); // TODO
		}
		break;
	}

#pragma GCC diagnostic pop

	assert(glGetError() == GL_NO_ERROR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);

	currentWidth = width;
	currentHeight = height;

	return true;
}

/**
 * @brief Texture::isColorTexture
 * @return
 */
bool Texture::isColorTexture(void) const
{
	return !isDepthTexture() && !isStencilTexture();
}

/**
 * @brief Texture::isDepthTexture
 * @return
 */
bool Texture::isDepthTexture(void) const
{
	return format == DEPTH_COMPONENT16 || format == DEPTH_COMPONENT24 || format == DEPTH_COMPONENT32F;
}

/**
 * @brief Texture::isStencilTexture
 * @return
 */
bool Texture::isStencilTexture(void) const
{
	return format == STENCIL_INDEX8;
}

/**
 * @brief Texture::isDepthStencilTexture
 * @return
 */
bool Texture::isDepthStencilTexture(void) const
{
	return format == DEPTH24_STENCIL8 || format == DEPTH32F_STENCIL8;
}

/**
 * @brief Get current width
 * @return
 */
unsigned int Texture::getWidth(void) const
{
	return currentWidth;
}

/**
 * @brief Get current height
 * @return
 */
unsigned int Texture::getHeight(void) const
{
	return currentHeight;
}

}
