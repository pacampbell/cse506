cmake_minimum_required(VERSION 2.8.4)
project(cse506)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O1 -std=c99 -D__thread= -Wall -Werror -nostdinc -Iinclude -msoft-float -mno-sse -mno-red-zone -fno-builtin -fPIC -march=amdfam10 -g3")

FILE(GLOB MyCSources bin/*/*.c)
#FILE(GLOB MyCHeaders include/*/*.h)

include_directories(include)

add_executable(sbush ${MyCSources} ${MyCHeaders})