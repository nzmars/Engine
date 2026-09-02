@echo off
setlocal

call scripts/config.cmd

pushd "%ORE_SWIG_DIR%"

echo BUILD ORESWIG

REM  add whatever build\lib.* dir setuptools produced (Python-version agnostic)
for /d %%D in ("%ORE_SWIG_DIR%\build\lib.*") do set "PYTHONPATH=%%D;%PYTHONPATH%"

python setup.py wrap
if errorlevel 1 exit /b %errorlevel%
python setup.py build
if errorlevel 1 exit /b %errorlevel%
python setup.py bdist_wheel
if errorlevel 1 exit /b %errorlevel%

popd


endlocal


