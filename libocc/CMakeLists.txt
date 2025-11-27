set(occ_libs
        "occ_io"
        "occ_opt"
        "occ_xdm"
        "occ_core"
        "occ_xtb"
        "occ_qm"
        "occ_dft"
        "occ_disp"
        "occ_dma"
        "occ_driver"
        "occ_solvent"
        "occ_interaction"
        "occ_isosurface"
        "occ_crystal"
        "occ_slater"
        "occ_geometry"
        "occ_gto"
        "occ_sht"
        "occ_descriptors"
        "occ_cg"
        "occ_elastic_fit"
        "_subprocess"
        "libecpint_static"
        "nlohmann_json"
        "gau2grid_static"
        "scn"
        "spdlog"
        "fast_float"
        "unordered_dense"
)

foreach (libname ${occ_libs})
    if (libname MATCHES "^occ_")
        string(REGEX REPLACE "^occ_" "" SHORT_NAME ${libname})
        add_library(occ::${SHORT_NAME} ALIAS ${libname})
    endif ()
    if(TARGET ${libname})
        install(TARGETS ${libname}
            EXPORT occTargets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
    endif()
endforeach ()
#install(EXPORT occTargets
#        FILE occTargets.cmake
#        NAMESPACE occ::
#        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake
#)
