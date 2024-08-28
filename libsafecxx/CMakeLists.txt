# Copyright 2024 Christian Mazakas
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.25)
project(circle_stdlib LANGUAGES CXX)

if (NOT (CMAKE_CXX_STANDARD GREATER_EQUAL "20"))
  message(FATAL_ERROR "Circle only supports C++20 and up.")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(GenerateExportHeader)

file(
  GLOB_RECURSE safe_cxx_headers
  CONFIGURE_DEPENDS
  "include/*.h"
)

file(
  GLOB safe_cxx_sources
  CONFIGURE_DEPENDS
  "src/*.cxx"
)

# add_library(safe_cxx ${safe_cxx_sources})
add_library(safe_cxx INTERFACE)

target_include_directories(safe_cxx INTERFACE include)

target_link_libraries(
  safe_cxx
  INTERFACE
)

target_sources(
  safe_cxx
  PUBLIC FILE_SET HEADERS
  BASE_DIRS include
  FILES ${safe_cxx_headers}
)

add_library(SafeCXX::core ALIAS safe_cxx)

option(SAFE_CXX_BUILD_TESTING "" OFF)
if(SAFE_CXX_BUILD_TESTING)
  set_target_properties(safe_cxx PROPERTIES VERIFY_INTERFACE_HEADER_SETS ON)
  target_compile_definitions(safe_cxx INTERFACE LIBSAFECXX_PANIC_THROWS)
  include(CTest)
  add_subdirectory(test)
endif()
