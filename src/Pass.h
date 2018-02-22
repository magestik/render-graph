#pragma once

namespace RenderGraph
{

class Pass
{
public:

	virtual bool init			(void) = 0;
	virtual void release		(void) = 0;

	virtual bool render			(void) = 0;

	virtual void setParameter	(const char * name, const char * value) = 0;

};

}
