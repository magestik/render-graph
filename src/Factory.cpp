#include "Factory.h"

#include "Pass.h"
#include "Instance.h"

#include "Texture.h"
#include "Framebuffer.h"

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

	"NEG",
	"ABS",
	"FMA",

	"EQ",
	"NEQ",
	"GT",
	"GTE",
	"LT",
	"LTE",

	"NOT",
	"AND",
	"OR",

	"JMP",
	"JMPT",
	"JMPF",

	"CALL",
	"HALT"
};

static_assert (sizeof(INSTRUCTION_NAMES)/sizeof(INSTRUCTION_NAMES[0]) == uint8_t(RenderGraph::OpCode::HALT), "Missing instruction names");

static inline void genOperatorBytecode(RenderGraph::OpCode opcode, unsigned int numParams, const Graph & graph, Node * node, const std::map<std::string, std::vector<unsigned int>> & mapValues, std::vector<uint8_t> & bytecode)
{
	//
	// Inputs
	const std::vector<uint16_t> inputs = [&]
	{
		std::vector<uint16_t> values;

		std::vector<Edge*> inEdges;
		graph.getEdgeTo(node, inEdges);

		values.resize(numParams);

		for (Edge * edge : inEdges)
		{
			Node * source = edge->getSource();

			const std::string & strId = source->getId();

			const std::string & source_id = edge->getMetaData("source_id");
			int outputParamIndex = atoi(source_id.c_str());
			assert(outputParamIndex < UINT8_MAX);

			const std::string & target_id = edge->getMetaData("target_id");
			int inputParamIndex = atoi(target_id.c_str());
			assert(inputParamIndex < UINT8_MAX);

			assert(source->getType() != "present" && source->getType() != "texture");
			{
				auto it = mapValues.find(strId);

				if (it != mapValues.end())
				{
					unsigned int index = it->second[outputParamIndex];
					uint16_t addr = (0 << 15) | (index & 0x7FFF);
					values[inputParamIndex] = addr;
				}
				else
				{
					assert(false);
				}
			}
		}

		return values;
	} ();

	assert(inputs.size() == numParams);

	//
	// Output
	uint16_t output = [&]
	{
		uint16_t value = 0;

		auto it = mapValues.find(node->getId());

		if (it != mapValues.end())
		{
			assert(it->second.size() == 1); // operators have exactly 1 output
			unsigned int index = it->second[0];
			uint16_t addr = (0 << 15) | (index & 0x7FFF);
			value = addr;
		}
		else
		{
			assert(false);
		}

		return value;
	} ();

	//
	// Gen bytecode
	for (int i = 0; i < numParams; ++i)
	{
		bytecode.push_back(uint8_t(RenderGraph::OpCode::PUSH));
		bytecode.push_back(uint8_t((inputs[i] >> 8) & 0xFF));
		bytecode.push_back(uint8_t((inputs[i]) & 0xFF));
		printf("PUSH %d\n", inputs[i]);
	}

	{
		bytecode.push_back(uint8_t(opcode));
		bytecode.push_back(uint8_t(2)); // float
		printf("%s (float)\n", INSTRUCTION_NAMES[uint8_t(opcode)]);
	}

	{
		bytecode.push_back(uint8_t(RenderGraph::OpCode::POP));
		bytecode.push_back(uint8_t((output >> 8) & 0xFF));
		bytecode.push_back(uint8_t((output) & 0xFF));
		printf("POP %d\n", output);
	}
}

