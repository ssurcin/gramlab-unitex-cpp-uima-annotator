#!/bin/bash

# Copy the UimaAnnotatorCpp's dependency libraries into the
# lib subdirectory under the current configuration output directory.
#
# The configuration must be given as an argument.

if [ -z "$1" ]; then
	echo ERROR: Configuration not specified
	exit -1
fi

if [ "$1" = "Debug" ]; then
	echo Debug distribution
	UIMACPP_BIN_DIR=$UIMACPP_HOME/bin-dbg
	UIMACPP_LIB_DIR=$UIMACPP_HOME/lib-dbg
else
	echo Release distribution
	UIMACPP_BIN_DIR=$UIMACPP_HOME/bin
	UIMACPP_LIB_DIR=$UIMACPP_HOME/lib
fi
if [ ! -d $UIMACPP_BIN_DIR ]; then
	echo ERROR: UIMA-C++ Framework binary directory does not exist 
	echo ERROR: $UIMACPP_BIN_DIR not found
	exit -1
fi

TARGET_DIR="$1"
echo Target directory is $TARGET_DIR

if [ ! -d $TARGET_DIR ]; then
	mkdir $TARGET_DIR
fi
if [ ! -d $TARGET_DIR ]; then
	echo ERROR: Could not create $TARGET_DIR
	exit -1
fi

DESC_DIR=$TARGET_DIR/desc
if [ ! -d $DESC_DIR ]; then
	mkdir $DESC_DIR
fi

echo
echo UimaAnnotatorCpp descriptors will be copied into $DESC_DIR
echo 

cp desc/*.xml $DESC_DIR

echo Done copying descriptors

DEP_DIR=$TARGET_DIR/lib
if [ ! -d $DEP_DIR ]; then
	mkdir $DEP_DIR
fi

if [ -z "$UIMACPP_HOME" ]; then
	echo UIMACPP_HOME must be specified
	echo and must contain the UIMA-C++ framework
	exit -1
fi

echo
echo UimaAnnotatorCpp dependency libraries will be copied into $DEP_DIR
echo

echo Copying libraries from UIMA-C++ framework
cp $UIMACPP_LIB_DIR/libapr*.* $DEP_DIR
cp $UIMACPP_LIB_DIR/libxerces-c*.* $DEP_DIR
cp $UIMACPP_LIB_DIR/libicu*.* $DEP_DIR
cp $UIMACPP_LIB_DIR/libuima*.* $DEP_DIR

echo Done copying UimaAnnotatorCpp dependency libraries

