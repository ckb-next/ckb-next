# Adopted from OpenChemistry projects.
#
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

    if (EXISTS ${git_command})
        execute_process(
                COMMAND ${git_command} describe
                WORKING_DIRECTORY ${source_dir}
                RESULT_VARIABLE result
                OUTPUT_VARIABLE output
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_STRIP_TRAILING_WHITESPACE)
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
        endif ()
    endif ()

    if (full)
        set(${var_prefix}_VERSION ${full} PARENT_SCOPE)
        set(${var_prefix}_VERSION_MAJOR ${major} PARENT_SCOPE)
        set(${var_prefix}_VERSION_MINOR ${minor} PARENT_SCOPE)
        set(${var_prefix}_VERSION_PATCH ${patch} PARENT_SCOPE)
        set(${var_prefix}_VERSION_PATCH_EXTRA ${patch_extra} PARENT_SCOPE)
        if ("${major}.${minor}.${patch}" STREQUAL "${full}")
            set(${var_prefix}_VERSION_IS_RELEASE TRUE PARENT_SCOPE)
        else ()
            set(${var_prefix}_VERSION_IS_RELEASE FALSE PARENT_SCOPE)
        endif ()
    else ()
        message(WARNING
                "\
Could not use git to determine source version, using version ${${var_prefix}_VERSION}. \
Consider cloning the project with git instead of downloading an archive. \
For example: git clone -b master --single-branch https://github.com/mattanger/ckb-next.git"
                )
    endif ()

endfunction()
