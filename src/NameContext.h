#ifndef STYLE_ANALYZER_NAME_CONTEXT_H
#define STYLE_ANALYZER_NAME_CONTEXT_H

#include <iostream>
#include <memory>

#include <clang-c/Index.h>

#include "Streams.h"

namespace sa
{

using std::ifstream;
using std::ofstream;
using std::unique_ptr;

class NameContext
{
public :
    void save (IOutputStream* stream);
    static unique_ptr <NameContext> load (IInputStream* stream);
    static unique_ptr <NameContext> create (CXTranslationUnit unit);

private :

    NameContext (const NameContext&) = delete;
    NameContext& operator= (const NameContext&) = delete;
};

}

#endif // STYLE_ANALYZER_NAME_CONTEXT_H
