/**
 *
 * @copyright (c) 2009-2016 The University of Tennessee and The University
 *                          of Tennessee Research Foundation.
 *                          All rights reserved.
 * @copyright (c) 2012-2016 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                          Univ. Bordeaux. All rights reserved.
 *
 **/

/**
 *
 * @file ztpgqrt.c
 *
 *  MORSE computational routines
 *  MORSE is a software package provided by Univ. of Tennessee,
 *  Univ. of California Berkeley and Univ. of Colorado Denver
 *
 * @version 0.9.0
 * @author Mathieu Faverge
 * @date 2016-12-15
 * @precisions normal z -> s d c
 *
 **/
#include "control/common.h"

/**
 ******************************************************************************
 *
 * @ingroup MORSE_Complex64_t
 *
 *  MORSE_ztpgqrt - Generates a partial Q matrix formed with a blocked QR
 *  factorization of a "triangular-pentagonal" matrix C, which is composed of an
 *  unused triangular block and a pentagonal block V, using the compact
 *  representation for Q. See MORSE_ztpqrt() to generate V.
 *
 *******************************************************************************
 *
 * @param[in] M
 *          The number of rows of the matrices Q2, and V2. M >= 0.
 *
 * @param[in] N
 *          The number of columns of the matrices Q1, and Q1. N >= 0.
 *
 * @param[in] K
 *          The number of elementary reflectors whose product defines
 *          the matrix Q in the matrix V.
 *          The number of rows of the matrices V1, and Q1.
 *          The number of columns of the matrices V1, and V2.
 *
 * @param[in] L
 *          The number of rows of the upper trapezoidal part of V2.
 *          MIN(M,N) >= L >= 0.  See Further Details.
 *
 * @param[in] V1
 *          The i-th row must contain the vector which defines the
 *          elementary reflector H(i), for i = 1,2,...,k, as returned by
 *          MORSE_ztpqrt().
 *          V1 is a matrix of size K-by-K.
 *
 * @param[in] LDV1
 *          The leading dimension of the array V1. LDV1 >= max(1,K).
 *
 * @param[int] descT1
 *          The auxiliary factorization data generated by the call to
 *          MORSE_zgeqrf() on V1.
 *
 * @param[in] V2
 *          The i-th row must contain the vector which defines the
 *          elementary reflector H(i), for i = 1,2,...,k, as returned by
 *          MORSE_ztpqrt() in the first k rows of its array argument V2.
 *          V2 is a matrix of size M-by-K. The first M-L rows
 *          are rectangular, and the last L rows are upper trapezoidal.
 *
 * @param[in] LDV2
 *          The leading dimension of the array V2. LDV2 >= max(1,M).
 *
 * @param[int] descT2
 *          The auxiliary factorization data, generated by the call to
 *          MORSE_ztpqrt() on V1 and V2, and associated to V2.
 *
 * @param[in,out] Q1
 *          Q1 is COMPLEX*16 array, dimension (LDQ1,N)
 *          On entry, the K-by-N matrix Q1.
 *          On exit, Q1 is overwritten by the corresponding block of
 *          Q*Q1.  See Further Details.
 *
 * @param[in] LDQ1
 *          The leading dimension of the array Q1. LDQ1 >= max(1,K).
 *
 * @param[in,out] Q2
 *          On entry, the pentagonal M-by-N matrix Q2.
 *          On exit, Q2 contains Q.
 *
 * @param[in] LDQ2
 *          The leading dimension of the array Q2.  LDQ2 >= max(1,M).
 *
 * @par Further Details:
 * =====================
 *
 *  The input matrix Q is a (K+M)-by-N matrix
 *
 *               Q = [ Q1 ]
 *                   [ Q2 ]
 *
 *  where Q1 is an identity matrix, and Q2 is a M-by-N matrix of 0.
 *  V is a matrix of householder reflectors with a pentagonal shape consisting
 *  of a K-by-K rectangular matrix V1 on top of a matrix V2 composed of
 *  (M-L)-by-K rectangular part on top of a L-by-N upper trapezoidal matrix:
 *
 *               V = [ V1  ]  <-     K-by-K rectangular
 *                   [ V2a ]  <- (M-L)-by-K rectangular
 *                   [ V2b ]  <-     L-by-K upper trapezoidal.
 *
 *  The upper trapezoidal part of the matrix V2 consists of the first L rows of
 *  a K-by-K upper triangular matrix, where 0 <= L <= MIN(M,K).  If L=0, V2 is
 *  rectangular M-by-K; if M=L=K, V2 is upper triangular. Those are the two
 *  cases only handled for now.
 *
 *******************************************************************************
 *
 * @return
 *          \retval MORSE_SUCCESS successful exit
 *          \retval <0 if -i, the i-th argument had an illegal value
 *
 *******************************************************************************
 *
 * @sa MORSE_ztpgqrt_Tile
 * @sa MORSE_ztpgqrt_Tile_Async
 * @sa MORSE_ctpgqrt
 * @sa MORSE_dtpgqrt
 * @sa MORSE_stpgqrt
 * @sa MORSE_zgeqrs
 *
 ******************************************************************************/
