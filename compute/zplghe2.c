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
 * @file zplghe2.c
 *
 *  MORSE computational routines
 *  MORSE is a software package provided by Univ. of Tennessee,
 *  Univ. of California Berkeley and Univ. of Colorado Denver
 *
 * @version 2.5.0
 * @comment This file is a copy of zplghe.c
 *          wich has been automatically generated
 *          from Plasma 2.5.0 for MORSE 1.0.0
 * @author Mathieu Faverge
 * @author Emmanuel Agullo
 * @author Cedric Castagnede
 * @author Rade Mathis
 * @date 2016-08-01
 * @precisions normal z -> c
 *
 **/
#include "control/common.h"

/***************************************************************************//**
 *
 * @ingroup MORSE_Complex64_t
 *
 *  MORSE_zplghe2 - One 'half' of a random hermitian matrix by tiles.
 *
 *******************************************************************************
 *
 * @param[in] bump
 *          The value to add to the diagonal to be sure
 *          to have a positive definite matrix.
 *
 * @param[in] N
 *          The order of the matrix A. N >= 0.
 *
 * @param[out] A
 *          On exit, The random hermitian matrix A generated.
 *
 * @param[in] LDA
 *          The leading dimension of the array A. LDA >= max(1,M).
 *
 * @param[in] seed
 *          The seed used in the random generation.
 *
 * @param[in] uplo
 *          The part of the Matrix wich will be generated.
 *
 *******************************************************************************
 *
 * @return
 *          \retval MORSE_SUCCESS successful exit
 *          \retval <0 if -i, the i-th argument had an illegal value
 *
 *******************************************************************************
 *
 * @sa MORSE_zplghe2_Tile
 * @sa MORSE_zplghe2_Tile_Async
 * @sa MORSE_cplghe2
 * @sa MORSE_dplghe2
 * @sa MORSE_splghe2
 * @sa MORSE_zplgsy2
 *
 ******************************************************************************/
int MORSE_zplghe2( double bump, int N,
                   MORSE_Complex64_t *A, int LDA,
                   unsigned long long int seed, MORSE_enum uplo )
{
    int NB;
    int status;
    MORSE_context_t *morse;
    MORSE_sequence_t *sequence = NULL;
    MORSE_request_t request = MORSE_REQUEST_INITIALIZER;
    MORSE_desc_t descA;

    morse = morse_context_self();
    if (morse == NULL) {
        morse_fatal_error("MORSE_zplghe2", "MORSE not initialized");
        return MORSE_ERR_NOT_INITIALIZED;
    }
    /* Check input arguments */
    if (N < 0) {
        morse_error("MORSE_zplghe2", "illegal value of N");
        return -2;
    }
    if (LDA < max(1, N)) {
        morse_error("MORSE_zplghe2", "illegal value of LDA");
        return -4;
    }
    /* Quick return */
    if (max(0, N) == 0)
        return MORSE_SUCCESS;

    /* Tune NB depending on M, N & NRHS; Set NBNB */
    status = morse_tune(MORSE_FUNC_ZGEMM, N, N, 0);
    if (status != MORSE_SUCCESS) {
        morse_error("MORSE_zplghe2", "morse_tune() failed");
        return status;
    }

    /* Set NT */
    NB = MORSE_NB;
    morse_sequence_create(morse, &sequence);

    morse_zdesc_alloc(descA, NB, NB, LDA, N, 0, 0, N, N, morse_desc_mat_free(&descA));

    /* Call the tile interface */
    MORSE_zplghe2_Tile_Async( bump, &descA, seed, uplo, sequence, &request );

    morse_zooptile2lap(descA, A, NB, NB, LDA, N,  sequence, &request);
    morse_sequence_wait(morse, sequence);
    morse_desc_mat_free(&descA);

    status = sequence->status;
    morse_sequence_destroy(morse, sequence);

    return status;
}

