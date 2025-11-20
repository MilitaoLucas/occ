add_library(occ_geometry_static STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/icosphere_mesh.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/index_cache.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/marching_cubes.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/marching_cubes_data.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/morton.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/quickhull.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/volume_grid.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/wulff.cpp"
    ${OCC_GEOMETRY_INCLUDE_FILES}
)

target_link_libraries(occ_geometry_static
    PUBLIC
    occ_core_static
)
target_include_directories(occ_geometry_static PUBLIC
    ${OCC_INCLUDE_DIR}
)

target_compile_features(occ_geometry_static PUBLIC cxx_std_20)

if (BUILD_SHARED_LIBS)
    add_library(occ_geometry SHARED $<TARGET_OBJECTS:occ_geometry_static>)
else ()
    add_library(occ_geometry ALIAS occ_geometry_static)
endif ()
