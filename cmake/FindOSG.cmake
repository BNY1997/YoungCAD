if(NOT WIN32)
  message(FATAL_ERROR "Unsupported platform")
endif()

if(TARGET OSG::OSG)
  set(OSG_FOUND TRUE)
  return()
endif()

set(OSG_VERSION 3.6.8)

set(OSG_ROOT ${CMAKE_SOURCE_DIR}/3rdParty/OSGD)

set(OSG_INCLUDE_DIR "${OSG_ROOT}/build/native/include")
set(OSG_PLUGIN_VERSION 3.6.2)

set(OSG_LIB_PATH "${OSG_ROOT}/lib/native/x64")

set(OSG_LIBRARY_DEBUG "${OSG_ROOT}/lib/native/x64/debug/osg157-osgd.dll")
set(OSG_IMPLIB_DEBUG "${OSG_ROOT}/lib/native/x64/debug/osgd.lib")
set(OSG_DB_LIBRARY_DEBUG "${OSG_ROOT}/lib/native/x64/debug/osg157-osgDBd.dll")
set(OSG_DB_IMPLIB_DEBUG "${OSG_ROOT}/lib/native/x64/debug/osgDBd.lib")
set(OSG_UTIL_LIBRARY_DEBUG "${OSG_ROOT}/lib/native/x64/debug/osg157-osgUtild.dll")
set(OSG_UTIL_IMPLIB_DEBUG "${OSG_ROOT}/lib/native/x64/debug/osgUtild.lib")
set(OSG_THREADS_LIBRARY_DEBUG "${OSG_ROOT}/lib/native/x64/debug/ot21-OpenThreadsd.dll")
set(OSG_THREADS_IMPLIB_DEBUG "${OSG_ROOT}/lib/native/x64/debug/OpenThreadsd.lib")
set(OSG_VIEWER_LIBRARY_DEBUG "${OSG_ROOT}/lib/native/x64/debug/osg157-osgViewerd.dll")
set(OSG_VIEWER_IMPLIB_DEBUG "${OSG_ROOT}/lib/native/x64/debug/osgViewerd.lib")
set(OSG_GA_LIBRARY_DEBUG "${OSG_ROOT}/lib/native/x64/debug/osg157-osgGAd.dll")
set(OSG_GA_IMPLIB_DEBUG "${OSG_ROOT}/lib/native/x64/debug/osgGAd.lib")
set(OSGDB_OSGD_LIBRARY_DEBUG
  "${OSG_ROOT}/lib/native/x64/debug/osgPlugins-${OSG_PLUGIN_VERSION}/osgdb_osgd.dll"
  "${OSG_ROOT}/lib/native/x64/debug/osgPlugins-${OSG_PLUGIN_VERSION}/osgdb_serializers_osgd.dll"
  "${OSG_ROOT}/lib/native/x64/debug/osgPlugins-${OSG_PLUGIN_VERSION}/osgdb_ived.dll"
)
set(OSGDB_JPEG_LIBRARY_DEBUG
  "${OSG_ROOT}/lib/native/x64/debug/osgPlugins-${OSG_PLUGIN_VERSION}/osgdb_jpegd.dll"
)

