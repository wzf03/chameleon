/**
 *
 * @file testing_zlantr.c
 *
 * @copyright 2019-2019 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon zlantr testing
 *
 * @version 0.9.2
 * @author Lucas Barros de Assis
 * @date 2014-07-17
 * @precisions normal z -> c d s
 *
 */
#include <chameleon.h>
#include "testing_zauxiliary.h"
#include "testing_zcheck.h"
#include "flops.h"

static cham_fixdbl_t
flops_zlantr( cham_normtype_t ntype, cham_uplo_t uplo, cham_diag_t diag, int M, int N )
{
    /* TODO: update formula */
    cham_fixdbl_t flops   = 0.;
    double coefabs = 1.;
#if defined( PRECISION_z ) || defined( PRECISION_c )
    coefabs = 3.;
#endif

    switch ( ntype ) {
        case ChamMaxNorm:
            flops = coefabs * ( N * ( N + 1 ) ) / 2.;
            break;
        case ChamOneNorm:
        case ChamInfNorm:
            flops = coefabs * ( N * ( N + 1 ) ) / 2. + N * ( N - 1 );
            break;
        case ChamFrobeniusNorm:
            flops = ( coefabs + 1. ) * ( N * ( N + 1 ) ) / 2.;
            break;
        default:;
    }
    return flops;
}

int
testing_zlantr( run_arg_list_t *args, int check )
{
    static int   run_id = 0;
    int          hres   = 0;
    double       norm;
    CHAM_desc_t *descA;

    /* Reads arguments */
    int             nb        = run_arg_get_int( args, "nb", 320 );
    int             P         = parameters_getvalue_int( "P" );
    cham_normtype_t norm_type = run_arg_get_ntype( args, "norm", ChamMaxNorm );
    cham_uplo_t     uplo      = run_arg_get_uplo( args, "uplo", ChamUpper );
    cham_diag_t     diag      = run_arg_get_diag( args, "diag", ChamNonUnit );
    int             N         = run_arg_get_int( args, "N", 1000 );
    int             M         = run_arg_get_int( args, "M", N );
    int             LDA       = run_arg_get_int( args, "LDA", M );
    int             seedA     = run_arg_get_int( args, "seedA", random() );
    int             Q         = parameters_compute_q( P );
    cham_fixdbl_t t, gflops;
    cham_fixdbl_t flops = flops_zlantr( norm_type, uplo, diag, M, N );

    CHAMELEON_Set( CHAMELEON_TILE_SIZE, nb );

    /* Creates the matrix */
    CHAMELEON_Desc_Create(
        &descA, NULL, ChamComplexDouble, nb, nb, nb * nb, LDA, N, 0, 0, M, N, P, Q );

    /* Fills the matrix with random values */
    CHAMELEON_zplrnt_Tile( descA, seedA );

    /* Calculates the norm */
    START_TIMING( t );
    norm = CHAMELEON_zlantr_Tile( norm_type, uplo, diag, descA );
    STOP_TIMING( t );
    gflops = flops * 1.e-9 / t;
    run_arg_add_fixdbl( args, "time", t );
    run_arg_add_fixdbl( args, "gflops", gflops );

    /* Checks the solution */
    if ( check ) {
        hres = check_znorm( args, ChamTriangular, norm_type, uplo, diag, norm, descA );
    }

    CHAMELEON_Desc_Destroy( &descA );

    run_id++;
    return hres;
}

testing_t   test_zlantr;
const char *zlantr_params[] = { "nb", "norm", "uplo", "diag", "m", "n", "lda", "seedA", NULL };
const char *zlantr_output[] = { NULL };
const char *zlantr_outchk[] = { "RETURN", NULL };

/**
 * @brief Testing registration function
 */
void testing_zlantr_init( void ) __attribute__( ( constructor ) );
void
testing_zlantr_init( void )
{
    test_zlantr.name        = "zlantr";
    test_zlantr.helper      = "zlantr";
    test_zlantr.params      = zlantr_params;
    test_zlantr.output      = zlantr_output;
    test_zlantr.outchk      = zlantr_outchk;
    test_zlantr.params_list = "nb;P;norm;uplo;diag;m;n;lda;seedA";
    test_zlantr.fptr        = testing_zlantr;
    test_zlantr.next        = NULL;

    testing_register( &test_zlantr );
}