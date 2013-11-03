#include "Debug.h"

using namespace std;

sa::Exception::~Exception() {}

string sa::InvalidArgumentException::toString() const
{
    if (argument.empty())
        return "Invalid argument '" + argument + "': " + description;
    else
        return "Invalid argument: " + description;
}

string sa::AssertionFailure::toString() const
{
    return "Assertion failed: '" + condition + "'.";
}

string sa::UnreachableCodeFailure::toString() const
{
    return "Normally unreachable code executed: " + description;
}

string sa::FileNotFoundException::toString() const
{
    return "File not found: '" + path + "' " + context;
}
