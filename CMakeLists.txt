cmake_minimum_required(VERSION 3.30)
project(TOTP)

add_subdirectory(external)

execute_process(COMMAND mkbcfnt -o ${CMAKE_CURRENT_BINARY_DIR}/firacode.bcfnt FiraCode-Regular.ttf WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

dkp_add_embedded_binary_library(firacode ${CMAKE_CURRENT_BINARY_DIR}/firacode.bcfnt)

file(GLOB TOTP_FILES source/**/*.cpp source/*.cpp)
add_executable(totp ${TOTP_FILES})
target_include_directories(totp PRIVATE include)
target_link_libraries(totp PRIVATE citro2d citro3d tinyhmac quirc firacode)


ctr_generate_smdh(totp.smdh NAME "TOTP" DESCRIPTION "Because why not" AUTHOR "Simon Hochrein")
ctr_create_3dsx(totp SMDH totp.smdh)