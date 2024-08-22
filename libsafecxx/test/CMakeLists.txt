# Copyright 2024 Christian Mazakas
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

find_package(Threads REQUIRED)

function(safe_cxx_test testname)
  add_executable(${testname} ${testname}.cxx)
  target_link_libraries(
    ${testname}
    PRIVATE
      SafeCXX::core
      Threads::Threads
  )
  add_test(NAME safecxx-${testname} COMMAND ${testname})
endfunction()

file(
  GLOB safe_cxx_test_sources
  CONFIGURE_DEPENDS
  "*.cxx"
)

foreach(test_source ${safe_cxx_test_sources})
  cmake_path(SET test_path ${test_source})
  cmake_path(GET test_path STEM test_filename)
  safe_cxx_test(${test_filename})
endforeach()

function(safe_cxx_lang_fail_test testname)
  cmake_path(SET test_path ${testname})
  cmake_path(GET test_path STEM test_filename)

  add_library("lang-fail-${test_filename}" STATIC EXCLUDE_FROM_ALL "${testname}")
  set(test_name "lang-fail-${test_filename}")
  target_link_libraries(
    "${test_name}"
    PRIVATE
      SafeCXX::core
      Threads::Threads
  )

  add_test(
    NAME "safecxx-lang-fail-${test_filename}"
    COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target "${test_name}" --config $<CONFIG>)

  set_property(TEST "safecxx-lang-fail-${test_filename}" PROPERTY WILL_FAIL TRUE)
endfunction()

function(safe_cxx_panic_fail_test testname)
  cmake_path(SET test_path ${testname})
  cmake_path(GET test_path STEM test_filename)

  set(test_name "panic-fail-${test_filename}")
  add_executable("${test_name}" "${testname}" "compile-fail/abort_handler.cxx")
  target_link_libraries(
    "${test_name}"
    PRIVATE
      SafeCXX::core
      Threads::Threads
  )

  add_test(
    NAME "safecxx-panic-fail-${test_filename}"
    COMMAND "${test_name}"
  )

  set_property(TEST "safecxx-panic-fail-${test_filename}" PROPERTY WILL_FAIL TRUE)
endfunction()

file(
  GLOB_RECURSE safe_cxx_lang_fail_test_sources
  CONFIGURE_DEPENDS
  RELATIVE "${CMAKE_CURRENT_LIST_DIR}"
  "${CMAKE_CURRENT_LIST_DIR}/compile-fail/unsafe/*.cxx"
)

file(
  GLOB_RECURSE safe_cxx_panic_fail_test_sources
  CONFIGURE_DEPENDS
  RELATIVE "${CMAKE_CURRENT_LIST_DIR}"
  "${CMAKE_CURRENT_LIST_DIR}/compile-fail/panic/*.cxx"
)

foreach(testname ${safe_cxx_lang_fail_test_sources})
  safe_cxx_lang_fail_test("${testname}")
endforeach()

foreach(testname ${safe_cxx_panic_fail_test_sources})
  safe_cxx_panic_fail_test("${testname}")
endforeach()

function(safe_cxx_compile_fail_test testname fail_regex)
  add_library("compile-fail-${testname}" STATIC EXCLUDE_FROM_ALL "compile-fail/${testname}.cxx")
  target_link_libraries(
    "compile-fail-${testname}"
    PRIVATE
      SafeCXX::core
      Threads::Threads
  )
  add_test(
    NAME "safecxx-compile-fail-${testname}"
    COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target "compile-fail-${testname}" --config $<CONFIG>)
  set_property(TEST "safecxx-compile-fail-${testname}" PROPERTY PASS_REGULAR_EXPRESSION "${fail_regex}")
endfunction()

safe_cxx_compile_fail_test(string_view1 "use of sv depends on expired loan")
safe_cxx_compile_fail_test(box_incomplete "member type incomplete is incomplete")
safe_cxx_compile_fail_test(vector1 "use of strs depends on expired loan")
safe_cxx_compile_fail_test(vector2 "use of it depends on expired loan")
safe_cxx_compile_fail_test(mutex1 "use of \\*x depends on expired loan")
safe_cxx_compile_fail_test(mutex2 "cannot convert prvalue const int\\^ to int\\^")
