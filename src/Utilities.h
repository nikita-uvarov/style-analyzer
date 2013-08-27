#ifndef STYLE_ANALYZER_UTILITIES_H
#define STYLE_ANALYZER_UTILITIES_H

#include <string>
#include <vector>
#include <sstream>
#include <type_traits>

namespace sa
{
	using std::vector;
	using std::string;

	vector <string> split (string str, char delimiter);

	template <typename T>
	string toString (T something)
	{
		std::stringstream stream;
		stream << something;
		return stream.str();
	}

	string getBitPositions (uint32_t flags);

	template <typename T>
	bool extractFlag (T& flags, T flag)
	{
		typedef typename std::underlying_type <T>::type utype;

		utype theFlags = utype (flags),
		      theFlag  = utype (flag);

		bool isSet = theFlags & theFlag;

		theFlags &= ~theFlag;
		flags = T (theFlags);

		return isSet;
	}

	string getRealAbsolutePath (string fileName);
	string getDirectory (string fileOrDirectoryName);
	string getFilePath (string directory, string fileName);

    template <class T>
    void printVector (std::ostream& stream, const vector <T>& value)
    {
        if (value.empty())
            stream << "(empty)";
        else if (value.size() == 1)
            stream << "\"" << value[0] << "\"";
        else
        {
            for (auto it = value.begin(); it != value.end(); it++)
                stream << (it == value.begin() ? "{ " : ", ") << "\"" << *it << "\"";
            stream << " }";
        }
    }
}

#endif // STYLE_ANALYZER_UTILITIES_H
