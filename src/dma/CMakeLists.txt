add_library(occ_dma_static STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/add_qlm.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/binomial.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/dma.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/gauss_hermite.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/linear_multipole_shifter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/linear_multipole_calculator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/mult.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/multipole_calculator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/multipole_shifter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/solid_harmonics.cpp"
)

target_link_libraries(occ_dma_static
    PUBLIC
    occ_core_static
    occ_gto_static
    occ_qm_static
    occ_dft_static
    libcint::libcint
)
if (BUILD_SHARED_LIBS)
    add_library(occ_dma SHARED $<TARGET_OBJECTS:occ_dma_static>)
else ()
    add_library(occ_dma ALIAS occ_dma_static)
endif ()
