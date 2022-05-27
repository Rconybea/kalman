# source this to enter nix environment (after running nix-shell) for kalman work;
# see ./default.nix
#

unset PATH
unset PKG_CONFIG_PATH
unset INCLUDEPATH

PATH=${coreutils}/bin

# findInputs() adapted from
#  https://github.com/NixOS/nixpkgs/blob/6675f0a52c0962042a1000c7f20e887d0d26ae25/pkgs/stdenv/generic/setup.sh#L60-L73

# Recursively find all build inputs.
#
#   findInputs ${pkg}
#
# adds direct-and-indirect dependencies of ${pkg} to shell variable ${pkgs}
#
function findInputs() {
    >&2 echo "findInputs: pkg=$1"

    local pkg=$1

    case ${pkgs} in
        *\ $pkg\ *)
            return 0
            ;;
    esac

    pkgs="${pkgs} $pkg "

    # stdenv runs setup hooks.
    # setup hooks are triggering errors here,  which we wilfully ignore
    #
    if test -f $pkg/nix-support/setup-hook; then
    	>&2 echo "findInputs: willfully skipping [${pkg}/nix-support/setup-hook]"
    #        source $pkg/nix-support/setup-hook
    fi

    if [[ -f ${pkg}/nix-support/propagated-build-inputs ]]; then
	>&2 echo "findInputs: consider [${pkg}/nix-support/propagated-build-inputs"
        for i in $(cat $pkg/nix-support/propagated-build-inputs); do
	    >&2 echo "findInputs: pickup propagated build input [${i}] on behalf of [${pkg}]"
            findInputs $i
        done
    fi
}

pkgs=""
for i in ${buildInputs} ${baseInputs} ${devInputs}; do
    findInputs $i
done

# end copied

# establish PATH, INCLUDEPATH, PKG_CONFIG_PATH based on explicitly-listed project dependencies
for p in ${pkgs}; do
    if [[ -d ${p}/bin ]]; then
	PATH=${p}/bin${PATH:+:}${PATH}
    fi
    if [[ -d ${p}/lib/pkgconfig ]]; then
	PKG_CONFIG_PATH="${p}/lib/pkgconfig${PKG_CONFIG_PATH:+:}${PKG_CONFIG_PATH}"
    elif [[ -d ${p}/include ]]; then
	INCLUDEPATH="${p}/include${INCLUDEPATH:+:}${INCLUDEPATH}"
    fi
done

export PATH
export PKG_CONFIG_PATH
export INCLUDEPATH

function display_phase() {
    self=display_phase

#    >&2 echo "${self}: home=${home}"
    >&2 echo "${self}: src=${srcdir}"
    >&2 echo "${self}: PATH=${PATH}"
    >&2 echo "${self}: PKG_CONFIG_PATH=${PKG_CONFIG_PATH}"
    >&2 echo "${self}: INCLUDEPATH=${INCLUDEPATH}"
    >&2 echo "${self}: baseInputs=${baseInputs}"
} # ..display_phase

function patchenv_phase() {
    if [[ ${system} -eq "aarch64-darwin" ]]; then
	# workaround -- looks like darwin package assumes codesign_allocate in path.
	# create a temporary link to /usr/bin/codesign_allocate
	# There's a PR with this hash 63f2bf3859321531c24b9558729d47b560e90c3e
	# that fixes darwin-packages.nix to use absolute path to codesign_allocate.

	# create a temporary bin directory for codesign_allocate
	if [[ ! -d bin || ! -f bin/codesign_allocate ]]; then
	    mkdir -pv bin
	    (cd bin && ln -sf /usr/bin/codesign_allocate)

	    PATH=${PATH}:$(pwd)/bin
	fi
    fi
} # ..patchenv_phase

function do_all_phases() {
    display_phase
    patchenv_phase
} # ..do_all_phases

function help() {
    echo "build sequence"
    echo "$ do_all_phases"
    echo "or:"
    echo "$ display_phase"
    echo "$ patchenv_phase"
} # ..help

# end setup.sh
