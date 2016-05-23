# This module tries to find the Tauola installation
#
# This module defines
# TAUOLA_DIR - Tauola installation directory
# TAUOLA_INCLUDE_DIR - where to locate Tauola headers
# TAUOLA_INCLUDE_DIRS - Tauola include directories
# TAUOLACXXINTERFACE_LIBRARY - where to find TauolaCxxInterface library
# TAUOLA_LIBRARIES - the libraries needed to use Tauola
# TAUOLA_FOUND

# setting the folders to search in
set(_tauola_dirs "${TAUOLA_ROOT_DIR}" "$ENV{TAUOLA_ROOT_DIR}" "/usr" "/usr/local")

# looking for Tauola headers
find_path(TAUOLA_INCLUDE_DIR
          NAMES "Tauola/Tauola.h"
          PATHS ${_tauola_dirs}
          PATH_SUFFIXES "include"
          DOC "Tauola headers directory")

set(TAUOLA_INCLUDE_DIRS "${TAUOLA_INCLUDE_DIR}")

# looking for TauolaCxxInterface library
find_library(TAUOLACXXINTERFACE_LIBRARY
             NAMES "TauolaCxxInterface"
             PATHS ${_tauola_dirs}
             PATH_SUFFIXES "lib"
             DOC "TauolaCxxInterface library")

set(TAUOLA_LIBRARIES "${TAUOLACXXINTERFACE_LIBRARY}")

# geting the installation directory
get_filename_component(TAUOLA_DIR
                       "${TAUOLA_INCLUDE_DIR}"
                       DIRECTORY)

# finalazing
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Tauola DEFAULT_MSG TAUOLA_DIR TAUOLA_INCLUDE_DIR TAUOLACXXINTERFACE_LIBRARY)
