cmake_minimum_required(VERSION 3.7)
project(tests LANGUAGES C)

set(CMAKE_C_STANDARD 99)

add_executable(converter src/main1.c)
add_executable(comparer src/main2.c)


