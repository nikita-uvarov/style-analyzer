#ifndef STYLE_ANALYZER_DEBUG_H
#define STYLE_ANALYZER_DEBUG_H

#include "Utilities.h"

#include <string>
#include <stdexcept>

namespace sa
{

using std::string;

class Exception : public std::runtime_error
{
public :

    const string& getFileOrigin() const
    {
        return fileOrigin;
    }

    const string& getFunctionOrigin() const
    {
        return functionOrigin;
    }

    int getLineOrigin() const
    {
        return lineOrigin;
    }

    string originToString()
    {
        return "'" + getFileOrigin() + ":" + sa::toString (getLineOrigin()) + "' at '" + getFunctionOrigin() + "'";
    }

    virtual string toString() const = 0;

protected :
    Exception (const char* fileOrigin, int lineOrigin, const char* functionOrigin, string what) :
        runtime_error (("\n" + string (fileOrigin) + ":" + sa::toString (lineOrigin) + ": " + what).c_str()),
        fileOrigin (fileOrigin), functionOrigin (functionOrigin), lineOrigin (lineOrigin)
    {}

    virtual ~Exception();

private :
    string fileOrigin, functionOrigin;
    int lineOrigin;
};

#define __ORIGIN__ __FILE__, __LINE__, __PRETTY_FUNCTION__

class AssertionFailure : public Exception
{
public :
    AssertionFailure (const char* fileOrigin, int lineOrigin, const char* functionOrigin, const char* condition) :
        Exception (fileOrigin, lineOrigin, functionOrigin,
                   "Assertion failed: " + string (condition)), condition (condition)
    {}

    const string& getCondition() const
    {
        return condition;
    }

    string toString() const;

private :
    string condition;
};

class UnreachableCodeFailure : public Exception
{
public :
    UnreachableCodeFailure (const char* fileOrigin, int lineOrigin, const char* functionOrigin, const char* description) :
        Exception (fileOrigin, lineOrigin, functionOrigin,
                   "Normally unreachable code executed: " + string (description)), description (description)
    {}

    const string& getDescription() const
    {
        return description;
    }

    string toString() const;

private :
    string description;
};

class InvalidArgumentException : public Exception
{
public :
    InvalidArgumentException (const char* fileOrigin, int lineOrigin, const char* functionOrigin,
                              string description, string argument = "") :
        Exception (fileOrigin, lineOrigin, functionOrigin,
                   "Invalid argument '" + string (argument) + "': " + description),
        description (description), argument (argument)
    {}

    const string& getDescription() const
    {
        return description;
    }

    const string& getArgument() const
    {
        return argument;
    }

    string toString() const;

private :
    string description, argument;
};

class FileNotFoundException : public Exception
{
public :
    FileNotFoundException (const char* fileOrigin, int lineOrigin, const char* functionOrigin, string path, string context) :
        Exception (fileOrigin, lineOrigin, functionOrigin, "File not found: '" + string (path) + "' " + context),
        context (context), path (path)
    {}

    const string& getContext() const
    {
        return context;
    }

    const string& getPath() const
    {
        return path;
    }

    string toString() const;

private :
    string context, path;
};

#ifndef IN_KDEVELOP_PARSER
#   define throwAssertionFailure(str) throw sa::AssertionFailure (__ORIGIN__, str)
#else
#   define throwAssertionFailure(str) (str)
#endif

#ifndef IN_KDEVELOP_PARSER
#   define ATTRIBUTE_NORETURN [[ noreturn ]]
#else
#   define ATTRIBUTE_NORETURN
#endif

#define saAssert(x) ((x) ? true : throwAssertionFailure (#x))
#define saUnreachable(reason) throw UnreachableCodeFailure (__ORIGIN__, reason)

}

#endif // STYLE_ANALYZER_DEBUG_H
