cmake_minimum_required(VERSION 3.20)
project(GameEngine)

if(NOT WIN32)
    message(FATAL_ERROR "This project can only be built on Windows.")
endif()

message(STATUS "Building on Windows environment.")

# C++ 표준 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(SANDBOXPROJECT "Sandbox/Project/Assets/Scripts")

# CMake 빌드 타입에 따라 변수 설정
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(START_SCENE "3DExample.ale")
    set(BUILD_TYPE "Debug")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(START_SCENE "3DExample.ale")
    set(BUILD_TYPE "Release")
endif()

message(${CMAKE_SOURCE_DIR})

# 템플릿 파일을 사용하여 실제 설정 파일 생성
configure_file(
    "${CMAKE_SOURCE_DIR}/projects/template/AfterLife.alproj.in"
    "${CMAKE_SOURCE_DIR}/projects/AfterLife.alproj"
    @ONLY
)

# 서브 디렉토리 추가
add_subdirectory(AL-ScriptCore)
add_subdirectory(AL)
add_subdirectory(${SANDBOXPROJECT})
add_subdirectory(Sandbox)