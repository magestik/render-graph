#include "Factory.h"

#include "Pass.h"
#include "Instance.h"

#include "Texture.h"
#include "Framebuffer.h"

#include "DefaultPass.h"

#include "Graph.h"
#include "Node.h"
#include "Edge.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <algorithm>

#define GL_GLEXT_PROTOTYPES 1
#include <GL/glcorearb.h>

static void createGraphAlias(const Graph & graph, Graph & outGraph)
{
//	outGraph.setDirected(graph.isDirected());
//	outGraph.setLabel(graph.getLabel());
//	outGraph.setType(graph.getType());

	for (Node * node : graph.getNodes())
	{
		outGraph.addNode(node);
	}

	for (Edge * edge : graph.getEdges())
	{
		outGraph.addEdge(edge);
	}
}

static bool ApplyTopologicalOrdering(const Graph & graph, std::vector<Node*> & L, Node * n)
{
	Graph G;

	createGraphAlias(graph, G);

	std::vector<Node*> S; S.push_back(n); // S ← Set of all nodes with no incoming edges

	while (!S.empty())
	{
		Node * n = S.back(); S.pop_back(); // remove a node n from S

		L.push_back(n); // add n to tail of L

		std::vector<Edge*> edges;
		G.getEdgeTo(n, edges);

		for (Edge * e : edges)
		{
			Node * m = e->getSource(); // for each node m with an edge e from n to m do

			G.removeEdge(e); // remove edge e from the graph

			std::vector<Edge*> tmp;
			G.getEdgeFrom(m, tmp);

			if (tmp.empty()) //	if m has no other incoming edges then
			{
				S.push_back(m); // insert m into S
			}
		}
	}

	//
	// Check for cycles
	{
		std::vector<Edge*> edges = G.getEdges();

		if (!edges.empty()) // (graph has edges)
		{
			return(false); // error (graph has at least one cycle)
		}
	}

	return(true);
}

