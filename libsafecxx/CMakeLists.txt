# Copyright 2024 Christian Mazakas
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.25)
project(circle_stdlib LANGUAGES CXX)

if (NOT (CMAKE_CXX_STANDARD EQUAL "17"))
  message(FATAL_ERROR "Circle only supports C++17.")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(GenerateExportHeader)

file(
  GLOB_RECURSE safe_cxx_headers
  CONFIGURE_DEPENDS
  "include/*.hxx"
)

file(
  GLOB safe_cxx_sources
  CONFIGURE_DEPENDS
  "src/*.cxx"
)

add_library(safe_cxx ${safe_cxx_sources})

target_include_directories(safe_cxx PUBLIC include)

target_link_libraries(
  safe_cxx
  PUBLIC
)

target_sources(
  safe_cxx
  PUBLIC FILE_SET HEADERS
  BASE_DIRS include
  FILES ${safe_cxx_headers}
)

generate_export_header(safe_cxx)
target_include_directories(safe_cxx PUBLIC ${CMAKE_BINARY_DIR})

add_library(SafeCXX::core ALIAS safe_cxx)

option(SAFE_CXX_BUILD_TESTING "" OFF)
if(SAFE_CXX_BUILD_TESTING)
  set_target_properties(safe_cxx PROPERTIES VERIFY_INTERFACE_HEADER_SETS ON)
  include(CTest)
  add_subdirectory(test)
endif()
