#pragma once
#include "linear_algebra.h"
#include "numgrid.h"

namespace craso::chem {
class Molecule;
}

namespace craso::dft {

using craso::Mat3N;
using craso::MatRM;
using craso::Mat4N;
using craso::Vec;
using craso::IVec;

class DFTGrid {
public:
    DFTGrid(const craso::chem::Molecule& mol);
    const auto& atomic_numbers() const { return m_atomic_numbers; }
    const auto n_atoms() const { return m_atomic_numbers.size(); }
    Mat4N grid_points(size_t idx) const;
    void set_radial_precision(double prec) { m_radial_precision = prec; }
    void set_min_angular_points(size_t n) { m_min_angular = n; }
    void set_max_angular_points(size_t n) { m_max_angular = n; }

private:
    double m_radial_precision{1e-12};
    size_t m_min_angular{86};
    size_t m_max_angular{302};
    Vec m_x;
    Vec m_y;
    Vec m_z;
    IVec m_atomic_numbers;
    Vec m_alpha_max;
    MatRM m_alpha_min;
    IVec m_l_max;
};

}
