add_library(occ_solvent_static STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/cosmo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/draco.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/parameters.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/smd.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/solvation_correction.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/surface.cpp"
    ${OCC_SOLVENT_INCLUDE_FILES}
)

target_link_libraries(occ_solvent_static
    PUBLIC
    occ_core
    occ_dft_static
)
target_include_directories(occ_solvent_static PUBLIC
    ${OCC_INCLUDE_DIR}
)

target_compile_features(occ_solvent_static PUBLIC cxx_std_20)
if (BUILD_SHARED_LIBS)
    add_library(occ_solvent SHARED $<TARGET_OBJECTS:occ_solvent_static>)
else ()
    add_library(occ_solvent ALIAS occ_solvent_static)
endif ()
