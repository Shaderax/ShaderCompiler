cmake_minimum_required(VERSION 3.0)

project(Examples)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	add_executable(FakeGame FakeGame.cpp)
endif()