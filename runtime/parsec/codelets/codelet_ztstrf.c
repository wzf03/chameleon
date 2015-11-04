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
 * @file codelet_ztstrf.c
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
CORE_ztstrf_parsec(dague_execution_unit_t *context, dague_execution_context_t * this_task)
{
    int *m;
    int *n;
    int *ib;
    int *nb;
    dague_data_copy_t *gU;
    int *ldu;
    dague_data_copy_t *gA;
    int *lda;
    dague_data_copy_t *gL;
    int *ldl;
    int *IPIV;
    MORSE_Complex64_t *WORK;
    int *ldwork;
    MORSE_bool *check_info;
    int *iinfo;

    int info;

    dague_dtd_unpack_args(this_task,
                          UNPACK_VALUE, &m,
                          UNPACK_VALUE, &n,
                          UNPACK_VALUE, &ib,
                          UNPACK_VALUE, &nb,
                          UNPACK_DATA,  &gU,
                          UNPACK_VALUE, &ldu,
                          UNPACK_DATA,  &gA,
                          UNPACK_VALUE, &lda,
                          UNPACK_DATA,  &gL,
                          UNPACK_VALUE, &ldl,
                          UNPACK_SCRATCH, &IPIV,
                          UNPACK_SCRATCH, &WORK,
                          UNPACK_VALUE, &ldwork,
                          UNPACK_VALUE, &check_info,
                          UNPACK_VALUE, &iinfo
                        );

    void *U = DAGUE_DATA_COPY_GET_PTR((dague_data_copy_t *)gU);
    void *A = DAGUE_DATA_COPY_GET_PTR((dague_data_copy_t *)gA);
    void *L = DAGUE_DATA_COPY_GET_PTR((dague_data_copy_t *)gL);

    CORE_ztstrf(*m, *n, *ib, *nb, U, *ldu, A, *lda, L, *ldl, IPIV, WORK, *ldwork, &info);

    return 0;
}

void MORSE_TASK_ztstrf(MORSE_option_t *options,
                       int m, int n, int ib, int nb,
                       MORSE_desc_t *U, int Um, int Un, int ldu,
                       MORSE_desc_t *A, int Am, int An, int lda,
                       MORSE_desc_t *L, int Lm, int Ln, int ldl,
                       int *IPIV,
                       MORSE_bool check_info, int iinfo)
{
    dague_dtd_handle_t* DAGUE_dtd_handle = (dague_dtd_handle_t *)(options->sequence->schedopt);

    insert_task_generic_fptr(DAGUE_dtd_handle,      CORE_ztstrf_parsec,               "tstrf",
                             sizeof(int),           &m,                                VALUE,
                             sizeof(int),           &n,                                VALUE,
                             sizeof(int),           &ib,                               VALUE,
                             sizeof(int),           &nb,                               VALUE,
                             PASSED_BY_REF,         RTBLKADDR( U, MORSE_Complex64_t, Um, Un ),     INOUT | REGION_FULL,
                             sizeof(int),           &ldu,                              VALUE,
                             PASSED_BY_REF,         RTBLKADDR( A, MORSE_Complex64_t, Am, An ),     INOUT | REGION_FULL,
                             sizeof(int),           &lda,                              VALUE,
                             PASSED_BY_REF,         RTBLKADDR( L, MORSE_Complex64_t, Lm, Ln ),     OUTPUT | REGION_FULL,
                             sizeof(int),           &ldl,                              VALUE,
                             sizeof(int)*nb,        IPIV,                              SCRATCH,
                             sizeof(MORSE_Complex64_t)*ib*nb,    NULL,                 SCRATCH,
                             sizeof(int),           &nb,                               VALUE,
                             sizeof(int),           &check_info,                       VALUE,
                             sizeof(int),           &iinfo,                            VALUE,
                             0);
}