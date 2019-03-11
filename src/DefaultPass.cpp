#include "DefaultPass.h"

/**
 * @brief DefaultPass::DefaultPass
 */
DefaultPass::DefaultPass()
{
	m_loadOp = RenderGraph::Pass::ATTACHMENT_LOAD_OP_CLEAR;
	m_storeOp = RenderGraph::Pass::ATTACHMENT_STORE_OP_DONT_CARE;

	m_fClearColorR = 1.0f;
	m_fClearColorG = 0.0f;
	m_fClearColorB = 0.0f;
	m_fClearColorA = 0.0f;

	m_fClearDepthValue = 1.0f;
}

/**
 * @brief DefaultPass::render
 * @return
 */
bool DefaultPass::render(const RenderGraph::Parameters & parameters)
{
	return true;
}
