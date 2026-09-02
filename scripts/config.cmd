@echo off
REM ============================================================================
REM  ORE build configuration - Windows / MSVC
REM
REM  Every value below is a DEFAULT ONLY. To override, set the environment
REM  variable BEFORE calling any build_*.bat, e.g.:
REM
REM      set BOOST_ROOT=C:\libs\boost_1_86_0
REM      set BUILD_TYPE=Debug
REM      scripts\build_msvc.bat
REM
REM  A fresh checkout on another machine normally only needs BOOST_ROOT (and
REM  SWIG_DIR / CMAKE_DIR if swig.exe / cmake.exe are not already on PATH).
REM
REM  Requires: Visual Studio 2022 (v143, C++20), CMake >= 3.15, Python 3.x,
REM            SWIG 4.x (for the Python wheel), Boost 1.86 built static.
REM ============================================================================

REM ---- Repo layout (derived from this script's location) ----------------------
set "ORE_ROOT_DIR=%~dp0.."
for %%I in ("%ORE_ROOT_DIR%") do set "ORE_ROOT_DIR=%%~fI"
if not defined ORE_SWIG_DIR set "ORE_SWIG_DIR=%ORE_ROOT_DIR%\ORE-SWIG"
set "ORE_DIR=%ORE_ROOT_DIR%"

REM ---- Toolchain locations ---------------------------------------------------
REM  Only needed when the tool is NOT already on PATH. If the directory exists
REM  it is prepended to PATH; otherwise we assume the tool is reachable.
if not defined SWIG_DIR  set "SWIG_DIR=D:\code\util\swigwin-4.2.1"
if not defined CMAKE_DIR set "CMAKE_DIR=D:\code\util\cmake-4.3.1-windows-x86_64\bin"
if exist "%CMAKE_DIR%\" set "PATH=%CMAKE_DIR%;%PATH%"
if exist "%SWIG_DIR%\"  set "PATH=%SWIG_DIR%;%PATH%"

REM ---- Boost ---------------------------------------------------------------
if not defined BOOST_ROOT set "BOOST_ROOT=%ORE_ROOT_DIR%\..\boost_1_86_0"
set "BOOST=%BOOST_ROOT%"
if not defined BOOST_LIB set "BOOST_LIB=%BOOST_ROOT%\stage\lib"
set "BOOST_LIB64=%BOOST_LIB%"
set "BOOST_LIBRARYDIR=%BOOST_LIB%"

REM ---- Build options (override any of these via env) -------------------------
if not defined GENERATOR          set "GENERATOR=Visual Studio 17 2022"
if not defined BUILD              set "BUILD=build"
if not defined BUILD_TYPE         set "BUILD_TYPE=Release"
if not defined WINVER             set "WINVER=0x0A00"
if not defined QL_ENABLE_SESSIONS set "QL_ENABLE_SESSIONS=ON"
if not defined QL_ENABLE_TRACING  set "QL_ENABLE_TRACING=ON"
if not defined QL_USE_STD_CLASSES set "QL_USE_STD_CLASSES=ON"
if not defined ORE_STATIC_RUNTIME set "ORE_STATIC_RUNTIME=1"
if not defined CPU_N              set "CPU_N=%NUMBER_OF_PROCESSORS%"

REM ---- Report --------------------------------------------------------------
echo ORE_ROOT_DIR=%ORE_ROOT_DIR%
echo ORE_SWIG_DIR=%ORE_SWIG_DIR%
echo SWIG_DIR=%SWIG_DIR%
echo CMAKE_DIR=%CMAKE_DIR%
echo BOOST_ROOT=%BOOST_ROOT%
echo GENERATOR=%GENERATOR%   BUILD=%BUILD%   BUILD_TYPE=%BUILD_TYPE%   CPU_N=%CPU_N%

where cmake >nul 2>nul || echo WARNING: cmake not found on PATH - set CMAKE_DIR or install CMake
where swig  >nul 2>nul || echo NOTE: swig not found on PATH - only needed for build_swig.bat (set SWIG_DIR)
cmake --version 2>nul
