:: This is the windows version of the foreground_heritrix shell script
:: The only difference to an invokation with "heritrix.cmd" is that no extra
:: (minimized) console window is created...
:: Caveats and version history, see heritrix.cmd
::
:: This script launches the heritrix crawler and keeps the window in foreground
::
:: Optional environment variables
::
:: JAVA_HOME        Point at a JDK install to use.
:: 
:: HERITRIX_HOME    Pointer to your heritrix install.  If not present, we 
::                  make an educated guess based of position relative to this
::                  script.
::
:: JAVA_OPTS        Java runtime options.
::
:: FOREGROUND       Set to any value -- e.g. 'true' -- if you want to run 
::                  heritrix in foreground (Used by build system when it runs
::                  selftest to see if completed successfully or not)..
::
@echo off
setlocal ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION
set PRGDIR=%~p0
set FOREGROUND=true
call "%PRGDIR%\heritrix.cmd" %*
