cmake_minimum_required(VERSION 3.22)

project(F1_Project)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

enable_language(C ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

add_executable(${CMAKE_PROJECT_NAME})

add_subdirectory(drivers)

# 指定链接器搜索库文件的路径
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    startup_stm32f103xb.s
    core/src/system_stm32f1xx.c
    core/src/stm32f1xx_hal_msp.c
    core/src/stm32f1xx_it.c
    core/src/main.c
    core/bsp/led.c
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    core/inc
    core/src
    core/app
    core/mid
    core/bsp
)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
)

target_link_libraries(${CMAKE_PROJECT_NAME}
    f1_drivers
)

# 生成 .hex 和 .bin 文件
add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.bin
)