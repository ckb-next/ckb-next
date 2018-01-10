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

# No need to check for WITH_ANIMATIONS, because all the options below
# already depend on it through cmake_dependent_option()

foreach (animation IN ITEMS GRADIENT HEAT PINWHEEL RAIN RANDOM RIPPLE WAVE)
  if (WITH_${animation})
    string(TOLOWER ${animation} animation)
    add_executable(${animation} ${animation}/main.c)

    target_link_libraries(
      ${animation}
        PRIVATE
          m
          ${CMAKE_PROJECT_NAME}::Animation)

    set_target_properties(
      ${animation}
        PROPERTIES
          C_STANDARD 11)

    target_compile_options(
      ${animation}
        PRIVATE
          "${CKB_NEXT_COMMON_COMPILE_FLAGS}")

    add_sanitizers(${animation})

    install(
      TARGETS ${animation}
      DESTINATION "${INSTALL_DIR_ANIMATIONS}")
  endif ()
endforeach ()

if (WITH_MVIZ)
  add_subdirectory(mviz)
endif ()

