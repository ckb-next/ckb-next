#
#        Copyright © 2017 Oleksii Vilchanskyi <alexvilchansky@yahoo.com>
#        
#        This file is part of ckb-next.
#
#        ckb-next is free software: you can redistribute it and/or modify
#        it under the terms of the GNU General Public License version 2
#        as published by the Free Software Foundation.
#
#        ckb-next is distributed in the hope that it will be useful,
#        but WITHOUT ANY WARRANTY; without even the implied warranty of
#        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#        GNU General Public License version 2 for more details.
#
#        You should have received a copy of the GNU General Public License
#        version 2 along with ckb-next. If not, see
#        <https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
#

# Export version, etc. to source code
configure_file(ckbnextconfig.h.in ckbnextconfig.h)

# Include this directory so that it's possible to write #include <ckbnextconfig.h> in source files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_subdirectory(daemon)

if (WITH_ANIMATIONS)
    add_subdirectory(animations)
endif ()

if (WITH_GUI)
    add_subdirectory(gui)
endif ()
