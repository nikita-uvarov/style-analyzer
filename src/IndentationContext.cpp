#include "IndentationContext.h"

using namespace std;
using namespace sa;

unique_ptr <IndentationContext> IndentationContext::create (CXTranslationUnit /*unit*/)
{
    return nullptr;
}
