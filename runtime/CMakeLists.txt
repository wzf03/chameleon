###
#
# @file CMakeLists.txt
#
# @copyright 2009-2014 The University of Tennessee and The University of
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
#  @author Philippe Virouleau
#  @author Matthieu Kuhn
#  @date 2023-09-11
#
###

# List of codelets required by all runtimes
# -----------------------------------------
set(CODELETS_ZSRC
    codelets/codelet_zlag2c.c
    codelets/codelet_dlag2z.c
    codelets/codelet_dzasum.c
    ##################
    # BLAS 1
    ##################
    codelets/codelet_zaxpy.c
    ##################
    # BLAS 2
    ##################
    codelets/codelet_zgemv.c
    ##################
    # BLAS 3
    ##################
    codelets/codelet_zgemm.c
    codelets/codelet_zhemm.c
    codelets/codelet_zher2k.c
    codelets/codelet_zherk.c
    codelets/codelet_zsymm.c
    codelets/codelet_zsyr2k.c
    codelets/codelet_zsyrk.c
    codelets/codelet_ztrmm.c
    codelets/codelet_ztrsm.c
    ##################
    # LAPACK
    ##################
    codelets/codelet_zgeadd.c
    codelets/codelet_zlascal.c
    codelets/codelet_zgelqt.c
    codelets/codelet_zgeqrt.c
    codelets/codelet_zgessm.c
    codelets/codelet_zgessq.c
    codelets/codelet_zgetrf.c
    codelets/codelet_zgetrf_incpiv.c
    codelets/codelet_zgetrf_nopiv.c
    codelets/codelet_zgetrf_nopiv_percol.c
    codelets/codelet_zgetrf_percol.c
    codelets/codelet_zgetrf_blocked.c
    codelets/codelet_zhe2ge.c
    codelets/codelet_zherfb.c
    codelets/codelet_zhessq.c
    codelets/codelet_zlacpy.c
    codelets/codelet_zlange.c
    codelets/codelet_zlanhe.c
    codelets/codelet_zlansy.c
    codelets/codelet_zlantr.c
    codelets/codelet_zlaset.c
    codelets/codelet_zlaset2.c
    codelets/codelet_zlaswp.c
    codelets/codelet_zlatro.c
    codelets/codelet_zlauum.c
    codelets/codelet_zplghe.c
    codelets/codelet_zplgsy.c
    codelets/codelet_zplrnt.c
    codelets/codelet_zplssq.c
    codelets/codelet_zpotrf.c
    codelets/codelet_zssssm.c
    codelets/codelet_zsyssq.c
    codelets/codelet_zsytrf_nopiv.c
    codelets/codelet_ztplqt.c
    codelets/codelet_ztpqrt.c
    codelets/codelet_ztpmlqt.c
    codelets/codelet_ztpmqrt.c
    codelets/codelet_ztradd.c
    codelets/codelet_ztrasm.c
    codelets/codelet_ztrssq.c
    codelets/codelet_ztrtri.c
    codelets/codelet_ztsmlq_hetra1.c
    codelets/codelet_ztsmqr_hetra1.c
    codelets/codelet_ztstrf.c
    codelets/codelet_zunmlq.c
    codelets/codelet_zunmqr.c
    ##################
    # BUILD
    ##################
    codelets/codelet_zbuild.c
    ##################
    # CENTERED-SCALED and GRAM
    ##################
    codelets/codelet_zgesum.c
    codelets/codelet_zcesca.c
    codelets/codelet_zgram.c
    ##################
    # Reduction methods
    ##################
    codelets/codelet_zgersum.c
    ##################
    # Precision modification kernels
    ##################
    codelets/codelet_zgered.c
    codelets/codelet_zgerst.c
    )

set(CODELETS_SRC
  codelets/codelet_map.c
  codelets/codelet_ipiv_to_perm.c
)

# Check for the subdirectories
# ----------------------------
if( CHAMELEON_SCHED_QUARK )
    add_subdirectory(quark)
elseif( CHAMELEON_SCHED_PARSEC )
    add_subdirectory(parsec)
elseif( CHAMELEON_SCHED_STARPU )
    add_subdirectory(starpu)
elseif( CHAMELEON_SCHED_OPENMP )
    add_subdirectory(openmp)
endif()

###
### END CMakeLists.txt
###
