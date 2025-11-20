add_library(occ_isosurface_static STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/curvature.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/deformation_density.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/eeq_esp.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/electric_potential.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/electron_density.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/isosurface.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/orbital_index.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/point_functors.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/promolecule_density.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/stockholder_weight.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/surface_types.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/void.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/volume.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/volume_calculator.cpp"
    ${OCC_ISOSURFACE_INCLUDE_FILES}
)
target_link_libraries(occ_isosurface_static PUBLIC
    occ_core
    occ_qm_static
    occ_dft_static
    occ_crystal_static
    occ_slater_static
    occ_geometry
    occ_gto_static
    Threads::Threads
    unordered_dense::unordered_dense
)

target_include_directories(occ_isosurface_static PUBLIC
    ${OCC_INCLUDE_DIR}
)

target_compile_features(occ_isosurface_static PUBLIC cxx_std_20)

if (BUILD_SHARED_LIBS)
    add_library(occ_isosurface SHARED $<TARGET_OBJECTS:occ_isosurface_static>)
else ()
    add_library(occ_isosurface ALIAS occ_isosurface_static)
endif ()
