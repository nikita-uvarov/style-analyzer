#ifndef STYLE_ANALYZER_PROJECT_CONTEXT_H
#define STYLE_ANALYZER_PROJECT_CONTEXT_H

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

#endif // STYLE_ANALYZER_PROJECT_CONTEXT_H
