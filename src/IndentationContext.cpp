#include "IndentationContext.h"
#include "LibclangHelpers.h"
#include "ApplicationLog.h"
#include "FileContext.h"

using namespace std;
using namespace sa;

unsigned getSourceLocationOffset (CXSourceLocation location)
{
    CXFile file;
    unsigned int line, column, offset;
    clang_getFileLocation (location, &file, &line, &column, &offset);
    return offset;
}

unique_ptr <IndentationContext> IndentationContext::create (FileContext& fileContext, CXTranslationUnit unit)
{
    unique_ptr <IndentationContext> context (new IndentationContext);

    CXSourceRange range = clang_getCursorExtent (clang_getTranslationUnitCursor (unit));
    CXToken* tokens;
    unsigned int nTokens;
    clang_tokenize (unit, range, &tokens, &nTokens);

    string fileContents = fileContext.getFileContents();

    //CXSourceLocation begin = clang_getRangeStart (range);

    unsigned previousTokenNextCharacterOffset = 0;
    unsigned lastLineSpaceLevel = 0;

    for (unsigned i = 0; i < nTokens; i++)
    {
        CXToken token = tokens[i];
        string spelling = convertCXString (clang_getTokenSpelling (unit, token));

        CXSourceRange tokenRange = clang_getTokenExtent (unit, token);
        CXSourceLocation tokenBegin = clang_getRangeStart (tokenRange);
        CXSourceLocation tokenLastCharacter = clang_getRangeEnd (tokenRange);

        unsigned beginOffset = getSourceLocationOffset (tokenBegin);
        // FIXME: UTF8 support (???, multi-byte in identifiers)
        unsigned endOffset = getSourceLocationOffset (tokenLastCharacter) + 1;

        saLog ("Token: '%1', kind: %2, offset: %3") << spelling << cxTokenKindToString (clang_getTokenKind (token))
                                                    << static_cast <int> (beginOffset);
        //clang_disposeString (spelling);

        Token tokenCopy;
        tokenCopy.token = token;
        tokenCopy.fileBufferOffset = beginOffset;
        tokenCopy.tokenValue = spelling;

        if (i > 0)
        {
            unsigned spaceCounter = 0;
            bool wasNewline = false;

            for (unsigned j = previousTokenNextCharacterOffset; j < beginOffset; j++)
            {
                if (fileContents[j] == ' ')
                {
                    spaceCounter++;
                }
                else if (fileContents[j] == '\n')
                {
                    wasNewline = true;
                    spaceCounter = 0;
                }
            }

            if (wasNewline)
            {
                unsigned offset = spaceCounter - lastLineSpaceLevel;
                lastLineSpaceLevel = spaceCounter;
                spaceCounter = offset;
            }

            TokenInterval& lastTokenInterval = context->tokenStream.back().afterTokenInterval.interval;
            lastTokenInterval.isAfterNewline = wasNewline;
            lastTokenInterval.nSpaces = spaceCounter;
        }

        context->tokenStream.push_back (tokenCopy);
        previousTokenNextCharacterOffset = endOffset;
    }

    return context;
}

/*void IndentationContext::addInvisibleModifier (IndentationContext::Token& token, string modifierName)
{

}*/

void IndentationContext::setTokenClass (IndentationContext::Token& token, string tokenClassName)
{
    auto it = tokenClassNameToId.find (tokenClassName);
    TokenClassId id;

    if (it == tokenClassNameToId.end())
    {
        id = static_cast <uint32_t> (tokenClassNameToId.size());
        tokenClassNameToId[tokenClassName] = id;
    }
    else
    {
        id = it->second;
    }

    token.tokenClass = id;
}

/*void IndentationContext::assignTokenTypes()
{
    for (Token& token: tokenStream)
    {
    }
}*/

unique_ptr <IndentationContext> sa::IndentationContext::load (IInputStream* /*stream*/)
{
    unique_ptr <IndentationContext> context (new IndentationContext);
    return context;
}

void sa::IndentationContext::save (IOutputStream* /*stream*/)
{

}
