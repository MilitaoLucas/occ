#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <filesystem>
#include <occ/core/molecule.h>
#include <occ/dma/dma.h>
#include <occ/dma/linear_multipole_calculator.h>
#include <occ/io/fchkreader.h>
#include <occ/qm/hf.h>
#include <occ/qm/scf.h>
#include <occ/qm/wavefunction.h>
#include <occ/core/data_directory.h>

using namespace occ;
using Catch::Approx;
using namespace occ::dma;
namespace fs = std::filesystem;

fs::path test_directory() {
  const char* data_dir = occ::get_data_directory();
  return fs::path(data_dir ? data_dir : ".") / "test_files";
}

TEST_CASE("DMA linear", "[dma]") {
  using namespace occ::qm;
  using namespace occ::io;
  FchkReader reader((test_directory() /  "common_files" /  "fchk_contents.fchk").string());

  occ::qm::Wavefunction wfn(reader);

  SECTION("Basis normalization") {
    const Vec3 coeffs_dma(0.27693435, 0.26783885, 0.08347367);
    const auto &shell = wfn.basis.shells()[0];
    for (int i = 0; i < 3; i++) {
      REQUIRE(shell.coeff_normalized_dma(0, i) ==
              Approx(coeffs_dma(i)).margin(1e-6));
    }
  }

  // Setup settings for the linear calculator
  LinearDMASettings settings;
  settings.max_rank = 2;
  settings.include_nuclei = true;
  settings.use_slices = false;

  // Create and use the linear multipole calculator
  LinearMultipoleCalculator calculator(wfn, settings);
  auto result = calculator.calculate();

  // Check the result
  REQUIRE(result.size());

  Mat expected(3, 2);
  expected << 0.0, 0.0, 0.19113723, -0.19113723, -0.11109289, -0.11109289;

  for (int site = 0; site < 2; site++) {
    const auto &m = result[site];
    for (int term = 0; term < 3; term++) {
      REQUIRE(m.q(term) == Approx(expected(term, site)).margin(1e-6));
    }
  }
}

TEST_CASE("DMA general", "[dma]") {
  using namespace occ::qm;
  using namespace occ::io;
  FchkReader reader((test_directory() / "dma" / "h2o_contents.fchk").string());

  occ::qm::Wavefunction wfn(reader);

  occ::dma::DMACalculator calc(wfn);

  occ::dma::DMASettings settings;
  settings.max_rank = 2;
  settings.big_exponent = 0.0;

  calc.update_settings(settings);
  calc.set_radius_for_element(1, 0.325);

  // Test DMA calculation (analytical method)
  auto dma_result = calc.compute_multipoles();
  const auto &result = dma_result.multipoles;

  // Check the result
  REQUIRE(result.size() > 0);

  auto expected = Mat(3, 12);

  expected << -0.704577, -0.004420, 0.102103, 0.169772, -0.397276, 0.003845,
      -0.013801, 0.104311, -0.233917, 0.000000, 0.000000, 0.000000, 0.352029,
      -0.000939, -0.014779, 0.039620, -0.013593, 0.000254, -0.000950, -0.016151,
      -0.011038, 0.000000, 0.000000, 0.000000, 0.352548, -0.000192, 0.043710,
      0.003513, -0.013054, -0.000235, -0.000130, 0.017866, 0.005581, 0.000000,
      0.000000, 0.000000;

  for (int site = 0; site < 3; site++) {
    const auto &m = result[site];
    for (int term = 0; term < 12; term++) {
      REQUIRE(m.q(term) == Approx(expected(site, term)).margin(1e-6));
    }
  }
}

TEST_CASE("DMA general4", "[dma]") {
  using namespace occ::qm;
  using namespace occ::io;
  FchkReader reader((test_directory() / "dma" / "h2o_contents.fchk").string());

  occ::qm::Wavefunction wfn(reader);

  occ::dma::DMACalculator calc(wfn);

  occ::dma::DMASettings settings;
  settings.max_rank = 2;
  settings.big_exponent = 4.0;

  calc.update_settings(settings);
  calc.set_radius_for_element(1, 0.325);

  // Test DMA calculation (analytical method)
  auto dma_result = calc.compute_multipoles();
  const auto &result = dma_result.multipoles;

  // Check the result
  REQUIRE(result.size() > 0);

  auto expected = Mat(3, 12);

  expected << -0.427605, -0.013835, 0.327911, 0.530539, -1.009920, 0.006672,
      -0.036657, 0.214050, -0.463494, 0.000000, 0.000000, 0.000000, 0.212067,
      0.001162, 0.018114, -0.049016, -0.107774, 0.002061, -0.006636, -0.092154,
      -0.093068, 0.000000, 0.000000, 0.000000, 0.215539, 0.000231, -0.051145,
      -0.004406, -0.107397, -0.001061, -0.001414, 0.129761, 0.012256, 0.000000,
      0.000000, 0.000000;

  for (int site = 0; site < 3; site++) {
    const auto &m = result[site];
    for (int term = 0; term < 12; term++) {
      REQUIRE(m.q(term) == Approx(expected(site, term)).margin(1e-3));
    }
  }
}

