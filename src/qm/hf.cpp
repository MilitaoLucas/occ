#include <occ/core/parallel.h>
#include <occ/qm/hf.h>
#include <occ/qm/fock.h>
#include <occ/qm/property_ints.h>

namespace occ::hf {

void HartreeFock::set_system_charge(int charge)
{
    m_num_e += m_charge;
    m_charge = charge;
    m_num_e -= m_charge;
}


HartreeFock::HartreeFock(const std::vector<occ::core::Atom> &atoms, const BasisSet &basis)
    : m_atoms(atoms), m_basis(basis), m_fockbuilder(basis.max_nprim(), basis.max_l()) 
{
  std::tie(m_shellpair_list, m_shellpair_data) = occ::ints::compute_shellpairs(m_basis);
  for (const auto &a : m_atoms) {
    m_num_e += a.atomic_number;
  }
  m_num_e -= m_charge;
}

double HartreeFock::nuclear_repulsion_energy() const 
{
  double enuc = 0.0;
  for (auto i = 0; i < m_atoms.size(); i++)
    for (auto j = i + 1; j < m_atoms.size(); j++) {
      auto xij = m_atoms[i].x - m_atoms[j].x;
      auto yij = m_atoms[i].y - m_atoms[j].y;
      auto zij = m_atoms[i].z - m_atoms[j].z;
      auto r2 = xij * xij + yij * yij + zij * zij;
      auto r = sqrt(r2);
      enuc += m_atoms[i].atomic_number * m_atoms[j].atomic_number / r;
    }
  return enuc;
}

Mat HartreeFock::compute_fock(SpinorbitalKind kind, const Mat &D,
                              double precision,
                              const Mat &Schwarz) const
{
  if(kind == SpinorbitalKind::General) return m_fockbuilder.compute_fock<SpinorbitalKind::General>(m_basis, m_shellpair_list, m_shellpair_data, D, precision, Schwarz);
  if(kind == SpinorbitalKind::Unrestricted) return m_fockbuilder.compute_fock<SpinorbitalKind::Unrestricted>(m_basis, m_shellpair_list, m_shellpair_data, D, precision, Schwarz);
  return m_fockbuilder.compute_fock<SpinorbitalKind::Restricted>(m_basis, m_shellpair_list, m_shellpair_data, D, precision, Schwarz);
}

std::pair<Mat, Mat> HartreeFock::compute_JK(SpinorbitalKind kind, const Mat &D,
                                            double precision,
                                            const Mat &Schwarz) const
{
  if(kind == SpinorbitalKind::General) return m_fockbuilder.compute_JK<SpinorbitalKind::General>(m_basis, m_shellpair_list, m_shellpair_data, D, precision, Schwarz);
  if(kind == SpinorbitalKind::Unrestricted) return m_fockbuilder.compute_JK<SpinorbitalKind::Unrestricted>(m_basis, m_shellpair_list, m_shellpair_data, D, precision, Schwarz);
  return m_fockbuilder.compute_JK<SpinorbitalKind::Restricted>(m_basis, m_shellpair_list, m_shellpair_data, D, precision, Schwarz);
}

Mat HartreeFock::compute_J(SpinorbitalKind kind, const Mat &D,
                           double precision,
                           const Mat &Schwarz) const
{
  if(kind == SpinorbitalKind::General) return m_fockbuilder.compute_J<SpinorbitalKind::General>(m_basis, m_shellpair_list, m_shellpair_data, D, precision, Schwarz);
  if(kind == SpinorbitalKind::Unrestricted) return m_fockbuilder.compute_J<SpinorbitalKind::Unrestricted>(m_basis, m_shellpair_list, m_shellpair_data, D, precision, Schwarz);
  return m_fockbuilder.compute_J<SpinorbitalKind::Restricted>(m_basis, m_shellpair_list, m_shellpair_data, D, precision, Schwarz);
}


Mat HartreeFock::compute_kinetic_matrix() const
{
    return compute_1body_ints<Operator::kinetic>(m_basis, m_shellpair_list)[0];
}

Mat HartreeFock::compute_overlap_matrix() const
{
    return compute_1body_ints<Operator::overlap>(m_basis, m_shellpair_list)[0];
}

Mat HartreeFock::compute_nuclear_attraction_matrix() const
{
    return compute_1body_ints<Operator::nuclear>(
        m_basis, m_shellpair_list, occ::core::make_point_charges(m_atoms))[0];
}

Mat HartreeFock::compute_point_charge_interaction_matrix(const std::vector<occ::core::PointCharge> &point_charges) const
{
    return compute_1body_ints<Operator::nuclear>(m_basis, m_shellpair_list, point_charges)[0];
}

std::vector<Mat> HartreeFock::compute_kinetic_energy_derivatives(unsigned derivative) const
{
    return compute_1body_ints_deriv<Operator::kinetic>(derivative, m_basis, m_shellpair_list, m_atoms);
}

std::vector<Mat> HartreeFock::compute_nuclear_attraction_derivatives(unsigned derivative) const
{
    return compute_1body_ints_deriv<Operator::nuclear>(derivative, m_basis, m_shellpair_list, m_atoms);
}

std::vector<Mat> HartreeFock::compute_overlap_derivatives(unsigned derivative) const
{
    return compute_1body_ints_deriv<Operator::overlap>(derivative, m_basis, m_shellpair_list, m_atoms);
}


Mat HartreeFock::compute_shellblock_norm(const Mat &A) const {
  return occ::ints::compute_shellblock_norm(m_basis, A);
}

Mat3N HartreeFock::nuclear_electric_field_contribution(const Mat3N &positions) const
{
    Mat3N result = Mat3N::Zero(3, positions.cols());
    for(const auto& atom: m_atoms)
    {
        double Z = atom.atomic_number;
        Vec3 atom_pos{atom.x, atom.y, atom.z};
        auto ab = positions.colwise() - atom_pos;
        auto r = ab.colwise().norm();
        auto r3 = r.array() * r.array() * r.array();
        result.array() += (Z * (ab.array().rowwise() / r3));
    }
    return result;
}



std::vector<Mat> HartreeFock::compute_electronic_multipole_matrices(int order, const Vec3 &center_of_mass) const
{
    std::array<double, 3> c{center_of_mass(0), center_of_mass(1), center_of_mass(2)};
    if(order == 1)
    {
        auto mats = compute_1body_ints<Operator::emultipole1>(m_basis, m_shellpair_list, c);
        return {mats[0], mats[1], mats[2], mats[3]};
    }
    return {};
}


Mat3N HartreeFock::electronic_electric_field_contribution(SpinorbitalKind kind, const Mat& D, const Mat3N &positions) const
{
    constexpr bool use_finite_differences = true;
    if constexpr(use_finite_differences) {
        double delta = 1e-8;
        occ::Mat3N efield_fd(positions.rows(), positions.cols());
        for(size_t i = 0; i < 3; i++) {
            auto pts_delta = positions;
            pts_delta.row(i).array() += delta;
            auto esp_f = electronic_electric_potential_contribution(kind, D, pts_delta);
            pts_delta.row(i).array() -= 2 * delta;
            auto esp_b = electronic_electric_potential_contribution(kind, D, pts_delta);
            efield_fd.row(i) = - (esp_f - esp_b) / (2 * delta);
        }
        return efield_fd;
    }
    else {
        switch(kind)
        {
            case SpinorbitalKind::Restricted:
                return occ::ints::compute_electric_field<SpinorbitalKind::Restricted>(D, m_basis, m_shellpair_list, positions);
            case SpinorbitalKind::Unrestricted:
                return occ::ints::compute_electric_field<SpinorbitalKind::Unrestricted>(D, m_basis, m_shellpair_list, positions);
            case SpinorbitalKind::General:
                return occ::ints::compute_electric_field<SpinorbitalKind::General>(D, m_basis, m_shellpair_list, positions);
        }
    }
}

Vec HartreeFock::electronic_electric_potential_contribution(SpinorbitalKind kind, const Mat &D, const Mat3N &positions) const
{
    switch(kind)
    {
        case SpinorbitalKind::Unrestricted: return occ::ints::compute_electric_potential<SpinorbitalKind::Unrestricted>(D, m_basis, m_shellpair_list, positions);
        case SpinorbitalKind::General: return occ::ints::compute_electric_potential<SpinorbitalKind::General>(D, m_basis, m_shellpair_list, positions);
        default: return occ::ints::compute_electric_potential<SpinorbitalKind::Restricted>(D, m_basis, m_shellpair_list, positions);
    }
}

Vec HartreeFock::nuclear_electric_potential_contribution(const Mat3N &positions) const
{
    Vec result = Vec::Zero(positions.cols());
    for(const auto& atom: m_atoms)
    {
        double Z = atom.atomic_number;
        Vec3 atom_pos{atom.x, atom.y, atom.z};
        auto ab = positions.colwise() - atom_pos;
        auto r = ab.colwise().norm();
        result.array() += Z / r.array();
    }
    return result;
}

} // namespace occ::hf
