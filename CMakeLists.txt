cmake_minimum_required(VERSION 3.20)
project(CountdownTimer CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Lokasi FFGL SDK (hasil clone resolume/ffgl)
set(FFGL_SDK_DIR "ffgl-sdk" CACHE STRING "Folder FFGL SDK")
get_filename_component(FFGL_SDK_DIR "${FFGL_SDK_DIR}" ABSOLUTE BASE_DIR "${CMAKE_SOURCE_DIR}")

set(FFGL_LIB_DIR "${FFGL_SDK_DIR}/source/lib")
if(NOT EXISTS "${FFGL_LIB_DIR}")
  message(FATAL_ERROR "Folder ${FFGL_LIB_DIR} tidak ditemukan. Cek FFGL_SDK_DIR.")
endif()

# Source dan header library SDK
file(GLOB_RECURSE FFGL_LIB_SRC CONFIGURE_DEPENDS
  "${FFGL_LIB_DIR}/*.cpp"
  "${FFGL_LIB_DIR}/*.c")

file(GLOB_RECURSE FFGL_LIB_HEADERS CONFIGURE_DEPENDS
  "${FFGL_LIB_DIR}/*.h"
  "${FFGL_LIB_DIR}/*.hpp")
set(FFGL_INCLUDE_DIRS "${FFGL_LIB_DIR}")
foreach(h ${FFGL_LIB_HEADERS})
  get_filename_component(d "${h}" DIRECTORY)
  list(APPEND FFGL_INCLUDE_DIRS "${d}")
endforeach()
list(REMOVE_DUPLICATES FFGL_INCLUDE_DIRS)

# Source plugin: terbaca di folder utama maupun folder src
file(GLOB PLUGIN_SRC CONFIGURE_DEPENDS
  "${CMAKE_SOURCE_DIR}/*.cpp"
  "${CMAKE_SOURCE_DIR}/src/*.cpp")
if(NOT PLUGIN_SRC)
  message(FATAL_ERROR "File .cpp plugin tidak ditemukan di repo.")
endif()

add_library(CountdownTimer MODULE ${PLUGIN_SRC} ${FFGL_LIB_SRC})

target_include_directories(CountdownTimer PRIVATE
  "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/src" ${FFGL_INCLUDE_DIRS})

set_target_properties(CountdownTimer PROPERTIES PREFIX "")

# Hasil build dikumpulkan di build/out
set(OUT_DIR "${CMAKE_BINARY_DIR}/out")
set_target_properties(CountdownTimer PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY         "${OUT_DIR}"
  LIBRARY_OUTPUT_DIRECTORY_RELEASE "${OUT_DIR}"
  RUNTIME_OUTPUT_DIRECTORY         "${OUT_DIR}"
  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUT_DIR}")

if(WIN32)
  target_compile_definitions(CountdownTimer PRIVATE NOMINMAX _CRT_SECURE_NO_WARNINGS)
  target_link_libraries(CountdownTimer PRIVATE opengl32)
endif()
