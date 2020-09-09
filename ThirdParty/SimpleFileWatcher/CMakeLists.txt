cmake_minimum_required(VERSION 3.0)

project(SimpleFileWatcher)

include_directories(include)

Set(SRC 
    source/FileWatcher.cpp
    source/FileWatcherLinux.cpp
    source/FileWatcherOSX.cpp
    source/FileWatcherWin32.cpp)

add_library(SimpleFileWatcher STATIC ${SRC})