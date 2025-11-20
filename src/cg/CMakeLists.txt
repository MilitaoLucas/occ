add_library(occ_cg_static STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/distance_partition.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interaction_mapper.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/neighbor_atoms.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/result_types.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/smd_solvation.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/solvation_contribution.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/solvation_types.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/solvent_surface.cpp"
    ${OCC_CG_INCLUDE_FILES}
)

target_link_libraries(occ_cg_static
    PUBLIC
    occ_io_static
    occ_dft_static
    occ_solvent_static
    occ_xtb_static
    occ_interaction_static
    occ_crystal_static
)
target_include_directories(occ_cg_static PUBLIC
    ${OCC_INCLUDE_DIR}
    ${GEMMI_INCLUDE_DIR}
)

target_compile_features(occ_cg_static PUBLIC cxx_std_20)

if (BUILD_SHARED_LIBS)
    add_library(occ_cg SHARED $<TARGET_OBJECTS:occ_cg_static>)
    target_link_libraries(occ_cg PUBLIC
        occ_io_static
        occ_dft_static
        occ_solvent_static
        occ_xtb_static
        occ_interaction_static
        occ_crystal_static
    )
else ()
    add_library(occ_cg ALIAS occ_cg_static)
endif ()
