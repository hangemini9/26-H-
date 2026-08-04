cmake_minimum_required(VERSION 3.22)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_PROJECT_NAME gimbal_MC02)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

project(${CMAKE_PROJECT_NAME} LANGUAGES C ASM)

option(GIMBAL_MANUAL_CALRUN
       "Enable the manual-only CALRUN commissioning command"
       ON)
option(GIMBAL_DELAY_TEST
       "Enable the dedicated, Ozone-gated total-loop-delay test"
       OFF)
option(GIMBAL_OZONE_WATCH
       "Keep DWARF type information for non-halting Ozone Watch"
       OFF)

add_executable(${CMAKE_PROJECT_NAME})
add_subdirectory(cmake/stm32cubemx)

if(GIMBAL_OZONE_WATCH)
    # Release uses -Os -g0 globally. Appending -g3 preserves the exact
    # optimization while restoring aggregate/member types for Ozone Watch.
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -g3)
endif()

if(GIMBAL_MANUAL_CALRUN)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
        GIMBAL_ENABLE_MANUAL_CALRUN=1)
else()
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
        GIMBAL_ENABLE_MANUAL_CALRUN=0)
endif()

if(GIMBAL_DELAY_TEST)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
        GIMBAL_ENABLE_DELAY_TEST=1)
else()
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
        GIMBAL_ENABLE_DELAY_TEST=0)
endif()

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    Core/Src/gimbal_app.c
    Core/Src/dm4310.c
    Core/Src/jetson_link.c
    Core/Src/ti_link.c
    Core/Src/usb_console.c
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    stm32cubemx
)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex
            $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
            ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary
            $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
            ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin
    COMMENT "Generating HEX and BIN images"
)
