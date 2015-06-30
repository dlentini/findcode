cmake_minimum_required (VERSION 2.6)
project (ext)

include_directories ("${PROJECT_SOURCE_DIR}")

add_subdirectory (taskpool)

find_package(Boost 1.36.0 COMPONENTS program_options)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  add_executable(ext ext.cpp search.cpp)
  target_link_libraries(ext ${Boost_LIBRARIES})
endif()

set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++ -Wall -Wextra -Wno-unused-parameter")
target_link_libraries (ext taskpool)
