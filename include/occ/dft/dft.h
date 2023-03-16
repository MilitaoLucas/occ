#pragma once
#include <occ/core/energy_components.h>
#include <occ/core/linear_algebra.h>
#include <occ/core/parallel.h>
#include <occ/core/timings.h>
#include <occ/dft/functional.h>
#include <occ/dft/grid.h>
#include <occ/dft/nonlocal_correlation.h>
#include <occ/dft/range_separated_parameters.h>
#include <occ/dft/xc_potential_matrix.h>
#include <occ/gto/density.h>
#include <occ/gto/gto.h>
#include <occ/qm/hf.h>
#include <occ/qm/mo.h>
#include <occ/qm/spinorbital.h>
#include <string>
#include <vector>

namespace occ::dft {
using occ::qm::expectation;
using occ::qm::MolecularOrbitals;
using occ::qm::SpinorbitalKind;

using occ::IVec;
using occ::Mat3N;
using occ::MatN4;
using occ::Vec;

namespace block = occ::qm::block;

std::vector<DensityFunctional> parse_method(const std::string &method_string,
                                            bool polarized = false);

namespace impl {

template <SpinorbitalKind spinorbital_kind, int derivative_order>
void set_params(DensityFunctional::Params &params, const Mat &rho) {
    if constexpr (spinorbital_kind == SpinorbitalKind::Restricted) {
        params.rho.col(0) = rho.col(0);
    } else if constexpr (spinorbital_kind == SpinorbitalKind::Unrestricted) {
        // correct assignment
        params.rho.col(0) = block::a(rho.col(0));
        params.rho.col(1) = block::b(rho.col(0));
    }

    if constexpr (derivative_order > 0) {
        if constexpr (spinorbital_kind == SpinorbitalKind::Restricted) {
            params.sigma.col(0) = (rho.block(0, 1, rho.rows(), 3).array() *
                                   rho.block(0, 1, rho.rows(), 3).array())
                                      .rowwise()
                                      .sum();
        } else if constexpr (spinorbital_kind ==
                             SpinorbitalKind::Unrestricted) {
            const auto rho_a = block::a(rho.array());
            const auto rho_b = block::b(rho.array());
            const auto &dx_rho_a = rho_a.col(1);
            const auto &dy_rho_a = rho_a.col(2);
            const auto &dz_rho_a = rho_a.col(3);
            const auto &dx_rho_b = rho_b.col(1);
            const auto &dy_rho_b = rho_b.col(2);
            const auto &dz_rho_b = rho_b.col(3);
            params.sigma.col(0) =
                dx_rho_a * dx_rho_a + dy_rho_a * dy_rho_a + dz_rho_a * dz_rho_a;
            params.sigma.col(1) =
                dx_rho_a * dx_rho_b + dy_rho_a * dy_rho_b + dz_rho_a * dz_rho_b;
            params.sigma.col(2) =
                dx_rho_b * dx_rho_b + dy_rho_b * dy_rho_b + dz_rho_b * dz_rho_b;
        }
    }
    if constexpr (derivative_order > 1) {
        if constexpr (spinorbital_kind == SpinorbitalKind::Restricted) {
            params.laplacian.col(0) = rho.col(4);
            params.tau.col(0) = rho.col(5);
        } else if constexpr (spinorbital_kind ==
                             SpinorbitalKind::Unrestricted) {
            params.laplacian.col(0) = block::a(rho.col(4));
            params.laplacian.col(1) = block::b(rho.col(4));
            params.tau.col(0) = block::a(rho.col(5));
            params.tau.col(1) = block::b(rho.col(5));
        }
    }
}

} // namespace impl

class DFT {

  public:
    DFT(const std::string &, const AOBasis &, const AtomGridSettings & = {});
    inline const auto &atoms() const { return m_hf.atoms(); }
    inline const auto &aobasis() const { return m_hf.aobasis(); }
    inline auto nbf() const { return m_hf.nbf(); }
    inline Vec3 center_of_mass() const { return m_hf.center_of_mass(); }

