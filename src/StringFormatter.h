#ifndef STYLE_ANALYZER_STRING_FORMATTER_H
#define STYLE_ANALYZER_STRING_FORMATTER_H

/* Provides string formatting functions to use in the whole project.

   Different modules need to produce string output to logs, html and so on.
   A unified interface of the following form is used:

   macro ("format string: %d %s") << objectOne << objectTwo

   This module comes in combination with logs and internationalization modules:
   saFormatPure ("something") << objects  - constructs pure format string
   saFormat ("something") << objects      - interacts with i18n module to translate format string
   saLog ("something") << objects;        - log entry + translation
   saError ("something") << objects;      - error entry (see ApplicationLog.h) + translation

   Module internals:
   - global function wrapFormattable should construct a wrapper around every acceptable object type (see IFormattable)
   - format strings do not longer define the type of accepted object, but how to handle it
   - format strings contain index of passed object to operate on (e. g. %1)
   - later clang-like formatting system may be used
     (see http://clang.llvm.org/docs/InternalsManual.html, "The Diagnostics Subsystem")
*/

#include <memory>
#include <vector>

#include "Internationalization.h"

namespace sa
{
using namespace std;

class IFormattable
{
public :
    virtual ~IFormattable();
};

class FormattableVector : public IFormattable
{
public :
    vector < unique_ptr <IFormattable> > elements;

    ~FormattableVector();
};

class FormattableString : public IFormattable
{
public :
    string contents;

    ~FormattableString();
};

unique_ptr <IFormattable> wrapFormattable (string s);
unique_ptr <IFormattable> wrapFormattable (int x);

template <class T>
unique_ptr <IFormattable> wrapFormattable (vector <T>& object)
{
    unique_ptr <IFormattable> result (new FormattableVector);
    FormattableVector* theVector = dynamic_cast <FormattableVector*> (result.get());
    for (auto& element: object)
        theVector->elements.push_back (move (wrapFormattable (element)));
    return result;
}

class IAfterformatAction
{
public :
    virtual ~IAfterformatAction();
    virtual void fire (string formattedString) = 0;
};

class FormatObjectsHolder
{
public :
    FormatObjectsHolder (string formatString, IAfterformatAction* action);
    ~FormatObjectsHolder();

    operator string();

    template <class T>
    FormatObjectsHolder operator<< (T argument)
    {
        return move (*this << wrapFormattable (argument));
    }

    FormatObjectsHolder operator<< (unique_ptr <IFormattable> argument);

private :
    FormatObjectsHolder();
    FormatObjectsHolder (FormatObjectsHolder&&) = default;

    unique_ptr <IAfterformatAction> action;
    string formatString;
    vector < unique_ptr <IFormattable> > arguments;

    string format();
    string formatSimple (IFormattable* object);
};

#define saFormatPure(x) sa::FormatObjectsHolder (x, nullptr)
#define saFormat(x) sa::FormatObjectsHolder (saTranslate (x), nullptr)

}

#endif // STYLE_ANALYZER_STRING_FORMATTER_H
