project(ckb-daemon)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/*.c")


# find libraries
find_package(ICONV REQUIRED)

# linux only support right now
# bit of cheat here with the flags. It's not *right*
set(CMAKE_C_FLAGS "-lpthread -ludev")

#get the current ckb version
#execute_process(COMMAND cat ${CMAKE_SOURCE_DIR}/VERSION RESULT_VARIABLE CKB_VERSION_STR)
#add_definitions(-DCKB_VERSION_STR="\\\"${CKB_VERSION_STR}\\\"")

add_executable(${PROJECT_NAME} ${SOURCE} ${HEADERS})
