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

# Compiler-related stuff specific to ckb-next

# CMAKE_CXX_FLAGS_DEBUG is -g
# CMAKE_CXX_FLAGS_RELEASE is -O3 -DNDEBUG
# CMAKE_CXX_FLAGS_RELWITHDEBINFO is -O2 -g -DNDEBUG
# CMAKE_CXX_FLAGS_MINSIZEREL is -Os -DNDEBUG

# Check for -Og for better debugging

# GCC supports it since 4.8.0
# clang supports it since 4.0.0
# Apple's Clang supports it since 9.0.0

include(CheckCCompilerFlag)
check_c_compiler_flag("-Og" C_COMPILER_SUPPORTS_-Og)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-Og" CXX_COMPILER_SUPPORTS_-Og)

if (C_COMPILER_SUPPORTS_-Og)
    set(opt_lvl "-Og")
else ()
    set(opt_lvl "-O0")
endif ()

set(CKB_NEXT_COMMON_COMPILE_FLAGS "")
list(APPEND CKB_NEXT_COMMON_COMPILE_FLAGS
        -fsigned-char
        -Wall
        -Wextra
        -Wcast-align
        -Winit-self
        -Wno-unused-parameter
        $<$<CONFIG:Debug>:${opt_lvl}>
        #    $<$<CONFIG:Debug>:-Wfloat-equal>
        #    $<$<CONFIG:Debug>:-Wundef>
        #    $<$<CONFIG:Debug>:-Wshadow>
        #    $<$<CONFIG:Debug>:-Wpointer-arith>
        #    $<$<CONFIG:Debug>:-Wcast-align>
        #    $<$<CONFIG:Debug>:-Wstrict-prototypes>
        #    $<$<CONFIG:Debug>:-Wstrict-overflow=5>
        #    $<$<CONFIG:Debug>:-Wwrite-strings>
        #    $<$<CONFIG:Debug>:-Wcast-qual>
        #    $<$<CONFIG:Debug>:-Wswitch-default>
        #    $<$<CONFIG:Debug>:-Wswitch-enum>
        #    $<$<CONFIG:Debug>:-Wconversion>
        #    $<$<CONFIG:Debug>:-Wformat=2>
        #    $<$<CONFIG:Debug>:-save-temps>
     )
