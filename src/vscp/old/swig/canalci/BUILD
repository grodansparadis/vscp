Builing the SWIG components on WIN32
====================================

Also check the file "SWIG install folder"/Doc/Manual/Windows.html


Perl
====

Set the environment variables

PERL5_INCLUDE : Set this to the directory containing perl.h
PERL5_LIB : Set this to the Perl library including path for linking

Example using ActivePerl

PERL5_INCLUDE: D:\perl\lib\CORE
PERL5_LIB: D:\perl\lib\CORE\perl58.lib

The custom build step in Visual C++ should be changed so that it points to the
SWIG executable.

Example:

echo In order to function correctly, please ensure the following environment variables are correctly set: 
echo PERL5_INCLUDE: %PERL5_INCLUDE% 
echo PERL5_LIB: %PERL5_LIB% 
echo on 
d:\dev\swigwin-1.3.25\swig.exe -perl5 $(InputPath) 


perl_cci.dll
perl_cci.pm

are needed by your program

runme.pl is a simple example that sends received CAN frames.



Python
======

PYTHON_INCLUDE : Set this to the directory that contains python.h
PYTHON_LIB : Set this to the python library including path for linking

Example using Python 2.1.1:
PYTHON_INCLUDE: d:\python21\include
PYTHON_LIB: d:\python21\libs\python21.lib



Java
====

JAVA_INCLUDE : Set this to the directory containing jni.h
JAVA_BIN : Set this to the bin directory containing javac.exe

Example using JDK1.3:
JAVA_INCLUDE: d:\jdk1.3\include
JAVA_BIN: d:\jdk1.3\bin


TCL
===
TCL_INCLUDE : Set this to the directory containing tcl.h
TCL_LIB : Set this to the TCL library including path for linking

Example using ActiveTcl 8.3.3.3
TCL_INCLUDE: d:\tcl\include
TCL_LIB: d:\tcl\lib\tcl83.lib


C#
==

