# Project option configurations

include(CheckCXXSourceCompiles)

# set PCH headers when PCH is enabled
macro(set_pch_options)
    cmake_parse_arguments(tinydb "" "" PCH ${ARGN})
    target_precompile_headers(tinydb_compile_opts INTERFACE ${tinydb_PCH})
endmacro()

# local configs
macro(tinydb_local_config)
    if(NOT tinydb_ENABLE_DEBUG_OPTIMIZATION)
        if(MSVC)
            target_compile_options(tinydb_compile_opts INTERFACE "/Od")
        else()
            target_compile_options(tinydb_compile_opts INTERFACE "-O0")
        endif()
    else()
        if(MSVC)
            # target_compile_options(tinydb_compile_opts INTERFACE "/Og")
        else()
            target_compile_options(tinydb_compile_opts INTERFACE "-Og")
        endif()
    endif()

    if(tinydb_ENABLE_COVERAGE)
        if(MSVC)
            target_compile_options(tinydb_compile_opts
                                   INTERFACE "/fsanitize-coverage")
        else()
            target_compile_options(tinydb_compile_opts INTERFACE "--coverage")
            target_link_libraries(tinydb_compile_opts INTERFACE "--coverage")
        endif()
    endif()
endmacro()
