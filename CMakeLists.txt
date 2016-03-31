cmake_minimum_required(VERSION 2.8)
project(generator)

set(Generator_VERSION_MAJOR 0)
set(Generator_VERSION_MINOR 1)

# should we use Boost to parse program options
option(USE_BOOST "Use Boost provided program options parser implementation" ON)

# configure a header file to pass some of the CMake settings to the source code
configure_file("${PROJECT_SOURCE_DIR}/GeneratorConfig.h.in" "${PROJECT_BINARY_DIR}/include/GeneratorConfig.h")

# should we use Boost to parse program options
# option(USE_BOOST "Use Boost provided program options parser implementation" ON)
# set(USE_BOOST ON BOOL "Use Boost provided program options parser implementation")

# check if environment variables are set
set(PYTHIA8DIR $ENV{PYTHIA8_DIR})
if(NOT PYTHIA8DIR)
    message(FATAL_ERROR "You need to set PYTHIA8_DIR to the install prefix of pythia.")
endif()

set(PYTHIA8DATA $ENV{PYTHIA8DATA})
if(NOT PYTHIA8DATA)
    message(FATAL_ERROR "You need to set PYTHIA8DATA to the path of the xmldoc directory of your pythia installation")
endif()

set(HEPMC_PREFIX $ENV{HEPMC_PREFIX})
if(NOT HEPMC_PREFIX)
    message(FATAL_ERROR "You need to set HEPMC_PREFIX to the install prefix of HepMC.")
endif()


# Set up C++11
set(CPP11FLAGS "-std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CPP11FLAGS} -O3 -fPIC -Wall -Wextra -Wpedantic -Wconversion")

# Make sure we find the Find*.cmake functions distributed with this package
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_PREFIX_PATH $ENV{FCCEDM})

# Make sure we find the Find*.cmake functions distributed with this package
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(podio REQUIRED HINTS $ENV{PODIO})
message(STATUS "Found podio: ${podio_DIR}")
# Make sure the library is found.
# Not the case if LD_LIBRARY_PATH is wrong
find_library(PODIOLIB podio PATHS $ENV{PODIO}/lib)
if (NOT PODIOLIB)
    message(FATAL_ERROR "libpodio.so(dylib) cannot be found dynamically. Make sure you have sourced PODIO init*.sh file to set up your environment to use PODIO")
endif()
link_directories(${podio_LIBRARY_DIR})

find_package(fccedm REQUIRED HINTS $ENV{FCCEDM})
message(STATUS "Found fcc-edm: ${fccedm_DIR}")

find_package(ROOT REQUIRED)

find_package(PYTHIA8 REQUIRED)

find_package(HepMC REQUIRED)

find_package(EvtGen REQUIRED)

find_package(Photos REQUIRED)

find_package(Tauola REQUIRED)

# set up include-directories
include_directories(
    "${PROJECT_BINARY_DIR}/include"
    "${ROOT_INCLUDE_DIR}"
    "${podio_INCLUDE_DIRS}"
    "${FCCEDM_INCLUDE_DIRS}"
    "${PYTHIA8_INCLUDE_DIRS}"
    "${HEPMC_INCLUDE}"
    "${EVTGEN_INCLUDE_DIRS}"
    "${PHOTOS_INCLUDE_DIRS}"
    "${TAUOLA_INCLUDE_DIRS}"
  )

# Add sub-directories
add_subdirectory(src)
