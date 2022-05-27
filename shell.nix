{ pkgs ? import <nixpkgs> {} }:

with pkgs;

let
  mkDerivation = import ./kalman.nix pkgs;

  attrs = {
    devInputs = [ emacs ripgrep less ps git gnuplot tree ];
    shellHook = ''
      alias ll='ls -l'
      source $setup
    '';
  };

in
  mkDerivation attrs



