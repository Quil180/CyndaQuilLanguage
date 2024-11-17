cmake_minimum_required(VERSION 3.30)
    
# here we specify that the project is C language only, so the default
# C compiler on the system will be used
project(CyndaQuil LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(ember src/main.cpp)

