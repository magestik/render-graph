#pragma once

#include <vector>

namespace RenderGraph
{

class Texture;
class Pass;

class Framebuffer
{
public:

	Framebuffer(const std::vector<Texture*> & textures, Pass * pass);
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
	Pass * pass;

	unsigned int currentWidth;
	unsigned int currentHeight;

	unsigned int /*GLuint*/ framebufferId;
};

}
