#include "IniConfiguration.h"
#include "FileSystem.h"
#include "FileStreams.h"

using namespace sa;
using namespace std;

IniProperty::Accessor IniConfiguration::operator[] (const string& key)
{
	return IniProperty::Accessor (*this, key);
}

IniProperty::Accessor::Accessor (IniConfiguration& parent, string key) :
	parent (parent), key (key)
{}

const IniProperty& IniProperty::Accessor::getProperty() const
{
    if (parent.properties.count (key) != 1)
        throw IniConfigurationException (__ORIGIN__, IniConfigurationException::Type::UNDEFINED_INI_PROPERTY,
                                         "Undefined property '" + key + "' read attempt.");
    return parent.properties[key];
}

IniProperty& IniProperty::Accessor::getProperty()
{
    parent.properties[key].used = true;
    return parent.properties[key];
}

const vector <string>& IniProperty::Accessor::asVector() const
{
    return getProperty().values;
}

int IniProperty::Accessor::asInteger() const
{
    stringstream t (*this);
    int number;
    t >> number;
    return number;
}

IniProperty::Accessor::operator string() const
{
    const vector <string>& values = getProperty().values;
    if (values.size() != 1)
        throw IniConfigurationException (__ORIGIN__, IniConfigurationException::Type::INVALID_INI_PROPERTY_VALUE, "Single value expected.");
    return values[0];
}

ostream& sa::operator<< (ostream& stream, const IniProperty::Accessor& accessor)
{
    printVector (stream, accessor.asVector());
    return stream;
}

IniConfigurationException::Type IniConfigurationException::getType() const
{
	return type;
}

string IniConfigurationException::toString() const
{
	return description;
}

unique_ptr <IniConfiguration::IniToken> IniConfiguration::tokenizeStreamContents (IInputStream* stream, IniFileId fileId)
{
	auto fileLength = stream->getNumBytesRemaining();
	unique_ptr <char[]> buffer (new char[fileLength + 1]);
	buffer[fileLength] = 0;
	auto nRead = stream->read (buffer.get(), fileLength);
	saAssert (nRead == fileLength);

	string fileContents (buffer.get());
	unique_ptr <IniConfiguration::IniToken> rawTokens = tokenizeStringContents (fileContents, fileId);
	return rawTokens;
}

namespace sa
{
    class IniParser
    {
    public :
        typedef IniConfiguration::IniToken Token;

        unique_ptr <Token> firstToken;
        IniConfiguration& configuration;

        Token* current;
        bool isUngotToken;

        string currentSection;

        IniParser (IniConfiguration& configuration, unique_ptr <Token> first) :
            firstToken (move (first)), configuration (configuration), current (nullptr), isUngotToken (false)
        {}

        void preprocess (IRelativeStreamsManager* includeManager, IInputStream* sourceStream)
        {
            saAssert (configuration.fileNamesUsed.size() == 1);
            saAssert (configuration.fileNamesUsed.size() == configuration.includedWith.size());

            vector < unique_ptr <IInputStream> > childStreams;

            Token* previous = nullptr;
            Token* now = firstToken.get();

            while (now)
            {
                if (now->type == Token::Type::SHARP_INCLUDE_OPERATOR)
                {
                    Token* inclusionKeyword = now->next.get();
                    if (!inclusionKeyword || inclusionKeyword->type != Token::Type::IDENTIFIER ||
                        inclusionKeyword->contents != "include")
                        syntaxError (now, "Expected 'include' keyword after sharp symbol.");

                    Token* fileName = inclusionKeyword->next.get();
                    if (!fileName || (fileName->type != Token::Type::INCLUSION_LITERAL &&
                                      fileName->type != Token::Type::STRING_LITERAL))
                        syntaxError (now, "Expected inclusion filename.");

                    Token* newLine = fileName->next.get();
                    if (!newLine || newLine->type != Token::Type::NEWLINE)
                        syntaxError (fileName, "Unexpected tokens after inclusion directive.");

                    string includeWhat = fileName->contents;
                    bool localInclusion = fileName->type == Token::Type::INCLUSION_LITERAL;

                    IniFileId fileId = fileName->location.fileId;
                    IInputStream* tokenStream = fileId ? childStreams[fileId - 1].get() : sourceStream;

                    IniRelativeInputStreamFlags flags = localInclusion ?
                        IniRelativeInputStreamFlags::LOCAL_INCLUSION : IniRelativeInputStreamFlags::GLOBAL_INCLUSION;

                    unique_ptr <IInputStream> relative = includeManager->openInputStream
                        (includeManager->getInputStreamId (tokenStream), includeWhat, &flags, RelativeInputStreamFlags::NONE);

                    if (!relative)
                        throw IniConfigurationException (__ORIGIN__, IniConfigurationException::Type::INVALID_INI_SYNTAX,
                                                         "Include file not found: " + string (localInclusion ? "\"" : "<") +
                                                         includeWhat + string (localInclusion ? "\"" : ">") + ".");

                    if (UniversalInputStream* universal = dynamic_cast <UniversalInputStream*> (relative.get()))
                        configuration.fileNamesUsed.push_back
                            (universal->isFileStream() ? universal->getSourceFileName() : universal->getSourceBufferName());
                    else
                        configuration.fileNamesUsed.push_back ("<unknown source>");

                    configuration.includedWith.push_back (fileName->location);

                    IniFileId newId = static_cast <IniFileId> (configuration.includedWith.size() - 1);
                    unique_ptr <Token> includedStream = configuration.tokenizeStreamContents (relative.get(), newId);

                    childStreams.push_back (move (relative));

                    getLastToken (includedStream.get())->next = move (newLine->next);

                    now = firstToken.get();

                    if (previous)
                        previous->next = move (includedStream);
                    else
                        firstToken = move (includedStream);
                }
                else
                {
                    previous = now;
                    now = now->next.get();
                }
            }

        }

