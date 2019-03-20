/**
 *
 * @file cuda_zher2k.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2019 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon cuda_zher2k GPU kernel
 *
 * @version 0.9.2
 * @author Florent Pruvost
 * @date 2015-09-17
 * @precisions normal z -> c
 *
 */
#include "cudablas.h"

int CUDA_zher2k(cham_uplo_t uplo, cham_trans_t trans,
                int n, int k,
                cuDoubleComplex *alpha,
                const cuDoubleComplex *A, int lda,
                const cuDoubleComplex *B, int ldb,
                double *beta,
                cuDoubleComplex *C, int ldc,
                CUBLAS_STREAM_PARAM)
{
    cublasZher2k(CUBLAS_HANDLE
                 chameleon_cublas_const(uplo), chameleon_cublas_const(trans),
                 n, k,
                 CUBLAS_VALUE(alpha), A, lda,
                                      B, ldb,
                 CUBLAS_VALUE(beta),  C, ldc);

    assert( CUBLAS_STATUS_SUCCESS == cublasGetError() );

    return CHAMELEON_SUCCESS;
}
