add_library(occ_descriptors_static
    "${CMAKE_CURRENT_SOURCE_DIR}/promolecule_shape.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/steinhardt.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/pdd_amd.cpp"
    ${OCC_DESCRIPTORS_INCLUDE_FILES}
)

target_link_libraries(occ_descriptors_static
    PUBLIC
    occ_sht_static
    occ_crystal_static
    occ_slater_static
)

target_include_directories(occ_descriptors_static PUBLIC
    ${OCC_INCLUDE_DIR}
)

target_compile_features(occ_descriptors_static PUBLIC cxx_std_20)
if (BUILD_SHARED_LIBS)
    add_library(occ_descriptors SHARED $<TARGET_OBJECTS:occ_descriptors_static>)
    target_link_libraries(occ_descriptors PUBLIC
        occ_sht_static
        occ_crystal_static
        occ_slater_static
    )
else ()
    add_library(occ_descriptors ALIAS occ_descriptors_static)
endif ()
