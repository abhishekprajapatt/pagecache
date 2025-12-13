@echo off
REM PageCache build script for Windows

setlocal enabledelayedexpansion

if not exist build (
    echo Creating build directory...
    mkdir build
)

echo Compiling PageCache library...

set "CXXFLAGS=-std=c++11 -Wall -Wextra -O3 -pthread -I."
set "CXX=g++"

REM Compile cache layer
echo [cache] Compiling Page.cpp...
%CXX% %CXXFLAGS% -c src\cache\Page.cpp -o build\Page.o
if errorlevel 1 goto error

echo [cache] Compiling PageCache.cpp...
%CXX% %CXXFLAGS% -c src\cache\PageCache.cpp -o build\PageCache.o
if errorlevel 1 goto error

echo [cache] Compiling Eviction.cpp...
%CXX% %CXXFLAGS% -c src\cache\Eviction.cpp -o build\Eviction.o
if errorlevel 1 goto error

REM Compile filesystem layer
echo [fs] Compiling Inode.cpp...
%CXX% %CXXFLAGS% -c src\fs\Inode.cpp -o build\Inode.o
if errorlevel 1 goto error

echo [fs] Compiling File.cpp...
%CXX% %CXXFLAGS% -c src\fs\File.cpp -o build\File.o
if errorlevel 1 goto error

REM Compile I/O layer
echo [io] Compiling ReadPath.cpp...
%CXX% %CXXFLAGS% -c src\io\ReadPath.cpp -o build\ReadPath.o
if errorlevel 1 goto error

echo [io] Compiling Writeback.cpp...
%CXX% %CXXFLAGS% -c src\io\Writeback.cpp -o build\Writeback.o
if errorlevel 1 goto error

echo [io] Compiling Readahead.cpp...
%CXX% %CXXFLAGS% -c src\io\Readahead.cpp -o build\Readahead.o
if errorlevel 1 goto error

REM Compile scheduler
echo [scheduler] Compiling IOThreadPool.cpp...
%CXX% %CXXFLAGS% -c src\scheduler\IOThreadPool.cpp -o build\IOThreadPool.o
if errorlevel 1 goto error

REM Compile metrics
echo [metrics] Compiling Counters.cpp...
%CXX% %CXXFLAGS% -c src\metrics\Counters.cpp -o build\Counters.o
if errorlevel 1 goto error

REM Compile API
echo [api] Compiling UserAPI.cpp...
%CXX% %CXXFLAGS% -c src\api\UserAPI.cpp -o build\UserAPI.o
if errorlevel 1 goto error

REM Create static library
echo Creating static library...
ar rcs build\libpagecache.a build\Page.o build\PageCache.o build\Eviction.o build\Inode.o build\File.o build\ReadPath.o build\Writeback.o build\Readahead.o build\IOThreadPool.o build\Counters.o build\UserAPI.o
if errorlevel 1 goto error

REM Compile tests
echo Compiling tests...
%CXX% %CXXFLAGS% -c tests\page_cache_tests.cpp -o build\page_cache_tests.o
if errorlevel 1 goto error

%CXX% %CXXFLAGS% build\page_cache_tests.o build\libpagecache.a -pthread -o build\test_page_cache.exe
if errorlevel 1 goto error

%CXX% %CXXFLAGS% -c tests\eviction_tests.cpp -o build\eviction_tests.o
if errorlevel 1 goto error

%CXX% %CXXFLAGS% build\eviction_tests.o build\libpagecache.a -pthread -o build\test_eviction.exe
if errorlevel 1 goto error

REM Compile benchmark
echo Compiling benchmark...
%CXX% %CXXFLAGS% -c src\bench\io_benchmark.cpp -o build\io_benchmark.o
if errorlevel 1 goto error

%CXX% %CXXFLAGS% build\io_benchmark.o build\libpagecache.a -pthread -o build\benchmark.exe
if errorlevel 1 goto error

echo.
echo ============================================
echo Build completed successfully!
echo ============================================
echo.
echo Executables:
echo   Tests:       build\test_page_cache.exe
echo                build\test_eviction.exe
echo   Benchmark:   build\benchmark.exe
echo.

goto end

:error
echo.
echo Build failed with error code %errorlevel%
echo.
exit /b 1

:end
endlocal
