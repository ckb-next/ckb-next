# - Try to find PulseAudioSimple
# Once done this will define
#
#  PULSEAUDIO_FOUND - system has pulseaudio simple
#  PULSEAUDIO_INCLUDE_DIRS - the pulseaudio simple include directory
#  PULSEAUDIO_LIBRARIES - Link these to use pulseaudio simple
#  PULSEAUDIOSIMPLE_FOUND - system has pulseaudio simple
#  PULSEAUDIOSIMPLE_INCLUDE_DIRS - the pulseaudio simple include directory
#  PULSEAUDIOSIMPLE_LIBRARIES - Link these to use pulseaudio simple
#
#  Copyright © 2014 Laszlo Papp <lpapp@kde.org>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

foreach (pulsevariablename PULSEAUDIOSIMPLE PULSEAUDIO)
    find_path(${pulsevariablename}_INCLUDE_DIR
        NAMES
          pulseaudio.h
          PATH_SUFFIXES pulse
        PATHS
          /usr/include
          /usr/local/include
          /opt/local/include
          /sw/include
    )

    set(pulselibname pulse)
    set(pulsepackagename PulseAudio)
    if (${pulsevariablename} STREQUAL "PULSEAUDIOSIMPLE")
        set(pulselibname pulse-simple)
        set(pulsepackagename PulseAudioSimple)
    endif()

    find_library(${pulsevariablename}_LIBRARY
        NAMES
        ${pulselibname}
        PATHS
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        /sw/lib
    )

    set(${pulsevariablename}_INCLUDE_DIRS
      ${${pulsevariablename}_DIR}
    )
    set(${pulsevariablename}_LIBRARIES
      ${${pulsevariablename}_LIBRARY}
    )

    include(FindPackageHandleStandardArgs)
    # handle the QUIETLY and REQUIRED arguments and set ${pulsevariablename}_FOUND to TRUE if
    # all listed variables are TRUE
    find_package_handle_standard_args(${pulsepackagename} DEFAULT_MSG ${pulsevariablename}_LIBRARY ${pulsevariablename}_INCLUDE_DIR)

    # show the ${pulsevariablename}_INCLUDE_DIRS and ${pulsevariablename}_LIBRARIES variables only in the advanced view
    mark_as_advanced(${pulsevariablename}_INCLUDE_DIRS ${pulsevariablename}_LIBRARIES)
endforeach(pulsevariablename)
