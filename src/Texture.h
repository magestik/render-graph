#pragma once

#include "Formats.h"

namespace RenderGraph
{

class Texture
{

public:

	Texture(TextureFormat format);
	~Texture(void);

	bool resize(unsigned int width, unsigned int height);

	bool isColorTexture(void) const;
	bool isDepthTexture(void) const;
	bool isStencilTexture(void) const;
	bool isDepthStencilTexture(void) const;

	unsigned int getWidth(void) const;
	unsigned int getHeight(void) const;

	unsigned int /*GLuint*/ getNativeHandle(void) const
	{
		return(textureId);
	}

private:

	TextureFormat format;

	unsigned int currentWidth;
	unsigned int currentHeight;

	unsigned int /*GLuint*/ textureId;
};

}
