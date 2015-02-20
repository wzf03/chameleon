###
#
# @copyright (c) 2009-2014 The University of Tennessee and The University
#                          of Tennessee Research Foundation.
#                          All rights reserved.
# @copyright (c) 2012-2014 Inria. All rights reserved.
# @copyright (c) 2012-2014 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria, Univ. Bordeaux. All rights reserved.
#
###
#
# - Find FFTW Version 3 include dirs and libraries
# Use this module by invoking find_package with the form:
#  find_package(FFTW
#               [REQUIRED] # Fail with error if fftw is not found
#               [COMPONENTS MKL]
#
#  COMPONENTS can be some of the following:
#   - MKL:     to detect the FFTW from Intel MKL
#   - THREADS: to detect the Threads version of FFTW
#   - OMP:     to detect the OpenMP version of FFTW
#   - SIMPLE:  to detect the FFTW simple precision fftw3f
#   - DOUBLE:  to detect the FFTW double precision fftw3 (default)
#   - LONG:    to detect the FFTW long double precision fftw3l
#   - QUAD:    to detect the FFTW quadruple precision fftw3q
#
# This module finds headers and fftw library.
# Results are reported in variables:
#  FFTW_FOUND            - True if headers and requested libraries were found
#  FFTW_INCLUDE_DIRS     - fftw include directories
#  FFTW_LIBRARY_DIRS     - Link directories for fftw libraries
#  FFTW_LIBRARIES        - fftw component libraries to be linked
#  FFTW_INCLUDE_DIRS_DEP - fftw + dependencies include directories
#  FFTW_LIBRARY_DIRS_DEP - fftw + dependencies link directories
#  FFTW_LIBRARIES_DEP    - fftw libraries + dependencies
#
# The user can give specific paths where to find the libraries adding cmake
# options at configure (ex: cmake path/to/project -DFFTW_DIR=path/to/fftw):
#  FFTW_DIR             - Where to find the base directory of fftw
#  FFTW_INCDIR          - Where to find the header files
#  FFTW_LIBDIR          - Where to find the library files

#=============================================================================
# Copyright 2012-2013 Inria
# Copyright 2012-2013 Emmanuel Agullo
# Copyright 2012-2013 Mathieu Faverge
# Copyright 2012      Cedric Castagnede
# Copyright 2013      Florent Pruvost
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file MORSE-Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of Morse, substitute the full
#  License text for the above reference.)


if (NOT FFTW_FOUND)
    set(FFTW_DIR "" CACHE PATH "Root directory of FFTW library")
    if (NOT FFTW_FIND_QUIETLY)
        message(STATUS "A cache variable, namely FFTW_DIR, has been set to specify the install directory of FFTW")
    endif()
endif()

# Set the version to find
set(FFTW_LOOK_FOR_MKL OFF)
set(FFTW_LOOK_FOR_THREADS OFF)
set(FFTW_LOOK_FOR_OMP OFF)
set(FFTW_LOOK_FOR_FFTW_SIMPLE OFF)
set(FFTW_LOOK_FOR_FFTW_DOUBLE ON)
set(FFTW_LOOK_FOR_FFTW_LONG OFF)
set(FFTW_LOOK_FOR_FFTW_QUAD OFF)

if( FFTW_FIND_COMPONENTS )
    foreach( component ${FFTW_FIND_COMPONENTS} )
        if (${component} STREQUAL "THREADS")
            # means we look for the Threads version of FFTW
            set(FFTW_LOOK_FOR_THREADS ON)
        endif()
        if (${component} STREQUAL "OMP")
            # means we look for the OpenMP version of FFTW
            set(FFTW_LOOK_FOR_OMP ON)
        endif()
        if (${component} STREQUAL "SIMPLE")
            # means we look for FFTW simple precision (fftw3f)
            set(FFTW_LOOK_FOR_FFTW_SIMPLE ON)
            set(FFTW_LOOK_FOR_FFTW_DOUBLE OFF)
            set(FFTW_LOOK_FOR_FFTW_LONG OFF)
            set(FFTW_LOOK_FOR_FFTW_QUAD OFF)
        endif()
        if (${component} STREQUAL "DOUBLE")
            # means we look for FFTW double precision (fftw3)
            set(FFTW_LOOK_FOR_FFTW_SIMPLE OFF)
            set(FFTW_LOOK_FOR_FFTW_DOUBLE ON)
            set(FFTW_LOOK_FOR_FFTW_LONG OFF)
            set(FFTW_LOOK_FOR_FFTW_QUAD OFF)
        endif()
        if (${component} STREQUAL "LONG")
            # means we look for FFTW long double precision (fftw3l)
            set(FFTW_LOOK_FOR_FFTW_SIMPLE OFF)
            set(FFTW_LOOK_FOR_FFTW_DOUBLE OFF)
            set(FFTW_LOOK_FOR_FFTW_LONG ON)
            set(FFTW_LOOK_FOR_FFTW_QUAD OFF)
        endif()
        if (${component} STREQUAL "QUAD")
            # means we look for FFTW quad precision (fftw3q)
            set(FFTW_LOOK_FOR_FFTW_SIMPLE OFF)
            set(FFTW_LOOK_FOR_FFTW_DOUBLE OFF)
            set(FFTW_LOOK_FOR_FFTW_LONG OFF)
            set(FFTW_LOOK_FOR_FFTW_QUAD ON)
        endif()
        if (${component} STREQUAL "MKL")
            # means we look for the Intel MKL version of FFTW
            set(FFTW_LOOK_FOR_MKL ON)
            if (FFTW_LOOK_FOR_FFTW_LONG)
                message(WARNING "Looking for FFTW -- long precision functions do not exist in MKL FFTW")
                set(FFTW_LOOK_FOR_FFTW_LONG OFF)
            endif()
            if (FFTW_LOOK_FOR_FFTW_QUAD)
                message(WARNING "Looking for FFTW -- quadruple functions do not exist in MKL FFTW")
                set(FFTW_LOOK_FOR_FFTW_QUAD OFF)
            endif()
        endif()
    endforeach()
