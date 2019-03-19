#include <stdio.h>

#include <SDL2/SDL.h>

#include "RenderGraph.h"

#include "Graph.h"

/**
 * @brief The TestPass class
 */
class TestPass : public RenderGraph::Pass
{
public:

	TestPass()
	{
		m_loadOp = RenderGraph::Pass::ATTACHMENT_LOAD_OP_CLEAR;
		m_storeOp = RenderGraph::Pass::ATTACHMENT_STORE_OP_STORE;

		m_fClearColorR = 1.0f;
	}

	virtual bool render(const RenderGraph::Parameters & parameters)
	{
		return true;
	}

	virtual void setParameter(RenderGraph::Instance * pInstance, const char * name, const char * value)
	{
		// nothing here
	}
};

/**
 * @brief TestPassFactory
 * @return
 */
RenderGraph::Pass * TestPassFactory(void)
{
	return new TestPass;
}

/**
 * @brief main
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char** argv)
{
	//
	// Load SDL2
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		return -1;
	}

	//
	// Create Window
	SDL_Window * pWindow = SDL_CreateWindow("RenderGraph SDL2 Demo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_OPENGL);

	if (!pWindow)
	{
		return -1;
	}

	//
	// Create OpenGL Context
	SDL_GLContext context = SDL_GL_CreateContext(pWindow);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	//
	// Load Graph
	Graph G;
	if (!G.loadFromFile("/tmp/render-graph.json"))
	{
		return -1;
	}

	//
	// Create render graph instance
	RenderGraph::Factory factory;

	factory.registerPass("test", TestPassFactory);

	RenderGraph::Instance * pRenderGraph = factory.createInstanceFromGraph(G, 0u);

	if (!pRenderGraph)
	{
		return -1;
	}

	//
	// Render
	bool quit = false;

	while (!quit)
	{
		SDL_Event e;
		while ( SDL_PollEvent( &e ) != 0 )
		{
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}
		}

		pRenderGraph->execute();

		SDL_GL_SwapWindow(pWindow);
	}

	SDL_GL_DeleteContext(context);

	SDL_Quit();

	return 0;
}
