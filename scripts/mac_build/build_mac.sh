#!/bin/bash
mkdir build
cd build
Qt5_DIR=/usr/local/opt/qt cmake -DCMAKE_BUILD_TYPE=Release -DSAFE_INSTALL=ON -DSAFE_UNINSTALL=ON -DMAC_LEGACY=0 .. -DUSE_BREW_QUAZIP=0 -DUSE_BREW_QT5=1 -DUSE_PORTAUDIO=1
make -j4 macos-package
