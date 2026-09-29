@echo off
rem Usage: new NAME    creates grammars\NAME\ from templates\ (NAME must start with an uppercase letter)
setlocal
set "ROOT=%~dp0"
if "%~1"=="" (
    echo usage: new NAME     e.g.  new Json
    exit /b 1
)
set "NAME=%~1"
set "DIR=%ROOT%grammars\%NAME%"
if exist "%DIR%" (
    echo grammars\%NAME% already exists
    exit /b 1
)
mkdir "%DIR%\examples"
powershell -NoProfile -Command ^
  "foreach ($f in 'Template.g4','main.cpp','examples\example.txt') {" ^
  "  $src = Join-Path '%ROOT%templates' $f;" ^
  "  $dst = Join-Path '%DIR%' ($f -replace 'Template', '%NAME%');" ^
  "  $text = (Get-Content -Raw $src) -creplace 'TEMPLATE', '%NAME%';" ^
  "  [IO.File]::WriteAllText($dst, $text)" ^
  "}" || exit /b 1

rem Re-run CMake so it notices the new grammar folder.
if exist "%ROOT%build\CMakeCache.txt" cmake -S "%ROOT%." -B "%ROOT%build" > nul

echo Created grammars\%NAME%\
echo   %NAME%.g4              the grammar
echo   main.cpp              your visitor + start rules
echo   examples\example.txt  sample input
echo.
echo Try:  play %NAME% examples\example.txt
