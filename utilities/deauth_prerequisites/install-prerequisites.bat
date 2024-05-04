@echo off
rem From nemo project https://github.com/n0xa/m5stick-nemo/tree/main/deauth_prerequisites
setlocal enabledelayedexpansion

set "baseFolder=%localappdata%\Arduino15\packages\m5stack\hardware\esp32"

rem Find the latest version of the folder
set "latestVersion="
for /d %%I in ("%baseFolder%\*") do (
    set "latestVersion=%%~nxI"
)

if not defined latestVersion (
    echo No version found.
    pause
    exit /b
)

rem Full path to the file to be edited
set "file=%baseFolder%\%latestVersion%\platform.txt"
set "backupFile=%baseFolder%\%latestVersion%\platform.txt.bkp"
set "prefix=build.extra_flags.esp32"
set "option= -w"

set "prefix2=compiler.c.elf.libs.esp32"
set "option2= -zmuldefs"

set "readyMark=##EVIL_DEAUTH_READY##"

rem Check if the ready mark already exists in the file
findstr /C:"%readyMark%" "%file%" >nul
if %errorlevel% equ 0 (
    echo The file is already OK.
    pause
    exit /b
)

rem Create a backup copy of the original file
copy "%file%" "%backupFile%"

rem Use PowerShell to edit the file
powershell -Command "(Get-Content '%file%') | ForEach-Object { $_ -replace ('^(%prefix%.*?)$', '$1%option%') -replace ('^(%prefix2%.*?)$', '$1%option2%') } | Set-Content '%file%'"

rem Add the ready mark to the end of the file
echo %readyMark%>> "%file%"

rem Open the latest version of the folder in Windows Explorer
start explorer "%baseFolder%\%latestVersion%"

echo Done.
pause

