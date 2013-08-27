#ifndef STYLE_ANALYZER_FILE_SYSTEM_H
#define STYLE_ANALYZER_FILE_SYSTEM_H

#include <cstdio>
#include <cstddef>
#include <string>

namespace sa
{
	using namespace std;

	class IFileSystem
	{
	public :
		virtual ~IFileSystem();

		virtual FILE* fopen (const char* filename, const char* mode) = 0;
		virtual int fseek (FILE* file, long long offset, int origin) = 0;
		virtual long long ftell (FILE* file) = 0;
		virtual size_t fwrite (void* ptr, size_t size, size_t n, FILE* file) = 0;
		virtual size_t fread (void* ptr, size_t size, size_t n, FILE* file) = 0;
		virtual int fclose (FILE* file) = 0;

		virtual bool fileExists (string absoluteOrRelativePath) = 0;
		virtual bool isDirectory (string absoluteOrRelativePath) = 0;
		virtual string getCanonicalPath (string absoluteOrRelativePath) = 0;
		virtual string appendPath (string directory, string relativePath) = 0;
		virtual string getDirectoryPath (string absoluteOrRelativePath) = 0;
	};

	class FileSystem : public IFileSystem
	{
	public :
		~FileSystem();

		FILE* fopen (const char* filename, const char* mode);
		int fseek (FILE* file, long long offset, int origin);
		long long ftell (FILE* file);
		size_t fwrite (void* ptr, size_t size, size_t n, FILE* file);
		size_t fread (void* ptr, size_t size, size_t n, FILE* file);
		int fclose (FILE* file);

		bool fileExists (string absoluteOrRelativePath);
		bool isDirectory (string absoluteOrRelativePath);
		string getCanonicalPath (string absoluteOrRelativePath);
		string appendPath (string directory, string relativePath);
		string getDirectoryPath (string absoluteOrRelativePath);

		static IFileSystem& instance();

		// Caller owns the filesystem object
		static void setFilesystem (IFileSystem* fileSystem);

	private :
		static IFileSystem* overrideFilesystem;
		static IFileSystem& realInstance();
	};
}

#endif // STYLE_ANALYZER_FILE_SYSTEM_H
