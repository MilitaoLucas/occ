{
  description = "A simple C++ project";
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    mdspan-src = {
      url = "github:kokkos/mdspan";
      flake = false;
    };
  };
  outputs =
    {
      self,
      nixpkgs,
      mdspan-src,
    }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
    in
    {
      # 1. The Build Artifact (nix build)
      packages.${system}.default = pkgs.stdenv.mkDerivation {
        name = "NoSpherA2";
        src = ./.;
        buildInputs = [
          pkgs.cmake
          pkgs.boost
          pkgs.cpm-cmake
          pkgs.cargo
          pkgs.mkl
        ];
        nativeBuildInputs = [ pkgs.cmake ];
        cmakeFlags = [
          "-DFETCHCONTENT_SOURCE_DIR_MYLIB=${mdspan-src}"
          "-DCPM_DOWNLOAD_LOCATION=${pkgs.cpm-cmake}/share/CPM/CPM.cmake"
          "-DCARGO_EXE=${pkgs.cargo/bin/cargo}"
          "-DMKL_PREFIX=${pkgs.mkl}/lib/cmake"
          "-DNIX_BUILD=ON"
        ];
        # CMake setup usually happens automatically via stdenv hooks
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
          export CARGO_EXE=${pkgs.cargo}/bin/cargo
          zsh
        '';
      };
    };
}
