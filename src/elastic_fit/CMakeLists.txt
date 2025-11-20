add_library(occ_elastic_fit_static
    "${CMAKE_CURRENT_SOURCE_DIR}/monkhorst_pack.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/pes.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/potentials.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/elastic_fitting.cpp"
    ${OCC_ELASTIC_FIT_INCLUDE_FILES}
)

target_link_libraries(occ_elastic_fit_static
    PUBLIC
    occ_core_static
    occ_crystal_static
    occ_io_static
)

target_include_directories(occ_elastic_fit_static PUBLIC
    ${OCC_INCLUDE_DIR}
)

target_compile_features(occ_elastic_fit_static PUBLIC cxx_std_20)

if (BUILD_SHARED_LIBS)
    add_library(occ_elastic_fit SHARED $<TARGET_OBJECTS:occ_elastic_fit_static>)
else ()
    add_library(occ_elastic_fit ALIAS occ_elastic_fit_static)
endif ()
