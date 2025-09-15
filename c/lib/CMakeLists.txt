cmake_minimum_required(VERSION 3.10)

project(kea LANGUAGES C)

add_library(kea STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/kea.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/commands/kea_cmd_processor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/stream/kea_stream_helpers.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/kea_model_mgr.c)

target_include_directories(kea PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_include_directories(kea PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)