        static Token* getLastToken (Token* list)
        {
            saAssert (list);
            while (list->next)
                list = list->next.get();
            return list;
        }

        void parseConfiguration()
        {
            current = firstToken.get();

            while (current)
                parseLine();
        }

        void parseLine()
        {
            if (current->type == Token::Type::NEWLINE)
                return void (getNextToken());

            if (current->type == Token::Type::OPEN_BRACKET_OPERATOR)
                return void (parseSectionHeaderLine());
            else
                return void (parseKeyValueLine());
        }

        void parseSectionHeaderLine()
        {
            expected (Token::Type::OPEN_BRACKET_OPERATOR);

            if (current->type == Token::Type::IDENTIFIER)
                currentSection = expected (Token::Type::IDENTIFIER)->contents;
            else
                currentSection = "";

            expected (Token::Type::CLOSE_BRACKET_OPERATOR);
            expected (Token::Type::NEWLINE);
        }

        void parseKeyValueLine()
        {
            string keyName = expected (Token::Type::IDENTIFIER)->contents;

            bool append = false;
            if (current->type == Token::Type::OPEN_BRACKET_OPERATOR)
            {
                getNextToken();
                expected (Token::Type::CLOSE_BRACKET_OPERATOR);
                append = true;
            }

            expected (Token::Type::ASSIGNMENT_OPERATOR);

            string value = expected (Token::Type::STRING_LITERAL)->contents;
            IniFileLocation location = expected (Token::Type::NEWLINE)->location;

            if (!currentSection.empty())
                keyName = currentSection + "." + keyName;

            configuration[keyName].push_back (value, !append, location);
        }

        Token* expected (Token::Type tokenType)
        {
            saAssert (current);
            if (current->type != tokenType)
                syntaxError (current, "Expected " + toString (tokenType) + ", " + toString (current->type) + " met.");

            Token* was = current;
            getNextToken();
            return was;
        }

        void ungetToken()
        {
            saAssert (!isUngotToken);
            isUngotToken = true;
        }

        Token* getNextToken()
        {
            if (isUngotToken)
            {
                isUngotToken = false;
                return current;
            }

            saAssert (current);
            current = current->next.get();
            return current;
        }

        ATTRIBUTE_NORETURN void syntaxError (Token* after, string description)
        {
            saAssert (after);
            description = "Invalid ini file syntax: " + description + "\n" + configuration.getSourceString (after->location.line, after->location.fileId);
            throw IniConfigurationException (__ORIGIN__, IniConfigurationException::Type::INVALID_INI_SYNTAX, description);
        }

        static string toString (Token::Type t)
        {
            switch (t)
            {
                case Token::Type::ASSIGNMENT_OPERATOR: return "assignment operator";
                case Token::Type::CLOSE_BRACKET_OPERATOR: return "closing bracket";
                case Token::Type::IDENTIFIER: return "identifier";
                case Token::Type::INCLUSION_LITERAL: return "inclusion literal";
                case Token::Type::NEWLINE: return "newline";
                case Token::Type::OPEN_BRACKET_OPERATOR: return "opening bracket";
                case Token::Type::SHARP_INCLUDE_OPERATOR: return "sharp symbol";
                case Token::Type::STRING_LITERAL: return "string literal";
            }
            saUnreachable ("Unknown IniToken type.");
        }
    };
}

