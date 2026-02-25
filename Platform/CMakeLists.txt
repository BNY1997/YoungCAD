
cmake_minimum_required(VERSION 3.30)

project(PlatformApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source and include directories
set(PLATFORM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(PLATFORM_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/source)

file(GLOB_RECURSE PLATFORM_SOURCES
	"${PLATFORM_SOURCE_DIR}/*.cpp"
	"${PLATFORM_SOURCE_DIR}/*.cxx"
	"${PLATFORM_SOURCE_DIR}/*.c"
)

file(GLOB_RECURSE PLATFORM_HEADERS
	"${PLATFORM_INCLUDE_DIR}/*.h"
	"${PLATFORM_INCLUDE_DIR}/*.hpp"
)

if(NOT PLATFORM_SOURCES)
	message(FATAL_ERROR "No source files found in ${PLATFORM_SOURCE_DIR}")
endif()

# Create executable
add_executable(${PROJECT_NAME} ${PLATFORM_SOURCES} ${PLATFORM_HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES
  AUTOUIC ON
  AUTOMOC ON
  AUTORCC ON
)


target_include_directories(${PROJECT_NAME} PRIVATE ${PLATFORM_INCLUDE_DIR})

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

if(WIN32 AND Qt5_DIR)
    # 获取 windeployqt 路径
    get_target_property(qt_core_location Qt5::Core LOCATION)
    get_filename_component(QT_BIN_DIR ${qt_core_location} DIRECTORY)
    set(WINDEPLOYQT "${QT_BIN_DIR}/windeployqt.exe")
    
    if(EXISTS ${WINDEPLOYQT})
        add_custom_target(deploy-qt
            COMMAND ${CMAKE_COMMAND} -E echo "Starting Qt deployment..."
            
            # 运行 windeployqt
            COMMAND ${WINDEPLOYQT}
                $<TARGET_FILE:${PROJECT_NAME}>
            
            COMMAND ${CMAKE_COMMAND} -E echo "Qt deployment completed!"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS ${PROJECT_NAME}

            COMMENT "Deploying Qt dependencies"

            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${OSG_LIB_PATH}/${CMAKE_BUILD_TYPE}"
            ${CMAKE_SOURCE_DIR}/build/bin/${CMAKE_BUILD_TYPE}
        )
    endif()

endif()



