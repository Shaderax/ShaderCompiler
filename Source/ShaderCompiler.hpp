#pragma once

#include <stdexcept>
#include <iostream>
#include <string>
#include <FileWatcher/FileWatcher.h>
#include <array>
#include <cstdio>
#include <sstream>
#include <memory>
#include <errno.h>
#include <sys/stat.h>

const std::string gValidShaderExt[] = { "vert", "frag", "tesc", "tese", "geom", "comp"};

class ShaderReCompiler : public FW::FileWatchListener
{
public:
    ShaderReCompiler( void ) = default;

    void handleFileAction(FW::WatchID watchid, const std::string& dir, const std::string& filename, FW::Action action);
};

bool        CreatePipe( void );
void        Close( void );
bool        DirExists(const std::string& inDirName);
bool        CheckIfOnInstance( char* argv );