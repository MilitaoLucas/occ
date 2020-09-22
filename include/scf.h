#pragma once
#include "ints.h"
#include <tuple>
#include <libint2/diis.h>
#include <libint2/chemistry/sto3g_atomic_density.h>
#include <Eigen/Dense>
#include <fmt/core.h>

namespace craso::scf
{

    using craso::MatRM;
    enum class SCFKind {
        rhf,
        uhf 
    };

    std::tuple<MatRM, MatRM, double> conditioning_orthogonalizer(const MatRM& S, double S_condition_number_threshold);

    // returns {X,X^{-1},rank,A_condition_number,result_A_condition_number}, where
    // X is the generalized square-root-inverse such that X.transpose() * A * X = I
    //
    // if symmetric is true, produce "symmetric" sqrtinv: X = U . A_evals_sqrtinv .
    // U.transpose()),
    // else produce "canonical" sqrtinv: X = U . A_evals_sqrtinv
    // where U are eigenvectors of A
    // rows and cols of symmetric X are equivalent; for canonical X the rows are
    // original basis (AO),
    // cols are transformed basis ("orthogonal" AO)
    //
    // A is conditioned to max_condition_number
    std::tuple<MatRM, MatRM, size_t, double, double> gensqrtinv(const MatRM& S, bool symmetric = false, double max_condition_number = 1e8);


    template <typename Procedure>
    struct SCF
    {
        SCF(Procedure &procedure, SCFKind kind = SCFKind::rhf) : m_procedure(procedure), diis(2)
        {
            scf_kind = kind;
            nelectrons = m_procedure.num_e();
            n_beta = nelectrons / 2;
            n_alpha = nelectrons - n_beta;
        }

        const auto& atoms() const { return m_procedure.atoms(); }

        MatRM compute_soad() const
        {
            // computes Superposition-Of-Atomic-Densities guess for the molecular density
            // matrix
            // in minimal basis; occupies subshells by smearing electrons evenly over the
            // orbitals
            // compute number of atomic orbitals
            size_t nao = 0;
            for (const auto &atom : atoms())
            {
                const auto Z = atom.atomic_number;
                nao += libint2::sto3g_num_ao(Z);
            }

            // compute the minimal basis density
            MatRM D = MatRM::Zero(nao, nao);
            size_t ao_offset = 0; // first AO of this atom
            for (const auto &atom : atoms())
            {
                const auto Z = atom.atomic_number;
                const auto &occvec = libint2::sto3g_ao_occupation_vector(Z);
                for (const auto &occ : occvec)
                {
                    D(ao_offset, ao_offset) = occ;
                    ++ao_offset;
                }
            }

            return D * 0.5; // we use densities normalized to # of electrons/2
        }

