cmake_minimum_required(VERSION 3.22)

project(F1_Project)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

enable_language(C ASM)

# 导出编译命令到 compile_commands.json
# 这个文件可以被 clangd、vscode 等工具读取，提供代码补全和跳转功能
# set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)


# 创建一个可执行目标(固件文件)
add_executable(${CMAKE_PROJECT_NAME}
    main.c
)

# Add STM32CubeMX generated sources
# add_subdirectory(cmake/stm32cubemx)

# ----- 链接目录设置 -----
# 指定链接器搜索库文件的路径
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# ----- 添加源文件 -----
target_sources(${CMAKE_PROJECT_NAME} PRIVATE

)

# ----- 添加头文件路径 -----
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    Core/Bsp
    Core/Mid
    Core/App
    Core/Lib
)

# 添加预处理宏定义
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
# target_compile_definitions(stm32cubemx INTERFACE ${MX_Defines_Syms})
	USE_HAL_DRIVER 
	STM32F103xB
)

# ----- 解决链接问题 -----
# 当使用 C++ 代码时，移除错误的 libob.a 库依赖
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# ----- 链接库文件 -----
# 把 STM32CubeMX 生成的 HAL 库(目标名：stm32cubemx)链接到可执行文件
# 这个库包含了启动文件、HAL 驱动、CMSIS 核心文件等
target_link_libraries(${CMAKE_PROJECT_NAME}
    # Add user defined libraries
)
