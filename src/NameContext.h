#ifndef NAME_CONTEXT_H
#define NAME_CONTEXT_H

#include <iostream>

namespace sa
{
	using std::ifstream;
	using std::ofstream;
		
	class NameContext
	{
	public :
		
		template <typename TOutputStream>
		void save (TOutputStream stream)
		{
			saveImpl (stream);
		}
		
		template <typename TInputStream>
		static unique_ptr <NameContext> load (TInputStream stream)
		{
			return loadImpl (stream);
		}
		
	private :
		
		NameContext (const NameContext&) = delete;
		NameContext& operator= (const NameContext&) = delete;
		
		template <typename TOutputStream>
		void saveImpl (TOutputStream /*stream*/)
		{
			// ...
		}
		
		template <typename TOutputStream>
		static unique_ptr <NameContext> loadImpl (TOutputStream /*stream*/)
		{
			// ...
			return nullptr;
		}
	};
}

#endif // NAME_CONTEXT_H
