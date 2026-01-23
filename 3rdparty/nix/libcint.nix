{
  stdenv,
  lib,
  fetchFromGitHub,
  cmake,
  blas,
  # Check Inputs
  python3,
}:

stdenv.mkDerivation rec {
  pname = "libcint";
  version = "6.1.3";

  src = fetchFromGitHub {
    owner = "peterspackman";
    repo = "libcint";
    rev = "master";
    hash = "sha256-JWk1B+Fz5nHxnGI5WlSynNqvkqIRvkbba8Nx3I5Tziw=";
  };

  postPatch = ''
    sed -i 's/libcint.so/libcint${stdenv.hostPlatform.extensions.sharedLibrary}/g' testsuite/*.py
  '';

  nativeBuildInputs = [ cmake ];
  buildInputs = [ blas ];
  cmakeFlags = [
    "-DENABLE_TEST=0"
    "-DQUICK_TEST=0"
    "-DCMAKE_INSTALL_PREFIX=" # ends up double-adding /nix/store/... prefix, this avoids issue
    "-DWITH_FORTRAN:STRING=OFF"
    "-DWITH_CINT2_INTERFACE=OFF"
    "-DENABLE_STATIC=ON"
    "-DBUILD_SHARED_LIBS=OFF"
    "-DPYPZPX=ON"
    "-DBUILD_MARCH_NATIVE=ON"
    "-DWITH_RANGE_COULOMB=ON"
  ];

  strictDeps = true;

  doCheck = true;
  nativeCheckInputs = [ python3.pkgs.numpy ];

  meta = {
    description = "General GTO integrals for quantum chemistry";
    longDescription = ''
      libcint is an open source library for analytical Gaussian integrals.
      It provides C/Fortran API to evaluate one-electron / two-electron
      integrals for Cartesian / real-spheric / spinor Gaussian type functions.
    '';
    homepage = "http://wiki.sunqm.net/libcint";
    downloadPage = "https://github.com/sunqm/libcint";
    changelog = "https://github.com/sunqm/libcint/blob/master/ChangeLog";
    license = lib.licenses.bsd2;
    maintainers = [ ];
    platforms = lib.platforms.unix;
  };
}
