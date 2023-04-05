/**
 *
 * @file zgetrf.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2023 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon zgetrf wrappers
 *
 * @version 1.2.0
 * @author Omar Zenati
 * @author Mathieu Faverge
 * @author Emmanuel Agullo
 * @author Cedric Castagnede
 * @author Florent Pruvost
 * @author Matthieu Kuhn
 * @date 2023-02-21
 *
 * @precisions normal z -> s d c
 *
 */
#include "control/common.h"

/**
 ********************************************************************************
 *
 * @ingroup CHAMELEON_Complex64_t
 *
 *  CHAMELEON_zgetrf_WS_Alloc - Allocate the required workspaces for
 *  asynchronous getrf
 *
 *******************************************************************************
 *
 * @param[in] A
 *          The descriptor of the matrix A.
 *
 *******************************************************************************
 *
 * @retval An allocated opaque pointer to use in CHAMELEON_zgetrf_Tile_Async()
 *         and to free with CHAMELEON_zgetrf_WS_Free().
 *
 *******************************************************************************
 *
 * @sa CHAMELEON_zgetrf_Tile_Async
 * @sa CHAMELEON_zgetrf_WS_Free
 *
 */
void *
CHAMELEON_zgetrf_WS_Alloc( const CHAM_desc_t *A )
{
    CHAM_context_t           *chamctxt;
    struct chameleon_pzgetrf_s *options;

    chamctxt = chameleon_context_self();
    if ( chamctxt == NULL ) {
        return NULL;
    }

    options = calloc( 1, sizeof( struct chameleon_pzgetrf_s ) );
    options->ib = CHAMELEON_IB;

#if defined(GETRF_NOPIV_PER_COLUMN)
    chameleon_desc_init( &(options->U), CHAMELEON_MAT_ALLOC_TILE,
                         ChamComplexDouble, 1, A->nb, A->nb,
                         A->mt, A->nt * A->nb, 0, 0,
                         A->mt, A->nt * A->nb, A->p, A->q,
                         NULL, NULL, A->get_rankof_init );
#endif

    return options;
}

/**
 ********************************************************************************
 *
 * @ingroup CHAMELEON_Complex64_t
 *
 * @brief Free the allocated workspaces for asynchronous getrf
 *
 *******************************************************************************
 *
 * @param[in,out] user_ws
 *          On entry, the opaque pointer allocated by
 *          CHAMELEON_zgetrf_WS_Alloc() On exit, all data are freed.
 *
 *******************************************************************************
 *
 * @sa CHAMELEON_zgetrf_Tile_Async
 * @sa CHAMELEON_zgetrf_WS_Alloc
 *
 */
void
CHAMELEON_zgetrf_WS_Free( const CHAM_desc_t *A, void *user_ws )
{
    struct chameleon_pzgetrf_s *ws = (struct chameleon_pzgetrf_s *)user_ws;

#if defined(GETRF_NOPIV_PER_COLUMN)
    chameleon_desc_destroy( &(ws->U) );
#endif

    free( ws );
}

#if defined(NOT_AVAILABLE_YET)
/**
 ********************************************************************************
 *
 * @ingroup CHAMELEON_Complex64_t
 *
 *  CHAMELEON_zgetrf - Computes an LU factorization of a general M-by-N matrix A
 *  using the tile LU algorithm without row pivoting.
 *  WARNING: Don't use this function if you are not sure your matrix is diagonal
 *  dominant.
 *
 *******************************************************************************
 *
 * @param[in] M
 *          The number of rows of the matrix A. M >= 0.
 *
 * @param[in] N
 *          The number of columns of the matrix A. N >= 0.
 *
 * @param[in,out] A
 *          On entry, the M-by-N matrix to be factored.
 *          On exit, the tile factors L and U from the factorization.
 *
 * @param[in] LDA
 *          The leading dimension of the array A. LDA >= max(1,M).
 *
 *******************************************************************************
 *
 * @retval CHAMELEON_SUCCESS successful exit
 * @retval <0 if -i, the i-th argument had an illegal value
 * @retval >0 if i, U(i,i) is exactly zero. The factorization has been
 *               completed, but the factor U is exactly singular, and division
 *               by zero will occur if it is used to solve a system of
 *               equations.
 *
 *******************************************************************************
 *
 * @sa CHAMELEON_zgetrf_Tile
 * @sa CHAMELEON_zgetrf_Tile_Async
 * @sa CHAMELEON_cgetrf
 * @sa CHAMELEON_dgetrf
 * @sa CHAMELEON_sgetrf
 *
 */
