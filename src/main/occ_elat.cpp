#include <cxxopts.hpp>
#include <filesystem>
#include <fmt/os.h>
#include <occ/core/kabsch.h>
#include <occ/core/logger.h>
#include <occ/core/units.h>
#include <occ/crystal/crystal.h>
#include <occ/dft/dft.h>
#include <occ/interaction/disp.h>
#include <occ/interaction/pairinteraction.h>
#include <occ/interaction/polarization.h>
#include <occ/io/cifparser.h>
#include <occ/io/fchkreader.h>
#include <occ/io/fchkwriter.h>
#include <occ/qm/hf.h>
#include <occ/qm/scf.h>
#include <occ/qm/wavefunction.h>
#include <occ/main/pair_energy.h>

namespace fs = std::filesystem;
using occ::core::Dimer;
using occ::core::Element;
using occ::core::Molecule;
using occ::crystal::Crystal;
using occ::crystal::SymmetryOperation;
using occ::hf::HartreeFock;
using occ::interaction::CEEnergyComponents;
using occ::interaction::CEModelInteraction;
using occ::qm::BasisSet;
using occ::qm::SpinorbitalKind;
using occ::qm::Wavefunction;
using occ::scf::SCF;
using occ::units::AU_TO_KCAL_PER_MOL;
using occ::units::AU_TO_KJ_PER_MOL;
using occ::units::BOHR_TO_ANGSTROM;
using occ::util::all_close;

std::string dimer_symop(const occ::core::Dimer &dimer, const Crystal &crystal) {
    const auto &a = dimer.a();
    const auto &b = dimer.b();
    if (a.asymmetric_molecule_idx() != b.asymmetric_molecule_idx())
        return "-";

    int sa_int = a.asymmetric_unit_symop()(0);
    int sb_int = b.asymmetric_unit_symop()(0);

    SymmetryOperation symop_a(sa_int);
    SymmetryOperation symop_b(sb_int);

    auto symop_ab = symop_b * symop_a.inverted();
    occ::Vec3 c_a =
        symop_ab(crystal.to_fractional(a.positions())).rowwise().mean();
    occ::Vec3 v_ab = crystal.to_fractional(b.centroid()) - c_a;

    symop_ab = symop_ab.translated(v_ab);
    return symop_ab.to_string();
}

Crystal read_crystal(const std::string &filename) {
    occ::io::CifParser parser;
    return parser.parse_crystal(filename).value();
}

std::vector<Wavefunction>
calculate_wavefunctions(const std::string &basename,
                        const std::vector<Molecule> &molecules,
                        const std::string &model) {
    std::string method = "b3lyp";
    std::string basis_name = "6-31G**";
    if (model == "ce-hf") {
        method = "hf";
        basis_name = "3-21G";
    }
    std::vector<Wavefunction> wfns;
    size_t index = 0;
    for (const auto &m : molecules) {
        fs::path fchk_path(
            fmt::format("{}_{}_{}.fchk", basename, index, model));
        fmt::print("Molecule ({})\n{:3s} {:^10s} {:^10s} {:^10s}\n", index,
                   "sym", "x", "y", "z");
        for (const auto &atom : m.atoms()) {
            fmt::print("{:^3s} {:10.6f} {:10.6f} {:10.6f}\n",
                       Element(atom.atomic_number).symbol(), atom.x, atom.y,
                       atom.z);
        }
        if (fs::exists(fchk_path)) {
            using occ::io::FchkReader;
            FchkReader fchk(fchk_path.string());
            auto wfn = Wavefunction(fchk);
            wfns.push_back(wfn);
        } else {
            BasisSet basis(basis_name, m.atoms());
            basis.set_pure(false);
            fmt::print("Loaded basis set, {} shells, {} basis functions\n",
                       basis.size(), libint2::nbf(basis));
            if (model == "ce-hf") {
                HartreeFock hf(m.atoms(), basis);
                SCF<HartreeFock, SpinorbitalKind::Restricted> scf(hf);
                scf.set_charge_multiplicity(0, 1);
                double e = scf.compute_scf_energy();
                auto wfn = scf.wavefunction();
                occ::io::FchkWriter fchk(fchk_path.string());
                fchk.set_title(fmt::format("{} {}/{} generated by occ",
                                           fchk_path.stem(), method,
                                           basis_name));
                fchk.set_method(method);
                fchk.set_basis_name(basis_name);
                wfn.save(fchk);
                fchk.write();
                wfns.push_back(wfn);
            } else {
                occ::dft::DFT rks(method, basis, m.atoms(),
                                  SpinorbitalKind::Restricted);
                SCF<occ::dft::DFT, SpinorbitalKind::Restricted> scf(rks);

                scf.set_charge_multiplicity(0, 1);
                scf.start_incremental_F_threshold = 0.0;
                double e = scf.compute_scf_energy();
                auto wfn = scf.wavefunction();
                occ::io::FchkWriter fchk(fchk_path.string());
                fchk.set_title(fmt::format("{} {}/{} generated by occ",
                                           fchk_path.stem(), method,
                                           basis_name));
                fchk.set_method(method);
                fchk.set_basis_name(basis_name);
                wfn.save(fchk);
                fchk.write();
                wfns.push_back(wfn);
            }
        }

        index++;
    }
    return wfns;
}

