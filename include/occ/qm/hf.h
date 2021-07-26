#pragma once
#include <occ/qm/fock.h>
#include <occ/qm/spinorbital.h>
#include <occ/core/energy_components.h>
#include <occ/core/point_charge.h>

namespace occ::hf {

using occ::qm::SpinorbitalKind;
using occ::ints::BasisSet;
using occ::ints::compute_1body_ints;
using occ::ints::compute_1body_ints_deriv;
using occ::ints::Operator;
using occ::ints::shellpair_data_t;
using occ::ints::shellpair_list_t;

/// to use precomputed shell pair data must decide on max precision a priori
const auto max_engine_precision = std::numeric_limits<double>::epsilon() / 1e10;

class HartreeFock {
public:
  HartreeFock(const std::vector<occ::core::Atom> &atoms, const BasisSet &basis);
  const auto &shellpair_list() const { return m_shellpair_list; }
  const auto &shellpair_data() const { return m_shellpair_data; }
  const auto &atoms() const { return m_atoms; }
  const auto &basis() const { return m_basis; }

  int system_charge() const { return m_charge; }
  int num_e() const { return m_num_e; }

  double two_electron_energy_alpha() const { return m_e_alpha; }
  double two_electron_energy_beta() const { return m_e_beta; }
  double two_electron_energy() const { return m_e_alpha + m_e_beta; }
  bool usual_scf_energy() const { return true; }
  void update_scf_energy(occ::core::EnergyComponents &energy, bool incremental) const { return; }
  bool supports_incremental_fock_build() const { return true; }

  void set_system_charge(int charge);
  double nuclear_repulsion_energy() const;

  Mat compute_fock(SpinorbitalKind kind, const Mat &D,
                    double precision = std::numeric_limits<double>::epsilon(),
                    const Mat &Schwarz = Mat()) const;
  std::pair<Mat, Mat> compute_JK(SpinorbitalKind kind, const Mat &D,
                    double precision = std::numeric_limits<double>::epsilon(),
                    const Mat &Schwarz = Mat()) const;
  Mat compute_J(SpinorbitalKind kind, const Mat &D,
                  double precision = std::numeric_limits<double>::epsilon(),
                  const Mat &Schwarz = Mat()) const;

  Mat compute_kinetic_matrix() const;
  Mat compute_overlap_matrix() const;
  Mat compute_nuclear_attraction_matrix() const; 
  Mat compute_point_charge_interaction_matrix(const std::vector<occ::core::PointCharge> &point_charges) const;

  std::vector<Mat> compute_kinetic_energy_derivatives(unsigned derivative) const;
  std::vector<Mat> compute_nuclear_attraction_derivatives(unsigned derivative) const;
  std::vector<Mat> compute_overlap_derivatives(unsigned derivative) const;

  Mat3N nuclear_electric_field_contribution(const Mat3N&) const;
  Mat3N electronic_electric_field_contribution(SpinorbitalKind kind, const Mat&, const Mat3N&) const;
  Vec electronic_electric_potential_contribution(SpinorbitalKind kind, const Mat&, const Mat3N&) const;
  Vec nuclear_electric_potential_contribution(const Mat3N&) const;

  Mat compute_shellblock_norm(const Mat &A) const;

  auto compute_schwarz_ints() const {
    return occ::ints::compute_schwarz_ints<>(m_basis);
  }

  void update_core_hamiltonian(occ::qm::SpinorbitalKind k, const Mat &D, Mat &H) { return; }

  std::vector<Mat> compute_electronic_multipole_matrices(int order, const Vec3 &o = {0.0, 0.0, 0.0}) const;
  std::vector<Vec> compute_nuclear_multipoles(int order, const Vec3 &o = {0.0, 0.0, 0.0}) const;

private:
  int m_charge{0};
  int m_num_e{0};
  std::vector<occ::core::Atom> m_atoms;
  BasisSet m_basis;
  shellpair_list_t m_shellpair_list{}; // shellpair list for OBS
  shellpair_data_t m_shellpair_data{}; // shellpair data for OBS
  occ::ints::FockBuilder m_fockbuilder;
  mutable double m_e_alpha{0};
  mutable double m_e_beta{0};
};

} // namespace occ::hf
