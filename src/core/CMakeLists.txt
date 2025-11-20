add_library(occ_core_static STATIC
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
    target_link_libraries(occ_core_static
	PUBLIC
	${BLAS_LIBRARIES})
    target_compile_definitions(occ_core_static PUBLIC EIGEN_USE_BLAS)
endif()

if("${USE_OPENMP}")
target_link_libraries(occ_core_static
    PUBLIC
    OpenMP::OpenMP_CXX
)
endif()

target_link_libraries(occ_core_static
    PUBLIC
    TBB::tbb
)

if(NOT ENABLE_HOST_OPT)
    target_compile_definitions(occ_core_static PUBLIC OCC_DISABLE_SIMD)
endif()

target_link_libraries(occ_core_static
    PUBLIC
    Eigen3::Eigen
    fmt::fmt
    LBFGSpp::LBFGSpp
    scn::scn
    spdlog::spdlog
    Threads::Threads
    unordered_dense::unordered_dense
    _subprocess
)
target_include_directories(occ_core_static PUBLIC
    ${OCC_INCLUDE_DIR}
)

target_compile_features(occ_core_static PUBLIC cxx_std_20)

if (BUILD_SHARED_LIBS)
    add_library(occ_core SHARED $<TARGET_OBJECTS:occ_core_static>)
    target_link_libraries(occ_core PUBLIC
        $<$<BOOL:${USE_SYSTEM_BLAS}>:${BLAS_LIBRARIES}>
        $<$<BOOL:${USE_OPENMP}>:OpenMP::OpenMP_CXX>
        TBB::tbb
        Eigen3::Eigen
        fmt::fmt
        LBFGSpp::LBFGSpp
        scn::scn
        spdlog::spdlog
        Threads::Threads
        unordered_dense::unordered_dense
        _subprocess
    )
else ()
    add_library(occ_core ALIAS occ_core_static)
endif ()
