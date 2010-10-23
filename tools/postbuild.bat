@echo off
SETLOCAL EnableDelayedExpansion

if not exist %1 (
	goto :eof
)

cd %1

if not exist %1bin\%2\libglog.dll (
    xcopy %1deps\glog\%2\libglog.dll %1bin\%2 /I /Y /s
)

if not exist %1bin\%2\libmysql.dll (
    xcopy %1deps\mysql\bin %1bin\%2 /I /Y /s
)

if not exist %1bin\%2\mysqlcppconn.dll (
    xcopy %1deps\mysql-connector-cpp\driver\%2 %1bin\%2 /I /Y /s
)

if %2 == "Debug" (
    if not exist %1bin\%2\tbb_debug.dll (
        xcopy %1deps\tbb\bin\ia32\vc10\tbb_debug.dll %1bin\%2 /I /Y /s
        xcopy %1deps\tbb\bin\ia32\vc10\tbb_debug.pdb %1bin\%2 /I /Y /s
    )
)

if %2 == "Release" (    
    if not exist %1bin\%2\tbb.dll (
        xcopy %1deps\tbb\bin\ia32\vc10\tbb.dll %1bin\%2 /I /Y /s
        xcopy %1deps\tbb\bin\ia32\vc10\tbb.pdb %1bin\%2 /I /Y /s
    )
)

if not exist %1bin\%2\config (
    xcopy %1data\config %1bin\%2\config /I /Y /s
)

if not exist %1bin\%2\logs (
    mkdir %1bin\%2\logs
)

%1tools\gitversion.exe > %1bin\%2\VERSION

xcopy %1data\script %1bin\%2\script /I /Y /s

if not exist %1bin\%2\heightmaps (
	if exist %1data\heightmaps (
		xcopy %1data\heightmaps\*.hmpw %1bin\%2\heightmaps /I /Y /s
	)
)

if not exist %1bin\%2\ServerStart.bat (
    xcopy %1data\ServerStart.bat %1bin\%2 /I /Y /s
)

ENDLOCAL