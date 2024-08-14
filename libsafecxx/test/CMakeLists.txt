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