/***************************************************************************//**
 *
 * @ingroup MORSE_Complex64_t_Tile
 *
 *  MORSE_zplghe2_Tile - Generate 'half' of a random hermitian matrix by tiles.
 *  Tile equivalent of MORSE_zplghe2().
 *  Operates on matrices stored by tiles.
 *  All matrices are passed through descriptors.
 *  All dimensions are taken from the descriptors.
 *
 *******************************************************************************
 *
 * @param[in] bump
 *          The value to add to the diagonal to be sure
 *          to have a positive definite matrix.
 *
 * @param[in] A
 *          On exit, The random hermitian matrix A generated.
 *
 * @param[in] seed
 *          The seed used in the random generation.
 *
 * @param[in] uplo
 *          The part of the Matrix wich will be generated.
 *
 *******************************************************************************
 *
 * @return
 *          \retval MORSE_SUCCESS successful exit
 *
 *******************************************************************************
 *
 * @sa MORSE_zplghe2
 * @sa MORSE_zplghe2_Tile_Async
 * @sa MORSE_cplghe2_Tile
 * @sa MORSE_dplghe2_Tile
 * @sa MORSE_splghe2_Tile
 * @sa MORSE_zplgsy2_Tile
 *
 ******************************************************************************/
int MORSE_zplghe2_Tile( double bump, MORSE_desc_t *A,
                       unsigned long long int seed, MORSE_enum uplo )
{
    MORSE_context_t *morse;
    MORSE_sequence_t *sequence = NULL;
    MORSE_request_t request = MORSE_REQUEST_INITIALIZER;
    int status;

    morse = morse_context_self();
    if (morse == NULL) {
        morse_fatal_error("MORSE_zplghe2_Tile", "MORSE not initialized");
        return MORSE_ERR_NOT_INITIALIZED;
    }
    morse_sequence_create(morse, &sequence);
    MORSE_zplghe2_Tile_Async( bump, A, seed, uplo, sequence, &request );
    morse_sequence_wait(morse, sequence);
    status = sequence->status;
    morse_sequence_destroy(morse, sequence);
    return status;
}

/***************************************************************************//**
 *
 * @ingroup MORSE_Complex64_t_Tile_Async
 *
 *  MORSE_zplghe2_Tile_Async - Generate a half random hermitian matrix by tiles.
 *  Non-blocking equivalent of MORSE_zplghe2_Tile().
 *  May return before the computation is finished.
 *  Allows for pipelining of operations at runtime.
 *
 *******************************************************************************
 *
 * @param[in] sequence
 *          Identifies the sequence of function calls that this call belongs to
 *          (for completion checks and exception handling purposes).
 *
 * @param[out] request
 *          Identifies this function call (for exception handling purposes).
 *
 *******************************************************************************
 *
 * @sa MORSE_zplghe2
 * @sa MORSE_zplghe2_Tile
 * @sa MORSE_cplghe2_Tile_Async
 * @sa MORSE_dplghe2_Tile_Async
 * @sa MORSE_splghe2_Tile_Async
 * @sa MORSE_zplghe2_Tile_Async
 * @sa MORSE_zplgsy2_Tile_Async
 *
 ******************************************************************************/
int MORSE_zplghe2_Tile_Async( double                 bump,
                              MORSE_desc_t             *A,
                              unsigned long long int seed,
                              MORSE_enum             uplo,
                              MORSE_sequence_t  *sequence,
                              MORSE_request_t    *request )
{
    MORSE_context_t *morse;

    morse = morse_context_self();
    if (morse == NULL) {
        morse_fatal_error("MORSE_zplghe2_Tile", "MORSE not initialized");
        return MORSE_ERR_NOT_INITIALIZED;
    }
    if (sequence == NULL) {
        morse_fatal_error("MORSE_zplghe2_Tile", "NULL sequence");
        return MORSE_ERR_UNALLOCATED;
    }
    if (request == NULL) {
        morse_fatal_error("MORSE_zplghe2_Tile", "NULL request");
        return MORSE_ERR_UNALLOCATED;
    }
    /* Check sequence status */
    if (sequence->status == MORSE_SUCCESS)
        request->status = MORSE_SUCCESS;
    else
        return morse_request_fail(sequence, request, MORSE_ERR_SEQUENCE_FLUSHED);

    /* Check descriptors for correctness */
    if (morse_desc_check(A) != MORSE_SUCCESS) {
        morse_error("MORSE_zplghe2_Tile", "invalid descriptor");
        return morse_request_fail(sequence, request, MORSE_ERR_ILLEGAL_VALUE);
    }
    /* Check input arguments */
    if (A->nb != A->mb) {
        morse_error("MORSE_zplghe2_Tile", "only square tiles supported");
        return morse_request_fail(sequence, request, MORSE_ERR_ILLEGAL_VALUE);
    }

    /* Quick return */
    if (min( A->m, A->n ) == 0)
        return MORSE_SUCCESS;


    morse_pzplghe2(bump,      A, seed, uplo, sequence,  request);

    return MORSE_SUCCESS;
}
