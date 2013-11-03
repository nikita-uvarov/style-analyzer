/* Streams interface used in this project.

   Provides 'streams' abstraction that basically:
   - hides IO from/to buffer and IO from/to file.
   - throws exceptions when something goes wrong, with meaningful descriptions to show the user.
   STL does not meet the second requirement, cstdio does not meet the first.

   So:
   - input streams are assumed to be buffered, or, at least, not modifiable in runtime, so
     that interface implementation can maintain position in stream (bytes remaining) efficiently.
   - some classes that use streams need to be able to get input from file/buffer and use
     IInputStream/IOutputStream. Others may also need to open subsequent streams (e. g. while
     processing include directives). For such cases, there is a 'relativity manager':
     relative streams are opened by name (typically file name) and pointer to flags (how to interpret file name,
     where to search etc.) relative to some stream previously opened by this manager.
     Manager must guarantee it can derive type of stream passed (e.g. by saving all allocated streams or using dynamic casts).
   - when opening a new stream via relativity manager, an object can specify stream flags (binary, append).
   - when accessing a passed stream via I*Stream an object can't verify that stream meets requirements.
     If a function reads file given by stream, and expects that stream to be binary, calling it on a text stream would result
     in an undefined behaviour.
   - a function that accepts a relativity manager typically assumes it follows some defined way of handling requests.
     There is also an agreement of what type the flags object is and so on.
   - functions that use IRelativityManager to open streams may catch InputOutputException to provide a better diagnostic.

   Streams are not designed to work with files larger than 4 GB and use uint32_t for sizes and offsets.
*/

#ifndef STYLE_ANALYZER_STREAMS_H
#define STYLE_ANALYZER_STREAMS_H

#include <memory>

#include "Debug.h"

namespace sa
{

using std::unique_ptr;
using std::string;

class IInputStream
{
public :
    virtual ~IInputStream();

    // Returns num bytes actually read
    virtual uint32_t read (char* buffer, uint32_t nBytes) = 0;

    // Stream implementation should read the file completely before processing,
    // Or at least guarantee it would not be changed in runtime and maintain byte counter.
    virtual uint32_t getNumBytesRemaining() const = 0;
};

class IOutputStream
{
public :
    virtual ~IOutputStream();

    virtual void write (const char* data, uint32_t nBytes) = 0;
};

class InputOutputException : public Exception
{
public :
    InputOutputException (const char* fileOrigin, int lineOrigin, const char* functionOrigin,
                          string streamDescription, string operation) :
        Exception (fileOrigin, lineOrigin, functionOrigin, operation + " failed on " + streamDescription),
        streamDescription (streamDescription), operation (operation)
    {}

    const string& getStreamDescription() const
    {
        return streamDescription;
    }

    const string& getOperation() const
    {
        return operation;
    }

    string toString() const;

private :
    string streamDescription, operation;
};

enum class RelativeInputStreamFlags : uint32_t
{
    NONE   = 0,

    BINARY = 1 << 0
};

enum class RelativeOutputStreamFlags : uint32_t
{
    NONE = 0,

    BINARY = 1 << 0,
    APPEND = 1 << 1
};

class IRelativeStreamsManager
{
public :
    typedef void* StreamId;

    virtual ~IRelativeStreamsManager();

    virtual StreamId getInputStreamId (IInputStream* stream) = 0;
    virtual StreamId getOutputStreamId (IOutputStream* stream) = 0;

    virtual unique_ptr <IInputStream> openInputStream (StreamId relativeTo, string relativeName,
                                                       void* nameTypeFlagsPointer, RelativeInputStreamFlags flags) = 0;
    virtual unique_ptr <IOutputStream> openOutputStream (StreamId relativeTo, string relativeName,
                                                         void* nameTypeFlagsPointer, RelativeOutputStreamFlags flags) = 0;
};

}

#endif // STYLE_ANALYZER_STREAMS_H
