###
#
# @file starpu/CMakeLists.txt
#
# @copyright 2009-2015 The University of Tennessee and The University of
#                      Tennessee Research Foundation. All rights reserved.
# @copyright 2012-2023 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
#                      Univ. Bordeaux. All rights reserved.
#
###
#
#  @project CHAMELEON
#  CHAMELEON is a software package provided by:
#     Inria Bordeaux - Sud-Ouest,
#     Univ. of Tennessee,
#     King Abdullah Univesity of Science and Technology
#     Univ. of California Berkeley,
#     Univ. of Colorado Denver.
#
# @version 1.3.0
#  @author Cedric Castagnede
#  @author Emmanuel Agullo
#  @author Mathieu Faverge
#  @author Florent Pruvost
#  @author Samuel Thibault
#  @author Matthieu Kuhn
#  @author Loris Lucido
#  @author Terry Cojean
#  @date 2023-08-22
#
###
cmake_minimum_required(VERSION 3.5)

include(CheckSymbolExists)
include(CheckStructHasMember)

set(CHAMELEON_STARPU_VERSION "1.3" CACHE STRING "necessary STARPU API version")

find_package(STARPU ${CHAMELEON_STARPU_VERSION} REQUIRED)
morse_export_imported_target(MORSE STARPU starpu chameleon)

