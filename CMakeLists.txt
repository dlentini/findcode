cmake_minimum_required (VERSION 2.6)
project (ext)

include_directories ("${PROJECT_SOURCE_DIR}")

add_subdirectory (taskpool)

set(Boost_USE_STATIC_LIBS ON)

find_package(Boost 1.36.0 COMPONENTS program_options)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  add_executable(ext ext.cpp search.cpp)
  target_link_libraries(ext ${Boost_LIBRARIES})
endif()

if(CMAKE_SYSTEM_NAME MATCHES ".*Linux")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -Wall -Wextra -Wno-unused-parameter -Wglobal-constructors")

target_link_libraries (ext taskpool)
