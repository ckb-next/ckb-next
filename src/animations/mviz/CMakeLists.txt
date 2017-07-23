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

project(mviz C)

find_package(PulseAudioSimple REQUIRED)

add_executable(mviz)

target_sources(mviz
                PRIVATE
                  main.c)

target_include_directories(mviz
                            PRIVATE
                              ${PULSEAUDIOSIMPLE_INCLUDE_DIRS})

target_link_libraries(mviz
                        PRIVATE
                          kissfft
                          m
                          ${PULSEAUDIOSIMPLE_LIBRARIES})

install(TARGETS mviz
        DESTINATION ${INSTALL_DIR_ANIMATIONS})

add_subdirectory(kissfft)
