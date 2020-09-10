cmake_minimum_required(VERSION 3.0)

project(ShaderCompilerSource)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	add_executable(ShaderCompilerSource ShaderCompilerWin.cpp)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	add_executable(ShaderCompilerSource ShaderCompilerLinux.cpp)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	add_executable(ShaderCompilerSource ShaderCompilerOSX.cpp)
endif()

target_link_libraries(ShaderCompilerSource SimpleFileWatcher)