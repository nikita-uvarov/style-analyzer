#include "Utilities.h"

using namespace std;
using namespace sa;

vector <string> sa::split (string str, char delimiter)
{
	vector <string> result;
	for (size_t i = str.length(); i--; )
		if (str[i] == delimiter)
		{
			result.push_back (str.substr (i + 1));
			str.erase (str.begin() + static_cast <long> (i), str.end());
		}
	result.push_back (str);

	size_t size = result.size();
	size_t halfSize = size / 2;

	for (unsigned i = 0; i < halfSize; i++)
		swap (result[i], result[size - i - 1]);

	return result;
}

string sa::getBitPositions (uint32_t flags)
{
	string bitPositions;

	for (uint32_t i = 0; i < 32; i++)
		if (flags & (1 << i))
			bitPositions += (i ? ", " : "") + toString (i);

	return bitPositions;
}
