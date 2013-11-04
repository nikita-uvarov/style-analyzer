#include <cstddef>
#include <cstdio>
#include <cassert>
#include <cstdint>
#include <boost/concept_check.hpp>

#include "ProjectContext.h"
#include "FileStreams.h"
#include "Debug.h"
#include "ApplicationLog.h"
#include "IniConfiguration.h"
#include "LibclangHelpers.h"

using namespace std;

void printTranslationUnitParseFailure()
{
    printf ("Failed to parse translation unit\n");
}

void grabDataFromFile (sa::IniConfiguration& project, string file)
{
    saLog ("Grabbing data from file '%1'...") << file;

    vector <string> compilerCommandLineOptions = project["datagrabbing.commonclangoptions"].asVector();

    sa::ClangIndex index (false, true);

    sa::ClangTranslationUnit unit = index.parseTranslationUnit (file, compilerCommandLineOptions);

    if (!unit)
        saError ("Translation unit not created, see stderr for more info");

    int nDiagnostics = unit.getNumDiagnostics();
    bool wereErrors = false;

    for (int i = 0; i < nDiagnostics; i++)
    {
        sa::ClangDiagnostic diag = unit.getDiagnostic (i);
        if (diag.getSeverity() == CXDiagnostic_Error || diag.getSeverity() == CXDiagnostic_Fatal)
            wereErrors = true;

        saLog (diag.formatDiagnostic (clang_defaultDiagnosticDisplayOptions()));
    }

    if (wereErrors)
        saError ("There were errors in a translation unit: grabbing impossible");

    saLog ("Translation unit parsed successfully");

    unique_ptr <sa::FileContext> fileContext = sa::FileContext::create (unit);
    saAssert (fileContext);

    saLog ("File context is ready to be serialized");

    unique_ptr <sa::IOutputStream> contextOutputStream
        = sa::FileOutputStream::openOutputStream (project["common.contextfilename"],
                                                  sa::RelativeOutputStreamFlags::APPEND | sa::RelativeOutputStreamFlags::BINARY);

    fileContext->save (contextOutputStream.get());

    saLog ("Data grabbing finished for file '%1'") << file;
}

void doDataGrabbing (sa::IniConfiguration& project)
{
    saLog ("Data grabbing is enabled.");
    vector <string> files = project["datagrabbing.files"].asVector();
    saLog ("Grabbing from files: %1") << files;

    for (unsigned i = 0; i < files.size(); i++)
        grabDataFromFile (project, project["datagrabbing.files"].resolveRelativePath (i, files[i]));
}

void processProjectFile (string projectFile)
{
    sa::IniIncludeManager includeManager;
    unique_ptr <sa::IInputStream> projectIniFileStream
        = includeManager.openInputStream (projectFile, sa::RelativeInputStreamFlags::NONE);
    unique_ptr <sa::IniConfiguration> project (sa::IniConfiguration::load (projectFile, projectIniFileStream.get(),
                                                                           &includeManager));

    string projectName = (*project)["project.name"];
    saLog ("Project name: '%1'") << projectName;
    saLog ("Project description: '%1'") << (*project)["project.description"].asString();

    if ((*project)["common.newcontext"].asBoolean())
    {
        string contextFileName = (*project)["common.contextfilename"].asString();
        saLog ("Context '%1' recreation required.") << contextFileName;
        unique_ptr <sa::IOutputStream> contextMakeBlankStream
            = sa::FileOutputStream::openOutputStream (contextFileName, sa::RelativeOutputStreamFlags::BINARY);
    }

    if ((*project)["datagrabbing.enabled"].asBoolean())
        doDataGrabbing (*project);

    // Must be invoked from style analyzer temporary directory already.
    // Create 'sa-context' file

    // Parse project configuration & form compiler invocation

    // Parse translation unit

    // If in debug build/option specified, invoke clang++ to check that it's possible to compile
    // ===> Only if parse TU failure || compilation error diagnostic.

    // Check what assertions are present. Create corresponding verifiers & run verification.

}

int unsafeMain (int argc, char** argv)
{
    saLog ("Entering unsafeMain");

    if (argc != 2)
    {
        saLog ("Expected single parameter: path to project file.");
        return 1;
    }

    string projectFile = argv[1];
    processProjectFile (projectFile);

    return 0;

#if 0
    CXIndex clangIndex = clang_createIndex (0, 0);
    saAssert (clangIndex);

    CXTranslationUnit clangTranslationUnit = clang_parseTranslationUnit (clangIndex, nullptr, argv + 1, argc - 1, nullptr,
                                                                         0, CXTranslationUnit_DetailedPreprocessingRecord);

    if (!clangTranslationUnit)
    {
        cerr << "Failed to parse translation unit. Common reasons:\n";
        cerr << "1. Invalid options, non-existing input file, failed to start compilation, etc. Try launching 'clang++";
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
        saLog ("Error met in a translation unit, exiting.");
        return 1;
    }

    saLog ("Ready to create FileContext");

    unique_ptr <sa::FileContext> fileContext = sa::FileContext::create (clangTranslationUnit);
    assert (fileContext);

    saLog ("FileContext created");

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

    saLog ("Ready to destroy translation unit and index");

    clang_disposeTranslationUnit (clangTranslationUnit);
    clang_disposeIndex (clangIndex);

    saLog ("Leaving unsafeMain");

    return 0;
#endif
}

int loggedMain (int argc, char** argv)
{
    try
    {
        return unsafeMain (argc, argv);
    }
    catch (sa::Exception& e)
    {
        saError ("sa::Exception caught:\n%1\nRaised in %2") << e.toString() << e.originToString();
    }
    catch (std::exception& e)
    {
        saError ("STL Exception caught:\n%1") << e.what();
    }
    catch (...)
    {
        saError ("Unknown exception leaves main.");
    }

    return 1;
}

int main (int argc, char** argv)
{
    cerr.sync_with_stdio (false);

#ifdef BUILD_CONFIGURATION_DEBUG
    cerr << "Running debug build." << endl;
    system ("echo Working directory: \"$PWD\"");
#endif

    cerr << "Tool started with: '";
    for (int i = 0; i < argc; i++)
    {
        if (i) cerr << " ";
        cerr << argv[i];
    }
    cerr << "'" << endl;

    unique_ptr <sa::FileOutputStream> logStream;

    {
        unique_ptr <sa::LogStreamHolder> logHolder;
        string applicationLogFileName = "application-log";

        try
        {
            logStream = sa::FileOutputStream::openOutputStream (applicationLogFileName, sa::RelativeOutputStreamFlags::BINARY);
            logHolder.reset (new sa::LogStreamHolder (logStream.get()));
            sa::ApplicationLogger::instance().setDuplicateToCerr (true);
        }
        catch (sa::InputOutputException& e)
        {
            cerr << "Failed to open application log file '" << applicationLogFileName << "':\n" << e.toString() << endl;
            return 1;
        }
        catch (...)
        {
            cerr << "Unknown exception trying to open application log file '" << applicationLogFileName << "'." << endl;
        }

        try
        {
            return loggedMain (argc, argv);
        }
        catch (...)
        {
            // FIXME: some temporary object to call this.
            sa::ApplicationLogger::instance().closeLog();
        }
    }

    return 1;
}
