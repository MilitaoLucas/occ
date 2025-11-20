add_library(occ_disp_static
    "${CMAKE_CURRENT_SOURCE_DIR}/dftd4.cpp"
    ${OCC_DISP_INCLUDE_FILES}
)

target_link_libraries(occ_disp_static
    PUBLIC
    occ_core_static
    dftd4::dftd4
)

target_include_directories(occ_disp_static PUBLIC
    ${OCC_INCLUDE_DIR}
)

target_compile_features(occ_disp_static PUBLIC cxx_std_20)

if (BUILD_SHARED_LIBS)
    add_library(occ_disp SHARED $<TARGET_OBJECTS:occ_disp_static>)
    target_link_libraries(occ_disp PUBLIC dftd4::dftd4 occ_core_static)
    target_compile_features(occ_disp PUBLIC cxx_std_20)
    target_include_directories(occ_disp PUBLIC
        ${OCC_INCLUDE_DIR}
    )
else ()
    add_library(occ_disp ALIAS occ_disp_static)
endif ()


