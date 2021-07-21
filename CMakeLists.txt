cmake_minimum_required(VERSION 3.19)
project(Interpreter_cpp)

set(CMAKE_CXX_STANDARD 98)

add_executable(Interpreter_cpp main.cpp symboltable.cpp symboltable.h)