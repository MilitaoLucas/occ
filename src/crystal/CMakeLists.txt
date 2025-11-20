add_library(occ_crystal_static STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/asymmetric_unit.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/crystal.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/dimer_labeller.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/dimer_mapping_table.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/hkl.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/muldin.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/site_mapping_table.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/spacegroup.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/standard_bonds.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/surface.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/symmetryoperation.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/unitcell.cpp"
    ${occ_INCLUDE_FILES}
)

target_link_libraries(occ_crystal_static
    PUBLIC
    occ_core_static
    gemmi::gemmi
)
target_include_directories(occ_crystal_static PUBLIC
    ${OCC_INCLUDE_DIR}
)

target_compile_features(occ_crystal_static PUBLIC cxx_std_20)
if (BUILD_SHARED_LIBS)
    add_library(occ_crystal SHARED $<TARGET_OBJECTS:occ_crystal_static>)

    target_link_libraries(occ_crystal
        PUBLIC
        occ_core
        gemmi::gemmi
    )
    target_include_directories(occ_crystal PUBLIC
        ${OCC_INCLUDE_DIR}
    )
else ()
    add_library(occ_crystal ALIAS occ_crystal_static)
endif ()
