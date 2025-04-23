@echo off
setlocal

rem Deploy using ant
ant deploy-nes
goto :end

rem Get ESC character
for /F %%a in ('"prompt $E$S & echo on & for %%b in (1) do rem"') do set "ESC=%%a"

set "CC65_HOME=C:\cc65"
set "EDLINK_EXE=%CC65_HOME%\edlink-n8.exe"

set "GAME_NAME=%1.nes"
set "GAME_PATH=bin\%GAME_NAME%"

rem Deploy using edlink
%EDLINK_EXE% %GAME_PATH% || goto :fail

:success
echo %ESC%[92mDeploy %GAME_PATH% success.%ESC%[0m
goto :end

:fail
echo %ESC%[91mDeploy %GAME_PATH% failed.%ESC%[0m
goto :end

:end
endlocal
