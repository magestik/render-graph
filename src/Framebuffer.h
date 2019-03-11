#pragma once

#include <vector>

namespace RenderGraph
{

class Texture;

class Framebuffer
{
public:

	Framebuffer(const std::vector<Texture*> & textures);
	~Framebuffer(void);

	bool resize(unsigned int width, unsigned int height);

	unsigned int getWidth(void) const;
	unsigned int getHeight(void) const;

	unsigned int /*GLuint*/ getNativeHandle(void) const
	{
		return(framebufferId);
	}

private:

	const std::vector<Texture*> textures;

	unsigned int currentWidth;
	unsigned int currentHeight;

	unsigned int /*GLuint*/ framebufferId;
};

}
