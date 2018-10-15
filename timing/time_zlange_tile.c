/**
 *
 * @file time_zlange_tile.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2016 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @version 1.0.0
 * @precisions normal z -> c d s
 *
 */
#define _TYPE  CHAMELEON_Complex64_t
#define _PREC  double
#define _LAMCH LAPACKE_dlamch_work

#define _NAME  "CHAMELEON_zlange_Tile"
/* See Lawn 41 page 120 */
#define _FMULS FMULS_LANGE(M, N)
#define _FADDS FADDS_LANGE(M, N)

#include "./timing.c"

static int
RunTest(int *iparam, double *dparam, chameleon_time_t *t_)
{
    double normcham;
    int    norm = ChamInfNorm;

    PASTE_CODE_IPARAM_LOCALS( iparam );

    /* Allocate Data */
    PASTE_CODE_ALLOCATE_MATRIX_TILE( descA, 1,     CHAMELEON_Complex64_t, ChamComplexDouble, LDA, M, N    );
    CHAMELEON_zplrnt_Tile( descA, 3436 );

    /* CHAMELEON ZPOSV */
    START_TIMING();
    normcham = CHAMELEON_zlange_Tile(norm, descA);
    STOP_TIMING();

#if !defined(CHAMELEON_SIMULATION)
    /* Check the solution */
    if ( check )
    {
        double normlapack, result;

        /* Allocate Data */
        PASTE_TILE_TO_LAPACK( descA, A, check, CHAMELEON_Complex64_t, M, N );
        double *work = (double*) malloc(chameleon_max(M,N)*sizeof(double));
        normlapack = LAPACKE_zlange_work(LAPACK_COL_MAJOR, chameleon_lapack_const(norm), M, N, A, LDA, work);
        result = fabs(normcham - normlapack);
        switch(norm) {
        case ChamMaxNorm:
            /* result should be perfectly equal */
            break;
        case ChamInfNorm:
            /* Sum order on the line can differ */
            result = result / (double)N;
            break;
        case ChamOneNorm:
            /* Sum order on the column can differ */
            result = result / (double)M;
            break;
        case ChamFrobeniusNorm:
            /* Sum oreder on every element can differ */
            result = result / ((double)M * (double)N);
            break;
        }
        if ( CHAMELEON_My_Mpi_Rank() == 0 ) {
            dparam[IPARAM_ANORM] = normlapack;
            dparam[IPARAM_BNORM] = 0.;
            dparam[IPARAM_XNORM] = 1.;
            dparam[IPARAM_RES] = result;
        }
        free( work );
        free( A );
    }
#endif

    PASTE_CODE_FREE_MATRIX( descA );
    (void)normcham;
    return 0;
}
