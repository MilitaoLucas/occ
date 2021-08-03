#pragma once
#include <occ/core/linear_algebra.h>
#include <occ/core/timings.h>
#include <occ/core/logger.h>
#include <occ/core/util.h>
#include <occ/qm/basisset.h>
#include <string>
#include <vector>
#include <array>
#include <occ/gto/shell_order.h>
#include <gau2grid/gau2grid.h>
#include <fmt/core.h>

namespace occ::gto {

using occ::qm::BasisSet;
const inline char shell_labels[] = "SPDFGHIKMNOQRTUVWXYZ";

inline std::string component_label(int i, int j, int k, int l) {
    std::string label{shell_labels[l]};
    for(int c = 0; c < i; c++) label += "x";
    for(int c = 0; c < j; c++) label += "y";
    for(int c = 0; c < k; c++) label += "z";
    return label;
}

template<bool Cartesian = true>
inline std::vector<std::string> shell_component_labels(int l)
{
    if(l == 0) return {std::string{shell_labels[0]}};
    int i, j, k;

    std::vector<std::string> labels;
    auto f = [&labels](int i, int j, int k, int l) {
        labels.push_back(component_label(i, j, k, l));
    };
    occ::gto::iterate_over_shell<Cartesian>(f, l);
    return labels;
}

struct GTOValues {

    inline void reserve(size_t nbf, size_t npts, int derivative_order)
    {
        phi = Mat(npts, nbf);
        if(derivative_order > 0)
        {
            phi_x = Mat(npts, nbf);
            phi_y = Mat(npts, nbf);
            phi_z = Mat(npts, nbf);
        }
        if(derivative_order > 1)
        {
            phi_xx = Mat(npts, nbf);
            phi_xy = Mat(npts, nbf);
            phi_xz = Mat(npts, nbf);
            phi_yy = Mat(npts, nbf);
            phi_yz = Mat(npts, nbf);
            phi_zz = Mat(npts, nbf);
        }
    }

    inline void set_zero() {
        phi.setZero();
        phi_x.setZero();
        phi_y.setZero();
        phi_z.setZero();
        phi_xx.setZero();
        phi_xy.setZero();
        phi_xz.setZero();
        phi_yy.setZero();
        phi_yz.setZero();
        phi_zz.setZero();
    }
    Mat phi;
    Mat phi_x;
    Mat phi_y;
    Mat phi_z;
    Mat phi_xx;
    Mat phi_xy;
    Mat phi_xz;
    Mat phi_yy;
    Mat phi_yz;
    Mat phi_zz;
};

constexpr unsigned int num_subshells(bool cartesian, unsigned int l)
{
    if (l == 0) return 1;
    if (l == 1) return 3;
    if (cartesian) return (l + 2) * (l + 1) / 2;
    return 2 * l + 1;
}

inline double cartesian_normalization_factor(int l, int m, int n)
{
    int angular_momenta = l + m + n;
    using occ::util::double_factorial;
    return sqrt(double_factorial(angular_momenta) / (double_factorial(l) * double_factorial(m) * double_factorial(n)));
}

struct Momenta {
    int l{0};
    int m{0};
    int n{0};

    std::string to_string() const {
        int am = l + m + n;
        if (am == 0) return std::string(1, shell_labels[0]);

        std::string suffix = "";
        for(int i = 0; i < l; i++) suffix += "x";
        for(int i = 0; i < m; i++) suffix += "y";
        for(int i = 0; i < n; i++) suffix += "z";

        return std::string(1, shell_labels[am]) + suffix;
    }
};

template<bool Cartesian = true>
std::vector<Momenta> subshell_ordering(int l) {
    if(l == 0) return {{0, 0, 0}};
    int i = 0, j = 0, k = 0;
    std::vector<Momenta> powers;
    auto f = [&powers](int i, int j, int k, int l) {
        powers.push_back({i, j, k});
    };
    occ::gto::iterate_over_shell<Cartesian>(f, l);
    return powers;
}

void evaluate_basis(const BasisSet &basis,
                             const std::vector<occ::core::Atom> &atoms,
                             const occ::Mat &grid_pts,
                             GTOValues &gto_values,
                             int max_derivative);


inline GTOValues evaluate_basis(const BasisSet &basis,
                         const std::vector<occ::core::Atom> &atoms,
                         const occ::Mat &grid_pts,
                         int max_derivative)
{
    GTOValues gto_values;
    evaluate_basis(basis, atoms, grid_pts, gto_values, max_derivative);
    return gto_values;
}


template<int angular_momentum>
std::vector<std::array<int, angular_momentum>> cartesian_gaussian_power_index_arrays()
{
    std::vector<std::array<int, angular_momentum>> result;
    int l, m, n;
    auto f = [&result](int l, int m, int n, int LL) {
        std::array<int, angular_momentum> powers;
        int idx = 0;
        for(int i = 0; i < l; i++) {
            powers[idx] = 0;
            idx++;
        }
        for(int i = 0; i < m; i++) {
            powers[idx] = 1;
            idx++;
        }
        for(int i = 0; i < n; i++) {
            powers[idx] = 2;
            idx++;
        }
        result.push_back(powers);
    };
    occ::gto::iterate_over_shell<true>(f, angular_momentum);
    return result;
}


/*
 * Result should be R: an MxM rotation matrix for P: a MxN set of coordinates
 * giving results P' = R P
 */
template<int l>
Mat cartesian_gaussian_rotation_matrix(const occ::Mat3 rotation)
{
    constexpr int num_moments = (l + 1) * (l + 2) / 2;
    Mat result = Mat::Zero(num_moments, num_moments);
    auto cg_powers = cartesian_gaussian_power_index_arrays<l>();
    for(int i = 0; i < num_moments; i++)
    {
        const auto ix = cg_powers[i];
        for(int j = 0; j < num_moments; j++)
        {
            std::array<int, l> jx = cg_powers[j];
            do {
                double tmp{1};
                for(int k = 0; k < ix.size(); k++) {
                    int u = ix[k], v = jx[k];
                    tmp *= rotation(v, u);
                }
                result(j, i) += tmp;
            } while(std::next_permutation(jx.begin(), jx.end()));
        }
    }
    return result;
}
}
