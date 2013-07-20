#ifndef PROJECT_CONTEXT_H
#define PROJECT_CONTEXT_H

#include <vector>
#include <iostream>
#include <memory>

#include "FileContext.h"

namespace sa
{
	using std::vector;
	using std::unique_ptr;
	//using std::ifstream;
	//using std::ofstream;
	
	class ProjectContext
	{
	public :
		vector < unique_ptr <FileContext> > fileContexts;
	};
}

#endif // PROJECT_CONTEXT_H