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

RelativeOutputStreamFlags sa::operator| (RelativeOutputStreamFlags a, RelativeOutputStreamFlags b)
{
    return static_cast <RelativeOutputStreamFlags> (static_cast <uint32_t> (a) | static_cast <uint32_t> (b));
}