void compute_monomer_energies(std::vector<Wavefunction> &wfns) {
    size_t complete = 0;
    for (auto &wfn : wfns) {
        fmt::print("Calculating monomer energies {}/{}\n", complete,
                   wfns.size());
        std::cout << std::flush;
        HartreeFock hf(wfn.atoms, wfn.basis);
	occ::Mat schwarz = hf.compute_schwarz_ints();
        occ::interaction::compute_ce_model_energies(wfn, hf, 1e-8, schwarz);
        complete++;
    }
    fmt::print("Finished calculating {} unique monomer energies\n", complete);
}

auto calculate_transform(const Wavefunction &wfn, const Molecule &m,
                         const Crystal &c) {
    int sint = m.asymmetric_unit_symop()(0);
    SymmetryOperation symop(sint);
    occ::Mat3N positions = wfn.positions() * BOHR_TO_ANGSTROM;

    occ::Mat3 rotation =
        c.unit_cell().direct() * symop.rotation() * c.unit_cell().inverse();
    occ::Vec3 translation =
        (m.centroid() - (rotation * positions).rowwise().mean()) /
        BOHR_TO_ANGSTROM;
    return std::make_pair(rotation, translation);
}

void write_xyz_dimer(const std::string &filename, const Dimer &dimer) {
    auto output = fmt::output_file(filename);
    const auto &pos = dimer.positions();
    const auto &nums = dimer.atomic_numbers();
    output.print("{}\n\n", nums.rows());
    for (size_t i = 0; i < nums.rows(); i++) {
        output.print("{} {} {} {}\n", Element(nums(i)).symbol(), pos(0, i),
                     pos(1, i), pos(2, i));
    }
}

auto calculate_interaction_energy(const Dimer &dimer,
                                  const std::vector<Wavefunction> &wfns,
                                  const Crystal &crystal,
                                  const std::string &model_name) {
    Molecule mol_A = dimer.a();
    Molecule mol_B = dimer.b();
    const auto &wfna = wfns[mol_A.asymmetric_molecule_idx()];
    const auto &wfnb = wfns[mol_B.asymmetric_molecule_idx()];
    Wavefunction A = wfns[mol_A.asymmetric_molecule_idx()];
    Wavefunction B = wfns[mol_B.asymmetric_molecule_idx()];
    auto transform_a = calculate_transform(wfna, mol_A, crystal);
    A.apply_transformation(transform_a.first, transform_a.second);

    occ::Mat3N pos_A = mol_A.positions();
    occ::Mat3N pos_A_t = A.positions() * BOHR_TO_ANGSTROM;

    assert(all_close(pos_A, pos_A_t, 1e-5, 1e-5));

    auto transform_b = calculate_transform(wfnb, mol_B, crystal);
    B.apply_transformation(transform_b.first, transform_b.second);

    const auto &pos_B = mol_B.positions();
    const auto pos_B_t = B.positions() * BOHR_TO_ANGSTROM;
    assert(all_close(pos_A, pos_A_t, 1e-5, 1e-5));

    auto model = occ::interaction::ce_model_from_string(model_name);

    CEModelInteraction interaction(model);

    auto interaction_energy = interaction(A, B);
    return interaction_energy;
}

