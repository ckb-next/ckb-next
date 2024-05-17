find_package(QT NAMES ${QT_LIST} COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

add_executable(ckb-next-sinfo sinfo.cpp)
target_link_libraries(ckb-next-sinfo PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

install(TARGETS ckb-next-sinfo DESTINATION "${INSTALL_DIR_UTILITIES}")
