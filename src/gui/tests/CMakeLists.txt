find_package(Qt5Test QUIET)
if(NOT ${Qt5Test_FOUND})
    message(STATUS "Not creating test target because Qt5Test is not found")
    return()
endif()
add_custom_target(build_tests)

add_executable(TestQImageICC
    EXCLUDE_FROM_ALL
    TestQImageICC.cpp
    TestQImageICC.h
    "${CMAKE_CURRENT_BINARY_DIR}/../image.qrc")

target_link_libraries(TestQImageICC PRIVATE Qt5::Test Qt5::Widgets)

set_target_properties(
    TestQImageICC
        PROPERTIES
          AUTOMOC ON
          AUTOUIC ON
          AUTORCC ON)

add_test(NAME TestQImageICC COMMAND TestQImageICC)
add_dependencies(build_tests TestQImageICC)