int
CHAMELEON_zgetrf( int M, int N, CHAMELEON_Complex64_t *A, int *IPIV, int LDA )
{
    int                 NB;
    int                 status;
    CHAM_desc_t         descAl, descAt;
    CHAM_context_t     *chamctxt;
    RUNTIME_sequence_t *sequence = NULL;
    RUNTIME_request_t   request  = RUNTIME_REQUEST_INITIALIZER;
    void               *ws;

    chamctxt = chameleon_context_self();
    if ( chamctxt == NULL ) {
        chameleon_fatal_error( "CHAMELEON_zgetrf", "CHAMELEON not initialized" );
        return CHAMELEON_ERR_NOT_INITIALIZED;
    }
    /* Check input arguments */
    if ( M < 0 ) {
        chameleon_error( "CHAMELEON_zgetrf", "illegal value of M" );
        return -1;
    }
    if ( N < 0 ) {
        chameleon_error( "CHAMELEON_zgetrf", "illegal value of N" );
        return -2;
    }
    if ( LDA < chameleon_max( 1, M ) ) {
        chameleon_error( "CHAMELEON_zgetrf", "illegal value of LDA" );
        return -4;
    }
    /* Quick return */
    if ( chameleon_min( M, N ) == 0 )
        return CHAMELEON_SUCCESS;

    /* Tune NB & IB depending on M, N & NRHS; Set NBNBSIZE */
    status = chameleon_tune( CHAMELEON_FUNC_ZGESV, M, N, 0 );
    if ( status != CHAMELEON_SUCCESS ) {
        chameleon_error( "CHAMELEON_zgetrf", "chameleon_tune() failed" );
        return status;
    }

    /* Set NT & NTRHS */
    NB = CHAMELEON_NB;

    chameleon_sequence_create( chamctxt, &sequence );

    /* Submit the matrix conversion */
    chameleon_zlap2tile( chamctxt, &descAl, &descAt, ChamDescInout, ChamUpperLower,
                         A, NB, NB, LDA, N, M, N, sequence, &request );

    /* Allocate workspace for partial pivoting */
    ws = CHAMELEON_zgetrf_WS_Alloc( &descAt );
    /* Call the tile interface */
    CHAMELEON_zgetrf_Tile_Async( &descAt, ws, sequence, &request );

    /* Submit the matrix conversion back */
    chameleon_ztile2lap( chamctxt, &descAl, &descAt,
                         ChamDescInout, ChamUpperLower, sequence, &request );

    chameleon_sequence_wait( chamctxt, sequence );

    /* Cleanup the temporary data */
    CHAMELEON_zgetrf_WS_Free( &descAt, ws );
    chameleon_ztile2lap_cleanup( chamctxt, &descAl, &descAt );

    status = sequence->status;
    chameleon_sequence_destroy( chamctxt, sequence );

    return status;
}
#endif

/**
 ********************************************************************************
 *
 * @ingroup CHAMELEON_Complex64_t_Tile
 *
 *  CHAMELEON_zgetrf_Tile - Computes the tile LU factorization of a matrix.
 *  Tile equivalent of CHAMELEON_zgetrf().  Operates on matrices stored by
 *  tiles.  All matrices are passed through descriptors.  All dimensions are
 *  taken from the descriptors.
 *
 *******************************************************************************
 *
 * @param[in,out] A
 *          On entry, the M-by-N matrix to be factored.
 *          On exit, the tile factors L and U from the factorization.
 *
 *******************************************************************************
 *
 * @retval CHAMELEON_SUCCESS successful exit
 * @retval >0 if i, U(i,i) is exactly zero. The factorization has been completed,
 *               but the factor U is exactly singular, and division by zero will occur
 *               if it is used to solve a system of equations.
 *
 *******************************************************************************
 *
 * @sa CHAMELEON_zgetrf
 * @sa CHAMELEON_zgetrf_Tile_Async
 * @sa CHAMELEON_cgetrf_Tile
 * @sa CHAMELEON_dgetrf_Tile
 * @sa CHAMELEON_sgetrf_Tile
 * @sa CHAMELEON_zgetrs_Tile
 *
 */
