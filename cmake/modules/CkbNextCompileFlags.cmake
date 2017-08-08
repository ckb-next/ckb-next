# Compiler-related stuff specific to ckb-next

# CMAKE_CXX_FLAGS_DEBUG is -g
# CMAKE_CXX_FLAGS_RELEASE is -O3 -DNDEBUG
# CMAKE_CXX_FLAGS_RELWITHDEBINFO is -O2 -g -DNDEBUG
# CMAKE_CXX_FLAGS_MINSIZEREL is -Os -DNDEBUG

# Check for -Og for better debugging

# gcc supports it since 4.8.0
# clang supports it since 4.0.0
# apple clang supports it since 9.0.0

include(CheckCCompilerFlag)
check_c_compiler_flag("-Og" C_COMPILER_SUPPORTS_-Og)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-Og" CXX_COMPILER_SUPPORTS_-Og)