    void set_integration_grid(const AtomGridSettings & = {});

    void set_system_charge(int charge) { m_hf.set_system_charge(charge); }
    int system_charge() const { return m_hf.system_charge(); }
    int total_electrons() const { return m_hf.total_electrons(); }
    int active_electrons() const { return m_hf.active_electrons(); }
    inline const auto &frozen_electrons() const {
        return m_hf.frozen_electrons();
    }

    void set_density_fitting_basis(const std::string &density_fitting_basis) {
        m_hf.set_density_fitting_basis(density_fitting_basis);
    }

    double exchange_correlation_energy() const { return m_exc_dft; }
    double exchange_energy_total() const { return m_exchange_energy; }

    bool usual_scf_energy() const { return false; }
    void update_scf_energy(occ::core::EnergyComponents &energy,
                           bool incremental) const {
        if (incremental) {
            energy["electronic.2e"] += m_two_electron_energy;
            energy["electronic"] += m_two_electron_energy;
            energy["electronic.dft_xc"] += exchange_correlation_energy();
        } else {
            energy["electronic"] = energy["electronic.1e"];
            energy["electronic.2e"] = m_two_electron_energy;
            energy["electronic"] += m_two_electron_energy;
            energy["electronic.dft_xc"] = exchange_correlation_energy();
        }

        if (m_nlc_energy != 0.0) {
            energy["electronic.nonlocal_correlation"] = m_nlc_energy;
        }

        energy["total"] = energy["electronic"] + energy["nuclear.repulsion"];
    }
    bool supports_incremental_fock_build() const { return false; }
    inline bool have_effective_core_potentials() const {
        return m_hf.have_effective_core_potentials();
    }

    int density_derivative() const;
    inline double exact_exchange_factor() const {
        return std::accumulate(m_funcs.begin(), m_funcs.end(), 0.0,
                               [&](double a, const auto &v) {
                                   return a + v.exact_exchange_factor();
                               });
    }

    RangeSeparatedParameters range_separated_parameters() const;

    double nuclear_repulsion_energy() const {
        return m_hf.nuclear_repulsion_energy();
    }

    auto compute_kinetic_matrix() const {
        return m_hf.compute_kinetic_matrix();
    }

    auto compute_overlap_matrix() const {
        return m_hf.compute_overlap_matrix();
    }

    auto compute_nuclear_attraction_matrix() const {
        return m_hf.compute_nuclear_attraction_matrix();
    }

    auto compute_effective_core_potential_matrix() const {
        return m_hf.compute_effective_core_potential_matrix();
    }

    auto compute_point_charge_interaction_matrix(
        const std::vector<std::pair<double, std::array<double, 3>>>
            &point_charges) const {
        return m_hf.compute_point_charge_interaction_matrix(point_charges);
    }

    auto compute_schwarz_ints() const { return m_hf.compute_schwarz_ints(); }

    template <unsigned int order = 1>
    inline auto compute_electronic_multipoles(const MolecularOrbitals &mo,
                                              const Vec3 &o = {0.0, 0.0,
                                                               0.0}) const {
        return m_hf.template compute_electronic_multipoles<order>(mo, o);
    }

    template <unsigned int order = 1>
    inline auto compute_nuclear_multipoles(const Vec3 &o = {0.0, 0.0,
                                                            0.0}) const {
        return m_hf.template compute_nuclear_multipoles<order>(o);
    }

    template <unsigned int order = 1>
    inline auto compute_multipoles(const MolecularOrbitals &mo,
                                   const Vec3 &o = {0.0, 0.0, 0.0}) const {
        return m_hf.template compute_multipoles<order>(mo, o);
    }

