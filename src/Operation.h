#pragma once

namespace RenderGraph
{

class Parameters;

class Operation
{
public:

	//
	// Destructor
	virtual	~Operation(void);

	//
	// Initialize / Release
	virtual bool	init			(void) = 0;
	virtual void	release			(void) = 0;

	//
	// Execute
	virtual bool	execute			(Parameters & parameters) = 0;

};

}
