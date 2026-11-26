cmake_minimum_required(VERSION 3.20)
project(CountdownTimer CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Lokasi FFGL SDK (hasil clone resolume/ffgl). Boleh path relatif.
set(FFGL_SDK_DIR "ffgl-sdk" CACHE STRING "Folder FFGL SDK")
get_filename_component(FFGL_SDK_DIR "${FFGL_SDK_DIR}" ABSOLUTE BASE_DIR "${CMAKE_SOURCE_DIR}")

set(FFGL_LIB_DIR "${FFGL_SDK_DIR}/source/lib")
if(NOT EXISTS "${FFGL_LIB_DIR}")
  message(FATAL_ERROR "Folder ${FFGL_LIB_DIR} tidak ditemukan. Cek FFGL_SDK_DIR.")
endif()

# Semua source library SDK (ffgl, ffglex, dll.)
file(GLOB_RECURSE FFGL_LIB_SRC CONFIGURE_DEPENDS
  "${FFGL_LIB_DIR}/*.cpp"
  "${FFGL_LIB_DIR}/*.c")

# Semua folder yang berisi header, supaya #include "FFGLSDK.h" dkk ketemu
file(GLOB_RECURSE FFGL_LIB_HEADERS CONFIGURE_DEPENDS
  "${FFGL_LIB_DIR}/*.h"
  "${FFGL_LIB_DIR}/*.hpp")
set(FFGL_INCLUDE_DIRS "${FFGL_LIB_DIR}")
foreach(h ${FFGL_LIB_HEADERS})
  get_filename_component(d "${h}" DIRECTORY)
  list(APPEND FFGL_INCLUDE_DIRS "${d}")
endforeach()
list(REMOVE_DUPLICATES FFGL_INCLUDE_DIRS)

add_library(CountdownTimer MODULE
  src/CountdownTimer.cpp
  src/CountdownTimer.h
  ${FFGL_LIB_SRC})

target_include_directories(CountdownTimer PRIVATE src ${FFGL_INCLUDE_DIRS})

set_target_properties(CountdownTimer PROPERTIES PREFIX "")

# Semua hasil build dikumpulkan di build/out
set(OUT_DIR "${CMAKE_BINARY_DIR}/out")
set_target_properties(CountdownTimer PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY         "${OUT_DIR}"
  LIBRARY_OUTPUT_DIRECTORY_RELEASE "${OUT_DIR}"
  RUNTIME_OUTPUT_DIRECTORY         "${OUT_DIR}"
  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUT_DIR}")

if(WIN32)
  target_compile_definitions(CountdownTimer PRIVATE NOMINMAX _CRT_SECURE_NO_WARNINGS)
  target_link_libraries(CountdownTimer PRIVATE opengl32)
elseif(APPLE)
  # Resolume di Mac memuat plugin berformat .bundle
  set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
  target_compile_definitions(CountdownTimer PRIVATE GL_SILENCE_DEPRECATION)
  set_target_properties(CountdownTimer PROPERTIES BUNDLE TRUE BUNDLE_EXTENSION "bundle")
  target_link_libraries(CountdownTimer PRIVATE "-framework OpenGL" "-framework CoreFoundation")
endif()
