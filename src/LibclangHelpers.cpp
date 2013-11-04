#include "LibclangHelpers.h"
#include "Debug.h"

#include <algorithm>

using namespace sa;

sa::ClangIndex::ClangIndex (bool excludeDeclarationsFromPCH, bool displayDiagnostics) :
    theIndex (nullptr)
{
    theIndex = clang_createIndex (excludeDeclarationsFromPCH, displayDiagnostics);
}

sa::ClangIndex::~ClangIndex()
{
    if (theIndex)
        clang_disposeIndex (theIndex);

    for (auto it: unitsAllocated)
        clang_disposeTranslationUnit (it);
}

sa::ClangIndex::operator CXIndex()
{
    return theIndex;
}

ClangTranslationUnit sa::ClangIndex::parseTranslationUnit (const string& sourceFilename, const vector <string>& commandLineArgs,
                                                           unsigned int options, vector <CXUnsavedFile>& unsavedFiles)
{
    vector <const char*> pureArguments (commandLineArgs.size());
    auto result = pureArguments.begin();
    for (auto it = commandLineArgs.begin(); it != commandLineArgs.end(); it++, result++)
        *result = it->c_str();

    return parseTranslationUnit (sourceFilename.c_str(), pureArguments.data(), static_cast <int> (pureArguments.size()),
                                 unsavedFiles.data(), static_cast <unsigned int> (unsavedFiles.size()), options);
}

ClangTranslationUnit sa::ClangIndex::parseTranslationUnit (const std::string& sourceFilename,
                                                           const std::vector <std::string>& commandLineArgs,
                                                           unsigned int options)
{
    vector <CXUnsavedFile> empty;
    return parseTranslationUnit (sourceFilename, commandLineArgs, options, empty);
}

ClangTranslationUnit sa::ClangIndex::parseTranslationUnit (const char* sourceFilename, const char* const* commandLineArgs,
                                                           int nCommandLineArgs, CXUnsavedFile* unsavedFiles,
                                                           unsigned int nUnsavedFiles, unsigned int options)
{
    saAssert (theIndex);

    CXTranslationUnit unit = clang_parseTranslationUnit (theIndex, sourceFilename, commandLineArgs,
                                                         nCommandLineArgs, unsavedFiles, nUnsavedFiles, options);
    unitsAllocated.push_back (unit);

    return ClangTranslationUnit (unit);
}

sa::ClangTranslationUnit::ClangTranslationUnit (CXTranslationUnit unit) :
    theUnit (unit)
{}

ClangDiagnostic::ClangDiagnostic (CXDiagnostic diagnostic) :
    diagnostic (diagnostic)
{}

string sa::ClangDiagnostic::formatDiagnostic (unsigned int options)
{
    return convertCXString (clang_formatDiagnostic (diagnostic, options));
}

ClangDiagnostic::~ClangDiagnostic()
{
    clang_disposeDiagnostic (diagnostic);
}

CXDiagnosticSeverity ClangDiagnostic::getSeverity()
{
    return clang_getDiagnosticSeverity (diagnostic);
}

ClangDiagnostic ClangTranslationUnit::getDiagnostic (int i)
{
    return ClangDiagnostic (clang_getDiagnostic (theUnit, static_cast <unsigned int> (i)));
}

int ClangTranslationUnit::getNumDiagnostics()
{
    return static_cast <int> (clang_getNumDiagnostics (theUnit));
}

string sa::convertCXString (CXString allocated)
{
    const char* contents = clang_getCString (allocated);
    string stdString = contents;
    clang_disposeString (allocated);
    return stdString;
}

sa::ClangTranslationUnit::operator CXTranslationUnit()
{
    return theUnit;
}

string sa::cxTokenKindToString (CXTokenKind kind)
{
    switch (kind)
    {
        case CXToken_Punctuation: return "punctuation";
        case CXToken_Keyword:     return "keyword";
        case CXToken_Identifier:  return "identifier";
        case CXToken_Literal:     return "literal";
        case CXToken_Comment:     return "comment";
    }
    saUnreachable ("Invalid token kind.");
}
