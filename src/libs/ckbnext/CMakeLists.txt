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

project(CkbNextLibs VERSION 0.2)

add_library(Animation INTERFACE)
add_library(CkbNext::Animation ALIAS Animation)

target_link_libraries(
    Animation
        INTERFACE
          m)

target_include_directories(
    Animation
        INTERFACE
          $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
          $<INSTALL_INTERFACE:include>)

install(
    TARGETS Animation
    EXPORT AnimationTargets
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    INCLUDES DESTINATION "include")

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/Animation/AnimationConfigVersion.cmake"
    VERSION ${CkbNextLibs_VERSION}
    COMPATIBILITY ExactVersion)

export(
    EXPORT AnimationTargets
    NAMESPACE
      CkbNext::
    FILE
      "${CMAKE_CURRENT_BINARY_DIR}/Animation/AnimationTargets.cmake")

configure_file(
    "cmake/AnimationConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/Animation/AnimationConfig.cmake"
    COPYONLY)

install(
    EXPORT AnimationTargets
    FILE
      "AnimationTargets.cmake"
    NAMESPACE
      CkbNext::
    DESTINATION
      "/usr/lib/cmake/Animation")

install(
    FILES
      "cmake/AnimationConfig.cmake"
      "${CMAKE_CURRENT_BINARY_DIR}/Animation/AnimationConfigVersion.cmake"
    DESTINATION
      "/usr/lib/cmake/Animation")
