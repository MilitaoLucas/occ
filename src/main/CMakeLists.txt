add_library(occ_main SHARED
    "${CMAKE_CURRENT_SOURCE_DIR}/cli_validators.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/crystal_surface_energy.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/monomer_wavefunctions.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/point_functors.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/properties.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/occ_elastic.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/occ_isosurface.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/occ_cg.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/occ_cube.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/occ_dimers.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/occ_describe.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/occ_dma.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/occ_elat.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/occ_elastic_fit.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/occ_embed.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/occ_pair.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/occ_scf.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/occ_surface_cuts.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/version.cpp"
    ${OCC_MAIN_INCLUDE_FILES}
)

target_link_libraries(occ_main
    PUBLIC
    occ_core
    occ_xtb
    occ_qm
    occ_dft
    occ_disp
    occ_dma
    occ_driver
    occ_solvent
    occ_interaction
    occ_isosurface
    occ_crystal
    occ_slater
    occ_geometry
    occ_gto
    occ_sht
    occ_descriptors
    occ_cg
    occ_elastic_fit
    CLI11::CLI11
)
target_include_directories(occ_main PUBLIC
    ${OCC_INCLUDE_DIR}
    ${GEMMI_INCLUDE_DIR}
)

target_compile_features(occ_main PUBLIC cxx_std_20)

add_executable(xdm EXCLUDE_FROM_ALL
    "${CMAKE_CURRENT_SOURCE_DIR}/xdm.cpp")
target_link_libraries(xdm
    PUBLIC
    occ_main
    occ_xdm
)

