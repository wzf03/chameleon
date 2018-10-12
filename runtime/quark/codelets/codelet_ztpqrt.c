/**
 *
 * @file codelet_ztpqrt.c
 *
 * @copyright 2009-2016 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2018 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon ztpqrt Quark codelet
 *
 * @version 1.0.0
 * @author Mathieu Faverge
 * @date 2016-12-15
 * @precisions normal z -> s d c
 *
 */
#include "chameleon_quark.h"
#include "chameleon/tasks_z.h"
#include "coreblas/coreblas_z.h"

static void
CORE_ztpqrt_quark( Quark *quark )
{
    int M;
    int N;
    int L;
    int ib;
    CHAMELEON_Complex64_t *A;
    int lda;
    CHAMELEON_Complex64_t *B;
    int ldb;
    CHAMELEON_Complex64_t *T;
    int ldt;
    CHAMELEON_Complex64_t *WORK;

    quark_unpack_args_11( quark, M, N, L, ib,
                          A, lda, B, ldb, T, ldt, WORK );

    CORE_ztpqrt( M, N, L, ib,
                 A, lda, B, ldb, T, ldt, WORK );
}

void INSERT_TASK_ztpqrt( const RUNTIME_option_t *options,
                         int M, int N, int L, int ib, int nb,
                         const CHAM_desc_t *A, int Am, int An, int lda,
                         const CHAM_desc_t *B, int Bm, int Bn, int ldb,
                         const CHAM_desc_t *T, int Tm, int Tn, int ldt )
{
    quark_option_t *opt = (quark_option_t*)(options->schedopt);
    DAG_CORE_TSQRT;

    int shapeB = ( L == 0 ) ? 0 : (QUARK_REGION_U | QUARK_REGION_D);

    QUARK_Insert_Task(
        opt->quark, CORE_ztpqrt_quark, (Quark_Task_Flags*)opt,
        sizeof(int),                         &M,   VALUE,
        sizeof(int),                         &N,   VALUE,
        sizeof(int),                         &L,   VALUE,
        sizeof(int),                         &ib,  VALUE,
        sizeof(CHAMELEON_Complex64_t)*nb*nb,      RTBLKADDR( A, CHAMELEON_Complex64_t, Am, An ), INOUT | QUARK_REGION_U | QUARK_REGION_D,
        sizeof(int),                         &lda, VALUE,
        sizeof(CHAMELEON_Complex64_t)*nb*nb,      RTBLKADDR( B, CHAMELEON_Complex64_t, Bm, Bn ), INOUT | shapeB | LOCALITY,
        sizeof(int),                         &ldb, VALUE,
        sizeof(CHAMELEON_Complex64_t)*nb*ib,      RTBLKADDR( T, CHAMELEON_Complex64_t, Tm, Tn ), OUTPUT,
        sizeof(int),                         &ldt, VALUE,
        sizeof(CHAMELEON_Complex64_t)*(ib+1)*nb,  NULL, SCRATCH,
        0);
}