TEST_CASE("C2H4 G03", "[dma]") {
  using namespace occ::qm;
  using namespace occ::io;
  FchkReader reader((test_directory() /  "dma" /  "c2h4_contents.fchk").string());

  occ::qm::Wavefunction wfn(reader);

  occ::dma::DMACalculator calc(wfn);

  occ::dma::DMASettings settings;
  settings.max_rank = 4;
  settings.big_exponent = 4.0;

  calc.update_settings(settings);
  calc.set_radius_for_element(1, 0.35);
  calc.set_limit_for_element(1, 1);

  auto t1 = std::chrono::high_resolution_clock::now();
  // Test DMA calculation (analytical method)
  auto dma_result = calc.compute_multipoles();
  const auto &result = dma_result.multipoles;
  auto t2 = std::chrono::high_resolution_clock::now();

  auto duration = std::chrono::duration<double, std::milli>(t2 - t1);
  occ::timing::print_timings();

  // Check the result
  REQUIRE(result.size());

  occ::Mat expected(6, 12);
  expected.setZero();
  expected << -0.035811, 0.095601, 0.000000, -0.000000, 0.731238, -0.000000,
      0.000000, -1.399558, 0.000000, -2.874517, 0.000000, -0.000000, -0.035811,
      -0.095601, 0.000000, -0.000000, 0.731238, -0.000000, -0.000000, -1.399558,
      0.000000, 2.874517, -0.000000, 0.000000, 0.017905, -0.067488, 0.000000,
      -0.070550, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.017905, -0.067488, -0.000000, 0.070550, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.017905, 0.067488, -0.000000, -0.070550, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.017905, 0.067488,
      -0.000000, 0.070550, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000;

  fmt::print("expected\n{}\n", format_matrix(expected));

  for (int site = 0; site < 6; site++) {
    int lm = (site < 2) ? 4 : 1;
    fmt::print("Site: {}\n{}\n", site, result[site].to_string(lm));
    const auto &m = result[site];
    for (int term = 0; term < 12; term++) {
      CAPTURE(site, term);
      CHECK(m.q(term) == Approx(expected(site, term)).margin(1e-3));
    }
  }
}

