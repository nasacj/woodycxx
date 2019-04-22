cmake_minimum_required(VERSION 2.8)

PROJECT(woodycxx)

set(CMAKE_BUILD_TYPE Debug)

include_directories(.. /usr/local/include)
link_directories(/usr/local/lib)

ADD_SUBDIRECTORY(smart_ptr)
ADD_SUBDIRECTORY(base)
ADD_SUBDIRECTORY(sysipc)
ADD_SUBDIRECTORY(io)
ADD_SUBDIRECTORY(net)
