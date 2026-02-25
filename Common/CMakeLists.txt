
cmake_minimum_required(VERSION 3.30)

project(Common LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source and include directories
set(COMMON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(COMMON_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/source)

file(GLOB_RECURSE COMMON_SOURCES
	"${COMMON_SOURCE_DIR}/*.cpp"
	"${COMMON_SOURCE_DIR}/*.cxx"
	"${COMMON_SOURCE_DIR}/*.c"
	"${COMMON_SOURCE_DIR}/*.ui"
)

file(GLOB_RECURSE COMMON_HEADERS
	"${COMMON_INCLUDE_DIR}/*.h"
	"${COMMON_INCLUDE_DIR}/*.hpp"
)


# Create shared library (DLL)
add_library(${PROJECT_NAME} SHARED ${COMMON_SOURCES} ${COMMON_HEADERS})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
# 创建导出头文件
include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME}
    EXPORT_FILE_NAME "${COMMON_INCLUDE_DIR}/${PROJECT_NAME}_Export.h"
    EXPORT_MACRO_NAME ${PROJECT_NAME}_EXPORT
    STATIC_DEFINE ${PROJECT_NAME}_STATIC_DEFINE
)

# 添加导出宏到编译定义
target_compile_definitions(${PROJECT_NAME} PRIVATE
    ${PROJECT_NAME}_EXPORTS
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  AUTOUIC ON
  AUTOMOC ON
  AUTORCC ON
)

target_include_directories(${PROJECT_NAME} PRIVATE ${COMMON_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE 
    Qt5::Core 
    Qt5::OpenGL 
    Qt5::PrintSupport 
    Qt5::Widgets 
    Qt5::Xml 
    Qt5::Concurrent 
    Qt5::Sql 
    OSG::OSG 
    OSG::VIEWER
    OSG::DB 
    OSG::UTIL
    OSG::GA
)

target_sources(${PROJECT_NAME}
  PRIVATE
    ${COMMON_SOURCES}
  PUBLIC FILE_SET HEADERS
    BASE_DIRS "${PROJECT_BINARY_DIR};${PROJECT_SOURCE_DIR}/include"
    FILES
    ${COMMON_HEADERS}
)


