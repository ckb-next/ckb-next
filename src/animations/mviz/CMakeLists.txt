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

if (NOT USE_PORTAUDIO)
    find_package(PulseAudio)
    if (NOT ${PULSEAUDIO_FOUND})
        message(WARNING "Missing PulseAudio API. Not building mviz.")
        return()
    endif ()
endif ()

add_executable(mviz main.c)

if (USE_PORTAUDIO)
    set_target_properties(mviz PROPERTIES COMPILE_DEFINITIONS "USE_PORTAUDIO=1")

    if (LINUX)
        set(AUDIO_LIBRARIES portaudio)
        set(PORTAUDIO_INCLUDE_DIRS "/usr/include/")
    elseif (MACOS)
        find_library(CORESERVICES_LIBRARY CoreServices)
        find_library(COREFOUNDATION_LIBRARY CoreFoundation)
        find_library(COREAUDIO_LIBRARY CoreAudio)
        find_library(AUDIOUNIT_LIBRARY AudioUnit)
        find_library(AUDIOTOOLBOX_LIBRARY AudioToolbox)
        set(PORTAUDIO_INCLUDE_DIRS "/usr/local/opt/portaudio/include/")
    else ()
        message(WARNING "Include path for PortAudio has not been specified for this OS.")
    endif ()

    target_include_directories(
        mviz
            PRIVATE
              "${PORTAUDIO_INCLUDE_DIRS}")

else ()
    target_include_directories(
        mviz
            PRIVATE
              "${PULSEAUDIO_INCLUDE_DIRS}")

    set(AUDIO_LIBRARIES "${PULSEAUDIO_LIBRARIES}")
endif ()

if (MACOS)
    target_link_libraries(
        mviz
            PRIVATE
              m
              KissFFT::KissFFT
              ${CMAKE_PROJECT_NAME}::Animation
              ${CORESERVICES_LIBRARY}
              ${COREFOUNDATION_LIBRARY}
              ${AUDIOUNIT_LIBRARY}
              ${AUDIOTOOLBOX_LIBRARY}
              ${COREAUDIO_LIBRARY}
              "/usr/local/opt/portaudio/lib/libportaudio.a")
elseif (LINUX)
        target_link_libraries(
        mviz
            PRIVATE
              m
              KissFFT::KissFFT
              ${CMAKE_PROJECT_NAME}::Animation
              "${AUDIO_LIBRARIES}")
else()
    message(WARNING "PortAudio link library has not been specified for this OS.")
endif()

set_target_properties(
    mviz
        PROPERTIES
          C_STANDARD 11)

target_compile_options(
    mviz
      PRIVATE
        "${CKB_NEXT_COMMON_COMPILE_FLAGS}"
        "${CKB_NEXT_EXTRA_C_FLAGS}")

add_sanitizers(mviz)

install(
    TARGETS mviz
    DESTINATION ${INSTALL_DIR_ANIMATIONS})
