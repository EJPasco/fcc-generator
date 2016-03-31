# Find the Photos includes and library.
#
# This module defines
# PHOTOS_INCLUDE_DIR   where to locate Photos.h file
# PHOTOS_LIBRARY       where to find the libPhotospp library
# PHOTOS_<lib>_LIBRARY Additional libraries
# PHOTOS_LIBRARIES     (not cached) the libraries to link against to use Photos
# PHOTOS_FOUND         if false, you cannot build anything that requires Photos
# PHOTOS_VERSION       version of Photos if found

set(_photosdirs ${PHOTOS_ROOT_DIR} $ENV{PHOTOS_ROOT_DIR} /usr)

find_path(PHOTOS_INCLUDE_DIR
          NAMES Photos.h Photos/Photos.h
          HINTS ${_photosdirs}
          PATH_SUFFIXES include
          DOC "Specify the directory containing Photos.h.")

find_library(PHOTOS_LIBRARY
             NAMES Photospp
             HINTS ${_photosdirs}
             PATH_SUFFIXES lib
             DOC "Specify the Photos library here.")
find_library(PHOTOS_HEPMC_LIBRARY
             NAMES PhotosppHepMC
             HINTS ${_photosdirs}
             PATH_SUFFIXES lib
             DOC "Specify the Photos library here.")

foreach(_lib PHOTOS_LIBRARY PHOTOS_HEPMC_LIBRARY)
  if(${_lib})
    set(PHOTOS_LIBRARIES ${PHOTOS_LIBRARIES} ${${_lib}})
  endif()
endforeach()
set(PHOTOS_INCLUDE_DIRS ${PHOTOS_INCLUDE_DIR} ${PHOTOS_INCLUDE_DIR}/Photos)

# handle the QUIETLY and REQUIRED arguments and set PYTHIA8_FOUND to TRUE if
# all listed variables are TRUE

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Photos DEFAULT_MSG PHOTOS_INCLUDE_DIR PHOTOS_LIBRARY PHOTOS_HEPMC_LIBRARY)
mark_as_advanced(PHOTOS_INCLUDE_DIR PHOTOS_LIBRARY PHOTOS_HEPMC_LIBRARY)
