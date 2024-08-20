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

function(safe_cxx_compile_fail_test testname fail_regex)
  add_library("compile-fail-${testname}" STATIC EXCLUDE_FROM_ALL "compile-fail/${testname}.cxx")
  target_link_libraries(
    "compile-fail-${testname}"
    PRIVATE
      SafeCXX::core
  )
  add_test(
    NAME "safecxx-compile-fail-${testname}"
    COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target "compile-fail-${testname}" --config $<CONFIG>)
    set_property(TEST "safecxx-compile-fail-${testname}" PROPERTY PASS_REGULAR_EXPRESSION "${fail_regex}")
endfunction()

safe_cxx_compile_fail_test(string_view1 "use of sv depends on expired loan")
safe_cxx_compile_fail_test(box_incomplete "member type incomplete is incomplete")
safe_cxx_compile_fail_test(vector1 "use of strs depends on expired loan")
