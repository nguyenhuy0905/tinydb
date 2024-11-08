include(../cmake/ModuleIfEnabled.cmake)

add_executable(tinydb_main main.cxx)
tinydb_target_module_if_enabled(
    TARGET
    tinydb_lib
    MODULE_FILES
    lib.cxx
    HEADER_FILES
    lib.hxx
    SOURCE_FILES
    lib.cxx)

include(GenerateExportHeader)
generate_export_header(tinydb_lib BASE_NAME "tinydb")

add_library(tinydb_system_header INTERFACE)

target_include_directories(
    tinydb_lib PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_BINARY_DIR}>
                      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(tinydb_main PRIVATE tinydb_compile_opts tinydb_lib)
