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

# MSVC never finishes optimising ccsd.cpp: the dozens of distinct Eigen tensor
# contractions in it kept cl.exe busy for 4 h 18 min (227 CPU-minutes) at /O2 on
# a 48-core machine before the build was killed, 14 Sep 2026. The other nine CC
# sources compile at /O2 in under a minute each, so only ccsd.cpp is built
# unoptimised there; the CC code is not on any hot path of the downstream MSVC
# consumers.
if(MSVC)
    set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/ccsd.cpp"
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
