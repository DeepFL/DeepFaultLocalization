:: This is the Windows version of the extractor shell script
:: Caveats and version history, see heritrix.cmd
::
:: This script runs the org.archive.crawler.extractor.ExtractorTool main.
:: Pass '--help' to get usage message.
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
@echo off
setlocal ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION
set PRGDIR=%~p0
set CLASS_MAIN=org.archive.crawler.extractor.ExtractorTool
call "%PRGDIR%\foreground_heritrix.cmd" %*