set(OSG_LIBRARY "${OSG_ROOT}/lib/native/x64/release/osg157-osg.dll")
set(OSG_IMPLIB "${OSG_ROOT}/lib/native/x64/release/osg.lib")
set(OSG_DB_LIBRARY "${OSG_ROOT}/lib/native/x64/release/osg157-osgDB.dll")
set(OSG_DB_IMPLIB "${OSG_ROOT}/lib/native/x64/release/osgDB.lib")
set(OSG_UTIL_LIBRARY "${OSG_ROOT}/lib/native/x64/release/osg157-osgUtil.dll")
set(OSG_UTIL_IMPLIB "${OSG_ROOT}/lib/native/x64/release/osgUtil.lib")
set(OSG_THREADS_LIBRARY "${OSG_ROOT}/lib/native/x64/release/ot21-OpenThreads.dll")
set(OSG_THREADS_IMPLIB "${OSG_ROOT}/lib/native/x64/release/OpenThreads.lib")
set(OSG_VIEWER_LIBRARY "${OSG_ROOT}/lib/native/x64/release/osg157-osgViewer.dll")
set(OSG_VIEWER_IMPLIB "${OSG_ROOT}/lib/native/x64/release/osgViewer.lib")
set(OSG_GA_LIBRARY "${OSG_ROOT}/lib/native/x64/release/osg157-osgGA.dll")
set(OSG_GA_IMPLIB "${OSG_ROOT}/lib/native/x64/release/osgGA.lib")
set(OSGDB_OSGD_LIBRARY
  "${OSG_ROOT}/lib/native/x64/release/osgPlugins-${OSG_PLUGIN_VERSION}/osgdb_osg.dll"
  "${OSG_ROOT}/lib/native/x64/release/osgPlugins-${OSG_PLUGIN_VERSION}/osgdb_serializers_osg.dll"
  "${OSG_ROOT}/lib/native/x64/release/osgPlugins-${OSG_PLUGIN_VERSION}/osgdb_ive.dll"
)
set(OSGDB_JPEG_LIBRARY
  "${OSG_ROOT}/lib/native/x64/release/osgPlugins-${OSG_PLUGIN_VERSION}/osgdb_jpeg.dll"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OSG
  REQUIRED_VARS OSG_INCLUDE_DIR OSG_LIBRARY OSG_DB_LIBRARY OSG_UTIL_LIBRARY OSGDB_OSGD_LIBRARY OSG_THREADS_LIBRARY OSGDB_JPEG_LIBRARY
  VERSION_VAR OSG_VERSION
  HANDLE_COMPONENTS
)

