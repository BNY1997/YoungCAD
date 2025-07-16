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

# 添加项目子目录
add_subdirectory(AppFrame)       

# 安装配置
include(cmake/Config.cmake)