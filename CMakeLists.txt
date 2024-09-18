cmake_minimum_required(VERSION 3.26.4)
cmake_policy(VERSION 3.12)
project(PenguinEngine LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

add_definitions(-DUNICODE -D_UNICODE)

set(project_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(include_dir "${project_dir}/ThirdParty")
set(lib_dir "${project_dir}/Libraries")
set(bin_dir "${project_dir}/Binaries")


set(build_dir "${project_dir}/Build")
file(MAKE_DIRECTORY ${build_dir})
set(release_dir "${build_dir}/Release")
file(MAKE_DIRECTORY ${release_dir})
set(debug_dir "${build_dir}/Debug")
file(MAKE_DIRECTORY ${debug_dir})

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

set(ZLIB_INCLUDE_DIR "${include_dir}/zlib")
set(ZLIB_LIBRARY "${lib_dir}/zlib.lib")

find_package(ZLIB REQUIRED)
if (ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
    target_link_libraries(PenguinEngine ${ZLIB_LIBRARIES})

    file(COPY "${bin_dir}/zlib.dll" DESTINATION ${release_dir})
    file(COPY "${bin_dir}/zlib.dll" DESTINATION ${debug_dir})
endif(ZLIB_FOUND)
