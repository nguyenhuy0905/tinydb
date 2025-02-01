# ##############################################################################
# libFuzzer compat check
# ##############################################################################

# check if libFuzzer can be linked
function(tinydb_check_fuzz_compile output_var)
  set(test_code
        "
#include <cstdint>

extern \"C\" int LLVMFuzzerTestOneInput(const std::uint8_t *data, std::size_t size) {
  return 0;
}
    ")
  if(MSVC)
    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS};/fsanitize=fuzzer")
    set(CMAKE_REQUIRED_LIBRARIES
            "${CMAKE_REQUIRED_LIBRARIES};/fsanitize=fuzzer")
  else()
    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS};-fsanitize=fuzzer")
    set(CMAKE_REQUIRED_LIBRARIES
            "${CMAKE_REQUIRED_LIBRARIES};-fsanitize=fuzzer")
  endif()
  check_cxx_source_compiles("${test_code}" ${output_var})
endfunction()
