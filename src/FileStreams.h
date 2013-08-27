/* File streams implementing interfaces from Streams.h */

#ifndef STYLE_ANALYZER_FILE_STREAMS_H
#define STYLE_ANALYZER_FILE_STREAMS_H

#include "Streams.h"

namespace sa
{
	class UniversalInputStream : public IInputStream
	{
	public :
		~UniversalInputStream() = default;

		uint32_t read (char* buffer, uint32_t nBytes);
		uint32_t getNumBytesRemaining() const;

		bool isFileStream() const;
		string getSourceFileName() const;
		string getSourceBufferName() const;

		static unique_ptr <UniversalInputStream> openInputStream (string fileName, RelativeInputStreamFlags flags);
		static unique_ptr <UniversalInputStream> openInputStream (string bufferName, string bufferContents);

	private :
		UniversalInputStream (const UniversalInputStream&) = delete;
		UniversalInputStream& operator= (const UniversalInputStream&) = delete;

		unique_ptr <char[]> fileContents;
		uint32_t bufferPosition, bufferSize;

		string fileName;
		bool isBinaryStream, isBroken, isFile;

		UniversalInputStream (string fileName, bool isBinaryStream);
		UniversalInputStream (string bufferName);

		string getStreamDescription() const;

		ATTRIBUTE_NORETURN void ioError (const char* fileOrigin, int lineOrigin, const char* functionOrigin, string operation);
	};

	class FileOutputStream : public IOutputStream
	{
	public :
		~FileOutputStream();

		void write (const char* data, uint32_t nBytes);

		static unique_ptr <FileOutputStream> openOutputStream (string fileName, RelativeOutputStreamFlags flags);

	private :
		FileOutputStream (const FileOutputStream&) = delete;
		FileOutputStream& operator= (const FileOutputStream&) = delete;

		FILE* file;
		string fileName;
		bool isBinaryStream, isBroken;

		FileOutputStream (string fileName, bool isBinaryStream);

		string getStreamDescription() const;

		ATTRIBUTE_NORETURN void ioError (const char* fileOrigin, int lineOrigin, const char* functionOrigin, string operation);
	};
}

#endif // STYLE_ANALYZER_FILE_STREAMS_H
