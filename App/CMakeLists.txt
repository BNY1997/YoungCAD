project(
    YCADAPP              # 项目名称
    VERSION 1.0.0          # 版本号
    LANGUAGES CXX          # 项目语言(C/CXX)
)

set(APP_INCLUDE_FILES
    include/MainWindow.h
)

set(APP_SOURCE_FILES
    source/MainWindow.cpp
    source/main.cpp
    source/MainWindow.ui
)

add_executable(
    ${PROJECT_NAME}
    ${APP_SOURCE_FILES}
)
target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include  # 其他项目可以包含
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source   # 仅内部使用
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    osgd
    osgviewerd
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
            "${OSG_DIR}/bin"
            ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}
        )
    endif()

endif()





