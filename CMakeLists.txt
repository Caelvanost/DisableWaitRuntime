cmake_minimum_required(VERSION 3.21)

project(
    DisableWaitRuntime
    VERSION 0.2.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CommonLibSSE CONFIG REQUIRED)

add_library(${PROJECT_NAME} SHARED
    src/plugin.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        CommonLibSSE::CommonLibSSE
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        _UNICODE
        UNICODE
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /permissive- /W4)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "DisableWaitRuntime"
)
