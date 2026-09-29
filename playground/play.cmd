@echo off
rem Usage: play GRAMMAR [FILE] [options]     e.g.  play Calc examples\basic.calc --tokens
rem Regenerates the parser if GRAMMAR.g4 changed, rebuilds if anything changed, then runs it.
setlocal
set "ROOT=%~dp0"
set "BUILD=%ROOT%build"
set "LOG=%BUILD%\last-build.log"

if "%~1"=="" goto :usage
set "GRAMMAR=%~1"
if not exist "%ROOT%grammars\%GRAMMAR%\%GRAMMAR%.g4" (
    echo No grammar named "%GRAMMAR%": expected grammars\%GRAMMAR%\%GRAMMAR%.g4
    echo Available grammars:
    for /d %%d in ("%ROOT%grammars\*") do echo   %%~nxd
    exit /b 1
)

rem Everything after the grammar name is passed through to the program.
set "ARGS=%*"
call set "ARGS=%%ARGS:*%1=%%"

if not exist "%BUILD%\CMakeCache.txt" (
    echo First run: configuring the build...
    cmake -S "%ROOT%." -B "%BUILD%" -G "MinGW Makefiles" || exit /b 1
)
if not exist "%BUILD%\bin\%GRAMMAR%.exe" (
    rem New grammar folder: re-run CMake so it creates a build target for it.
    cmake -S "%ROOT%." -B "%BUILD%" > nul || exit /b 1
    echo Building %GRAMMAR%... ^(the very first build also compiles the ANTLR runtime and takes a few minutes^)
)

cmake --build "%BUILD%" --target %GRAMMAR% -j 8 > "%LOG%" 2>&1
if errorlevel 1 (
    type "%LOG%"
    echo.
    echo BUILD FAILED - see the messages above. ANTLR grammar errors look like "error(NN): File.g4:line:col: ..."
    exit /b 1
)
rem Show ANTLR grammar warnings even when the build succeeds.
findstr /C:"warning(" "%LOG%"

set "PG_GRAMMAR_DIR=%ROOT%grammars\%GRAMMAR%"
"%BUILD%\bin\%GRAMMAR%.exe" %ARGS%
exit /b %errorlevel%

:usage
echo usage: play GRAMMAR [FILE] [options]
echo        play GRAMMAR --help      for all options
echo.
echo grammars:
for /d %%d in ("%ROOT%grammars\*") do echo   %%~nxd
exit /b 1
