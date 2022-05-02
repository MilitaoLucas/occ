#pragma once
#include <occ/3rdparty/robin_hood.h>
#include <occ/core/linear_algebra.h>
#include <string>
#include <vector>

namespace occ::slater {

using occ::IVec;
using occ::Mat;
using occ::Vec;

class Shell {
  public:
    Shell();
    Shell(const IVec &, const IVec &, const Vec &, const Mat &);
    Shell(const std::vector<int> &, const std::vector<int> &,
          const std::vector<double> &,
          const std::vector<std::vector<double>> &);
    double rho(double r) const;
    double grad_rho(double r) const;
    Vec rho(const Vec &) const;
    void rho(const Vec &, Vec &) const;
    Vec grad_rho(const Vec &) const;
    void grad_rho(const Vec &, Vec &) const;
    size_t n_prim() const;
    size_t n_orb() const;
    void renormalize();
    void unnormalize();
    const auto &occupation() const { return m_occupation; }
    const auto &n() const { return m_n; }
    const auto &c() const { return m_c; }
    const auto &z() const { return m_z; }

  private:
    IVec m_occupation;
    IVec m_n;
    Vec m_n1;
    Vec m_z;
    Mat m_c;
};

class Basis {
  public:
    Basis() {}
    Basis(const std::vector<Shell> &);
    double rho(double r) const;
    double grad_rho(double r) const;
    Vec rho(const Vec &) const;
    Vec grad_rho(const Vec &) const;
    void renormalize();
    void unnormalize();
    const auto &shells() const { return m_shells; }

  private:
    std::vector<Shell> m_shells;
};

robin_hood::unordered_map<std::string, Basis>
load_slaterbasis(const std::string &);
} // namespace occ::slater
