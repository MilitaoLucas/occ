add_library(occ_driver_static STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/geometry_optimization.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/crystal_growth.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/single_point.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/dma_driver.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/vibrational_analysis.cpp"
    ${OCC_DRIVER_INCLUDE_FILES}
)

target_link_libraries(occ_driver_static
    PUBLIC
    occ_io_static
    occ_interaction
    occ_xtb_static
    occ_dft_static
    occ_qm_static
    occ_dma_static
    occ_solvent_static
    occ_crystal_static
    occ_cg_static
    occ_opt_static
)
target_include_directories(occ_driver_static PUBLIC
    ${OCC_INCLUDE_DIR}
    ${GEMMI_INCLUDE_DIR}
)

target_compile_features(occ_driver_static PUBLIC cxx_std_20)
if (BUILD_SHARED_LIBS)
    add_library(occ_driver SHARED $<TARGET_OBJECTS:occ_driver_static>)
else ()
    add_library(occ_driver ALIAS occ_driver_static)
endif ()
