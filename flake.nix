{
  description = "Open Computational Chemestry";
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
  };
  outputs =
    {
      self,
      nixpkgs,
    }:
    let
      fast-float-6_1_6 = pkgs.fast-float.overrideAttrs (old: rec {
        version = "6.1.6";
        src = pkgs.fetchFromGitHub {
          owner = "fastfloat";
          repo = "fast_float";
          rev = "v${version}";
          hash = "sha256-MEJMPQZZZhOFiKlPAKIi0zVzaJBvjAlbSyg3wLOQ1fg=";
        };
      });
      cli11-2_4_2 = pkgs.cli11.overrideAttrs (old: rec {
        version = "2.4.2";
        src = pkgs.fetchFromGitHub {
          owner = "CLIUtils";
          repo = "CLI11";
          rev = "v${version}";
          hash = "sha256-73dfpZDnKl0cADM4LTP3/eDFhwCdiHbEaGRF7ZyWsdQ=";
        };
      });
      gemmi = pkgs.gemmi.overrideAttrs (old: rec {
        version = "0.6.5";
        src = pkgs.fetchFromGitHub {
          owner = "project-gemmi";
          repo = "gemmi";
          tag = "v${version}";
          hash = "sha256-JJ6YBsdL3J+d0ihuJ2Nowp40c7FkDdfTqBhDrxWgSFw=";
        };

      });
      dftd4 = pkgs.callPackage ./3rdparty/nix/dftd4.nix { };
      scnlib = pkgs.callPackage ./3rdparty/nix/scnlib.nix {
        fast-float = fast-float-6_1_6;
      };
      lbfgspp-src = pkgs.fetchFromGitHub {
        owner = "yixuan";
        repo = "LBFGSpp";
        rev = "master";
        hash = "sha256-PUzZ2jUVgHq1LJDWIWW93KnV7vBBEdZlspOrE5TcYBc=";
      };
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};

    in
    {
      # 1. The Build Artifact (nix build)
      packages.${system} = {
        default = pkgs.stdenv.mkDerivation {
          name = "occ";
          src = ./.;
          buildInputs = [
            pkgs.cmake
            # Project dependencies installed with cpm.cmake
            pkgs.cpm-cmake
            pkgs.pkg-config
            pkgs.spdlog
            pkgs.onetbb
            pkgs.tomlplusplus
            pkgs.unordered_dense
            pkgs.fmt
            pkgs.nlohmann_json
            pkgs.eigen
            scnlib
            cli11-2_4_2
          ];
          nativeBuildInputs = [
            pkgs.cmake
            pkgs.pkg-config
          ];
          cmakeFlags = [
            "-DCPM_DOWNLOAD_LOCATION=${pkgs.cpm-cmake}/share/cpm/CPM.cmake"
            "-DCPM_USE_LOCAL_PACKAGES=ON"
            "-DCPM_spdlog_SOURCE=${pkgs.spdlog.src}"
            "-DCPM_oneTBB_SOURCE=${pkgs.onetbb.src}"
            "-DCPM_scnlib_SOURCE=${scnlib.src}"
            "-DCPM_eigen3_SOURCE=${pkgs.eigen.src}"
            "-DCPM_gemmi_SOURCE=${gemmi.src}"
            "-Dfmt_DIR=${pkgs.fmt.dev}/lib/cmake/fmt"
            "-Dcli11_DIR=${cli11-2_4_2}/lib/cmake/cli11"
            #"-Ddftd4_DIR=${dftd4.dev}/lib/cmake/dftd4" # Lets use src for now. This is bad for caching though
            "-DCPM_dftd4_cpp_SOURCE=${dftd4.src}"
            "-DCPM_LBFGSpp_SOURCE=${lbfgspp-src}"
            "-DFETCHCONTENT_SOURCE_DIR_FAST_FLOAT=${fast-float-6_1_6.src}"
            "-DNIX_BUILD=ON"
          ];
        };
        dftd4 = dftd4;
      };

      # 2. The Development Environment (nix develop)
      devShells.${system}.default = pkgs.mkShell {
        packages = with pkgs; [
          cmake
          cargo
          gcc
          clang-tools
          gdb
        ];

        # Environment variables for the shell
        shellHook = ''
          exec zsh
        '';
      };
    };
}
