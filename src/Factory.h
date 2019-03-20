#pragma once

#include <vector>
#include <map>
#include <string>

class Graph;

namespace RenderGraph
{

class Operation;
class Instance;

typedef Operation* (*OperationFactory)(void);

class Factory
{
public:

	Factory(void);
	~Factory(void);

	bool			registerOperation			(const char * identifier, OperationFactory factory);

	Instance *		createInstanceFromGraph		(const Graph & graph) const;
	Instance *		createInstanceFromGraph		(const Graph & graph, std::map<std::string, unsigned int> & mapTextures, std::map<std::string, std::vector<unsigned int>> & mapValues) const;
	Instance *		createInstanceFromGraph		(const Graph & graph, unsigned int /*GLuint*/ defaultFramebuffer) const;
	Instance *		createInstanceFromGraph		(const Graph & graph, unsigned int /*GLuint*/ defaultFramebuffer, std::map<std::string, unsigned int> & mapTextures, std::map<std::string, std::vector<unsigned int>> & mapValues) const;
	void			destroyInstance				(Instance * queue) const;

protected:

	Instance *		createInstanceFromGraph		(const Graph & graph, std::map<std::string, unsigned int> & mapTextures, std::map<std::string, std::vector<unsigned int>> & mapValues, bool bUseDefaultFramebuffer, unsigned int /*GLuint*/ defaultFramebuffer = 0) const;

	Operation *		createOperation				(const char * identifier) const;
	void			destroyOperation			(Operation * operation) const;

private:

	std::map<std::string, OperationFactory> m_factories;
};

}
