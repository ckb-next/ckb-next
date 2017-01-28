# create a boolean for Linux
if (UNIX AND NOT APPLE)
    set(LINUX TRUE)
else ()
    set(LINUX FALSE)
endif ()
