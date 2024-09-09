@echo off
setlocal

call scripts/config.cmd

REM  set ORE_ROOT_DIR=%CD%
echo ORE_ROOT_DIR=%ORE_ROOT_DIR%

set BIN=%ORE_ROOT_DIR%\bin


         REM  -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=%BIN%             ^
         REM  -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=%BIN%             ^
         REM  -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=%BIN%             ^
cmake.exe -B %BUILD% -G %GENERATOR% -A x64                  ^
         -DBOOST_LIBRARYDIR=%BOOST_LIBRARYDIR%              ^
         -DBOOST_ROOT=%BOOST% -DBoost_ROOT=%BOOST%          ^
         -DBUILD_SHARED_LIBS=OFF                            ^
         -DBoost_NO_SYSTEM_PATHS=ON                         ^
         -DORE_BUILD_DOC=OFF                                ^
         -DORE_BUILD_EXAMPLES=OFF                           ^
         -DORE_BUILD_TESTS=OFF                              ^
         -DORE_USE_ZLIB=OFF                                 ^
         -DQL_BUILD_BENCHMARK=OFF                           ^
         -DQL_BUILD_EXAMPLES=OFF                            ^
         -DQL_BUILD_TEST_SUITE=OFF                          ^
         -DQL_ENABLE_SESSIONS=%QL_ENABLE_SESSIONS%          ^
         -DQL_ENABLE_TRACING=%QL_ENABLE_TRACING%            ^
         -DQL_USE_STD_CLASSES=%QL_USE_STD_CLASSES%          ^
         -DWINVER=%WINVER%                                  ^
         -DCMAKE_BUILD_TYPE=%BUILD_TYPE%                    ^
         -DMSVC_LINK_DYNAMIC_RUNTIME=false

cmake.exe --build %BUILD% --parallel 8 --config %BUILD_TYPE% --verbose

endlocal
