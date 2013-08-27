#ifndef STYLE_ANALYZER_UNIT_TESTS_COMMON_H
#define STYLE_ANALYZER_UNIT_TESTS_COMMON_H

#include <boost/test/unit_test.hpp>

void smartChangeDirectory (const char* toFile);

#define CHANGE_DIRECTORY() smartChangeDirectory(__FILE__)

#endif // STYLE_ANALYZER_UNIT_TESTS_COMMON_H