static inline void genOperationBytecode(const std::map<std::string, unsigned int> & mapOperations, const std::map<std::string, unsigned int> & mapTextures, const Graph & graph, Node * node, const std::map<std::string, std::vector<unsigned int>> & mapValues, std::vector<uint8_t> & bytecode)
{
	//
	// Inputs
	const std::vector<uint16_t> inputs = [&]
	{
		std::vector<uint16_t> values;

		std::vector<Edge*> inEdges;
		graph.getEdgeTo(node, inEdges);

		values.resize(inEdges.size());

		for (Edge * edge : inEdges)
		{
			Node * source = edge->getSource();

			const std::string & strId = source->getId();

			const std::string & source_id = edge->getMetaData("source_id");
			int outputParamIndex = atoi(source_id.c_str());
			assert(outputParamIndex < UINT8_MAX);

			const std::string & target_id = edge->getMetaData("target_id");
			int inputParamIndex = atoi(target_id.c_str());
			assert(inputParamIndex < UINT8_MAX);

			if (source->getType() == "texture")
			{
				std::vector<Edge*> inEdges2;
				graph.getEdgeTo(source, inEdges2);

				if (inEdges2.size() > 0)
				{
					Edge * edge2 = inEdges2[0];

					Node * source2 = edge2->getSource();

					const std::string & strId = source2->getId();

					const std::string & source_id = edge2->getMetaData("source_id");
					int outputParamIndex = atoi(source_id.c_str());
					assert(outputParamIndex < UINT8_MAX);

					assert(source2->getType() != "present" && source2->getType() != "texture");

					auto it = mapValues.find(strId);

					if (it != mapValues.end())
					{
						unsigned int index = it->second[outputParamIndex];
						uint16_t addr = (0 << 15) | (index & 0x7FFF);
						values[inputParamIndex] = addr;
					}
					else
					{
						assert(false);
					}
				}
			}
			else
			{
				assert(source->getType() != "present" && source->getType() != "texture");

				auto it = mapValues.find(strId);

				if (it != mapValues.end())
				{
					unsigned int index = it->second[outputParamIndex];
					uint16_t addr = (0 << 15) | (index & 0x7FFF);
					values[inputParamIndex] = addr;
				}
				else
				{
					assert(false);
				}
			}
		}

		return values;
	} ();

	assert(inputs.size() < UINT8_MAX);

	//
	// Outputs
	const std::vector<uint16_t> outputs = [&]
	{
		std::vector<uint16_t> values;

		std::vector<Edge*> outEdges;
		graph.getEdgeFrom(node, outEdges);

		values.resize(outEdges.size());

		for (Edge * edge : outEdges)
		{
			Node * target = edge->getTarget();

			const std::string & strId = target->getId();

			const std::string & source_id = edge->getMetaData("source_id");
			int outputParamIndex = atoi(source_id.c_str());
			assert(outputParamIndex < UINT8_MAX);

			const std::string & target_id = edge->getMetaData("target_id");
			int inputParamIndex = atoi(target_id.c_str());
			assert(inputParamIndex < UINT8_MAX);

			auto it = mapValues.find(node->getId());

			if (it != mapValues.end())
			{
				unsigned int index = it->second[outputParamIndex];
				uint16_t addr = (0 << 15) | (index & 0x7FFF);
				values[values.size() - 1 - outputParamIndex] = addr; // pop in reverse order
			}
			else
			{
				assert(false);
			}
		}

		return values;
	} ();

	assert(outputs.size() < UINT8_MAX);

	//
	// Gen bytecode
	for (int i = 0; i < inputs.size(); ++i)
	{
		bytecode.push_back(uint8_t(RenderGraph::OpCode::PUSH));
		bytecode.push_back(uint8_t((inputs[i] >> 8) & 0xFF));
		bytecode.push_back(uint8_t((inputs[i]) & 0xFF));
		printf("PUSH %d\n", inputs[i]);
	}

	{
		auto it = mapOperations.find(node->getId());

		if (it != mapOperations.end())
		{
			unsigned int index = it->second;
			uint16_t addr = (index & 0xFFFF);

			bytecode.push_back(uint8_t(RenderGraph::OpCode::CALL));
			bytecode.push_back(uint8_t((addr >> 8) & 0xFF));
			bytecode.push_back(uint8_t((addr) & 0xFF));
			printf("CALL %d\n", index);
		}
		else
		{
			assert(false);
		}
	}

	for (int i = 0; i < outputs.size(); ++i)
	{
		bytecode.push_back(uint8_t(RenderGraph::OpCode::POP));
		bytecode.push_back(uint8_t((outputs[i] >> 8) & 0xFF));
		bytecode.push_back(uint8_t((outputs[i]) & 0xFF));
		printf("POP %d\n", outputs[i]);
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
	std::map<std::string, std::vector<unsigned int>> mapValues;

	Instance * pInstance = createInstanceFromGraph(graph, mapTextures, mapValues, false);

	assert(pInstance != nullptr);

	return pInstance;
}

/**
 * @brief Factory::createInstanceFromGraph
 * @param graph
 * @return
 */
Instance * Factory::createInstanceFromGraph(const Graph & graph, std::map<std::string, unsigned int> & mapTextures, std::map<std::string, std::vector<unsigned int>> & mapValues) const
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
	std::map<std::string, std::vector<unsigned int>> mapValues;

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
Instance * Factory::createInstanceFromGraph(const Graph & graph, unsigned int /*GLuint*/ defaultFramebuffer, std::map<std::string, unsigned int> & mapTextures, std::map<std::string, std::vector<unsigned int>> & mapValues) const
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
Instance * Factory::createInstanceFromGraph(const Graph & graph, std::map<std::string, unsigned int> & mapTextures, std::map<std::string, std::vector<unsigned int>> & mapValues, bool bUseDefaultFramebuffer, unsigned int /*GLuint*/ defaultFramebuffer) const
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

		std::vector<unsigned int> outputs;

		{
			unsigned int index = values.size();
			outputs.push_back(index);

			RenderGraph::Value value;
			value.asFloat = atof(node->getMetaData("value").c_str());
			values.push_back(value);

			printf("MEM[%d] (const float) = %f\n", index, value.asFloat);
		}

		mapValues.insert(std::pair<std::string, std::vector<unsigned int>>(strId, outputs));
	}

	for (Node * node : aNodesOperator)
	{
		const std::string & strId = node->getId();

		std::vector<unsigned int> outputs;

		{
			unsigned int index = values.size();
			outputs.push_back(index);

			RenderGraph::Value value;
			value.asUInt = 0;
			values.push_back(value);

			printf("MEM[%d] (operator output) = 0\n", index);
		}

		mapValues.insert(std::pair<std::string, std::vector<unsigned int>>(strId, outputs));
	}

	for (Node * node : aNodesPass)
	{
		const std::string & strId = node->getId();

		std::vector<unsigned int> outputs;

		std::vector<Edge*> outEdges;
		graph.getEdgeFrom(node, outEdges);

		unsigned int i = 0;
		for (Edge * edge : outEdges)
		{
			unsigned int index = values.size();
			outputs.push_back(index);

			RenderGraph::Value value;
			value.asUInt = 0;
			values.push_back(value);

			printf("MEM[%d] (operation output %d) = 0\n", index, i++);
		}

		mapValues.insert(std::pair<std::string, std::vector<unsigned int>>(strId, outputs));
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

	std::vector<Operation*> operations;
	operations.reserve(aNodesPass.size());

	std::map<std::string, unsigned int> mapOperations;

	for (Node * node : aNodesPass)
	{
		const std::string & strId = node->getId();
		const std::string & strSubType = node->getMetaData("subtype");

		Operation * operation = createOperation(strSubType.c_str());

		mapOperations.insert(std::pair<std::string, unsigned int>(strId, operations.size()));

		operations.push_back(operation);
	}

	// ----------------------------------------------------------------------------------------

	Framebuffer * pDefaultFramebuffer = nullptr;

	std::vector<Framebuffer*> framebuffers;
	framebuffers.reserve(aNodesPass.size());

	for (Node * node : aNodesPass)
	{
		const std::string & strId = node->getId();
		const std::string & strSubType = node->getMetaData("subtype");

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
				if (target->getType() == "texture")
				{
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
			}

			if (!fbTextures.empty())
			{
				auto it = mapOperations.find(strId);

				if (it != mapOperations.end())
				{
					Framebuffer * framebuffer = new Framebuffer(fbTextures, static_cast<Pass*>(operations[it->second]));

					if (graph.hasEdge(node, pDefaultFramebufferNode))
					{
						pDefaultFramebuffer = framebuffer;
					}

					framebuffers.push_back(framebuffer);
				}
			}
		}
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
			genOperationBytecode(mapOperations, mapTextures, graph, node, mapValues, bytecode);
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
		pRenderGraph = new InstanceWithExternalFramebuffer(bytecode, operations, framebuffers, textures, values, defaultFramebuffer);
	}
	else
	{
		pRenderGraph = new InstanceWithInternalFramebuffer(bytecode, operations, framebuffers, textures, values, pDefaultFramebuffer);
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
 * @brief Factory::registerOperation
 * @param identifier
 * @param factory
 * @return
 */
bool Factory::registerOperation(const char * identifier, OperationFactory factory)
{
	m_factories.insert(std::pair<std::string, OperationFactory>(identifier, factory));

	return(true);
}

/**
 * @brief Factory::createOperation
 * @param identifier
 * @return
 */
Operation * Factory::createOperation(const char * identifier) const
{
	auto it = m_factories.find(identifier);

	if (it != m_factories.end())
	{
		OperationFactory factory = it->second;
		Operation * operation = factory();
		assert(operation != nullptr);
		operation->init();
		return operation;
	}

	return(nullptr);
}

/**
 * @brief Factory::destroyOperation
 * @param operation
 */
void Factory::destroyOperation(Operation * operation) const
{
	operation->release();
	delete operation;
}

}
