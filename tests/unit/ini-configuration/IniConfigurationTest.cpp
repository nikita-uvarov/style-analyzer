#include "Common.h"
#include "IniConfiguration.h"
#include "ApplicationLog.h"
#include "FileStreams.h"
#include <boost/filesystem.hpp>

#include <cstdio>

using namespace sa;

unique_ptr <IniConfiguration> loadFromString (string str, string includeContents = "")
{
	IniIncludeManager includeManager;
	unique_ptr <IInputStream> iniFile = includeManager.openInputStream ("<memory buffer>", str);
    includeManager.addSystemHeader ("library", includeContents);
	unique_ptr <IniConfiguration> config (IniConfiguration::load ("<memory buffer>", iniFile.get(), &includeManager));
	return config;
}

unique_ptr <IniConfiguration> loadFromFile (string str)
{
	CHANGE_DIRECTORY();
	IniIncludeManager includeManager;
	unique_ptr <IInputStream> iniFile = includeManager.openInputStream (str, RelativeInputStreamFlags::NONE);
	unique_ptr <IniConfiguration> config (IniConfiguration::load (str, iniFile.get(), &includeManager));
	return config;
}

BOOST_AUTO_TEST_CASE (IniConfigurationLexical)
{
	// Different empty strings
	BOOST_CHECK_NO_THROW (loadFromString (""));
	BOOST_CHECK_NO_THROW (loadFromString ("\n\t\n\t\t\t\t   \n   \n\t\n"));

    auto invalidToken = [](IniConfigurationException e)
                        {
                            return e.getType() == IniConfigurationException::Type::INVALID_INI_TOKEN;
                        };

	// Inclusion
	BOOST_CHECK_EXCEPTION (loadFromString ("\n\n\n  #include <not-line-beginning>\n"), IniConfigurationException, invalidToken);
    BOOST_CHECK_NO_THROW (loadFromString ("\n\r\t\t\t   \n#include <library>\n#include \"library\"    \n"));
    BOOST_CHECK_NO_THROW (loadFromString ("\n\r\t\t\t   \n#   include <library>\n#  \t\tinclude \"library\"    \n"));

    // String literals
    BOOST_CHECK_NO_THROW (loadFromString ("key=\"value with \\\" escape sequence\\\"\\\\\\n \""));
    BOOST_CHECK_EXCEPTION (loadFromString ("a=\"\n\"\n"), IniConfigurationException, invalidToken);
    BOOST_CHECK_EXCEPTION (loadFromString ("a=\""), IniConfigurationException, invalidToken);

    // Identifiers
    BOOST_CHECK_NO_THROW (loadFromString ("key=\"value\""));
    BOOST_CHECK_EXCEPTION (loadFromString ("0key=\"\n\"\n"), IniConfigurationException, invalidToken);
    BOOST_CHECK_NO_THROW (loadFromString ("key0=\"value\""));
    BOOST_CHECK_NO_THROW (loadFromString ("___k__2342.34.eyWITHALLTYPESofLeTtErS0123456789=\"value\""));
    BOOST_CHECK_NO_THROW (loadFromString (".data=\"value\""));

    // Comments
    BOOST_CHECK_NO_THROW (loadFromString ("key = \"value\" // There is// no ;// ;;; ___ error here!! # ^@#$% 0 123"));
    BOOST_CHECK_NO_THROW (loadFromString ("key = \"value\" // There is// no ;// ;;; ___ error here!! # ^@#$% 0 123"));
    BOOST_CHECK_EXCEPTION (loadFromString ("key 0 = \"value\" ; There is// no ;// ;;; ___ error here!! # ^@#$% 0 123"), IniConfigurationException, invalidToken);
    BOOST_CHECK_EXCEPTION (loadFromString ("key 0 = \"value\" // There is// no ;// ;;; ___ error here!! # ^@#$% 0 123"), IniConfigurationException, invalidToken);
    BOOST_CHECK_EXCEPTION (loadFromString ("key = \"value\" // There is// no ;// ;;; ___ error here!! # ^@#$% 0 123\n0\n"), IniConfigurationException, invalidToken);

    // Operators
    BOOST_CHECK_NO_THROW (loadFromString ("key=\"value\""));
    BOOST_CHECK_NO_THROW (loadFromString ("key[]=\"value\""));
    BOOST_CHECK_NO_THROW (loadFromString ("[section.with.subsections]\nkey[]=\"value\"\n[]\nkey=\"newkey\""));

    // Compound test
    BOOST_CHECK_NO_THROW (loadFromFile ("data/syntax.ini"));
}

