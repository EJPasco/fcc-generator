SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  THIS_DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$THIS_DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
THIS_DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

LCG_RELEASE=/afs/cern.ch/sw/lcg/releases/LCG_80

export EVTGEN_ROOT_DIR=$LCG_RELEASE/MCGenerators/evtgen/1.5.0/x86_64-slc6-gcc49-opt
export PHOTOS_ROOT_DIR=$LCG_RELEASE/MCGenerators/photos++/3.61/x86_64-slc6-gcc49-opt
export TAUOLA_ROOT_DIR=$LCG_RELEASE/MCGenerators/tauola++/1.1.5/x86_64-slc6-gcc49-opt
export PATH=/afs/cern.ch/sw/lcg/contrib/CMake/3.3.2/Linux-x86_64/bin:${PATH}
source /afs/cern.ch/sw/lcg/contrib/gcc/4.9.3/x86_64-slc6/setup.sh
source /afs/cern.ch/exp/fcc/sw/0.6/LCG_80/ROOT/6.04.06/x86_64-slc6-gcc49-opt/bin/thisroot.sh

export PYTHIA8_DIR=/afs/cern.ch/sw/lcg/releases/LCG_80/MCGenerators/pythia8/212/x86_64-slc6-gcc49-opt/
export HEPMC_PREFIX=/afs/cern.ch/sw/lcg/releases/LCG_80/HepMC/2.06.09/x86_64-slc6-gcc49-opt/

export PYTHIA8DATA=${PYTHIA8_DIR}/share/Pythia8/xmldoc

export GENERATOR=$THIS_DIR/install
export LD_LIBRARY_PATH=$GENERATOR/lib:$HEPMC_PREFIX/lib:$FASTJET_ROOT_DIR/lib:$PYTHIA8_DIR/lib:$EVTGEN_ROOT_DIR/lib:$PHOTOS_ROOT_DIR/lib:$TAUOLA_ROOT_DIR/lib:$LD_LIBRARY_PATH
export PATH=$GENERATOR/bin:$PATH
