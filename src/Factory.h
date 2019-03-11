#pragma once

#include <vector>

class Graph;

namespace RenderGraph
{

class Pass;
class Instance;

typedef Pass* (*PassFactory)(void);

class Factory
{
public:

	Factory(void);
	~Factory(void);

	bool			registerPass				(const char * identifier, PassFactory factory);

	Instance *		createInstanceFromGraph		(const Graph & graph) const;
	Instance *		createInstanceFromGraph		(const Graph & graph, unsigned int /*GLuint*/ defaultFramebuffer) const;
	void			destroyInstance				(Instance * queue) const;

protected:

	Instance *		createInstanceFromGraph		(const Graph & graph, bool bUseDefaultFramebuffer, unsigned int /*GLuint*/ defaultFramebuffer = 0) const;

	Pass *			createPass					(const char * identifier) const;
	void			destroyPass					(Pass * pass) const;

private:

	struct PassDescriptor
	{
		const char * identifier;
		PassFactory factory;
	};

	std::vector<PassDescriptor> m_descriptors;
};

}
