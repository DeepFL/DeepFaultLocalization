:: This script launches the heritrix crawler on windows.  While Heritrix
:: is unsupported on windows, see 2.1.1.3 in the User Manual
:: [http://crawler.archive.org/articles/user_manual.html], this script was
:: provided by Eric Jensen as a convenience to the windows-afflicted.
::
:: It is a direct translation of the heritrix linux wrapper script -- and
:: because windows is not supported on Heritrix, it will likely lag the unix
:: start script.
::
:: See also:
:: https://sourceforge.net/tracker/index.php?func=detail&aid=1514538&group_id=73833&atid=539102
::
:: Versions:
::
:: 2006-07-17  Original Version by Eric Jensen
::
:: 2006-08-04  Disclaimer added by Michael Stack
::
:: 2006-08-28  A few fixes by Max Schfmann:
::             - command extensions and veriable expansion are automatically
::               enabled
::             - JMX configuration fixed (not the fancy "sed" stuff however)
::             - Try to set permissions of JMX password file if Heritrix
::               fails to start and JMX is enabled
::             - a few more small improvements (java detection, fake background
::               execution...)
::             - comments changed from rem to :: and file renamed to .cmd 
::               (to make clear it won't work on Win 9x...)
::
:: 2006-09-03  Updated error handling (Max):
::             - Do justice to compulsory --admin switch
::             - Handle missing JMX permissions file (it's not created
::               automatically on Windows as we have nothing like sed)
::             - No double confirmation for fixing permissions
::             - More verbose on errors
::
:: 2006-09-07  REALLY fixed the classpath this time (Max)
::
:: 2007-01-06  Minor improvements (Max):
::             - Simplified code (using setlocal) 
::             - Removed old comments
::             - Show hint when admin switch / nowui wasn't given
::             - Start message wasn't shown on slow/busy machines
::
::  Optional environment variables
:: 
::  JAVA_HOME        Point at a JDK install to use.
::  
::  HERITRIX_HOME    Pointer to your heritrix install.  If not present, we 
::                   make an educated guess based of position relative to this
::                   script.
:: 
::  HERITRIX_OUT     Pathname to the Heritrix log file written when run in
::                   daemon mode.
::                   Default setting is %HERITRIX_HOME%\heritrix_out.log
:: 
::  JAVA_OPTS        Java runtime options.  Default setting is '-Xmx256m'.
:: 
::  FOREGROUND       Set to any value -- e.g. 'true' -- if you want to run 
::                   heritrix in foreground (Used by build system when it runs
::                   selftest to see if completed successfully or not).
:: 
::  JMX_OPTS         Default is to startup the JVM JMX administration 
::                   on port 8849 if the JVM is SUN JVM 1.5.  This allows JMX
::                   administration of Heritrix.  If the JVM is other than the
::                   SUN JDK 1.5, the arguments are ignored. If you do not want
::                   to start the JVM JXM administration server on the SUN JDK
::                   1.5, set this variable to empty string.
:: 
::  JMX_PORT         Port you'd like the JVM JMX administration server to run
::                   on. Default is 8849.
:: 
::  JMX_OFF          Set to a non-empty string to disable JMX (and JMX setup of
::                   password file, etc.)
:: 
@echo off
setlocal ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION
set PRG=%0
set PRGDIR=%~p0
:: windows doesn't have a sleep command build-in
set SLEEP=ping 127.0.0.1 -n 2 -w 1000

:: Main heritrix class.
if not defined CLASS_MAIN set CLASS_MAIN=org.archive.crawler.Heritrix
if "%CLASS_MAIN%"=="org.archive.crawler.Heritrix" (
    if "%1"=="" goto admin_switch_missing
)
if "%1"=="BGR" goto run_in_background
:: preserve original command line arguments
set HERITRIX_CMDLINE=%*

:: Read local heritrix properties if any.
:: To do this on Windows, tempor. rename .heritrixrc to heritrixrc.cmd
:: This is of course only useful if .heritrixrc contains Windows style "set VAR=value" statements
set RC_PATH=%HOMEPATH%
if "%RC_PATH%"=="\" set RC_PATH=\.
if defined HOMEDRIVE set RC_PATH=%HOMEDRIVE%!RC_PATH!
if exist "!RC_PATH!\.heritrixrc" (
    ren "!RC_PATH!\.heritrixrc" heritrixrc.cmd
    call "!RC_PATH!\heritrixrc.cmd"
    ren "!RC_PATH!\heritrixrc.cmd" .heritrixrc
)
set RC_PATH=

:: Set HERITRIX_HOME.
if defined HERITRIX_HOME goto find_java
set HERITRIX_HOME=%PRGDIR:~0,-4%
if "%PRGDIR:~-1%"=="\" set HERITRIX_HOME=%PRGDIR:~0,-5%

:: Find JAVA_HOME or java if JAVACMD is not defined.
:find_java
if defined JAVACMD goto java_found
if defined JAVA_HOME goto set_javacmd

:: Try to find java if neither JAVACMD nor JAVA_HOME is set:
java -version >nul 2>&1
:: 9009 means "command not found"
if errorlevel 9009 goto no_java_home
:: something else is wrong with executing java
if errorlevel 1 goto no_java_home

:: java seems to be in PATH
set JAVACMD=java -Dje.disable.java.adler32=true
:set_javacmd
if not defined JAVACMD set JAVACMD="%JAVA_HOME%\bin\java" -Dje.disable.java.adler32=true
:: It may be defined in env - including flags!!
:: See '[ 1482761 ] BDB Adler32 gc-lock OOME risk' for why we include the
:: 'je.disable.java.adler32'.
:java_found

:: Ignore previous classpath.  Build one that contains heritrix jar and content
:: of the lib directory into the variable CP.
set CP=
for %%j in ("%HERITRIX_HOME%\*.jar" "%HERITRIX_HOME%\lib\*.jar") do set CP=!CP!;%%j
set CLASSPATH=!CP!

:: Make sure of java opts.
if not defined JAVA_OPTS set JAVA_OPTS= -Xmx256m

:: Enable JMX?
if not defined JMX_OFF goto configure_jmx
goto jmx_configured

:configure_jmx
if not defined JMX_PORT set JMX_PORT=8849
if not defined JMX_OPTS set JMX_OPTS=-Dcom.sun.management.jmxremote.port=%JMX_PORT% -Dcom.sun.management.jmxremote.ssl=false "-Dcom.sun.management.jmxremote.password.file=%HERITRIX_HOME%\jmxremote.password"

:jmx_configured

:: heritrix_dmesg.log contains startup output from the crawler main class. 
:: As soon as content appears in this log, this shell script prints the 
:: successful (or failed) startup content and moves off waiting on heritrix
:: startup. This technique is done so we can show on the console startup 
:: messages emitted by java subsequent to the redirect of stdout and stderr.
set startMessage=%HERITRIX_HOME%\heritrix_dmesg.log

:: Remove any file that may have been left over from previous starts.
if exist "%startMessage%" del "%startmessage%"
if exist "%HERITRIX_HOME%\jmx_permissions_broken" del "%HERITRIX_HOME%\jmx_permissions_broken"

:: Redirect stdout and stderr to a file.
:: Print start message with date, java version, java opts, ulimit, and uname.
if not defined HERITRIX_OUT set HERITRIX_OUT=%HERITRIX_HOME%\heritrix_out.log
set stdouterrlog=%HERITRIX_OUT%
echo %DATE% %TIME% Starting heritrix >>"%stdouterrlog%"
echo %OS% %COMPUTERNAME% %PROCESSOR_IDENTIFIER% >>"%stdouterrlog%"
%JAVACMD% %JAVA_OPTS% -version >>"%stdouterrlog%"  2>&1
echo JAVA_OPTS=%JAVA_OPTS% >>"%stdouterrlog%"

:: If FOREGROUND is set, run heritrix in foreground.
:start_heritrix
if not defined FOREGROUND goto run_in_background
%JAVACMD% "-Dheritrix.home=%HERITRIX_HOME%" -Djava.protocol.handler.pkgs=org.archive.net "-Dheritrix.out=%HERITRIX_OUT%" %JAVA_OPTS% %JMX_OPTS% %CLASS_MAIN% %HERITRIX_CMDLINE%
:: errorlevel 130 if aborted with Ctrl+c (at least my sun jvm 1.5_07...)
if errorlevel 130 goto end
if errorlevel 1 goto handle_errors
goto end

:run_in_background
if not "%1"=="BGR" (
    start /MIN cmd /E:ON /F:ON /V:ON /c %PRG% BGR
    goto wait_for_log_file
) else (
    title Heritrix
    :: adding  ">>%stdouterrlog% 2>&1" causes an access denied error as heritrix writes also to this file	
    %JAVACMD% "-Dheritrix.home=%HERITRIX_HOME%" -Djava.protocol.handler.pkgs=org.archive.net "-Dheritrix.out=%HERITRIX_OUT%" %JAVA_OPTS% %JMX_OPTS% %CLASS_MAIN% %HERITRIX_CMDLINE%	
    if errorlevel 130 goto end
    if errorlevel 1 echo.!ERRORLEVEL! >"%HERITRIX_HOME%\jmx_permissions_broken"
	pause
	)
goto end

:wait_for_log_file
SET HERITRIX_COUNTER=
echo WARNING: It's currently not possible to run Heritrix in background
echo          on Windows. It was just started minimized in a new Window
echo          and will be shut down as soon as you log off.
echo.
echo %DATE% %TIME% Starting heritrix
:print_logfile
%SLEEP%>nul
if exist "%HERITRIX_HOME%\jmx_permissions_broken" (
    del "%HERITRIX_HOME%\jmx_permissions_broken"
    goto fix_jmx_permissions
)
if exist "%startMessage%" (
    goto print_and_delete_logfile
)
:: keep trying for 30 more seconds
if "!HERITRIX_COUNTER!"==".............................." goto start_may_failed
set HERITRIX_COUNTER=.!HERITRIX_COUNTER!
echo .
goto print_logfile

:print_and_delete_logfile
%SLEEP%>nul
%SLEEP%>nul
type "%startMessage%"
:: can happen when heritrix writes to the file at the same time
if errorlevel 1 goto print_logfile
del "%startMessage%" >nul 2>&1
:: del doesn't set the ERRORLEVEL var if unsuccessful, so we can't try again
goto end

:handle_errors

:fix_jmx_permissions
if exist "%startMessage%" type "%startMessage%"
if not "%CLASS_MAIN%"=="org.archive.crawler.Heritrix" goto end
if defined PERMISSIONS_FIXED goto fix_jmx_permission_failed
echo.
echo Heritrix failed to start properly. Possible causes:
echo.
echo - Login and password have not been specified (see --admin switch)
echo - another program uses the port for the web UI (8080 by default)
echo   (e.g. another Heritrix instance)
if defined JMX_OFF goto end
echo - JMX password file is missing or permissions not set correctly
echo.
if not exist "%HERITRIX_HOME%\jmxremote.password" goto permissions_file_missing
set /P FIXIT=Do you want to try to fix the permissions (Y/N)?
if /I "%FIXIT:~0,1%"=="n" goto end
echo y|cacls "%HERITRIX_HOME%\jmxremote.password" /P %USERNAME%:R >nul
if errorlevel 1 goto fix_jmx_permission_failed
set PERMISSIONS_FIXED=true
set /P RESTART=Restart Heritrix (Y/N)?
if /I "%RESTART:~0,1%"=="y" goto start_heritrix
goto end

:admin_switch_missing
echo You have to specify either a username and password for the 
echo web interface or start Heritrix without the web ui.
echo.
echo Example: %0 --admin=admin:letmein
echo          %0 --nowui myOrder.xml
goto end

:permissions_file_missing
echo.
echo JMX permissions file missing. A template can be found in
echo   %HERITRIX_HOME%\jmxremote.password.template.
echo Copy it to 
echo   %HERITRIX_HOME%\jmxremote.password 
echo and edit the passwords at the end of the file.
goto end

:fix_jmx_permission_failed
echo.
echo Either fixing the permissions failed or there was another problem.
echo You may have to set the ownership of the file 
echo   %HERITRIX_HOME%\jmxremote.password
echo manually to yourself (%USERNAME%) and restrict the access to read-only.
goto end

:start_may_failed
echo.
echo Starting Heritrix seems to have failed
goto end

:no_java_home
echo.
echo Please define either JAVA_HOME or JAVACMD or make sure java.exe is in PATH
goto end

:end