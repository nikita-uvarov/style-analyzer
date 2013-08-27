/* Ini-like format configuration parser.

   Supported .ini files features:
   - comments via ; (original ini), // (C++; no multi-line comments).
   - strings & C++ escape sequences to correctly represent reserved characters:
     ; exampleKey = hello       ; not allowed
     exampleKey = "   hello   " ; to include spaces
	 exampleKey = "hello\nworld!"
   - redefinitions: second definition overrides first.
     key = "smth"
	 key = "overriden"
   - multiple values: array symbol appends definition:
     exampleKey = first # exampleKey[] possible too, unless this setting overrides default
     exampleKey[] = second
     exampleKey[] = third
   - equal sign is not the only operator:
     description unite "\n" ; NOT IMPLEMENTED YET
   - sections:
     []        # Global section
     key = "x" # 'key' symbol defined
     [a]       # Section a
     key = "x" # 'a.key' symbol defined
     [a.b]     # Section a.b
     key = "x" # 'a.b.key' symbol defined
   - include directives in C-style.

   C++ interface features:
   - 'override' methods:
     typedef ... config;
	 config derived = default;
     derived.overrideWith (loaded);
   - smart bracket operator (via helper object):
     cfg["a"] = "b";
	 cfg["a"].push_back ("c");
   - require methods
     cfg["a"].requireSingle()
   - issues list & exceptions. Warnings (ignored value) if no errors.
*/

#ifndef STYLE_ANALYZER_INI_CONFIGURATION_H
#define STYLE_ANALYZER_INI_CONFIGURATION_H

#include <map>
#include <string>
#include <memory>
#include <vector>
#include <iostream>

#include "Streams.h"

// TODO: Relative paths support library

namespace sa
{
	using std::map;
	using std::string;
	using std::vector;
	using std::unique_ptr;
	using std::pair;
    using std::ostream;

	typedef unsigned IniFileId;

	struct IniFileLocation
	{
		IniFileId fileId;
		unsigned line;

		bool isValid()
		{
			return line > 0;
		}

		static IniFileLocation invalidLocation();
	};

	template <typename T>
	const T& assign (class IniProperty& p, const T& rhs);

	class IniConfiguration;

	class IniProperty
	{
		vector <IniFileLocation> valuesDefinedAt;
		bool used;

	public :
		string key;
		vector <string> values;

		class Accessor
		{
			IniConfiguration& parent;
			string key;

            friend ostream& operator<< (ostream&, const Accessor&);

            const IniProperty& getProperty() const;
            IniProperty& getProperty();
		public :

			Accessor (IniConfiguration& parent, string key);

            operator string() const;

            template <typename T>
            bool operator== (T& x) const
            {
                return operator string() == toString (x);
            }

            const vector <string>& asVector() const;
            int asInteger() const;

            template <typename T>
            friend const T& assign (IniProperty& p, const T& rhs)
            {
                p.values.assign (1, toString (rhs));
                p.valuesDefinedAt.push_back (IniFileLocation::invalidLocation());
                p.used = false;

                return rhs;
            }

			template <typename T>
			const T& operator= (const T& rhs)
			{
				return sa::assign <T> (getProperty(), rhs);
			}

            template <typename T>
            void push_back (const T& value)
            {
                IniProperty& p = getProperty();

                p.values.push_back (toString (value));
                p.valuesDefinedAt.push_back (IniFileLocation::invalidLocation());
                p.used = false;
            }

            template <typename T>
            void push_back (const T& value, bool overwrite, IniFileLocation location)
            {
                IniProperty& p = getProperty();

                saAssert (p.values.size() == p.valuesDefinedAt.size());

                if (overwrite)
                {
                    p.values.clear();
                    p.valuesDefinedAt.clear();
                }

                p.values.push_back (toString (value));
                p.valuesDefinedAt.push_back (location);
                p.used = false;
            }

            string resolveRelativePath (unsigned index, string transformedPath);
		};
	};

	enum class IniRelativeInputStreamFlags : uint32_t
	{
		// File included by #include "..."
		LOCAL_INCLUSION,

		// File included by #include <...>
		GLOBAL_INCLUSION
	};

	class IniConfiguration
	{
		friend class IniProperty::Accessor;
        friend class IniParser;

	public :
		IniProperty::Accessor operator[] (const string& key);

        // TODO: Make it save file info for entries coming from file
		void overrideWith (IniConfiguration& other);

