if (WITH_MVIZ) # otherwise doesn't make sense to include
    add_subdirectory(kissfft)
endif ()

# ckb-next libraries
add_subdirectory(ckbnext)
