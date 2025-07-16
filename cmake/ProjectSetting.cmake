
list(APPEND CMAKE_PREFIX_PATH "D:/Qt/Qt5.14.2/5.14.2/msvc2017_64/bin")

find_package(Qt5
COMPONENTS
    Core Widgets Gui Xml 
REQUIRED
)