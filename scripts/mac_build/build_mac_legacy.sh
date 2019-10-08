#!/bin/bash
if [[ "$PATH" == *"/usr/local/opt/qt/bin"* ]]; then
    echo "Brew's Qt is in your PATH (/usr/local/opt/qt/bin). Please remove it and try again."
    exit
fi

mkdir build
cd build
Qt5_DIR=~/Qt5.6.3/5.6.3/clang_64/ cmake -DCMAKE_OSX_DEPLOYMENT_TARGET=10.10 -DCMAKE_BUILD_TYPE=Release -DSAFE_INSTALL=ON -DSAFE_UNINSTALL=ON -DMAC_LEGACY=1 .. -DUSE_BREW_QUAZIP=0 -DUSE_BREW_QT5=0 -DUSE_PORTAUDIO=1 -DWITH_MVIZ=1
make -j4 macos-package
