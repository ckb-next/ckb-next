#   Copyright (c) 2014 Laszlo Papp <lpapp@kde.org>
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
#   
#   Once done this will define:
#       PULSEAUDIO_FOUND - system has pulseaudio
#       PULSEAUDIO_INCLUDE_DIRS - the pulseaudio include directory
#       PULSEAUDIO_LIBRARIES - Link these to use pulseaudio

find_path(PULSEAUDIO_INCLUDE_DIR
        NAMES
        pulseaudio.h
        PATH_SUFFIXES pulse
        PATHS
        /usr/include
        /usr/local/include
        /opt/local/include
        /sw/include
        )

find_library(PULSEAUDIO_LIBRARY
        NAMES
        pulse
        PATHS
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        /sw/lib
        )

set(PULSEAUDIO_INCLUDE_DIRS
        ${PULSEAUDIO_DIR}
        )
set(PULSEAUDIO_LIBRARIES
        ${PULSEAUDIO_LIBRARY}
        )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set PULSEAUDIO_FOUND to TRUE if
# all listed variables are TRUE
find_package_handle_standard_args(PulseAudio DEFAULT_MSG PULSEAUDIO_LIBRARY PULSEAUDIO_INCLUDE_DIR)

# show the PULSEAUDIO_INCLUDE_DIRS and PULSEAUDIO_LIBRARIES variables only in the advanced view
mark_as_advanced(PULSEAUDIO_INCLUDE_DIRS PULSEAUDIO_LIBRARIES)
