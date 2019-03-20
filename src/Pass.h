#pragma once

#include "Operation.h"

namespace RenderGraph
{

class Framebuffer;
class Parameters;

class Pass : public Operation
{
public:

	Pass();
	virtual ~Pass() override;

	virtual bool	init			(void) override;
	virtual void	release			(void) override;

	virtual bool	execute			(Parameters & parameters) override;

	virtual bool	begin			(void);
	virtual bool	render			(Parameters & parameters) = 0;
	virtual bool	end				(void);

	inline void setFramebuffer(unsigned int /*GLuint*/ framebuffer, unsigned int width, unsigned int height)
	{
		m_iFramebufferNativeHandle = framebuffer;
		m_iFramebufferWidth = width;
		m_iFramebufferHeight = height;
	}

protected:

	enum AttachmentLoadOp
	{
		ATTACHMENT_LOAD_OP_LOAD = 0,
		ATTACHMENT_LOAD_OP_CLEAR = 1,
		ATTACHMENT_LOAD_OP_DONT_CARE = 2,
	};

	AttachmentLoadOp m_loadOp;
	AttachmentLoadOp m_depthLoadOp;

	enum AttachmentStoreOp
	{
		ATTACHMENT_STORE_OP_STORE = 0,
		ATTACHMENT_STORE_OP_DONT_CARE = 1,
	};

	AttachmentStoreOp m_storeOp;
	AttachmentStoreOp m_depthStoreOp;

	float m_fClearColorR;
	float m_fClearColorG;
	float m_fClearColorB;
	float m_fClearColorA;

	float m_fClearDepthValue;

	int m_iClearStencilValue;

private:

	unsigned int /*GLuint*/ m_iFramebufferNativeHandle;
	unsigned int m_iFramebufferWidth;
	unsigned int m_iFramebufferHeight;
};

}
