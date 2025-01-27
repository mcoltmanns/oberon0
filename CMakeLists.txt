cmake_minimum_required(VERSION 3.29.6)

set(OBERON0_C "oberon0c")
project(${OBERON0_C} VERSION 0.0.1)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # warning level 4
    add_compile_options(/Wall /W4 /external:anglebrackets /external:W0)
else ()
    # lots of warnings and all warnings as errors -Wglobal-constructors -Wexit-time-destructors
    add_compile_options(-W -Wall -Wextra -Werror -Wpedantic -Wno-error=deprecated -Wunreachable-code -Winvalid-pch -Wcast-align -Wformat=2 -Wformat-nonliteral -Wmissing-declarations -Wmissing-noreturn -Wmissing-format-attribute -Wmissing-include-dirs -Wredundant-decls -Wswitch-default -Wsign-conversion)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Woverloaded-virtual>)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_options(-Wweak-vtables)
        if (WIN32)
            add_compile_options(-fms-extensions)
        endif ()
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # using GCC
    endif ()
endif ()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# define DEBUG macro
if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-D_DEBUG)
    set(-DLLVM_USE_SPLIT_DWARF ON) # apparently this can speed up compilation - but not sure if it really works
endif ()

# add the cmake folder of the project for extension modules
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(Boost_USE_MULTITHREADED ON)
set(Boost_NO_WARN_NEW_VERSIONS 1)

# add source tree to search path to avoid relative includes
include_directories(BEFORE .)

set(UTIL_SOURCES
        util/Logger.cpp util/Logger.h)

set(SCANNER_SOURCES
        scanner/Scanner.cpp scanner/Scanner.h
        scanner/Token.cpp scanner/Token.h
        scanner/LiteralToken.cpp scanner/LiteralToken.h
        scanner/IdentToken.cpp scanner/IdentToken.h
        scanner/UndefinedToken.cpp scanner/UndefinedToken.h)

set(PARSER_SOURCES
        parser/Parser.h
        parser/ast/visitor/NodeVisitor.cpp parser/ast/visitor/NodeVisitor.h
        parser/Parser.cpp
        parser/ast/nodes/IdentNode.cpp
        parser/ast/nodes/LiteralNode.cpp
        parser/ast/nodes/LiteralNode.h
        parser/ast/nodes/Node.cpp
        parser/ast/nodes/Node.h
        parser/ast/nodes/OperatorNode.cpp
        parser/ast/nodes/OperatorNode.h
)

set(SCOPER_SOURCES
        scoper/symbols/types/BaseTypes.cpp
        scoper/symbols/types/BaseTypes.h
        scoper/symbols/types/ConstructedTypes.cpp
        scoper/symbols/types/ConstructedTypes.h
        scoper/symbols/types/Type.cpp
        scoper/symbols/types/Type.h
        scoper/symbols/Constant.cpp
        scoper/symbols/Constant.h
        scoper/symbols/Procedure.cpp
        scoper/symbols/Procedure.h
        scoper/symbols/Symbol.cpp
        scoper/symbols/Symbol.h
        scoper/symbols/Variable.cpp
        scoper/symbols/Variable.h
        scoper/symbols/Module.cpp
        scoper/symbols/Module.h
        scoper/Scope.cpp
        scoper/Scope.h
        scoper/symbols/PassedParam.cpp
        scoper/symbols/PassedParam.h
        scoper/Scoper.cpp
        scoper/Scoper.h)

set(TYPECHECKER_SOURCES
        typechecker/TypeChecker.cpp
        typechecker/TypeChecker.h)

set(CODEGEN_SOURCES
        codegen/Generator.cpp
        codegen/Generator.h)

set(ALL_SOURCES global.h ${UTIL_SOURCES} ${SCANNER_SOURCES} ${PARSER_SOURCES} ${SCOPER_SOURCES} ${TYPECHECKER_SOURCES} ${CODEGEN_SOURCES}
        codegen/LLVMMachine.cpp
        codegen/LLVMMachine.h)

add_executable(${OBERON0_C} main.cpp ${ALL_SOURCES})

find_package(Boost REQUIRED COMPONENTS program_options)
if (Boost_FOUND)
    message(STATUS "Found Boost: ${Boost_VERSION}")
    target_link_libraries(${OBERON0_C} PRIVATE Boost::headers Boost::program_options)
endif ()

find_package(LLVM REQUIRED CONFIG)
if (LLVM_FOUND)
    if (${LLVM_VERSION} LESS 16.0.0)
        message(STATUS "Warning: found legacy LLVM: ${LLVM_VERSION}")
        add_definitions(-D_LLVM_LEGACY)
    else ()
        message(STATUS "Found LLVM: ${LLVM_VERSION}")
        if (${LLVM_VERSION} MATCHES "16.*.*")
            add_definitions(-D_LLVM_16)
        elseif (${LLVM_VERSION} MATCHES "17.*.*")
            add_definitions(-D_LLVM_17)
        elseif (${LLVM_VERSION} MATCHES "18.*.*")
            add_definitions(-D_LLVM_18)
        endif()
    endif ()
    add_definitions(${LLVM_DEFINITIONS})
    include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
    llvm_map_components_to_libnames(llvm_libs core support passes ${LLVM_TARGETS_TO_BUILD})
    target_link_libraries(${OBERON0_C} PRIVATE ${llvm_libs})
endif ()

configure_file(config.h.in config.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})