    template <int derivative_order,
              SpinorbitalKind spinorbital_kind = SpinorbitalKind::Restricted>
    Mat compute_K_dft(const MolecularOrbitals &mo, double precision,
                      const Mat &Schwarz) {
        using occ::parallel::nthreads;
        const auto &basis = m_hf.aobasis();
        const auto &atoms = m_hf.atoms();
        size_t K_rows, K_cols;
        size_t nbf = basis.nbf();
        const auto &D = mo.D;
        std::tie(K_rows, K_cols) =
            occ::qm::matrix_dimensions<spinorbital_kind>(nbf);
        Mat K = Mat::Zero(K_rows, K_cols);
        m_two_electron_energy = 0.0;
        m_exc_dft = 0.0;

        constexpr size_t BLOCKSIZE = 64;
        size_t num_rows_factor = 1;
        if (spinorbital_kind == SpinorbitalKind::Unrestricted)
            num_rows_factor = 2;

        double total_density_a{0.0}, total_density_b{0.0};
        const Mat D2 = 2 * D;
        DensityFunctional::Family family{DensityFunctional::Family::LDA};
        if constexpr (derivative_order == 1) {
            family = DensityFunctional::Family::GGA;
        }
        if constexpr (derivative_order == 2) {
            family = DensityFunctional::Family::MGGA;
        }

        std::vector<Mat> Kt(occ::parallel::nthreads,
                            Mat::Zero(D.rows(), D.cols()));
        std::vector<double> energies(occ::parallel::nthreads, 0.0);
        std::vector<double> alpha_densities(occ::parallel::nthreads, 0.0);
        std::vector<double> beta_densities(occ::parallel::nthreads, 0.0);
        const auto &funcs = m_funcs;
        size_t atom_grid_idx{0};
        for (const auto &atom_grid : m_atom_grids) {
            const auto &atom_pts = atom_grid.points;
            const auto &atom_weights = atom_grid.weights;
            const size_t npt_total = atom_pts.cols();
            const size_t num_blocks = npt_total / BLOCKSIZE + 1;

            auto lambda = [&](int thread_id) {
                Mat rho_storage(num_rows_factor * BLOCKSIZE,
                                occ::density::num_components(derivative_order));
                for (size_t block = 0; block < num_blocks; block++) {
                    if (block % nthreads != thread_id)
                        continue;
                    Eigen::Index l = block * BLOCKSIZE;
                    Eigen::Index u =
                        std::min(npt_total - 1, (block + 1) * BLOCKSIZE);
                    Eigen::Index npt = u - l;
                    if (npt <= 0)
                        continue;
                    Eigen::Ref<Mat> rho = rho_storage.block(
                        0, 0, num_rows_factor * npt, rho_storage.cols());

                    auto &k = Kt[thread_id];
                    const auto &pts_block = atom_pts.middleCols(l, npt);
                    const auto &weights_block = atom_weights.segment(l, npt);
                    auto gto_vals = occ::gto::evaluate_basis(basis, pts_block,
                                                             derivative_order);
                    occ::density::evaluate_density<derivative_order,
                                                   spinorbital_kind>(
                        D2, gto_vals, rho);

                    double max_density_block = rho.col(0).maxCoeff();
                    if constexpr (spinorbital_kind ==
                                  SpinorbitalKind::Restricted) {
                        alpha_densities[thread_id] +=
                            rho.col(0).dot(weights_block);
                    } else if constexpr (spinorbital_kind ==
                                         SpinorbitalKind::Unrestricted) {
                        Vec rho_a_tmp = block::a(rho.col(0));
                        Vec rho_b_tmp = block::b(rho.col(0));
                        double tot_density_a = rho_a_tmp.dot(weights_block);
                        double tot_density_b = rho_b_tmp.dot(weights_block);
                        alpha_densities[thread_id] += tot_density_a;
                        beta_densities[thread_id] += tot_density_b;
                    }
                    // if(max_density_block < m_density_threshold) continue;

                    DensityFunctional::Params params(npt, family,
                                                     spinorbital_kind);
                    impl::set_params<spinorbital_kind, derivative_order>(params,
                                                                         rho);

                    DensityFunctional::Result res(npt, family,
                                                  spinorbital_kind);
                    for (const auto &func : funcs) {
                        res += func.evaluate(params);
                    }

                    Mat KK = Mat::Zero(k.rows(), k.cols());

                    // Weight the arrays by the grid weights
                    res.weight_by(weights_block);
                    xc_potential_matrix<spinorbital_kind, derivative_order>(
                        res, rho, gto_vals, KK, energies[thread_id]);
                    k.noalias() += KK;
                }
            };
            occ::timing::start(occ::timing::category::dft_xc);
            occ::parallel::parallel_do(lambda);
            occ::timing::stop(occ::timing::category::dft_xc);
            atom_grid_idx++;
        }
        for (size_t i = 0; i < nthreads; i++) {
            K += Kt[i];
            m_exc_dft += energies[i];
            total_density_a += alpha_densities[i];
            total_density_b += beta_densities[i];
        }
        occ::log::debug("Total density: alpha = {} beta = {}", total_density_a,
                        total_density_b);

        if (have_nonlocal_correlation()) {
            auto nlc_result = m_nlc(basis, mo);
            occ::log::debug("NLC energy = {}", nlc_result.energy);
            m_nlc_energy = nlc_result.energy;
            m_exc_dft += nlc_result.energy;
            occ::log::debug("NLC Vxc = {} {}", nlc_result.Vxc.rows(),
                            nlc_result.Vxc.cols());
            K.noalias() += nlc_result.Vxc;
        }

        return K;
    }

