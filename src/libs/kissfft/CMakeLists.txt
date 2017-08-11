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

add_library(KissFFT kiss_fft.c)
add_library(KissFFT::KissFFT ALIAS KissFFT)

target_sources(
    KissFFT
        PRIVATE
          kiss_fftr.c)


target_include_directories(
    KissFFT
        PUBLIC
          "${CMAKE_CURRENT_SOURCE_DIR}/include"
          "${CMAKE_CURRENT_SOURCE_DIR}/include/kissfft")

target_link_libraries(
    KissFFT
        PUBLIC
          m)

set_target_properties(
    KissFFT
        PROPERTIES
          C_STANDARD 11)

if (C_COMPILER_SUPPORTS_-Og)
    set(opt_lvl "-Og")
else ()
    set(opt_lvl "-O0")
endif ()

target_compile_options(
    KissFFT
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

add_sanitizers(KissFFT)
