add_library(occ_dft
    "${CMAKE_CURRENT_SOURCE_DIR}/dft.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/dft_kernels.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/dft_gradient_kernels.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/dft_gradients.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/dft_method.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/functional.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/hirshfeld.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/voronoi_charges.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/nonlocal_correlation.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/xc_potential_matrix.cpp"
    ${OCC_DFT_INCLUDE_FILES}
)

target_link_libraries(occ_dft
    PUBLIC
    occ_core
    occ_qm
    occ_io
    occ_numint
    Libxc::xc
    PRIVATE
    libcint::libcint
    libecpint_static
)
target_include_directories(occ_dft PUBLIC
    $<BUILD_INTERFACE:${OCC_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
    ${GEMMI_INCLUDE_DIR}
)

target_compile_features(occ_dft PUBLIC cxx_std_20)

