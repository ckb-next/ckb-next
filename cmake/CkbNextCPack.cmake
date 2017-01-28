##set(CPACK_PACKAGE_NAME "CKB Next")
#set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
#set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
#set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
##set(CPACK_PACKAGE_INSTALL_DIRECTORY "ckb-next")
#set(CPACK_PACKAGE_VENDOR "CKB Next development team")
#set(CPACK_PACKAGE_DESCRIPTION
#        "Corsair products RGB driver for Linux and macOS")
#
#if(APPLE)
#    configure_file("${${PROJECT_NAME}_SOURCE_DIR}/osx/resources/LICENSE.html"
#            "${${PROJECT_NAME}_BINARY_DIR}/LICENSE.html" @ONLY)
#    set(CPACK_RESOURCE_FILE_LICENSE "${${PROJECT_NAME}_BINARY_DIR}/LICENSE.html")
#    set(CPACK_PACKAGE_ICON
#            "${${PROJECT_NAME}_SOURCE_DIR}/src/ckb/ckb-logo.icns")
#    set(CPACK_BUNDLE_ICON "${CPACK_PACKAGE_ICON}")
#endif()
#
#set(CPACK_PACKAGE_EXECUTABLES "${PROJECT_NAME}" "CKB Next")
#set(CPACK_CREATE_DESKTOP_LINKS "${PROJECT_NAME}")
#
#configure_file("${CMAKE_CURRENT_LIST_DIR}/CkbNextCPackOptions.cmake.in"
#        "${${PROJECT_NAME}_BINARY_DIR}/CkbNextCPackOptions.cmake" @ONLY)
#set(CPACK_PROJECT_CONFIG_FILE
#        "${${PROJECT_NAME}_BINARY_DIR}/CkbNextCPackOptions.cmake")
#
## Should we add extra install rules to make a self-contained bundle, this is
## usually only required when attempting to create self-contained installers.
#option(INSTALL_BUNDLE_FILES "Add install rules to bundle files" OFF)
##if(INSTALL_BUNDLE_FILES)
##endif()

#set(CPACK_GENERATOR "DragNDrop")
set(CPACK_GENERATOR "Bundle")
set(CPACK_PACKAGE_NAME "ckb-next")
set(CPACK_PACKAGE_VERSION_MAJOR ${ckb-next_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${ckb-next_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${ckb-next_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION "${ckb-next_VERSION_MAJOR}.${ckb-next_VERSION_MINOR}.${ckb-next_VERSION_PATCH}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Corsair products RGB driver for Linux and macOS")
set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/ckb-logo.icns")
set(CPACK_PACKAGE_EXECUTABLES "ckb-next" "CKB Next")
set(CPACK_VERBATIM_VARIABLES TRUE)
set(CPACK_SYSTEM_NAME "macOS")
