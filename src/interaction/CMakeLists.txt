add_library(occ_interaction_static STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/ce_energy_model.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/coulomb.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/disp.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/external_energy_model.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interaction_json.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/lattice_energy.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/pair_energy.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/pair_energy_store.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/pairinteraction.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/pair_potential.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/polarization.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/polarization_partitioning.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/wavefunction_transform.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/wolf.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/xtb_energy_model.cpp"
    ${OCC_INTERACTION_INCLUDE_FILES}
)

target_link_libraries(occ_interaction_static
    PUBLIC
    occ_core_static
    occ_crystal_static
    occ_qm_static
    occ_xdm_static
    occ_xtb_static
)
target_include_directories(occ_interaction_static PUBLIC
    ${OCC_INCLUDE_DIR}
    ${GEMMI_INCLUDE_DIR}
)

target_compile_features(occ_interaction_static PUBLIC cxx_std_20)
if (BUILD_SHARED_LIBS)
    add_library(occ_interaction SHARED $<TARGET_OBJECTS:occ_interaction_static>)
    target_link_libraries(occ_interaction
        PUBLIC
        occ_core_static
        occ_crystal_static
        occ_qm_static
        occ_xdm_static
        occ_xtb_static
    )
    target_include_directories(occ_interaction PUBLIC
        ${OCC_INCLUDE_DIR}
        ${GEMMI_INCLUDE_DIR}
    )
else ()
    add_library(occ_interaction ALIAS occ_interaction_static)
endif ()
