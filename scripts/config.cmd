@echo off

if not defined SWIG_DIR (
  set SWIG_DIR=D:\code\util\swigwin-4.2.1
)

echo "SWIG_DIR=%SWIG_DIR%"

set ORE_ROOT_DIR=D:\code\model\ore\Engine
set ORE_SWIG_DIR=D:\code\model\ore\ORE-SWIG

set ORE_DIR=%ORE_ROOT_DIR%

set PATH=D:\code\util\cmake-3.30.2-windows-x86_64\bin\;%PATH%
set PATH=%SWIG_DIR%;%PATH%

cmake --version

if not defined BOOST_ROOT (
  set BOOST_ROOT=d:\code\model\ore\boost_1_86_0
) 
echo "BOOST_ROOT=%BOOST_ROOT%"

set BOOST=%BOOST_ROOT%
set BOOST_LIB=%BOOST_ROOT%\stage\lib
set BOOST_LIBRARYDIR=%BOOST_LIB%


set GENERATOR="Visual Studio 17 2022"
set BUILD=build

set WINVER=0x0A00

set BUILD_TYPE=Release


REM ----------------------------------------
set ORE_STATIC_RUNTIME=1
