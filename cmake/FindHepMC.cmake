# This module tries to find the HepMC installation
#
# This module defines
# HEPMC_DIR - HepMC installation directory
# HEPMC_INCLUDE_DIR - where to locate HepMC headers
# HEPMC_INCLUDE_DIRS - HepMC include directories
# HEPMC_LIBRARY - where to find HepMC library
# HEPMC_LIBRARIES - the libraries needed to use HepMC
# HEPMC_FOUND

# setting the folders to search in
set(_hepmc_dirs "${HEPMC_PREFIX}" "$ENV{HEPMC_PREFIX}" "/usr" "/usr/local")

# looking for HepMC headers
find_path(HEPMC_INCLUDE_DIR
          NAMES "HepMC/GenEvent.h"
          PATHS ${_hepmc_dirs}
          PATH_SUFFIXES "include"
          DOC "HepMC headers directory"
          )

set(HEPMC_INCLUDE_DIRS "${HEPMC_INCLUDE_DIR}")

# looking for HepMC library
find_library(HEPMC_LIBRARY
             NAMES "HepMC"
             PATHS ${_hepmc_dirs}
             PATH_SUFFIXES "lib"
             DOC "HepMC library"
             )

set(HEPMC_LIBRARIES "${HEPMC_LIBRARY}")

# getting the installation directory
get_filename_component(HEPMC_DIR
                       "${HEPMC_INCLUDE_DIR}"
                       DIRECTORY
                       )

# finalazing
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HepMC DEFAULT_MSG HEPMC_DIR HEPMC_INCLUDE_DIR HEPMC_LIBRARY)
