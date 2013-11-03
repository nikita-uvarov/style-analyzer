#include "Common.h"
#include "StringFormatter.h"
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <cstdio>

using namespace sa;

BOOST_AUTO_TEST_CASE (StringFormatterBasicTypes)
{
    // Simple logic in correct usecases
    BOOST_CHECK_EQUAL (string (saFormatPure ("hello, %1") << "world"), "hello, world");
    BOOST_CHECK_EQUAL (string (saFormatPure ("%2%1") << "random" << "shuffle"), "shufflerandom");

    vector <string> vectorOfStrings = { "a", "b" };
    BOOST_CHECK_EQUAL (string (saFormatPure ("%1%1%1") << vectorOfStrings), "{ \"a\", \"b\" }{ \"a\", \"b\" }{ \"a\", \"b\" }");

    // Escape sequences
    BOOST_CHECK_EQUAL (string (saFormatPure ("%%%%hello%%")), "%%hello%");
}
