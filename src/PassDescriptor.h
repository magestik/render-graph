#pragma once

namespace RenderGraph
{

class Pass;

typedef Pass* (*PassFactory)(void);

struct PassDescriptor
{
	const char * identifier;
	const char * name;
	const char * description;
	PassFactory factory;
};

}
