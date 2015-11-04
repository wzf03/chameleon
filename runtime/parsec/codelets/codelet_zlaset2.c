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
 * @file codelet_zlaset2.c
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

/**
 *
 * @ingroup CORE_MORSE_Complex64_t
 *
 *  CORE_zlaset2 - Sets the elements of the matrix A to alpha.
 *  Not LAPACK compliant! Read below.
 *
 *******************************************************************************
 *
 * @param[in] uplo
 *          Specifies which elements of the matrix are to be set
 *          = MorseUpper: STRICT Upper part of A is set to alpha;
 *          = MorseLower: STRICT Lower part of A is set to alpha;
 *          = MorseUpperLower: ALL elements of A are set to alpha.
 *          Not LAPACK Compliant.
 *
 * @param[in] M
 *          The number of rows of the matrix A.  M >= 0.
 *
 * @param[in] N
 *         The number of columns of the matrix A.  N >= 0.
 *
 * @param[in] alpha
 *         The constant to which the elements are to be set.
 *
 * @param[in,out] A
 *         On entry, the M-by-N tile A.
 *         On exit, A has been set to alpha accordingly.
 *
 * @param[in] LDA
 *         The leading dimension of the array A.  LDA >= max(1,M).
 *
 **/
static int
CORE_zlaset2_parsec(dague_execution_unit_t *context, dague_execution_context_t * this_task)
{
    MORSE_enum *uplo;
    int *M;
    int *N;
    MORSE_Complex64_t *alpha;
    dague_data_copy_t *gA;
    int *LDA;

    dague_dtd_unpack_args(this_task,
                          UNPACK_VALUE, &uplo,
                          UNPACK_VALUE, &M,
                          UNPACK_VALUE, &N,
                          UNPACK_VALUE, &alpha,
                          UNPACK_DATA,  &gA,
                          UNPACK_VALUE, &LDA
                        );

    void *A = DAGUE_DATA_COPY_GET_PTR((dague_data_copy_t *)gA);
    CORE_zlaset2(*uplo, *M, *N, *alpha, A, *LDA);

    return 0;
}

void MORSE_TASK_zlaset2(MORSE_option_t *options,
                       MORSE_enum uplo, int M, int N,
                       MORSE_Complex64_t alpha, MORSE_desc_t *A, int Am, int An, int LDA)
{
    dague_dtd_handle_t* DAGUE_dtd_handle = (dague_dtd_handle_t *)(options->sequence->schedopt);

    insert_task_generic_fptr(DAGUE_dtd_handle,      CORE_zlaset2_parsec,   "laset2",
                             sizeof(MORSE_enum),                &uplo,      VALUE,
                             sizeof(int),                       &M,         VALUE,
                             sizeof(int),                       &N,         VALUE,
                             sizeof(MORSE_enum),                &alpha,     VALUE,
                             PASSED_BY_REF,         RTBLKADDR( A, MORSE_Complex64_t, Am, An ),     OUTPUT | REGION_FULL,
                             sizeof(int),                       &LDA,       VALUE,
                             0);
}