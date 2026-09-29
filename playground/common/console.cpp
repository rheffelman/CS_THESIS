#include "console.h"

#ifdef _WIN32
#include <initializer_list>
#include <io.h>
#include <stdio.h>
#include <windows.h>

void pg::enableConsole() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    for (DWORD which : {STD_OUTPUT_HANDLE, STD_ERROR_HANDLE}) {
        HANDLE h = GetStdHandle(which);
        DWORD mode = 0;
        if (h != INVALID_HANDLE_VALUE && GetConsoleMode(h, &mode))
            SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
}

bool pg::stdoutIsTerminal() { return _isatty(_fileno(stdout)) != 0; }

#else
#include <unistd.h>

void pg::enableConsole() {}
bool pg::stdoutIsTerminal() { return isatty(STDOUT_FILENO) != 0; }
#endif
