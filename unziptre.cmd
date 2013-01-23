@echo off
cd Unitex-C++\build
if not exist tre-0.8.0\lib\regcomp.c (
	echo Unzipping TRE library source
	untgz.exe ../tre-0.8.0.tar.gz
) else (
	echo No need to unzip TRE
)