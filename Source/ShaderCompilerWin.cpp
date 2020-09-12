#include <Windows.h>
#include <stdexcept>
#include <iostream>
#include <string>
#include <FileWatcher/FileWatcher.h>
#include <array>
#include <cstdio>
#include <sstream>

static const std::string gValidShaderExt[] = { "vert", "frag", "tesc", "tese", "geom", "comp"};
HANDLE hMutex;

bool CheckIfOnInstance( char* argv )
{
    hMutex = OpenMutex(
      MUTEX_ALL_ACCESS, 0, argv);

    if (!hMutex)
      hMutex = CreateMutex(0, 0, argv);
    else
    {
        ReleaseMutex(hMutex);
        return false;
    }

    return true;
}

std::string exec(const char* cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);

    if (!pipe)
        throw std::runtime_error("popen() failed!");

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        result += buffer.data();

    return result;
}

HANDLE gHPipe;
DWORD dwRead;

class ShaderReCompiler : public FW::FileWatchListener
{
    public:
    ShaderReCompiler( void ) = default;

    void handleFileAction(FW::WatchID watchid, const std::string& dir, const std::string& filename, FW::Action action)
	{
        std::size_t pos = filename.rfind(".");
        for (const std::string& ext : gValidShaderExt)
        {
            if (pos != std::string::npos && filename.substr(pos + 1) == ext)
            {
                std::ostringstream cmd;

                std::string cmplPath =  dir + '/' + filename;
                std::cout << cmplPath << (int)action << std::endl;

                cmd << "glslc " << cmplPath << " -o " << cmplPath << ".spv" << " 2>&1";
                std::string log = exec(cmd.str().c_str());
                std::cout << "Exec return: " << log << std::endl;
                WriteFile(gHPipe, log.c_str(), log.size() + 1, &dwRead, NULL);
                std::cout << "End Log" << std::endl;

                return ;
            }
        }
        std::cout << dir << "/" << filename << "Don't know what is that" << std::endl;
	}
};

bool DirExists(const std::string& inDirName)
{
  DWORD ftyp = GetFileAttributesA(inDirName.c_str());
  if (ftyp == INVALID_FILE_ATTRIBUTES)
    return false;  //something is wrong with your path!

  if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
    return true;   // this is a directory!

  return false;    // this is not a directory!
}

// ugly globals for demo
FW::FileWatcher* gFileWatcher = 0;
FW::WatchID gWatchID = 0;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cout << "error: usage: ShaderCompiler <directory path>" << std::endl;
        return -1;
    }

    if (!DirExists(argv[1]))
    {
        std::cout << "Not a valid Directory: " << argv[1] << std::endl;
        return -1;
    }

    /** Check One instance*/
    if (!CheckIfOnInstance(argv[1]))
        return 0;

    gFileWatcher = new FW::FileWatcher();

    char buffer[1024];

    gHPipe = CreateNamedPipeA(TEXT("\\\\.\\pipe\\ShaderCompilerOutput"),
                        PIPE_ACCESS_DUPLEX,
                        PIPE_TYPE_MESSAGE | PIPE_WAIT | PIPE_READMODE_MESSAGE,
                        5,
                        1024 * 16,
                        1024 * 16,
                        NMPWAIT_USE_DEFAULT_WAIT,
                        NULL);

    if (gHPipe == INVALID_HANDLE_VALUE)
        throw std::runtime_error("hPipe == nullptr");

/*
    if (ConnectNamedPipe(hPipe, NULL) != FALSE)
        {
            while (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &dwRead, NULL) != FALSE)
            {
                buffer[dwRead] = '\0';

                printf("%s", buffer);
            }
        }
*/

    gWatchID = gFileWatcher->addWatch(argv[1], new ShaderReCompiler(), true);

    while(1)
	{
		Sleep(500);
		gFileWatcher->update();
	}

    //DisconnectNamedPipe(gHPipe);
    ReleaseMutex(hMutex);

    std::cin.get();

    return 0;
}