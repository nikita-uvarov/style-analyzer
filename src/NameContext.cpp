#include "NameContext.h"

using namespace std;
using namespace sa;

unique_ptr <NameContext> NameContext::create (CXTranslationUnit /*unit*/)
{
	return nullptr;
}
