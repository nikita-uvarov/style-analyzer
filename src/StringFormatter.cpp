#include "StringFormatter.h"
#include "Debug.h"

using namespace sa;

sa::IAfterformatAction::~IAfterformatAction()
{}

sa::IFormattable::~IFormattable()
{}

FormattableString::~FormattableString()
{}

FormattableVector::~FormattableVector()
{}

sa::FormatObjectsHolder::FormatObjectsHolder (std::string formatString, sa::IAfterformatAction* action) :
    action (action), formatString (formatString)
{}

sa::FormatObjectsHolder::FormatObjectsHolder()
{}

sa::FormatObjectsHolder::operator string()
{
    return format();
}

sa::FormatObjectsHolder::~FormatObjectsHolder()
{
    if (action)
        action->fire (format());
}

FormatObjectsHolder sa::FormatObjectsHolder::operator<< (unique_ptr <IFormattable> argument)
{
    FormatObjectsHolder newObject (move (*this));
    newObject.arguments.push_back (move (argument));
    return move (newObject);
}

std::string sa::FormatObjectsHolder::format()
{
    string result = "";

    bool wasPercent = false;
    for (char c: formatString)
    {
        if (c == '%')
        {
            if (wasPercent)
            {
                wasPercent = false;
                result += '%';
            }
            else
            {
                wasPercent = true;
            }

            continue;
        }

        if (wasPercent)
        {
            unsigned number = static_cast <unsigned char> (c) - '1';
            saAssert (number >= 0 && number < arguments.size());

            result += formatSimple (arguments[number].get());
            wasPercent = false;
            continue;
        }

        result += c;
    }
    return result;
}

string sa::FormatObjectsHolder::formatSimple (IFormattable* object)
{
    saAssert (object);

    if (FormattableString* theString = dynamic_cast <FormattableString*> (object))
    {
        return theString->contents;
    }
    else if (FormattableVector* theVector = dynamic_cast <FormattableVector*> (object))
    {
        string result = "{ ";
        if (theVector->elements.empty())
            result += "empty";

        for (unsigned i = 0; i < theVector->elements.size(); i++)
        {
            if (i > 0)
                result += ", ";
            result += '"';
            result += formatSimple (theVector->elements[i].get());
            result += '"';
        }
        result += " }";
        return result;
    }

    saUnreachable ("Not supported IFormattable instance.");
}

unique_ptr <IFormattable> sa::wrapFormattable (string s)
{
    unique_ptr <IFormattable> result (new FormattableString);
    FormattableString* theString = dynamic_cast <FormattableString*> (result.get());
    theString->contents = s;
    return result;
}

unique_ptr <IFormattable> sa::wrapFormattable (int x)
{
    unique_ptr <IFormattable> result (new FormattableString);
    FormattableString* theString = dynamic_cast <FormattableString*> (result.get());
    std::stringstream s;
    s << x;
    theString->contents = s.str();
    return result;
}
