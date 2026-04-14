cmake_minimum_required(VERSION 3.22)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# 添加components目录
set(EXTRA_COMPONENT_DIRS ${CMAKE_SOURCE_DIR}/components)

# "Trim" the build. Include the minimal set of components, main, and anything it depends on.
idf_build_set_property(MINIMAL_BUILD ON)

# ========== 版本与 Git 信息自动生成 ==========
function(read_sdkconfig_value out_var key default_val)
    set(value ${default_val})
    if(EXISTS "${CMAKE_SOURCE_DIR}/sdkconfig")
        file(STRINGS "${CMAKE_SOURCE_DIR}/sdkconfig" lines REGEX "^${key}=")
        if(lines)
            list(GET lines 0 line)
            string(REGEX REPLACE "${key}=(.*)" "\\1" extracted "${line}")
            set(value ${extracted})
        endif()
    endif()
    set(${out_var} ${value} PARENT_SCOPE)
endfunction()

read_sdkconfig_value(PROJECT_VERSION_MAJOR "CONFIG_APP_VERSION_MAJOR" 1)
read_sdkconfig_value(PROJECT_VERSION_MINOR "CONFIG_APP_VERSION_MINOR" 0)
read_sdkconfig_value(PROJECT_VERSION_PATCH "CONFIG_APP_VERSION_PATCH" 0)

find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
endif()

if(NOT GIT_COMMIT)
    set(GIT_COMMIT "unknown")
endif()
if(NOT GIT_BRANCH)
    set(GIT_BRANCH "unknown")
endif()

string(TIMESTAMP BUILD_TIME "%Y-%m-%d %H:%M:%S")

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/generated)

configure_file(
    ${CMAKE_SOURCE_DIR}/main/version_info.h.in
    ${CMAKE_BINARY_DIR}/generated/version_info.h
)

project(AlientekAiBox1)
