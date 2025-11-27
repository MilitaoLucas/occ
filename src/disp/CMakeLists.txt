add_library(occ_disp
    "${CMAKE_CURRENT_SOURCE_DIR}/dftd4.cpp"
    ${OCC_DISP_INCLUDE_FILES}
)

target_link_libraries(occ_disp
    PUBLIC
    occ_core
    dftd4::dftd4
    PRIVATE
    libcint::libcint
    libecpint_static)

target_include_directories(occ_disp PUBLIC
    $<BUILD_INTERFACE:${OCC_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(occ_disp PUBLIC cxx_std_20)
