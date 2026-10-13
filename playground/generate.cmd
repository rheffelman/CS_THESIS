@echo off
rem Runs the ANTLR tool: turns Test.g4 into Python files in the generated\ folder.
rem Run this again every time you change Test.g4.
java -jar "%USERPROFILE%\.m2\repository\org\antlr\antlr4\4.13.2\antlr4-4.13.2-complete.jar" -Dlanguage=Python3 -visitor -o generated -Xexact-output-dir Test.g4
