#!/bin/bash

# Get the directory this file is in (the project base)
basedir=$(cd $(dirname $0) && pwd)

# Configuration and options
version=0.4.3       # mmoserver project version
cflag=              # clean build - removes the build dir before building
rflag=              # rebuild - rebuilds the dependencies and source
tflag=              # test - runs all project tests after building

# parse the command line parameters
while getopts 'crt' OPTION 
do case "$OPTION" in
    c)  cflag=1
        ;;
    r)  rflag=1
        ;;
    t)  tflag=1
        ;;
    ?)  printf "Builds the MMOServer project and dependencies from source\n"
        printf ""
        printf "\nUsage: %s [-r] [-t]\n" $(basename $0) >&2
        printf "\n"
        printf "    -r  Rebuild dependencies and source\n"
        printf "    -t  Run the project test suite\n\n"
        exit 2
        ;;
esac
done
shift $(($OPTIND - 1))

# Check to see if the current dependencies should be cleaned
if [[ -d $basedir/deps && "$rflag" ]]; then
    printf "Rebuilding deps\n"
    rm -rf $basedir/deps
fi

# If there are no dependencies attempt to build them
if [ ! -d $basedir/deps ]; then
    filename=mmoserver-deps-$version.tar.bz2
    
    # Look for the dependencies source file and download if missing
    if [ ! -f $basedir/$filename ]; then
        wget -nc --no-check-certificate https://github.com/downloads/swganh/mmoserver/$filename
    fi
    
    # Unpack the dependencies and build the deps
    tar -xvjf $basedir/$filename    
    $basedir/deps/build_deps.sh
fi

if [ -d $basedir/build ]; then
    # Remove any tests from a previous run
    rm -f $basedir/build/*Tests.xml
    
    # Check to see if the build directory should be cleaned
    if [[ "$rflag" || "$cflag" ]]; then        
        printf "Rebuilding source\n"
        rm -rf $basedir/build
    fi
fi

# If no build directory exists create it
if [ ! -d $basedir/build ]; then
    mkdir $basedir/build
fi

# Run cmake from the build dir to ensure the build environment is up-to-date
cd $basedir/build
cmake -DCMAKE_INSTALL_PREFIX=$basedir -DENABLE_TEST_REPORT=ON ..

# If we're on the darwin platform set the DYLD_LIBRARY_PATH before running
# TODO: Libraries should be installed with the exe to prevent this from being
#   necessary.
os=`uname`

if [ "Darwin" == "$os" ]; then
    export DYLD_LIBRARY_PATH=$basedir/deps/mysql/lib:$basedir/deps/mysql-connector-cpp/lib:$basedir/deps/tbb/lib:$PATH
fi

# Build the project
make

# If tests are enabled run them
if [ "$tflag" ]; then
    make test
fi