    inline Mat
    compute_J(const MolecularOrbitals &mo,
              double precision = std::numeric_limits<double>::epsilon(),
              const Mat &Schwarz = Mat()) const {
        return m_hf.compute_J(mo, precision, Schwarz);
    }

    inline Mat
    compute_vxc(const MolecularOrbitals &mo,
                double precision = std::numeric_limits<double>::epsilon(),
                const Mat &Schwarz = Mat()) {
        int deriv = density_derivative();
        switch (mo.kind) {
        case SpinorbitalKind::Unrestricted: {
            occ::log::debug("Unrestricted vxc evaluation");
            switch (deriv) {
            case 0:
                return compute_K_dft<0, SpinorbitalKind::Unrestricted>(
                    mo, precision, Schwarz);
            case 1:
                return compute_K_dft<1, SpinorbitalKind::Unrestricted>(
                    mo, precision, Schwarz);
            case 2:
                return compute_K_dft<2, SpinorbitalKind::Unrestricted>(
                    mo, precision, Schwarz);
            default:
                throw std::runtime_error(
                    "Not implemented: DFT for derivative order > 2");
            }
        }
        case SpinorbitalKind::Restricted: {
            switch (deriv) {
            case 0:
                return compute_K_dft<0, SpinorbitalKind::Restricted>(
                    mo, precision, Schwarz);
            case 1:
                return compute_K_dft<1, SpinorbitalKind::Restricted>(
                    mo, precision, Schwarz);
            case 2:
                return compute_K_dft<2, SpinorbitalKind::Restricted>(
                    mo, precision, Schwarz);
            default:
                throw std::runtime_error(
                    "Not implemented: DFT for derivative order > 2");
            }
        }
        default:
            throw std::runtime_error(
                "Not implemented: DFT for General spinorbitals");
        }
    }

