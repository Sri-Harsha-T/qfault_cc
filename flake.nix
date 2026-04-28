{
  description = "QFault — fault-tolerant quantum compiler (reproducible Nix build per ADR-0017)";

  inputs = {
    nixpkgs.url      = "github:NixOS/nixpkgs/nixos-24.05";
    flake-utils.url  = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils, ... }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };

        # Pinned to match cmake/dependency_versions.cmake.
        # Bumping these requires updating dependency_versions.cmake AND
        # Dockerfile in lockstep.
        gcc       = pkgs.gcc13;
        clang     = pkgs.clang_18;
        cmakePkg  = pkgs.cmake;

        commonNativeBuildInputs = [
          cmakePkg
          pkgs.ninja
          pkgs.git
          pkgs.python3
          pkgs.pkg-config
        ];
      in
      {
        ##
        ## Default development shell — drop in here with `nix develop`.
        ##
        devShells.default = pkgs.mkShell {
          name = "qfault-dev";
          packages = commonNativeBuildInputs ++ [
            gcc
            clang
            pkgs.clang-tools_18    # clang-tidy / clang-format
            pkgs.gdb
            pkgs.lldb
          ];

          shellHook = ''
            echo "QFault dev shell — gcc-13 / clang-18 / cmake $(cmake --version | head -1)"
            echo "Configure with:  cmake --preset gcc13-debug"
            echo "Build with:      cmake --build build/gcc13-debug -j"
            echo "Test with:       ctest --test-dir build/gcc13-debug -j"
          '';
        };

        ##
        ## Default package — `nix build` produces the libqfault static lib + tests.
        ##
        packages.default = pkgs.stdenv.mkDerivation {
          pname   = "qfault";
          version = "0.1.0";
          src     = ./.;

          nativeBuildInputs = commonNativeBuildInputs;
          buildInputs       = [ gcc ];

          configurePhase = ''
            runHook preConfigure
            cmake --preset gcc13-release \
              -DCMAKE_INSTALL_PREFIX=$out
            runHook postConfigure
          '';

          buildPhase = ''
            runHook preBuild
            cmake --build build/gcc13-release -j $NIX_BUILD_CORES
            runHook postBuild
          '';

          # Stage-1+2 invariant: tests pass on every release build.
          doCheck = true;
          checkPhase = ''
            runHook preCheck
            ctest --test-dir build/gcc13-release --output-on-failure -j $NIX_BUILD_CORES
            runHook postCheck
          '';

          installPhase = ''
            runHook preInstall
            mkdir -p $out
            # Headers and static lib (when install rules are added in Stage 5)
            cp -r build/gcc13-release/. $out/build/ 2>/dev/null || true
            cp -r include $out/include
            runHook postInstall
          '';

          meta = with pkgs.lib; {
            description = "Fault-tolerant quantum compiler (Clifford+T → surface code)";
            homepage    = "https://github.com/Sri-Harsha-T/qfault_cc";
            license     = licenses.asl20;
            platforms   = platforms.linux ++ platforms.darwin;
          };
        };
      });
}
