# The DFT-D4 reference data and functional parameters are compiled into the
# library so an executable works without OCC_DATA_PATH; a file found on disk
# still takes precedence (see d4_data.cpp / d4.cpp).
set(OCC_D4_SHARE_DIR "${PROJECT_SOURCE_DIR}/share/dftd4")
set(OCC_D4_EMBEDDED_SOURCES)
foreach(_name refdata functionals)
    set(_in "${OCC_D4_SHARE_DIR}/${_name}.json")
    set(_out "${CMAKE_CURRENT_BINARY_DIR}/d4_embedded_${_name}.cpp")
    add_custom_command(
        OUTPUT "${_out}"
        COMMAND ${CMAKE_COMMAND}
            -DINPUT=${_in} -DOUTPUT=${_out} -DSYMBOL=d4_${_name}_json
            -P "${PROJECT_SOURCE_DIR}/cmake/EmbedFile.cmake"
        DEPENDS "${_in}" "${PROJECT_SOURCE_DIR}/cmake/EmbedFile.cmake"
        COMMENT "Embedding share/dftd4/${_name}.json"
        VERBATIM
    )
    list(APPEND OCC_D4_EMBEDDED_SOURCES "${_out}")
endforeach()

add_library(occ_disp_obj
    OBJECT
    "${CMAKE_CURRENT_SOURCE_DIR}/d3.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/d3_data.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/d4.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/d4_data.cpp"
    ${OCC_D4_EMBEDDED_SOURCES}
    ${OCC_DISP_INCLUDE_FILES}
)
add_library(occ_disp $<TARGET_OBJECTS:occ_disp_obj>)
target_include_directories(occ_disp_obj PUBLIC ${OCC_INCLUDE_DIR})
target_link_libraries(occ_disp_obj PUBLIC occ_core nlohmann_json::nlohmann_json)

target_link_libraries(occ_disp PUBLIC occ_core nlohmann_json::nlohmann_json)

target_include_directories(occ_disp PUBLIC ${OCC_INCLUDE_DIR})

target_compile_features(occ_disp PUBLIC cxx_std_20)
