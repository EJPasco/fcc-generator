# This module tries to find the PYTHIA 8 installation
#
# This module defines
# PYTHIA8_DIR - PYTHIA 8 installation directory
# PYTHIA8_INCLUDE_DIR - where to locate PYTHIA 8 headers
# PYTHIA8_INCLUDE_DIRS - PYTHIA 8 include directories
# PYTHIA8_LIBRARY - where to find pythia8 library
# PYTHIA8_LIBRARIES - the libraries needed to use PYTHIA 8
# PYTHIA8_FOUND

# setting the folders to search in
set(_pythia8_dirs "${PYTHIA8_DIR}" "$ENV{PYTHIA8_DIR}" "/usr" "/usr/local" "/opt/pythia8")

# looking for PYTHIA 8 headers
find_path(PYTHIA8_INCLUDE_DIR
          NAMES "Pythia8/Pythia.h"
          PATHS ${_pythia8_dirs}
          PATH_SUFFIXES "include"
          DOC "PYTHIA headers directory")

set(PYTHIA8_INCLUDE_DIRS "${PYTHIA8_INCLUDE_DIR}")

# looking for pythia8 library
find_library(PYTHIA8_LIBRARY
             NAMES "pythia8" "Pythia8"
             PATHS ${_pythia8_dirs}
             PATH_SUFFIXES "lib"
             DOC "pythia8 library")

set(PYTHIA8_LIBRARIES "${PYTHIA8_LIBRARY}")

# geting the installation directory
get_filename_component(PYTHIA8_DIR
                       "${PYTHIA8_INCLUDE_DIR}"
                       DIRECTORY)

# finalizing
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PYTHIA8 DEFAULT_MSG PYTHIA8_DIR PYTHIA8_INCLUDE_DIR PYTHIA8_LIBRARY)
