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

add_library(kissfft STATIC kiss_fft.c)

target_sources(
    kissfft
        PRIVATE
          _kiss_fft_guts.h
          kiss_fft.h
          kiss_fftr.c
          kiss_fftr.h)

target_include_directories(
    kissfft
        PRIVATE
          ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(
    kissfft
        PRIVATE
          m)

target_compile_features(
    kissfft
        PRIVATE
          c_std_11)

if (C_COMPILER_SUPPORTS_-Og)
    set(opt_lvl "-Og")
else ()
    set(opt_lvl "-O0")
endif ()

target_compile_options(
    kissfft
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

add_sanitizers(kissfft)
