/**
 *
 * @file openmp/codelet_zplghe.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2018 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon zplghe StarPU codelet
 *
 * @version 1.0.0
 * @comment This file has been automatically generated
 *          from Plasma 2.5.0 for CHAMELEON 1.0.0
 * @author Piotr Luszczek
 * @author Pierre Lemarinier
 * @author Mathieu Faverge
 * @author Emmanuel Agullo
 * @author Cedric Castagnede
 * @author Philippe Virouleau
 * @date 2018-06-20
 * @precisions normal z -> c
 *
 */
#include "chameleon_openmp.h"
#include "chameleon/tasks_z.h"
#include "coreblas/coreblas_z.h"

/*   INSERT_TASK_zplghe - Generate a tile for random hermitian (positive definite if bump is large enough) matrix. */

void INSERT_TASK_zplghe( const RUNTIME_option_t *options,
                        double bump, int m, int n, const CHAM_desc_t *A, int Am, int An, int lda,
                        int bigM, int m0, int n0, unsigned long long int seed )
{
    CHAMELEON_Complex64_t *ptrA = RTBLKADDR(A, CHAMELEON_Complex64_t, Am, An);
#pragma omp task firstprivate(bump, m, n, ptrA, lda, bigM, m0, n0, seed) depend(inout:ptrA[0:Am*An])
    CORE_zplghe( bump, m, n, ptrA, lda, bigM, m0, n0, seed );
}