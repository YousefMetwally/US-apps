@echo off

:: Fix issue with "Run as Administrator" current dir
setlocal enableextensions
cd /d "%~dp0"

set PLATFORM=%1
if NOT DEFINED PLATFORM (
  set PLATFORM=x64
)

:: Add CMake & Qt to PATH
set PATH=%PATH%;C:\Program Files\Cmake\bin
if %PLATFORM% == x64 (
  if DEFINED QT_ROOT_64 (
    set "PATH=%QT_ROOT_64%\bin;%PATH%"
  )
) else (
  if DEFINED QT_ROOT_32 (
    set "PATH=%QT_ROOT_32%\bin;%PATH%"
  )
)

:: Generate projects and solution file
:: Add "-T v142" to explicitly build with VS2019
cmake.exe -A %PLATFORM% -DCMAKE_CONFIGURATION_TYPES=Release;Debug .

::pause
