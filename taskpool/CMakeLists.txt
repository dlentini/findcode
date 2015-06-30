
add_library(taskpool InternalTask.cpp TaskPool.cpp WorkerThread.cpp ParallelFor.cpp)
set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")
