#ifndef STYLE_ANALYZER_LIBCLANG_HELPERS
#define STYLE_ANALYZER_LIBCLANG_HELPERS

#include <clang-c/Index.h>
#include <string>
#include <vector>

#include "StringFormatter.h"

namespace sa
{

using namespace std;

string convertCXString (CXString allocated);

class ClangDiagnostic
{
public :
    ClangDiagnostic (CXDiagnostic diagnostic);
    ~ClangDiagnostic();

    string formatDiagnostic (unsigned options);
    CXDiagnosticSeverity getSeverity();

private :
    CXDiagnostic diagnostic;
};

class ClangTranslationUnit
{
public :
    ClangTranslationUnit (CXTranslationUnit unit);

    int getNumDiagnostics();
    ClangDiagnostic getDiagnostic (int i);

    operator CXTranslationUnit();

private :
    CXTranslationUnit theUnit;
};

class ClangIndex
{
public :
    ClangIndex (bool excludeDeclarationsFromPCH, bool displayDiagnostics);
    ~ClangIndex();

    ClangTranslationUnit parseTranslationUnit (const char* sourceFilename, const char* const* commandLineArgs,
                                               int nCommandLineArgs, struct CXUnsavedFile* unsavedFiles,
                                               unsigned nUnsavedFiles, unsigned options);

    ClangTranslationUnit parseTranslationUnit (const string& sourceFilename, const vector <string>& commandLineArgs,
                                               unsigned options, vector <CXUnsavedFile>& unsavedFiles);

    ClangTranslationUnit parseTranslationUnit (const string& sourceFilename, const vector <string>& commandLineArgs,
                                               unsigned options = CXTranslationUnit_None);

    operator CXIndex ();

private:
    CXIndex theIndex;
    vector <CXTranslationUnit> unitsAllocated;
};

string cxTokenKindToString (CXTokenKind kind);

}

#endif // STYLE_ANALYZER_LIBCLANG_HELPERS
