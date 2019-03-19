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

#include "OpenGL.h"

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

static const char * INSTRUCTION_NAMES [] =
{
	"NOP",
	"PUSH",
	"POP",
	"ADD",
	"SUB",
	"MUL",
	"DIV",
	"EQ",
	"NEQ",
	"GT",
	"GTE",
	"LT",
	"LTE",
	"JMP",
	"CALL",
	"HALT"
};

static inline void genOperatorBytecode(RenderGraph::OpCode opcode, unsigned int numParams, const Graph & graph, Node * node, const std::map<std::string, unsigned int> & mapValues, std::vector<uint8_t> & bytecode)
{
	uint16_t parameters [numParams];

	std::vector<Edge*> inEdges;
	graph.getEdgeTo(node, inEdges);

	assert(inEdges.size() == numParams);

	for (Edge * edge : inEdges)
	{
		Node * source = edge->getSource();

		const std::string & strId = source->getId();

		const std::string & target_id = edge->getMetaData("target_id");
		int paramIndex = atoi(target_id.c_str());
		assert(paramIndex < UINT8_MAX);

		assert(source->getType() != "present" && source->getType() != "pass" && source->getType() != "texture");
		{
			auto it = mapValues.find(strId);

			if (it != mapValues.end())
			{
				unsigned int index = it->second;
				uint16_t addr = (0 << 15) | (index & 0x7FFF);
				parameters[paramIndex] = addr;
			}
		}
	}

	for (int i = 0; i < numParams; ++i)
	{
		bytecode.push_back(uint8_t(RenderGraph::OpCode::PUSH));
		bytecode.push_back(uint8_t((parameters[i] >> 8) & 0xFF));
		bytecode.push_back(uint8_t((parameters[i]) & 0xFF));

		printf("PUSH %d\n", parameters[i]);
	}

	bytecode.push_back(uint8_t(opcode));
	bytecode.push_back(uint8_t(2)); // float

	printf("%s (float)\n", INSTRUCTION_NAMES[uint8_t(opcode)]);

	auto it = mapValues.find(node->getId());

	if (it != mapValues.end())
	{
		unsigned int index = it->second;
		uint16_t addr = (index & 0xFFFF);

		bytecode.push_back(uint8_t(RenderGraph::OpCode::POP));
		bytecode.push_back(uint8_t((addr >> 8) & 0xFF));
		bytecode.push_back(uint8_t((addr) & 0xFF));

		printf("POP %d\n", addr);
	}
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
	std::map<std::string, unsigned int> mapTextures;
	std::map<std::string, unsigned int> mapValues;

	Instance * pInstance = createInstanceFromGraph(graph, mapTextures, mapValues, false);

	assert(pInstance != nullptr);

	return pInstance;
}

/**
 * @brief Factory::createInstanceFromGraph
 * @param graph
 * @return
 */
Instance * Factory::createInstanceFromGraph(const Graph & graph, std::map<std::string, unsigned int> & mapTextures, std::map<std::string, unsigned int> & mapValues) const
{
	Instance * pInstance = createInstanceFromGraph(graph, mapTextures, mapValues, false);

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
	std::map<std::string, unsigned int> mapTextures;
	std::map<std::string, unsigned int> mapValues;

	Instance * pInstance = createInstanceFromGraph(graph, mapTextures, mapValues, true, defaultFramebuffer);

	assert(pInstance != nullptr);

	return pInstance;
}

/**
 * @brief Factory::createInstanceFromGraph
 * @param graph
 * @param defaultFramebuffer
 * @return
 */
Instance * Factory::createInstanceFromGraph(const Graph & graph, unsigned int /*GLuint*/ defaultFramebuffer, std::map<std::string, unsigned int> & mapTextures, std::map<std::string, unsigned int> & mapValues) const
{
	Instance * pInstance = createInstanceFromGraph(graph, mapTextures, mapValues, true, defaultFramebuffer);

	assert(pInstance != nullptr);

	return pInstance;
}

/**
 * @brief Factory::createInstanceFromGraph
 * @param graph
 * @return
 */