unique_ptr <IniConfiguration> IniConfiguration::load (string iniSourceName, IInputStream* stream, IRelativeStreamsManager* includeManager)
{
	unique_ptr <IniConfiguration> configuration (new IniConfiguration);

	configuration->fileNamesUsed.push_back (iniSourceName);
	IniFileId mainFileId = static_cast <IniFileId> (0);
	configuration->includedWith.push_back (IniFileLocation { mainFileId, 0 });

	unique_ptr <IniConfiguration::IniToken> tokens = configuration->tokenizeStreamContents (stream, mainFileId);
    IniParser parser (*configuration.get(), move (tokens));
    parser.preprocess (includeManager, stream);
    parser.parseConfiguration();

	return configuration;
}

void IniConfiguration::appendToken (IniConfiguration::IniToken*& appendTo, IniConfiguration::IniToken::Type type, string contents, IniFileId fileId, unsigned line)
{
	saAssert (appendTo);
	appendTo->next.reset (new IniConfiguration::IniToken);
	appendTo = appendTo->next.get();

	appendTo->type = type;
	appendTo->location = IniFileLocation { fileId, line };
	appendTo->contents = contents;
}

string IniConfiguration::getSourceString (unsigned int line, IniFileId fileId)
{
	string sourceString = "at " + fileNamesUsed[fileId] + ":" + toString (line);
	while (fileId)
	{
		line = includedWith[fileId].line;
		fileId = includedWith[fileId].fileId;
		sourceString += "\nincluded by " + fileNamesUsed[fileId] + ":" + toString (line);
	}
	return sourceString;
}

ATTRIBUTE_NORETURN void IniConfiguration::invalidToken (string description, IniFileId fileId, unsigned int line)
{
	description = "Invalid ini file token: " + description + "\n" + getSourceString (line, fileId);
	throw IniConfigurationException (__ORIGIN__, IniConfigurationException::Type::INVALID_INI_TOKEN, description);
}

constexpr bool IniConfiguration::isIdentifierSymbol (char c, bool first)
{
	return (!first && (c >= '0' && c <= '9')) || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_') || (c == '.');
}

unique_ptr <IniConfiguration::IniToken> IniConfiguration::tokenizeStringContents (string contents, IniFileId fileId)
{
	const char* buffer = contents.c_str();
	const char* current = buffer;

	unique_ptr <IniToken> tokenStream (new IniToken { IniToken::Type::NEWLINE, "", IniFileLocation { fileId, 0 }, nullptr });
	IniToken* lastToken = tokenStream.get();

	unsigned currentLine = 1;
	const char* lastLineBeginning = current;

	while (true)
	{
		while (true)
		{
			char c = *current;
			if (c == ' ' || c == '\t' || c == '\r')
				current++;
			else
				break;
		}

		if (*current == ';' || *current == '/')
		{
			if (*current == '/' && *(current + 1) != '/')
				invalidToken ("'//' expected after '/' met.", fileId, currentLine);

			while (*current != '\n' && *current != '\0')
				current++;
		}

		if (*current == '\n' || *current == '\0')
		{
			currentLine++;
			appendToken (lastToken, IniToken::Type::NEWLINE, "", fileId, currentLine);
			if (!*current)
				break;
			current++;
			lastLineBeginning = current;
		}
		else if (*current == '#')
		{
			if (current != lastLineBeginning)
				invalidToken ("'#' met not in the beginning of the line.", fileId, currentLine);

			appendToken (lastToken, IniToken::Type::SHARP_INCLUDE_OPERATOR, "", fileId, currentLine);
			current++;
		}
		else if (*current == '=' || *current == '[' || *current == ']')
		{
			IniToken::Type type = IniToken::Type::ASSIGNMENT_OPERATOR;
			if (*current == '[') type = IniToken::Type::OPEN_BRACKET_OPERATOR;
			if (*current == ']') type = IniToken::Type::CLOSE_BRACKET_OPERATOR;

			appendToken (lastToken, type, "", fileId, currentLine);
			current++;
		}
		else if (isIdentifierSymbol (*current, true))
		{
			string contents = "";
			contents += *current;
			current++;

			while (isIdentifierSymbol (*current, false))
				contents += *(current++);

			appendToken (lastToken, IniToken::Type::IDENTIFIER, contents, fileId, currentLine);
		}
		else if (*current == '<' || *current == '"')
		{
			string contents = "";

			bool inclusion = *current == '<';
			current++;

			bool escaped = false;
			while (true)
			{
				char c = *current;
				current++;

				if (c == '\n' || c == '\0')
					invalidToken ("Unclosed " + string (inclusion ? "inclusion" : "string") + " literal: " +
				    			  string (c == '\0' ? "EOF" : "newline") + " met.", fileId, currentLine);

				if (escaped)
				{
					if (c == 'n')
						c = '\n';
					else if (!(c == '\\' || c == '>' || c == '"'))
						invalidToken (string ("Invalid escape sequence '\\") + c + "'.", fileId, currentLine);

					contents += c;
					escaped = false;
					continue;
				}

				if ((inclusion && c == '>') || (!inclusion && c == '"'))
					break;

				if (c == '\\')
				{
					escaped = true;
					continue;
				}

				contents += c;
			}

			appendToken (lastToken, inclusion ? IniToken::Type::INCLUSION_LITERAL : IniToken::Type::STRING_LITERAL, contents, fileId, currentLine);
		}
		else
		{
			invalidToken (string ("A token can not begin with character '") + *current
			              + "' (code " + toString (*reinterpret_cast <const unsigned char*> (current)) + ".", fileId, currentLine);
		}
	}

	return tokenStream;
}

