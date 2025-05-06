{
  description = "fpga-assembler";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/2c8d3f48d33929642c1c12cd243df4cc7d2ce434";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
    }@inputs:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs {
          inherit system;
        };
        common = with pkgs; {
          bazel = bazel_7;
          jdk = jdk;
        };
      in
      {
        devShells.default =
          let
            # There is too much volatility between even micro-versions of
            # newer clang-format. Use slightly older version for now.
            clang_for_formatting = pkgs.llvmPackages_17.clang-tools;

            # clang tidy: use latest.
            clang_for_tidy = pkgs.llvmPackages_18.clang-tools;
          in
          with pkgs;
          pkgs.mkShell {
            packages = with pkgs; [
              git
              common.bazel
              common.jdk
              bash
              gdb

              # For clang-tidy and clang-format.
              clang_for_formatting
              clang_for_tidy

              # For buildifier, buildozer.
              bazel-buildtools
              bant

              # Profiling and sanitizers.
              linuxPackages_latest.perf
              pprof
              perf_data_converter
              valgrind

              # FPGA utils.
              openfpgaloader
            ];

            CLANG_TIDY = "${clang_for_tidy}/bin/clang-tidy";
            CLANG_FORMAT = "${clang_for_formatting}/bin/clang-format";

            shellHook = ''
              exec bash
            '';
          };

        # Package fpga-assembler.
        packages.default =
          (pkgs.callPackage (
            {
              buildBazelPackage,
              stdenv,
              fetchFromGitHub,
              lib,
              nix-gitignore,
            }:
            let
              system = stdenv.hostPlatform.system;
              registry = fetchFromGitHub {
                owner = "bazelbuild";
                repo = "bazel-central-registry";
                rev = "63f3af762b2fdd7acaa7987856cd3ac314eaea09";
                hash = "sha256-ugNzoP0gdrhl9vH1TRdwoevuTsSqjitXnAoMSSTlCgI=";
              };
            in
            buildBazelPackage {
              pname = "fpga-as";

              version = "0.0.1";

              src = nix-gitignore.gitignoreSourcePure [ ] ./.;

              bazelFlags = [
                "--registry"
                "file://${registry}"
              ];

              postPatch = ''
                patchShebangs scripts/create-workspace-status.sh
              '';

              fetchAttrs = {
                hash =
                  {
                    aarch64-linux = "sha256-E4VHjDa0qkHmKUNpTBfJi7dhMLcd1z5he+p31/XvUl8=";
                    x86_64-linux = "sha256-hVBJB0Hsd9sXuEoNcjhTkbPl89vlZT1w39JppCD+n8Y=";
                  }
                  .${system} or (throw "No hash for system: ${system}");
              };

              removeRulesCC = false;
              removeLocalConfigCc = false;
              removeLocalConfigSh = false;

              nativeBuildInputs = [
                common.jdk
                pkgs.git
                pkgs.bash
                # Convenient tool to enter into the sandbox and start debugging.
                pkgs.breakpointHook
              ];

              bazel = common.bazel;

              bazelBuildFlags = [ "-c opt" ];
              bazelTestTargets = [ "//..." ];
              bazelTargets = [ "//fpga:fpga-as" ];

              buildAttrs = {
                installPhase = ''
                  install -D --strip bazel-bin/fpga/fpga-as "$out/bin/fpga-as"
                '';
              };

              meta = {
                description = "Tool to convert FASM to FPGA bitstream.";
                homepage = "https://github.com/lromor/fpga-assembler";
                license = lib.licenses.asl20;
                platforms = lib.platforms.linux;
              };
            }
          ) { }).overrideAttrs
            (
              final: prev: {
                # Fixup the deps so they always contain correrct
                # shebangs paths pointing to the store.
                deps = prev.deps.overrideAttrs (
                  final: prev: {
                    installPhase =
                      ''
                        patchShebangs $bazelOut/external
                      ''
                      + prev.installPhase;
                  }
                );
              }
            );
      }
    );
}