TEST_CASE("Mult accessor functions", "[dma]") {
  // Create a test multipole with rank 4
  Mult mult(4);

  // Set some test values using direct access
  mult.q(0) = 1.5;    // Q00
  mult.q(1) = -0.3;   // Q10
  mult.q(2) = 0.7;    // Q11c
  mult.q(3) = -0.2;   // Q11s
  mult.q(4) = 0.9;    // Q20
  mult.q(5) = 0.4;    // Q21c
  mult.q(6) = -0.6;   // Q21s
  mult.q(7) = 0.1;    // Q22c
  mult.q(8) = 0.8;    // Q22s
  mult.q(16) = 2.1;   // Q40
  mult.q(23) = -1.2;  // Q44c
  mult.q(24) = 0.5;   // Q44s

  SECTION("Test component_name_to_lm helper function") {
    // Test various component name formats
    CHECK(Mult::component_name_to_lm("charge") == std::make_pair(0, 0));
    CHECK(Mult::component_name_to_lm("Q00") == std::make_pair(0, 0));
    CHECK(Mult::component_name_to_lm("Q10") == std::make_pair(1, 0));
    CHECK(Mult::component_name_to_lm("Q11c") == std::make_pair(1, 1));
    CHECK(Mult::component_name_to_lm("Q11s") == std::make_pair(1, -1));
    CHECK(Mult::component_name_to_lm("Q20") == std::make_pair(2, 0));
    CHECK(Mult::component_name_to_lm("Q21c") == std::make_pair(2, 1));
    CHECK(Mult::component_name_to_lm("Q21s") == std::make_pair(2, -1));
    CHECK(Mult::component_name_to_lm("Q22c") == std::make_pair(2, 2));
    CHECK(Mult::component_name_to_lm("Q22s") == std::make_pair(2, -2));
    CHECK(Mult::component_name_to_lm("Q40") == std::make_pair(4, 0));
    CHECK(Mult::component_name_to_lm("Q44c") == std::make_pair(4, 4));
    CHECK(Mult::component_name_to_lm("Q44s") == std::make_pair(4, -4));

    // Test invalid names
    CHECK(Mult::component_name_to_lm("invalid") == std::make_pair(-1, 0));
    CHECK(Mult::component_name_to_lm("Q") == std::make_pair(-1, 0));
    CHECK(Mult::component_name_to_lm("Q1") == std::make_pair(-1, 0));
  }

  SECTION("Test get_multipole vs inline accessors") {
    // Test rank 0 (monopole)
    CHECK(mult.get_multipole(0, 0) == Approx(mult.Q00()));
    CHECK(mult.get_multipole(0, 0) == Approx(mult.charge()));

    // Test rank 1 (dipole)
    CHECK(mult.get_multipole(1, 0) == Approx(mult.Q10()));
    CHECK(mult.get_multipole(1, 1) == Approx(mult.Q11c()));
    CHECK(mult.get_multipole(1, -1) == Approx(mult.Q11s()));

    // Test rank 2 (quadrupole)
    CHECK(mult.get_multipole(2, 0) == Approx(mult.Q20()));
    CHECK(mult.get_multipole(2, 1) == Approx(mult.Q21c()));
    CHECK(mult.get_multipole(2, -1) == Approx(mult.Q21s()));
    CHECK(mult.get_multipole(2, 2) == Approx(mult.Q22c()));
    CHECK(mult.get_multipole(2, -2) == Approx(mult.Q22s()));

    // Test rank 4 (hexadecapole)
    CHECK(mult.get_multipole(4, 0) == Approx(mult.Q40()));
    CHECK(mult.get_multipole(4, 4) == Approx(mult.Q44c()));
    CHECK(mult.get_multipole(4, -4) == Approx(mult.Q44s()));
  }

  SECTION("Test get_component vs inline accessors") {
    // Test using component names
    CHECK(mult.get_component("charge") == Approx(mult.charge()));
    CHECK(mult.get_component("Q00") == Approx(mult.Q00()));
    CHECK(mult.get_component("Q10") == Approx(mult.Q10()));
    CHECK(mult.get_component("Q11c") == Approx(mult.Q11c()));
    CHECK(mult.get_component("Q11s") == Approx(mult.Q11s()));
    CHECK(mult.get_component("Q20") == Approx(mult.Q20()));
    CHECK(mult.get_component("Q21c") == Approx(mult.Q21c()));
    CHECK(mult.get_component("Q21s") == Approx(mult.Q21s()));
    CHECK(mult.get_component("Q22c") == Approx(mult.Q22c()));
    CHECK(mult.get_component("Q22s") == Approx(mult.Q22s()));
    CHECK(mult.get_component("Q40") == Approx(mult.Q40()));
    CHECK(mult.get_component("Q44c") == Approx(mult.Q44c()));
    CHECK(mult.get_component("Q44s") == Approx(mult.Q44s()));
  }

  SECTION("Test boundary conditions") {
    // Test invalid l,m combinations
    CHECK(mult.get_multipole(-1, 0) == 0.0);  // Negative rank
    CHECK(mult.get_multipole(1, 2) == 0.0);   // |m| > l
    CHECK(mult.get_multipole(1, -2) == 0.0);  // |m| > l
    CHECK(mult.get_multipole(10, 0) == 0.0);  // Rank > max_rank

    // Test invalid component names
    CHECK(mult.get_component("invalid") == 0.0);
    CHECK(mult.get_component("Q99c") == 0.0);  // Beyond max rank
    CHECK(mult.get_component("Q1x") == 0.0);   // Invalid format
  }

  SECTION("Test modifiable accessors") {
    // Test that we can modify values through the new accessors
    mult.get_multipole(3, 1) = 99.9;
    CHECK(mult.Q31c() == Approx(99.9));

    mult.get_component("Q32s") = -88.8;
    CHECK(mult.Q32s() == Approx(-88.8));
    CHECK(mult.get_multipole(3, -2) == Approx(-88.8));
  }
}
