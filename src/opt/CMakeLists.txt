add_library(occ_opt_static STATIC
    angle_coordinate.cpp
    berny_optimizer.cpp
    bond_coordinate.cpp
    dihedral_coordinate.cpp
    internal_coordinates.cpp
    linear_search.cpp
    pseudoinverse.cpp
)

target_include_directories(occ_opt_static PUBLIC
    ${OCC_INCLUDE_DIR}
)

target_link_libraries(occ_opt_static PUBLIC occ_core_static fmt::fmt Eigen3::Eigen)
if (BUILD_SHARED_LIBS)
    add_library(occ_opt SHARED $<TARGET_OBJECTS:occ_opt_static>)
    target_link_libraries(occ_opt PUBLIC occ_core fmt::fmt Eigen3::Eigen)
else ()
    add_library(occ_opt ALIAS occ_opt_static)
endif ()
