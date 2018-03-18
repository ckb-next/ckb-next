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

#.rst:
#     FindAppindicator
#     -----------
#    
#     Find any available appindicator (gtk2, gtk3, sharp).
#    
#     Result variables
#     ^^^^^^^^^^^^^^^^
#    
#     This module will set the following variables in your project:
#    
#     ``Appindicator_FOUND``
#       true if appindicator headers and libraries were found
#     ``Appindicator_INCLUDE_DIRS``
#       list of the include directories needed to use appindicator
#     ``Appindicator_LIBRARIES``
#       appindicator libraries to be linked
#     ``Appindicator_DEFINITIONS``
#       the compiler switches required for using appindicator
#     ``Appindicator_VERSION``
#       the version of appindicator found

find_package(PkgConfig REQUIRED)
pkg_search_module(Appindicator appindicator-0.1)
set(Appindicator_DEFINITIONS ${Appindicator_CFLAGS_OTHER})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    Appindicator
        REQUIRED_VARS
          Appindicator_LIBRARIES
          Appindicator_INCLUDE_DIRS
          Appindicator_DEFINITIONS
        VERSION_VAR
          Appindicator_VERSION)

mark_as_advanced(Appindicator_LIBRARIES Appindicator_DEFINITIONS Appindicator_INCLUDE_DIRS Appindicator_VERSION)

if(NOT Appindicator_FOUND)
    message(WARNING "Appindicator was not found.\n"
        "If you encounter issues with the tray icon, please install it.")
endif()
