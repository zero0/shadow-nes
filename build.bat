@echo off
setlocal

rem Get ESC character
for /F %%a in ('"prompt $E$S & echo on & for %%b in (1) do rem"') do set "ESC=%%a"

set "BUILD_TYPE=DEBUG"

set "CC65_HOME=C:\cc65"
set "CC65_BIN=%CC65_HOME%\bin"
set "CC65_LIB=%CC65_HOME%\lib"
set "CC65_ASMINC=%CC65_HOME%\asminc"

set "CC65_CA=%CC65_BIN%\ca65.exe"
set "CC65_CC=%CC65_BIN%\cc65.exe"
set "CC65_LD=%CC65_BIN%\ld65.exe"

set "GAME_NAME=%1"
set "CPU_TYPE=-t nes"
set "COMPILE_ASSETS=0"

set "CA_FLAGS="
set "CC_FLAGS="
set "LD_FLAGS="

if %BUILD_TYPE%==DEBUG (
    set "CA_FLAGS=-g -D DEBUG_BUILD"
    set "CC_FLAGS=-g -D DEBUG_BUILD --add-source"
    set "LD_FLAGS=--dbgfile bin\%GAME_NAME%.dbg"
) else if %BUILD_TYPE%==RELEASE (
    set "CA_FLAGS=-g -D RELEASE_BUILD"
    set "CC_FLAGS=-g -O -D RELEASE_BUILD --add-source"
    set "LD_FLAGS=--dbgfile bin\%GAME_NAME%.dbg"
) else if %BUILD_TYPE%==DISTRO (
    set "CA_FLAGS=-D DISTRO_BUILD"
    set "CC_FLAGS=-O -Oi -D DISTRO_BUILD"
    set "LD_FLAGS="
) else (
    echo %ESC%[91mNo build type defined. DEBUG, RELEASE, or DISTRO required.%ESC%[0m
    goto :end
)

rem Create bin directory for final .nes file
if not exist bin md bin

rem Clean tmp and obj directories
if exist tmp (del /Q tmp) else md tmp
if exist obj (del /Q obj) else md obj

if %COMPILE_ASSETS%==1 (
    rem Run img2chr tool to generate .s files from images
    pushd tools\img2chr\
    dotnet run -c Release -- assets || goto :fail_assetbuild
    popd
)

set "GAME_ASSETS_INCLUDE=assets/generated/include"
set "GAME_ASSETS_ASM=assets/generated/asm"

rem Compile source assemblie files
for %%S in (asm\*.s) do %CC65_CA% %CPU_TYPE% -I lib -I %CC65_ASMINC% -I %GAME_ASSETS_ASM% %CA_FLAGS% -o obj\%%~nS.o %%S

rem Compile C files
for %%C in (src\*.c) do %CC65_CC% %CPU_TYPE% -I include -I %CC65_ASMINC% -I %GAME_ASSETS_INCLUDE% %CC_FLAGS% -Oisr -o tmp\c_%%~nC.s %%C

rem Compile temp assemblie files
for %%S in (tmp\*.s) do %CC65_CA% %CPU_TYPE% -I lib -I %CC65_ASMINC% %CA_FLAGS% -o obj\%%~nS.o %%S

rem Build .o file list
set "OBJ_FILES="
for %%F in (obj\*.o) do call set "OBJ_FILES=%%F %%OBJ_FILES%%"

rem set "LD_CONFIG=-t nes"
rem set "LD_CONFIG=-C config\nrom_256_horz.cfg"
rem set "LD_CONFIG=-C config\nes.cfg"
rem set "LD_CONFIG=-C config\nes_mmc1.cfg"
set "LD_CONFIG=-C config\nes_mmc5.cfg"

rem set "LD_DBG="

rem Link all .o files into .nes file
%CC65_LD% %LD_CONFIG% %LD_FLAGS% --lib-path lib --lib-path %CC65_LIB% -o bin\%GAME_NAME%.nes %OBJ_FILES% nes.lib || goto :fail

:success
echo %ESC%[92mCompile bin\%GAME_NAME%.nes success.%ESC%[0m
goto :end

:fail
echo %ESC%[91mCompile bin\%GAME_NAME%.nes failed.%ESC%[0m
goto :end

:fail_assetbuild
echo %ESC%[91mAsset Build for bin\%GAME_NAME%.nes failed.%ESC%[0m
goto :end

:end
endlocal
