cmake_minimum_required(VERSION 2.8.3)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake/modules/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)   #for executables

set(project_name FloatyChipAsm)
project(${project_name})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wextra -Werror=return-type -Os")

include_directories("include")

file(GLOB_RECURSE source_files "src/*.cpp")
file(GLOB_RECURSE header_files "include/*.hpp" "include/*.def" "include/ctre/ctre")

set(Boost_USE_STATIC_LIBS   ON)
find_package(Boost COMPONENTS wave REQUIRED)

find_package (Threads)

add_executable(${project_name} ${header_files} ${source_files})
target_link_libraries(${project_name} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
