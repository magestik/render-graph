#pragma once

#include "PassDescriptor.h"

#include <vector>

class Graph;

namespace RenderGraph
{

class Queue;

class Factory
{
public:

	Factory(void);
	~Factory(void);

	Queue * createQueueFromGraph(const Graph & graph);
	void destroyQueue(Queue * queue);

	bool registerPassDescriptor(const PassDescriptor & descriptor);

private:

	std::vector<PassDescriptor> m_descriptors;
};

}
