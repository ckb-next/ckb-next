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

