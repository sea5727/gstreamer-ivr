cmake_minimum_required(VERSION 3.0.0)
project(gstreamer-media-server VERSION 0.1.0)

if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0)
    # message( FATAL_ERROR "You can not do this at all, CMake will exit... change your gcc version" )
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

include(CTest)
enable_testing()

# add_subdirectory(liboops)


include(FindPkgConfig)
if(PKG_CONFIG_FOUND)
    message(PKG_CONFIG_FOUND: ${PKG_CONFIG_FOUND} )
    message(PKG_CONFIG_EXECUTABLE: ${PKG_CONFIG_EXECUTABLE} )
    message(PKG_CONFIG_VERSION_STRING: ${PKG_CONFIG_VERSION_STRING} )
    message(PKG_CONFIG: ${PKG_CONFIG} ) 
    pkg_check_modules (GST gstreamer-1.0)
    pkg_check_modules (GLIB glib-2.0)
    pkg_check_modules (GSTAUDIO gstreamer-audio-1.0)
    
    message(GST_INCLUDE_DIRS: ${GST_INCLUDE_DIRS} )
    message(GST_LIBRARY_DIRS: ${GST_LIBRARY_DIRS} )
    message(GST_LIBRARIES: ${GST_LIBRARIES} )   
    INCLUDE_DIRECTORIES(${GST_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS} ${GSTAUDIO_INCLUDE_DIRS})
    LINK_LIBRARIES(${GST_LIBRARIES} ${GLIB_LIBRARIES} ${GSTAUDIO_LIBRARIES})
endif()

find_package(cpprestsdk REQUIRED)

include_directories( /usr/local/boost/include/ /usr/include/gstreamer-1.0 )
link_directories( /usr/local/lib64 /usr/local/gcc/lib64 /usr/local/boost/lib/)
link_libraries(pthread boost_thread spdlog)


# add_executable(gstreamer-media-server 
#                 ms_main.cpp
#                 ms_hmp_server.cpp
#                 ms_core_gst.cpp
#                 ms_core_gst_builder.cpp)
add_executable(gstreamer-media-server ms_main.cpp)
# add_dependencies(gstreamer-media-server oops)
target_link_libraries(gstreamer-media-server cpprestsdk::cpprest ssl crypto)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
