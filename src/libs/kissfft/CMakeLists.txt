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
          "${CMAKE_CURRENT_SOURCE_DIR}/include")

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
          $<$<CONFIG:Debug>:${opt_lvl}>
          -fsigned-char
          -Wall
          -Wextra
          -Wfloat-equal
          -Wundef
          -Wshadow
          -Wpointer-arith
          -Wcast-align
          -Wstrict-prototypes
          -Wstrict-overflow=5
          -Wwrite-strings
          -Waggregate-return
          -Wcast-qual
          -Wswitch-default
          -Wswitch-enum
          -Wconversion
          -Wformat=2
          -Winit-self
          $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:-save-temps>)

add_sanitizers(KissFFT)
