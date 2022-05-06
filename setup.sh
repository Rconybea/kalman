# source this to enter nix environment (after running nix-shell) for kalman work;
# see ./default.nix
#

unset PATH
unset PKG_CONFIG_PATH

# establish PATH, PKG_CONFIG_PATH based on explicitly-listed project dependencies
for p in ${buildInputs} ${baseInputs} ${devInputs}; do
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
