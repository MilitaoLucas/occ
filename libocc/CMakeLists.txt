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
        "occ_main"
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
install_lib_srcs(CLI11::CLI11 INAME "../include/CLI")

install(DIRECTORY "${CMAKE_SOURCE_DIR}/src/3rdparty/gau2grid/gau2grid" DESTINATION include)
install(DIRECTORY "${CMAKE_BINARY_DIR}/_deps/gemmi-src/include/gemmi" DESTINATION include)
# eigen
install(DIRECTORY "${eigen3_SOURCE_DIR}/Eigen" DESTINATION include)
install(DIRECTORY "${eigen3_SOURCE_DIR}/unsupported" DESTINATION include)
# =========================== end lib include installs ===========================
install(DIRECTORY ${OCC_INCLUDE_DIR}/occ DESTINATION include)
