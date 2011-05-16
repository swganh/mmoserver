#!/bin/bash

# Get the directory this file is in (the project base)
builddir=$(pwd)/build
basedir=$(cd $(dirname $0) && pwd)

# this script accepts a directory in which to build as it's only parameter
if [ "$1" != "" ]; then
    if [ ! -d $1 ]; then     
        printf "Passed an invalid directory to build in!"
	    printf ""
	    printf "Create the directory you want to build in, then rerun this script."
	    exit 1
    fi

    builddir=$1
fi

# prepare a directory to build the project and enter it
if [ -d $builddir ]; then
    if [ "$(ls -A $builddir)" ]; then
        printf "Attempting to build in an existing, non-empty directory!"
        printf ""
	    printf "Please clear out the contents of the build directory and"
        printf "run this script again."
        exit 1
    fi
else
    cmake -E make_directory $builddir
fi

cd $builddir

cmake -DENABLE_TEST_REPORT=ON $basedir
cmake --build .
