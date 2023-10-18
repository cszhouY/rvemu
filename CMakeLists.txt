cmake_minimum_required (VERSION 3.16)
project (RISCV-emu)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

aux_source_directory(src DIR_SRCS)
# aux_source_directory(test DIR_TESTS)

if(${TEST})
	message("TEST VERSION")
	find_library(LIBGTEST gtest PATHS lib)
	find_library(LIBGTEST_MAIN gtest_main PATHS lib)
	find_library(LIBGMOCK gmock PATHS lib)
	find_library(LIBGMOCK_MAIN gmock_main PATHS lib)
	add_executable(emu-test test/test.cpp)
	target_link_libraries(emu-test ${LIBGTEST} ${LIBGTEST_MAIN} ${LIBGMOCK} ${LIBGMOCK_MAIN} pthread)

	add_executable(list-test test/circularListTest.cpp)
else()
	message("DEBUG VERSION")
	add_executable(emu ${DIR_SRCS})
	target_link_libraries(emu pthread)
endif()
