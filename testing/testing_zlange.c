/**
 *
 * @file testing_zlange.c
 *
 * @copyright 2019-2021 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon zlange testing
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

static cham_fixdbl_t
flops_zlange( cham_normtype_t ntype, int M, int N )
{
    cham_fixdbl_t flops   = 0.;
    cham_fixdbl_t coefabs = 1.;
    cham_fixdbl_t size;
#if defined( PRECISION_z ) || defined( PRECISION_c )
    coefabs = 3.;
#endif

    size = (cham_fixdbl_t)M * (cham_fixdbl_t)N;
    switch ( ntype ) {
        case ChamMaxNorm:
            flops = coefabs * size;
            break;
        case ChamOneNorm:
            flops = coefabs * size + M * ( N - 1 );
            break;
        case ChamInfNorm:
            flops = coefabs * size + N * ( M - 1 );
            break;
        case ChamFrobeniusNorm:
            flops = ( coefabs + 1. ) * size;
            break;
        default:;
    }
    (void)flops;
    return sizeof( CHAMELEON_Complex64_t ) * size;
}

int
testing_zlange_desc( run_arg_list_t *args, int check )
{
    testdata_t test_data = { .args = args };
    int        hres      = 0;

    /* Read arguments */
    int             async     = parameters_getvalue_int( "async" );
    intptr_t        mtxfmt    = parameters_getvalue_int( "mtxfmt" );
    int             nb        = run_arg_get_int( args, "nb", 320 );
    int             P         = parameters_getvalue_int( "P" );
    cham_normtype_t norm_type = run_arg_get_ntype( args, "norm", ChamMaxNorm );
    int             N         = run_arg_get_int( args, "N", 1000 );
    int             M         = run_arg_get_int( args, "M", N );
    int             LDA       = run_arg_get_int( args, "LDA", M );
    int             seedA     = run_arg_get_int( args, "seedA", random() );
    int             Q         = parameters_compute_q( P );

    /* Descriptors */
    double       norm;
    CHAM_desc_t *descA;

    CHAMELEON_Set( CHAMELEON_TILE_SIZE, nb );

    /* Creates the matrix */
    CHAMELEON_Desc_Create(
        &descA, (void*)(-mtxfmt), ChamComplexDouble, nb, nb, nb * nb, LDA, N, 0, 0, M, N, P, Q );

    /* Fills the matrix with random values */
    CHAMELEON_zplrnt_Tile( descA, seedA );

    /* Calculates the norm */
    testing_start( &test_data );
    if ( async ) {
        hres = CHAMELEON_zlange_Tile_Async( norm_type, descA, &norm,
                                            test_data.sequence, &test_data.request );
        CHAMELEON_Desc_Flush( descA, test_data.sequence );
    }
    else {
        norm = CHAMELEON_zlange_Tile( norm_type, descA );
    }
    test_data.hres = hres;
    testing_stop( &test_data, flops_zlange( norm_type, M, N ) );

    /* Checks the solution */
    if ( check ) {
        hres = check_znorm( args, ChamGeneral, norm_type, ChamUpperLower,
                            ChamNonUnit, norm, descA );
    }

    CHAMELEON_Desc_Destroy( &descA );

    return hres;
}

testing_t   test_zlange;
const char *zlange_params[] = { "mtxfmt", "nb", "norm", "m", "n", "lda", "seedA", NULL };
const char *zlange_output[] = { NULL };
const char *zlange_outchk[] = { "RETURN", NULL };

/**
 * @brief Testing registration function
 */
void testing_zlange_init( void ) __attribute__( ( constructor ) );
void
testing_zlange_init( void )
{
    test_zlange.name   = "zlange";
    test_zlange.helper = "General matrix norm";
    test_zlange.params = zlange_params;
    test_zlange.output = zlange_output;
    test_zlange.outchk = zlange_outchk;
    test_zlange.fptr_desc = testing_zlange_desc;
    test_zlange.fptr_std  = NULL;
    test_zlange.next   = NULL;

    testing_register( &test_zlange );
}
