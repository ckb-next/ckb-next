# QuaZip_FOUND               - QuaZip library was found
# QuaZip_INCLUDE_DIR         - Path to QuaZip include dir
# QuaZip_INCLUDE_DIRS        - Path to QuaZip and zlib include dir (combined from QuaZip_INCLUDE_DIR + ZLIB_INCLUDE_DIR)
# QuaZip_LIBRARIES           - List of QuaZip libraries
# QuaZip_ZLIB_INCLUDE_DIR    - The include dir of zlib headers


IF (QuaZip_INCLUDE_DIRS AND QuaZip_LIBRARIES)
    # in cache already
    SET(QuaZip_FOUND TRUE)
ELSE (QuaZip_INCLUDE_DIRS AND QuaZip_LIBRARIES)
    IF (Qt5Core_FOUND)
        set(QuaZip_LIB_VERSION_SUFFIX 5)
    ENDIF ()
#        FIND_PACKAGE(PkgConfig)
#        pkg_check_modules(PC_QCA2 QUIET qca2)
#        pkg_check_modules(PC_QuaZip quazip)
    FIND_LIBRARY(QuaZip_LIBRARIES
            WIN32_DEBUG_POSTFIX d
            NAMES quazip${QuaZip_LIB_VERSION_SUFFIX} libquazip.dylib
            HINTS /usr/lib /usr/lib64 /usr/local/opt/lib
            PATH_SUFFIXES quazip
            )
    FIND_PATH(QuaZip_INCLUDE_DIR quazip.h
            HINTS /usr/include /usr/local/include /usr/local/bin /usr/local/opt/include
            PATH_SUFFIXES quazip quazip${QuaZip_LIB_VERSION_SUFFIX}
            )
    FIND_PATH(QuaZip_ZLIB_INCLUDE_DIR zlib.h HINTS /usr/include /usr/local/include)
    INCLUDE(FindPackageHandleStandardArgs)
    SET(QuaZip_INCLUDE_DIRS ${QuaZip_INCLUDE_DIR} ${QuaZip_ZLIB_INCLUDE_DIR})
    find_package_handle_standard_args(QuaZip DEFAULT_MSG QuaZip_LIBRARIES QuaZip_INCLUDE_DIR QuaZip_ZLIB_INCLUDE_DIR QuaZip_INCLUDE_DIRS)
ENDIF (QuaZip_INCLUDE_DIRS AND QuaZip_LIBRARIES)
