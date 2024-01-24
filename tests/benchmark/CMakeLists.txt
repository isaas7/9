cmake_minimum_required(VERSION 3.25)

project(beast-test)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
endif()
find_package(GTest REQUIRED)
file(GLOB_RECURSE SOURCE_FILES "../../src/*.cpp" "test_benchmark.cpp")
add_executable(main ${SOURCE_FILES})
target_link_libraries(main PRIVATE ${Boost_LIBRARIES} GTest::GTest GTest::Main)

