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
#        GNU General Public License for more details.
#       
#        You should have received a copy of the GNU General Public License
#        along with ckb-next. If not, see <http://www.gnu.org/licenses/>.
#

add_library(kissfft STATIC)

target_sources(kissfft
                PRIVATE
                  _kiss_fft_guts.h
                  kiss_fft.c
                  kiss_fft.h
                  kiss_fftr.c
                  kiss_fftr.h)

target_include_directories(kissfft
                            PRIVATE
                              ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(kissfft
                       PRIVATE
                         m)

add_subdirectory(kissfft)
