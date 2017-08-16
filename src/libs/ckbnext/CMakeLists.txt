project(CkbNextLibs VERSION 0.2)

add_library(Animation INTERFACE)
add_library(CkbNext::Animation ALIAS Animation)

target_link_libraries(
    Animation
        INTERFACE
          m)

target_include_directories(
    Animation
        INTERFACE
          $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
          $<INSTALL_INTERFACE:include>)

install(
    TARGETS Animation
    EXPORT AnimationTargets
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    INCLUDES DESTINATION "include")

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/Animation/AnimationConfigVersion.cmake"
    VERSION ${CkbNextLibs_VERSION}
    COMPATIBILITY ExactVersion)

export(
    EXPORT AnimationTargets
    NAMESPACE
      CkbNext::
    FILE
      "${CMAKE_CURRENT_BINARY_DIR}/Animation/AnimationTargets.cmake")

configure_file(
    "cmake/AnimationConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/Animation/AnimationConfig.cmake"
    COPYONLY)

install(
    EXPORT AnimationTargets
    FILE
      "AnimationTargets.cmake"
    NAMESPACE
      CkbNext::
    DESTINATION
      "/usr/lib/cmake/Animation")

install(
    FILES
      "cmake/AnimationConfig.cmake"
      "${CMAKE_CURRENT_BINARY_DIR}/Animation/AnimationConfigVersion.cmake"
    DESTINATION
      "/usr/lib/cmake/Animation")
