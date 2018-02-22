#pragma once

#include <vector>

namespace RenderGraph
{

class Pass;

class Queue
{
public:

	Queue(void);
	~Queue(void);

	bool render(void);

private:

	std::vector<Pass*> m_renderQueue;

};

}
