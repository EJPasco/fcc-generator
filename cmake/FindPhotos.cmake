# This module tries to find the Photos installation
#
# This module defines
# PHOTOS_DIR - Photos installation directory
# PHOTOS_INCLUDE_DIR - where to locate Photos headers
# PHOTOS_INCLUDE_DIRS - Photos include directories
# PHOTOSPP_LIBRARY - where to find Photospp library
# PHOTOS_LIBRARIES - the libraries needed to use Photos
# PHOTOS_FOUND

# setting the folders to search in
set(_photos_dirs "${PHOTOS_ROOT_DIR}" "$ENV{PHOTOS_ROOT_DIR}" "/usr" "/usr/local")

# looking for Photos headers
find_path(PHOTOS_INCLUDE_DIR
          NAMES "Photos/Photos.h"
          PATHS ${_photos_dirs}
          PATH_SUFFIXES "include"
          DOC "Photos headers directory")

set(PHOTOS_INCLUDE_DIRS "${PHOTOS_INCLUDE_DIR}")

# looking for Photospp library
find_library(PHOTOSPP_LIBRARY
             NAMES "Photospp"
             PATHS ${_photos_dirs}
             PATH_SUFFIXES "lib"
             DOC "Phototspp library")

# looking for PhotosppHepMC library (optional)
find_library(PHOTOSPPHEPMC_LIBRARY
             NAMES "PhotosppHepMC"
             PATHS ${_photos_dirs}
             PATH_SUFFIXES "lib"
             DOC "PhotosppHepMC library")

set(PHOTOS_LIBRARIES "${PHOTOSPP_LIBRARY}" "${PHOTOSPPHEPMC_LIBRARY}")

# geting the installation directory
get_filename_component(PHOTOS_DIR
                       "${PHOTOS_INCLUDE_DIR}"
                       DIRECTORY)

# finalizing
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Photos DEFAULT_MSG PHOTOS_DIR PHOTOS_INCLUDE_DIR PHOTOSPP_LIBRARY)
