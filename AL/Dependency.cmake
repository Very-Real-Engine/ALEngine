# ExternalProject 관련 명령어 셋 추가
include(ExternalProject)

# Dependency 관련 변수 설정
set(DEP_INSTALL_DIR ${PROJECT_BINARY_DIR}/install)
set(DEP_INCLUDE_DIR ${DEP_INSTALL_DIR}/include)
set(DEP_LIB_DIR ${DEP_INSTALL_DIR}/lib)

# glfw
ExternalProject_Add(
    dep_glfw
    GIT_REPOSITORY "https://github.com/glfw/glfw.git"
    GIT_TAG "3.3.2"
    GIT_SHALLOW 1
    UPDATE_COMMAND "" PATCH_COMMAND "" TEST_COMMAND ""
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${DEP_INSTALL_DIR}
        -DGLFW_BUILD_EXAMPLES=OFF
        -DGLFW_BUILD_TESTS=OFF
        -DGLFW_BUILD_DOCS=OFF
)
set(DEP_LIST ${DEP_LIST} dep_glfw)
set(DEP_LIBS ${DEP_LIBS} glfw3)

# spdlog
ExternalProject_Add(
    dep_spdlog
    GIT_REPOSITORY "https://github.com/gabime/spdlog.git"
    GIT_TAG "v1.11.0"
    GIT_SHALLOW 1
    UPDATE_COMMAND "" PATCH_COMMAND "" TEST_COMMAND ""
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${DEP_INSTALL_DIR}
        -DSPDLOG_BUILD_EXAMPLES=OFF
        -DSPDLOG_BUILD_TESTS=OFF
        -DSPDLOG_BUILD_BENCH=OFF
        -DSPDLOG_BUILD_SHARED=OFF
)

set(DEP_LIST ${DEP_LIST} dep_spdlog)
set(DEP_LIBS ${DEP_LIBS} 
    $<$<CONFIG:Debug>:spdlogd>
    $<$<CONFIG:Release>:spdlog>
)

# imgui 추가
ExternalProject_Add(
    dep_imgui
    GIT_REPOSITORY "https://github.com/Very-Real-Engine/imgui.git"
    GIT_TAG "main"
    GIT_SHALLOW 1
    UPDATE_COMMAND "" PATCH_COMMAND "" TEST_COMMAND ""
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${DEP_INSTALL_DIR}
)
set(DEP_LIST ${DEP_LIST} dep_imgui)
set(DEP_LIBS ${DEP_LIBS} imgui)

# glm
ExternalProject_Add(
	dep_glm
	GIT_REPOSITORY "https://github.com/g-truc/glm"
	GIT_TAG "0.9.9.8"
	GIT_SHALLOW 1
	UPDATE_COMMAND ""
	PATCH_COMMAND ""
    CMAKE_ARGS
        -DGLM_TEST_ENABLE=OFF  # 🔥 GLM 테스트 코드 비활성화
	TEST_COMMAND ""
	INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory
		${PROJECT_BINARY_DIR}/dep_glm-prefix/src/dep_glm/glm
		${DEP_INSTALL_DIR}/include/glm
	)
set(DEP_LIST ${DEP_LIST} dep_glm)

# stb
ExternalProject_Add(
	dep_stb
	GIT_REPOSITORY "https://github.com/nothings/stb"
	GIT_TAG "master"
	GIT_SHALLOW 1
	UPDATE_COMMAND ""
	PATCH_COMMAND ""
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	TEST_COMMAND ""
	INSTALL_COMMAND ${CMAKE_COMMAND} -E copy
		${PROJECT_BINARY_DIR}/dep_stb-prefix/src/dep_stb/stb_image.h
		${DEP_INSTALL_DIR}/include/stb/stb_image.h
	)
set(DEP_LIST ${DEP_LIST} dep_stb)

