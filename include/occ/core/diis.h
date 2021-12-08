#pragma once
#include <deque>
#include <occ/core/linear_algebra.h>
namespace occ::core::diis {

Mat diis_commutator(const Mat &A, const Mat &B, const Mat &overlap);

class DIIS {
public:
    DIIS(size_t start = 4, size_t diis_subspace = 6, double damping_factor = 0,
         size_t ngroup = 1, size_t ngroup_diis = 1, double mixing_fraction = 0);

    void extrapolate(Mat &x, Mat &error, bool extrapolate_error = false);

private:
    double m_error;
    bool m_error_is_set{false};
    size_t m_start{1};
    size_t m_diis_subspace_size{6};
    size_t m_iter{0};
    size_t m_num_group{1};
    size_t m_num_group_diis{1};
    double m_damping_factor;
    double m_mixing_fraction;

    Mat m_B; //!< B(i,j) = <ei|ej>

    std::deque<Mat> m_x; //!< set of most recent x given as input (i.e. not exrapolated)
    std::deque<Mat> m_errors;       //!< set of most recent errors
    std::deque<Mat> m_extrapolated; //!< set of most recent extrapolated x

    void set_error(double e);
    double error() const;

    void init();
};


class EDIIS : public DIIS {
    
};

} // namespace occ::diis
