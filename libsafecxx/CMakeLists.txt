# Copyright 2024 Christian Mazakas
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.25)
project(circle_stdlib LANGUAGES CXX VERSION 0.1.0)

if (NOT (CMAKE_CXX_STANDARD GREATER_EQUAL "20"))
  message(FATAL_ERROR "Circle only supports C++20 and up.")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(GenerateExportHeader)
include(GNUInstallDirs)

file(
  GLOB_RECURSE safe_cxx_headers
  CONFIGURE_DEPENDS
  "single-header/*.h"
  # "include/*.h"
)

add_library(safe_cxx INTERFACE)

target_include_directories(
  safe_cxx
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/single-header>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_sources(
  safe_cxx
  INTERFACE FILE_SET HEADERS
  BASE_DIRS single-header
  FILES ${safe_cxx_headers}
)

add_library(SafeCXX::core ALIAS safe_cxx)

install(
  TARGETS
    safe_cxx

  EXPORT
    safecxx-targets

  FILE_SET HEADERS

  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
  EXPORT
    safecxx-targets

  NAMESPACE SafeCXX::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/safecxx
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/safecxx-config-version.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/safecxx-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/safecxx-config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/safecxx
)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/safecxx-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/safecxx-config-version.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/safecxx
)

option(SAFE_CXX_BUILD_TESTING "" OFF)
if(SAFE_CXX_BUILD_TESTING)
  set_target_properties(safe_cxx PROPERTIES VERIFY_INTERFACE_HEADER_SETS ON)
  target_compile_definitions(safe_cxx INTERFACE LIBSAFECXX_PANIC_THROWS)
  include(CTest)
  add_subdirectory(test)
endif()
