# # # ############################################################################
# Sanitizer compat check
# # # ############################################################################

# check if ASan can be linked,
# then link whichever sanitizer(s) is/are included
function(tinydb_check_san_compile target visibility)
    set(sanitizer_list "")
    set(test_code
        "
    int main() {
        return 0;
    }
    ")

    if(tinydb_ENABLE_ASAN)
        list(APPEND san_list "address")
    endif()
    if(tinydb_ENABLE_UBSAN)
        list(APPEND san_list "undefined")
    endif()
    if(tinydb_ENABLE_MSAN)
        list(APPEND san_list "memory")
    endif()
    if(tinydb_ENABLE_TSAN)
        list(APPEND san_list "thread")
    endif()
    list(JOIN san_list "," sanitizer_option)
    if(MSVC)
        set(CMAKE_REQUIRED_FLAGS "/fsanitize=${sanitizer_option}")
        if(tinydb_ENABLE_ASAN)
            set(CMAKE_REQUIRED_DEFINITIONS
            "/D_DISABLE_VECTOR_ANNOTATION;/D_DISABLE_STRING_ANNOTATION")
        endif()
    else()
        set(CMAKE_REQUIRED_FLAGS "-fsanitize=${sanitizer_option}")
    endif()
    check_cxx_source_compiles("${test_code}" compiled)
    if(compiled)
        if(MSVC)
            target_compile_definitions(${target} ${visibility} "/D_DISABLE_VECTOR_ANNOTATION;/D_DISABLE_STRING_ANNOTATION")
            target_compile_options(${target} ${visibility} "/fsanitize=${sanitizer_option}")
            # target_link_options(${target} ${visibility} "/fsanitize=${sanitizer_option}")
        else()
            target_compile_options(${target} ${visibility} "-fsanitize=${sanitizer_option};-fno-omit-frame-pointer;-fno-optimize-sibling-calls")
            target_link_options(${target} ${visibility} "-fsanitize=${sanitizer_option}")
        endif()
    endif()

endfunction()
