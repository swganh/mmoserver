@echo off
SETLOCAL EnableDelayedExpansion

cd %1

if not exist "%1bin\%2\libmysql.dll" (
    xcopy "%1deps\mysql\bin" "%0bin\%2" /I /Y /s
)

if not exist "%1bin\%2\config" (
    xcopy "%1data\config" "%1bin\%2\config" /I /Y /s
)

if not exist "%1bin\%2\logs" (
    mkdir "%1bin\%2\logs"
)

"%1tools\gitversion.exe" > "%1bin\%2\VERSION"

xcopy "%1data\script" "%1bin\%2\script" /I /Y /s

if not exist "%1bin\%2\*.hmpw" (
    xcopy "%1data\heightmaps\*.hmpw" "%1bin\%2" /I /Y /s
)

if not exist "%1bin\%2\ServerStart.bat" (
    xcopy "%1data\ServerStart.bat" "%1bin\%2" /I /Y /s
)

ENDLOCAL