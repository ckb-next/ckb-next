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

# CMAKE_CXX_FLAGS_DEBUG is -g -O0
# CMAKE_CXX_FLAGS_RELEASE is -O3 -DNDEBUG
# CMAKE_CXX_FLAGS_RELWITHDEBINFO is now -O3 -g -DNDEBUG
# CMAKE_CXX_FLAGS_MINSIZEREL is -Os -DNDEBUG

set(opt_lvl "-O0")
set(CKB_NEXT_COMMON_COMPILE_FLAGS "")
list(APPEND CKB_NEXT_COMMON_COMPILE_FLAGS
        -fsigned-char
        -Wall
        -Wextra
        -Wcast-align
        -Winit-self
        -Wno-unused-parameter
        -Werror=return-type
        -Wshadow
        -Wno-cpp
        -Wno-unused-function
        $<$<CONFIG:Debug>:${opt_lvl}>
        $<$<CONFIG:Debug>:-Wvla>
        $<$<CONFIG:Debug>:-Wcpp>
        $<$<CONFIG:Debug>:-Wunused-function>
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
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "8.0")
    list(APPEND CKB_NEXT_COMMON_COMPILE_FLAGS
        -Wcast-align=strict
    )
endif()

set(CKB_NEXT_EXTRA_C_FLAGS "")
list(APPEND CKB_NEXT_EXTRA_C_FLAGS
        -Werror=incompatible-pointer-types
    )

set(CKB_NEXT_EXTRA_CXX_FLAGS "")
list(APPEND CKB_NEXT_EXTRA_CXX_FLAGS
        -Wzero-as-null-pointer-constant
    )

string(REPLACE "-O2" "-O3" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
string(REPLACE "-O2" "-O3" CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")

if(WERROR)
    list(APPEND CKB_NEXT_EXTRA_C_FLAGS
        -Werror
        -Wno-error=cpp
        -Wno-error=cast-align
    )
    list(APPEND CKB_NEXT_EXTRA_CXX_FLAGS
        -Werror
        -Wno-error=cpp
        -Wno-error=deprecated-declarations
    )
endif()
