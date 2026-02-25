cmake_minimum_required(VERSION 3.30)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# remove last end of "/"
string(REGEX REPLACE "/$" "" CURRENT_FOLDER_ABSOLUTE ${CMAKE_CURRENT_SOURCE_DIR})

# get current relative dir name and set target name
string(REGEX REPLACE ".*/(.*)" "\\1" CURRENT_FOLDER ${CURRENT_FOLDER_ABSOLUTE})

project(SDK
  VERSION 1.0.0
  LANGUAGES CXX
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin)
if(CMAKE_CONFIGURATION_TYPES)
  foreach(config ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER "${config}" config_upper)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_upper} ${CMAKE_SOURCE_DIR}/build/bin/${CMAKE_BUILD_TYPE})
  endforeach()
endif()

list(APPEND CMAKE_MODULE_PATH
  ${CMAKE_SOURCE_DIR}/cmake
)

#for VS to use filter organize sources
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(BUILD_TESTING "Enable testing" ON)

# cmake构建过程中，要做以下的事情：
# 1. 项目中的一些选项变量的设置
# 2. 恢复nuget包
# 3. 设置Qt相关的环境
# 4. 设置oda相关的变量
# 5. 添加子文件夹

# 1. 工程中的一些选项
include(cmake/ProjectSettings.cmake)

set(OSG_VERSION 3.6.8)

find_package(Qt5
  COMPONENTS
    Core OpenGL PrintSupport Widgets Xml Concurrent Sql Qml
  REQUIRED
)
# TODO: remove since VS 2019
if (MSVC AND (MSVC_VERSION VERSION_LESS 1920) AND (Qt5_VERSION VERSION_GREATER_EQUAL 5.12))
  # https://bugreports.qt.io/browse/QTBUG-72073
  target_compile_definitions(Qt5::Core INTERFACE "QT_NO_FLOAT16_OPERATORS")
endif()

#find_package(LsLandxmlParser REQUIRED)

find_package(OSG REQUIRED)

#设置公共目录头文文件路径
# SET(COMMON_INCLUDE ${CMAKE_SOURCE_DIR}/Common/include)
# SET(TOOL_INCLUDE ${CMAKE_SOURCE_DIR}/BasePlugin/include)

# 6. 添加子文件夹
add_subdirectory(Platform)                    # SiteOffice最底层的基础设施工具模块
add_subdirectory(Common)