int
CHAMELEON_zgetrf_Tile( CHAM_desc_t *A )
{
    CHAM_context_t     *chamctxt;
    RUNTIME_sequence_t *sequence = NULL;
    RUNTIME_request_t   request  = RUNTIME_REQUEST_INITIALIZER;
    int                 status;
    void               *ws;

    chamctxt = chameleon_context_self();
    if ( chamctxt == NULL ) {
        chameleon_fatal_error( "CHAMELEON_zgetrf_Tile", "CHAMELEON not initialized" );
        return CHAMELEON_ERR_NOT_INITIALIZED;
    }
    chameleon_sequence_create( chamctxt, &sequence );

    ws = CHAMELEON_zgetrf_WS_Alloc( A );
    CHAMELEON_zgetrf_Tile_Async( A, ws, sequence, &request );

    CHAMELEON_Desc_Flush( A, sequence );

    chameleon_sequence_wait( chamctxt, sequence );
    CHAMELEON_zgetrf_WS_Free( A, ws );

    status = sequence->status;
    chameleon_sequence_destroy( chamctxt, sequence );
    return status;
}

/**
 ********************************************************************************
 *
 * @ingroup CHAMELEON_Complex64_t_Tile_Async
 *
 *  CHAMELEON_zgetrf_Tile_Async - Computes the tile LU factorization of a
 *  matrix.  Non-blocking equivalent of CHAMELEON_zgetrf_Tile().  May return
 *  before the computation is finished.  Allows for pipelining of operations ar
 *  runtime.
 *
 *******************************************************************************
 *
 * @param[in,out] A
 *          On entry, the M-by-N matrix to be factored.
 *          On exit, the tile factors L and U from the factorization.
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
 * @sa CHAMELEON_zgetrf
 * @sa CHAMELEON_zgetrf_Tile
 * @sa CHAMELEON_cgetrf_Tile_Async
 * @sa CHAMELEON_dgetrf_Tile_Async
 * @sa CHAMELEON_sgetrf_Tile_Async
 * @sa CHAMELEON_zgetrs_Tile_Async
 *
 */
int
CHAMELEON_zgetrf_Tile_Async( CHAM_desc_t        *A,
                             void               *user_ws,
                             RUNTIME_sequence_t *sequence,
                             RUNTIME_request_t  *request )
{
    CHAM_context_t *chamctxt;
    chamctxt = chameleon_context_self();

    if ( chamctxt == NULL ) {
        chameleon_fatal_error( "CHAMELEON_zgetrf_Tile_Async", "CHAMELEON not initialized" );
        return CHAMELEON_ERR_NOT_INITIALIZED;
    }
    if ( chamctxt->scheduler != RUNTIME_SCHED_STARPU ) {
        chameleon_fatal_error( "CHAMELEON_zgetrf_Tile_Async", "CHAMELEON_zgetrf_Tile_Async is only available with StarPU" );
        return CHAMELEON_ERR_NOT_INITIALIZED;
    }
    if ( user_ws == NULL ) {
        chameleon_fatal_error( "CHAMELEON_zgetrf_Tile_Async", "NULL user_ws" );
        return CHAMELEON_ERR_NOT_INITIALIZED;
    }
    if ( sequence == NULL ) {
        chameleon_fatal_error( "CHAMELEON_zgetrf_Tile_Async", "NULL sequence" );
        return CHAMELEON_ERR_UNALLOCATED;
    }
    if ( request == NULL ) {
        chameleon_fatal_error( "CHAMELEON_zgetrf_Tile_Async", "NULL request" );
        return CHAMELEON_ERR_UNALLOCATED;
    }
    /* Check sequence status */
    if ( sequence->status == CHAMELEON_SUCCESS ) {
        request->status = CHAMELEON_SUCCESS;
    }
    else {
        return chameleon_request_fail( sequence, request, CHAMELEON_ERR_SEQUENCE_FLUSHED );
    }

    /* Check descriptors for correctness */
    if ( chameleon_desc_check( A ) != CHAMELEON_SUCCESS ) {
        chameleon_error( "CHAMELEON_zgetrf_Tile", "invalid first descriptor" );
        return chameleon_request_fail( sequence, request, CHAMELEON_ERR_ILLEGAL_VALUE );
    }

    /* Check input arguments */
    if ( A->nb != A->mb ) {
        chameleon_error( "CHAMELEON_zgetrf_Tile", "only square tiles supported" );
        return chameleon_request_fail( sequence, request, CHAMELEON_ERR_ILLEGAL_VALUE );
    }

    chameleon_pzgetrf( user_ws, A, sequence, request );

    return CHAMELEON_SUCCESS;
}