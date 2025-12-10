add_library(occ_core
    "${CMAKE_CURRENT_SOURCE_DIR}/combinations.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/conditioning_orthogonalizer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/data_directory.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/diis.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/dimer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/eem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/eeq.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/elastic_tensor.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/element.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/energy_components.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/fraction.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/gensqrtinv.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/kabsch.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/kalman_estimator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inertia_tensor.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/linear_algebra.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/log.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/meshgrid.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/molecule.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/molecular_axis.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/molecular_symmetry.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/multipole.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/optimize.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/parallel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/point_group.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/progress.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/quasirandom.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/table.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/timings.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/vibration.cpp"
    ${OCC_CORE_INCLUDE_FILES}
)

if("${USE_SYSTEM_BLAS}")
    target_link_libraries(occ_core
	PUBLIC
	${BLAS_LIBRARIES})
    target_compile_definitions(occ_core PUBLIC EIGEN_USE_BLAS)
endif()

if("${USE_OPENMP}")
target_link_libraries(occ_core
    PUBLIC
    OpenMP::OpenMP_CXX
)
endif()
include(${CMAKE_SOURCE_DIR}/cmake/get_all_properties.cmake)

print_target_properties(TBB::tbb)
target_link_libraries(occ_core
    PUBLIC
    $<$<PLATFORM_ID:Windows>:${TBB_PREFIX_PATH}/lib/tbb12.lib>
    $<$<PLATFORM_ID:Linux>:${TBB_PREFIX_PATH}/lib/libtbb12.a>
)
target_include_directories(occ_core PRIVATE ${TBB_PREFIX_PATH}/include)

if(NOT ENABLE_HOST_OPT)
    target_compile_definitions(occ_core PUBLIC OCC_DISABLE_SIMD)
endif()

target_link_libraries(occ_core
    PUBLIC
    Eigen3::Eigen
    libecpint_static
    fmt::fmt
    LBFGSpp::LBFGSpp
    scn::scn
    spdlog::spdlog
    Threads::Threads
    unordered_dense::unordered_dense
    _subprocess
)
get_target_property(LIBECPINT_INCLUDE libecpint_static INCLUDE_DIRECTORIES)
target_include_directories(occ_core PUBLIC
    ${OCC_INCLUDE_DIR}
    ${LIBECPINT_INCLUDE}
)

target_compile_features(occ_core PUBLIC cxx_std_20)