int MORSE_ztpgqrt( int M, int N, int K, int L,
                   MORSE_Complex64_t *V1, int LDV1, MORSE_desc_t *descT1,
                   MORSE_Complex64_t *V2, int LDV2, MORSE_desc_t *descT2,
                   MORSE_Complex64_t *Q1, int LDQ1,
                   MORSE_Complex64_t *Q2, int LDQ2 )
{
    int NB;
    int status;
    MORSE_context_t *morse;
    MORSE_sequence_t *sequence = NULL;
    MORSE_request_t request = MORSE_REQUEST_INITIALIZER;
    MORSE_desc_t descQ1, descQ2, descV1, descV2;
    int minMK = chameleon_min( M, K );

    morse = morse_context_self();
    if (morse == NULL) {
        morse_fatal_error("MORSE_ztpgqrt", "MORSE not initialized");
        return MORSE_ERR_NOT_INITIALIZED;
    }

    /* Check input arguments */
    if (M < 0) {
        morse_error("MORSE_ztpgqrt", "illegal value of M");
        return -1;
    }
    if (N < 0) {
        morse_error("MORSE_ztpgqrt", "illegal value of N");
        return -2;
    }
    if (K < 0) {
        morse_error("MORSE_ztpgqrt", "illegal value of K");
        return -3;
    }
    if ((L < 0) || ((L > minMK) && (minMK > 0))) {
        morse_error("MORSE_ztpgqrt", "illegal value of N");
        return -4;
    }
    if (K != N) {
        morse_error("MORSE_ztpgqrt", "illegal value of K and N. K must be equal to N");
        return -3;
    }
    if (LDV1 < chameleon_max(1, K)) {
        morse_error("MORSE_ztpgqrt", "illegal value of LDV1");
        return -6;
    }
    if (LDV2 < chameleon_max(1, M)) {
        morse_error("MORSE_ztpgqrt", "illegal value of LDV2");
        return -9;
    }
    if (LDQ1 < chameleon_max(1, K)) {
        morse_error("MORSE_ztpgqrt", "illegal value of LDQ1");
        return -11;
    }
    if (LDQ2 < chameleon_max(1, M)) {
        morse_error("MORSE_ztpgqrt", "illegal value of LDQ2");
        return -13;
    }

    /* Quick return */
    if (minMK == 0)
        return MORSE_SUCCESS;

    /* Tune NB & IB depending on M, N & NRHS; Set NBNBSIZE */
    status = morse_tune(MORSE_FUNC_ZGELS, M, K, 0);
    if (status != MORSE_SUCCESS) {
        morse_error("MORSE_ztpgqrt", "morse_tune() failed");
        return status;
    }

    /* Set NT */
    NB = MORSE_NB;

    morse_sequence_create(morse, &sequence);

    /* Submit the matrix conversion */
    morse_zlap2tile( morse, &descV1l, &descV1t, MorseUpperLower,
                     V1, NB, NB, LDV1, K, M, K, sequence, &request );
    morse_zlap2tile( morse, &descV2l, &descV2t, MorseUpperLower,
                     V2, NB, NB, LDV2, K, M, K, sequence, &request );
    morse_zlap2tile( morse, &descQ1l, &descQ1t, MorseUpperLower,
                     Q1, NB, NB, LDQ1, N, K, N, sequence, &request );
    morse_zlap2tile( morse, &descQ2l, &descQ2t, MorseUpperLower,
                     Q2, NB, NB, LDQ2, N, M, N, sequence, &request );

    /* Call the tile interface */
    MORSE_ztpgqrt_Tile_Async(L, &descV1, descT1, &descV2, descT2, &descQ1, &descQ2, sequence, &request);

    /* Submit the matrix conversion back */
    morse_ztile2lap( morse, &descQ1l, &descQ1t,
                     MorseUpperLower, sequence, &request );
    morse_ztile2lap( morse, &descQ2l, &descQ2t,
                     MorseUpperLower, sequence, &request );

    morse_sequence_wait(morse, sequence);

    /* Cleanup the temporary data */
    morse_ztile2lap_cleanup( morse, &descV1l, &descV1t );
    morse_ztile2lap_cleanup( morse, &descV2l, &descV2t );
    morse_ztile2lap_cleanup( morse, &descQ1l, &descQ1t );
    morse_ztile2lap_cleanup( morse, &descQ2l, &descQ2t );

    status = sequence->status;
    morse_sequence_destroy(morse, sequence);
    return status;
}

