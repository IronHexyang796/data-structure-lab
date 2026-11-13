@echo off
rem ===========================================================================
rem  PIXEL FARM  -  build / run / test helper for Windows
rem
rem  This file is ASCII-only on purpose: a .bat with Chinese text breaks
rem  cmd.exe's parser on some Windows setups (codepage mismatch), and that
rem  failure looks like "nothing happens when I press the button".
rem  All Chinese instructions live in the task book and docs/.
rem
rem  Usage (run it in the project folder, or just press Ctrl+Shift+B):
rem
rem    pf               compile
rem    pf run           compile + play
rem    pf test          run all tests
rem    pf test 3        only week 3
rem    pf demo          auto-play 30 days
rem    pf json          export web/data.json
rem    pf serve         start the web server
rem    pf clean         remove build/ and farm.exe
rem    pf help
rem ===========================================================================
setlocal EnableDelayedExpansion
cd /d "%~dp0"

set "TASK=%~1"
if "%TASK%"=="" set "TASK=build"
set "WEEK=%~2"

if /i "%TASK%"=="help"   goto :usage
if /i "%TASK%"=="-h"     goto :usage
if /i "%TASK%"=="--help" goto :usage

rem --------------------------------------------------------------- check gcc
where gcc >nul 2>nul
if errorlevel 1 (
    echo.
    echo   [ERROR] gcc not found on PATH.
    echo.
    echo   What to do:
    echo     1. Install a C compiler  -^> task book, chapter 02
    echo     2. Add its ...\bin folder to PATH  -^> chapter 02
    echo     3. Close ALL terminals and VSCode, then reopen
    echo.
    echo   Quick check: open a new terminal and type   gcc -v
    echo.
    exit /b 1
)

rem --------------------------------------------------------------- scan src/
rem  If src\<module>.c exists, define PF_HAVE_<MODULE> so the temporary
rem  implementation of that module inside src/stub.c is switched off.
set "DEFS="
set "HAVE="
set "PEND="
call :scan field    FIELD
call :scan growth   GROWTH
call :scan undo     UNDO
call :scan order    ORDER
call :scan map      MAP
call :scan tech     TECH
call :scan huffman  HUFF
call :scan road     ROAD
call :scan path     PATH
call :scan market   MARKET
call :scan index    INDEX

set "ENG="
for %%f in (engine\*.c) do set "ENG=!ENG! "%%f""
set "SRC="
for %%f in (src\*.c) do set "SRC=!SRC! "%%f""
set "CFLAGS=-std=c11 -Wall -Wextra -O2 -g -Iinclude -Iengine"

if /i "%TASK%"=="clean" goto :do_clean
if /i "%TASK%"=="test"  goto :do_test
if /i "%TASK%"=="run"   goto :do_run
if /i "%TASK%"=="demo"  goto :do_demo
if /i "%TASK%"=="json"  goto :do_json
if /i "%TASK%"=="serve" goto :do_serve
if /i "%TASK%"=="bench" goto :do_bench
if /i "%TASK%"=="build" goto :do_build
echo   Unknown command: %TASK%
goto :usage

rem ==================================================================== build
:do_build
echo.
echo   ------------------------------------------------------------
echo    PIXEL FARM  -  build
echo   ------------------------------------------------------------
echo    engine  : engine\*.c
echo    yours   : src\*.c
if defined HAVE echo    done    :!HAVE!
if defined PEND echo    still stub:!PEND!
echo.

tasklist /FI "IMAGENAME eq farm.exe" /NH 2>nul | findstr /I /C:"farm.exe" >nul
if not errorlevel 1 (
    echo   [WARN] farm.exe is still running - press Q in the game first,
    echo          otherwise linking will fail with "Permission denied".
    echo.
)

call gcc %CFLAGS% !DEFS! !ENG! !SRC! -o farm.exe -lws2_32
if errorlevel 1 (
    echo.
    echo   [FAILED] compile error - read the FIRST error above, fix it,
    echo            then press Ctrl+Shift+B again.
    echo            See the file in docs\ for the usual suspects.
    echo.
    exit /b 1
)

echo.
echo   [OK] farm.exe is ready.
echo        play   : pf run
echo        tests  : pf test
echo.
exit /b 0

rem ====================================================================== run
:do_run
call :build_quiet
if errorlevel 1 exit /b 1
echo   Arrow keys = move,  ? = help,  Q = quit.
echo.
farm.exe
exit /b 0

rem ===================================================================== demo
:do_demo
call :build_quiet
if errorlevel 1 exit /b 1
farm.exe --demo
exit /b 0

