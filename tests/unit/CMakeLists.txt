find_package(Boost COMPONENTS unit_test_framework REQUIRED)
include_directories(${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../../src)

set(style_analyzer_unit_test_sources
    Common.cpp
	ini-configuration/IniConfigurationTest.cpp)

add_definitions(-DBOOST_TEST_DYN_LINK)
add_executable (style_analyzer_unit_test ${style_analyzer_unit_test_sources})
target_link_libraries (style_analyzer_unit_test style_analyzer_library ${Boost_LIBRARIES})
enable_testing ()
add_test (style_analyzer_unit_test style_analyzer_unit_test)

set(unit_test_run_flag_file
    "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/unit_test_run.flag")

add_custom_command(OUTPUT "${unit_test_run_flag_file}"
    COMMAND "./style_analyzer_unit_test" "--log_level=test_suite"
    COMMAND ${CMAKE_COMMAND} -E touch "${unit_test_run_flag_file}"
    COMMENT "Running unit tests"
    DEPENDS style_analyzer_unit_test)

add_custom_target(unit_tests_run ALL DEPENDS "${unit_test_run_flag_file}")
