set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(HEADERS
        crypt.h
        ioapi.h
        JlCompress.h
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
        zip.h
        )

set(SOURCES
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
        unzip.c
        zip.c
        )

add_library(LocalQuaZip STATIC ${SOURCES} ${HEADERS})

add_definitions(-DQUAZIP_STATIC)
