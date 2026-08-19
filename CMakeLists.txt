cmake_minimum_required(VERSION 3.20)

set(CMAKE_UNITY_BUILD ON)
set(CMAKE_UNITY_BUILD_BATCH_SIZE 64)

set(CMAKE_CXX_STANDARD 17)

project(compiler)

add_executable(
    compiler
    src/codegen.cpp
    src/codegen.h
    src/common.h
    src/common_arena.h
    src/common_array.h
    src/common_defer.h
    src/common_file.cpp
    src/common_file.h
    src/common_string.h
    src/common_types.h
    src/compiler.cpp
    src/compiler.h
    src/constants_table.h
    src/function_table.h
    src/lexer.cpp
    src/lexer.h
    src/main.cpp
    src/parser.cpp
    src/parser.h
    src/semantic_pass.cpp
    src/semantic_pass.h
    src/stuff.natvis
    src/symbol_table.h
    src/type.h
    src/type_table.h
)

add_executable(
    run_tests
    src/run_tests.cpp
)

add_dependencies(run_tests compiler)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    PROPERTY VS_STARTUP_PROJECT run_tests
)

set_property(TARGET compiler PROPERTY
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
set_property(TARGET run_tests PROPERTY
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

set_target_properties(compiler PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}"
)

if(MSVC)
    set(FLAGS
        /fp:fast /Oi /W4 /WX
        /wd4201 /wd4100 /wd4189 /wd4127 /wd4505 /wd4200
        /we4062 /we5262
        /Zc:strictStrings-
    )
    target_compile_options(compiler PRIVATE ${FLAGS})
    target_compile_options(run_tests PRIVATE ${FLAGS})
endif()