/**
 *******************************************************************************
 *
 * @ingroup MORSE_Complex64_t_Tile
 *
 *  MORSE_ztpgqrt_Tile - Generates a partial Q matrix formed with a blocked QR
 *  factorization of a "triangular-pentagonal" matrix C, which is composed of an
 *  unused triangular block and a pentagonal block V, using the compact
 *  representation for Q. See MORSE_ztpqrt() to generate V.
 *
 *******************************************************************************
 *
 *
 *******************************************************************************
 *
 * @return
 *          \retval MORSE_SUCCESS successful exit
 *
 *******************************************************************************
 *
 * @sa MORSE_ztpgqrt
 * @sa MORSE_ztpgqrt_Tile_Async
 * @sa MORSE_ctpgqrt_Tile
 * @sa MORSE_dtpgqrt_Tile
 * @sa MORSE_stpgqrt_Tile
 * @sa MORSE_zgeqrs_Tile
 *
 ******************************************************************************/
int MORSE_ztpgqrt_Tile( int L,
                        MORSE_desc_t *V1, MORSE_desc_t *T1,
                        MORSE_desc_t *V2, MORSE_desc_t *T2,
                        MORSE_desc_t *Q1, MORSE_desc_t *Q2 )
{
    MORSE_context_t *morse;
    MORSE_sequence_t *sequence = NULL;
    MORSE_request_t request = MORSE_REQUEST_INITIALIZER;
    int status;

    morse = morse_context_self();
    if (morse == NULL) {
        morse_fatal_error("MORSE_ztpgqrt_Tile", "MORSE not initialized");
        return MORSE_ERR_NOT_INITIALIZED;
    }
    morse_sequence_create(morse, &sequence);
    MORSE_ztpgqrt_Tile_Async(L, V1, T1, V2, T2, Q1, Q2, sequence, &request);
    RUNTIME_desc_flush( Q1, sequence );
    RUNTIME_desc_flush( Q2, sequence );

    morse_sequence_wait(morse, sequence);

    status = sequence->status;
    morse_sequence_destroy(morse, sequence);
    return status;
}

/**
 *******************************************************************************
 *
 * @ingroup MORSE_Complex64_t_Tile_Async
 *
 *  MORSE_ztpgqrt_Tile_Async - Generates a partial Q matrix formed with a blocked QR
 *  factorization of a "triangular-pentagonal" matrix C, which is composed of an
 *  unused triangular block and a pentagonal block V, using the compact
 *  representation for Q. See MORSE_ztpqrt() to generate V.
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
 * @sa MORSE_ztpgqrt
 * @sa MORSE_ztpgqrt_Tile
 * @sa MORSE_ctpgqrt_Tile_Async
 * @sa MORSE_dtpgqrt_Tile_Async
 * @sa MORSE_stpgqrt_Tile_Async
 * @sa MORSE_zgeqrs_Tile_Async
 *
 ******************************************************************************/