BOOST_AUTO_TEST_CASE (IniConfigurationSyntax)
{
    auto invalidSyntax = [](IniConfigurationException e)
                         {
                             return e.getType() == IniConfigurationException::Type::INVALID_INI_SYNTAX;
                         };

    // Valid examples checked in lexical part

    // Inclusion
    BOOST_CHECK_EXCEPTION (loadFromString ("\n\n\n#include <library> someshit\n"), IniConfigurationException, invalidSyntax);
    BOOST_CHECK_EXCEPTION (loadFromString ("\n\n\n#include\n<library>\n"), IniConfigurationException, invalidSyntax);
    BOOST_CHECK_EXCEPTION (loadFromString ("\n\n\n#include <include-not-found>\n"), IniConfigurationException, invalidSyntax);

    // Key-value pairs
    BOOST_CHECK_EXCEPTION (loadFromString ("key hello = \"test\""), IniConfigurationException, invalidSyntax);
    BOOST_CHECK_EXCEPTION (loadFromString ("key = \"test\" \"test\""), IniConfigurationException, invalidSyntax);
    BOOST_CHECK_EXCEPTION (loadFromString ("key = ="), IniConfigurationException, invalidSyntax);
    BOOST_CHECK_EXCEPTION (loadFromString ("key[ = \"value\""), IniConfigurationException, invalidSyntax);
    BOOST_CHECK_EXCEPTION (loadFromString ("key = value"), IniConfigurationException, invalidSyntax);

    // Sections
    BOOST_CHECK_EXCEPTION (loadFromString ("[\n]"), IniConfigurationException, invalidSyntax);
    BOOST_CHECK_EXCEPTION (loadFromString ("[\nhello]"), IniConfigurationException, invalidSyntax);
    BOOST_CHECK_EXCEPTION (loadFromString ("[hello\n]"), IniConfigurationException, invalidSyntax);
}

namespace std
{
    ostream& operator<< (std::ostream& stream, const std::vector <string>& vec)
    {
        printVector (stream, vec);
        return stream;
    }
}

BOOST_AUTO_TEST_CASE (IniConfigurationSemantics)
{
    auto invalidPropertyValue = [](IniConfigurationException e)
                                {
                                    return e.getType() == IniConfigurationException::Type::INVALID_INI_PROPERTY_VALUE;
                                };

    auto undefinedIniProperty = [](IniConfigurationException e)
                                {
                                    return e.getType() == IniConfigurationException::Type::UNDEFINED_INI_PROPERTY;
                                };

#define load(...) unique_ptr <IniConfiguration> configuration = loadFromString(__VA_ARGS__); IniConfiguration& config = *configuration;

    // Simple cases
    {
        load ("key=\"value\"");
        BOOST_CHECK_EQUAL (config["key"], "value");
    }
    {
        load ("key=\"value\"\nkey=\"reassign\"");
        BOOST_CHECK_EQUAL (config["key"], "reassign");
    }

    // Sections
    {
        load ("[section]\nkey=\"value\"\n[]\nkey=\"reassign\"");
        BOOST_CHECK_EQUAL (config["key"], "reassign");
        BOOST_CHECK_EQUAL (config["section.key"], "value");
    }

    // Multi-values
    {
        load ("key=\"one\"\nkey[]=\"two\"");
        BOOST_CHECK_EQUAL (config["key"].asVector(), (vector <string> { "one", "two" }));
    }
    {
        load ("key[]=\"one\"\nkey[]=\"two\"");
        BOOST_CHECK_EQUAL (config["key"].asVector(), (vector <string> { "one", "two" }));
    }
    {
        load ("key[]=\"one\"\nkey=\"two\"\nkey[]=\"three\"");
        BOOST_CHECK_EQUAL (config["key"].asVector(), (vector <string> { "two", "three" }));
    }

    // Escape sequences
    {
        load ("key=\"\\\"\\n\\\\\\\"\"");
        BOOST_CHECK_EQUAL (config["key"], "\"\n\\\"");
    }

    // Includes
    {
        load ("key=\"one\"\n#include <library>", "key=\"two\"");
        BOOST_CHECK_EQUAL (config["key"], "two");
    }

    // C++ API
    {
        load ("key=\"one\"");
        config["key"] = "two";
        config["key"] = "three";
        config["key"].push_back ("four");
        BOOST_CHECK_EQUAL (config["key"].asVector(), (vector <string> { "three", "four" }));

        bool test = false;
        BOOST_CHECK_EXCEPTION (test = config["key"] == "hello", IniConfigurationException, invalidPropertyValue);

        BOOST_CHECK_EXCEPTION (test = config["notfound"] == "hello", IniConfigurationException, undefinedIniProperty);
    }

#undef load
}
