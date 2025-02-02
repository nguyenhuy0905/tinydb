# target_sources that:
# - include MODULES if tinydb_ENABLE_MODULE is true.
# - doesn't otherwise.
function(target_module_if_enabled target)
  cmake_parse_arguments(modules "" ""
    "HEADERS;MODULES;SOURCES" ${ARGN})
  if("${modules_HEADERS}")
    target_sources(${target} PUBLIC FILE_SET HEADERS FILES ${modules_HEADERS})
  endif()
  if(tinydb_ENABLE_MODULE)
    target_sources(${target}
      PUBLIC FILE_SET CXX_MODULES FILES
      ${modules_MODULES}
      PRIVATE
      ${modules_SOURCES})
  else()
    target_sources(${target} PRIVATE ${modules_SOURCES})
  endif()

endfunction()
