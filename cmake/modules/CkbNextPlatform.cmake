# Internal booleans to use as platform checks

set(MACOS OFF CACHE INTERNAL "macOS host")
set(LINUX OFF CACHE INTERNAL "Linux host")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOS ON)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(LINUX ON)
else ()
    message(FATAL_ERROR "${CMAKE_SYSTEM_NAME} is not supported.")
endif ()
