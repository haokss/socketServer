cmake_minimum_required(VERSION 3.5)

project(server LANGUAGES CXX)

add_executable(server socketServer.cpp MeowMessage.hpp MeowMessage.cpp sqlserver.h sqlserver.cpp)