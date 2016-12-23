@echo off
setlocal EnableDelayedExpansion 

:: batch script that runs generateMetaData.py with either
::
:: -environment variable PYTHON
:: -p4sw location of python
:: -python.exe in PATH 
::
:: see readme.txt

:: look for python in p4 location unless PYTHON is set
if not defined PYTHON (
	call :find_root_path %~p0 "tools\python\3.3" _RESULT
	if not !_RESULT!==0 (
		set PYTHON=!_RESULT!\tools\python\3.3\python.exe
	)
)

:: look for python in PATH unless PYTHON is set
if not defined PYTHON (
	where python.exe > nul 2>&1
	IF !ERRORLEVEL! == 0 (
		set PYTHON=python.exe
	)
)

if defined PYTHON (
	echo using: %PYTHON%
	%PYTHON% generateMetaData.py %1
) else (
	echo no python found, please set PYTHON environment variable to python.exe path, or make sure python.exe is in PATH.
)

endlocal
exit /b 0

:: **************************************************************************
:: functions
:: **************************************************************************

:: find a root directory containing a known directory (as a hint)
:find_root_path
	setlocal
	set START_DIR=%~1
	set CONTAINED_DIR=%~2
	
	:: search directory tree
	set TMP_DIR=!START_DIR!
	set OUT_DIR=0
	:find_root_path_loop
	if exist !TMP_DIR!\!CONTAINED_DIR! goto :find_root_path_loop_end
	set TMP_DIR=!TMP_DIR!..\
	
	:: normalize path
	pushd !TMP_DIR!
	set OUT_DIR=!CD!
	popd
	
	:: break if we reach the root, by checking the last two charactors
	if "!OUT_DIR:~-2!"==":\" (
		set OUT_DIR=0
		goto :find_root_path_loop_end
	)
	
	goto :find_root_path_loop
	:find_root_path_loop_end
		
	::echo no idea why we need to use % here
	endlocal & set _RESULT=%OUT_DIR%
	exit /b 0

