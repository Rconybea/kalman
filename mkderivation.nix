# i.e. lambda pkgs . lambda attrs . (with pkgs ... )
# expected to be invoked like
#   let mkDerivation = import path/to/autotools.nix pkgs;
# in
#   mkDerivation { ... };
# 
pkgs: attrs:
  # any attributes supplied in argument attrs replace the default values below
  with pkgs;

  let
    # this sets NIX_CC etc to clang-13.0.1;
    # omitting it picks up the default clang-11.1.0 (as of 25may2022 at least)
    #
    #stdenv = clang13Stdenv;

    # attributes below will be passed as environment variables
    # to ${codebase}/builder.sh,  after overriding at least
    #   .name, .args, .setup
    # for example see nxfs/hello/{default.nix, setup.sh}
    #
    defaultAttrs = rec {
      name = "please-override-name";

      builder = "${bash}/bin/bash";

      # args, setup: these defaults only work
      # if .sh files are in the same directory as this autotools.nix file.
      # expect to override these in ${subsystem}.nix
      #
      args = [ ./stub-builder.sh ];
      setup = ./stub-setup.sh;

      # need next two things on darwin
      # in nixpkgs, clang -> 11.1.0 (as of 25may2022).
      # works for building regular executables,  but llvm-cov seems to be glitched
      # tried switching to clang_13,  however some tooling seems to insist on pulling
      # clang11 libraries.
      #
      #localgcc = (if builtins.currentSystem == "aarch64-darwin" then clang_13 else gcc);
      #localbinutils = (if builtins.currentSystem == "aarch64-darwin" then clang_13.bintools.bintools_bin else binutils-unwrapped);
      #
      localgcc = (if builtins.currentSystem == "aarch64-darwin" then clang else gcc);
      localbinutils = (if builtins.currentSystem == "aarch64-darwin" then clang.bintools.bintools_bin else binutils-unwrapped);

      # all autotools builds will get these packages
      # (don't really need 'which',  this is for debugging)
      #
      baseInputs = [ localgcc
      		     localbinutils ]
                    ++ [ gnumake
		         texinfo
			 gnugrep
			 gawk
			 gnused
			 gnutar
			 xz
			 gzip
			 file
			 diffutils
			 which
			 stdenv
			 coreutils
			 findutils ];
      buildInputs = [];

      system = builtins.currentSystem;

#      shellHook = ''XYZ=foo
#        '';
    };

    in
    stdenv.mkDerivation (defaultAttrs // attrs)
  