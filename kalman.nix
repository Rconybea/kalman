# i.e. lambda pkgs . lambda attrs . (with pkgs ... )
# expected to be invoked like
#   let mkDerivation = import path/to/kalman.nix pkgs;
# in
#   mkDerivation { ... };
# 
pkgs: attrs:
  # any attributes supplied in argument attrs replace the default values below
  with pkgs;

  let
    mkDerivation = import ./mkderivation.nix pkgs;

    kalmanAttrs = rec {
      name = "kalman";
      args = [ ./builder.sh ];
      setup = ./setup.sh;
      srcdir = ./src;

      # want shell variable ${coreutils},
      # so we can get basic utilites (like cat) into $PATH,  before we try to
      # do things like run
      #   cat some/nix/pkg/nix-support/propagated-build-inputs
      # in setup.sh
      #
      inherit coreutils;

      # when ./builder.sh runs, value will be the nix store location for nixpkgs.${x},
      # which will typically be the build artifact for a nix derivation.
      #
      # libcxxdev: if omitted, we get linking problems for -lstdc++
      # llvmPackages.libcxx llvmPackages_13.llvm:
      #            tried adding for llvm-cov.   Unfortunately collides with clang-11,  which is
      #            being forced somewhere else (not sure where).
      #		   for non-darwin build expect to use gcc/gcov/lcov
      #            see also localgcc in mkderivation.nix,  presumably llvm+clang versions
      #            should coordinate      		   
      #
      #localcxxdev = (if builtins.currentSystem == "aarch64-darwin" then [ libcxx.dev llvmPackages_13.llvm ] else []);
      #localcxxdev = (if builtins.currentSystem == "aarch64-darwin" then [ llvmPackages_13.libcxx llvmPackages_13.llvm ] else []);
      localcxxdev = (if builtins.currentSystem == "aarch64-darwin" then [ llvm ] else []);
      buildInputs = localcxxdev ++ [ pkg-config eigen cmake boost which catch2 ];

      devInputs = [];
    };

  in
    mkDerivation (kalmanAttrs // attrs)

# kalman.nix
