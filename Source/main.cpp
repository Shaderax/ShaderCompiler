#include <Windows.h>
#include <stdexcept>
#include <iostream>
#include <string>
#include <FileWatcher/FileWatcher.h>
#include <array>
#include <cstdio>
#include <sstream>

static const std::string gValidShaderExt[] = { ".vert", ".frag", ".tesc", ".tese", ".geom", ".comp"};

std::string exec(const char* cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
    // _popen Win
    // popen Linux

    if (!pipe)
        throw std::runtime_error("popen() failed!");

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        result += buffer.data();

    return result;
}

class ShaderReCompiler : public FW::FileWatchListener
{
    public:
    ShaderReCompiler( void ) = default;

    void handleFileAction(FW::WatchID watchid, const std::string& dir, const std::string& filename, FW::Action action)
	{
        std::size_t pos = filename.rfind(".");
        for (const std::string& ext : gValidShaderExt)
        {
            if (!filename.compare(pos, ext.size(), ext))
            {
                std::ostringstream cmd;

                std::string cmplPath =  dir + '/' + filename;
                std::cout << cmplPath << std::endl;

                cmd << "glslc " << cmplPath << ' ' << cmplPath << ".spv";
                // Exec glslc
                std::string log = exec(cmd.str().c_str());
                std::cout << "Exec return: " << log << std::endl;
                return ;
            }
        }
        std::cout << dir << "/" << filename << "Don't know what is that" << std::endl;
	}
};

    /**
     * Je dois recevoir en param un dossier, et moi je m'occupe de savoir lequel est un shader
     * A chaque fois qu'il y a un truc qui bouge dans le dossier je recup ce qui à été fait et le fichier en question et le fichier je check si c'est un shader:

     * Si oui alors j'exec la command glslc <filename> -o <filename>+.spv
     * Et le résultat de la commande est envoyé dans ShaderCompilerOutput
     *  Si ya pas d'erreur alors "" sinon l'error.
     **/

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

    gFileWatcher = new FW::FileWatcher();
    gWatchID = gFileWatcher->addWatch(argv[1], new ShaderReCompiler());

    while(1)
	{
		gFileWatcher->update();
	}

    /**/

    HANDLE hPipe;
    char buffer[1024];
    DWORD dwRead;

    hPipe = CreateNamedPipeA(TEXT("\\\\.\\pipe\\ShaderCompilerOutput"),
                        PIPE_ACCESS_DUPLEX,
                        PIPE_TYPE_MESSAGE | PIPE_WAIT | PIPE_READMODE_MESSAGE,
                        5,
                        1024 * 16,
                        1024 * 16,
                        NMPWAIT_USE_DEFAULT_WAIT,
                        NULL);

    if (hPipe == INVALID_HANDLE_VALUE)
        throw std::runtime_error("hPipe == nullptr");

    if (ConnectNamedPipe(hPipe, NULL) != FALSE)
        {
            while (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &dwRead, NULL) != FALSE)
            {
                buffer[dwRead] = '\0';

                printf("%s", buffer);
            }
        }

    DisconnectNamedPipe(hPipe);

    std::cin.get();

    return 0;
}