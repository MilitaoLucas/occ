#include <fstream>
#include <occ/core/element.h>
#include <occ/core/logger.h>
#include <occ/core/timings.h>
#include <occ/io/basis_g94.h>

namespace occ::io::basis::g94 {

using occ::core::Element;

void fortran_dfloats_to_efloats(std::string &str) {
    for (auto &ch : str) {
        if (ch == 'd')
            ch = 'e';
        if (ch == 'D')
            ch = 'E';
    }
}

std::vector<std::vector<Shell>> read(const std::string &file_dot_g94,
                                     bool force_cartesian_d,
                                     std::string locale_name) {
    occ::timing::start(occ::timing::category::io);
    std::locale locale(
        locale_name.c_str()); // TODO omit c_str() with up-to-date stdlib
    std::vector<std::vector<libint2::Shell>> ref_shells(
        118); // 118 = number of chemical elements
    std::ifstream is(file_dot_g94);
    is.imbue(locale);

    if (is.good()) {
        occ::log::debug("Will read basis set from {}", file_dot_g94);
        std::string line, rest;

        // skip till first basis
        while (std::getline(is, line) && line != "****") {
        }

#define LIBINT2_LINE_TO_STRINGSTREAM(line)                                     \
    std::istringstream iss(line);                                              \
    iss.imbue(locale);

        size_t Z;
        auto nextbasis = true, nextshell = false;
        // read lines till end
        while (std::getline(is, line)) {
            // skipping empties and starting with '!' (the comment delimiter)
            if (line.empty() || line[0] == '!')
                continue;
            if (line == "****") {
                nextbasis = true;
                nextshell = false;
                continue;
            }
            if (nextbasis) {
                nextbasis = false;
                LIBINT2_LINE_TO_STRINGSTREAM(line);
                std::string elemsymbol;
                iss >> elemsymbol >> rest;

                bool found = false;

                Element e(elemsymbol, true);
                found = e.atomic_number() > 0;
                occ::log::debug("Found basis for symbol {} (match={})",
                                elemsymbol, e.symbol());
                Z = e.atomic_number();
                if (!found) {
                    std::string errstr =
                        fmt::format("Unknown element symbol {} found in {}",
                                    elemsymbol, file_dot_g94);
                    throw std::logic_error(errstr);
                }

                nextshell = true;
                continue;
            }
            if (nextshell) {
                LIBINT2_LINE_TO_STRINGSTREAM(line);
                std::string amlabel;
                std::size_t nprim;
                iss >> amlabel >> nprim >> rest;
                if (amlabel != "SP" && amlabel != "sp") {
                    assert(amlabel.size() == 1);
                    auto l = Shell::am_symbol_to_l(amlabel[0]);
                    libint2::svector<double> exps;
                    libint2::svector<double> coeffs;
                    for (decltype(nprim) p = 0; p != nprim; ++p) {
                        while (std::getline(is, line) &&
                               (line.empty() || line[0] == '!')) {
                        }
                        fortran_dfloats_to_efloats(line);
                        LIBINT2_LINE_TO_STRINGSTREAM(line);
                        double e, c;
                        iss >> e >> c;
                        exps.emplace_back(e);
                        coeffs.emplace_back(c);
                    }
                    auto pure = force_cartesian_d ? (l > 2) : (l > 1);
                    ref_shells.at(Z).push_back(
                        libint2::Shell{std::move(exps),
                                       {{l, pure, std::move(coeffs)}},
                                       {{0, 0, 0}}});
                } else { // split the SP shells
                    libint2::svector<double> exps;
                    libint2::svector<double> coeffs_s, coeffs_p;
                    for (decltype(nprim) p = 0; p != nprim; ++p) {
                        while (std::getline(is, line) &&
                               (line.empty() || line[0] == '!')) {
                        }
                        fortran_dfloats_to_efloats(line);
                        LIBINT2_LINE_TO_STRINGSTREAM(line);
                        double e, c1, c2;
                        iss >> e >> c1 >> c2;
                        exps.emplace_back(e);
                        coeffs_s.emplace_back(c1);
                        coeffs_p.emplace_back(c2);
                    }
                    ref_shells.at(Z).push_back(libint2::Shell{
                        exps, {{0, false, coeffs_s}}, {{0, 0, 0}}});
                    ref_shells.at(Z).push_back(
                        libint2::Shell{std::move(exps),
                                       {{1, false, std::move(coeffs_p)}},
                                       {{0, 0, 0}}});
                }
            }
        }

#undef LIBINT2_LINE_TO_STRINGSTREAM
    } else {
        std::string errmsg =
            fmt::format("Could not open {} for reading", file_dot_g94);
        throw std::ios_base::failure(errmsg);
    }

    occ::timing::stop(occ::timing::category::io);
    return ref_shells;
}

std::vector<std::vector<occ::qm::OccShell>>
read_occshell(const std::string &file_dot_g94, bool force_cartesian_d,
              std::string locale_name) {
    occ::timing::start(occ::timing::category::io);
    std::locale locale(
        locale_name.c_str()); // TODO omit c_str() with up-to-date stdlib
    std::vector<std::vector<occ::qm::OccShell>> ref_shells(
        118); // 118 = number of chemical elements
    std::ifstream is(file_dot_g94);
    is.imbue(locale);

    if (is.good()) {
        occ::log::debug("Will read basis set from {}", file_dot_g94);
        std::string line, rest;

        // skip till first basis
        while (std::getline(is, line) && line != "****") {
        }

#define LIBINT2_LINE_TO_STRINGSTREAM(line)                                     \
    std::istringstream iss(line);                                              \
    iss.imbue(locale);

        size_t Z;
        auto nextbasis = true, nextshell = false;
        // read lines till end
        while (std::getline(is, line)) {
            // skipping empties and starting with '!' (the comment delimiter)
            if (line.empty() || line[0] == '!')
                continue;
            if (line == "****") {
                nextbasis = true;
                nextshell = false;
                continue;
            }
            if (nextbasis) {
                nextbasis = false;
                LIBINT2_LINE_TO_STRINGSTREAM(line);
                std::string elemsymbol;
                iss >> elemsymbol >> rest;

                bool found = false;

                Element e(elemsymbol, true);
                found = e.atomic_number() > 0;
                occ::log::debug("Found basis for symbol {} (match={})",
                                elemsymbol, e.symbol());
                Z = e.atomic_number();
                if (!found) {
                    std::string errstr =
                        fmt::format("Unknown element symbol {} found in {}",
                                    elemsymbol, file_dot_g94);
                    throw std::logic_error(errstr);
                }

                nextshell = true;
                continue;
            }
            if (nextshell) {
                LIBINT2_LINE_TO_STRINGSTREAM(line);
                std::string amlabel;
                std::size_t nprim;
                iss >> amlabel >> nprim >> rest;
                if (amlabel != "SP" && amlabel != "sp") {
                    assert(amlabel.size() == 1);
                    auto l = Shell::am_symbol_to_l(amlabel[0]);
                    std::vector<double> exps;
                    std::vector<double> coeffs;
                    for (decltype(nprim) p = 0; p != nprim; ++p) {
                        while (std::getline(is, line) &&
                               (line.empty() || line[0] == '!')) {
                        }
                        fortran_dfloats_to_efloats(line);
                        LIBINT2_LINE_TO_STRINGSTREAM(line);
                        double e, c;
                        iss >> e >> c;
                        exps.emplace_back(e);
                        coeffs.emplace_back(c);
                    }
                    auto pure = force_cartesian_d ? (l > 2) : (l > 1);
                    ref_shells.at(Z).push_back(
                        occ::qm::OccShell(l, exps, {coeffs}, {0, 0, 0}));
                } else { // split the SP shells
                    std::vector<double> exps;
                    std::vector<double> coeffs_s, coeffs_p;
                    for (decltype(nprim) p = 0; p != nprim; ++p) {
                        while (std::getline(is, line) &&
                               (line.empty() || line[0] == '!')) {
                        }
                        fortran_dfloats_to_efloats(line);
                        LIBINT2_LINE_TO_STRINGSTREAM(line);
                        double e, c1, c2;
                        iss >> e >> c1 >> c2;
                        exps.emplace_back(e);
                        coeffs_s.emplace_back(c1);
                        coeffs_p.emplace_back(c2);
                    }
                    ref_shells.at(Z).push_back(
                        occ::qm::OccShell(0, exps, {coeffs_s}, {0, 0, 0}));
                    ref_shells.at(Z).push_back(
                        occ::qm::OccShell(1, exps, {coeffs_p}, {0, 0, 0}));
                }
            }
        }

#undef LIBINT2_LINE_TO_STRINGSTREAM
    } else {
        std::string errmsg =
            fmt::format("Could not open {} for reading", file_dot_g94);
        throw std::ios_base::failure(errmsg);
    }

    occ::timing::stop(occ::timing::category::io);
    return ref_shells;
}

} // namespace occ::io::basis::g94