int main(int argc, char *argv[]) {
    cxxopts::Options options("occ-elat", "Crystal lattice energy");

    double radius = 3.8;
    double increment = 3.8;
    using occ::parallel::nthreads;
    std::string cif_name;
    options.add_options()("i,input", "Input CIF",
                          cxxopts::value<std::string>(cif_name))(
        "t,threads", "Number of threads",
        cxxopts::value<int>(nthreads)->default_value("1"))(
        "ce-hf", "Use CE-HF model",
        cxxopts::value<bool>()->default_value("false"))(
        "r,radius", "maximum radius (angstroms) for neighbours",
        cxxopts::value<double>(radius)->default_value("30.0"))(
        "radius-increment", "step size (angstroms) for direct space summation",
        cxxopts::value<double>(increment)->default_value("3.8"));

    occ::log::set_level(occ::log::level::info);
    spdlog::set_level(spdlog::level::info);
    libint2::Shell::do_enforce_unit_normalization(true);
    libint2::initialize();
    std::string model_name = "ce-b3lyp";

    try {
        auto result = options.parse(argc, argv);
        if (result.count("ce-hf"))
            model_name = "ce-hf";
    } catch (const std::runtime_error &err) {
        occ::log::error("error when parsing command line arguments: {}",
                        err.what());
        fmt::print("{}\n", options.help());
        exit(1);
    }

    fmt::print("Parallelized over {} threads & {} Eigen threads\n", nthreads,
               Eigen::nbThreads());

    const std::string error_format =
        "Exception:\n    {}\nTerminating program.\n";
    try {
        std::string filename = cif_name;
        std::string basename = fs::path(filename).stem();
        Crystal c = read_crystal(filename);
        fmt::print("Energy model: {}\n", model_name);
        fmt::print("Loaded crystal from {}\n", filename);
        auto molecules = c.symmetry_unique_molecules();
        fmt::print("Symmetry unique molecules in {}: {}\n", filename,
                   molecules.size());
        auto wfns = calculate_wavefunctions(basename, molecules, model_name);
        compute_monomer_energies(wfns);
        fmt::print("Calculating symmetry unique dimers\n");
	occ::crystal::CrystalDimers crystal_dimers;
	std::vector<CEEnergyComponents> energies;
	occ::main::LatticeConvergenceSettings settings;
	settings.max_radius = radius;
	std::tie(crystal_dimers, energies) = occ::main::converged_lattice_energies(c, wfns, wfns, basename, settings);

        const auto &dimers = crystal_dimers.unique_dimers;
        if (dimers.size() < 1) {
            fmt::print("No dimers found using neighbour radius {:.3f}\n",
                       radius);
            exit(0);
        }

        const std::string row_fmt_string =
            "{:>7.3f} {:>7.3f} {:>20s} {: 8.3f} {: 8.3f} {: 8.3f} {: 8.3f} {: "
            "8.3f}\n";
	size_t mol_idx{0};
	double etot{0.0}, elat{0.0};
	for (const auto &n: crystal_dimers.molecule_neighbors) {

	    fmt::print("Neighbors for molecule {}\n", mol_idx);

	    fmt::print("{:>7s} {:>7s} {:>20s} {:>8s} {:>8s} {:>8s} {:>8s} "
		       "{:>8s}\n",
		       "Rn", "Rc", "Symop", "E_coul", "E_rep", "E_pol",
		       "E_disp", "E_tot");
	    fmt::print("==================================================="
		       "================================\n");
	    CEEnergyComponents molecule_total;

	    size_t j = 0;
	    for (const auto &dimer : n) {
		auto s_ab = dimer_symop(dimer, c);
		size_t idx = crystal_dimers.unique_dimer_idx[mol_idx][j];
		double rn = dimer.nearest_distance();
		double rc = dimer.center_of_mass_distance();
		const auto &e = energies[idx];
		if(!e.is_computed) {
		    j++;
		    continue;
		}
		double ecoul = e.coulomb_kjmol(), erep = e.exchange_kjmol(),
		       epol = e.polarization_kjmol(),
		       edisp = e.dispersion_kjmol(), etot = e.total_kjmol();
		molecule_total.coulomb += ecoul;
		molecule_total.exchange_repulsion += erep;
		molecule_total.polarization += epol;
		molecule_total.dispersion += edisp;
		molecule_total.total += etot;
		fmt::print(row_fmt_string, rn, rc, s_ab, ecoul, erep, epol,
			   edisp, etot);
		j++;
	    }
	    fmt::print("Molecule {} total: {:.3f} kJ/mol ({} pairs)\n", mol_idx,
		       molecule_total.total, j);
	    etot += molecule_total.total;
        }
        fmt::print("Final energy: {:.3f} kJ/mol\n", etot * 0.5);

    } catch (const char *ex) {
        fmt::print(error_format, ex);
        return 1;
    } catch (std::string &ex) {
        fmt::print(error_format, ex);
        return 1;
    } catch (std::exception &ex) {
        fmt::print(error_format, ex.what());
        return 1;
    } catch (...) {
        fmt::print("Exception:\n- Unknown...\n");
        return 1;
    }

    return 0;
}
