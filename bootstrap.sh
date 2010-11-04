#!/bin/sh

basedir=$(cd $(dirname $0) && pwd)

filename=mmoserver-deps-0.4.0-src.tar.gz

if [ -d $basedir/deps ]; then
    rm -rf $basedir/deps
fi

if [ ! -f $basedir/$filename ]; then
    wget -nc --no-check-certificate https://github.com/downloads/swganh/mmoserver/$filename
fi

tar -xvzf $basedir/$filename

./deps/build_deps.sh

if [ -d $basedir/build ]; then
    rm -rf $basedir/build
fi

mkdir $basedir/build
cd $basedir/build

cmake ..

make