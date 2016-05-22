# This module tries to find the ROOT installation
#
# This module defines
# ROOT_DIR - ROOT installation directory
# ROOT_INCLUDE_DIR - where to locate ROOT headers
# ROOT_INCLUDE_DIRS - ROOT include directories
# ROOT_LIBRARIES - the libraries needed to use ROOT
# ROOT_FOUND

# looking for root-config script
find_program(ROOT_CONFIG_EXECUTABLE
             NAMES "root-config"
             PATHS "$ENV{ROOTSYS}/bin"
             )

if(ROOT_CONFIG_EXECUTABLE)
    # determining ROOT installation location
    execute_process(COMMAND "${ROOT_CONFIG_EXECUTABLE}" --prefix
                    OUTPUT_VARIABLE ROOT_DIR
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    )

    # determining ROOT headers directory
    execute_process(COMMAND "${ROOT_CONFIG_EXECUTABLE}" --incdir
                    OUTPUT_VARIABLE ROOT_INCLUDE_DIR
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    )

    set(ROOT_INCLUDE_DIRS "${ROOT_INCLUDE_DIR}")

    # determining ROOT libraries
    execute_process(COMMAND "${ROOT_CONFIG_EXECUTABLE}" --libs
                    OUTPUT_VARIABLE ROOT_LIBRARIES
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ROOT DEFAULT_MSG ROOT_DIR ROOT_INCLUDE_DIR ROOT_LIBRARIES)
