#include "FileContext.h"
#include "FileStreams.h"
#include "ApplicationLog.h"

using namespace sa;
using namespace std;

string convertClangString (CXString clangString, bool assertNonNull = true)
{
    const char* cString = clang_getCString (clangString);
    saAssert (!assertNonNull || cString);
    string result (cString);
    clang_disposeString (clangString);

    return result;
}

unique_ptr <FileContext> FileContext::create (CXTranslationUnit unit)
{
    string sourceFileName = convertClangString (clang_getTranslationUnitSpelling (unit));
    saLog ("Translation unit corresponds to file '%1'") << sourceFileName;

    unique_ptr <UniversalInputStream> stream
        = UniversalInputStream::openInputStream (sourceFileName, RelativeInputStreamFlags::NONE);
    uint32_t fileSize = stream->getNumBytesRemaining();
    unique_ptr <char[]> fileContentsBuffer (new char[fileSize + 1]);
    fileContentsBuffer[fileSize] = 0;
    stream->read (fileContentsBuffer.get(), fileSize);

    unique_ptr <FileContext> context (new FileContext (string (fileContentsBuffer.get()), sourceFileName));
    saLog ("Read file and created file context.");

    saLog ("Ready to create indentation subcontext");
    context->indentationContext = IndentationContext::create (*context, unit);
    saLog ("Indentation subcontext created");

    saLog ("Ready to create name subcontext");
    context->nameContext = NameContext::create (unit);
    saLog ("Name subcontext created");

    return context;
}

void sa::FileContext::save (IOutputStream* stream)
{
    serializeString (stream, fileName);
    serializeString (stream, fileContents);

    indentationContext->save (stream);
    nameContext->save (stream);
}

string sa::deserializeString (IInputStream* stream)
{
    uint32_t length;
    saVerify (stream->read (reinterpret_cast <char*> (&length), 4) == 4);

    unique_ptr <char[]> contents (new char[length + 1]);
    saVerify (stream->read (contents.get(), length) == length);

    string result = contents.get();
    return result;
}

void sa::serializeString (IOutputStream* stream, const string& s)
{
    uint32_t length = static_cast <uint32_t> (s.length());
    stream->write (reinterpret_cast <char*> (&length), 4);

    stream->write (s.c_str(), length);
}


/*unique_ptr <FileContext> FileContext::load (IInputStream* stream)
{

}
*/
