# Check whether libc++ can be linked
function(tinydb_check_libcxx_compile libcxx_output_var)
    set(test_code "int main() { return 0; }")
    set(CMAKE_REQUIRED_FLAGS "-stdlib=libc++")
    set(CMAKE_REQUIRED_LIBRARIES "-stdlib=libc++")
    check_cxx_source_compiles("${test_code}" ${libcxx_output_var})
endfunction()
