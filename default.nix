{ pkgs ? import <nixpkgs> {} }:

with pkgs;

let
  mkDerivation = import ./kalman.nix pkgs;

in
  mkDerivation {}


  


  