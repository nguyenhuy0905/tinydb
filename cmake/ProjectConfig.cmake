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

    include(${PROJECT_SOURCE_DIR}/cmake/CheckSanitizerSourceCompile.cmake)
    if(tinydb_ENABLE_ASAN OR tinydb_ENABLE_UBSAN OR tinydb_ENABLE_MSAN OR tinydb_ENABLE_TSAN)
        message(STATUS "Running checks on whether ASan, UBSan, MSan or TSan can be linked")
        tinydb_check_san_compile(tinydb_compile_opts INTERFACE)
    endif()

    if(tinydb_ENABLE_HARDENING)
        include(${PROJECT_SOURCE_DIR}/cmake/AddHardeningFlags.cmake)
        tinydb_add_hardening_flags(tinydb_compile_opts INTERFACE)
    endif()

    if(tinydb_USE_IMPORT_STD)
        target_compile_definitions(tinydb_compile_opts INTERFACE IMPORT_STD)
    endif()
endmacro()
