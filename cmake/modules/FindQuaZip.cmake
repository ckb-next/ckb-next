#   Copyright (C) 2005-2012 Sergey A. Tachenov
#   
#   This program is free software; you can redistribute it and/or modify it
#   under the terms of the GNU Lesser General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or (at
#   your option) any later version.
#   
#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
#   General Public License for more details.
#   
#   You should have received a copy of the GNU Lesser General Public License
#   along with this program; if not, write to the Free Software Foundation,
#   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

# QuaZip_FOUND               - QuaZip library was found
# QuaZip_INCLUDE_DIR         - Path to QuaZip include dir
# QuaZip_INCLUDE_DIRS        - Path to QuaZip and zlib include dir (combined from QuaZip_INCLUDE_DIR + ZLIB_INCLUDE_DIR)
# QuaZip_LIBRARIES           - List of QuaZip libraries
# QuaZip_ZLIB_INCLUDE_DIR    - The include dir of zlib headers

# First try to find QuaZip 1.0
find_package(QuaZip-Qt${QT_VERSION_MAJOR} QUIET)

IF (QuaZip_LIBRARIES)
    # in cache already
    SET(QuaZip_FOUND TRUE)
ELSE ()
    IF(QuaZip-Qt${QT_VERSION_MAJOR}_FOUND)
        message("-- Found QuaZip: QuaZip::QuaZip")
        SET(QuaZip_FOUND TRUE)
        SET(QuaZip_LIBRARIES "QuaZip::QuaZip" CACHE INTERNAL "")
        SET(QuaZip_INCLUDE_DIR "QuaZip::QuaZip" CACHE INTERNAL "")
    ELSE ()
        set(QuaZip_LIB_VERSION_SUFFIX ${QT_VERSION_MAJOR})
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
    ENDIF ()
ENDIF ()
