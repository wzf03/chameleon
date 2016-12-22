/**
 *
 * @copyright (c) 2009-2014 The University of Tennessee and The University
 *                          of Tennessee Research Foundation.
 *                          All rights reserved.
 * @copyright (c) 2012-2016 Inria. All rights reserved.
 * @copyright (c) 2012-2014 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria, Univ. Bordeaux. All rights reserved.
 *
 **/

/**
 *
 * @file cuda_zgessm.c
 *
 *  MORSE cudablas kernel
 *  MORSE is a software package provided by Univ. of Tennessee,
 *  Univ. of California Berkeley and Univ. of Colorado Denver,
 *  and INRIA Bordeaux Sud-Ouest
 *
 * @author Florent Pruvost
 * @date 2015-09-16
 * @precisions normal z -> c d s
 *
 **/
#include "cudablas/include/cudablas.h"
#include "cudablas/include/cudablas_z.h"

#if defined(CHAMELEON_USE_MAGMA)
#if defined(HAVE_MAGMA_GETRF_INCPIV_GPU)
int CUDA_zgessm(
        char storev, magma_int_t m, magma_int_t n, magma_int_t k, magma_int_t ib,
        magma_int_t *ipiv,
        cuDoubleComplex *dL1, magma_int_t lddl1,
        cuDoubleComplex *dL,  magma_int_t lddl,
        cuDoubleComplex *dA,  magma_int_t ldda,
        magma_int_t *info)
{
    int ret;
    /* The kernel is just using the inverted part or nothing */
    if ( lddl1 >= 2*ib ) {
      dL1 += ib;
      ret = magma_zgessm_gpu( storev, m, n, k, ib,
                  ipiv, dL1, lddl1, dL, lddl, dA, ldda, info );
    }
    else {
      ret = magma_zgessm_gpu( storev, m, n, k, ib,
              ipiv, NULL, 1, dL, lddl, dA, ldda, info );
    }

    if (ret != MAGMA_SUCCESS) {
        fprintf(stderr, "Error in MAGMA: %d\n", ret);
        exit(-1);
    }

    return MORSE_SUCCESS;
}
#endif
#endif
