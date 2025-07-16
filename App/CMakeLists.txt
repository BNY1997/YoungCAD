project(
    APP              # 项目名称
    VERSION 1.0.0          # 版本号
    LANGUAGES CXX          # 项目语言(C/CXX)
)

set(APP_INCLUDE_FILES

)

set(APP_SOURCE_FILES
    source/main.cpp
)

add_executable(
    ${PROJECT_NAME}
    ${APP_SOURCE_FILES}
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
)



