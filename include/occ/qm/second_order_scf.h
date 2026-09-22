#pragma once
#include <Eigen/Eigenvalues>
#include <algorithm>
#include <cmath>
#include <limits>
#include <occ/core/linear_algebra.h>
#include <occ/core/log.h>
#include <occ/qm/mo.h>
#include <occ/qm/orbital_smearing.h>
#include <occ/qm/scf_method.h>
#include <vector>

namespace occ::qm {

template <SCFMethod Procedure> struct SCF;

/// Knobs for the trust-region augmented-Hessian (TRAH) second-order step.
struct SecondOrderSettings {
  /// Whether the SCF may hand over to a second-order step at all.
  bool enabled{true};
  /// Hand over as soon as the SCF reaches the quadratic region rather than
  /// waiting for DIIS to stall.
  bool request_early{false};
  /// With `request_early`, the commutator error the DIIS stage has to reach.
  double start_threshold{1e-2};
  /// Iterations DIIS is given to halve the commutator error before the
  /// second-order step takes over; the second value applies when the step was
  /// requested, where DIIS only has to get into the quadratic region.
  int patience{30};
  int patience_requested{8};
  /// Hessian-vector products per macro iteration.
  int micro_max{30};
  double trust_max{1.0};
  double trust_first{0.5};
  /// Energy rise still attributable to the noise of a Fock build.
  double noise{1e-8};
  /// Largest element of the density perturbation used to difference a Fock
  /// build that is not linear in the density.
  double fd_step{1e-3};
};

/// Trust-region augmented-Hessian SCF: the second-order step an SCF falls back
/// on when DIIS stalls, and the one it can be asked to take as soon as the
/// orbitals are in the quadratic region.
///
/// Parametrises the orbitals as C exp(kappa) with kappa in the occupied-virtual
/// blocks, and minimises the energy over kappa with the augmented Hessian
/// [[0, a g^T], [a g, H]] built in a Davidson subspace, `a` raised by bisection
/// until the step fits the trust radius. The radius follows the ratio of the
/// actual to the predicted decrease and a step that raises the energy is
/// rejected outright, so the sequence is monotonic whether or not the Hessian
/// is exact.
///
/// It is exact for a Fock build linear in the density (Hartree-Fock): the
/// Hessian-vector product then needs one G(dD) build. For a build that is not
/// linear - DFT's XC quadrature, COSX exchange, implicit solvation - the same
/// product is obtained by differencing the build along dD, which is the same
/// one build per product and correct for any functional, at the cost of the
/// truncation error of a forward difference. The trust region absorbs that:
/// steps are accepted on the actual energy, so an inexact Hessian costs
/// iterations, not correctness.
template <SCFMethod Procedure> class SecondOrderSCF {
public:
  SecondOrderSettings settings;

  inline bool active() const { return m_active; }

  void reset();

  /// Call once per iteration, after `diis_error` is known. Returns true on the
  /// iteration the second-order step takes over - the caller owes it a Fock
  /// matrix rebuilt at the current orbitals before the first `macro_step`.
  bool consider(const SCF<Procedure> &scf);

  /// One macro iteration: judge the step that produced these orbitals, then
  /// solve for and take the next one. `energy` is the total energy at the
  /// current orbitals, with `scf.ctx.F` built from them.
  void macro_step(SCF<Procedure> &scf, double energy);

private:
  Vec gradient(const SCF<Procedure> &scf, Vec &diagonal_hessian) const;
  void rotate(SCF<Procedure> &scf, const Mat &C_from, const Vec &kappa) const;
  Mat fock_response(SCF<Procedure> &scf, const Mat &dD);
  Vec hessian_vector(SCF<Procedure> &scf, const Vec &v);
  void solve(SCF<Procedure> &scf, bool extend);

  static inline int spin_blocks(const MolecularOrbitals &mo) {
    return mo.kind == SpinorbitalKind::Unrestricted ? 2 : 1;
  }
  static inline Eigen::Index occupied(const MolecularOrbitals &mo, int block) {
    return static_cast<Eigen::Index>(block == 0 ? mo.n_alpha : mo.n_beta);
  }

  bool m_active{false};
  bool m_linear{true};
  int m_patience_iter{0};
  double m_patience_error{0.0};
  double m_trust{0.0};
  double m_energy{std::numeric_limits<double>::infinity()};
  double m_predicted{0.0};
  bool m_boundary{false};
  int m_micro_total{0};
  Vec m_kappa, m_gradient, m_diagonal_hessian;
  Mat m_C;
  /// The two-electron part at `m_C`, kept only for the differenced response.
  Mat m_G;
  std::vector<Vec> m_B, m_HB;
};

template <SCFMethod P> void SecondOrderSCF<P>::reset() {
  m_active = false;
  m_patience_iter = 0;
  m_patience_error = 0.0;
  m_trust = settings.trust_first;
  m_energy = std::numeric_limits<double>::infinity();
  m_predicted = 0.0;
  m_boundary = false;
  m_micro_total = 0;
  m_kappa.resize(0);
  m_gradient.resize(0);
  m_diagonal_hessian.resize(0);
  m_C.resize(0, 0);
  m_G.resize(0, 0);
  m_B.clear();
  m_HB.clear();
}

template <SCFMethod P> bool SecondOrderSCF<P>::consider(const SCF<P> &scf) {
  if (m_active || !settings.enabled)
    return false;
  if (scf.iter == 1 || scf.diis_error < 0.5 * m_patience_error) {
    m_patience_error = scf.diis_error;
    m_patience_iter = scf.iter;
  }
  const int patience =
      settings.request_early ? settings.patience_requested : settings.patience;
  const bool stuck = scf.iter - m_patience_iter >= patience;
  if (!stuck &&
      !(settings.request_early && scf.diis_error < settings.start_threshold))
    return false;

  // The parametrisation is a rotation of occupied into virtual orbitals, so it
  // needs both, integer occupations and a spin structure it can address block
  // by block. Anything else keeps DIIS, and asks once.
  const MolecularOrbitals &mo = scf.ctx.mo;
  const char *unsupported =
      mo.kind == SpinorbitalKind::General  ? "general spinorbitals"
      : mo.smearing.kind != OrbitalSmearing::Kind::None
          ? "smeared occupations"
          : (mo.C.cols() <= static_cast<Eigen::Index>(
                                std::max(mo.n_alpha, mo.n_beta))
                 ? "no virtual orbitals"
                 : nullptr);
  if (unsupported) {
    log::debug("second-order SCF unavailable ({}), staying with DIIS",
               unsupported);
    settings.enabled = false;
    return false;
  }

  m_active = true;
  m_linear = scf.m_procedure.fock_build_properties().linear_in_density;
  m_trust = settings.trust_first;
  log::info("switching to second-order SCF at iteration {} (max|FDS-SDF| "
            "{:.2e}, {} Hessian-vector products)",
            scf.iter, scf.diis_error, m_linear ? "exact" : "differenced");
  return true;
}

// The gradient of the energy with respect to the rotation kappa_ai of occupied
// orbital i into virtual a, C -> C exp(kappa), one spin block after the other:
// 4 F_ai for the restricted density C_occ C_occ^T, 2 F_ai per spin block
// otherwise, F in the MO basis. diagonal_hessian gets the usual approximation
// of its diagonal over the diagonal of that F, floored so a near-degenerate
// pair cannot blow up the step. F is taken, not mo.energies: after a rotation
// the orbitals are no longer canonical and the stored eigenvalues are stale.
template <SCFMethod P>
Vec SecondOrderSCF<P>::gradient(const SCF<P> &scf, Vec &diagonal_hessian) const {
  const MolecularOrbitals &mo = scf.ctx.mo;
  const int nb = spin_blocks(mo);
  const Eigen::Index nao = static_cast<Eigen::Index>(mo.n_ao),
                     nmo = mo.C.cols();
  const double fac = nb == 1 ? 4.0 : 2.0, gap_floor = 0.02;
  Eigen::Index size = 0;
  for (int b = 0; b < nb; b++) {
    const Eigen::Index nocc = occupied(mo, b);
    size += nocc * (nmo - nocc);
  }
  Vec g(size);
  diagonal_hessian.resize(size);
  Eigen::Index at = 0;
  for (int b = 0; b < nb; b++) {
    const Eigen::Index nocc = occupied(mo, b), nvir = nmo - nocc,
                       row = static_cast<Eigen::Index>(b) * nao;
    const Mat Cb = mo.C.middleRows(row, nao);
    const Mat F_mo = Cb.transpose() * scf.ctx.F.middleRows(row, nao) * Cb;
    const Vec e = F_mo.diagonal();
    for (Eigen::Index i = 0; i < nocc; i++) {
      for (Eigen::Index a = 0; a < nvir; a++, at++) {
        g(at) = fac * F_mo(nocc + a, i);
        diagonal_hessian(at) = fac * std::max(e(nocc + a) - e(i), gap_floor);
      }
    }
  }
  return g;
}

// C_from exp(kappa) by the Cayley transform (I - K/2)^-1 (I + K/2), exactly
// orthogonal for the antisymmetric K that carries kappa in its
// occupied-virtual blocks, so the orbitals stay S-orthonormal. The first nocc
// columns stay the occupied ones; the density follows.
template <SCFMethod P>
void SecondOrderSCF<P>::rotate(SCF<P> &scf, const Mat &C_from,
                               const Vec &kappa) const {
  MolecularOrbitals &mo = scf.ctx.mo;
  const int nb = spin_blocks(mo);
  const Eigen::Index nao = static_cast<Eigen::Index>(mo.n_ao),
                     nmo = mo.C.cols();
  Eigen::Index at = 0;
  for (int b = 0; b < nb; b++) {
    const Eigen::Index nocc = occupied(mo, b), nvir = nmo - nocc,
                       row = static_cast<Eigen::Index>(b) * nao;
    Mat K = Mat::Zero(nmo, nmo);
    for (Eigen::Index i = 0; i < nocc; i++) {
      for (Eigen::Index a = 0; a < nvir; a++, at++) {
        K(nocc + a, i) = 0.5 * kappa(at);
        K(i, nocc + a) = -0.5 * kappa(at);
      }
    }
    const Mat I = Mat::Identity(nmo, nmo);
    const Mat U = (I - K).partialPivLu().solve(I + K);
    mo.C.middleRows(row, nao) = C_from.middleRows(row, nao) * U;
  }
  mo.update_occupied_orbitals();
  mo.update_density_matrix();
}

// G(dD): for a build linear in the density the Fock build reads the
// perturbation straight, screening off its size. Otherwise the same quantity
// is the derivative of the build along dD, taken as a forward difference at a
// fixed largest element - one build either way.
//
// ponytail: forward difference, not central; the truncation error is a
// fraction of a percent of a Hessian the trust region already tolerates being
// inexact. An exact XC kernel contraction would need second derivatives from
// libxc, which `occ::dft::functional::Result` does not carry.
template <SCFMethod P>
Mat SecondOrderSCF<P>::fock_response(SCF<P> &scf, const Mat &dD) {
  MolecularOrbitals &mo = scf.ctx.mo;
  if (m_linear) {
    Mat D = dD;
    std::swap(mo.D, D);
    Mat dF = scf.m_procedure.compute_fock(mo, scf.ctx.K);
    std::swap(mo.D, D);
    return dF;
  }
  const double step =
      settings.fd_step / std::max(dD.cwiseAbs().maxCoeff(), 1e-12);
  Mat D = mo.D + step * dD;
  std::swap(mo.D, D);
  Mat F = scf.m_procedure.compute_fock(mo, scf.ctx.K);
  std::swap(mo.D, D);
  return (F - m_G) / step;
}

// H v for the rotation direction v, the derivative of the gradient along it:
// dC_occ = C_vir V, dC_vir = -C_occ V^T from C exp(kappa); dD from the
// density's own convention (C_occ C_occ^T, halved per spin block when
// unrestricted); dF = G(dD); and
// H v = fac (dC_vir^T F C_occ + C_vir^T dF C_occ + C_vir^T F dC_occ).
// The occupied-occupied and virtual-virtual parts of the second-order orbital
// change leave the energy alone, so nothing else contributes.
template <SCFMethod P>
Vec SecondOrderSCF<P>::hessian_vector(SCF<P> &scf, const Vec &v) {
  MolecularOrbitals &mo = scf.ctx.mo;
  const int nb = spin_blocks(mo);
  const Eigen::Index nao = static_cast<Eigen::Index>(mo.n_ao),
                     nmo = mo.C.cols();
  const double fac = nb == 1 ? 4.0 : 2.0, dfac = nb == 1 ? 1.0 : 0.5;
  Mat dC = Mat::Zero(mo.C.rows(), nmo), dD = Mat::Zero(mo.D.rows(), nao);
  Eigen::Index at = 0;
  for (int b = 0; b < nb; b++) {
    const Eigen::Index nocc = occupied(mo, b), nvir = nmo - nocc,
                       row = static_cast<Eigen::Index>(b) * nao;
    Mat V(nvir, nocc);
    for (Eigen::Index i = 0; i < nocc; i++)
      for (Eigen::Index a = 0; a < nvir; a++, at++)
        V(a, i) = v(at);
    const Mat Cocc = mo.C.block(row, 0, nao, nocc),
              Cvir = mo.C.block(row, nocc, nao, nvir);
    dC.block(row, 0, nao, nocc) = Cvir * V;
    dC.block(row, nocc, nao, nvir) = -Cocc * V.transpose();
    const Mat dCocc = dC.block(row, 0, nao, nocc);
    dD.middleRows(row, nao) =
        dfac * (dCocc * Cocc.transpose() + Cocc * dCocc.transpose());
  }
  const Mat dF = fock_response(scf, dD);
  Vec Hv(v.size());
  at = 0;
  for (int b = 0; b < nb; b++) {
    const Eigen::Index nocc = occupied(mo, b), nvir = nmo - nocc,
                       row = static_cast<Eigen::Index>(b) * nao;
    const Mat Cocc = mo.C.block(row, 0, nao, nocc),
              Cvir = mo.C.block(row, nocc, nao, nvir);
    const Mat dCocc = dC.block(row, 0, nao, nocc),
              dCvir = dC.block(row, nocc, nao, nvir);
    const Mat F = scf.ctx.F.middleRows(row, nao),
              dFb = dF.middleRows(row, nao);
    const Mat M = fac * (dCvir.transpose() * F * Cocc +
                         Cvir.transpose() * dFb * Cocc +
                         Cvir.transpose() * F * dCocc);
    for (Eigen::Index i = 0; i < nocc; i++)
      for (Eigen::Index a = 0; a < nvir; a++, at++)
        Hv(at) = M(a, i);
  }
  return Hv;
}

// The step of the current macro iteration: the lowest eigenvector of the
// augmented Hessian [[0, a g^T], [a g, H]] in the Davidson subspace m_B (H m_B
// in m_HB), scaled to kappa = x / (a x0), with a >= 1 raised by bisection until
// |kappa| fits the trust radius (a = 1 is the plain augmented-Hessian step).
// With extend, the subspace grows by the preconditioned residual of the
// level-shifted Newton equation (H - theta) kappa = -g, one Hessian-vector
// product per micro-iteration, until the residual is below a fraction of the
// gradient that shrinks with it, or micro_max is reached; without, the
// retained subspace is re-solved at the current radius, because after a
// rejected step the Fock matrix belongs to the rejected orbitals and no
// product could be added.
template <SCFMethod P>
void SecondOrderSCF<P>::solve(SCF<P> &scf, const bool extend) {
  const Vec &g = m_gradient;
  const double gnorm = g.norm();
  const double tol = gnorm * std::min(0.1, std::max(1e-3, std::sqrt(gnorm)));
  auto add = [&](Vec b) {
    for (int pass = 0; pass < 2; pass++)
      for (const Vec &Bk : m_B)
        b -= Bk.dot(b) * Bk;
    const double nb = b.norm();
    if (nb < 1e-10)
      return false;
    b /= nb;
    m_HB.push_back(hessian_vector(scf, b));
    m_B.push_back(b);
    return true;
  };
  if (extend && m_B.empty())
    add(-g.cwiseQuotient(m_diagonal_hessian));

  // the small problem at shift parameter a: theta, x0, xs and |kappa|
  Mat Hs;
  Vec gs;
  double theta = 0, x0 = 1, alpha = 1;
  Vec xs;
  auto reduced = [&](const double a, double &th, double &v0, Vec &v) {
    const Eigen::Index m = gs.size();
    Mat A = Mat::Zero(m + 1, m + 1);
    A(0, 0) = 0;
    A.block(0, 1, 1, m) = a * gs.transpose();
    A.block(1, 0, m, 1) = a * gs;
    A.block(1, 1, m, m) = Hs;
    Eigen::SelfAdjointEigenSolver<Mat> es(A);
    th = es.eigenvalues()(0);
    const Vec ev = es.eigenvectors().col(0);
    v0 = ev(0);
    v = ev.tail(m);
    return std::abs(v0) > 1e-14 ? v.norm() / (a * std::abs(v0))
                                : std::numeric_limits<double>::infinity();
  };
  int micro = 0;
  double knorm = 0, rnorm = 0;
  Vec kappa, Hkappa;
  for (;;) {
    const Eigen::Index m = static_cast<Eigen::Index>(m_B.size());
    Hs.resize(m, m);
    gs.resize(m);
    for (Eigen::Index i = 0; i < m; i++) {
      gs(i) = m_B[i].dot(g);
      for (Eigen::Index j = 0; j <= i; j++)
        Hs(i, j) = Hs(j, i) =
            0.5 * (m_B[i].dot(m_HB[j]) + m_B[j].dot(m_HB[i]));
    }
    alpha = 1;
    knorm = reduced(alpha, theta, x0, xs);
    m_boundary = knorm > m_trust;
    if (m_boundary) {
      // |kappa| falls monotonically with a: bracket, then bisect on log a
      double lo = 1, hi = 1;
      while (hi < 1e8 && reduced(hi, theta, x0, xs) > m_trust) {
        lo = hi;
        hi *= 2;
      }
      for (int it = 0; it < 60 && hi / lo > 1 + 1e-6; it++) {
        const double mid = std::sqrt(lo * hi);
        if (reduced(mid, theta, x0, xs) > m_trust)
          lo = mid;
        else
          hi = mid;
      }
      alpha = hi;
      knorm = reduced(alpha, theta, x0, xs);
    }
    kappa = Vec::Zero(g.size());
    Hkappa = Vec::Zero(g.size());
    for (Eigen::Index i = 0; i < m; i++) {
      kappa += xs(i) * m_B[i];
      Hkappa += xs(i) * m_HB[i];
    }
    if (std::isfinite(knorm)) {
      kappa /= alpha * x0;
      Hkappa /= alpha * x0;
      // the bisection tolerance may leave the step a hair outside: scale, do
      // not re-solve
      if (knorm > m_trust) {
        kappa *= m_trust / knorm;
        Hkappa *= m_trust / knorm;
        knorm = m_trust;
      }
    } else {
      // no finite step from the subspace: the preconditioned gradient, scaled
      // into the radius. Its curvature comes from a Fock build only while the
      // Fock matrix still belongs to these orbitals - after a rejected step
      // the diagonal approximation is all that is available.
      kappa = -g.cwiseQuotient(m_diagonal_hessian);
      kappa *= m_trust / kappa.norm();
      Hkappa = extend ? hessian_vector(scf, kappa)
                      : m_diagonal_hessian.cwiseProduct(kappa);
      knorm = m_trust;
      m_boundary = true;
    }
    const Vec r = g + Hkappa - theta * kappa;
    rnorm = r.norm();
    if (!extend || rnorm < tol || micro >= settings.micro_max)
      break;
    if (!add(-r.cwiseQuotient(
            (m_diagonal_hessian.array() - theta).max(1e-2).matrix())))
      break;
    micro++;
  }
  m_micro_total += micro;
  m_kappa = kappa;
  m_predicted = g.dot(kappa) + 0.5 * kappa.dot(Hkappa);
  log::debug("TRAH: {} micro-iterations ({} total), |g| {:.1e}, residual "
             "{:.1e}, |kappa| {:.1e} {} the trust radius {:.3f}, shift {:.1e}, "
             "predicted {:.1e} Eh",
             micro, m_micro_total, gnorm, rnorm, knorm,
             m_boundary ? "on" : "within", m_trust, theta, m_predicted);
}

// A step that raised the energy by more than the noise of a Fock build is
// rejected: the trust radius halves and the step is re-solved in the subspace
// the micro-iterations already built, from the orbitals it left. Otherwise the
// radius follows the ratio of the actual to the predicted decrease (doubled
// after a good step that reached the boundary, halved after a poor one) and a
// fresh gradient starts the next macro step.
template <SCFMethod P>
void SecondOrderSCF<P>::macro_step(SCF<P> &scf, const double energy) {
  const bool stepped = m_kappa.size() > 0;
  if (stepped && energy > m_energy + settings.noise &&
      m_kappa.cwiseAbs().maxCoeff() > 1e-6) {
    m_trust = 0.5 * std::min(m_trust, m_kappa.norm());
    log::info("TRAH: energy {:.1e} Eh above the orbitals the step left, trust "
              "radius {:.3f}, step re-solved",
              energy - m_energy, m_trust);
    solve(scf, false);
    rotate(scf, m_C, m_kappa);
    return;
  }
  if (stepped && m_predicted < 0) {
    const double rho = (energy - m_energy) / m_predicted;
    if (rho > 0.75 && m_boundary)
      m_trust = std::min(2.0 * m_trust, settings.trust_max);
    else if (rho < 0.25)
      m_trust *= 0.5;
  }
  m_energy = energy;
  m_gradient = gradient(scf, m_diagonal_hessian);
  m_B.clear();
  m_HB.clear();
  m_C = scf.ctx.mo.C;
  if (!m_linear)
    m_G = scf.ctx.F - scf.ctx.H;
  solve(scf, true);
  rotate(scf, m_C, m_kappa);
}

} // namespace occ::qm
