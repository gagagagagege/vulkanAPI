cmake_minimum_required(VERSION 3.20)

project(VulkanProject
    VERSION 1.0.0
    DESCRIPTION "Vulkan + GLFW + GLM project"
    LANGUAGES CXX
)

# =============================================================================
# C++ Standard
# =============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =============================================================================
# Vulkan SDK
#   默认指向本机安装位置;其他机器上用 -DVulkan_ROOT=<路径> 覆盖
#   (装了 SDK 的话 find_package 也能靠 VULKAN_SDK 环境变量自己找到)
# =============================================================================
if(NOT DEFINED Vulkan_ROOT)
    set(Vulkan_ROOT "D:/Vulkan_SDK" CACHE PATH "Vulkan SDK 安装目录")
endif()
find_package(Vulkan REQUIRED)

message(STATUS "Vulkan include: ${Vulkan_INCLUDE_DIRS}")
message(STATUS "Vulkan lib:     ${Vulkan_LIBRARIES}")
message(STATUS "Vulkan found:   ${Vulkan_FOUND}")

# =============================================================================
# GLM - OpenGL Mathematics (header-only math library)
# =============================================================================
set(GLM_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_LIBRARY  OFF CACHE BOOL "" FORCE)   # 只用 header-only 模式
add_subdirectory(glm)

# =============================================================================
# GLFW - Graphics Library Framework (窗口、输入、Vulkan surface)
# =============================================================================
set(GLFW_BUILD_DOCS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS   OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(GLFW)

# GLFW 编译 vulkan.c 需要 Vulkan 头文件，给它加上 include 路径
target_include_directories(glfw PRIVATE ${Vulkan_INCLUDE_DIRS})

# =============================================================================
# stb_image - 单头文件图像加载库 (header-only)
#   从 nothings/stb 下载,放在 Vulkan SDK 的 third_party 目录
#   实现(STB_IMAGE_IMPLEMENTATION)需在单个 .cpp 中定义,见 src/texture.cpp
# =============================================================================
set(STB_IMAGE_DIR "${Vulkan_ROOT}/third_party")

# =============================================================================
# 应用程序
# =============================================================================
file(GLOB_RECURSE APP_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

add_executable(${PROJECT_NAME} ${APP_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${STB_IMAGE_DIR}"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    glm::glm
    Vulkan::Vulkan
)

# =============================================================================
# Debug 模式 — 启用 Vulkan 验证层
#   使用生成器表达式，仅对 VulkanProject 目标生效，不污染第三方库。
#   支持单配置 (Ninja/Make) 和多配置 (Visual Studio/Xcode) 生成器。
# =============================================================================
target_compile_definitions(${PROJECT_NAME} PRIVATE
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
    VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
    NOMINMAX
    # 不定义这个,vk::raii 的 acquireNextImage / presentKHR 遇到
    # VK_ERROR_OUT_OF_DATE_KHR 会直接抛异常,drawFrame 里那两个 if 分支永远走不到。
    # 定义了才是"把错误码当返回值还给我"的行为。
    VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS=1
    $<$<CONFIG:Debug>:DEBUG>
)

# =============================================================================
# 着色器:构建后把编译好的 SPIR-V 复制到可执行文件输出目录
#   运行时按相对路径 "shaders/slang.spv" 加载,需保证工作目录为输出目录
#   (见下方 MSVC 的 VS_DEBUGGER_WORKING_DIRECTORY 设置)
# =============================================================================
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/slang.spv"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/slang.spv"
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/textures"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/src/textures/texture.jpg"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/textures/texture.jpg"
    COMMENT "Copying shader and texture to output directory"
)

# =============================================================================
# MSVC 优化设置
# =============================================================================
if(MSVC)
    # Visual Studio 文件夹结构
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    # 多核编译加速
    target_compile_options(${PROJECT_NAME} PRIVATE /MP)

    # 源文件和执行字符集均为 UTF-8（中文注释支持）
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)

    # 字符集: Unicode
    add_compile_definitions(UNICODE _UNICODE)

    # 调试时工作目录设为可执行文件输出目录,使 "shaders/slang.spv" 相对路径可解析
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
endif()