int MORSE_ztpgqrt_Tile_Async( int L,
                              MORSE_desc_t *V1, MORSE_desc_t *T1,
                              MORSE_desc_t *V2, MORSE_desc_t *T2,
                              MORSE_desc_t *Q1, MORSE_desc_t *Q2,
                              MORSE_sequence_t *sequence, MORSE_request_t *request )
{
    MORSE_context_t *morse;
    MORSE_desc_t D, *Dptr = NULL;

    morse = morse_context_self();
    if (morse == NULL) {
        morse_error("MORSE_ztpgqrt_Tile", "MORSE not initialized");
        return MORSE_ERR_NOT_INITIALIZED;
    }
    if (sequence == NULL) {
        morse_fatal_error("MORSE_ztpgqrt_Tile", "NULL sequence");
        return MORSE_ERR_UNALLOCATED;
    }
    if (request == NULL) {
        morse_fatal_error("MORSE_ztpgqrt_Tile", "NULL request");
        return MORSE_ERR_UNALLOCATED;
    }
    /* Check sequence status */
    if (sequence->status == MORSE_SUCCESS)
        request->status = MORSE_SUCCESS;
    else
        return morse_request_fail(sequence, request, MORSE_ERR_SEQUENCE_FLUSHED);

    /* Check descriptors for correctness */
    if (morse_desc_check(V1) != MORSE_SUCCESS) {
        morse_error("MORSE_ztpgqrt_Tile", "invalid V1 descriptor");
        return morse_request_fail(sequence, request, MORSE_ERR_ILLEGAL_VALUE);
    }
    if (morse_desc_check(T1) != MORSE_SUCCESS) {
        morse_error("MORSE_ztpgqrt_Tile", "invalid T1 descriptor");
        return morse_request_fail(sequence, request, MORSE_ERR_ILLEGAL_VALUE);
    }
    if (morse_desc_check(V2) != MORSE_SUCCESS) {
        morse_error("MORSE_ztpgqrt_Tile", "invalid V2 descriptor");
        return morse_request_fail(sequence, request, MORSE_ERR_ILLEGAL_VALUE);
    }
    if (morse_desc_check(T2) != MORSE_SUCCESS) {
        morse_error("MORSE_ztpgqrt_Tile", "invalid T2 descriptor");
        return morse_request_fail(sequence, request, MORSE_ERR_ILLEGAL_VALUE);
    }
    if (morse_desc_check(Q1) != MORSE_SUCCESS) {
        morse_error("MORSE_ztpgqrt_Tile", "invalid Q1 descriptor");
        return morse_request_fail(sequence, request, MORSE_ERR_ILLEGAL_VALUE);
    }
    if (morse_desc_check(Q2) != MORSE_SUCCESS) {
        morse_error("MORSE_ztpgqrt_Tile", "invalid Q2 descriptor");
        return morse_request_fail(sequence, request, MORSE_ERR_ILLEGAL_VALUE);
    }
    /* Check input arguments */
    if (Q1->nb != Q1->mb) {
        morse_error("MORSE_ztpgqrt_Tile", "only square tiles supported");
        return morse_request_fail(sequence, request, MORSE_ERR_ILLEGAL_VALUE);
    }
    if ( (L != 0) && (((Q2->m - L) % Q2->mb) != 0) ) {
        morse_error("MORSE_ztpgqrt_Tile", "Triangular part must be aligned with tiles");
        return morse_request_fail(sequence, request, MORSE_ERR_ILLEGAL_VALUE);
    }
#if defined(CHAMELEON_COPY_DIAG)
    {
        int minMT;
        if (V1->m > V1->n) {
            minMT = V1->nt;
        } else {
            minMT = V1->mt;
        }
        morse_zdesc_alloc_diag(D, V1->mb, V1->nb, minMT*V1->mb, V1->nb, 0, 0, minMT*V1->mb, V1->nb, V1->p, V1->q);
        Dptr = &D;
    }
#endif

    /* if (morse->householder == MORSE_FLAT_HOUSEHOLDER) { */
    morse_pzlaset( MorseUpperLower, 0., 1., Q1, sequence, request );
    morse_pzlaset( MorseUpperLower, 0., 0., Q2, sequence, request );
    morse_pztpgqrt( L, V1, T1, V2, T2, Q1, Q2, Dptr, sequence, request );
    /* } */
    /* else { */
    /*    morse_pztpgqrtrh(Q1, T, MORSE_RHBLK, sequence, request); */
    /* } */
    if (Dptr != NULL) {
        morse_desc_mat_free( Dptr );
    }
    (void)D;
    return MORSE_SUCCESS;
}
