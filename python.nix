# Use:
#   $ nix-shell ./python.nix
#

with import <nixpkgs> {};
(python39.withPackages (ps: [ps.numpy ps.jupyterlab])).env


