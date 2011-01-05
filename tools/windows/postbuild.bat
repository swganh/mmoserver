@echo off
SETLOCAL EnableDelayedExpansion

if not exist %1 (
    goto :eof
)

cd %1

if not exist %2\bin\%3\libglog.dll (
    xcopy %1\deps\src\glog\%3\libglog.dll %2\bin\%3\ /I /Y /s
)

if not exist %2\bin\%3\libmysql.dll (
    xcopy %1\deps\src\mysql-connector-c\libmysql\%3\libmysql.dll %2\bin\%3\ /I /Y /s
)

if not exist %2\bin\%3\mysqlcppconn.dll (
    xcopy %1\deps\src\mysql-connector-cpp\driver\%3\mysqlcppconn.dll %2\bin\%3\ /I /Y /s
)


if %3 == "Debug" (
    if not exist %2\bin\%3\tbb_debug.dll (
        xcopy %1\deps\src\tbb\build\vsproject\ia32\%3\tbb_debug.dll %2\bin\%3\ /I /Y /s
        xcopy %1\deps\src\tbb\build\vsproject\ia32\%3\tbbmalloc_debug.dll %2\bin\%3\ /I /Y /s
    )

    if not exist %2\bin\%3\zlibd1.dll (
        xcopy %1\deps\src\zlib\%3\zlibd1.dll %2\bin\%3\ /I /Y /s
    )
	
	if not exist %2\bin\%3\boost_python3-vc100-*.dll (
		xcopy %1\deps\src\boost\bin.v2\libs\python\build\msvc-10.0\debug\threading-multi\%3\boost_python3-vc100-*.dll %2\bin\%3 /I /Y /s
	)
)

if %3 == "Release" (
    if not exist %2\bin\%3\tbb.dll (
        xcopy %1\deps\src\tbb\build\vsproject\ia32\%3\tbb.dll %2\bin\%3\ /I /Y /s
        xcopy %1\deps\src\tbb\build\vsproject\ia32\%3\tbbmalloc.dll %2\bin\%3\ /I /Y /s
    )

    if not exist %2\bin\%3\zlib1.dll (
        xcopy %1\deps\src\zlib\%3\zlib1.dll %2\bin\%3\ /I /Y /s
    )
)

if not exist %2\bin\%3\config (
    xcopy %1\data\config\*.cfg %2\bin\%3\config\ /I /Y /s
)

if not exist %2\bin\%3\heightmaps (
    if exist %1\data\heightmaps (
        xcopy %1\data\heightmaps\*.hmpw %2\bin\%3\heightmaps\ /I /Y /s
    )
)

if not exist %2\bin\%3\logs (
    mkdir %2\bin\%3\logs
)

if not exist %2\bin\%3\script (
    xcopy %1\data\script %2\bin\%3\script\ /I /Y /s
)
if not exist %2\bin\%3\scripts (
    xcopy %1\data\scripts %2\bin\%3\scripts\ /I /Y /s
)

xcopy %1\data\ServerStart.bat %2\bin\%3\ /I /Y /s

%1"\tools\windows\gitversion.exe" > %2"\bin\%3\VERSION"

ENDLOCAL
