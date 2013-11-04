#include "NameContext.h"

using namespace std;
using namespace sa;

unique_ptr <NameContext> sa::NameContext::create (CXTranslationUnit /*unit*/)
{
    return unique_ptr <NameContext> (new NameContext);
}

unique_ptr <NameContext> sa::NameContext::load (IInputStream* /*stream*/)
{
    unique_ptr <NameContext> context (new NameContext);
    return context;
}

void sa::NameContext::save (IOutputStream* /*stream*/)
{

}
