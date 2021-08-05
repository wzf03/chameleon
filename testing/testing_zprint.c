/**
 *
 * @file testing_zprint.c
 *
 * @copyright 2019-2021 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon zprint testing
 *
 * @version 1.1.0
 * @author Lucas Barros de Assis
 * @author Mathieu Faverge
 * @date 2020-11-19
 * @precisions normal z -> c d s
 *
 */
#include <chameleon.h>
#include "testings.h"
#include "testing_zcheck.h"
#include <chameleon/flops.h>

/**
 *  Internal function to return address of block (m,n) with m,n = block indices
 */
inline static void *chameleon_getaddr_cm(const CHAM_desc_t *A, int m, int n)
{
    size_t mm = m + A->i / A->mb;
    size_t nn = n + A->j / A->nb;
    size_t eltsize = CHAMELEON_Element_Size(A->dtyp);
    size_t offset = 0;

#if defined(CHAMELEON_USE_MPI)
    assert( A->myrank == A->get_rankof( A, mm, nn) );
    mm = mm / A->p;
    nn = nn / A->q;
#endif

    offset = (size_t)(A->llm * A->nb) * nn + (size_t)(A->mb) * mm;
    return (void*)((intptr_t)A->mat + (offset*eltsize) );
}

inline static int chameleon_getblkldd_cm(const CHAM_desc_t *A, int m) {
    (void)m;
    return A->llm;
}

int
testing_zprint( run_arg_list_t *args, int check )
{
    int          hres = 0;
    CHAM_desc_t *descA;

    /* Reads arguments */
    intptr_t    mtxfmt = parameters_getvalue_int( "mtxfmt" );
    int         nb     = run_arg_get_int( args, "nb", 320 );
    int         P      = parameters_getvalue_int( "P" );
    int         N      = run_arg_get_int( args, "N", 1000 );
    int         M      = run_arg_get_int( args, "M", N );
    int         LDA    = run_arg_get_int( args, "LDA", M );
    int         l1    = run_arg_get_int( args, "l1", nb / 2 );
    int         l2    = run_arg_get_int( args, "l2", l1 / 3 );
    int         l3    = run_arg_get_int( args, "l3", l2 / 2 );
    int         Q      = parameters_compute_q( P );

    int list_nb[] = { nb, l1, l2, l3, 0 };

    CHAMELEON_Set( CHAMELEON_TILE_SIZE, nb );

    fprintf( stdout, "--- Tile layout ---\n" );
    CHAMELEON_Desc_Create(
        &descA, (void*)(-mtxfmt), ChamComplexDouble, nb, nb, nb * nb, LDA, N, 0, 0, M, N, P, Q );

    CHAMELEON_Desc_Print( descA );

    CHAMELEON_Desc_Destroy( &descA );

    fprintf( stdout, "--- Lapacke layout ---\n" );
    CHAMELEON_Desc_Create_User(
        &descA, (void*)(-mtxfmt), ChamComplexDouble, nb, nb, nb * nb, LDA, N, 0, 0, M, N, P, Q,
        chameleon_getaddr_cm, chameleon_getblkldd_cm, NULL );

    CHAMELEON_Desc_Print( descA );
    CHAMELEON_Desc_Destroy( &descA );

    fprintf( stdout, "--- Recursive layout (Tile)---\n" );
    CHAMELEON_Recursive_Desc_Create(
        &descA, CHAMELEON_MAT_ALLOC_GLOBAL, ChamComplexDouble,
        list_nb, list_nb, LDA, N, M, N, P, Q,
        NULL, NULL, NULL );

    CHAMELEON_Desc_Print( descA );
    CHAMELEON_Desc_Destroy( &descA );

    fprintf( stdout, "--- Recursive layout (Lapack) ---\n" );
    CHAMELEON_Recursive_Desc_Create(
        &descA, CHAMELEON_MAT_ALLOC_GLOBAL, ChamComplexDouble,
        list_nb, list_nb, LDA, N, M, N, P, Q,
        chameleon_getaddr_cm, chameleon_getblkldd_cm, NULL );

    CHAMELEON_Desc_Print( descA );
    CHAMELEON_Desc_Destroy( &descA );

    run_arg_add_fixdbl( args, "time", 1. );
    run_arg_add_fixdbl( args, "gflops", 1. );

    return hres;
}

testing_t   test_zprint;
const char *zprint_params[] = { "mtxfmt", "nb", "l1", "l2", "l3", "m", "n", "lda", NULL };
const char *zprint_output[] = { NULL };
const char *zprint_outchk[] = { "RETURN", NULL };

/**
 * @brief Testing registration function
 */
void testing_zprint_init( void ) __attribute__( ( constructor ) );
void
testing_zprint_init( void )
{
    test_zprint.name        = "zprint";
    test_zprint.helper      = "Print descriptors";
    test_zprint.params      = zprint_params;
    test_zprint.output      = zprint_output;
    test_zprint.outchk      = zprint_outchk;
    test_zprint.fptr        = testing_zprint;
    test_zprint.next        = NULL;

    testing_register( &test_zprint );
}