#include "FileStreams.h"
#include "FileSystem.h"
#include "Utilities.h"

#include <cstring>

using namespace sa;
using namespace std;

uint32_t UniversalInputStream::getNumBytesRemaining() const
{
	saAssert (bufferPosition >= 0 && bufferPosition <= bufferSize);
	return bufferSize - bufferPosition;
}

std::string UniversalInputStream::getStreamDescription() const
{
	return string (isBinaryStream ? "binary " : "text ") + "input stream created on '" + fileName + "'";
}

UniversalInputStream::UniversalInputStream (std::string fileName, bool isBinaryStream) :
	bufferPosition (0), bufferSize (0), fileName (fileName), isBinaryStream (isBinaryStream), isBroken (false), isFile (true)
{}

UniversalInputStream::UniversalInputStream (std::string fileName) :
	bufferPosition (0), bufferSize (0), fileName (fileName), isBinaryStream (false), isBroken (false), isFile (false)
{}

bool UniversalInputStream::isFileStream() const
{
	return isFile;
}

string UniversalInputStream::getSourceFileName() const
{
	saAssert (isFileStream());
	return fileName;
}

string UniversalInputStream::getSourceBufferName() const
{
	saAssert (!isFileStream());
	return fileName;
}

uint32_t UniversalInputStream::read (char* buffer, uint32_t nBytes)
{
	if (isBroken) return 0;

	uint32_t nRead = min (nBytes, getNumBytesRemaining());
	memcpy (buffer, fileContents.get(), nRead);
	return nRead;
}

ATTRIBUTE_NORETURN void UniversalInputStream::ioError (const char* fileOrigin, int lineOrigin, const char* functionOrigin, std::string operation)
{
	isBroken = true;
	throw InputOutputException (fileOrigin, lineOrigin, functionOrigin, getStreamDescription(), operation);
}

unique_ptr <UniversalInputStream> UniversalInputStream::openInputStream (string fileName, RelativeInputStreamFlags flags)
{
	bool binary = extractFlag (flags, RelativeInputStreamFlags::BINARY);

	unique_ptr <UniversalInputStream> stream (new UniversalInputStream (fileName, binary));

	if (uint32_t (flags))
		throw InvalidArgumentException (__ORIGIN__, "Unknown RelativeInputStreamFlags flags remaining: '" + toString (uint32_t (flags)) +
		                                "' (bits " + getBitPositions (uint32_t (flags)) + ").", "flags");

	IFileSystem& fileSystem = FileSystem::instance();

	FILE* file = fileSystem.fopen (fileName.c_str(), binary ? "rb" : "r");
	if (!file)
		stream->ioError (__ORIGIN__, "create (fopen)");

	if (fileSystem.fseek (file, 0, SEEK_END) != 0)
		stream->ioError (__ORIGIN__, "seek to end (fseek)");

	long int fileSize = fileSystem.ftell (file);
	if (fileSize < 0) stream->ioError (__ORIGIN__, "tell (ftell)");

	stream->bufferSize = static_cast <uint32_t> (fileSize);

	if (fileSystem.fseek (file, 0, SEEK_SET) != 0)
		stream->ioError (__ORIGIN__, "seek to begin (fseek)");

	stream->fileContents.reset (new char[fileSize]);
	size_t nRead = fileSystem.fread (stream->fileContents.get(), 1, static_cast <size_t> (fileSize), file);

	if (nRead != static_cast <size_t> (fileSize))
		stream->ioError (__ORIGIN__, "read (fread)");

	fileSystem.fclose (file);

	return stream;
}

unique_ptr <UniversalInputStream> UniversalInputStream::openInputStream (string bufferName, string bufferContents)
{
	unique_ptr <UniversalInputStream> stream (new UniversalInputStream (bufferName));

	stream->bufferSize = static_cast <uint32_t> (bufferContents.size());
	stream->fileContents.reset (new char[stream->bufferSize]);
	std::memcpy (stream->fileContents.get(), bufferContents.c_str(), stream->bufferSize);

	return stream;
}


FileOutputStream::FileOutputStream (string fileName, bool isBinaryStream) :
	fileName (fileName), isBinaryStream (isBinaryStream)
{}

FileOutputStream::~FileOutputStream()
{
	if (file)
		fclose (file);
}

string FileOutputStream::getStreamDescription() const
{
	return string (isBinaryStream ? "binary " : "text ") + "output stream created on '" + fileName + "'";
}

ATTRIBUTE_NORETURN void FileOutputStream::ioError (const char* fileOrigin, int lineOrigin, const char* functionOrigin, string operation)
{
	isBroken = true;
	throw InputOutputException (fileOrigin, lineOrigin, functionOrigin, getStreamDescription(), operation);
}

void FileOutputStream::write (const char* data, uint32_t nBytes)
{
	if (isBroken) return;

	if (fwrite (data, 1, nBytes, file) != nBytes)
		ioError (__ORIGIN__, "write (fwrite)");
}


unique_ptr <FileOutputStream> FileOutputStream::openOutputStream (string fileName, RelativeOutputStreamFlags flags)
{
	bool binary = extractFlag (flags, RelativeOutputStreamFlags::BINARY);

	if (uint32_t (flags))
		throw InvalidArgumentException (__ORIGIN__, "Unknown RelativeOutputStreamFlags flags remaining: '" + toString (uint32_t (flags)) +
		                                "' (bits " + getBitPositions (uint32_t (flags)) + ").", "flags");

	unique_ptr <FileOutputStream> stream (new FileOutputStream (fileName, binary));

	if (!(stream->file = fopen (fileName.c_str(), binary ? "wb" : "w")))
		stream->ioError (__ORIGIN__, "create (fopen)");

	return stream;
}
