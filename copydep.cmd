@echo off
setlocal

REM Copy the UimaAnnotatorCpp's dependency libraries into the
REM lib subdirectory under the current configuration output directory.
REM
REM The configuration must be given as an argument.

if "%~1" == "" (
	echo ERROR: Configuration not specified
	goto error
)

if "%~1" == "Debug" (
	echo Debug distribution
	set UIMACPP_BIN_DIR=%UIMACPP_HOME%\bin-dbg
) else (
	echo Release distribution
	set UIMACPP_BIN_DIR=%UIMACPP_HOME%\bin
)
if not exist %UIMACPP_BIN_DIR% (
	echo ERROR: UIMA-C++ Framework binary directory does not exist 
	echo ERROR: %UIMACPP_BIN_DIR% not found
	goto error
)

set TARGET_DIR=%~1
echo Target directory is %TARGET_DIR%

if not exist %TARGET_DIR% ( mkdir %TARGET_DIR% )
if not exist %TARGET_DIR% (
	echo ERROR: Could not create %TARGET_DIR%
	goto error
)

set DESC_DIR=%TARGET_DIR%\desc

echo.
echo UimaAnnotatorCpp descriptors will be copied into %DESC_DIR%
echo.

xcopy /Y desc\*.xml %DESC_DIR%\

echo Done copying descriptors
REM goto end

set DEP_DIR=%TARGET_DIR%\lib

if "%UIMACPP_HOME%" == "" goto Missing

echo.
echo UimaAnnotatorCpp's dependency libraries will be copied into %DEP_DIR%
echo.

echo Copying libraries from UIMA-C++ framework
set DEP_SOURCE=%UIMACPP_BIN_DIR%
xcopy /Y %DEP_SOURCE%\libapr*.* %DEP_DIR%\
xcopy /Y %DEP_SOURCE%\xerces-c*.* %DEP_DIR%\
xcopy /Y %DEP_SOURCE%\icu*.* %DEP_DIR%\
xcopy /Y %DEP_SOURCE%\uima*.* %DEP_DIR%\
xcopy /Y %DEP_SOURCE%\msvc*.dll %DEP_DIR%\
if "%~1" == "Debug" (
	echo Copying Visual Studio debug databases if any into %DEP_DIR%
	xcopy /Q /Y %DEP_SOURCE%\*.pdb %DEP_DIR%\
)

REM echo Copying libraries from Boost
REM xcopy /Q /Y %BOOST_ROOT%\lib\boost_filesystem-vc100-mt-1_51.lib;boost_system-vc100-mt-1_51.lib;boost_date_time-vc100-mt-1_51.lib;boost_thread-vc100-mt-1_51.lib

if "%~1" == "Debug" (
	echo DEBUG mode
	echo Copying UnitexAnnotatorCpp.dll into %DEP_DIR%
	xcopy /Q /Y %TARGET_DIR%\UnitexAnnotatorCpp*.* %DEP_DIR%\
)

echo Done copying UimaAnnotatorCpp's dependency libraries
goto end

:Missing
echo UIMACPP_HOME must be specified
echo and must contain the UIMA-C++ framework
goto end

:error
echo FAILED: UimaAnnotatorCpp's dependency libraries not copied.

:end
