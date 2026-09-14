#include <chrono>
#include <occ/core/diis.h>
#include <occ/core/log.h>
#include <occ/core/timings.h>
#include <occ/qm/cc/ccsd.h>
#include <occ/qm/cc/gemm.h> // pcon, pcon2, ppermute (parallel contractions and permutes)

// Restricted closed-shell CCSD, a direct port of the thc_cct reference
// rccsd.py (PySCF/Hirata RCCSD equations). The O(V^4) vvvv ladder term goes
// through eris.ladder (exact / DF / THC); everything else uses the cheap
// blocks plus ovvv. A canonical reference is assumed (Fock diagonal, f_ov = 0),
// so the bare orbital energies live only in the amplitude denominators and the
// F/L intermediates carry the correlation parts only.

namespace occ::qm::cc {

namespace {

using T2 = Eigen::Tensor<double, 2>;
using T4 = Eigen::Tensor<double, 4>;
using Sh2 = Eigen::array<int, 2>;
using Sh4 = Eigen::array<int, 4>;

inline Eigen::IndexPair<int> ip(int a, int b) { return {a, b}; }
template <int N> using IA = Eigen::array<Eigen::IndexPair<int>, N>;

// t1(i,a) t1(j,b) -> (i,j,a,b)
T4 t1t1_outer(const T2 &t1) {
  const T4 o = t1.contract(t1, IA<0>{}); // (i,a,j,b)
  return o.shuffle(Sh4{0, 2, 1, 3});     // (i,j,a,b)
}

// P(ij,ab): x + x.transpose(1,0,3,2)
// out(i,j,a,b) = x(i,j,a,b) + x(j,i,b,a), parallelised (Eigen's shuffle+add is
// serial and is called for every T2 residual term).
T4 sym_ijab(const T4 &x) {
  const Eigen::Index o = x.dimension(0), v = x.dimension(2);
  const Eigen::Index oo = o * o, oov = oo * v;
  T4 out(o, o, v, v);
  const double *xd = x.data();
  double *od = out.data();
  occ::parallel::parallel_for(size_t(0), static_cast<size_t>(v), [&](size_t bu) {
    const Eigen::Index b = static_cast<Eigen::Index>(bu);
    for (Eigen::Index a = 0; a < v; ++a)
      for (Eigen::Index j = 0; j < o; ++j)
        for (Eigen::Index i = 0; i < o; ++i)
          od[i + j * o + a * oo + b * oov] =
              xd[i + j * o + a * oo + b * oov] +
              xd[j + i * o + b * oo + a * oov];
  });
  return out;
}

} // namespace

double ccsd_energy(const T2 &t1, const T4 &t2, const CCIntegrals &eris) {
  const T4 &ovov = eris.ovov;
  const T4 tau = t2 + t1t1_outer(t1);
  // 2 (ijab,iajb) - (ijab,ibja); reindex ovov to (i,j,a,b)
  const T4 g_iajb = ovov.shuffle(Sh4{0, 2, 1, 3}); // (ia|jb)
  const T4 g_ibja = ovov.shuffle(Sh4{0, 2, 3, 1}); // (ib|ja)
  Eigen::Tensor<double, 0> s1 = (tau * g_iajb).sum();
  Eigen::Tensor<double, 0> s2 = (tau * g_ibja).sum();
  return 2.0 * s1(0) - s2(0);
}

namespace {

// Parallel, out-of-line x.shuffle(perm) for 4-tensors (Odim[k] = dim[perm[k]],
// the numpy transpose convention). Eigen's shuffle is serial, and inlined into
// update_amps every one of them was one more expression template for the
// optimiser: MSVC's /O2 never finished the old single-function update_amps
// (killed after 4 h 18 min, 14 Sep 2026). Materialising the permutations and
// splitting the update into the four stages below brought that under a minute.
T4 perm4(const T4 &x, const Sh4 &perm) {
  const Eigen::array<Eigen::Index, 4> dims{x.dimension(0), x.dimension(1),
                                           x.dimension(2), x.dimension(3)};
  T4 out(dims[perm[0]], dims[perm[1]], dims[perm[2]], dims[perm[3]]);
  ppermute<4>(out.data(), x.data(), dims, perm);
  return out;
}

// The F, L and W intermediates of one amplitude update (correlation parts
// only; the reference is canonical, so there is no bare Fock in them).
struct Intermediates {
  T2 Fov, Foo, Fvv, Loo, Lvv;
  T4 Woooo, Wvoov, Wvovo;
};

void f_and_l_intermediates(const T2 &t1, const T4 &tau, const CCIntegrals &e,
                           Intermediates &w) {
  const T4 &ovov = e.ovov, &ovoo = e.ovoo, &ovvv = e.ovvv;

  // Fov(k,c) = 2 (kc|ld) t1(ld) - (kd|lc) t1(ld)
  w.Fov = 2.0 * ovov.contract(t1, IA<2>{ip(2, 0), ip(3, 1)});
  w.Fov -= ovov.contract(t1, IA<2>{ip(2, 0), ip(1, 1)});

  // Foo(k,i) = 2 (kc|ld) tau(ilcd) - (kd|lc) tau(ilcd)
  w.Foo = 2.0 * ovov.contract(tau, IA<3>{ip(1, 2), ip(2, 1), ip(3, 3)});
  w.Foo -= ovov.contract(tau, IA<3>{ip(1, 3), ip(2, 1), ip(3, 2)});

  // Fvv(a,c) = -2 (kc|ld) tau(klad) + (kd|lc) tau(klad)
  w.Fvv =
      -2.0 * pcon<4, 4, 3>(tau, ovov, IA<3>{ip(0, 0), ip(1, 2), ip(3, 3)});
  w.Fvv += pcon<4, 4, 3>(tau, ovov, IA<3>{ip(0, 0), ip(1, 2), ip(3, 1)});

  // Loo(k,i) = Foo + 2 (lc|ki) t1(lc) - (kc|li) t1(lc)
  w.Loo = w.Foo;
  w.Loo += 2.0 * ovoo.contract(t1, IA<2>{ip(0, 0), ip(1, 1)});
  w.Loo -= ovoo.contract(t1, IA<2>{ip(2, 0), ip(1, 1)});

  // Lvv(a,c) = Fvv + 2 (kd|ac) t1(kd) - (kc|ad) t1(kd)
  w.Lvv = w.Fvv;
  w.Lvv += 2.0 * ovvv.contract(t1, IA<2>{ip(0, 0), ip(1, 1)});
  w.Lvv -= ovvv.contract(t1, IA<2>{ip(0, 0), ip(3, 1)}).shuffle(Sh2{1, 0});
}

void w_intermediates(const T2 &t1, const T4 &t2, const T4 &tau, const T4 &z4,
                     const CCIntegrals &e, Intermediates &w) {
  const T4 &oooo = e.oooo, &oovv = e.oovv, &ovoo = e.ovoo;
  const T4 &ovov = e.ovov, &ovvo = e.ovvo, &ovvv = e.ovvv;

  // Woooo(k,l,i,j)
  const T4 X_ovoo_t1 =
      pcon<4, 2, 1>(ovoo, t1, IA<1>{ip(1, 1)}); // (l/k, k/l, i/j, j/i)
  w.Woooo = perm4(X_ovoo_t1, Sh4{1, 0, 2, 3});  // "lcki,jc"
  w.Woooo += perm4(X_ovoo_t1, Sh4{0, 1, 3, 2}); // "kclj,ic"
  w.Woooo += pcon2(ovov, 1, 3, tau, 2, 3);      // kcld,ijcd
  w.Woooo += perm4(oooo, Sh4{0, 2, 1, 3});      // (ki|lj)

  // Wvoov(a,k,i,c)
  w.Wvoov = perm4(pcon<4, 2, 1>(ovvv, t1, IA<1>{ip(3, 1)}),
                  Sh4{2, 0, 3, 1}); // kcad,id
  w.Wvoov -= perm4(pcon<4, 2, 1>(ovoo, t1, IA<1>{ip(2, 0)}),
                   Sh4{3, 0, 2, 1});       // kcli,la
  w.Wvoov += perm4(ovvo, Sh4{2, 0, 3, 1}); // (kc|ai)
  w.Wvoov -= perm4(pcon2(ovov, 0, 1, z4, 1, 2),
                   Sh4{3, 0, 2, 1}); // ldkc,ilda (0.5 t2 + t1t1)
  w.Wvoov -= 0.5 * perm4(pcon2(ovov, 0, 3, t2, 1, 3),
                         Sh4{3, 1, 2, 0}); // lckd,ilad
  w.Wvoov += perm4(pcon2(ovov, 0, 1, t2, 1, 3),
                   Sh4{3, 0, 2, 1}); // ldkc,ilad

  // Wvovo(a,k,c,i)
  w.Wvovo = perm4(pcon<4, 2, 1>(ovvv, t1, IA<1>{ip(1, 1)}),
                  Sh4{1, 0, 2, 3}); // kdac,id
  w.Wvovo -= perm4(pcon<4, 2, 1>(ovoo, t1, IA<1>{ip(0, 0)}),
                   Sh4{3, 1, 0, 2});       // lcki,la
  w.Wvovo += perm4(oovv, Sh4{2, 0, 3, 1}); // (ki|ac)
  w.Wvovo -= perm4(pcon2(ovov, 0, 3, z4, 1, 2),
                   Sh4{3, 1, 0, 2}); // lckd,ilda (0.5 t2 + t1t1)
}

// T1 residual (before the denominators)
T2 t1_residual(const T2 &t1, const T4 &t2, const Intermediates &w,
               const CCIntegrals &e) {
  const T4 &ooov = e.ooov, &oovv = e.oovv, &ovvo = e.ovvo, &ovvv = e.ovvv;
  const T2 &Fov = w.Fov, &Foo = w.Foo, &Fvv = w.Fvv;

  T2 r1 = t1.contract(Fvv, IA<1>{ip(1, 1)});               // ac,ic
  r1 -= Foo.contract(t1, IA<1>{ip(0, 0)});                 // ki,ka
  r1 += 2.0 * Fov.contract(t2, IA<2>{ip(0, 0), ip(1, 2)}); // kc,kica
  r1 -= Fov.contract(t2, IA<2>{ip(0, 1), ip(1, 2)});       // kc,ikca
  {
    const T2 y = Fov.contract(t1, IA<1>{ip(0, 0)}); // (c,a)
    r1 += t1.contract(y, IA<1>{ip(1, 0)});          // kc,ic,ka
  }
  r1 += 2.0 * ovvo.contract(t1, IA<2>{ip(0, 0), ip(1, 1)})
                  .shuffle(Sh2{1, 0});                // kcai,kc
  r1 -= oovv.contract(t1, IA<2>{ip(0, 0), ip(3, 1)}); // kiac,kc
  r1 += 2.0 * pcon<4, 4, 3>(ovvv, t2, IA<3>{ip(0, 1), ip(1, 3), ip(3, 2)})
                  .shuffle(Sh2{1, 0}); // kdac,ikcd
  r1 -= pcon<4, 4, 3>(ovvv, t2, IA<3>{ip(0, 1), ip(1, 2), ip(3, 3)})
            .shuffle(Sh2{1, 0}); // kcad,ikcd
  {
    const T2 y = ovvv.contract(t1, IA<2>{ip(0, 0), ip(1, 1)});  // (a,c)
    r1 += 2.0 * t1.contract(y, IA<1>{ip(1, 1)});                // kdac,kd,ic
    const T2 y2 = ovvv.contract(t1, IA<2>{ip(0, 0), ip(3, 1)}); // (c,a)
    r1 -= t1.contract(y2, IA<1>{ip(1, 0)});                     // kcad,kd,ic
  }
  r1 -= 2.0 * ooov.contract(t2, IA<3>{ip(0, 0), ip(2, 1), ip(3, 3)}); // kilc,klac
  r1 += ooov.contract(t2, IA<3>{ip(0, 1), ip(2, 0), ip(3, 3)});       // likc,klac
  {
    const T2 y = ooov.contract(t1, IA<2>{ip(2, 0), ip(3, 1)});      // (k,i)
    r1 -= 2.0 * t1.contract(y, IA<1>{ip(0, 0)}).shuffle(Sh2{1, 0}); // kilc,lc,ka
    const T2 y2 = ooov.contract(t1, IA<2>{ip(0, 0), ip(3, 1)});     // (i,k)
    r1 += y2.contract(t1, IA<1>{ip(1, 0)});                         // likc,lc,ka
  }
  return r1;
}

// T2 residual (before the denominators)
T4 t2_residual(const T2 &t1, const T4 &t2, const T4 &tau,
               const Intermediates &w, const CCIntegrals &e) {
  const T4 &ooov = e.ooov, &oovv = e.oovv, &ovov = e.ovov, &ovvo = e.ovvo;
  const T4 &ovvv = e.ovvv;

  T4 r2 = perm4(ovov, Sh4{0, 2, 1, 3});  // (ia|jb) -> (i,j,a,b)
  r2 += pcon2(w.Woooo, 0, 1, tau, 0, 1); // klij,klab
  occ::timing::start(occ::timing::category::ccsd_ladder);
  r2 += e.ladder(tau); // vvvv ladder
  occ::timing::stop(occ::timing::category::ccsd_ladder);
  {
    const T4 b1 = perm4(pcon2(ovvv, 1, 3, tau, 3, 2),
                        Sh4{2, 3, 1, 0});             // kdac,ijcd -> (i,j,a,k)
    r2 -= pcon<4, 2, 1>(b1, t1, IA<1>{ip(3, 0)}); // ijak,kb
    const T4 b2 = perm4(pcon2(ovvv, 1, 3, tau, 2, 3),
                        Sh4{2, 3, 1, 0}); // kcbd,ijcd -> (i,j,b,k)
    r2 -= perm4(pcon<4, 2, 1>(b2, t1, IA<1>{ip(3, 0)}),
                Sh4{0, 1, 3, 2}); // ijbk,ka
  }
  r2 += sym_ijab(perm4(pcon<2, 4, 1>(w.Lvv, t2, IA<1>{ip(1, 2)}),
                       Sh4{1, 2, 0, 3}));                    // ac,ijcb
  r2 -= sym_ijab(pcon<2, 4, 1>(w.Loo, t2, IA<1>{ip(0, 0)})); // ki,kjab
  {
    T4 tmp = 2.0 * perm4(pcon2(w.Wvoov, 1, 3, t2, 0, 2),
                         Sh4{1, 2, 0, 3}); // akic,kjcb
    tmp -= perm4(pcon2(w.Wvovo, 1, 2, t2, 0, 2),
                 Sh4{1, 2, 0, 3}); // akci,kjcb
    r2 += sym_ijab(tmp);
  }
  r2 -= sym_ijab(perm4(pcon2(w.Wvoov, 1, 3, t2, 0, 3),
                       Sh4{1, 2, 0, 3})); // akic,kjbc
  r2 -= sym_ijab(perm4(pcon2(w.Wvovo, 1, 2, t2, 0, 3),
                       Sh4{1, 2, 3, 0})); // bkci,kjac
  {
    // tmp2(a,b,i,c) = -(ki|bc) t1(ka) + (ia|cb form) ovvv[i,a,c,b]
    T4 tmp2 = -1.0 * perm4(pcon<4, 2, 1>(oovv, t1, IA<1>{ip(0, 0)}),
                           Sh4{3, 1, 0, 2});
    tmp2 += perm4(ovvv, Sh4{1, 3, 0, 2}); // ovvv.transpose(1,3,0,2)
    r2 += sym_ijab(perm4(pcon<4, 2, 1>(tmp2, t1, IA<1>{ip(3, 1)}),
                         Sh4{2, 3, 0, 1})); // abic,jc
  }
  {
    // tmp2(a,k,i,j) = (kc|ai) t1(jc) + ooov.transpose(3,1,2,0)
    T4 tmp2 = perm4(pcon<4, 2, 1>(ovvo, t1, IA<1>{ip(1, 1)}),
                    Sh4{1, 0, 2, 3});
    tmp2 += perm4(ooov, Sh4{3, 1, 2, 0});
    r2 -= sym_ijab(perm4(pcon<4, 2, 1>(tmp2, t1, IA<1>{ip(1, 0)}),
                         Sh4{1, 2, 0, 3})); // akij,kb
  }
  return r2;
}

// One CCSD amplitude update. Returns (t1new, t2new) already divided by the
// orbital-energy denominators (matching rccsd.update_amps).
std::pair<T2, T4> update_amps(const T2 &t1, const T4 &t2,
                              const CCIntegrals &e) {
  const int o = e.nocc, v = e.nvir;

  const T4 tt = t1t1_outer(t1); // t1(i,a) t1(j,b) -> (i,j,a,b)
  const T4 tau = t2 + tt;       // make_tau
  const T4 z4 = 0.5 * t2 + tt;  // 0.5 t2 + t1t1 (Wvoov/Wvovo)

  Intermediates w;
  f_and_l_intermediates(t1, tau, e, w);
  w_intermediates(t1, t2, tau, z4, e, w);
  const T2 r1 = t1_residual(t1, t2, w, e);
  const T4 r2 = t2_residual(t1, t2, tau, w, e);

  // --- divide by denominators --------------------------------------------
  const Vec &mo_e = e.mo_energy;
  T2 t1new(o, v);
  for (int i = 0; i < o; ++i)
    for (int a = 0; a < v; ++a)
      t1new(i, a) = r1(i, a) / (mo_e(i) - mo_e(o + a));
  T4 t2new(o, o, v, v);
  for (int i = 0; i < o; ++i)
    for (int j = 0; j < o; ++j)
      for (int a = 0; a < v; ++a)
        for (int b = 0; b < v; ++b)
          t2new(i, j, a, b) =
              r2(i, j, a, b) /
              (mo_e(i) + mo_e(j) - mo_e(o + a) - mo_e(o + b));
  return {t1new, t2new};
}

} // namespace

CCSDResult ccsd(const CCIntegrals &eris, const CCSDOptions &opts) {
  occ::timing::start(occ::timing::category::ccsd);
  const int o = eris.nocc, v = eris.nvir;
  const Vec &mo_e = eris.mo_energy;

  // MP1 guess: t1 = 0, t2 = (ia|jb) / Dijab
  T2 t1(o, v);
  t1.setZero();
  T4 t2(o, o, v, v);
  const T4 g_iajb = eris.ovov.shuffle(Sh4{0, 2, 1, 3});
  for (int i = 0; i < o; ++i)
    for (int j = 0; j < o; ++j)
      for (int a = 0; a < v; ++a)
        for (int b = 0; b < v; ++b)
          t2(i, j, a, b) =
              g_iajb(i, j, a, b) /
              (mo_e(i) + mo_e(j) - mo_e(o + a) - mo_e(o + b));

  const Eigen::Index n1 = static_cast<Eigen::Index>(o) * v;
  const Eigen::Index n2 = static_cast<Eigen::Index>(o) * o * v * v;
  occ::core::diis::DIIS diis;

  CCSDResult result;
  occ::log::info("starting CCSD iterations ({} occ, {} virt)", o, v);
  double e_old = ccsd_energy(t1, t2, eris);
  double total_time = 0.0;
  for (int it = 0; it < opts.max_cycle; ++it) {
    const auto tstart = std::chrono::high_resolution_clock::now();
    auto [t1n, t2n] = update_amps(t1, t2, eris);

    // Amplitude-change residual ||t_new - t_old|| (the DIIS error vector).
    Mat x(n1 + n2, 1), err(n1 + n2, 1);
    std::copy(t1n.data(), t1n.data() + n1, x.data());
    std::copy(t2n.data(), t2n.data() + n2, x.data() + n1);
    for (Eigen::Index k = 0; k < n1; ++k)
      err(k, 0) = t1n.data()[k] - t1.data()[k];
    for (Eigen::Index k = 0; k < n2; ++k)
      err(n1 + k, 0) = t2n.data()[k] - t2.data()[k];
    const double rnorm = err.norm();

    if (opts.diis) {
      diis.extrapolate(x, err);
      std::copy(x.data(), x.data() + n1, t1n.data());
      std::copy(x.data() + n1, x.data() + n1 + n2, t2n.data());
    }

    t1 = t1n;
    t2 = t2n;
    const double e_new = ccsd_energy(t1, t2, eris);
    const double de = e_new - e_old;
    const auto tstop = std::chrono::high_resolution_clock::now();
    const double secs = std::chrono::duration<double>(tstop - tstart).count();
    total_time += secs;

    if (it == 0)
      occ::log::info("{:>4s} {: >20s} {: >12s} {: >12s}  {: >8s}", "#",
                     "E_corr (Ha)", "|dE|", "|dT|", "T (s)");
    occ::log::info("{:>4d} {:>20.12f} {:>12.5e} {:>12.5e}  {:>8.2e}", it + 1,
                   e_new, std::abs(de), rnorm, secs);
    occ::log::flush();

    result.iterations = it + 1;
    e_old = e_new;
    if (std::abs(de) < opts.tol) {
      result.converged = true;
      break;
    }
  }
  occ::log::info("CCSD {} after {} iterations ({:.3f} s)",
                 result.converged ? "converged" : "NOT converged",
                 result.iterations, total_time);

  result.e_corr = e_old;
  result.t1 = t1;
  result.t2 = t2;
  occ::timing::stop(occ::timing::category::ccsd);
  return result;
}

} // namespace occ::qm::cc