# assimp
ExternalProject_Add(
	dep_assimp
	GIT_REPOSITORY "https://github.com/assimp/assimp"
	GIT_TAG "v5.4.3"
	GIT_SHALLOW 1
	UPDATE_COMMAND ""
	PATCH_COMMAND ""
	CMAKE_ARGS
		-DCMAKE_INSTALL_PREFIX=${DEP_INSTALL_DIR}
        -DCMAKE_BUILD_TYPE=$<CONFIG>
		-DBUILD_SHARED_LIBS=OFF
		-DASSIMP_BUILD_ASSIMP_TOOLS=OFF
		-DASSIMP_BUILD_TESTS=OFF
		-DASSIMP_INJECT_DEBUG_POSTFIX=ON
		-DASSIMP_BUILD_ZLIB=ON
	TEST_COMMAND ""
)
set(DEP_LIST ${DEP_LIST} dep_assimp)
set(DEP_LIBS ${DEP_LIBS}
    $<$<CONFIG:Debug>:assimp-vc143-mtd>
    $<$<CONFIG:Release>:assimp-vc143-mt>
    $<$<CONFIG:Debug>:zlibstaticd>
    $<$<CONFIG:Release>:zlibstatic>
)
	
# yaml-cpp
ExternalProject_Add(
    dep_yaml_cpp
    GIT_REPOSITORY "https://github.com/jbeder/yaml-cpp.git"
    GIT_TAG "yaml-cpp-0.7.0" 
    GIT_SHALLOW 1
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${DEP_INSTALL_DIR}
        -DYAML_BUILD_SHARED_LIBS=OFF
        -DYAML_CPP_BUILD_TESTS=OFF
        -DYAML_CPP_BUILD_TOOLS=OFF
    TEST_COMMAND ""
)
set(DEP_LIST ${DEP_LIST} dep_yaml_cpp)
set(DEP_LIBS ${DEP_LIBS}
    $<$<CONFIG:Debug>:yaml-cppd>
    $<$<CONFIG:Release>:yaml-cpp>
)

# Mono
ExternalProject_Add(
    dep_mono
    GIT_REPOSITORY "https://github.com/Very-Real-Engine/ALE_mono.git"
    GIT_TAG "main"
    GIT_SHALLOW 1
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CONFIGURE_COMMAND "" 
    BUILD_COMMAND ""     
    INSTALL_COMMAND      
        ${CMAKE_COMMAND} -E copy_directory
        <SOURCE_DIR>/mono  
        ${DEP_INCLUDE_DIR}/mono
        COMMAND ${CMAKE_COMMAND} -E copy
        <SOURCE_DIR>/Debug/libmono-static-sgen.lib
        ${DEP_INSTALL_DIR}/lib/libmono-static-sgen-debug.lib
        COMMAND ${CMAKE_COMMAND} -E copy
        <SOURCE_DIR>/Release/libmono-static-sgen.lib
        ${DEP_INSTALL_DIR}/lib/libmono-static-sgen.lib
)

# Dependency 리스트 추가
set(DEP_LIST ${DEP_LIST} dep_mono)
# Mono 라이브러리 경로 설정
set(MONO_LIB_DEBUG ${DEP_INSTALL_DIR}/lib/libmono-static-sgen-debug.lib)
set(MONO_LIB_RELEASE ${DEP_INSTALL_DIR}/lib/libmono-static-sgen.lib)

# alglm 추가
ExternalProject_Add(
    dep_alglm
    GIT_REPOSITORY "https://github.com/Very-Real-Engine/ALGLM"
    GIT_TAG "main"
    GIT_SHALLOW 1
    UPDATE_COMMAND "" PATCH_COMMAND "" TEST_COMMAND ""
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${DEP_INSTALL_DIR}

)
set(DEP_LIST ${DEP_LIST} dep_alglm)
set(DEP_LIBS ${DEP_LIBS}
    $<$<CONFIG:Debug>:alglmd>
    $<$<CONFIG:Release>:alglm>
)
# CMake에서 빌드 타입에 따라 올바른 라이브러리를 링크
set(DEP_LIBS ${DEP_LIBS} 
    $<$<CONFIG:Debug>:${MONO_LIB_DEBUG}>
    $<$<CONFIG:Release>:${MONO_LIB_RELEASE}>
)