		// Issue messages would contain 'on line <..> of <iniSourceName>'
		// Include manager would be used for opening input streams only
		// void* passed to include manager would point to IniRelativeInputStreamFlags.
		static unique_ptr <IniConfiguration> load (string iniSourceName, IInputStream* stream, IRelativeStreamsManager* includeManager);

	private :
		class IniToken
		{
		public :
			enum class Type
			{
				// a key or identifier-like operator (e. g. '<key> unite <separator>' construction)
				// identifiers may contain: A-Z, a-z, 0-9, ., _
				// 'contents' contains identifier
				IDENTIFIER,
				// =, 'contents' empty
				ASSIGNMENT_OPERATOR,
				// quoted string, 'contents' contains string with escapes replaced
				STRING_LITERAL,
				// string quoted with '<>', used in includes, 'contents' contains string with escapes replaced
				INCLUSION_LITERAL,
				// [, 'contents' empty
				OPEN_BRACKET_OPERATOR,
				// ], 'contents' empty
				CLOSE_BRACKET_OPERATOR,
				// #, 'contents' empty. If not first character of line, invalid token exception is thrown.
				SHARP_INCLUDE_OPERATOR,
				// end of line/file marker
				NEWLINE
			};

			Type type;
			string contents;
			IniFileLocation location;
			unique_ptr <IniToken> next;
		};

		static constexpr bool isIdentifierSymbol (char c, bool first);

		void appendToken (IniToken*& appendTo, IniToken::Type type, string contents, IniFileId fileId, unsigned line);

		string getSourceString (unsigned line, IniFileId fileId);

		ATTRIBUTE_NORETURN void invalidToken (string description, IniFileId fileId, unsigned line);

		map <string, IniProperty> properties;

		vector <string> fileNamesUsed;

		// The following property is helf: file id of included file is more than file id of parent
		// If it is not true, the file is 'root' of inclusions
		vector <IniFileLocation> includedWith;

		unique_ptr <IniToken> tokenizeStreamContents (IInputStream* stream, IniFileId fileId);

		unique_ptr <IniToken> tokenizeStringContents (string contents, IniFileId iniFileId);
	};

	class IniConfigurationException : public Exception
	{
	public :
		enum class Type
		{
			INVALID_INI_TOKEN,
			INVALID_INI_SYNTAX,
			INVALID_INI_PROPERTY_VALUE,
			UNDEFINED_INI_PROPERTY
		};

		IniConfigurationException (const char* fileOrigin, int lineOrigin, const char* functionOrigin, Type type, string description) :
			Exception (fileOrigin, lineOrigin, functionOrigin, description), type (type), description (description)
		{}

		Type getType() const;

		string toString() const;

	private :
		Type type;
		string description;
	};

	class IniIncludeManager : public IRelativeStreamsManager
	{
	public :
		IniIncludeManager() = default;
		~IniIncludeManager();

		StreamId getInputStreamId (IInputStream* stream);
		StreamId getOutputStreamId (IOutputStream* stream);

		// nameTypeFlagsPointer points to IniRelativeInputStreamFlags object
		unique_ptr <IInputStream> openInputStream (StreamId relativeTo, string relativeName, void* nameTypeFlagsPointer, RelativeInputStreamFlags flags);

		unique_ptr <IInputStream> openInputStream (string fileName, RelativeInputStreamFlags flags);
		unique_ptr <IInputStream> openInputStream (string fileName, string fileContents);

		void addIncludePath (string includePath);
		void addSystemHeader (string headerName, string headerContents);

		// throws runtime error, not implemented
		unique_ptr <IOutputStream> openOutputStream (StreamId relativeTo, string relativeName, void* nameTypeFlagsPointer, RelativeOutputStreamFlags flags);
	private :
		struct IniInputStream
		{
			IInputStream* stream;
			string sourceDirectory;

			IniInputStream (IInputStream* stream, string sourceDirectory) :
				stream (stream), sourceDirectory (sourceDirectory)
			{}

			bool isRealFileStream()
			{
				return sourceDirectory != "";
			}
		};

		vector <string> includePaths;
		map <string, string> systemHeaders;

		// Not guaranteed to be valid: ownership belongs to user
		vector <IniInputStream> inputStreamsOpened;
		map <IInputStream*, StreamId> inputStreamToId;

		// If relative path exists, returns it. Otherwise returns empty string.
		string tryRelative (string somePath, string subPath);

		StreamId addStream (IInputStream* stream, string fromFile);
		IniInputStream& getStream (StreamId id);
	};

    ostream& operator<< (ostream& stream, const IniProperty::Accessor& accessor);
}

#endif // STYLE_ANALYZER_INI_CONFIGURATION_H
