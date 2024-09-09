@echo off
setlocal

call scripts/config.cmd

pushd %ORE_SWIG_DIR%

echo BUILD ORESWIG

cd %ORE_SWIG_DIR%\OREAnalytics-SWIG\Python

        REM  -DORE_LIB_DIR=%ORE_ROOT_DIR%\bin\Release                                        ^
cmake -Wno-dev -B %BUILD% -G %GENERATOR% -A x64                                         ^
        -DORE=%ORE_ROOT_DIR% -DORE_BUILD=%ORE_ROOT_DIR%/%BUILD%                         ^
        -DMSVC_LINK_DYNAMIC_RUNTIME=OFF -DORE_USE_ZLIB=OFF -DBoost_NO_SYSTEM_PATHS=ON   ^
        -DBOOST_LIBRARYDIR=%BOOST_LIBRARYDIR%                                           ^
        -DBOOST_ROOT=%BOOST% -DBoost_ROOT=%BOOST%                                       ^
        -DQL_ENABLE_SESSIONS=%QL_ENABLE_SESSIONS%                                       ^
        -DQL_ENABLE_TRACING=%QL_ENABLE_TRACING%                                         ^
        -DQL_USE_STD_CLASSES=%QL_USE_STD_CLASSES%                                       ^
        -DCMAKE_BUILD_TYPE=%BUILD_TYPE%                                                 ^
        -DWINVER=%WINVER%                                  

cmake --build %BUILD% --parallel 8 --config %BUILD_TYPE% --verbose


SET PYTHONPATH=%ORE_SWIG_DIR%\OREAnalytics-SWIG\Python\%BUILD%;%ORE_SWIG_DIR%\OREAnalytics-SWIG\Python\build\Release

python setup.py wrap
python setup.py build
python setup.py bdist_wheel

popd


endlocal