set_target_properties(xdm
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_executable(occ "${CMAKE_CURRENT_SOURCE_DIR}/occ.cpp")
target_link_libraries(occ
    PUBLIC
    occ_main
)
set_target_properties(occ
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Use for GCC time report when compiling occ
# target_compile_options(occ PRIVATE "-ftime-report")

add_executable(make_radial_interpolators EXCLUDE_FROM_ALL
    "${CMAKE_CURRENT_SOURCE_DIR}/make_radial_interpolators.cpp")
target_link_libraries(make_radial_interpolators PUBLIC occ_slater)
set_target_properties(make_radial_interpolators
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_executable(occ-mlx EXCLUDE_FROM_ALL
    "${CMAKE_CURRENT_SOURCE_DIR}/mlx.cpp")
target_link_libraries(occ-mlx
    PUBLIC
    mlx
    Eigen3::Eigen
)
target_compile_features(occ-mlx PUBLIC cxx_std_20)

add_executable(make_atomic_pair_potentials EXCLUDE_FROM_ALL
    "${CMAKE_CURRENT_SOURCE_DIR}/make_atomic_pair_potentials.cpp")
target_link_libraries(make_atomic_pair_potentials
    PUBLIC
    occ_main
)
target_compile_features(make_atomic_pair_potentials PUBLIC cxx_std_20)
set_target_properties(make_atomic_pair_potentials
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
if (WITH_LIBOCC)
    include(GNUInstallDirs)
    include(CMakePrintHelpers)
    include(InstallRequiredSystemLibraries)
    include(${CMAKE_SOURCE_DIR}/cmake/get_all_properties.cmake)
    if ("${SHARED_LIBOCC}")
        add_library(libocc SHARED $<TARGET_OBJECTS:occ_main>)
    else ()
        add_library(libocc STATIC $<TARGET_OBJECTS:occ_main>)
    endif ()
    target_link_libraries(libocc PRIVATE
        occ_core
        occ_xtb
        occ_qm
        occ_dft
        occ_disp
        occ_dma
        occ_driver
        occ_solvent
        occ_interaction
        occ_isosurface
        occ_crystal
        occ_slater
        occ_geometry
        occ_gto
        occ_sht
        occ_descriptors
        occ_cg
        occ_elastic_fit
        CLI11::CLI11
    )
    # ============================= lib include installs =============================
    function(install_lib_srcs lib)
        set(oneValueArgs INAME ITDIR)
        set(options BUILD_INCLUDE ISFILE NONAMEDIR)

        cmake_parse_arguments(
            INST_OPT
            "${options}"
            "${oneValueArgs}"
            ""
           ${ARGN}
        )
        if(NOT TARGET ${lib})
          message(STATUS "There is no target named '${lib}'")
          return()
        endif()
        get_target_property(target_HEADERS ${lib} PUBLIC_HEADER)
        get_target_property(target_SOURCE_DIR ${lib} SOURCE_DIR)
        get_target_property(target_name ${lib} NAME)
        if(INST_OPT_BUILD_INCLUDE)
            get_target_property(target_SOURCE_DIR ${lib} BINARY_DIR)
        endif ()
        if (NOT INST_OPT_INAME)
            set(INST_OPT_INAME include/${target_name})
            if (INST_OPT_NONAMEDIR)
                set(INST_OPT_INAME include)
            endif ()
        endif ()
        if (NOT INST_OPT_ITDIR)
            set(INST_OPT_ITDIR "include")
        endif ()
        if (target_HEADERS STREQUAL "target_HEADERS-NOTFOUND")
            message(WARNING ${target_SOURCE_DIR}/${INST_OPT_INAME})

            message(WARNING ${INST_OPT_ITDIR})
            if (INST_OPT_ISFILE)
                install(DIRECTORY "${target_SOURCE_DIR}/${INST_OPT_INAME}"
                        DESTINATION ${INST_OPT_ITDIR} FILES_MATCHING
                        PATTERN
                            "*.h"
                        PATTERN
                            "*.hpp"

                )
            else ()
                install(DIRECTORY ${target_SOURCE_DIR}/${INST_OPT_INAME} DESTINATION ${INST_OPT_ITDIR})
            endif ()
            return()
        endif ()
        list(TRANSFORM target_HEADERS PREPEND "${target_SOURCE_DIR}/")
        install(FILES ${target_HEADERS} DESTINATION include/${target_name})
    endfunction()
    install_lib_srcs(fmt::fmt)

    install_lib_srcs(unordered_dense INAME "include/ankerl" ITDIR "include")
    install_lib_srcs(nlohmann_json::nlohmann_json INAME "include/nlohmann" ITDIR "include")
    install_lib_srcs(spdlog::spdlog)
    install_lib_srcs(TBB::tbb INAME "../../include/tbb")
    install_lib_srcs(TBB::tbb INAME "../../include/oneapi")
    install_lib_srcs(cint BUILD_INCLUDE ISFILE NONAMEDIR ITDIR "INCLUDE/../")
    install_lib_srcs(cint ISFILE NONAMEDIR ITDIR "INCLUDE/../")

#    print_target_properties(TBB::tbb)
    # unordered_dense

#    get_target_property(unordered_HEADERS t PUBLIC_HEADER)
#    get_target_property(fmt_SOURCE_DIR fmt::fmt SOURCE_DIR)
    # eigen
    install(DIRECTORY "${eigen3_SOURCE_DIR}/Eigen" DESTINATION include)
    install(DIRECTORY "${eigen3_SOURCE_DIR}/unsupported" DESTINATION include)
    # =========================== end lib include installs ===========================

#    Eigen3::Eigen
#    fmt::fmt
#    LBFGSpp::LBFGSpp
#    scn::scn
#    spdlog::spdlog
#    Threads::Threads
#    unordered_dense::unordered_dense
#    _subprocess
    install(DIRECTORY ${OCC_INCLUDE_DIR}/occ DESTINATION include)
    target_include_directories(libocc PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>
    )
    set_target_properties(libocc PROPERTIES
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )

    # Set proper target properties for export
    set_target_properties(libocc PROPERTIES
        EXPORT_NAME libocc
    )

    # Export to package registry for development
    install(TARGETS libocc
        EXPORT occTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    cmake_print_variables(CMAKE_INSTALL_INCLUDEDIR)
    cmake_print_variables(CMAKE_INSTALL_LIBDIR)

    include(CMakePackageConfigHelpers)

    configure_package_config_file(
        "${CMAKE_SOURCE_DIR}/cmake/occ-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/occ-config.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}/cmake/occ
        PATH_VARS
            CMAKE_INSTALL_INCLUDEDIR
            CMAKE_INSTALL_LIBDIR
    )

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/occConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/occ-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/occConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_PREFIX}/cmake/occ
    )

    install(EXPORT occTargets
        FILE occTargets.cmake
        NAMESPACE occ::
        DESTINATION ${CMAKE_INSTALL_PREFIX}/cmake/occ
    )
endif ()