namespace RenderGraph
{

/**
 * @brief Default constructor
 */
Factory::Factory(void)
{
	// ...
}

/**
 * @brief Destructor
 */
Factory::~Factory(void)
{
	// ...
}

/**
 * @brief Factory::createInstanceFromGraph
 * @param graph
 * @return
 */
Instance * Factory::createInstanceFromGraph(const Graph & graph) const
{
	Instance * pInstance = createInstanceFromGraph(graph, false);

	assert(pInstance != nullptr);

	return pInstance;
}

/**
 * @brief Factory::createInstanceFromGraph
 * @param graph
 * @param defaultFramebuffer
 * @return
 */
Instance * Factory::createInstanceFromGraph(const Graph & graph, unsigned int /*GLuint*/ defaultFramebuffer) const
{
	Instance * pInstance = createInstanceFromGraph(graph, true, defaultFramebuffer);

	assert(pInstance != nullptr);

	return pInstance;
}

/**
 * @brief Factory::createInstanceFromGraph
 * @param graph
 * @return
 */
Instance * Factory::createInstanceFromGraph(const Graph & graph, bool bUseDefaultFramebuffer, unsigned int /*GLuint*/ defaultFramebuffer) const
{
	Node * pNodePresent = nullptr;
	std::vector<Node*> aNodesTexture;
	std::vector<Node*> aNodesFloat;
	std::vector<Node*> aNodesPass;
	std::vector<Node*> aNodesComparison;

	for (Node * node : graph.getNodes())
	{
		if (node->getType() == "present")
		{
			assert(nullptr == pNodePresent);
			pNodePresent = node;
		}
		else if (node->getType() == "texture")
		{
			aNodesTexture.push_back(node);
		}
		else if (node->getType() == "float")
		{
			aNodesFloat.push_back(node);
		}
		else if (node->getType() == "pass")
		{
			aNodesPass.push_back(node);
		}
		else if (node->getType() == "comparison")
		{
			aNodesComparison.push_back(node);
		}
		else
		{
			assert(false);
		}
	}

	if (nullptr == pNodePresent)
	{
		assert(false);
		return nullptr;
	}

	// ----------------------------------------------------------------------------------------

	// TODO : remove nodes that can't reach pNodePresent

	// ----------------------------------------------------------------------------------------

	std::vector<Node*> queue;
	queue.reserve(aNodesPass.size());

	if (!ApplyTopologicalOrdering(graph, queue, pNodePresent))
	{
		return(nullptr);
	}

	// ----------------------------------------------------------------------------------------

	std::vector<Edge*> edges;
	graph.getEdgeTo(pNodePresent, edges);

	if (1 != edges.size())
	{
		return(nullptr);
	}

	Node * pDefaultFramebufferNode = edges[0]->getSource();

	std::string strFinalTextureId = pDefaultFramebufferNode->getId();

	// ----------------------------------------------------------------------------------------

	std::vector<RenderGraph::Value> values;
	values.reserve(aNodesFloat.size());

	std::map<std::string, unsigned int> mapValues; // node identifier -> VM addr

	for (Node * node : aNodesFloat)
	{
		const std::string & strId = node->getId();
		const std::string & strValue = node->getMetaData("value");

		mapValues.insert(std::pair<std::string, unsigned int>(strId, values.size()));

		RenderGraph::Value value;
		value.asFloat = atof(strValue.c_str());
		values.push_back(value);
	}

	// ----------------------------------------------------------------------------------------

	std::vector<Texture*> textures;
	textures.reserve(aNodesTexture.size());

	std::map<std::string, unsigned int> mapTextures; // node identifier -> VM addr

	for (Node * node : aNodesTexture)
	{
		if (!bUseDefaultFramebuffer || node != pDefaultFramebufferNode)
		{
			const std::string & strId = node->getId();
			const std::string & strFormat = node->getMetaData("format");

			TextureFormat format = strToFormat(strFormat.c_str());

			Texture * texture = new Texture(format);

			mapTextures.insert(std::pair<std::string, unsigned int>(strId, textures.size()));

			textures.push_back(texture);
		}
	}

	// ----------------------------------------------------------------------------------------

	Framebuffer * pDefaultFramebuffer = nullptr;

	std::vector<Framebuffer*> framebuffers;
	framebuffers.reserve(aNodesPass.size());

	std::map<std::string, Framebuffer*> mapFramebuffers;

	for (Node * node : aNodesPass)
	{
		const std::string & strId = node->getId();
		const std::string & strSubType = node->getMetaData("subtype");

		Framebuffer * framebuffer = nullptr;

		std::vector<Edge*> outEdges;
		graph.getEdgeFrom(node, outEdges);

		if (bUseDefaultFramebuffer && graph.hasEdge(node, pDefaultFramebufferNode))
		{
			assert(outEdges.size() == 1);
		}
		else
		{
			std::vector<Texture*> fbTextures;

			for (Edge * edge : outEdges)
			{
				Node * target = edge->getTarget();
				assert(target->getType() == "texture");

				const std::string & strId = target->getId();

				auto it = mapTextures.find(strId);

				if (it != mapTextures.end())
				{
					fbTextures.push_back(textures[it->second]);
				}
				else
				{
					assert(false);
					fbTextures.push_back(nullptr);
				}
			}

			framebuffer = new Framebuffer(fbTextures);

			if (graph.hasEdge(node, pDefaultFramebufferNode))
			{
				pDefaultFramebuffer = framebuffer;
			}
		}

		framebuffers.push_back(framebuffer);

		mapFramebuffers.insert(std::pair<std::string, Framebuffer*>(strId, framebuffer));
	}

	// ----------------------------------------------------------------------------------------

	std::vector<Pass*> passes;
	passes.reserve(aNodesPass.size());

	std::map<std::string, Pass*> mapPasses;

	for (Node * node : aNodesPass)
	{
		const std::string & strId = node->getId();
		const std::string & strSubType = node->getMetaData("subtype");

		Pass * pass = createPass(strSubType.c_str());

		passes.push_back(pass);

		mapPasses.insert(std::pair<std::string, Pass*>(strId, pass));
	}

	// ----------------------------------------------------------------------------------------

	std::vector<Instruction> instructions;

	for (std::vector<Node*>::reverse_iterator it = queue.rbegin(); it != queue.rend(); ++it)
	{
		Node * node = *it;

		if (node->getType() == "pass")
		{
			//
			// Parameters
			{
				std::vector<Edge*> inEdges;
				graph.getEdgeTo(node, inEdges);

				for (Edge * edge : inEdges)
				{
					Node * source = edge->getSource();

					const std::string & strId = source->getId();

					const std::string & target_id = edge->getMetaData("target_id");
					int paramIndex = atoi(target_id.c_str());
					assert(paramIndex < UINT8_MAX);

					if (source->getType() == "texture")
					{
						auto it = mapTextures.find(strId);

						if (it != mapTextures.end())
						{
							unsigned int index = it->second;
							Instruction instr = (uint32_t(OpCode::PUSH) << 24) | ((paramIndex & 0xFF) << 16) | (1 << 15) | (index & 0x7FFF);
							instructions.push_back(instr);
							printf("PUSH %d (%d)\n", index, textures[it->second]->getNativeHandle());
						}
					}
					else if (source->getType() == "float")
					{
						auto it = mapValues.find(strId);

						if (it != mapValues.end())
						{
							unsigned int index = it->second;
							Instruction instr = (uint32_t(OpCode::PUSH) << 24) | ((paramIndex & 0xFF) << 16) | (0 << 15) | (index & 0x7FFF);
							instructions.push_back(instr);
							printf("PUSH %d\n", index);
						}
					}
				}
			}

			//
			// Call
			{
				const std::string & strId = node->getId();
				const std::string & strSubType = node->getMetaData("subtype");

				auto it = mapPasses.find(strId);

				if (it != mapPasses.end())
				{
					std::vector<Pass*>::iterator it2 = std::find(passes.begin(), passes.end(), it->second);
					unsigned int index = std::distance(passes.begin(), it2);

					Instruction instr = (uint32_t(OpCode::CALL) << 24) | (0 << 16) | (index & 0xFFFF);
					instructions.push_back(instr);
					printf("CALL %d\n", index);
				}
				else
				{
					assert(false);
				}
			}
		}
	}

	fflush(stdout);

	if (instructions.size() == 0)
	{
		return nullptr;
	}

	Instance * pRenderGraph = nullptr;

	if (bUseDefaultFramebuffer)
	{
		pRenderGraph = new InstanceWithExternalFramebuffer(instructions, passes, framebuffers, textures, values, mapTextures, defaultFramebuffer);
	}
	else
	{
		pRenderGraph = new InstanceWithInternalFramebuffer(instructions, passes, framebuffers, textures, values, mapTextures, pDefaultFramebuffer);
	}

	assert(pRenderGraph != nullptr);

	return(pRenderGraph);
}

/**
 * @brief Factory::destroyQueue
 * @param queue
 */
void Factory::destroyInstance(Instance * queue) const
{
	delete queue;
}

/**
 * @brief Factory::registerPass
 * @param descriptor
 * @return
 */
bool Factory::registerPass(const char * identifier, PassFactory factory)
{
	for (const PassDescriptor & desc : m_descriptors)
	{
		if (!strcmp(desc.identifier, identifier))
		{
			return(false); // already exists
		}
	}

	PassDescriptor descriptor;
	descriptor.identifier = identifier;
	descriptor.factory = factory;

	m_descriptors.push_back(descriptor);

	return(true);
}

/**
 * @brief Factory::createPass
 * @param name
 * @return
 */
Pass * Factory::createPass(const char * identifier) const
{
	for (const PassDescriptor & desc : m_descriptors)
	{
		if (!strcmp(desc.identifier, identifier))
		{
			Pass * pass = desc.factory();
			assert(pass != nullptr);
			pass->init();
			return(pass);
		}
	}

	Pass * pass = new DefaultPass;
	assert(pass != nullptr);
	pass->init();
	return(pass);
}

/**
 * @brief Factory::destroyPass
 * @param pass
 */
void Factory::destroyPass(Pass * pass) const
{
	pass->release();
	delete pass;
}

}
