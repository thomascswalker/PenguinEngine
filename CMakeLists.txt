cmake_minimum_required(VERSION 3.30.0)
cmake_policy(VERSION 3.12)
project(PenguinEngine LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

include(CMakePrintHelpers)

add_definitions(-DUNICODE -D_UNICODE)

set(project_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(include_dir "${project_dir}/ThirdParty")
set(lib_dir "${project_dir}/Libraries")
set(bin_dir "${project_dir}/Binaries")
set(build_dir "${project_dir}/Build")
file(MAKE_DIRECTORY ${build_dir})

file(
    GLOB_RECURSE source_list
    LIST_DIRECTORIES false
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    "${project_dir}/*.cpp"
    "${project_dir}/*.h"
)

add_executable(PenguinEngine WIN32 "Main.cpp" ${source_list})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT PenguinEngine)

# All header and source files
foreach(source_file IN ITEMS ${source_list})
    get_filename_component(source_path "${source_file}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source_file}")
endforeach()

# Additional include paths
target_include_directories(PenguinEngine PUBLIC source)
target_include_directories(PenguinEngine PUBLIC)
target_include_directories(PenguinEngine PUBLIC lib)

# ZLib
set(ZLIB_LIBRARY "${lib_dir}/zlib.lib")
set(ZLIB_INCLUDE_DIR "${include_dir}/zlib")
find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
    target_link_libraries(PenguinEngine PRIVATE ${ZLIB_LIBRARIES})
    target_include_directories(PenguinEngine PRIVATE ${ZLIB_INCLUDE_DIRS})
endif(ZLIB_FOUND)

# FreeType
set(FREETYPE_LIBRARY "${lib_dir}/freetype.lib")
set(FREETYPE_INCLUDE_DIRS "${include_dir}")
find_package(Freetype)
if(FREETYPE_FOUND)
    target_link_libraries(PenguinEngine PRIVATE ${FREETYPE_LIBRARIES})
    target_include_directories(PenguinEngine PRIVATE ${FREETYPE_INCLUDE_DIRS})
endif(FREETYPE_FOUND)