IniIncludeManager::~IniIncludeManager() {}

unique_ptr <IOutputStream> IniIncludeManager::openOutputStream (IRelativeStreamsManager::StreamId, string, void*, RelativeOutputStreamFlags)
{
	saUnreachable ("'openOutputStream' is not supported by IniIncludeManager objects.");
}

IRelativeStreamsManager::StreamId IniIncludeManager::getOutputStreamId (IOutputStream*)
{
	throw sa::InvalidArgumentException (__ORIGIN__, "Output stream was not allocated by this manager.", "stream");
}

IRelativeStreamsManager::StreamId IniIncludeManager::getInputStreamId (IInputStream* stream)
{
	auto inputStreamIt = inputStreamToId.find (stream);
	if (inputStreamIt == inputStreamToId.end())
		throw sa::InvalidArgumentException (__ORIGIN__, "Input stream was not allocated by this manager.", "stream");

	return inputStreamIt->second;
}

void IniIncludeManager::addIncludePath (string includePath)
{
	includePaths.push_back (includePath);
}

void IniIncludeManager::addSystemHeader (string headerName, string headerContents)
{
	if (systemHeaders.find (headerName) != systemHeaders.end())
		throw sa::InvalidArgumentException (__ORIGIN__, "System header '" + headerName + "' is already present.");

	systemHeaders[headerName] = headerContents;
}

unique_ptr <IInputStream> IniIncludeManager::openInputStream (IRelativeStreamsManager::StreamId relativeTo, string relativeName, void* /*nameTypeFlagsPointer*/, RelativeInputStreamFlags flags)
{
	auto systemHeaderIt = systemHeaders.find (relativeName);
	if (systemHeaderIt != systemHeaders.end())
	{
		unique_ptr <IInputStream> stream (UniversalInputStream::openInputStream (systemHeaderIt->first, systemHeaderIt->second));
		return stream;
	}

	for (unsigned i = 0; i <= includePaths.size(); i++)
	{
		string directory = "";
		if (i == 0)
			directory = getStream (relativeTo).sourceDirectory;
		else
			directory = includePaths[i - 1];

		string relative = tryRelative (directory, relativeName);
		if (FileSystem::instance().fileExists (relative))
		{
			unique_ptr <IInputStream> stream (UniversalInputStream::openInputStream (relative, flags));
			addStream (stream.get(), relative);
			return stream;
		}
	}

	return nullptr;
}

unique_ptr <IInputStream> IniIncludeManager::openInputStream (string fileName, RelativeInputStreamFlags flags)
{
	unique_ptr <IInputStream> stream (UniversalInputStream::openInputStream (fileName, flags));
    addStream (stream.get(), fileName);
	return stream;
}

unique_ptr <IInputStream> IniIncludeManager::openInputStream (string fileName, string fileContents)
{
	unique_ptr <IInputStream> stream (UniversalInputStream::openInputStream (fileName, fileContents));
    addStream (stream.get(), fileName);
	return stream;
}

string IniIncludeManager::tryRelative (string somePath, string subPath)
{
	IFileSystem& fileSystem = FileSystem::instance();
	string path = fileSystem.appendPath (somePath, subPath);
	return fileSystem.fileExists (path) ? path : "";
}

IRelativeStreamsManager::StreamId IniIncludeManager::addStream (IInputStream* stream, string fromFile)
{
	saAssert (inputStreamToId.count (stream) == 0);
	IFileSystem& fileSystem = FileSystem::instance();
	fromFile = fileSystem.getDirectoryPath (fromFile);

	inputStreamsOpened.push_back (IniInputStream (stream, fromFile));

	StreamId id = reinterpret_cast <StreamId> (inputStreamsOpened.size() - 1);
	inputStreamToId[stream] = id;
	return id;
}

IniIncludeManager::IniInputStream& IniIncludeManager::getStream (IRelativeStreamsManager::StreamId id)
{
	size_t index = reinterpret_cast <size_t> (id);
	return inputStreamsOpened[index];
}
