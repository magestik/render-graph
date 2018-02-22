#include "Queue.h"
#include "Pass.h"

#include "Graph.h"

namespace RenderGraph
{

/**
 * @brief Default constructor
 */
Queue::Queue(void)
{
	// ...
}

/**
 * @brief Destructor
 */
Queue::~Queue(void)
{
	// ...
}

/**
 * @brief RenderGraph::render
 * @return
 */
bool Queue::render(void)
{
	for (Pass * pass : m_renderQueue)
	{
		pass->render();
	}

	return(true);
}

}