Instance * Factory::createInstanceFromGraph(const Graph & graph, std::map<std::string, unsigned int> & mapTextures, std::map<std::string, unsigned int> & mapValues, bool bUseDefaultFramebuffer, unsigned int /*GLuint*/ defaultFramebuffer) const
{
	mapTextures.clear();
	mapValues.clear();

	Node * pNodePresent = nullptr;
	std::vector<Node*> aNodesTexture;
	std::vector<Node*> aNodesFloat;
	std::vector<Node*> aNodesPass;
	std::vector<Node*> aNodesOperator;

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
		else if (node->getType() == "pass")
		{
			aNodesPass.push_back(node);
		}
		else if (node->getType() == "float")
		{
			aNodesFloat.push_back(node);
		}
		else if (node->getType() == "addition")
		{
			aNodesOperator.push_back(node);
		}
		else if (node->getType() == "subtraction")
		{
			aNodesOperator.push_back(node);
		}
		else if (node->getType() == "multiplication")
		{
			aNodesOperator.push_back(node);
		}
		else if (node->getType() == "division")
		{
			aNodesOperator.push_back(node);
		}
		else if (node->getType() == "negation")
		{
			aNodesOperator.push_back(node);
		}
		else if (node->getType() == "absolute")
		{
			aNodesOperator.push_back(node);
		}
		else if (node->getType() == "equal_to")
		{
			aNodesOperator.push_back(node);
		}
		else if (node->getType() == "not_equal_to")
		{
			aNodesOperator.push_back(node);
		}
		else if (node->getType() == "greater_than")
		{
			aNodesOperator.push_back(node);
		}
		else if (node->getType() == "greater_than_or_equal_to")
		{
			aNodesOperator.push_back(node);
		}
		else if (node->getType() == "less_than")
		{
			aNodesOperator.push_back(node);
		}
		else if (node->getType() == "less_than_or_equal_to")
		{
			aNodesOperator.push_back(node);
		}
		else if (node->getType() == "not")
		{
			aNodesOperator.push_back(node);
		}
		else if (node->getType() == "and")
		{
			aNodesOperator.push_back(node);
		}
		else if (node->getType() == "or")
		{
			aNodesOperator.push_back(node);
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

	for (Node * node : aNodesFloat)
	{
		const std::string & strId = node->getId();
		const std::string & strValue = node->getMetaData("value");

		unsigned int index = values.size();

		mapValues.insert(std::pair<std::string, unsigned int>(strId, index));

		RenderGraph::Value value;
		value.asFloat = atof(strValue.c_str());
		values.push_back(value);

		printf("CONSTANT[%d] = %f\n", index, value.asFloat);
	}

	// ----------------------------------------------------------------------------------------

	for (Node * node : aNodesOperator)
	{
		const std::string & strId = node->getId();

		unsigned int index = values.size();

		mapValues.insert(std::pair<std::string, unsigned int>(strId, index));

		RenderGraph::Value value;
		value.asFloat = 0.0f;
		values.push_back(value);

		printf("VARIABLE[%d] = 0\n", index);
	}

	// ----------------------------------------------------------------------------------------

	std::vector<Texture*> textures;
	textures.reserve(aNodesTexture.size());

	for (Node * node : aNodesTexture)
	{
		if (!bUseDefaultFramebuffer || node != pDefaultFramebufferNode)
		{
			const std::string & strId = node->getId();
			const std::string & strFormat = node->getMetaData("format");

			unsigned int index = textures.size();

			TextureFormat format = strToFormat(strFormat.c_str());

			Texture * texture = new Texture(format);

			mapTextures.insert(std::pair<std::string, unsigned int>(strId, index));

			textures.push_back(texture);

			printf("TEXTURE[%d] = %" PRIXPTR "\n", index, (uintptr_t)texture);
		}
	}

	// ----------------------------------------------------------------------------------------

	Framebuffer * pDefaultFramebuffer = nullptr;

	std::vector<Framebuffer*> framebuffers;
	framebuffers.reserve(aNodesPass.size());

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
	}

	// ----------------------------------------------------------------------------------------

	std::vector<Pass*> passes;
	passes.reserve(aNodesPass.size());

	std::map<std::string, unsigned int> mapPasses;

	for (Node * node : aNodesPass)
	{
		const std::string & strId = node->getId();
		const std::string & strSubType = node->getMetaData("subtype");

		Pass * pass = createPass(strSubType.c_str());

		mapPasses.insert(std::pair<std::string, unsigned int>(strId, passes.size()));

		passes.push_back(pass);
	}

	// ----------------------------------------------------------------------------------------

	std::vector<uint8_t> bytecode;

	for (std::vector<Node*>::reverse_iterator it = queue.rbegin(); it != queue.rend(); ++it)
	{
		Node * node = *it;

		const std::string & strCurrentNodeId = node->getId();

		if (node->getType() == "addition")
		{
			genOperatorBytecode(OpCode::ADD, 2, graph, node, mapValues, bytecode);
		}
		else if (node->getType() == "subtraction")
		{
			genOperatorBytecode(OpCode::SUB, 2, graph, node, mapValues, bytecode);
		}
		else if (node->getType() == "multiplication")
		{
			genOperatorBytecode(OpCode::MUL, 2, graph, node, mapValues, bytecode);
		}
		else if (node->getType() == "division")
		{
			genOperatorBytecode(OpCode::DIV, 2, graph, node, mapValues, bytecode);
		}
		else if (node->getType() == "negation")
		{
			genOperatorBytecode(OpCode::NEG, 1, graph, node, mapValues, bytecode);
		}
		else if (node->getType() == "absolute")
		{
			genOperatorBytecode(OpCode::ABS, 1, graph, node, mapValues, bytecode);
		}
		else if (node->getType() == "equal")
		{
			genOperatorBytecode(OpCode::EQ, 2, graph, node, mapValues, bytecode);
		}
		else if (node->getType() == "not_equal")
		{
			genOperatorBytecode(OpCode::NEQ, 2, graph, node, mapValues, bytecode);
		}
		else if (node->getType() == "greater_than")
		{
			genOperatorBytecode(OpCode::GT, 2, graph, node, mapValues, bytecode);
		}
		else if (node->getType() == "greater_than_or_equal")
		{
			genOperatorBytecode(OpCode::GTE, 2, graph, node, mapValues, bytecode);
		}
		else if (node->getType() == "less_than")
		{
			genOperatorBytecode(OpCode::LT, 2, graph, node, mapValues, bytecode);
		}
		else if (node->getType() == "less_than_or_equal")
		{
			genOperatorBytecode(OpCode::LTE, 2, graph, node, mapValues, bytecode);
		}
		else if (node->getType() == "not")
		{
			genOperatorBytecode(OpCode::NOT, 1, graph, node, mapValues, bytecode);
		}
		else if (node->getType() == "and")
		{
			genOperatorBytecode(OpCode::AND, 2, graph, node, mapValues, bytecode);
		}
		else if (node->getType() == "or")
		{
			genOperatorBytecode(OpCode::OR, 2, graph, node, mapValues, bytecode);
		}
		else if (node->getType() == "pass")
		{
			std::vector<uint16_t> parameters;

			//
			// Get parameters
			std::vector<Edge*> inEdges;
			graph.getEdgeTo(node, inEdges);

			parameters.resize(inEdges.size());

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
						uint16_t addr = (1 << 15) | (index & 0x7FFF);
						parameters[paramIndex] = addr;
					}
				}
				else
				{
					assert(source->getType() != "present" && source->getType() != "pass" && source->getType() != "texture");

					auto it = mapValues.find(strId);

					if (it != mapValues.end())
					{
						unsigned int index = it->second;
						uint16_t addr = (0 << 15) | (index & 0x7FFF);
						parameters[paramIndex] = addr;
					}
				}
			}

			assert(parameters.size() < UINT8_MAX);

			//
			// Compile parameters
			for (int i = 0; i < parameters.size(); ++i)
			{
				bytecode.push_back(uint8_t(OpCode::PUSH));
				bytecode.push_back(uint8_t((parameters[i] >> 8) & 0xFF));
				bytecode.push_back(uint8_t((parameters[i]) & 0xFF));

				printf("PUSH %d\n", parameters[i]);
			}

			//
			// Call
			auto it = mapPasses.find(strCurrentNodeId);

			if (it != mapPasses.end())
			{
				unsigned int index = it->second;
				uint16_t addr = (index & 0xFFFF);

				bytecode.push_back(uint8_t(OpCode::CALL));
				bytecode.push_back(0 << 16); // Pass
				bytecode.push_back(uint8_t(parameters.size()));
				bytecode.push_back(uint8_t((addr >> 8) & 0xFF));
				bytecode.push_back(uint8_t((addr) & 0xFF));

				printf("CALL (PASS) %d\n", index);
			}
			else
			{
				assert(false);
			}
		}
	}

	if (bytecode.size() == 0)
	{
		return nullptr;
	}

	bytecode.push_back(uint8_t(OpCode::HALT));
	printf("HALT\n");

	fflush(stdout);

	Instance * pRenderGraph = nullptr;

	if (bUseDefaultFramebuffer)
	{
		pRenderGraph = new InstanceWithExternalFramebuffer(bytecode, passes, framebuffers, textures, values, defaultFramebuffer);
	}
	else
	{
		pRenderGraph = new InstanceWithInternalFramebuffer(bytecode, passes, framebuffers, textures, values, pDefaultFramebuffer);
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