        void compute_initial_guess()
        {
            const auto tstart = std::chrono::high_resolution_clock::now();
            S = m_procedure.compute_overlap_matrix();
            // compute orthogonalizer X such that X.transpose() . S . X = I
            // one should think of columns of Xinv as the conditioned basis
            // Re: name ... cond # (Xinv.transpose() . Xinv) = cond # (X.transpose() .
            // X)
            // by default assume can manage to compute with condition number of S <=
            // 1/eps
            // this is probably too optimistic, but in well-behaved cases even 10^11 is
            // OK
            double S_condition_number_threshold = 1.0 / std::numeric_limits<double>::epsilon();
            std::tie(X, Xinv, XtX_condition_number) = conditioning_orthogonalizer(S, S_condition_number_threshold);
            T = m_procedure.compute_kinetic_matrix();
            V = m_procedure.compute_nuclear_attraction_matrix();
            H = T + V;
            auto D_minbs = compute_soad(); // compute guess in minimal basis
            libint2::BasisSet minbs("STO-3G", m_procedure.atoms());

            if (minbs == m_procedure.basis()) {
                if(scf_kind == SCFKind::rhf) {
                    D = D_minbs;
                }
                if(scf_kind == SCFKind::uhf) {
                    Da = D_minbs;
                    Db = D_minbs;
                }
            }
            else
            {
                // if basis != minimal basis, map non-representable SOAD guess
                // into the AO basis
                // by diagonalizing a Fock matrix
                if(verbose) fmt::print("Projecting SOAD into atomic orbital basis: ");
                F = H;
                F += craso::ints::compute_2body_fock_general(m_procedure.basis(), D_minbs, minbs, true, std::numeric_limits<double>::epsilon());

                // solve F C = e S C by (conditioned) transformation to F' C' = e C',
                // where
                // F' = X.transpose() . F . X; the original C is obtained as C = X . C'
                Eigen::SelfAdjointEigenSolver<MatRM> eig_solver(X.transpose() * F * X);
                C = X * eig_solver.eigenvectors();

                // compute density, D = C(occ) . C(occ)T
                if(scf_kind == SCFKind::rhf) {
                    C_occ = C.leftCols(n_alpha);
                    D = C_occ * C_occ.transpose();
                }
                if (scf_kind == SCFKind::uhf) {
                    Ca_occ = C.leftCols(n_alpha);
                    Cb_occ = C.leftCols(n_beta);
                    Da = Ca_occ * Ca_occ.transpose();
                    Db = Cb_occ * Cb_occ.transpose();
                }

                const auto tstop = std::chrono::high_resolution_clock::now();
                const std::chrono::duration<double> time_elapsed = tstop - tstart;
                if(verbose) fmt::print("{:.5f} s\n", time_elapsed.count());
            }
        }

        double compute_scf_energy()
        {
            if(scf_kind == SCFKind::rhf) return compute_scf_energy_restricted();
            if (scf_kind == SCFKind::uhf) return compute_scf_energy_unrestricted();
        }

        double compute_scf_energy_restricted()
        {
            // compute one-body integrals
            // count the number of electrons
            compute_initial_guess();
            K = m_procedure.compute_schwarz_ints();
            enuc = m_procedure.nuclear_repulsion_energy();
            MatRM D_diff;
            auto n2 = D.cols() * D.rows();
            MatRM evals;

            fmt::print("Beginning SCF\n");
            total_time = 0.0;
            do
            {
                const auto tstart = std::chrono::high_resolution_clock::now();
                ++iter;

                // Last iteration's energy and density
                auto ehf_last = ehf;
                MatRM D_last = D;

                if (not incremental_Fbuild_started &&
                    rms_error < start_incremental_F_threshold)
                {
                    incremental_Fbuild_started = true;
                    reset_incremental_fock_formation = false;
                    last_reset_iteration = iter - 1;
                    next_reset_threshold = rms_error / 1e1;
                    if (verbose) fmt::print("Starting incremental fock build\n");
                }
                if (reset_incremental_fock_formation || not incremental_Fbuild_started)
                {
                    F = H;
                    D_diff = D;
                }
                if (reset_incremental_fock_formation && incremental_Fbuild_started)
                {
                    reset_incremental_fock_formation = false;
                    last_reset_iteration = iter;
                    next_reset_threshold = rms_error / 1e1;
                    if (verbose) fmt::print("Resetting incremental fock build\n");
                }

                // build a new Fock matrix
                // totally empirical precision variation, involves the condition number
                const auto precision_F = std::min(
                    std::min(1e-3 / XtX_condition_number, 1e-7),
                    std::max(rms_error / 1e4, std::numeric_limits<double>::epsilon()));
                F += m_procedure.compute_2body_fock(D_diff, precision_F, K);

                // compute HF energy with the non-extrapolated Fock matrix
                ehf = D.cwiseProduct(H + F).sum();
                ediff_rel = std::abs((ehf - ehf_last) / ehf);

                // compute SCF error
                MatRM FD_comm = F * D * S - S * D * F;
                rms_error = FD_comm.norm() / n2;
                if (rms_error < next_reset_threshold || iter - last_reset_iteration >= 8)
                    reset_incremental_fock_formation = true;

                // DIIS extrapolate F
                MatRM F_diis = F; // extrapolated F cannot be used in incremental Fock
                                   // build; only used to produce the density
                                   // make a copy of the unextrapolated matrix
                diis.extrapolate(F_diis, FD_comm);

                // solve F C = e S C by (conditioned) transformation to F' C' = e C',
                // where
                // F' = X.transpose() . F . X; the original C is obtained as C = X . C'
                Eigen::SelfAdjointEigenSolver<MatRM> eig_solver(X.transpose() * F_diis *
                                                                 X);
                evals = eig_solver.eigenvalues();
                C = X * eig_solver.eigenvectors();

                // compute density, D = C(occ) . C(occ)T
                C_occ = C.leftCols(n_alpha);
                D = C_occ * C_occ.transpose();
                D_diff = D - D_last;

                const auto tstop = std::chrono::high_resolution_clock::now();
                const std::chrono::duration<double> time_elapsed = tstop - tstart;

                if (iter == 1) {
                    fmt::print("{:>6s} {: >20s} {: >20s} {: >20s} {: >10s}\n",
                            "cycle", "energy", "D(E)/E", "rms([F,D])/nn", "time");
                }
                fmt::print("{:>6d} {:>20.12f} {:>20.12e} {:>20.12e} {:>10.5f}\n", iter, ehf + enuc,
                       ediff_rel, rms_error, time_elapsed.count());
                total_time += time_elapsed.count();

            } while (((ediff_rel > conv) || (rms_error > conv)) && (iter < maxiter));
            fmt::print("SCF complete in {:.6f} s wall time\n", total_time);
            return ehf + enuc;
        }

