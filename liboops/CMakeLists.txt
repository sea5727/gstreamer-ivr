cmake_minimum_required(VERSION 3.0.0)
project(gstreamer-media-server VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

include(CTest)
enable_testing()

add_library(oops configure.cpp utils.cpp)
target_link_libraries(oops libboost_filesystem.a)
# target_link_libraries(oop /usr/local/boost/lib/libboost_filesystem.a)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
