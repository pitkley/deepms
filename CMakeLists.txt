cmake_minimum_required(VERSION 3.3)
project(deepms)

find_path(LIBXML2_INCLUDE_DIR libxml/xmlstring.h
        HINTS ${PC_LIBXML_INCLUDEDIR} ${PC_LIBXML_INCLUDE_DIRS}
        PATH_SUFFIXES libxml2 )

include_directories(${LIBXML2_INCLUDE_DIR})

link_libraries(ddccontrol X11 Xext)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

set(SOURCE_FILES main.c main.h)
add_executable(deepms ${SOURCE_FILES})