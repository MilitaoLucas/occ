#include <tonto/core/molecule.h>
#include <tonto/core/util.h>
#include <fmt/ostream.h>
#include "catch.hpp"

using tonto::chem::Molecule;
using tonto::util::all_close;

TEST_CASE("Molecule constructor", "[molecule]")
{
    tonto::Mat3N pos(3, 2);
    tonto::IVec nums(2);
    nums << 1, 1;
    pos << -1.0, 1.0,
            0.0, 0.0,
            1.0, 1.0;
    Molecule m(nums, pos);
    REQUIRE(m.size() == 2);
}


TEST_CASE("Molecule atom properties", "[molecule]")
{
    tonto::Mat3N pos(3, 3);
    tonto::IVec nums(3);
    nums << 8, 1, 1;
    pos << -1.32695761, -0.10593856,  0.01878821,
           -1.93166418,  1.60017351, -0.02171049,
            0.48664409,  0.07959806,  0.00986248;
    Molecule m(nums, pos);

    auto masses = m.atomic_masses();
    tonto::Vec3 expected_masses = {15.994, 1.00794, 1.00794};

    fmt::print("Atomic masses:\n{}\n\n", masses);
    REQUIRE(all_close(masses, expected_masses, 1e-3, 1e-3));

}

TEST_CASE("Molecule centroids", "[molecule]")
{
    tonto::Mat3N pos(3, 3);
    tonto::IVec nums(3);
    nums << 8, 1, 1;
    pos << -1.32695761, -1.93166418, 0.48664409,
           -0.10593856,  1.60017351, 0.07959806,
            0.01878821, -0.02171049, 0.00986248;
    Molecule m(nums, pos);

    
    tonto::Vec3 expected_centroid = {-0.92399257, 0.524611, 0.0023134};
    tonto::Vec3 calc_centroid = m.centroid();
    fmt::print("Calculated centroid:\n{}\n\n", calc_centroid);
    REQUIRE(all_close(expected_centroid, calc_centroid, 1e-05, 1e-05));

    tonto::Vec3 expected_com = {-1.25932093, -0.000102380208, 0.0160229578};
    tonto::Vec3 calc_com = m.center_of_mass();
    fmt::print("Calculated center of mass:\n{}\n\n", calc_com);
    REQUIRE(all_close(expected_com, calc_com, 1e-05, 1e-05));
}
