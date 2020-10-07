#include "dft.h"
#include <libint2/basis.h>
#include <libint2/atom.h>


namespace tonto::dft {

DFT::DFT(const std::string& method, const libint2::BasisSet& basis, const std::vector<libint2::Atom>& atoms) :
    m_hf(atoms, basis), m_grid(basis, atoms)
{
    m_funcs.push_back(DensityFunctional("LDA"));
}


MatRM DFT::compute_2body_fock(const MatRM &D, double precision, const MatRM &Schwarz) const
{
    return m_hf.compute_2body_fock(D, precision, Schwarz);
}

std::pair<MatRM, MatRM> DFT::compute_JK(const MatRM &D, double precision, const MatRM &Schwarz) const
{
    return m_hf.compute_JK(D, precision, Schwarz);
}

}
