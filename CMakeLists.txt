project(style_analyzer)
cmake_minimum_required(VERSION 2.6)

find_package(Boost COMPONENTS filesystem system REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Instead of trying to keep an up-to-date list of supported warnings, just blacklist the unneeded and noisy ones
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything -Werror")

# No backward compatibility aims
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++98-compat -Wno-c++98-compat-pedantic")

# Want to use shadowing when handy (i. e. in constructors)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-shadow")

# Unneeded optimizations should be avoided. Let the structures be padded as the compiler wants.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-padded")

# Required for Boost.Test macros to compile
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-global-constructors")

# How to implement singletones without them?
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-exit-time-destructors")

# Stupid warnings go here.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-prototypes")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -DBUILD_CONFIGURATION_DEBUG")
set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE}")

set(style_analyzer_library_sources
    src/FileContext.cpp
    src/Streams.cpp
    src/Debug.cpp
    src/ApplicationLog.cpp
    src/IndentationContext.cpp
    src/NameContext.cpp
    src/Utilities.cpp
    src/IniConfiguration.cpp
    src/FileStreams.cpp
    src/FileSystem.cpp
    src/StringFormatter.cpp
    src/Internationalization.cpp)

add_subdirectory(tests/unit)

add_library(style-analyzer-library ${style_analyzer_library_sources})
target_link_libraries(style-analyzer-library clang stdc++ ${Boost_LIBRARIES})

set(style_analyzer_tool_sources
	src/Main.cpp)

add_executable(style-analyzer-tool ${style_analyzer_tool_sources})
target_link_libraries(style-analyzer-tool style-analyzer-library)

#install(TARGETS style_analyzer_library RUNTIME DESTINATION bin)

enable_testing()
