# Find the Tauola includes and library.
#
# This module defines
# TAUOLA_INCLUDE_DIR   where to locate Tauola.h file
# TAUOLA_LIBRARY       where to find the libTauolapp library
# TAUOLA_<lib>_LIBRARY Additional libraries
# TAUOLA_LIBRARIES     (not cached) the libraries to link against to use Tauola
# TAUOLA_FOUND         if false, you cannot build anything that requires Tauola
# TAUOLA_VERSION       version of Tauola if found

set(_tauoladirs ${TAUOLA_ROOT_DIR} $ENV{TAUOLA_ROOT_DIR} /usr)

find_path(TAUOLA_INCLUDE_DIR
          NAMES Tauola.h Tauola/Tauola.h
          HINTS ${_tauoladirs}
          PATH_SUFFIXES include
          DOC "Specify the directory containing Tauola.h.")

find_library(TAUOLA_FORTRAN_LIBRARY
             NAMES TauolaFortran
             HINTS ${_tauoladirs}
             PATH_SUFFIXES lib
             DOC "Specify the Tauola Fortran library here.")

find_library(TAUOLA_CXX_INTERFACE_LIBRARY
             NAMES TauolaCxxInterface
             HINTS ${_tauoladirs}
             PATH_SUFFIXES lib
             DOC "Specify the Tauola C++ Interface library here.")

foreach(_lib TAUOLA_FORTRAN_LIBRARY TAUOLA_CXX_INTERFACE_LIBRARY)
  if(${_lib})
    set(TAUOLA_LIBRARIES ${TAUOLA_LIBRARIES} ${${_lib}})
  endif()
endforeach()
set(TAUOLA_INCLUDE_DIRS ${TAUOLA_INCLUDE_DIR} ${TAUOLA_INCLUDE_DIR}/Tauola)

# handle the QUIETLY and REQUIRED arguments and set PYTHIA8_FOUND to TRUE if
# all listed variables are TRUE

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Tauola DEFAULT_MSG TAUOLA_INCLUDE_DIR TAUOLA_FORTRAN_LIBRARY TAUOLA_CXX_INTERFACE_LIBRARY)
mark_as_advanced(TAUOLA_INCLUDE_DIR TAUOLA_FORTRAN_LIBRARY TAUOLA_CXX_INTERFACE_LIBRARY)
