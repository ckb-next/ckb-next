if (WITH_MVIZ) # otherwise doesn't make sense to include
    add_subdirectory(kissfft)
endif ()

if (WITH_SHIPPED_QUAZIP)
    add_subdirectory(quazip)
endif ()

# ckb-next libraries
add_subdirectory(ckbnext)
