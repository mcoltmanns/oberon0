cmake_minimum_required(VERSION 3.21)
set(OBERON0_C "oberon0c")
project(${OBERON0_C})


if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # warning level 4
    add_compile_options(/Wall /W4 /w44265 /w44061 /w44062 /experimental:external /external:W0 /external:anglebrackets)
else()
    # lots of warnings and all warnings as errors -Wglobal-constructors -Wexit-time-destructors
    add_compile_options(-W -Wall -Wextra -Werror -Wpedantic -Wunreachable-code -Wshadow -Woverloaded-virtual -Winvalid-pch -Wcast-align -Wformat=2 -Wformat-nonliteral -Wmissing-declarations -Wmissing-noreturn -Wmissing-format-attribute -Wmissing-include-dirs -Wredundant-decls -Wswitch-default -Wsign-conversion)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_options(-Wweak-vtables)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # using GCC
    endif()
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSIONS ON)

set(Boost_USE_MULTITHREADED TRUE)

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
        parser/ast/Node.cpp parser/ast/Node.h
        parser/ast/NodeVisitor.cpp parser/ast/NodeVisitor.h)

set(ALL_SOURCES global.h ${UTIL_SOURCES} ${SCANNER_SOURCES} ${PARSER_SOURCES})

add_executable(${OBERON0_C} main.cpp ${ALL_SOURCES})

find_package(Boost REQUIRED COMPONENTS filesystem program_options)
if (Boost_FOUND)
    include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
    target_link_libraries(${OBERON0_C} ${Boost_LIBRARIES})
endif()

configure_file(config.h.in config.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})