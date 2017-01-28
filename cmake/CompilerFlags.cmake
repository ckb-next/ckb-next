if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    option(ENABLE_COMPILER_WARNINGS "Enable compiler warnings in Debug mode" ON)
endif ()

#-------------------------------------------------------------------------------
# flags for C
set(CMAKE_C_STANDARD 11)

if (ENABLE_COMPILER_WARNINGS)
    if (CMAKE_C_COMPILER_ID MATCHES "Clang")
        # using regular Clang or AppleClang
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Weverything")
    else ()
        # using GNU
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Wextra")
    endif ()
else ()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter")
endif ()

#-------------------------------------------------------------------------------
# flags for C++
set(CMAKE_CXX_STANDARD 14)

if (ENABLE_COMPILER_WARNINGS)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # using regular Clang or AppleClang
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Weverything")
    else ()
        # using GNU
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra")
    endif ()
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter")
endif ()
