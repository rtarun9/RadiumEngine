cmake_minimum_required(VERSION 3.17)

project(RadiumEngine)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE")

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Bin")

find_path(STB_INCLUDE_DIRS "stb.h")

find_package(tinyobjloader CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

include_directories(
  ${PROJECT_NAME}
  Src/Include
)

file(GLOB_RECURSE SRC_FILES
    ${PROJECT_SOURCE_DIR}/**/*.cpp
)

add_executable(${PROJECT_NAME} WIN32 ${SRC_FILES})

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    ${STB_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/Src/Include
)

target_precompile_headers(${PROJECT_NAME} 
    PRIVATE
    ${PROJECT_SOURCE_DIR}/Src/Include/Pch.hpp
)

target_link_libraries(${PROJECT_NAME} tinyobjloader::tinyobjloader imgui::imgui d3d11.lib dxgi.lib d3dcompiler.lib dxguid.lib) 

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})