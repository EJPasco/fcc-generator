# This module tries to find the EvtGen installation
#
# This module defines
# EVTGEN_DIR - EvtGen installation directory
# EVTGEN_INCLUDE_DIR - where to locate EvtGen headers
# EVTGEN_INCLUDE_DIRS - EvtGen include directories
# EVTGEN_LIBRARY - where to find EvtGen library
# EVTGEN_EXTERNAL_LIBRARY - where to find EvtGenExternal library
# EVTGEN_LIBRARIES - the libraries needed to use EvtGen
# EVTGEN_FOUND

# setting the folders to search in
set(_evtgen_dirs "${EVTGEN_ROOT_DIR}" "$ENV{EVTGEN_ROOT_DIR}" "/usr" "/usr/local")

# looking for EvtGen headers
find_path(EVTGEN_INCLUDE_DIR
          NAMES "EvtGen/EvtGen.hh"
          PATHS ${_evtgen_dirs}
          PATH_SUFFIXES "include"
          DOC "EvtGen headers directory"
          )

set(EVTGEN_INCLUDE_DIRS "${EVTGEN_INCLUDE_DIR}")

# looking for EvtGen library
find_library(EVTGEN_LIBRARY
             NAMES "EvtGen"
             PATHS ${_evtgen_dirs}
             PATH_SUFFIXES "lib"
             DOC "EvtGen library"
             )

# looking for EvtGenExternal library
find_library(EVTGEN_EXTERNAL_LIBRARY
             NAMES "EvtGenExternal"
             PATHS ${_evtgen_dirs}
             PATH_SUFFIXES "lib"
             DOC "EvtGenExternal library"
             )

set(EVTGEN_LIBRARIES "${EVTGEN_LIBRARY}" "${EVTGEN_EXTERNAL_LIBRARY}")

# geting the installation directory
get_filename_component(EVTGEN_DIR
                       "${EVTGEN_INCLUDE_DIR}"
                       DIRECTORY
                       )

# finalazing
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(EvtGen DEFAULT_MSG EVTGEN_DIR EVTGEN_INCLUDE_DIR EVTGEN_LIBRARY EVTGEN_EXTERNAL_LIBRARY)
