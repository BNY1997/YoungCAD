
list(APPEND CMAKE_PREFIX_PATH "D:/Qt/Qt5.14.2/5.14.2/msvc2017_64/bin")
list(APPEND CMAKE_PREFIX_PATH "D:/YoungCAD/3rdParty/OSGD/cmake")
list(APPEND CMAKE_PREFIX_PATH "D:/YoungCAD/3rdParty/OSGD/bin")


find_package(Qt5
COMPONENTS
    Core Widgets Gui Xml 
REQUIRED
)

set(OSG_DIR "D:/YoungCAD/3rdParty/OSGD")
include_directories(${OSG_DIR}/include)
link_directories(${OSG_DIR}/lib)