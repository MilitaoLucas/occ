add_library(occ_sht_static
    "${CMAKE_CURRENT_SOURCE_DIR}/clebsch.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/legendre.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/quadrature.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/sht.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/spherical_harmonics.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/wigner3j.cpp"
    ${OCC_SHT_INCLUDE_FILES}
)
set(sht_targets occ_core_static Eigen3::Eigen)
target_link_libraries(occ_sht_static
    PUBLIC
    ${sht_targets}
)

target_include_directories(occ_sht_static PUBLIC
    ${OCC_INCLUDE_DIR}
)
target_compile_features(occ_sht_static PUBLIC cxx_std_20)
if (BUILD_SHARED_LIBS)
    add_library(occ_sht SHARED $<TARGET_OBJECTS:occ_sht_static>)
    target_link_libraries(occ_sht PUBLIC ${sht_targets})
    target_compile_features(occ_sht PUBLIC cxx_std_20)
else ()
    add_library(occ_sht ALIAS occ_sht_static)
endif ()
