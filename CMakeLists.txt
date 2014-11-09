cmake_minimum_required(VERSION 2.8)
project("Process")

if(APPLE)
    set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")
elseif(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-std=c++11")
endif()

set(SRC Source/Example.cpp)
add_executable(Example ${SRC})
