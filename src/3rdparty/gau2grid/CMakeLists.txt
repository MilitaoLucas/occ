add_library(gau2grid_static
    STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/gau2grid_deriv1.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/gau2grid_deriv2.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/gau2grid_deriv3.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/gau2grid_helper.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/gau2grid_orbital.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/gau2grid_phi.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/gau2grid_transform.c"
)

target_include_directories(gau2grid_static
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/occ/3rdparty/gau2grid>
)

if("${GG_NO_PRAGMA}")
  target_compile_definitions(gau2grid_static PRIVATE $<BUILD_INTERFACE:__GG_NO_PRAGMA> )
endif()
