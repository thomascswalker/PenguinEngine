cmake_minimum_required(VERSION 3.30.0)
cmake_policy(VERSION 3.12)
project(PenguinEngine LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

add_definitions(-DUNICODE -D_UNICODE)

# Files

set(source_root_path "${CMAKE_CURRENT_SOURCE_DIR}")
file(
    GLOB_RECURSE source_list 
    LIST_DIRECTORIES false
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    "${source_root_path}/*.cpp"
    "${source_root_path}/*.h*"
)

add_executable(PenguinEngine WIN32 "Main.cpp" ${source_list})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT PenguinEngine)

foreach(source_file IN ITEMS ${source_list})
    get_filename_component(source_path "${source_file}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source_file}")
endforeach()

target_include_directories(PenguinEngine PUBLIC "${PROJECT_SOURCE_DIR}/Source/Public")
target_include_directories(PenguinEngine PUBLIC "${PROJECT_SOURCE_DIR}/Include")

# Options

