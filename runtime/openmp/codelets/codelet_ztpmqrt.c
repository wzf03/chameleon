/**
 *
 * @file openmp/codelet_ztpmqrt.c
 *
 * @copyright 2009-2016 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2018 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 * @brief Chameleon ztpmqrt StarPU codelet
 *
 * @version 1.0.0
 * @author Mathieu Faverge
 * @date 2016-12-15
 * @precisions normal z -> s d c
 *
 */
#include "chameleon_openmp.h"
#include "chameleon/tasks_z.h"
void
INSERT_TASK_ztpmqrt( const RUNTIME_option_t *options,
                    cham_side_t side, cham_trans_t trans,
                    int M, int N, int K, int L, int ib, int nb,
                    const CHAM_desc_t *V, int Vm, int Vn, int ldv,
                    const CHAM_desc_t *T, int Tm, int Tn, int ldt,
                    const CHAM_desc_t *A, int Am, int An, int lda,
                    const CHAM_desc_t *B, int Bm, int Bn, int ldb )
{
    CHAMELEON_Complex64_t *ptrA = RTBLKADDR(A, CHAMELEON_Complex64_t, Am, An);
    CHAMELEON_Complex64_t *ptrB = RTBLKADDR(B, CHAMELEON_Complex64_t, Bm, Bn);
    CHAMELEON_Complex64_t *ptrT = RTBLKADDR(T, CHAMELEON_Complex64_t, Tm, Tn);
    CHAMELEON_Complex64_t *ptrV = RTBLKADDR(V, CHAMELEON_Complex64_t, Vm, Vn);
    int ws_size = options->ws_wsize;
#pragma omp task firstprivate(ws_size, side, trans, M, N, K, L, ib, nb, ptrV, ldv, ptrT, ldt, ptrA, lda, ptrB, ldb) depend(in:ptrV[0], ptrT[0]) depend(inout:ptrA[0], ptrB[0])
    {
        CHAMELEON_Complex64_t tmp[ws_size];
        CORE_ztpmqrt( side, trans, M, N, K, L, ib,
                      ptrV, ldv, ptrT, ldt, ptrA, lda, ptrB, ldb, tmp );
    }
}