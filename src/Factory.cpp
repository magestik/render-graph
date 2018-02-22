#include "Factory.h"

#include "Pass.h"
#include "Queue.h"

#include "Graph.h"

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
 * @brief Factory::createQueueFromGraph
 * @param graph
 * @return
 */
Queue * Factory::createQueueFromGraph(const Graph & graph)
{
	Queue * q = new Queue();

	// TODO : serialize graph + create Passes and add them in the RenderQueue

	return(q);
}

/**
 * @brief Factory::destroyQueue
 * @param queue
 */
void Factory::destroyQueue(Queue * queue)
{
	delete queue;
}

/**
 * @brief Factory::registerPassDescriptor
 * @param descriptor
 * @return
 */
bool Factory::registerPassDescriptor(const PassDescriptor & descriptor)
{
	// FIXME : check that it does not exist already
	m_descriptors.push_back(descriptor);
	return(true);
}

}
