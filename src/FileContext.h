#ifndef STYLE_ANALYZER_FILE_CONTEXT_H
#define STYLE_ANALYZER_FILE_CONTEXT_H

#include <iostream>
#include <memory>

#include "IndentationContext.h"
#include "NameContext.h"
#include "Streams.h"

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
		
		void save (IOutputStream* stream);
		static unique_ptr <FileContext> load (IInputStream* stream);
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
	};
}

#endif // STYLE_ANALYZER_FILE_CONTEXT_H
