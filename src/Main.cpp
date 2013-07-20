#include <cstddef>
#include <cstdio>
#include <cassert>
#include <cstdint>

#include "ProjectContext.h"

using namespace std;

void printTranslationUnitParseFailure()
{
	printf ("Failed to parse translation unit\n");
}

int main (int argc, char** argv)
{
	cerr.sync_with_stdio (false);
	
	cerr << "Tool started with: '";
	for (int i = 0; i < argc; i++)
	{
		if (i) cerr << " ";
		cerr << argv[i];
	}
	cerr << "'" << endl;
	
	CXIndex clangIndex = clang_createIndex (0, 0);
	assert (clangIndex);
	
    CXTranslationUnit clangTranslationUnit = clang_parseTranslationUnit (clangIndex, nullptr, argv + 1, argc - 1, nullptr, 0, CXTranslationUnit_DetailedPreprocessingRecord);

	if (!clangTranslationUnit)
	{
		cerr << "Failed to parse translation unit. Common reasons:\n";
		cerr << "1. Invalid options, non-existing input file, etc. Try launching 'clang++";
		for (int i = 1; i < argc; i++)
			cerr << " " << argv[i];
		cerr << "'\n";
		cerr << "2. Multiple input files." << endl;
		
		return 1;
	}

	unsigned nDiagnostics = clang_getNumDiagnostics (clangTranslationUnit);
	bool wereErrors = false;
	
	for (unsigned i = 0; i < nDiagnostics; i++)
	{
		CXDiagnostic diagnostic = clang_getDiagnostic (clangTranslationUnit, i);
		assert (diagnostic);
		
		CXString diagnosticString = clang_formatDiagnostic (diagnostic, clang_defaultDiagnosticDisplayOptions());		
		const char* cDiagnosticString = clang_getCString (diagnosticString);
		assert (cDiagnosticString);
		
		cerr << cDiagnosticString << endl;
		
		CXDiagnosticSeverity severity = clang_getDiagnosticSeverity (diagnostic);
				
		clang_disposeString (diagnosticString);
		clang_disposeDiagnostic (diagnostic);
		
		if (severity == CXDiagnosticSeverity::CXDiagnostic_Error || severity == CXDiagnosticSeverity::CXDiagnostic_Fatal)
			wereErrors = true;
	}
	
	if (wereErrors)
	{
		cerr << "Error met in a translation unit, exiting." << endl;
		return 1;
	}
	
	cerr << "Context processing started." << endl;

    //clang_visitChildren (clang_getTranslationUnitCursor (TU), visit_fn, nullptr);

	/*CXSourceRange range = clang_getCursorExtent (clang_getTranslationUnitCursor (TU));
	CXToken* tokens;
	unsigned int nTokens;
	clang_tokenize(TU, range, &tokens, &nTokens);
	*/
	//CXSourceLocation begin = clang_getRangeStart (range);

	/*for (unsigned i = 0; i < nTokens; i++)
	{
		CXToken token = tokens[i];
		CXString spelling = clang_getTokenSpelling (TU, token);
		CXSourceLocation tl = clang_getTokenLocation (TU, token);

		CXFile file;
		unsigned int line, column, offset;
		clang_getFileLocation (tl, &file, &line, &column, &offset);

		printf ("Token: '%s', kind: %s\n", clang_getCString (spelling), tokenKindToString (clang_getTokenKind (token)));
		clang_disposeString (spelling);
	}*/

    clang_disposeTranslationUnit (clangTranslationUnit);
    clang_disposeIndex (clangIndex);
	
	return 0;
}
