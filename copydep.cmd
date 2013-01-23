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

set TARGET_DIR=%~1

if not exist %TARGET_DIR% ( mkdir %TARGET_DIR% )
if not exist %TARGET_DIR% (
	echo ERROR: Could not create %TARGET_DIR%
	goto error
)

set DESC_DIR=%TARGET_DIR%\desc

echo.
echo UimaAnnotatorCpp descriptors will be copied into %DESC_DIR%
echo.

xcopy /Q /Y desc\*.xml %DESC_DIR%\

echo Done copying descriptors
goto end

set DEP_DIR=%TARGET_DIR%\lib

if "%UIMACPP_HOME%" == "" goto Missing

echo.
echo UimaAnnotatorCpp's dependency libraries will be copied into %DEP_DIR%
echo.

echo Copying libraries from UIMA-C++ framework
set DEP_SOURCE=%UIMACPP_HOME%\bin
xcopy /Q /Y %DEP_SOURCE%\libapr*.dll %DEP_DIR%\
xcopy /Q /Y %DEP_SOURCE%\xerces-c*.dll %DEP_DIR%\
xcopy /Q /Y %DEP_SOURCE%\icu*.dll %DEP_DIR%\
xcopy /Q /Y %DEP_SOURCE%\uima*.dll %DEP_DIR%\
xcopy /Q /Y %DEP_SOURCE%\msvc*.dll %DEP_DIR%\

REM echo Copying libraries from Boost
REM xcopy /Q /Y %BOOST_ROOT%\lib\boost_filesystem-vc100-mt-1_51.lib;boost_system-vc100-mt-1_51.lib;boost_date_time-vc100-mt-1_51.lib;boost_thread-vc100-mt-1_51.lib

echo Done copying UimaAnnotatorCpp's dependency libraries
goto end

:Missing
echo UIMACPP_HOME must be specified
echo and must contain the UIMA-C++ framework
goto end

:error
echo FAILED: UimaAnnotatorCpp's dependency libraries not copied.

:end
