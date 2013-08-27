#define BOOST_TEST_MODULE "main"

#include "Common.h"
#include <cstdlib>
#include <string>
#include <boost/filesystem.hpp>

using namespace std;

void smartChangeDirectory (const char* toFile)
{
	boost::filesystem::current_path (boost::filesystem::path (toFile).parent_path());
}