if(OSG_FOUND)
  add_library(OSG::OSG SHARED IMPORTED)
  target_include_directories(OSG::OSG SYSTEM INTERFACE
    "${OSG_INCLUDE_DIR}"
  )
  set_target_properties(OSG::OSG PROPERTIES
    IMPORTED_LOCATION "${OSG_LIBRARY}"
    IMPORTED_LOCATION_DEBUG "${OSG_LIBRARY_DEBUG}"
  )

  if(WIN32)
    set_target_properties(OSG::OSG PROPERTIES
      IMPORTED_IMPLIB "${OSG_IMPLIB}"
      IMPORTED_IMPLIB_DEBUG "${OSG_IMPLIB_DEBUG}"
    )
  endif()

  set(OSG_INCLUDE_DIRS $<TARGET_PROPERTY:OSG::OSG,INTERFACE_INCLUDE_DIRECTORIES>)
  set(OSG_LIBRARIES OSG::OSG)

  add_library(OSG::DB SHARED IMPORTED)
  target_include_directories(OSG::DB SYSTEM INTERFACE
    "${OSG_INCLUDE_DIR}"
  )
  set_target_properties(OSG::DB PROPERTIES
    IMPORTED_LOCATION "${OSG_DB_LIBRARY}"
    IMPORTED_LOCATION_DEBUG "${OSG_DB_LIBRARY_DEBUG}"
  )

  if(WIN32)
    set_target_properties(OSG::DB PROPERTIES
      IMPORTED_IMPLIB "${OSG_DB_IMPLIB}"
      IMPORTED_IMPLIB_DEBUG "${OSG_DB_IMPLIB_DEBUG}"
    )
  endif()

  set(OSG_DB_INCLUDE_DIRS $<TARGET_PROPERTY:OSG::DB,INTERFACE_INCLUDE_DIRECTORIES>)
  set(OSG_DB_LIBRARIES OSG::DB)

  add_library(OSG::UTIL SHARED IMPORTED)
  target_include_directories(OSG::UTIL SYSTEM INTERFACE
    "${OSG_INCLUDE_DIR}"
  )
  set_target_properties(OSG::UTIL PROPERTIES
    IMPORTED_LOCATION "${OSG_UTIL_LIBRARY}"
    IMPORTED_LOCATION_DEBUG "${OSG_UTIL_LIBRARY_DEBUG}"
  )

  if(WIN32)
    set_target_properties(OSG::UTIL PROPERTIES
      IMPORTED_IMPLIB "${OSG_UTIL_IMPLIB}"
      IMPORTED_IMPLIB_DEBUG "${OSG_UTIL_IMPLIB_DEBUG}"
    )
  endif()

  set(OSG_UTIL_INCLUDE_DIRS $<TARGET_PROPERTY:OSG::UTIL,INTERFACE_INCLUDE_DIRECTORIES>)
  set(OSG_UTIL_LIBRARIES OSG::UTIL)

  add_library(OSG::THREADS SHARED IMPORTED)
  target_include_directories(OSG::THREADS SYSTEM INTERFACE
    "${OSG_INCLUDE_DIR}"
  )
  set_target_properties(OSG::THREADS PROPERTIES
    IMPORTED_LOCATION "${OSG_THREADS_LIBRARY}"
    IMPORTED_LOCATION_DEBUG "${OSG_THREADS_LIBRARY_DEBUG}"
  )

  if(WIN32)
    set_target_properties(OSG::THREADS PROPERTIES
      IMPORTED_IMPLIB "${OSG_THREADS_IMPLIB}"
      IMPORTED_IMPLIB_DEBUG "${OSG_THREADS_IMPLIB_DEBUG}"
    )
  endif()

  set(OSG_THREADS_INCLUDE_DIRS $<TARGET_PROPERTY:OSG::THREADS,INTERFACE_INCLUDE_DIRECTORIES>)
  set(OSG_THREADS_LIBRARIES OSG::THREADS)

  add_library(OSG::VIEWER SHARED IMPORTED)
  target_include_directories(OSG::VIEWER SYSTEM INTERFACE
    "${OSG_INCLUDE_DIR}"
  )
  set_target_properties(OSG::VIEWER PROPERTIES
    IMPORTED_LOCATION "${OSG_VIEWER_LIBRARY}"
    IMPORTED_LOCATION_DEBUG "${OSG_VIEWER_LIBRARY_DEBUG}"
  )

  if(WIN32)
    set_target_properties(OSG::VIEWER PROPERTIES
      IMPORTED_IMPLIB "${OSG_VIEWER_IMPLIB}"
      IMPORTED_IMPLIB_DEBUG "${OSG_VIEWER_IMPLIB_DEBUG}"
    )
  endif()

  set(OSG_VIEWER_INCLUDE_DIRS $<TARGET_PROPERTY:OSG::VIEWER,INTERFACE_INCLUDE_DIRECTORIES>)
  set(OSG_VIEWER_LIBRARIES OSG::VIEWER)

  add_library(OSG::GA SHARED IMPORTED)
  target_include_directories(OSG::GA SYSTEM INTERFACE
    "${OSG_INCLUDE_DIR}"
  )
  set_target_properties(OSG::GA PROPERTIES
    IMPORTED_LOCATION "${OSG_GA_LIBRARY}"
    IMPORTED_LOCATION_DEBUG "${OSG_GA_LIBRARY_DEBUG}"
  )

  if(WIN32)
    set_target_properties(OSG::GA PROPERTIES
      IMPORTED_IMPLIB "${OSG_GA_IMPLIB}"
      IMPORTED_IMPLIB_DEBUG "${OSG_GA_IMPLIB_DEBUG}"
    )
  endif()

  set(OSG_GA_INCLUDE_DIRS $<TARGET_PROPERTY:OSG::GA,INTERFACE_INCLUDE_DIRECTORIES>)
  set(OSG_GA_LIBRARIES OSG::GA)
endif()