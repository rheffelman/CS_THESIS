#pragma once

// Kept separate from the ANTLR headers because <windows.h> defines macros
// that clash with them.
namespace pg {

// Switch the Windows console to UTF-8 and turn on ANSI color codes.
void enableConsole();

// True when stdout is an interactive terminal (not redirected to a file).
bool stdoutIsTerminal();

} // namespace pg