endif()

if (FFTW_LOOK_FOR_THREADS)
    if (FFTW_FIND_REQUIRED)
        find_package(Threads REQUIRED)
    else()
        find_package(Threads)
    endif()
endif()

if (FFTW_LOOK_FOR_OMP)
    if (FFTW_FIND_REQUIRED)
        find_package(OpenMP REQUIRED)
    else()
        find_package(OpenMP)
    endif()
endif()

if( NOT FFTW_FOUND )

    # Looking for include
    # -------------------

    # Add system include paths to search include
    # ------------------------------------------
    unset(_inc_env)
    if(WIN32)
        string(REPLACE ":" ";" _inc_env "$ENV{INCLUDE}")
    else()
        string(REPLACE ":" ";" _path_env "$ENV{INCLUDE}")
        list(APPEND _inc_env "${_path_env}")
        string(REPLACE ":" ";" _path_env "$ENV{C_INCLUDE_PATH}")
        list(APPEND _inc_env "${_path_env}")
        string(REPLACE ":" ";" _path_env "$ENV{CPATH}")
        list(APPEND _inc_env "${_path_env}")
        string(REPLACE ":" ";" _path_env "$ENV{INCLUDE_PATH}")
        list(APPEND _inc_env "${_path_env}")
    endif()
    list(APPEND _inc_env "${CMAKE_PLATFORM_IMPLICIT_INCLUDE_DIRECTORIES}")
    list(APPEND _inc_env "${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES}")
    list(REMOVE_DUPLICATES _inc_env)

    # set paths where to look for
    set(PATH_TO_LOOK_FOR "${_inc_env}")

    # Try to find the fftw header in the given paths
    # -------------------------------------------------
    # call cmake macro to find the header path
    if(FFTW_INCDIR)
        set(FFTW_fftw3.h_DIRS "FFTW_fftw3.h_DIRS-NOTFOUND")
        find_path(FFTW_fftw3.h_DIRS
          NAMES fftw3.h
          HINTS ${FFTW_INCDIR})
    else()
        if(FFTW_DIR)
            set(FFTW_fftw3.h_DIRS "FFTW_fftw3.h_DIRS-NOTFOUND")
            find_path(FFTW_fftw3.h_DIRS
              NAMES fftw3.h
              HINTS ${FFTW_DIR}
              PATH_SUFFIXES include)
        else()
            set(FFTW_fftw3.h_DIRS "FFTW_fftw3.h_DIRS-NOTFOUND")
            find_path(FFTW_fftw3.h_DIRS
                      NAMES fftw3.h
                      HINTS ${PATH_TO_LOOK_FOR})
        endif()
    endif()
    mark_as_advanced(FFTW_fftw3.h_DIRS)

    # Add path to cmake variable
    # ------------------------------------
    if (FFTW_fftw3.h_DIRS)
        set(FFTW_INCLUDE_DIRS "${FFTW_fftw3.h_DIRS}")
    else ()
        set(FFTW_INCLUDE_DIRS "FFTW_INCLUDE_DIRS-NOTFOUND")
        if(NOT FFTW_FIND_QUIETLY)
            message(STATUS "Looking for FFTW -- fftw3.h not found")
        endif()
    endif ()


    # Looking for lib
    # ---------------

    # Add system library paths to search lib
    # --------------------------------------
    unset(_lib_env)
    if(WIN32)
        string(REPLACE ":" ";" _lib_env "$ENV{LIB}")
    else()
        if(APPLE)
            string(REPLACE ":" ";" _lib_env "$ENV{DYLD_LIBRARY_PATH}")
        else()
            string(REPLACE ":" ";" _lib_env "$ENV{LD_LIBRARY_PATH}")
        endif()
        list(APPEND _lib_env "${CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES}")
        list(APPEND _lib_env "${CMAKE_C_IMPLICIT_LINK_DIRECTORIES}")
    endif()
    list(REMOVE_DUPLICATES _lib_env)

    # set paths where to look for
    set(PATH_TO_LOOK_FOR "${_lib_env}")

    if(FFTW_LOOK_FOR_FFTW_SIMPLE)
        set(FFTW_PREC "f")
        set(FFTW_PREC_TESTFUNC "s")
    elseif(FFTW_LOOK_FOR_FFTW_DOUBLE)
        set(FFTW_PREC "")
        set(FFTW_PREC_TESTFUNC "d")
    elseif(FFTW_LOOK_FOR_FFTW_LONG)
        set(FFTW_PREC "l")
        set(FFTW_PREC_TESTFUNC "l")
    elseif(FFTW_LOOK_FOR_FFTW_QUAD)
        set(FFTW_PREC "q")
        set(FFTW_PREC_TESTFUNC "q")
    endif()

    if (FFTW_LOOK_FOR_MKL)

        set(FFTW_libs_to_find "mkl_intel_lp64;mkl_sequential;mkl_core")

        # Try to find the MKL fftw lib in the given paths
        # -----------------------------------------------

        # call cmake macro to find the lib path
        if(FFTW_LIBDIR)
            foreach(fftw_lib ${FFTW_libs_to_find})
                set(FFTW_${fftw_lib}_LIBRARY "FFTW_${fftw_lib}_LIBRARY-NOTFOUND")
                find_library(FFTW_${fftw_lib}_LIBRARY
                    NAMES ${fftw_lib}
                    HINTS ${FFTW_LIBDIR})
            endforeach()
        else()
            if(FFTW_DIR)
                foreach(fftw_lib ${FFTW_libs_to_find})
                    set(FFTW_${fftw_lib}_LIBRARY "FFTW_${fftw_lib}_LIBRARY-NOTFOUND")
                    find_library(FFTW_${fftw_lib}_LIBRARY
                        NAMES ${fftw_lib}
                        HINTS ${FFTW_DIR}
                        PATH_SUFFIXES lib lib32 lib64)
                endforeach()
            else()
                foreach(fftw_lib ${FFTW_libs_to_find})
                    set(FFTW_${fftw_lib}_LIBRARY "FFTW_${fftw_lib}_LIBRARY-NOTFOUND")
                    find_library(FFTW_${fftw_lib}_LIBRARY
                             NAMES ${fftw_lib}
                             HINTS ${PATH_TO_LOOK_FOR})
                endforeach()
            endif()
        endif()

    else(FFTW_LOOK_FOR_MKL)

        if (FFTW_LOOK_FOR_THREADS)
            set(FFTW_libs_to_find "fftw3${FFTW_PREC}_threads;fftw3${FFTW_PREC}")
        elseif (FFTW_LOOK_FOR_OMP)
            set(FFTW_libs_to_find "fftw3${FFTW_PREC}_omp;fftw3${FFTW_PREC}")
        else()
            set(FFTW_libs_to_find "fftw3${FFTW_PREC}")
        endif()

        # Try to find the fftw lib in the given paths
        # ----------------------------------------------

        # call cmake macro to find the lib path
        if(FFTW_LIBDIR)
            foreach(fftw_lib ${FFTW_libs_to_find})
                set(FFTW_${fftw_lib}_LIBRARY "FFTW_${fftw_lib}_LIBRARY-NOTFOUND")
                find_library(FFTW_${fftw_lib}_LIBRARY
                    NAMES ${fftw_lib}
                    HINTS ${FFTW_LIBDIR})
            endforeach()
        else()
            if(FFTW_DIR)
                foreach(fftw_lib ${FFTW_libs_to_find})
                    set(FFTW_${fftw_lib}_LIBRARY "FFTW_${fftw_lib}_LIBRARY-NOTFOUND")
                    find_library(FFTW_${fftw_lib}_LIBRARY
                        NAMES ${fftw_lib}
                        HINTS ${FFTW_DIR}
                        PATH_SUFFIXES lib lib32 lib64)
                endforeach()
            else()
                foreach(fftw_lib ${FFTW_libs_to_find})
                    set(FFTW_${fftw_lib}_LIBRARY "FFTW_${fftw_lib}_LIBRARY-NOTFOUND")
                    find_library(FFTW_${fftw_lib}_LIBRARY
                             NAMES ${fftw_lib}
                             HINTS ${PATH_TO_LOOK_FOR})
                endforeach()
            endif()
        endif()

    endif(FFTW_LOOK_FOR_MKL)

    # If found, add path to cmake variable
    # ------------------------------------
    set(FFTW_LIBRARIES "")
    set(FFTW_LIBRARY_DIRS "")
    foreach(fftw_lib ${FFTW_libs_to_find})

        if (FFTW_${fftw_lib}_LIBRARY)
            get_filename_component(${fftw_lib}_lib_path "${FFTW_${fftw_lib}_LIBRARY}" PATH)
            # set cmake variables
            list(APPEND FFTW_LIBRARIES "${FFTW_${fftw_lib}_LIBRARY}")
            list(APPEND FFTW_LIBRARY_DIRS "${${fftw_lib}_lib_path}")
        else ()
            list(APPEND FFTW_LIBRARIES "${FFTW_${fftw_lib}_LIBRARY}")
            if (NOT FFTW_FIND_QUIETLY)
                message(STATUS "Looking for FFTW -- lib ${fftw_lib} not found")
            endif()
        endif ()
        mark_as_advanced(FFTW_${fftw_lib}_LIBRARY)

    endforeach()

    list(REMOVE_DUPLICATES FFTW_INCLUDE_DIRS)
    list(REMOVE_DUPLICATES FFTW_LIBRARY_DIRS)

    # check a function to validate the find
    if(FFTW_LIBRARIES)

        set(REQUIRED_INCDIRS)
        set(REQUIRED_LIBDIRS)
        set(REQUIRED_LIBS)

        # FFTW
        if (FFTW_INCLUDE_DIRS)
            set(REQUIRED_INCDIRS "${FFTW_INCLUDE_DIRS}")
        endif()
        if (FFTW_LIBRARY_DIRS)
            set(REQUIRED_LIBDIRS "${FFTW_LIBRARY_DIRS}")
        endif()
        set(REQUIRED_LIBS "${FFTW_LIBRARIES}")
        # THREADS
        if (FFTW_LOOK_FOR_THREADS)
            list(APPEND REQUIRED_LIBS "${CMAKE_THREAD_LIBS_INIT}")
        endif()
        # OMP
        if(FFTW_LOOK_FOR_OMP)
            if (CMAKE_C_COMPILER MATCHES ".+gcc.*")
                set(CMAKE_REQUIRED_FLAGS "-fopenmp")
            endif()
        endif()
        # MKL
        if(FFTW_LOOK_FOR_MKL)
            list(APPEND REQUIRED_LIBS "-Wl,--no-as-needed;${CMAKE_REQUIRED_LIBRARIES};${CMAKE_THREAD_LIBS_INIT};-lm")
        endif()

        # set required libraries for link
        set(CMAKE_REQUIRED_INCLUDES "${REQUIRED_INCDIRS}")
        set(CMAKE_REQUIRED_LIBRARIES)
        foreach(lib_dir ${REQUIRED_LIBDIRS})
            list(APPEND CMAKE_REQUIRED_LIBRARIES "-L${lib_dir}")
        endforeach()
        list(APPEND CMAKE_REQUIRED_LIBRARIES "${REQUIRED_LIBS}")

        # test link
        unset(FFTW_WORKS CACHE)
        include(CheckFunctionExists)
        check_function_exists(${FFTW_PREC_TESTFUNC}fftw_execute_ FFTW_WORKS)
        mark_as_advanced(FFTW_WORKS)

        if(FFTW_WORKS)
            # save link with dependencies
            set(FFTW_LIBRARIES_DEP "${REQUIRED_LIBS}")
            set(FFTW_LIBRARY_DIRS_DEP "${REQUIRED_LIBDIRS}")
            set(FFTW_INCLUDE_DIRS_DEP "${REQUIRED_INCDIRS}")
        else()
            if(NOT FFTW_FIND_QUIETLY)
                message(STATUS "Looking for FFTW : test of ${FFTW_PREC_TESTFUNC}fftw_execute_ with fftw library fails")
                message(STATUS "CMAKE_REQUIRED_LIBRARIES: ${CMAKE_REQUIRED_LIBRARIES}")
                message(STATUS "CMAKE_REQUIRED_INCLUDES: ${CMAKE_REQUIRED_INCLUDES}")
                message(STATUS "Check in CMakeFiles/CMakeError.log to figure out why it fails")
            endif()
        else()
            set(FFTW_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
        endif()
        set(CMAKE_REQUIRED_INCLUDES)
        set(CMAKE_REQUIRED_FLAGS)
        set(CMAKE_REQUIRED_LIBRARIES)
    endif(FFTW_LIBRARIES)

endif( NOT FFTW_FOUND )


# check that FFTW has been found
# -------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FFTW DEFAULT_MSG
                                  FFTW_LIBRARIES
                                  FFTW_INCLUDE_DIRS
                                  FFTW_WORKS)
