# Copyright 2024 Christian Mazakas
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

function(safe_cxx_test testname)
  add_executable(${testname} ${testname}.cxx)
  target_link_libraries(
    ${testname}
    PRIVATE
      SafeCXX::core
  )
  add_test(NAME safecxx-${testname} COMMAND ${testname})
endfunction()

file(
  GLOB safe_cxx_test_sources
  CONFIGURE_DEPENDS
  "*_test.cxx"
)

foreach(test_source ${safe_cxx_test_sources})
  cmake_path(SET test_path ${test_source})
  cmake_path(GET test_path STEM test_filename)
  safe_cxx_test(${test_filename})
endforeach()

function(safe_cxx_compile_file_test testname)
  add_library("compile-fail-${testname}" STATIC EXCLUDE_FROM_ALL "compile-fail/${testname}.cxx")
  target_link_libraries(
    "compile-fail-${testname}"
    PRIVATE
      SafeCXX::core
  )
  add_test(NAME "safecxx-compile-fail-${testname}" COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target "compile-fail-${testname}" --config $<CONFIG>)
  set_tests_properties("safecxx-compile-fail-${testname}" PROPERTIES WILL_FAIL TRUE RUN_SERIAL TRUE)
endfunction()

file(
  GLOB safe_cxx_compile_fail_test_sources
  CONFIGURE_DEPENDS
  "compile-fail/*.cxx"
)

foreach(test_source ${safe_cxx_compile_fail_test_sources})
  cmake_path(SET test_path ${test_source})
  cmake_path(GET test_path STEM test_filename)
  safe_cxx_compile_file_test(${test_filename})
endforeach()
