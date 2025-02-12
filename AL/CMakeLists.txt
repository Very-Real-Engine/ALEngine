cmake_minimum_required(VERSION 3.20)
set(PROJECT_NAME AfterLife)
project(${PROJECT_NAME})

# Set Macros
set(WINDOW_NAME "ALEngine")
set(WINDOW_WIDTH 1920)
set(WINDOW_HEIGHT 1080)

# 의존성 설정
include(${CMAKE_SOURCE_DIR}/AL/Dependency.cmake)

# 소스 파일 및 헤더 파일 검색
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

# AL 라이브러리 생성
add_library(${PROJECT_NAME} STATIC ${SOURCES}) # SOURCES만 전달

# PCH 설정
target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/alpch.h)

# VulkanSDK 설정 - 설치 여부 확인 필요. 버전 확인 필요.
set(CMAKE_PREFIX_PATH "C:/VulkanSDK/1.3.296.0")
find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan::Vulkan)

# 헤더 파일 경로 포함
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${DEP_INCLUDE_DIR})

# 시스템 라이브러리 추가 - 최소 환경 조건으로 명시하기(Visual Studio - Window SDK 설치)
set(SYSTEM_LIBS WS2_32.lib WinMM.lib Version.lib Bcrypt.lib)
target_link_libraries(${PROJECT_NAME} PRIVATE ${SYSTEM_LIBS})

# lib 경로 설정
target_link_directories(${PROJECT_NAME} PUBLIC ${DEP_LIB_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC ${DEP_LIBS})

# AL의 컴파일 옵션 설정 (필요에 따라 추가)
target_compile_options(${PROJECT_NAME} PUBLIC "/utf-8")

# 매크로 정의
target_compile_definitions(${PROJECT_NAME} PUBLIC
WINDOW_NAME="${WINDOW_NAME}"
WINDOW_WIDTH=${WINDOW_WIDTH}
WINDOW_HEIGHT=${WINDOW_HEIGHT})

# 출력 디렉토리 설정
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# Dependency들이 먼저 build 될 수 있게 관계 설정 / 뒤에서 부터 컴파일
add_dependencies(${PROJECT_NAME} ${DEP_LIST})


