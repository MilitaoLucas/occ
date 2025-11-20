add_library(occ_slater_static STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/hirshfeld.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/promolecule.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/slaterbasis.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/thakkar_basis.cpp"
    ${OCC_SLATER_INCLUDE_FILES}
)

target_link_libraries(occ_slater_static
    PUBLIC
    occ_core_static
    nlohmann_json::nlohmann_json
    unordered_dense
)

target_include_directories(occ_slater_static PUBLIC
    ${OCC_INCLUDE_DIR}
)

target_compile_features(occ_slater_static PUBLIC cxx_std_20)
if (BUILD_SHARED_LIBS)
    add_library(occ_slater SHARED $<TARGET_OBJECTS:occ_slater_static>)
    target_link_libraries(occ_slater PUBLIC
        occ_core_static
        nlohmann_json::nlohmann_json
        unordered_dense
    )
else ()
    add_library(occ_slater ALIAS occ_slater_static)
endif ()

