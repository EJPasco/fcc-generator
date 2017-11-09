# fcc-generator

**DISCONTINUED**, but feel free to fork it.

Generator of events for _B<sup>0</sup><sub>d</sub> &rarr; K<sup>*0</sup> &tau;<sup>+</sup> &tau;<sup>-</sup>_ studies. It combines PYTHIA and EvtGen generation facilities with PODIO/fcc-edm storing system.
## Requirements
### Basic
+ C++11-compatible compiler (we recommend [G++](https://gcc.gnu.org/))
+ [CMake](https://cmake.org/) 3.0 or higher
+ [Boost](http://www.boost.org/) (optionally)
+ [Git](https://git-scm.com/) (optionally)

### Analysis related
+ [PODIO](https://github.com/HEP-FCC/podio)
+ [fcc-edm](https://github.com/cbernet/fcc-edm)
+ [PYTHIA](http://home.thep.lu.se/~torbjorn/Pythia.html)
+ [EvtGen](http://evtgen.warwick.ac.uk/)

## Installation
+ Make sure that environment variables for all packages are set properly
+ Clone this repo:
```bash
git clone https://github.com/semkiv/fcc-generator.git
```
or download a zipped copy of it using web interface and extract it
+ Navigate to newly created __fcc-generator__ directory:
```bash
cd fcc-generator
```
+ Check out __init.sh__ script and make changes to it if necessary
+ Set up the environment:
```bash
source init.sh
```
+ Create a dedicated build directory e.g. __build__:
```bash
mkdir build
```
+ Navigate to this directory:
```bash
cd build
```
+ Configure the build process:
```bash
cmake -DCMAKE_INSTALL_PREFIX=install ..
```
Here __install__ is the installation directory and __..__ is the path to source folder. If you don't want to use Boost libraries add `-DUSE_BOOST=OFF` to this command.
+ Build and install the project:
```bash
cmake --build . --target install -- -j 4
```
Here __4__ is the number of parallel threads used for building. Set your value or omit it to use as many threads as your system provides (use with care: may cause GUI freezes)
## Usage
If compiled with Boost libraries usage is:
```bash
generator options
```
where possible `options` are:
+ `--help` - show usage help and exit
+ `-n, --nevents=NUM` - generate NUM events. Required argument
+ `-k, --keyparticle=PDGID` - PDG ID of "key" particle (the one the redefined decay chain starts with). Optional argument, by default __511__ (_B<sup>0</sup><sub>d</sub>_)
+ `-P, --pythiacfg=CFGFILE` - PYTHIA config file. Optional argument, by default __pythia.cmnd__
+ `-E, --customdec=DECFILE` - EvtGen user decay file. Optional argument, by default __user.dec__
+ `--evtgendec=DECFILE` - EvtGen decay file. Optional argument, by default __$EVTGEN_ROOT_DIR/share/DECAY_2010.DEC__
+ `--evtgenpdl=PDLFILE` - EvtGen PDL file. Optional argument, by default __$EVTGEN_ROOT_DIR/share/evt.pdl__
+ `-o, --outfile=FILENAME` - Output file name. Optional argument, by default __output.root__
+ `-v, --verbosity` - Verbosity level. Possible values 0, 1, 2. Otional argument, by default 0

If compiled without Boost, usage is:
```bash
generator n
```
where `n` is the number of events to generate. All other options are hardcoded with the Boost-case default values.
