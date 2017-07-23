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

project(gradient C)

add_executable(gradient)

target_sources(gradient
                PRIVATE
                  main.c)

target_link_libraries(gradient
                        PRIVATE
                          m)

install(TARGETS gradient
        DESTINATION "${INSTALL_DIR_ANIMATIONS}")
