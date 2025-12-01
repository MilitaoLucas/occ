find_path(QUADMATH_INCLUDE_DIR names quadmath.h)

set(_original_cmake_find_library_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})

if(WIN32)
  set(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
else()
  set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
endif()

find_library(QUADMATH_LIBRARY names quadmath libquadmath)

set(CMAKE_FIND_LIBRARY_SUFFIXES ${_original_cmake_find_library_suffixes})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(QUADMATH DEFAULT_MSG QUADMATH_LIBRARY QUADMATH_INCLUDE_DIR)

if(QUADMATH_FOUND)
  if(NOT TARGET quadmath)
    add_library(quadmath UNKNOWN IMPORTED)
    set_target_properties(quadmath PROPERTIES
      IMPORTED_LOCATION "${QUADMATH_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${QUADMATH_INCLUDE_DIR}")
  endif()
endif()

mark_as_advanced(QUADMATH_INCLUDE_DIR QUADMATH_LIBRARY)
