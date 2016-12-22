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
 * @file cuda_zssssm.c
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
int CUDA_zssssm(
        magma_storev_t storev, magma_int_t m1, magma_int_t n1,
        magma_int_t m2, magma_int_t n2, magma_int_t k, magma_int_t ib,
        magmaDoubleComplex *dA1, magma_int_t ldda1,
        magmaDoubleComplex *dA2, magma_int_t ldda2,
        magmaDoubleComplex *dL1, magma_int_t lddl1,
        magmaDoubleComplex *dL2, magma_int_t lddl2,
        magma_int_t *IPIV, magma_int_t *info)
{
    magma_zssssm_gpu(
        storev, m1, n1, m2, n2, k, ib,
        dA1, ldda1, dA2, ldda2,
        dL1, lddl1, dL2, lddl2,
        IPIV, info);
    return MORSE_SUCCESS;
}
#endif
#endif
