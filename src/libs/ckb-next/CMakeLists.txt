#   Copyright 2017-2018 ckb-next Development Team <ckb-next@googlegroups.com>
#   All rights reserved.
#
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions are met:
#   
#   1. Redistributions of source code must retain the above copyright notice,
#   this list of conditions and the following disclaimer.
#   2. Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
#   3. Neither the name of the copyright holder nor the names of its
#   contributors may be used to endorse or promote products derived from this
#   software without specific prior written permission. 
#   
#   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
#   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#   POSSIBILITY OF SUCH DAMAGE.

project(Animation VERSION 0.2)

add_library(${PROJECT_NAME} INTERFACE)
add_library(${CMAKE_PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_link_libraries(
    ${PROJECT_NAME}
        INTERFACE
          m)

target_include_directories(
    ${PROJECT_NAME}
        INTERFACE
          $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
          $<INSTALL_INTERFACE:include>)

install(
    TARGETS Animation
    EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    INCLUDES DESTINATION "include")

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY ExactVersion)

export(
    EXPORT ${PROJECT_NAME}Targets
    NAMESPACE
      ${CMAKE_PROJECT_NAME}::
    FILE
      "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Targets.cmake")

configure_file(
    "cmake/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake"
    COPYONLY)

# Ignore these for now on macOS
if(NOT MACOS)
    install(
        EXPORT ${PROJECT_NAME}Targets
        FILE
          "${PROJECT_NAME}Targets.cmake"
        NAMESPACE
          ${CMAKE_PROJECT_NAME}::
        DESTINATION
          "/usr/lib/cmake/${CMAKE_PROJECT_NAME}/${PROJECT_NAME}")

    install(
        FILES
          "cmake/${PROJECT_NAME}Config.cmake"
          "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION
          "/usr/lib/cmake/${CMAKE_PROJECT_NAME}/${PROJECT_NAME}")
endif()
