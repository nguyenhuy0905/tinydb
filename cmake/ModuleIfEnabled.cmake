# Publicly add CXX_MODULES to target if tinydb_ENABLE_MODULE is equivalent to TRUE.
#
#   tinydb_target_module_if_enabled(<target>
#       [HEADER_FILES <files>]
#       [SOURCE_FILES <files>]
#       [MODULE_FILES <files>])
#
# - If tinydb_ENABLE_MODULE is equivalent to TRUE, target MODULE_FILES as public
# CXX_MODULES and SOURCE_FILES as PRIVATE, otherwise target both MODULE_FILES and
# SOURCE_FILES as PRIVATE.
function(tinydb_target_module_if_enabled target)
    cmake_parse_arguments(tinydb "" ""
                          "MODULE_FILES;SOURCE_FILES;HEADER_FILES" ${ARGN})
    if(tinydb_UNPARSED_ARGUMENTS)
        message(
            FATAL_ERROR
                "Unknown arguments passed into tinydb_target_module_if_enabled:
                ${tinydb_UNPARSED_ARGUMENTS}")
    endif()
    target_sources(${target} PUBLIC FILE_SET HEADERS FILES ${tinydb_HEADER_FILES})
    if(tinydb_ENABLE_MODULE)
        target_sources(${target} PUBLIC FILE_SET CXX_MODULES FILES ${tinydb_MODULE_FILES}
            PRIVATE ${tinydb_SOURCE_FILES})
    else()
        target_sources(${target} PRIVATE ${tinydb_MODULE_FILES} ${tinydb_SOURCE_FILES})
    endif()
endfunction()
