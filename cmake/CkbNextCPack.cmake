set(CPACK_PACKAGE_NAME "ckb-next")
set(CPACK_PACKAGE_VERSION_MAJOR ${ckb-next_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${ckb-next_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${ckb-next_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${ckb-next_VERSION})
set(CPACK_PACKAGE_VENDOR "https://github.com/mattanger/ckb-next/")  # replace with ckb-next.org when it's ready
set(CPACK_PACKAGE_DESCRIPTION
        "Corsair RGB driver for Linux and Mac.")
set(CPACK_VERBATIM_VARIABLES TRUE)

if (APPLE)
    set(CPACK_SYSTEM_NAME "macOS")
    configure_file("${ckb-next_SOURCE_DIR}/LICENSE" "${CMAKE_BINARY_DIR}/LICENSE.txt" @ONLY)
    configure_file("${ckb-next_SOURCE_DIR}/osx/Welcome.txt" "${CMAKE_BINARY_DIR}/osx/Welcome.txt" @ONLY)
    configure_file("${ckb-next_SOURCE_DIR}/README.md" "${CMAKE_BINARY_DIR}/README.txt" @ONLY)
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_BINARY_DIR}/LICENSE.txt")
    set(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_BINARY_DIR}/osx/Welcome.txt")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_BINARY_DIR}/README.txt")
    set(CPACK_PACKAGE_ICON "${CMAKE_BINARY_DIR}/ckb-next.icns")
endif ()

if (LINUX)
    configure_file("${ckb-next_SOURCE_DIR}/LICENSE" "${CMAKE_BINARY_DIR}/LICENSE" @ONLY)
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_BINARY_DIR}/LICENSE")
endif ()

configure_file("${CMAKE_CURRENT_LIST_DIR}/CKBNextCPackOptions.cmake.in" "${ckb-next_BINARY_DIR}/CKBNextCPackOptions.cmake" @ONLY)
set(CPACK_PROJECT_CONFIG_FILE "${ckb-next_BINARY_DIR}/CKBNextCPackOptions.cmake")

include(CPack)
