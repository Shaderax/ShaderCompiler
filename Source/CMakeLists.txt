cmake_minimum_required(VERSION 3.0)

project(ShaderCompilerSource)

Set( SRC
    main.cpp)

add_executable(ShaderCompilerSource ${SRC})

target_link_libraries(ShaderCompilerSource SimpleFileWatcher)