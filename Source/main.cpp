#include "ShaderCompiler.hpp"
#include <chrono>
#include <thread>

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

    FW::FileWatcher* fileWatcher = 0;
    FW::WatchID watchID = 0;

    fileWatcher = new FW::FileWatcher();

    char buffer[1024];

    if (!CreatePipe())
        return -1;

    watchID = fileWatcher->addWatch(argv[1], new ShaderReCompiler(), true);

    while(1)
	{
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		fileWatcher->update();
	}

    Close();

    std::cin.get();

    return 0;
}