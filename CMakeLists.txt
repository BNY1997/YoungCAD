cmake_minimum_required(VERSION 3.12)  # 最低CMake版本
project(
    YoungCAD              # 项目名称
    VERSION 1.0.0          # 版本号
    LANGUAGES CXX          # 项目语言(C/CXX)
)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 配置构建类型（默认为Release）
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

add_definitions(-D_OPENTHREADS_ATOMIC_USE_MUTEX)

# 按配置类型设置（Debug/Release）
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib/Debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib/Debug)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib/Release)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib/Release)

include(cmake/ProjectSetting.cmake)

include(cmake/FindOSG.cmake)

# 添加项目子目录
add_subdirectory(App)  
add_subdirectory(AppFrame)     

