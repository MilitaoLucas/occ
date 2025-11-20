add_library(occ_gto_static STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/density.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/gto.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/rotation.cpp"
    ${OCC_GTO_INCLUDE_FILES}
)

target_link_libraries(occ_gto_static
    PUBLIC
    occ_core_static
    occ_qm_static
    gau2grid_static
)
target_include_directories(occ_gto_static PUBLIC
    ${OCC_INCLUDE_DIR}
    ${GEMMI_INCLUDE_DIR}
)

target_compile_features(occ_gto_static PUBLIC cxx_std_20)


if (BUILD_SHARED_LIBS)
    add_library(occ_gto SHARED $<TARGET_OBJECTS:occ_gto_static>)
    target_link_libraries(occ_gto PUBLIC
        occ_core_static
        occ_qm_static
        gau2grid_static
    )
    target_include_directories(occ_gto PUBLIC
        ${OCC_INCLUDE_DIR}
        ${GEMMI_INCLUDE_DIR}
    )

    target_compile_features(occ_gto PUBLIC cxx_std_20)

else ()
    add_library(occ_gto ALIAS occ_gto_static)
endif ()

