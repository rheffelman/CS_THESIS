cmake_minimum_required(VERSION 3.20)
project(antlr_playground CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# ---------------------------------------------------------------------------
# ANTLR tool (the .jar that turns .g4 files into C++)
# ---------------------------------------------------------------------------
set(ANTLR_VERSION 4.13.2)
find_package(Java REQUIRED COMPONENTS Runtime)
set(ANTLR_JAR "$ENV{USERPROFILE}/.m2/repository/org/antlr/antlr4/${ANTLR_VERSION}/antlr4-${ANTLR_VERSION}-complete.jar"
    CACHE FILEPATH "Path to antlr4-${ANTLR_VERSION}-complete.jar")
if(NOT EXISTS "${ANTLR_JAR}")
  message(FATAL_ERROR "ANTLR jar not found at ${ANTLR_JAR}\n"
                      "Run `antlr4-parse -v ${ANTLR_VERSION} -h` once to download it, or pass -DANTLR_JAR=<path>.")
endif()

# ---------------------------------------------------------------------------
# ANTLR C++ runtime (the library generated code links against).
# Downloaded once on first configure, then compiled once.
# ---------------------------------------------------------------------------
include(FetchContent)
set(ANTLR_BUILD_CPP_TESTS OFF CACHE BOOL "" FORCE)
set(ANTLR_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(ANTLR_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(WITH_STATIC_CRT OFF CACHE BOOL "" FORCE)
set(DISABLE_WARNINGS ON CACHE BOOL "" FORCE)
FetchContent_Declare(antlr4_runtime
  URL https://www.antlr.org/download/antlr4-cpp-runtime-${ANTLR_VERSION}-source.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(antlr4_runtime)
set(ANTLR_RUNTIME_INCLUDE "${antlr4_runtime_SOURCE_DIR}/runtime/src")

# ---------------------------------------------------------------------------
# Shared playground driver (tree printer, token table, error messages, CLI)
# ---------------------------------------------------------------------------
add_library(playground_common STATIC common/playground.cpp common/console.cpp)
target_include_directories(playground_common PUBLIC common ${ANTLR_RUNTIME_INCLUDE})
target_compile_definitions(playground_common PUBLIC ANTLR4CPP_STATIC)
target_link_libraries(playground_common PUBLIC antlr4_static)

# ---------------------------------------------------------------------------
# One executable per grammar: grammars/<Name>/<Name>.g4 + grammars/<Name>/*.cpp
# ---------------------------------------------------------------------------
file(GLOB GRAMMAR_DIRS LIST_DIRECTORIES true CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/grammars/*")
foreach(dir ${GRAMMAR_DIRS})
  get_filename_component(name "${dir}" NAME)
  set(g4 "${dir}/${name}.g4")
  if(NOT EXISTS "${g4}")
    continue()
  endif()

  set(gen "${CMAKE_BINARY_DIR}/generated/${name}")
  set(gen_srcs
    "${gen}/${name}Lexer.cpp"   "${gen}/${name}Parser.cpp"
    "${gen}/${name}Visitor.cpp" "${gen}/${name}BaseVisitor.cpp"
    "${gen}/${name}Listener.cpp" "${gen}/${name}BaseListener.cpp")

  add_custom_command(
    OUTPUT ${gen_srcs}
    COMMAND "${Java_JAVA_EXECUTABLE}" -jar "${ANTLR_JAR}"
            -Dlanguage=Cpp -visitor -listener -Xexact-output-dir -o "${gen}" "${g4}"
    DEPENDS "${g4}"
    COMMENT "ANTLR: generating C++ from ${name}.g4"
    VERBATIM)

  file(GLOB user_srcs CONFIGURE_DEPENDS "${dir}/*.cpp")
  if(NOT user_srcs)
    # No main.cpp yet: use a generic one that just prints trees.
    set(NAME ${name})
    configure_file("${CMAKE_SOURCE_DIR}/templates/default_main.cpp.in" "${gen}/default_main.cpp" @ONLY)
    set(user_srcs "${gen}/default_main.cpp")
  endif()
  add_executable(${name} ${user_srcs} ${gen_srcs})
  target_include_directories(${name} PRIVATE "${gen}")
  target_link_libraries(${name} PRIVATE playground_common)
  set_target_properties(${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
  # The generated code triggers a few harmless warnings; keep output readable.
  target_compile_options(${name} PRIVATE -w)
  if(MINGW)
    # Self-contained .exe: no dependency on MinGW's DLLs being on PATH.
    target_link_options(${name} PRIVATE -static)
  endif()
endforeach()
