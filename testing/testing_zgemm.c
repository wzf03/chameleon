/**
 *
 * @file testing_zgemm.c
 *
 * @copyright 2019-2021 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon zgemm testing
 *
 * @version 1.1.0
 * @author Lucas Barros de Assis
 * @author Florent Pruvost
 * @author Mathieu Faverge
 * @date 2020-11-19
 * @precisions normal z -> c d s
 *
 */
#include <chameleon.h>
#include "testings.h"
#include "testing_zcheck.h"
#include <chameleon/flops.h>

int
testing_zgemm_desc( run_arg_list_t *args, int check )
{
    testdata_t test_data = { .args = args };
    int        hres      = 0;

    /* Read arguments */
    int          async  = parameters_getvalue_int( "async" );
    intptr_t     mtxfmt = parameters_getvalue_int( "mtxfmt" );
    int          nb     = run_arg_get_int( args, "nb", 320 );
    int          P      = parameters_getvalue_int( "P" );
    cham_trans_t transA = run_arg_get_trans( args, "transA", ChamNoTrans );
    cham_trans_t transB = run_arg_get_trans( args, "transB", ChamNoTrans );
    int          N      = run_arg_get_int( args, "N", 1000 );
    int          M      = run_arg_get_int( args, "M", N );
    int          K      = run_arg_get_int( args, "K", N );
    int          LDA    = run_arg_get_int( args, "LDA", ( ( transA == ChamNoTrans ) ? M : K ) );
    int          LDB    = run_arg_get_int( args, "LDB", ( ( transB == ChamNoTrans ) ? K : N ) );
    int          LDC    = run_arg_get_int( args, "LDC", M );
    CHAMELEON_Complex64_t alpha = testing_zalea();
    CHAMELEON_Complex64_t beta  = testing_zalea();
    int                   seedA = run_arg_get_int( args, "seedA", random() );
    int                   seedB = run_arg_get_int( args, "seedB", random() );
    int                   seedC = run_arg_get_int( args, "seedC", random() );
    int                   Q     = parameters_compute_q( P );

    /* Descriptors */
    int          Am, An, Bm, Bn;
    CHAM_desc_t *descA, *descB, *descC, *descCinit;
    void        *ws = NULL;

    alpha = run_arg_get_complex64( args, "alpha", alpha );
    beta  = run_arg_get_complex64( args, "beta", beta );

    CHAMELEON_Set( CHAMELEON_TILE_SIZE, nb );

    /* Calculate the dimensions according to the transposition */
    if ( transA == ChamNoTrans ) {
        Am = M;
        An = K;
    }
    else {
        Am = K;
        An = M;
    }
    if ( transB == ChamNoTrans ) {
        Bm = K;
        Bn = N;
    }
    else {
        Bm = N;
        Bn = K;
    }

    /* Create the matrices */
    CHAMELEON_Desc_Create(
        &descA, (void*)(-mtxfmt), ChamComplexDouble, nb, nb, nb * nb, LDA, An, 0, 0, Am, An, P, Q );
    CHAMELEON_Desc_Create(
        &descB, (void*)(-mtxfmt), ChamComplexDouble, nb, nb, nb * nb, LDB, Bn, 0, 0, Bm, Bn, P, Q );
    CHAMELEON_Desc_Create(
        &descC, (void*)(-mtxfmt), ChamComplexDouble, nb, nb, nb * nb, LDC, N, 0, 0, M, N, P, Q );

    /* Fill the matrices with random values */
    CHAMELEON_zplrnt_Tile( descA, seedA );
    CHAMELEON_zplrnt_Tile( descB, seedB );
    CHAMELEON_zplrnt_Tile( descC, seedC );

    if ( async ) {
        ws = CHAMELEON_zgemm_WS_Alloc( transA, transB, descA, descB, descC );
    }

    /* Calculate the product */
    testing_start( &test_data );
    if ( async ) {
        hres = CHAMELEON_zgemm_Tile_Async( transA, transB, alpha, descA, descB, beta, descC, ws,
                                           test_data.sequence, &test_data.request );
        CHAMELEON_Desc_Flush( descA, test_data.sequence );
        CHAMELEON_Desc_Flush( descB, test_data.sequence );
        CHAMELEON_Desc_Flush( descC, test_data.sequence );
    }
    else {
        hres = CHAMELEON_zgemm_Tile( transA, transB, alpha, descA, descB, beta, descC );
    }
    test_data.hres = hres;
    testing_stop( &test_data, flops_zgemm( M, N, K ) );

    if ( ws != NULL ) {
        CHAMELEON_zgemm_WS_Free( ws );
    }

    /* Check the solution */
    if ( check ) {
        CHAMELEON_Desc_Create(
            &descCinit, (void*)(-mtxfmt), ChamComplexDouble, nb, nb, nb * nb, LDC, N, 0, 0, M, N, P, Q );
        CHAMELEON_zplrnt_Tile( descCinit, seedC );

        hres += check_zgemm( args, transA, transB, alpha, descA, descB, beta, descCinit, descC );

        CHAMELEON_Desc_Destroy( &descCinit );
    }

    CHAMELEON_Desc_Destroy( &descA );
    CHAMELEON_Desc_Destroy( &descB );
    CHAMELEON_Desc_Destroy( &descC );

    return hres;
}

testing_t   test_zgemm;
const char *zgemm_params[] = { "mtxfmt", "nb",    "transA", "transB", "m",     "n",
                               "k",      "lda",   "ldb",    "ldc",    "alpha", "beta",
                               "seedA",  "seedB", "seedC",  NULL };
const char *zgemm_output[] = { NULL };
const char *zgemm_outchk[] = { "||A||", "||B||", "||C||", "||R||", "RETURN", NULL };

/**
 * @brief Testing registration function
 */
void testing_zgemm_init( void ) __attribute__( ( constructor ) );
void
testing_zgemm_init( void )
{
    test_zgemm.name   = "zgemm";
    test_zgemm.helper = "General matrix-matrix multiply";
    test_zgemm.params = zgemm_params;
    test_zgemm.output = zgemm_output;
    test_zgemm.outchk = zgemm_outchk;
    test_zgemm.fptr_desc = testing_zgemm_desc;
    test_zgemm.fptr_std  = NULL;
    test_zgemm.next   = NULL;

    testing_register( &test_zgemm );
}