    inline std::pair<Mat, Mat>
    compute_JK(const MolecularOrbitals &mo,
               double precision = std::numeric_limits<double>::epsilon(),
               const Mat &Schwarz = Mat()) {

        Mat J;
        Mat K = -compute_vxc(mo, precision, Schwarz);
        double ecoul{0.0}, exc{0.0};
        double exchange_factor = exact_exchange_factor();
        RangeSeparatedParameters rs = range_separated_parameters();
        if (rs.omega != 0.0) {
            // range separated hybrid
            J = m_hf.compute_J(mo, precision, Schwarz);
            Mat Ksr;
            std::tie(J, Ksr) = m_hf.compute_JK(mo, precision, Schwarz);
            m_hf.set_range_separated_omega(rs.omega);
            Mat Jlr, Klr;
            std::tie(Jlr, Klr) = m_hf.compute_JK(mo, precision, Schwarz);
            Mat Khf = Ksr * (rs.alpha + rs.beta) + Klr * (-rs.beta);
            ecoul = expectation(mo.kind, mo.D, J);
            exc = -expectation(mo.kind, mo.D, Khf);
            K.noalias() += Khf;
            m_hf.set_range_separated_omega(0.0);
        } else if (exchange_factor != 0.0) {
            // global hybrid
            Mat Khf;
            std::tie(J, Khf) = m_hf.compute_JK(mo, precision, Schwarz);
            ecoul = expectation(mo.kind, mo.D, J);
            exc = -expectation(mo.kind, mo.D, Khf) * exchange_factor;
            K.noalias() += Khf * exchange_factor;
        } else {
            J = m_hf.compute_J(mo, precision, Schwarz);
            ecoul = expectation(mo.kind, mo.D, J);
        }
        occ::log::debug("EXC_dft = {}, EXC = {}, E_coul = {}\n", m_exc_dft, exc,
                        ecoul);
        m_exchange_energy = m_exc_dft + exc;
        m_two_electron_energy += m_exchange_energy + ecoul;
        return {J, K};
    }

    inline bool have_nonlocal_correlation() const {
        for (const auto &func : m_funcs) {
            if (func.needs_nlc_correction()) {
                return true;
            }
        }
        return false;
    }

    Mat compute_fock(const MolecularOrbitals &mo,
                     double precision = std::numeric_limits<double>::epsilon(),
                     const Mat &Schwarz = Mat()) {
        auto [J, K] = compute_JK(mo, precision, Schwarz);
        return J - K;
    }

    const auto &hf() const { return m_hf; }

    inline Mat compute_fock_mixed_basis(const MolecularOrbitals &mo_bs,
                                        const qm::AOBasis &bs,
                                        bool is_shell_diagonal) {
        return m_hf.compute_fock_mixed_basis(mo_bs, bs, is_shell_diagonal);
    }

    inline Vec
    electronic_electric_potential_contribution(const MolecularOrbitals &mo,
                                               const Mat3N &pts) const {
        return m_hf.electronic_electric_potential_contribution(mo, pts);
    }

    inline Vec nuclear_electric_potential_contribution(const Mat3N &pts) const {
        return m_hf.nuclear_electric_potential_contribution(pts);
    }

    inline Mat3N nuclear_electric_field_contribution(const Mat3N &pts) const {
        return m_hf.nuclear_electric_field_contribution(pts);
    }

    inline Mat3N
    electronic_electric_field_contribution(const MolecularOrbitals &mo,
                                           const Mat3N &pts) const {
        return m_hf.electronic_electric_field_contribution(mo, pts);
    }

    void update_core_hamiltonian(const MolecularOrbitals &mo, Mat &H) {
        return;
    }

    void set_method(const std::string &method_string,
                    bool unrestricted = false);

    void set_unrestricted(bool unrestricted);

  private:
    std::string m_method_string{"svwn5"};
    occ::qm::HartreeFock m_hf;
    MolecularGrid m_grid;
    std::vector<DensityFunctional> m_funcs;
    std::vector<AtomGrid> m_atom_grids;
    NonLocalCorrelationFunctional m_nlc;
    mutable double m_two_electron_energy{0.0};
    mutable double m_exc_dft{0.0};
    mutable double m_exchange_energy{0.0};
    mutable double m_nlc_energy{0.0};
    double m_density_threshold{1e-10};
    RangeSeparatedParameters m_rs_params;
};
} // namespace occ::dft
