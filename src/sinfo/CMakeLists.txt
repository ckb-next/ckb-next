find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(ckb-next-sinfo sinfo.cpp)
target_link_libraries(ckb-next-sinfo PRIVATE Qt5::Widgets)

install(TARGETS ckb-next-sinfo DESTINATION "${INSTALL_DIR_UTILITIES}")