rem ===================================================================== json
:do_json
call :build_quiet
if errorlevel 1 exit /b 1
if not exist web mkdir web
farm.exe --json web\data.json
echo   written to web\data.json
exit /b 0

rem ==================================================================== serve
:do_serve
call :build_quiet
if errorlevel 1 exit /b 1
echo   Open http://localhost:8080 in a browser.  Ctrl+C to stop.
farm.exe --serve
exit /b 0

rem ==================================================================== bench
:do_bench
if not exist build mkdir build
echo.
echo   ------------------------------------------------------------
echo    build tools\bench.c  (week 9: 7 sorting algorithms)
echo   ------------------------------------------------------------
call gcc %CFLAGS% !DEFS! tools\bench.c engine\util.c engine\clock.c engine\json.c engine\state.c !SRC! -o build\bench.exe -lws2_32
if !ERRORLEVEL! NEQ 0 (
    echo.
    echo   [FAILED] bench.c did not compile.
    echo.
    exit /b 1
)
if not exist out mkdir out
build\bench.exe
exit /b 0

rem ===================================================================== test
:do_test
if not exist build mkdir build

set "PATT=t*"
if not "%WEEK%"=="" (
    set "PATT=t0%WEEK%_*"
    if "%WEEK%"=="10" set "PATT=t10_*"
)

set "N=0"
set "BAD=0"
echo.
echo   ------------------------------------------------------------
echo    build tests  (tests\!PATT!.c)
echo   ------------------------------------------------------------
for %%f in (tests\!PATT!.c) do (
    set /a N+=1
    if exist "build\%%~nf.exe" del /q "build\%%~nf.exe"
    echo    [cc] %%~nxf
    call gcc %CFLAGS% !DEFS! "%%f" engine\util.c engine\clock.c engine\json.c engine\state.c !SRC! -o "build\%%~nf.exe" -lws2_32
    if !ERRORLEVEL! NEQ 0 set "BAD=1"
)

if "%N%"=="0" (
    echo.
    echo   [ERROR] no such file: tests\!PATT!.c
    echo           run "pf test" to see all weeks.
    echo.
    exit /b 1
)

if "%BAD%"=="1" (
    echo.
    echo   [FAILED] a test program did not compile. Fix the error first.
    echo.
    exit /b 1
)

echo.
echo   ------------------------------------------------------------
echo    run tests
echo   ------------------------------------------------------------
for %%f in (tests\!PATT!.c) do (
    if exist "build\%%~nf.exe" (
        echo.
        echo   ================ %%~nxf ================
        "build\%%~nf.exe"
        if !ERRORLEVEL! NEQ 0 set "BAD=1"
    )
)

echo.
if "%BAD%"=="1" (
    echo   ------------------------------------------------------------
    echo    [FAIL] some checks failed. Fix them one by one and rerun.
    echo   ------------------------------------------------------------
) else (
    echo   ------------------------------------------------------------
    echo    [ALL GREEN] every check passed.
    echo   ------------------------------------------------------------
)
echo.
exit /b 0

rem ==================================================================== clean
:do_clean
if exist build rmdir /s /q build
if exist farm.exe del /q farm.exe
echo   clean: build\ and farm.exe removed
exit /b 0

rem ==================================================================== usage
:usage
echo.
echo   PIXEL FARM - commands
echo   ------------------------------------------------------
echo     pf              compile
echo     pf run          compile and play
echo     pf test         run all tests
echo     pf test 3       only week 3
echo     pf demo         auto-play 30 days
echo     pf json         export web\data.json
echo     pf serve        start the web server
echo     pf bench        week 9: compare 7 sorting algorithms
echo     pf clean        remove build\ and farm.exe
echo   ------------------------------------------------------
echo   In VSCode you normally do not type these:
echo     Ctrl+Shift+B    compile
echo     F5              run
echo     left panel "Run and Debug" has the rest
echo.
exit /b 0

rem ================================================================== helpers
:build_quiet
call gcc %CFLAGS% !DEFS! !ENG! !SRC! -o farm.exe -lws2_32
if errorlevel 1 (
    echo.
    echo   [FAILED] compile error - fix it first.
    echo.
    exit /b 1
)
exit /b 0

:scan
if exist "src\%~1.c" (
    set "DEFS=!DEFS! -DPF_HAVE_%~2"
    set "HAVE=!HAVE! %~1"
) else (
    set "PEND=!PEND! %~1"
)
exit /b
