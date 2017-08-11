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

find_package(PulseAudio REQUIRED)
if (NOT ${PULSEAUDIOSIMPLE_FOUND})
    message(FATAL_ERROR "Missing PulseAudio Simple API")
endif ()

add_executable(mviz main.c)

target_include_directories(
    mviz
        PRIVATE
          ${PULSEAUDIOSIMPLE_INCLUDE_DIRS})

target_link_libraries(
    mviz
        PRIVATE
          m
          KissFFT::KissFFT
          CkbNext::Animation
          ${PULSEAUDIOSIMPLE_LIBRARIES})

set_target_properties(
    mviz
        PROPERTIES
          C_STANDARD 11)

if (C_COMPILER_SUPPORTS_-Og)
    set(opt_lvl "-Og")
else ()
    set(opt_lvl "-O0")
endif ()

target_compile_options(
    mviz
        PRIVATE
          -fsigned-char
          -Wall
          -Wextra
          -Winit-self
          $<$<CONFIG:Debug>:-Wfloat-equal>
          $<$<CONFIG:Debug>:-Wundef>
          $<$<CONFIG:Debug>:-Wshadow>
          $<$<CONFIG:Debug>:-Wpointer-arith>
          $<$<CONFIG:Debug>:-Wcast-align>
          $<$<CONFIG:Debug>:-Wstrict-prototypes>
          $<$<CONFIG:Debug>:-Wstrict-overflow=5>
          $<$<CONFIG:Debug>:-Wwrite-strings>
          $<$<CONFIG:Debug>:-Wcast-qual>
          $<$<CONFIG:Debug>:-Wswitch-default>
          $<$<CONFIG:Debug>:-Wswitch-enum>
          $<$<CONFIG:Debug>:-Wconversion>
          $<$<CONFIG:Debug>:-Wformat=2>
          $<$<CONFIG:Debug>:-save-temps>
          $<$<CONFIG:Debug>:${opt_lvl}>)

add_sanitizers(mviz)

install(
    TARGETS mviz
    DESTINATION ${INSTALL_DIR_ANIMATIONS})
