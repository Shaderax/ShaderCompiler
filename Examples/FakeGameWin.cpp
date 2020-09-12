#include <Windows.h>
#include <stdexcept>

int main(void)
{
    HANDLE hPipe;
    DWORD dwRead;

    hPipe = CreateFile(TEXT("\\\\.\\pipe\\ShaderCompilerOutput"), 
                       GENERIC_READ | GENERIC_WRITE, 
                       0,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL);

    if (hPipe == INVALID_HANDLE_VALUE)
        throw std::runtime_error("hPipe == INVALID_HANDLE_VALUE");

    char buffer[1024];
    
    while (1)
    {
        if (ConnectNamedPipe(hPipe, NULL) != FALSE)
        {
            while (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &dwRead, NULL) != FALSE)
            {
                buffer[dwRead] = '\0';

                printf("%s", buffer);
            }
        }
    }
    CloseHandle(hPipe);

    return (0);
}