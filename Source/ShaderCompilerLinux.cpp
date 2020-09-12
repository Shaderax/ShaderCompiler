#include <stdexcept>
#include <iostream>
#include <string>
#include <FileWatcher/FileWatcher.h>
#include <array>
#include <cstdio>
#include <sstream>
#include <memory>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/file.h>
#include <functional>

static const std::string gValidShaderExt[] = { "vert", "frag", "tesc", "tese", "geom", "comp"};

int gFd;
char* myfifo = "/tmp/ShaderCompilerOutput";
int rc;


bool CheckIfOnInstance( char* argv )
{
	std::ostringstream mutexPath;
	std::size_t hashed = std::hash<std::string>{}(argv);

	mutexPath << "/tmp/" << hashed;

	int pid_file = open(mutexPath.str().c_str(), O_CREAT | O_RDWR, 0666);

	if (pid_file == -1)
		return false;

	rc = flock(pid_file, LOCK_EX | LOCK_NB);

	if(rc) {
	    if(EWOULDBLOCK == errno)
			return false;
	}
	else {
		return true;
	}
}

std::string exec(const char* cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);

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
		if (action != FW::Action::Modified)
			return ;

        std::size_t pos = filename.rfind(".");
        for (const std::string& ext : gValidShaderExt)
        {
			if (pos != std::string::npos && filename.substr(pos + 1) == ext)
            {
        		std::cout << dir << "/" << filename << " I know what is that" << std::endl;

				// https://stackoverflow.com/questions/5782279/why-does-a-read-only-open-of-a-named-pipe-block
				gFd = open(myfifo, O_WRONLY | O_NONBLOCK);

                std::ostringstream cmd;

                std::string cmplPath =  dir + '/' + filename;
                std::cout << cmplPath << (int)action << std::endl;

                cmd << "glslc " << cmplPath << " -o " << cmplPath << ".spv" << " 2>&1";
                std::string log = exec(cmd.str().c_str());
                std::cout << "Exec return: " << log << std::endl;
				if (gFd != -1)
    				write(gFd, log.c_str(), log.size()+1); 
				else
					std::cout << "fd == -1: errno: " << errno << std::endl;
                std::cout << "End Log" << std::endl;

				close(gFd);

                return ;
            }
        }
        std::cout << dir << "/" << filename << " Don't know what is that" << std::endl;
	}
};

bool DirExists(const std::string& inDirName)
{
	DIR* dir = opendir(inDirName.c_str());

	if (dir)
	{
    	closedir(dir);
		return true;
	}
	else if (ENOENT == errno)
		return false;

	return false;
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

    mkfifo(myfifo, 0666); 
  
    gFileWatcher = new FW::FileWatcher();
    gWatchID = gFileWatcher->addWatch(argv[1], new ShaderReCompiler(), true);

    while(1)
	{
		sleep(1);
		gFileWatcher->update();
	}

	unlink(myfifo);

    std::cin.get();

    return 0;
}