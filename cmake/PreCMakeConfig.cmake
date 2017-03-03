# create a boolean for Linux
if (UNIX AND NOT APPLE)
    set(LINUX TRUE)
else ()
    set(LINUX FALSE)
endif ()

macro(makeLink src dest target)
    add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ln -sf ${src} ${dest} DEPENDS ${dest} COMMENT "mklink ${src} -> ${dest}")
endmacro()
