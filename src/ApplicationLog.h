#ifndef STYLE_ANALYZER_APPLICATION_LOG_H
#define STYLE_ANALYZER_APPLICATION_LOG_H

#include <string>
#include <vector>
#include <map>

#include "Streams.h"

namespace sa
{

using std::string;
using std::vector;
using std::map;

class ApplicationLogger
{
public :
    void log (const char* file, int line, const char* function, string message, bool error);

    void openLog (IOutputStream* binaryOutputStream);
    void closeLog();

    void setDuplicateToCerr (bool duplicate);

    static ApplicationLogger& instance();

private :
    ApplicationLogger() = default;
    ApplicationLogger (const ApplicationLogger&) = delete;
    ApplicationLogger operator= (const ApplicationLogger&) = delete;

    // Makes the class POD
    IOutputStream* stream;
    map <string, uint32_t>* stringTable;

    bool duplicateToCerr;

    void writeString (string string);
    void writeInteger (uint32_t integer);
};

class LogStreamHolder
{
public :
    LogStreamHolder (IOutputStream* stream);
    ~LogStreamHolder();
};

class ApplicationLog
{
public :
    unsigned getNumEntries() const;

    string getEntryFileOrigin (unsigned entryIndex) const;
    string getEntryFunctionOrigin (unsigned entryIndex) const;
    unsigned getEntryLineOrigin (unsigned entryIndex) const;

    string getEntryMessage (unsigned entryIndex) const;
    bool isErrorEntry (unsigned entryIndex) const;

    static unique_ptr <ApplicationLog> load (IInputStream* binaryInputStream);

private :
    ApplicationLog() = default;
    ApplicationLog (const ApplicationLog&) = delete;
    ApplicationLog& operator= (const ApplicationLog&) = delete;

    vector <string> stringTable;

    struct ApplicationLogEntry
    {
        unsigned fileOriginIndex, functionOriginIndex;
        unsigned lineOrigin;

        unsigned messageIndex;
        bool isError;
    };

    vector <ApplicationLogEntry> entries;

    unsigned readString (IInputStream* stream);
    uint32_t readInteger (IInputStream* stream);
};

string formatLogEntryForStderr (const char* file, int line, const char* function, string message, bool error);

#define saLog(message) sa::ApplicationLogger::instance().log (__ORIGIN__, (message), false)
#define saError(message) sa::ApplicationLogger::instance().log (__ORIGIN__, (message), true)

}

#endif // STYLE_ANALYZER_APPLICATION_LOG_H
