#include <Windows.h>
#include <stdexcept>
#include <iostream>

int main()
{
    HANDLE hPipe;
    char buffer[1024];
    DWORD dwRead;

    hPipe = CreateNamedPipeA(TEXT("\\\\.\\pipe\\Pipe"),
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