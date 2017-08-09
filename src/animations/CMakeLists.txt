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

if (WITH_ANIMATIONS)
    add_subdirectory(libckbnextanim)

    if (WITH_GRADIENT)
        add_subdirectory(gradient)
    endif ()
    if (WITH_HEAT)
        add_subdirectory(heat)
    endif ()
    if (WITH_PINWHEEL)
        add_subdirectory(pinwheel)
    endif ()
    if (WITH_RAIN)
        add_subdirectory(rain)
    endif ()
    if (WITH_RANDOM)
        add_subdirectory(random)
    endif ()
    if (WITH_RIPPLE)
        add_subdirectory(ripple)
    endif ()
    if (WITH_WAVE)
        add_subdirectory(wave)
    endif ()
    if (WITH_MVIZ)
        add_subdirectory(mviz)
    endif ()
endif ()

