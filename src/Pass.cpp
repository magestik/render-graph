#include "Pass.h"

#include "Framebuffer.h"

#include "OpenGL.h"

#include <assert.h>

static_assert(sizeof(unsigned int) == sizeof(GLuint), "size of unsigned int must match size of GLuint");

namespace RenderGraph
{

/**
 * @brief Constructor
 */
Pass::Pass(void) : m_loadOp(ATTACHMENT_LOAD_OP_LOAD), m_depthLoadOp(ATTACHMENT_LOAD_OP_LOAD), m_storeOp(ATTACHMENT_STORE_OP_STORE), m_depthStoreOp(ATTACHMENT_STORE_OP_STORE), m_iFramebufferNativeHandle(0), m_iFramebufferWidth(0), m_iFramebufferHeight(0)
{
	m_fClearColorR = 0.0f;
	m_fClearColorG = 0.0f;
	m_fClearColorB = 0.0f;
	m_fClearColorA = 0.0f;

	m_fClearDepthValue = 1.0f;
	m_iClearStencilValue = 0;
}

/**
 * @brief Destructor
 */
Pass::~Pass(void)
{
	// ...
}

/**
 * @brief Initialize
 * @return
 */
bool Pass::init(void)
{
	assert(m_iFramebufferNativeHandle == 0);
	return true;
}

/**
 * @brief Release
 */
void Pass::release(void)
{
	m_iFramebufferNativeHandle = 0;
}

/**
 * @brief Pass::execute
 * @param parameters
 */
bool Pass::execute(Parameters & parameters)
{
	if (begin())
	{
		bool success = render(parameters);

		if (!success)
		{
			return false;
		}

		return end();
	}

	return false;
}

/**
 * @brief Begin
 */
bool Pass::begin(void)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_iFramebufferNativeHandle);

	if (m_iFramebufferNativeHandle != 0)
	{
		GLenum buffers [4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glDrawBuffers(4, buffers); // FIXME
	}

	glViewport(0, 0, m_iFramebufferWidth, m_iFramebufferHeight);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	if (m_loadOp == ATTACHMENT_LOAD_OP_CLEAR && m_depthLoadOp == ATTACHMENT_LOAD_OP_CLEAR)
	{
		glClearColor(m_fClearColorR, m_fClearColorG, m_fClearColorB, m_fClearColorA);
		glClearDepthf(m_fClearDepthValue);
		glClearStencil(m_iClearStencilValue);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}
	else if (m_loadOp == ATTACHMENT_LOAD_OP_CLEAR)
	{
		glClearColor(m_fClearColorR, m_fClearColorG, m_fClearColorB, m_fClearColorA);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	else if (m_depthLoadOp == ATTACHMENT_LOAD_OP_CLEAR)
	{
		glClearDepthf(m_fClearDepthValue);
		glClearStencil(m_iClearStencilValue);
		glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	if (m_loadOp == ATTACHMENT_LOAD_OP_DONT_CARE)
	{
		// TODO : glInvalidateFramebuffer or glDiscardFramebufferEXT (or glClear on tile-based GPUs)
	}

	assert(glGetError() == GL_NO_ERROR);

	return true;
}

/**
 * @brief End
 */
bool Pass::end(void)
{
	if (m_storeOp == ATTACHMENT_STORE_OP_DONT_CARE)
	{
		// TODO : glInvalidateFramebuffer or glDiscardFramebufferEXT (or glClear on tile-based GPUs)
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	assert(glGetError() == GL_NO_ERROR);

	return true;
}

}
