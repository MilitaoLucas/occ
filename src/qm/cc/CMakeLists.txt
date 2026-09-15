add_library(
    occ_cc_obj
    OBJECT
    "${CMAKE_CURRENT_SOURCE_DIR}/thc.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/laplace.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/thc_mp2.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/integrals.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ccsd.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/triples.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/uccsd_so.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/uintegrals.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utriples.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/uccsd.cpp"
    ${OCC_CC_INCLUDE_FILES}
)

# MSVC's optimiser does not cope with the chained Eigen tensor contractions of
# the unrestricted CC sources: at /O2 uccsd.cpp takes 6 min, uccsd_so.cpp 11 min
# and utriples.cpp 17 min (MSVC 14.44, 15 Sep 2026), a serial tail behind an
# otherwise finished build. ccsd.cpp was staged so it compiles in under a
# minute (9bde072f7); until the same is done for these three they are built
# unoptimised there. No downstream MSVC consumer runs the CC code, it is only in
# occ_main's link closure.
if(MSVC)
    set_source_files_properties(
        "${CMAKE_CURRENT_SOURCE_DIR}/uccsd.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/uccsd_so.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/utriples.cpp"
        PROPERTIES COMPILE_OPTIONS "/Od;/Ob0")
endif()

add_library(occ_cc $<TARGET_OBJECTS:occ_cc_obj>)
target_include_directories(occ_cc_obj PUBLIC ${OCC_INCLUDE_DIR} ${GEMMI_INCLUDE_DIR})
target_link_libraries(occ_cc_obj PUBLIC occ_qm occ_correlation occ_numint occ_ints)
target_link_libraries(occ_cc PUBLIC occ_qm occ_correlation occ_numint occ_ints)
target_include_directories(
    occ_cc
    PUBLIC ${OCC_INCLUDE_DIR} ${GEMMI_INCLUDE_DIR}
)

target_compile_features(occ_cc PUBLIC cxx_std_20)
