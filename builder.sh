# nix build script for the kalman project
#
# NOTE: to debug this builder:
#
#   $ cd path/to/kalman
#   $ nix-shell
#   $ source setup.sh
#
# nix supplies shell variables:
#   ${out}     output location in nix store
# along with a shell variable for each member of this directory's nix derivation;
# see derivation { ... } in ./default.nix:
#   ${name}        name of this project (kalman)
#   ${builder}     builder executable
#   ${args}        build script
#   ${setup}       setup script
#   ${baseInputs}  required nix packages (clang, make, ...)
#   ${buildInputs} more required nix packages
#   ${gccc}        clang or gcc
#   ${binutils}    bintools_bin or binutils
#   ${srcdir}      location of source tree (will be ./src,  but presented somewhere else)
#   ${system}      running system name

set -e
#set -x

source ${setup}  # see ./setup.sh

mkdir -pv ${out}
env > ${out}/build.env

do_all_phases

# end builder.sh

