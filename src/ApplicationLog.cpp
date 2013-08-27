#include <iostream>
#include <iomanip>
#include <sstream>

#include "ApplicationLog.h"
#include "Utilities.h"

using namespace std;
using namespace sa;

ApplicationLogger& ApplicationLogger::instance()
{
	static ApplicationLogger theInstance;
	return theInstance;
}

void ApplicationLogger::openLog (IOutputStream* binaryOutputStream)
{
	saAssert (!stream && !stringTable);
	stream = binaryOutputStream;
	stringTable = new map <string, uint32_t>;
}

void ApplicationLogger::closeLog()
{
	if (stream)
		stream = nullptr;

	if (stringTable)
	{
		delete stringTable;
		stringTable = nullptr;
	}
}

void ApplicationLogger::setDuplicateToCerr (bool duplicate)
{
	duplicateToCerr = duplicate;
}

void ApplicationLogger::writeInteger (uint32_t integer)
{
	saAssert (stream);
	stream->write (reinterpret_cast <char*> (&integer), 4);
}

void ApplicationLogger::writeString (std::string str)
{
	saAssert (stream && stringTable);

	map <string, uint32_t>::iterator it = stringTable->find (str);
	if (it == stringTable->end())
	{
		it = stringTable->insert (make_pair (str, stringTable->size())).first;
		writeInteger (it->second);
		writeInteger (static_cast <uint32_t> (it->first.length()));
		stream->write (it->first.c_str(), static_cast <uint32_t> (it->first.length()));
	}
	else
	{
		writeInteger (it->second);
	}
}

void ApplicationLogger::log (const char* file, int line, const char* function, string message, bool error)
{
	saAssert (line >= 0);

	if (duplicateToCerr)
		cerr << formatLogEntryForStderr (file, line, function, message, error);

	if (stream)
	{
		writeString (file);
		writeInteger (static_cast <uint32_t> (line));
		writeString (function);
		writeString (message);
		writeInteger (error ? 1 : 0);
	}
}

string sa::formatLogEntryForStderr (const char* file, int line, const char* /*function*/, string message, bool /*error*/)
{
	const char* sourcePathSubstring = "src/";
	const int fileFieldWidth = 18 + 1 + 3;
	const int lineFieldWidth = 4;

	string prettyFile = file;

	size_t srcPos = string (file).find (sourcePathSubstring);
	if (srcPos != string::npos)
		prettyFile = prettyFile.substr (srcPos + string (sourcePathSubstring).length());

	string header = "";
	{
		ostringstream headerStream;
		headerStream << "[" << right << setw (fileFieldWidth) << prettyFile << ":" << right << setw (lineFieldWidth) << line << "]: ";

		header = headerStream.str();
	}

	vector <string> lines = split (message, '\n');
	string result;

	for (string& line: lines)
		result += header + line + "\n";

	return result;
}

LogStreamHolder::LogStreamHolder (IOutputStream* stream)
{
	ApplicationLogger::instance().openLog (stream);
}

LogStreamHolder::~LogStreamHolder()
{
	ApplicationLogger::instance().closeLog();
}

unique_ptr <ApplicationLog> ApplicationLog::load (IInputStream* binaryInputStream)
{
	unique_ptr <ApplicationLog> log (new ApplicationLog);
	while (binaryInputStream->getNumBytesRemaining())
	{
		ApplicationLogEntry entry;
		entry.fileOriginIndex = log->readString (binaryInputStream);
		entry.lineOrigin = log->readInteger (binaryInputStream);
		entry.functionOriginIndex = log->readString (binaryInputStream);
		entry.messageIndex = log->readString (binaryInputStream);
		entry.isError = log->readInteger (binaryInputStream) != 0;
		log->entries.push_back (entry);
	}

	return log;
}

uint32_t ApplicationLog::readInteger (IInputStream* stream)
{
	uint32_t integer;
	stream->read (reinterpret_cast <char*> (&integer), 4);
	return integer;
}

unsigned ApplicationLog::readString (IInputStream* stream)
{
	unsigned index = static_cast <unsigned> (readInteger (stream));
	if (index < stringTable.size())
		return index;

	saAssert (index == stringTable.size());

	unsigned length = static_cast <unsigned> (readInteger (stream));
	unique_ptr <char[]> buffer (new char[length + 1]);
	buffer[length] = 0;
	stream->read (buffer.get(), static_cast <uint32_t> (length));
	stringTable.push_back (string (buffer.get()));

	return index;
}

string ApplicationLog::getEntryMessage (unsigned int entryIndex) const
{
	return stringTable[entries[entryIndex].messageIndex];
}

string ApplicationLog::getEntryFileOrigin (unsigned int entryIndex) const
{
	return stringTable[entries[entryIndex].fileOriginIndex];
}

unsigned ApplicationLog::getEntryLineOrigin (unsigned int entryIndex) const
{
	return entries[entryIndex].lineOrigin;
}

string ApplicationLog::getEntryFunctionOrigin (unsigned int entryIndex) const
{
	return stringTable[entries[entryIndex].functionOriginIndex];
}

unsigned ApplicationLog::getNumEntries() const
{
	return static_cast <unsigned> (entries.size());
}

bool ApplicationLog::isErrorEntry (unsigned int entryIndex) const
{
	return entries[entryIndex].isError;
}
