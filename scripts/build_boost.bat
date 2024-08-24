@echo off
setlocal

call scripts/config.cmd

pushd %BOOST_ROOT%

REM  bootstrap.bat

.\b2.exe address-model=64 toolset=msc-14.3                ^
         define=_WIN32_WINNT=%WINVER%                     ^
         define=BOOST_USE_WINAPI_VERSION=%WINVER%         ^
         define=_WINVER=%WINVER% stage

popd

endlocal
