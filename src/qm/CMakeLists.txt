add_library(occ_qm_static STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/cdiis.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/chelpg.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/cint_interface.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ediis.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/gradients.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/guess_density.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/hf.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/integral_engine_2c.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/integral_engine_3c.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/integral_engine_4c.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/integral_engine_grad1.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/integral_engine_grad2.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/integral_engine_df.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/merge.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/mo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/mo_integral_engine.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/mp2.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/oniom.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/orbital_smearing.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/orthogonalizer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/partitioning.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/post_hf_method.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/shell.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/scf.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/scf_method.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/sto3g_basis.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/wavefunction.cpp"
    ${OCC_QM_INCLUDE_FILES}
)

target_link_libraries(occ_qm_static PUBLIC
    occ_core_static
    occ_io_static
    occ_gto_static
    occ_disp_static
    occ_xdm_static
    libcint::libcint
    libecpint_static
)
target_include_directories(occ_qm_static PUBLIC
    ${OCC_INCLUDE_DIR}
    ${GEMMI_INCLUDE_DIR}
)

target_compile_definitions(occ_qm_static PUBLIC HAVE_ECPINT=1)

target_compile_features(occ_qm_static PUBLIC cxx_std_20)
if (BUILD_SHARED_LIBS)
    add_library(occ_qm SHARED $<TARGET_OBJECTS:occ_qm_static>)
    target_link_libraries(occ_qm PUBLIC
        occ_core_static
        occ_io_static
        occ_gto_static
        occ_disp_static
        occ_xdm_static
        libcint::libcint
        libecpint_static
    )
    target_include_directories(occ_qm PUBLIC
        ${OCC_INCLUDE_DIR}
        ${GEMMI_INCLUDE_DIR}
    )
    target_compile_definitions(occ_qm PUBLIC HAVE_ECPINT=1)

    target_include_directories(occ_qm PUBLIC
        ${OCC_INCLUDE_DIR}
        ${GEMMI_INCLUDE_DIR}
    )
else ()
    add_library(occ_qm ALIAS occ_qm_static)
endif ()

