cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 17)

project(compiler)

add_executable(
    compiler
    src/common.h
    src/lexer.cpp
    src/lexer.h
    src/main.cpp
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    PROPERTY VS_STARTUP_PROJECT compiler
)

set_property(TARGET compiler PROPERTY
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

if(MSVC)
    set(FLAGS
        /fp:fast /Oi /W4 /WX
        /wd4201 /wd4100 /wd4189 /wd4127 /wd4505 /wd4200
        /we4062 /we5262
    )
    target_compile_options(compiler PRIVATE ${FLAGS})
endif()