# check available functions
# note: recall STARPU_VERSION provides the version
if ( STARPU_FOUND )
  message("-- ${Blue}Add definition CHAMELEON_SCHED_STARPU"
    " - Activate StarPU in Chameleon${ColourReset}")
  morse_cmake_required_set( STARPU )
  check_function_exists(starpu_data_idle_prefetch_on_node HAVE_STARPU_IDLE_PREFETCH)
  if ( HAVE_STARPU_IDLE_PREFETCH )
    message("-- ${Blue}Add definition HAVE_STARPU_IDLE_PREFETCH${ColourReset}")
  endif()
  check_function_exists(starpu_iteration_push HAVE_STARPU_ITERATION_PUSH)
  if ( HAVE_STARPU_ITERATION_PUSH )
    message("-- ${Blue}Add definition HAVE_STARPU_ITERATION_PUSH${ColourReset}")
  endif()
  check_function_exists(starpu_data_wont_use HAVE_STARPU_DATA_WONT_USE)
  if ( HAVE_STARPU_DATA_WONT_USE )
    message("-- ${Blue}Add definition HAVE_STARPU_DATA_WONT_USE${ColourReset}")
  endif()
  check_function_exists(starpu_data_set_coordinates HAVE_STARPU_DATA_SET_COORDINATES)
  if ( HAVE_STARPU_DATA_SET_COORDINATES )
    message("-- ${Blue}Add definition HAVE_STARPU_DATA_SET_COORDINATES${ColourReset}")
  endif()
  check_function_exists(starpu_malloc_on_node_set_default_flags HAVE_STARPU_MALLOC_ON_NODE_SET_DEFAULT_FLAGS)
  if ( HAVE_STARPU_MALLOC_ON_NODE_SET_DEFAULT_FLAGS )
    message("-- ${Blue}Add definition HAVE_STARPU_MALLOC_ON_NODE_SET_DEFAULT_FLAGS${ColourReset}")
  endif()
  check_function_exists(starpu_data_set_ooc_flag HAVE_STARPU_DATA_SET_OOC_FLAG)
  if ( HAVE_STARPU_DATA_SET_OOC_FLAG )
    message("-- ${Blue}Add definition HAVE_STARPU_DATA_SET_OOC_FLAG${ColourReset}")
  endif()
  check_function_exists(starpu_interface_copy2d HAVE_STARPU_INTERFACE_COPY2D)
  if ( HAVE_STARPU_INTERFACE_COPY2D )
    message("-- ${Blue}Add definition HAVE_STARPU_INTERFACE_COPY2D${ColourReset}")
  endif()
  check_function_exists(starpu_data_peek HAVE_STARPU_DATA_PEEK)
  if ( HAVE_STARPU_DATA_PEEK )
    message("-- ${Blue}Add definition HAVE_STARPU_DATA_PEEK${ColourReset}")
  endif()
  check_function_exists(starpu_set_limit_min_submitted_tasks HAVE_STARPU_SET_LIMIT_SUBMITTED_TASKS )
  if ( HAVE_STARPU_SET_LIMIT_SUBMITTED_TASKS )
    message("-- ${Blue}Add definition HAVE_STARPU_SET_LIMIT_SUBMITTED_TASKS${ColourReset}")
  endif()
  check_function_exists(starpu_parallel_worker_init HAVE_STARPU_PARALLEL_WORKER)
  if ( HAVE_STARPU_PARALLEL_WORKER )
    message("-- ${Blue}Add definition HAVE_STARPU_PARALLEL_WORKER${ColourReset}")
  endif()
  check_struct_has_member( "struct starpu_data_interface_ops" reuse_data_on_node "starpu_data_interfaces.h" HAVE_STARPU_REUSE_DATA_ON_NODE LANGUAGE "C" )
  if ( HAVE_STARPU_REUSE_DATA_ON_NODE )
    message("-- ${Blue}Add definition HAVE_STARPU_REUSE_DATA_ON_NODE${ColourReset}")
  endif()

  if (CHAMELEON_USE_MPI)
    # Add MPI in case StarPU don't have a public dependency on it
    check_function_exists(starpu_mpi_init_conf HAVE_STARPU_MPI_INIT_CONF)
    if ( HAVE_STARPU_MPI_INIT_CONF )
      message("-- ${Blue}Add definition HAVE_STARPU_MPI_INIT_CONF${ColourReset}")
    endif()
    check_function_exists(starpu_mpi_wait_for_all HAVE_STARPU_MPI_WAIT_FOR_ALL)
    if ( HAVE_STARPU_MPI_WAIT_FOR_ALL )
      message("-- ${Blue}Add definition HAVE_STARPU_MPI_WAIT_FOR_ALL${ColourReset}")
    endif()
    check_function_exists(starpu_mpi_data_migrate HAVE_STARPU_MPI_DATA_MIGRATE)
    if ( HAVE_STARPU_MPI_DATA_MIGRATE )
      message("-- ${Blue}Add definition HAVE_STARPU_MPI_DATA_MIGRATE${ColourReset}")
    elseif(CHAMELEON_USE_MIGRATE)
      set(CHAMELEON_USE_MIGRATE "OFF")
      message("-- ${Blue}CHAMELEON_USE_MIGRATE is turned OFF because starpu_mpi_data_migrate not found${ColourReset}")
    endif()
    check_function_exists(starpu_mpi_interface_datatype_node_register HAVE_STARPU_MPI_INTERFACE_DATATYPE_NODE_REGISTER)
    check_function_exists(starpu_mpi_interface_datatype_register HAVE_STARPU_MPI_INTERFACE_DATATYPE_REGISTER)
    if ( HAVE_STARPU_MPI_INTERFACE_DATATYPE_NODE_REGISTER )
      message("-- ${Blue}Add definition HAVE_STARPU_MPI_INTERFACE_DATATYPE_NODE_REGISTER${ColourReset}")
    elseif ( HAVE_STARPU_MPI_INTERFACE_DATATYPE_REGISTER )
      message("-- ${Blue}Add definition HAVE_STARPU_MPI_INTERFACE_DATATYPE_REGISTER${ColourReset}")
    else()
      if( CHAMELEON_USE_MPI_DATATYPES )
        set( CHAMELEON_USE_MPI_DATATYPES OFF CACHE BOOL "Enable MPI datatypes whenever supported by the runtime" FORCE)
      endif()
    endif()
    check_function_exists(starpu_mpi_data_register_comm HAVE_STARPU_MPI_DATA_REGISTER)
    if ( HAVE_STARPU_MPI_DATA_REGISTER )
      message("-- ${Blue}Add definition HAVE_STARPU_MPI_DATA_REGISTER - Activate"
        " use of starpu_mpi_data_register() in Chameleon with StarPU${ColourReset}")
    endif()
    check_function_exists(starpu_mpi_comm_rank HAVE_STARPU_MPI_COMM_RANK)
    if ( HAVE_STARPU_MPI_COMM_RANK )
      message("-- ${Blue}Add definition HAVE_STARPU_MPI_COMM_RANK - Activate"
        " use of starpu_mpi_comm_rank() in Chameleon with StarPU${ColourReset}")
    endif()
    check_function_exists(starpu_mpi_cached_receive HAVE_STARPU_MPI_CACHED_RECEIVE)
    if ( HAVE_STARPU_MPI_CACHED_RECEIVE )
      message("-- ${Blue}Add definition HAVE_STARPU_MPI_CACHED_RECEIVE${ColourReset}")
    endif()
    check_function_exists(starpu_mpi_comm_get_attr HAVE_STARPU_MPI_COMM_GET_ATTR)
    if ( HAVE_STARPU_MPI_COMM_GET_ATTR )
      message("-- ${Blue}Add definition HAVE_STARPU_MPI_COMM_GET_ATTR${ColourReset}")
    endif()
    check_function_exists(starpu_mpi_redux_data_prio_tree HAVE_STARPU_MPI_REDUX)
    if ( HAVE_STARPU_MPI_REDUX )
      message("-- ${Blue}Add definition HAVE_STARPU_MPI_REDUX${ColourReset}")
    endif()
  endif()

  if (CHAMELEON_USE_CUDA AND NOT CHAMELEON_SIMULATION)
    check_function_exists(starpu_cublas_get_local_handle HAVE_STARPU_CUBLAS_GET_LOCAL_HANDLE)
    if ( NOT HAVE_STARPU_CUBLAS_GET_LOCAL_HANDLE )
      message(FATAL_ERROR "The detected StarPU library does not include cublas support. Please disable CHAMELEON_USE_CUDA or provide a StarPU library with cublas support.")
    endif()
  endif()

  if (CHAMELEON_USE_HIP AND NOT CHAMELEON_SIMULATION)
    check_function_exists(starpu_hipblas_get_local_handle HAVE_STARPU_HIPBLAS_GET_LOCAL_HANDLE)
    if ( NOT HAVE_STARPU_HIPBLAS_GET_LOCAL_HANDLE )
      message(FATAL_ERROR "The detected StarPU library does not include hipblas support. Please disable CHAMELEON_USE_HIP_{CUDA,ROC} or provide a StarPU library with hipblas support.")
    endif()
  endif()

  morse_cmake_required_unset()
