cmake_minimum_required(VERSION 3.30.0)
cmake_policy(VERSION 3.12)
project(PenguinEngine LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

include(CMakePrintHelpers)

add_definitions(-DUNICODE -D_UNICODE)

set(PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(INCLUDE_DIR "${PROJECT_DIR}/ThirdParty")
set(LIB_DIR "${PROJECT_DIR}/Libraries")
set(BIN_DIR "${PROJECT_DIR}/Binaries")
set(BUILD_DIR "${PROJECT_DIR}/Build")
file(MAKE_DIRECTORY ${BUILD_DIR})

# Glob all .cpp Core files
file(
    GLOB_RECURSE CORE_SOURCE_FILES
    LIST_DIRECTORIES false
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ./Source/Core/*.cpp
    ./Source/Core/*.h
)
foreach(source_file IN ITEMS ${CORE_SOURCE_FILES})
    get_filename_component(source_path "${source_file}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source_file}")
endforeach()

# Add to PCore.dll
add_library(
    PCore
    SHARED
    ${CORE_SOURCE_FILES}
)

file(
    GLOB_RECURSE EDITOR_SOURCE_FILES
    LIST_DIRECTORIES false
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ./Source/Editor/*.cpp
    ./Source/Editor/*.h
)

add_executable(PEditor WIN32 ./Source/Editor/Main.cpp)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT PEditor)

install(TARGETS PCore
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

find_library(PCoreLocation PCore)
target_link_libraries(PEditor PUBLIC PCore)

# Additional include paths
target_include_directories(PCore PUBLIC "Source/Core")
target_include_directories(PCore PUBLIC "Source/Editor")
target_include_directories(PCore PUBLIC "ThirdParty")

# ZLib
set(ZLIB_LIBRARY "${LIB_DIR}/zlib.lib")
set(ZLIB_INCLUDE_DIR "${INCLUDE_DIR}/zlib")
find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
    target_link_libraries(PCore PRIVATE ${ZLIB_LIBRARIES})
    target_include_directories(PCore PRIVATE ${ZLIB_INCLUDE_DIRS})
endif(ZLIB_FOUND)

# FreeType
set(FREETYPE_LIBRARY "${LIB_DIR}/freetype.lib")
set(FREETYPE_INCLUDE_DIRS "${INCLUDE_DIR}")
find_package(Freetype)
if(FREETYPE_FOUND)
    target_link_libraries(PCore PRIVATE ${FREETYPE_LIBRARIES})
    target_include_directories(PCore PRIVATE ${FREETYPE_INCLUDE_DIRS})
endif(FREETYPE_FOUND)

