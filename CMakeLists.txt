cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

# set(imager_cmake_module_dir "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules")
# list(APPEND CMAKE_MODULE_PATH "${imager_cmake_module_dir}")

set(imager_major_version 0)
set(imager_minor_version 1)
set(imager_patch_version 0)
set(imager_version ${imager_major_version}.${imager_minor_version}.${imager_patch_version})

project(Imager LANGUAGES CXX VERSION ${imager_version})

# include(${imager_cmake_module_dir}/Platform.cmake)
# include(${imager_cmake_module_dir}/Compiler.cmake)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# 全局设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # 必须支持指定的标准
set(CMAKE_CXX_EXTENSIONS OFF)        # 禁用编译器特定扩展

add_subdirectory(External)
add_subdirectory(Source)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "Imager")
