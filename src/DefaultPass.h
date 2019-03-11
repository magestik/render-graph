#pragma once

#include "Pass.h"

class DefaultPass : public RenderGraph::Pass
{
public:

	DefaultPass();

	virtual bool render(const RenderGraph::Parameters & parameters) override;

};
