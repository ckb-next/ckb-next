#   Copyright (c) 2017 Oleksii Vilchanskyi <alexvilchansky@yahoo.com>
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
if (WITH_GRADIENT)
    add_subdirectory(gradient)
endif ()
if (WITH_HEAT)
    add_subdirectory(heat)
endif ()
if (WITH_PINWHEEL)
    add_subdirectory(pinwheel)
endif ()
if (WITH_RAIN)
    add_subdirectory(rain)
endif ()
if (WITH_RANDOM)
    add_subdirectory(random)
endif ()
if (WITH_RIPPLE)
    add_subdirectory(ripple)
endif ()
if (WITH_WAVE)
    add_subdirectory(wave)
endif ()
if (WITH_MVIZ)
    add_subdirectory(mviz)
endif ()

