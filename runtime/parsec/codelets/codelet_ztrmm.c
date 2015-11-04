/**
 *
 * @copyright (c) 2009-2015 The University of Tennessee and The University
 *                          of Tennessee Research Foundation.
 *                          All rights reserved.
 * @copyright (c) 2012-2015 Inria. All rights reserved.
 * @copyright (c) 2012-2015 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria, Univ. Bordeaux. All rights reserved.
 *
 **/

/**
 * @file codelet_ztrmm.c
 *
 *  MORSE codelets kernel
 *  MORSE is a software package provided by Univ. of Tennessee,
 *  Univ. of California Berkeley and Univ. of Colorado Denver
 *
 * @version 2.5.0
 * @author Reazul Hoque
 * @precisions normal z -> c d s
 *
 **/
#include "runtime/parsec/include/morse_parsec.h"

static int
CORE_ztrmm_parsec(dague_execution_unit_t *context, dague_execution_context_t * this_task)
{
    MORSE_enum *side;
    MORSE_enum *uplo;
    MORSE_enum *transA;
    MORSE_enum *diag;
    int *M;
    int *N;
    MORSE_Complex64_t *alpha;
    dague_data_copy_t *gA;
    int *LDA;
    dague_data_copy_t *gB;
    int *LDB;

    dague_dtd_unpack_args(this_task,
                          UNPACK_VALUE, &side,
                          UNPACK_VALUE, &uplo,
                          UNPACK_VALUE, &transA,
                          UNPACK_VALUE, &diag,
                          UNPACK_VALUE, &M,
                          UNPACK_VALUE, &N,
                          UNPACK_VALUE, &alpha,
                          UNPACK_DATA,  &gA,
                          UNPACK_VALUE, &LDA,
                          UNPACK_DATA,  &gB,
                          UNPACK_VALUE, &LDB
                        );

    void *A = DAGUE_DATA_COPY_GET_PTR((dague_data_copy_t *)gA);
    void *B = DAGUE_DATA_COPY_GET_PTR((dague_data_copy_t *)gB);

    CORE_ztrmm(*side, *uplo,
        *transA, *diag,
        *M, *N,
        *alpha, A, *LDA,
        B, *LDB);

    return 0;
}

void MORSE_TASK_ztrmm(MORSE_option_t *options,
                      MORSE_enum side, MORSE_enum uplo, MORSE_enum transA, MORSE_enum diag,
                      int m, int n, int nb,
                      MORSE_Complex64_t alpha, MORSE_desc_t *A, int Am, int An, int lda,
                      MORSE_desc_t *B, int Bm, int Bn, int ldb)
{
    dague_dtd_handle_t* DAGUE_dtd_handle = (dague_dtd_handle_t *)(options->sequence->schedopt);

    insert_task_generic_fptr(DAGUE_dtd_handle,      CORE_ztrmm_parsec,     "trmm",
                            sizeof(MORSE_enum),     &side,                  VALUE,
                            sizeof(MORSE_enum),     &uplo,                  VALUE,
                            sizeof(MORSE_enum),     &transA,                VALUE,
                            sizeof(MORSE_enum),     &diag,                  VALUE,
                            sizeof(int),            &m,                     VALUE,
                            sizeof(int),            &n,                     VALUE,
                            sizeof(MORSE_Complex64_t),         &alpha,      VALUE,
                            PASSED_BY_REF,          RTBLKADDR( A, MORSE_Complex64_t, Am, An ),     INPUT | REGION_FULL,
                            sizeof(int),            &lda,                   VALUE,
                            PASSED_BY_REF,          RTBLKADDR( B, MORSE_Complex64_t, Bm, Bn ),     INOUT | REGION_FULL,
                            sizeof(int),            &ldb,                   VALUE,
                            0);
}