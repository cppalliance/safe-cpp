# Copyright 2024 Sean Baxter
# Copyright 2024 Christian Mazakas
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.25)
project(circle_proposal LANGUAGES CXX VERSION 0.1.0)

set(BUILD_TESTING ON)

find_package(Threads REQUIRED)

include(CTest)

add_library(safe_cxx INTERFACE)
target_sources(
  safe_cxx
  INTERFACE FILE_SET HEADERS
  BASE_DIRS ../libsafecxx/single-header
  FILES std2.h
)

function(add_compile_fail testname)
  add_library("compile-fail-${testname}" STATIC EXCLUDE_FROM_ALL "${testname}.cxx")
  target_link_libraries(
    "compile-fail-${testname}"
    PRIVATE
      safe_cxx
      Threads::Threads
  )
  add_test(
    NAME "compile-fail-${testname}"
    COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target "compile-fail-${testname}" --config $<CONFIG>)
  set_property(TEST "compile-fail-${testname}" PROPERTY WILL_FAIL TRUE)
endfunction()

function(add_safecxx_library libname)
  add_library(${libname} ${libname}.cxx)
  target_link_libraries(
    ${libname}
    PRIVATE
      safe_cxx
      Threads::Threads
  )
endfunction()

function(add_safecxx_test testname)
  add_executable(${testname} ${testname}.cxx)
  target_link_libraries(
    ${testname}
    PRIVATE
      safe_cxx
      Threads::Threads
  )
  add_test(NAME ${testname} COMMAND ${testname})
endfunction()

add_safecxx_test(match)
add_safecxx_test(thread_safety)
add_safecxx_test(subscript_array)
add_safecxx_test(subscript_vector)
add_safecxx_test(unsafe2)
add_safecxx_test(initlist0)
add_safecxx_test(tuple1)
add_safecxx_test(choice)
add_safecxx_test(match1)
add_safecxx_test(match2)
add_safecxx_test(cell)

add_safecxx_library(unsafe_bounds)
add_safecxx_library(unsafe3)
add_safecxx_library(unsafe5)

add_compile_fail(box)
add_compile_fail(initlist1)
add_compile_fail(iterator)
add_compile_fail(rel1)
add_compile_fail(safe)
add_compile_fail(string_view)
add_compile_fail(unsafe4)
add_compile_fail(view)