endif ( STARPU_FOUND )

configure_file("include/chameleon_starpu.h.in"
               "include/chameleon_starpu.h"
               @ONLY)

# Generate headers for all possible precisions
# --------------------------------------------
set(RUNTIME_HDRS_GENERATED "")
set(ZHDR
  include/runtime_codelet_z.h
  include/runtime_codelet_zc.h
  )

precisions_rules_py(RUNTIME_HDRS_GENERATED "${ZHDR}"
  PRECISIONS "s;d;c;z;ds;zc"
  TARGETDIR  "include")

# Define the list of headers
# --------------------------
set(RUNTIME_HDRS
  ${CMAKE_CURRENT_BINARY_DIR}/include/chameleon_starpu.h
  include/runtime_codelet_profile.h
  include/runtime_codelets.h
  include/runtime_profiling.h
  include/runtime_workspace.h
  include/cham_tile_interface.h
  )

# Add generated headers
# ---------------------
foreach( hdr_file ${RUNTIME_HDRS_GENERATED} )
  list(APPEND RUNTIME_HDRS ${CMAKE_CURRENT_BINARY_DIR}/${hdr_file})
endforeach()

# Force generation of headers
# ---------------------------
add_custom_target(
  runtime_starpu_include
  ALL SOURCES ${RUNTIME_HDRS})

# Installation
# ------------
install(
  FILES ${RUNTIME_HDRS}
  DESTINATION include/runtime/starpu )

# Generate the Chameleon common for all possible precisions
# ---------------------------------------------------------
set(RUNTIME_COMMON_GENERATED "")
set(ZSRC
  control/runtime_zprofiling.c
  control/runtime_zlocality.c
  )

precisions_rules_py(
  RUNTIME_COMMON_GENERATED "${ZSRC}"
  PRECISIONS "${CHAMELEON_PRECISION}"
  TARGETDIR "control")

set(RUNTIME_COMMON
  control/runtime_async.c
  control/runtime_context.c
  control/runtime_control.c
  control/runtime_descriptor.c
  control/runtime_descriptor_ipiv.c
  control/runtime_tags.c
  control/runtime_options.c
  control/runtime_profiling.c
  control/runtime_workspace.c
  interface/cham_tile_interface.c
  interface/cppi_interface.c
  ${RUNTIME_COMMON_GENERATED}
  )

set(flags_to_add "")
foreach(_prec ${CHAMELEON_PRECISION})
  set(flags_to_add "${flags_to_add} -DPRECISION_${_prec}")
endforeach()
set_source_files_properties(control/runtime_profiling.c PROPERTIES COMPILE_FLAGS "${flags_to_add}")

# Generate the Chameleon sources for all possible precisions
# ----------------------------------------------------------
set(RUNTIME_SRCS_GENERATED "")
set(ZSRC
  codelets/codelet_zcallback.c
  codelets/codelet_zccallback.c
  codelets/codelet_dlag2h.c
  ${CODELETS_ZSRC}
  )

