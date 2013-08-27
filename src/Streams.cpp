#include "Streams.h"

#include <cstring>

using namespace std;
using namespace sa;

IInputStream::~IInputStream() {}

IOutputStream::~IOutputStream() {}

IRelativeStreamsManager::~IRelativeStreamsManager() {}


string InputOutputException::toString() const
{
	return "Operation '" + operation + "' failed on " + streamDescription + ".";
}
