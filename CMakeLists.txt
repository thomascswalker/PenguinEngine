cmake_minimum_required(VERSION 3.26.4)
cmake_policy(VERSION 3.12)
project(PenguinEngine LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

add_definitions(-DUNICODE -D_UNICODE)

# Files

set(project_dir "${CMAKE_CURRENT_SOURCE_DIR}")
file(
    GLOB_RECURSE source_list 
    LIST_DIRECTORIES false
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    "${project_dir}/*.cpp"
    "${project_dir}/*.h*"
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
target_include_directories(PenguinEngine PUBLIC include)
target_include_directories(PenguinEngine PUBLIC lib)
target_include_directories(PenguinEngine PUBLIC bin)

# cmake 
# -DZLIB_INCLUDE_DIR=d:\mystuff\zlib-1.2.11 
# -DZLIB_LIBRARY=d:\mystuff\zlib-1.2.11\build\zlibstatic.lib 
# -DBUILD_SHARED_LIBS=OFF 
# -DCMAKE_BUILD_TYPE=Release
# Libraries
set(ZLIB_INCLUDE_DIR "${project_dir}/include/zlib")
set(ZLIB_LIBRARY "${project_dir}/lib/zlib.lib")
message("${ZLIB_ROOT}")
find_package(ZLIB REQUIRED)
if (ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
    target_link_libraries(PenguinEngine ${ZLIB_LIBRARIES})

    install(FILES "${project_dir}/lib/zlib.dll" DESTINATION "${project_dir}/build/release/zlib.dll")
endif(ZLIB_FOUND)