        double compute_scf_energy_unrestricted()
        {
            // compute one-body integrals
            // count the number of electrons
            compute_initial_guess();
            K = m_procedure.compute_schwarz_ints();
            enuc = m_procedure.nuclear_repulsion_energy();
            MatRM Da_diff, Db_diff;
            auto n2a = Da.cols() * Da.rows();
            auto n2b = Db.cols() * Db.rows();
            MatRM evals_a, evals_b;

            fmt::print("Beginning SCF\n");
            total_time = 0.0;
            do
            {
                const auto tstart = std::chrono::high_resolution_clock::now();
                ++iter;

                // Last iteration's energy and density
                auto ehf_last = ehf;
                MatRM Da_last = Da, Db_last = Db;

                if (not incremental_Fbuild_started &&
                    rms_error < start_incremental_F_threshold)
                {
                    incremental_Fbuild_started = true;
                    reset_incremental_fock_formation = false;
                    last_reset_iteration = iter - 1;
                    next_reset_threshold = rms_error / 1e1;
                    if (verbose) fmt::print("Starting incremental fock build\n");
                }
                if (reset_incremental_fock_formation || not incremental_Fbuild_started)
                {
                    Fa = H; Fb = H;
                    Da_diff = Da;
                    Db_diff = Db;
                }
                if (reset_incremental_fock_formation && incremental_Fbuild_started)
                {
                    reset_incremental_fock_formation = false;
                    last_reset_iteration = iter;
                    next_reset_threshold = rms_error / 1e1;
                    if (verbose) fmt::print("Resetting incremental fock build\n");
                }

                // build a new Fock matrix
                // totally empirical precision variation, involves the condition number
                const auto precision_Fa = std::min(
                    std::min(1e-3 / XtX_condition_number, 1e-7),
                    std::max(rms_error / 1e4, std::numeric_limits<double>::epsilon()));
                const auto precision_Fb = std::min(
                    std::min(1e-3 / XtX_condition_number, 1e-7),
                    std::max(rms_error / 1e4, std::numeric_limits<double>::epsilon()));
                const auto precision_F = std::min(precision_Fa, precision_Fb);
                MatRM Fa_tmp, Fb_tmp;
                std::tie(Fa_tmp, Fb_tmp) = m_procedure.compute_2body_fock_unrestricted(Da_diff, Db_diff, precision_F, K);
                Fa += Fa_tmp;
                Fb += Fb_tmp;

                // compute HF energy with the non-extrapolated Fock matrix
                ehf = Da.cwiseProduct(H + Fa).sum() + Db.cwiseProduct(H + Fb).sum();
                ediff_rel = std::abs((ehf - ehf_last) / ehf);

                // compute SCF error
                MatRM FD_comm_a = Fa * Da * S - S * Da * Fa;
                MatRM FD_comm_b = Fb * Db * S - S * Db * Fb;
                rms_error = FD_comm_a.norm() / n2a + FD_comm_b.norm() / n2b;
                if (rms_error < next_reset_threshold || iter - last_reset_iteration >= 8)
                    reset_incremental_fock_formation = true;

                // DIIS extrapolate F
                MatRM Fa_diis = Fa; 
                diis.extrapolate(Fa_diis, FD_comm_a);

                MatRM Fb_diis = Fb; 
                diis.extrapolate(Fb_diis, FD_comm_b);

                // solve F C = e S C by (conditioned) transformation to F' C' = e C',
                // where
                // F' = X.transpose() . F . X; the original C is obtained as C = X . C'
                Eigen::SelfAdjointEigenSolver<MatRM> eig_solver_a(X.transpose() * Fa_diis * X);
                evals_a = eig_solver_a.eigenvalues();
                Ca = X * eig_solver_a.eigenvectors();

                Eigen::SelfAdjointEigenSolver<MatRM> eig_solver_b(X.transpose() * Fb_diis * X);
                evals_b = eig_solver_b.eigenvalues();
                Cb = X * eig_solver_b.eigenvectors();

                // compute density, D = C(occ) . C(occ)T
                Ca_occ = Ca.leftCols(n_alpha);
                Da = Ca_occ * Ca_occ.transpose();
                Da_diff = Da - Da_last;
                // beta
                Cb_occ = Cb.leftCols(n_beta);
                Db = Cb_occ * Cb_occ.transpose();
                Db_diff = Db - Db_last;



                const auto tstop = std::chrono::high_resolution_clock::now();
                const std::chrono::duration<double> time_elapsed = tstop - tstart;

                if (iter == 1) {
                    fmt::print("{:>6s} {: >20s} {: >20s} {: >20s} {: >10s}\n",
                            "cycle", "energy", "D(E)/E", "rms([F,D])/nn", "time");
                }
                fmt::print("{:>6d} {:>20.12f} {:>20.12e} {:>20.12e} {:>10.5f}\n", iter, ehf + enuc,
                       ediff_rel, rms_error, time_elapsed.count());
                total_time += time_elapsed.count();

            } while (((ediff_rel > conv) || (rms_error > conv)) && (iter < maxiter));
            fmt::print("SCF complete in {:.6f} s wall time\n", total_time);
            return ehf + enuc;
        }

        Procedure &m_procedure;
        SCFKind scf_kind{SCFKind::rhf};
        int nelectrons{0};
        int n_alpha{0};
        int n_beta{0};
        int maxiter{100};
        double conv = 1e-12;
        int iter = 0;
        double rms_error = 1.0;
        double ediff_rel = 0.0;
        double enuc{0.0};
        double ehf{0.0};
        double total_time{0.0};
        libint2::DIIS<MatRM> diis; // start DIIS on second iteration

        bool reset_incremental_fock_formation = false;
        bool incremental_Fbuild_started = false;
        double start_incremental_F_threshold = 1e-5;
        double next_reset_threshold = 0.0;
        size_t last_reset_iteration = 0;
        MatRM D, S, T, V, H, K, X, Xinv, C, C_occ, F;
        MatRM Da, Ca, Ca_occ, Fa;
        MatRM Db, Cb, Cb_occ, Fb;
        double XtX_condition_number;
        bool verbose{false};
    };
} // namespace craso::scf
