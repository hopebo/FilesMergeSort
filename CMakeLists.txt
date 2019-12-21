cmake_minimum_required(VERSION 3.0)

project(FilesQuickSort VERSION 1.0)

include_directories(${PROJECT_SOURCE_DIR})

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
message(STATUS "Compile options: ${CMAKE_CXX_FLAGS}")

aux_source_directory(${PROJECT_SOURCE_DIR}/classes SOURCE_FILES)
aux_source_directory(${PROJECT_SOURCE_DIR}/unit_tests UNIT_TEST_FILES)

add_executable(unit_tests ${UNIT_TEST_FILES} ${SOURCE_FILES})
add_executable(data_gen ${PROJECT_SOURCE_DIR}/test_utils/data_gen.cc)
add_executable(main ${PROJECT_SOURCE_DIR}/main.cc ${SOURCE_FILES})

set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin")

target_link_libraries(unit_tests gtest pthread)
target_link_libraries(main pthread)
