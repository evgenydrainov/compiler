cmake_minimum_required(VERSION 3.20)

set(CMAKE_UNITY_BUILD ON)
set(CMAKE_UNITY_BUILD_BATCH_SIZE 64)

set(CMAKE_CXX_STANDARD 17)

project(compiler)

add_executable(
    compiler
    src/base/base_arena.h
    src/base/base_bump_array.h
    src/base/base_defer.h
    src/base/base_dynamic_array.h
    src/base/base_enum.h
    src/base/base_file.h
    src/base/base_list.h
    src/base/base_slice.h
    src/base/base_static_bump_array.h
    src/base/base_string.h
    src/base/base_string_builder.h
    src/base/base_types.h
    src/base/base.h
    src/codegen.cpp
    src/codegen.h
    src/compiler.cpp
    src/compiler.h
    src/constants_table.h
    src/function_table.h
    src/lexer.cpp
    src/lexer.h
    src/macro_table.h
    src/main.cpp
    src/microsoft_craziness.h
    src/parser.cpp
    src/parser.h
    src/print_code.cpp
    src/print_code.h
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
    RUNTIME_OUTPUT_DIRECTORY_DEBUG          "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL     "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE        "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}"
)

set(FLAGS
    /fp:fast /Oi /W4 /WX
    /wd4201 /wd4100 /wd4189 /wd4127 /wd4505 /wd4200
    /we4062 /we5262
    /Zc:strictStrings-
)
target_compile_options(compiler PRIVATE ${FLAGS})
target_compile_options(run_tests PRIVATE ${FLAGS})

target_compile_options(
    compiler PRIVATE
    /MT
)
target_link_options(
    compiler PRIVATE
    /INCREMENTAL:NO
)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/distrib)

install(TARGETS compiler
    RUNTIME DESTINATION .
)
install(DIRECTORY examples/
    DESTINATION examples
    PATTERN "*.asm" EXCLUDE
    PATTERN "*.obj" EXCLUDE
    PATTERN "*.exe" EXCLUDE
    PATTERN "*.pdb" EXCLUDE
)
install(DIRECTORY modules/ DESTINATION modules)
install(DIRECTORY vendor/ DESTINATION vendor)
