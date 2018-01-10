#   Copyright 2017-2018 ckb-next Development Team <ckb-next@googlegroups.com>
#   All rights reserved.
#
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions are met:
#   
#   1. Redistributions of source code must retain the above copyright notice,
#   this list of conditions and the following disclaimer.
#   2. Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
#   3. Neither the name of the copyright holder nor the names of its
#   contributors may be used to endorse or promote products derived from this
#   software without specific prior written permission. 
#   
#   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
#   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#   POSSIBILITY OF SUCH DAMAGE.
#
#   See COPYING regarding the distribution of quazip library and not this file.

project(QuaZip VERSION 0.7.3)

add_library(quazip STATIC quazip.cpp)
add_library(QuaZip::quazip5 ALIAS quazip)

find_package(Qt5 REQUIRED COMPONENTS Core)
find_package(ZLIB REQUIRED)

set_target_properties(
    quazip
        PROPERTIES
          AUTOMOC ON
          AUTOUIC ON
          AUTORCC ON)

target_sources(
    quazip
        PRIVATE
          unzip.c
          zip.c
          JlCompress.cpp
          qioapi.cpp
          quaadler32.cpp
          quacrc32.cpp
          quagzipfile.cpp
          quaziodevice.cpp
          quazip.cpp
          quazipdir.cpp
          quazipfile.cpp
          quazipfileinfo.cpp
          quazipnewinfo.cpp
          JlCompress.h
          crypt.h
          ioapi.h
          quaadler32.h
          quachecksum32.h
          quacrc32.h
          quagzipfile.h
          quaziodevice.h
          quazip.h
          quazip_global.h
          quazipdir.h
          quazipfile.h
          quazipfileinfo.h
          quazipnewinfo.h
          unzip.h
          zip.h)

    target_link_libraries(
        quazip
            PUBLIC
              Qt5::Core
              ZLIB::ZLIB)

target_include_directories(
    quazip
        PUBLIC
          "${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties(
    quazip
        PROPERTIES
          CXX_STANDARD 11)

