/**
 *
 * @file openmp/codelet_dzasum.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2019 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon dzasum OpenMP codelet
 *
 * @version 0.9.2
 * @comment This file has been automatically generated
 *          from Plasma 2.6.0 for CHAMELEON 0.9.2
 * @author Florent Pruvost
 * @author Philippe Virouleau
 * @date 2018-06-15
 * @precisions normal z -> c d s
 *
 */
#include "chameleon_openmp.h"
#include "coreblas/coreblas_z.h"
#include "chameleon/tasks_z.h"

void INSERT_TASK_dzasum(const RUNTIME_option_t *options,
                       cham_store_t storev, cham_uplo_t uplo, int M, int N,
                       const CHAM_desc_t *A, int Am, int An, int lda,
                       const CHAM_desc_t *B, int Bm, int Bn)
{
    CHAMELEON_Complex64_t *ptrA = RTBLKADDR( A, CHAMELEON_Complex64_t, Am, An );
    double *ptrB = RTBLKADDR( B, double, Bm, Bn );
#pragma omp task firstprivate(storev, uplo, M, N, lda, ptrA, ptrB) depend(in:ptrA[0]) depend(inout:ptrB[0])
    CORE_dzasum(storev, uplo, M, N, ptrA, lda, ptrB);
}

