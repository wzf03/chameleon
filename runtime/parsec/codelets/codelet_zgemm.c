/**
 *
 * @file parsec/codelet_zgemm.c
 *
 * @copyright 2009-2015 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2018 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon zgemm PaRSEC codelet
 *
 * @version 1.0.0
 * @author Reazul Hoque
 * @precisions normal z -> c d s
 *
 */
#include "chameleon_parsec.h"
#include "chameleon/tasks_z.h"
#include "coreblas/coreblas_z.h"

/**
 *
 * @ingroup INSERT_TASK_Complex64_t
 *
 */
static inline int
CORE_zgemm_parsec( parsec_execution_stream_t *context,
                   parsec_task_t             *this_task )
{
    cham_trans_t transA;
    cham_trans_t transB;
    int m;
    int n;
    int k;
    CHAMELEON_Complex64_t alpha;
    CHAMELEON_Complex64_t *A;
    int lda;
    CHAMELEON_Complex64_t *B;
    int ldb;
    CHAMELEON_Complex64_t beta;
    CHAMELEON_Complex64_t *C;
    int ldc;

    parsec_dtd_unpack_args(
        this_task, &transA, &transB, &m, &n, &k, &alpha, &A, &lda, &B, &ldb, &beta, &C, &ldc );

    CORE_zgemm( transA, transB, m, n, k,
               alpha, A, lda,
                      B, ldb,
               beta,  C, ldc);

    (void)context;
    return PARSEC_HOOK_RETURN_DONE;
}

void INSERT_TASK_zgemm( const RUNTIME_option_t *options,
                       cham_trans_t transA, cham_trans_t transB,
                       int m, int n, int k, int nb,
                       CHAMELEON_Complex64_t alpha, const CHAM_desc_t *A, int Am, int An, int lda,
                                                const CHAM_desc_t *B, int Bm, int Bn, int ldb,
                       CHAMELEON_Complex64_t beta,  const CHAM_desc_t *C, int Cm, int Cn, int ldc )
{
    parsec_taskpool_t* PARSEC_dtd_taskpool = (parsec_taskpool_t *)(options->sequence->schedopt);

    parsec_dtd_taskpool_insert_task(
        PARSEC_dtd_taskpool, CORE_zgemm_parsec, options->priority, "Gemm",
        sizeof(int),    &transA,                           VALUE,
        sizeof(int),    &transB,                           VALUE,
        sizeof(int),           &m,                                VALUE,
        sizeof(int),           &n,                                VALUE,
        sizeof(int),           &k,                                VALUE,
        sizeof(CHAMELEON_Complex64_t),           &alpha,              VALUE,
        PASSED_BY_REF,     RTBLKADDR( A, CHAMELEON_Complex64_t, Am, An ), chameleon_parsec_get_arena_index( A ) | INPUT,
        sizeof(int),           &lda,                              VALUE,
        PASSED_BY_REF,     RTBLKADDR( B, CHAMELEON_Complex64_t, Bm, Bn ), chameleon_parsec_get_arena_index( B ) | INPUT,
        sizeof(int),           &ldb,                              VALUE,
        sizeof(CHAMELEON_Complex64_t),           &beta,               VALUE,
        PASSED_BY_REF,     RTBLKADDR( C, CHAMELEON_Complex64_t, Cm, Cn ), chameleon_parsec_get_arena_index( C ) | INOUT | AFFINITY,
        sizeof(int),           &ldc,                              VALUE,
        PARSEC_DTD_ARG_END );

    (void)nb;
}
