#ifndef FILE_CONTEXT_H
#define FILE_CONTEXT_H

#include <iostream>
#include <memory>

#include "IndentationContext.h"
#include "NameContext.h"

namespace sa
{
	using std::string;
	using std::unique_ptr;
	using std::ifstream;
	using std::ofstream;

	class FileContext
	{
	public :

		const string& getFileName() const
		{
			return fileName;
		}
		
		const string& getFileContents() const
		{
			return fileContents;
		}
		
		IndentationContext* getIndentationContext()
		{
			return indentationContext.get();
		}
		
		NameContext* getNameContext()
		{
			return nameContext.get();
		}
		
		template <typename TOutputStream>
		void save (TOutputStream stream)
		{
			saveImpl (stream);
		}
		
		template <typename TInputStream>
		static unique_ptr <FileContext> load (TInputStream stream)
		{
			return loadImpl (stream);
		}

		static unique_ptr <FileContext> create (CXTranslationUnit unit);
		
	private :

		FileContext (const FileContext&) = delete;
		FileContext& operator= (const FileContext&) = delete;
		
		string fileContents;
		string fileName;
		
		unique_ptr <IndentationContext> indentationContext;
		unique_ptr <NameContext> nameContext;
		
		FileContext (string fileContents, string fileName) :
			fileContents (fileContents), fileName (fileName)
		{}
		
		template <typename TOutputStream>
		void saveImpl (TOutputStream /*stream*/)
		{
			// ...
		}
		
		template <typename TOutputStream>
		static unique_ptr <FileContext> loadImpl (TOutputStream /*stream*/)
		{
			// ...
		}
	};
}

#endif // FILE_CONTEXT_H
