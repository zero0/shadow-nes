@echo off
setlocal

rem Get ESC character
for /F %%a in ('"prompt $E$S & echo on & for %%b in (1) do rem"') do set "ESC=%%a"

set "CC65_HOME=C:\cc65"
set "CC65_BIN=%CC65_HOME%\bin"
set "CC65_LIB=%CC65_HOME%\lib"
set "CC65_ASMINC=%CC65_HOME%\asminc"

set "CC65_CA=%CC65_BIN%\ca65.exe"
set "CC65_CC=%CC65_BIN%\cc65.exe"
set "CC65_LD=%CC65_BIN%\ld65.exe"

set "GAME_NAME=%1"
rem set "CPU_TYPE=--cpu 6502"
set "CPU_TYPE=-t nes"

set "CA_DBG=-g"
set "CC_DBG=-g"

rem Create bin directory for final .nes file
if not exist bin md bin

rem Clean tmp and obj directories
if exist tmp (del /Q tmp) else md tmp
if exist obj (del /Q obj) else md obj

rem Compile source assemblie files
for %%S in (asm\*.s) do %CC65_CA% %CPU_TYPE% -I lib -I %CC65_ASMINC% %CA_DBG% -o obj\%%~nS.o %%S

rem Compile C files
for %%C in (src\*.c) do %CC65_CC% %CPU_TYPE% -I include -I %CC65_ASMINC% --add-source %CC_DBG% -Oisr -o tmp\%%~nC.s %%C

rem Compile temp assemblie files
for %%S in (tmp\*.s) do %CC65_CA% %CPU_TYPE% -I lib -I %CC65_ASMINC% %CA_DBG% -o obj\%%~nS.o %%S

rem Build .o file list
set "OBJ_FILES="
for %%F in (obj\*.o) do call set "OBJ_FILES=%%F %%OBJ_FILES%%"

rem set "LD_CONFIG=-t nes"
rem set "LD_CONFIG=-C config\nrom_256_horz.cfg"
set "LD_CONFIG=-C config\nes.cfg"
set "LD_DBG=--dbgfile bin\%GAME_NAME%.dbg"
rem set "LD_DBG="

rem Link all .o files into .nes file
%CC65_LD% %LD_CONFIG% %LD_DBG% --lib-path lib --lib-path %CC65_LIB% -o bin\%GAME_NAME%.nes %OBJ_FILES% nes.lib || goto :fail

:success
echo %ESC%[92mCompile bin\%GAME_NAME%.nes success.%ESC%[0m
goto :end

:fail
echo %ESC%[91mCompile bin\%GAME_NAME%.nes failed.%ESC%[0m
goto :end

:end
endlocal