precisions_rules_py(RUNTIME_SRCS_GENERATED "${ZSRC}"
  PRECISIONS "${CHAMELEON_PRECISION}"
  TARGETDIR "codelets")

set(CODELETS_SRC
  codelets/codelet_convert.c
  codelets/codelet_hgemm.c
  codelets/codelet_gemm.c
  ${CODELETS_SRC}
  )

set(RUNTIME_SRCS
  ${RUNTIME_COMMON}
  ${RUNTIME_SRCS_GENERATED}
  ${CODELETS_SRC}
  )

# Force generation of sources
# ---------------------------
add_custom_target(starpu_sources ALL SOURCES ${RUNTIME_SRCS})
set(CHAMELEON_SOURCES_TARGETS "${CHAMELEON_SOURCES_TARGETS};runtime_starpu_include;starpu_sources" CACHE INTERNAL "List of targets of sources")

# Add library
# -----------
add_library(chameleon_starpu ${RUNTIME_SRCS})
set_target_properties(chameleon_starpu PROPERTIES VERSION ${CHAMELEON_VERSION})
set_target_properties(chameleon_starpu PROPERTIES SOVERSION ${CHAMELEON_VERSION_MAJOR})
set_property(TARGET chameleon_starpu PROPERTY INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")

add_dependencies(chameleon_starpu
  chameleon_include
  control_include
  runtime_starpu_include
  starpu_sources
  )
if (NOT CHAMELEON_SIMULATION)
  add_dependencies(chameleon_starpu coreblas_include)
  if (CHAMELEON_USE_CUDA)
    add_dependencies(chameleon_starpu gpucublas_include)
  endif()
  if (CHAMELEON_USE_HIP)
    add_dependencies(chameleon_starpu gpuhipblas_include)
  endif()
endif()

target_include_directories(chameleon_starpu PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  $<BUILD_INTERFACE:${CHAMELEON_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CHAMELEON_BINARY_DIR}>
  $<INSTALL_INTERFACE:include>)
target_include_directories(chameleon_starpu PRIVATE
  $<BUILD_INTERFACE:${CHAMELEON_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CHAMELEON_BINARY_DIR}/include>
  $<BUILD_INTERFACE:${CHAMELEON_SOURCE_DIR}/coreblas/include>
  $<BUILD_INTERFACE:${CHAMELEON_BINARY_DIR}/coreblas/include>)

if (CHAMELEON_SIMULATION)
  # need PUBLIC here because simulation mode with starpu requires to replace the
  # main function by a starpu main one
  target_link_libraries(chameleon_starpu PUBLIC MORSE::STARPU)
else()
  target_link_libraries(chameleon_starpu PRIVATE MORSE::STARPU)
endif()
if (NOT CHAMELEON_SIMULATION)
  target_link_libraries(chameleon_starpu PRIVATE coreblas)
  if(CHAMELEON_USE_CUDA)
    target_link_libraries(chameleon_starpu PRIVATE gpucublas)
  endif(CHAMELEON_USE_CUDA)
  if(CHAMELEON_USE_HIP)
    target_link_libraries(chameleon_starpu PRIVATE gpuhipblas)
  endif(CHAMELEON_USE_HIP)
endif(NOT CHAMELEON_SIMULATION)
target_link_libraries(chameleon_starpu PRIVATE MORSE::M)

# necessary cause of include control/common.h which depends on hqr, mpi, cuda, ...
# TODO: remove this dependency as soon as headers are clean
target_link_libraries(chameleon_starpu PUBLIC hqr)
if (CHAMELEON_USE_MPI)
  target_link_libraries(chameleon_starpu PUBLIC MPI::MPI_C)
endif()
if (NOT CHAMELEON_SIMULATION)
  if (CHAMELEON_USE_CUDA)
    target_link_libraries(chameleon_starpu PUBLIC CUDA::CUBLAS)
  endif()
  if (CHAMELEON_USE_HIP)
    target_link_libraries(chameleon_starpu PUBLIC HIP::HIPBLAS)
  endif()
endif()

# export target
install(EXPORT chameleon_starpuTargets
        NAMESPACE CHAMELEON::
        DESTINATION lib/cmake/chameleon
        )

# installation
# ------------
install(TARGETS chameleon_starpu
        EXPORT chameleon_starpuTargets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        )

###
### END CMakeLists.txt
###
