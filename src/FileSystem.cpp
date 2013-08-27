#include "FileSystem.h"
#include <boost/filesystem.hpp>

sa::IFileSystem::~IFileSystem() {}

sa::FileSystem::~FileSystem() {}

FILE* sa::FileSystem::fopen (const char* filename, const char* mode)
{
	return sa::fopen (filename, mode);
}

int sa::FileSystem::fclose (FILE* file)
{
	return sa::fclose (file);
}

int sa::FileSystem::fseek (FILE* file, long long int offset, int origin)
{
	return sa::fseek (file, offset, origin);
}

long long sa::FileSystem::ftell (FILE* file)
{
	return sa::ftell (file);
}

std::size_t sa::FileSystem::fwrite (void* ptr, std::size_t size, std::size_t n, FILE* file)
{
	return sa::fwrite (ptr, size, n, file);
}

std::size_t sa::FileSystem::fread (void* ptr, std::size_t size, std::size_t n, FILE* file)
{
	return sa::fread (ptr, size, n, file);
}

bool sa::FileSystem::fileExists (std::string absoluteOrRelativePath)
{
	return boost::filesystem::exists (boost::filesystem::path (absoluteOrRelativePath));
}

std::string sa::FileSystem::appendPath (std::string directory, std::string relativePath)
{
	boost::filesystem::path dir (directory);
	dir /= relativePath;
	return dir.string();
}

std::string sa::FileSystem::getCanonicalPath (std::string absoluteOrRelativePath)
{
	return boost::filesystem::canonical (boost::filesystem::path (absoluteOrRelativePath)).string();
}

bool sa::FileSystem::isDirectory (std::string absoluteOrRelativePath)
{
	return boost::filesystem::is_directory (boost::filesystem::path (absoluteOrRelativePath));
}

std::string sa::FileSystem::getDirectoryPath (std::string absoluteOrRelativePath)
{
	return boost::filesystem::canonical (boost::filesystem::path (absoluteOrRelativePath).parent_path()).string();
}

sa::IFileSystem& sa::FileSystem::instance()
{
	if (overrideFilesystem)
		return *overrideFilesystem;
	else
		return realInstance();
}

sa::IFileSystem& sa::FileSystem::realInstance()
{
	static FileSystem real;
	return real;
}

void sa::FileSystem::setFilesystem (sa::IFileSystem* fileSystem)
{
	overrideFilesystem = fileSystem;
}

sa::IFileSystem* sa::FileSystem::overrideFilesystem;
