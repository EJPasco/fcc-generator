# Find the EvtGen includes and library.
#
# This module defines
# EVTGEN_INCLUDE_DIR   where to locate EvtGen.hh file
# EVTGEN_LIBRARY       where to find the libEvtGen library
# EVTGEN_LIBRARIES     (not cached) the libraries to link against to use EvtGen
# EVTGEN_FOUND         if false, you cannot build anything that requires EvtGen
# EVTGEN_VERSION       version of EvtGen if found

set(_evtgendirs ${EVTGEN_ROOT_DIR} $ENV{EVTGEN_ROOT_DIR} /usr)

find_path(EVTGEN_INCLUDE_DIR
          NAMES EvtGen.hh EvtGen/EvtGen.hh
          HINTS ${_evtgendirs}
          PATH_SUFFIXES include
          DOC "Specify the directory containing EvtGen.hh.")

find_library(EVTGEN_LIBRARY
             NAMES EvtGen
             HINTS ${_evtgendirs}
             PATH_SUFFIXES lib
             DOC "Specify the EvtGen library here.")

find_library(EVTGEN_EXTERNAL_LIBRARY
             NAMES EvtGenExternal
             HINTS ${_evtgendirs}
             PATH_SUFFIXES lib)

foreach(_lib EVTGEN_LIBRARY EVTGEN_EXTERNAL_LIBRARY)
  if(${_lib})
    set(EVTGEN_LIBRARIES ${EVTGEN_LIBRARIES} ${${_lib}})
  endif()
endforeach()
set(EVTGEN_INCLUDE_DIRS ${EVTGEN_INCLUDE_DIR} ${EVTGEN_INCLUDE_DIR}/EvtGen )

# handle the QUIETLY and REQUIRED arguments and set PYTHIA8_FOUND to TRUE if
# all listed variables are TRUE

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(EvtGen DEFAULT_MSG EVTGEN_INCLUDE_DIR EVTGEN_LIBRARY EVTGEN_EXTERNAL_LIBRARY)
mark_as_advanced(EVTGEN_INCLUDE_DIR EVTGEN_LIBRARY EVTGEN_EXTERNAL_LIBRARY)
