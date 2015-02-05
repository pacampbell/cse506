cmake_minimum_required(VERSION 2.8.4)
project(cse506)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

FILE(GLOB MyCSources bin/*/*.c)
#FILE(GLOB MyCHeaders include/*/*.h)



#set(SOURCE_FILES
#    bin/sbush/sbush.c
#    include/stdio.h
#    include/.h
#    include/sys/defs.h)

include_directories(include)

add_executable(sbush ${MyCSources} ${MyCHeaders})