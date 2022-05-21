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

      localcxxdev = (if builtins.currentSystem == "aarch64-darwin" then [ libcxx.dev ] else []);

      # inherit $x -> shell variable ${x} will be defined
      # when ./builder.sh runs, value will be the nix store location for nixpkgs.${x},
      # which will typically be the build artifact for a nix derivation
      #
      buildInputs = localcxxdev ++ [ pkg-config eigen cmake boost which ];
      devInputs = [];
    };

  in
    mkDerivation (kalmanAttrs // attrs)

# kalman.nix
