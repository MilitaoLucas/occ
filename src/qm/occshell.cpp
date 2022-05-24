#include <cmath>
#include <occ/core/constants.h>
#include <occ/core/util.h>
#include <occ/qm/cint_interface.h>
#include <occ/qm/occshell.h>

namespace occ::qm {

double gint(int n, double alpha) {
    double n1_2 = 0.5 * (n + 1);
    return std::tgamma(n1_2) / (2 * std::pow(alpha, n1_2));
}

double gto_norm(int l, double alpha) {
    // to evaluate without the gamma function:
    // sqrt of the following:
    //
    // 2^{2l + 3}(l + 1)!\alpha^{l + 3/2}
    // -------------------
    // (2l + 2)! \pi^{1/2}
    //
    /*
    double nn = std::pow(2, (2 * l + 3)) * util::factorial(l + 1) *
                pow((2 * alpha), (l + 1.5)) /
                (util::factorial(2 * l + 2) * constants::sqrt_pi<double>);
    return sqrt(nn);
    */
    return 1.0 / std::sqrt(gint(l * 2 + 2, 2 * alpha));
}

void normalize_contracted_gto(int l, const Vec &alpha, Mat &coeffs) {
    Mat ee = alpha.rowwise().replicate(alpha.rows()) +
             alpha.transpose().colwise().replicate(alpha.rows());

    for (size_t i = 0; i < ee.rows(); i++) {
        for (size_t j = 0; j < ee.cols(); j++) {
            ee(i, j) = gint(l * 2 + 2, ee(i, j));
        }
    }
    Eigen::ArrayXd s1 = coeffs.transpose() * ee * coeffs;
    s1 = 1.0 / s1.sqrt();
    coeffs = coeffs * s1.matrix();
}

OccShell::OccShell(const int ang, const std::vector<double> &expo,
                   const std::vector<std::vector<double>> &contr,
                   const std::array<double, 3> &pos)
    : l(ang), origin() {
    size_t nprim = expo.size();
    size_t ncont = contr.size();
    assert(nprim > 0);
    assert(ncont > 0);
    for (const auto &c : contr) {
        assert(c.size() == nprim);
    }
    exponents = Eigen::VectorXd(nprim);
    contraction_coefficients = Eigen::MatrixXd(nprim, ncont);
    for (size_t j = 0; j < ncont; j++) {
        for (size_t i = 0; i < nprim; i++) {
            if (j == 0)
                exponents(i) = expo[i];
            contraction_coefficients(i, j) = contr[j][i];
        }
    }
    origin = {pos[0], pos[1], pos[2]};
}

bool OccShell::operator==(const OccShell &other) const {
    return &other == this ||
           (origin == other.origin && exponents == other.exponents &&
            contraction_coefficients == other.contraction_coefficients);
}

bool OccShell::operator!=(const OccShell &other) const {
    return !this->operator==(other);
}

bool OccShell::operator<(const OccShell &other) const { return l < other.l; }

size_t OccShell::num_primitives() const { return exponents.rows(); }
size_t OccShell::num_contractions() const {
    return contraction_coefficients.cols();
}

double OccShell::norm() const {
    double result = 0.0;
    for (Eigen::Index i = 0; i < contraction_coefficients.rows(); i++) {
        Eigen::Index j;
        double a = exponents(i);
        for (j = 0; j < i; j++) {
            double b = exponents(j);
            double ab = 2 * sqrt(a * b) / (a + b);
            result += 2 * contraction_coefficients(i, 0) *
                      contraction_coefficients(j, 0) * pow(ab, l + 1.5);
        }
        result +=
            contraction_coefficients(i, 0) * contraction_coefficients(j, 0);
    }
    return sqrt(result) * pi2_34;
}

double OccShell::max_exponent() const { return exponents.maxCoeff(); }
double OccShell::min_exponent() const { return exponents.minCoeff(); }

void OccShell::incorporate_shell_norm() {
    for (size_t i = 0; i < num_primitives(); i++) {
        double n = gto_norm(static_cast<int>(l), exponents(i));
        contraction_coefficients.row(i).array() *= n;
    }
    normalize_contracted_gto(l, exponents, contraction_coefficients);
}

double OccShell::coeff_normalized(Eigen::Index contr_idx,
                                  Eigen::Index coeff_idx) const {
    return contraction_coefficients(coeff_idx, contr_idx) /
           gto_norm(static_cast<int>(l), exponents(coeff_idx));
}

size_t OccShell::size() const {
    switch (kind) {
    case Spherical:
        return 2 * l + 1;
    default:
        return (l + 1) * (l + 2) / 2;
    }
}

char OccShell::l_to_symbol(uint_fast8_t l) {
    assert(l <= 19);
    static std::array<char, 20> l_symbols = {'s', 'p', 'd', 'f', 'g', 'h', 'i',
                                             'k', 'm', 'n', 'o', 'q', 'r', 't',
                                             'u', 'v', 'w', 'x', 'y', 'z'};
    return l_symbols[l];
}

uint_fast8_t OccShell::symbol_to_l(char symbol) {
    const char usym = ::toupper(symbol);
    switch (usym) {
    case 'S':
        return 0;
    case 'P':
        return 1;
    case 'D':
        return 2;
    case 'F':
        return 3;
    case 'G':
        return 4;
    case 'H':
        return 5;
    case 'I':
        return 6;
    case 'K':
        return 7;
    case 'M':
        return 8;
    case 'N':
        return 9;
    case 'O':
        return 10;
    case 'Q':
        return 11;
    case 'R':
        return 12;
    case 'T':
        return 13;
    case 'U':
        return 14;
    case 'V':
        return 15;
    case 'W':
        return 16;
    case 'X':
        return 17;
    case 'Y':
        return 18;
    case 'Z':
        return 19;
    default:
        throw "invalid angular momentum label";
    }
}

char OccShell::symbol() const { return l_to_symbol(l); }

OccShell OccShell::translated_copy(const Eigen::Vector3d &origin) const {
    OccShell other = *this;
    other.origin = origin;
    return other;
}

size_t OccShell::libcint_environment_size() const {
    return exponents.size() + contraction_coefficients.size();
}

int OccShell::find_atom_index(const std::vector<Atom> &atoms) const {
    double x = origin(0), y = origin(1), z = origin(2);
    auto same_site = [&x, &y, &z](const Atom &atom) {
        return atom.square_distance(x, y, z) < 1e-6;
    };
    return std::distance(begin(atoms),
                         std::find_if(begin(atoms), end(atoms), same_site));
}

bool OccShell::is_pure() const { return kind == Spherical; }

std::ostream &operator<<(std::ostream &stream, const OccShell &shell) {
    stream << shell.symbol() << " (" << shell.origin(0) << ","
           << shell.origin(1) << ", " << shell.origin(2) << ")\n";
    stream << "exp   contr\n";
    for (int i = 0; i < shell.num_primitives(); i++) {
        stream << " " << shell.exponents(i);
        for (int j = 0; j < shell.num_contractions(); j++) {
            stream << " " << shell.contraction_coefficients(i, j);
        }
        stream << "\n";
    }
    return stream;
}

} // namespace occ::qm
