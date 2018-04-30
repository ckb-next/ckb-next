#   Copyright 2015 Marcus D. Hanwell <marcus.hanwell@kitware.com>
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

# Used to determine the version for ckb-next source using "git describe", if git
# is found. On success sets following variables in caller's scope:
#   ${var_prefix}_VERSION
#   ${var_prefix}_VERSION_MAJOR
#   ${var_prefix}_VERSION_MINOR
#   ${var_prefix}_VERSION_PATCH
#   ${var_prefix}_VERSION_PATCH_EXTRA
#   ${var_prefix}_VERSION_IS_RELEASE if patch-extra is empty.
#
# If git is not found, or git describe cannot be run successfully, then these
# variables are left unchanged and a warning message is printed.
#
# Arguments are:
#   source_dir : Source directory
#   git_command : git executable
#   var_prefix : project name

function(determine_version source_dir git_command var_prefix)
    set(major)
    set(minor)
    set(patch)
    set(full)
    set(patch_extra)

    if (NOT ${${var_prefix}_VERSION_IS_RELEASE})
        if (EXISTS ${git_command})
            execute_process(
                    COMMAND ${git_command} describe --tags
                    WORKING_DIRECTORY ${source_dir}
                    RESULT_VARIABLE result
                    OUTPUT_VARIABLE output
                    ERROR_QUIET
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
            if (${result} EQUAL 0)
                string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)[-]*(.*)"
                        version_matches ${output})
                if (CMAKE_MATCH_0)
                    # message(STATUS "Determined Git Version: ${CMAKE_MATCH_0}")
                    set(full ${CMAKE_MATCH_0})
                    set(major ${CMAKE_MATCH_1})
                    set(minor ${CMAKE_MATCH_2})
                    set(patch ${CMAKE_MATCH_3})
                    set(patch_extra ${CMAKE_MATCH_4})
                endif ()
            else ()
                message("\
    Git directory to deduce precise version has not been not found.\n\
    Consider cloning the project with git instead of downloading an archive.\n\
    For example:\n\
        git clone -b master --single-branch https://github.com/ckb-next/ckb-next.git")
            endif ()
        else ()
            message("Git executable not found.")
        endif ()

        if (full)
            set(${var_prefix}_VERSION ${full} PARENT_SCOPE)
            set(${var_prefix}_VERSION_MAJOR ${major} PARENT_SCOPE)
            set(${var_prefix}_VERSION_MINOR ${minor} PARENT_SCOPE)
            set(${var_prefix}_VERSION_PATCH ${patch} PARENT_SCOPE)
            set(${var_prefix}_VERSION_PATCH_EXTRA ${patch_extra} PARENT_SCOPE)

            if ("${major}.${minor}.${patch}" STREQUAL "${full}")
                set(${var_prefix}_VERSION_IS_RELEASE TRUE PARENT_SCOPE)
                message(STATUS "${var_prefix} version: ${full} (Release)")
            else ()
                set(${var_prefix}_VERSION_IS_RELEASE FALSE PARENT_SCOPE)
                message(STATUS "${var_prefix} version: ${full} (Non-release)")
            endif ()

        else()
            set(${var_prefix}_VERSION ${full} "${${var_prefix}_VERSION}-unknown" PARENT_SCOPE)
            set(${var_prefix}_VERSION_IS_RELEASE FALSE PARENT_SCOPE)
            message(STATUS "${var_prefix} version: ${${var_prefix}_VERSION}-unknown (Non-release)")
        endif ()

    endif()

endfunction()
