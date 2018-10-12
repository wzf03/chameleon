/**
 *
 * @file testing_ztrmm.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2018 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon ztrmm testing
 *
 * @version 1.0.0
 * @comment This file has been automatically generated
 *          from Plasma 2.5.0 for CHAMELEON 1.0.0
 * @author Mathieu Faverge
 * @author Emmanuel Agullo
 * @author Cedric Castagnede
 * @date 2010-11-15
 * @precisions normal z -> c d s
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <chameleon.h>
#include <coreblas/cblas.h>
#include <coreblas/lapacke.h>
#include <coreblas.h>
#include "testing_zauxiliary.h"

static int check_solution(cham_side_t side, cham_uplo_t uplo, cham_trans_t trans, cham_diag_t diag,
                          int M, int N, CHAMELEON_Complex64_t alpha,
                          CHAMELEON_Complex64_t *A, int LDA,
                          CHAMELEON_Complex64_t *Bref, CHAMELEON_Complex64_t *Bcham, int LDB);

int testing_ztrmm(int argc, char **argv)
{
    int hres = 0;
    /* Check for number of arguments*/
    if ( argc != 5 ) {
        USAGE("TRMM", "alpha M N LDA LDB",
              "   - alpha  : alpha coefficient\n"
              "   - M      : number of rows of matrices B\n"
              "   - N      : number of columns of matrices B\n"
              "   - LDA    : leading dimension of matrix A\n"
              "   - LDB    : leading dimension of matrix B\n");
        return -1;
    }

    CHAMELEON_Complex64_t alpha = (CHAMELEON_Complex64_t) atol(argv[0]);
    int M     = atoi(argv[1]);
    int N     = atoi(argv[2]);
    int LDA   = atoi(argv[3]);
    int LDB   = atoi(argv[4]);

    double eps;
    int info_solution;
    int s, u, t, d, i;
    int LDAxM = LDA*max(M,N);
    int LDBxN = LDB*max(M,N);

    CHAMELEON_Complex64_t *A      = (CHAMELEON_Complex64_t *)malloc(LDAxM*sizeof(CHAMELEON_Complex64_t));
    CHAMELEON_Complex64_t *B      = (CHAMELEON_Complex64_t *)malloc(LDBxN*sizeof(CHAMELEON_Complex64_t));
    CHAMELEON_Complex64_t *Binit  = (CHAMELEON_Complex64_t *)malloc(LDBxN*sizeof(CHAMELEON_Complex64_t));
    CHAMELEON_Complex64_t *Bfinal = (CHAMELEON_Complex64_t *)malloc(LDBxN*sizeof(CHAMELEON_Complex64_t));

    /* Check if unable to allocate memory */
    if ( (!A) || (!B) || (!Binit) || (!Bfinal) )
    {
        free(A); free(B);
        free(Binit); free(Bfinal);
        printf("Out of Memory \n ");
        return -2;
    }

    eps = LAPACKE_dlamch_work('e');

    printf("\n");
    printf("------ TESTS FOR CHAMELEON ZTRMM ROUTINE -------  \n");
    printf("            Size of the Matrix B : %d by %d\n", M, N);
    printf("\n");
    printf(" The matrix A is randomly generated for each test.\n");
    printf("============\n");
    printf(" The relative machine precision (eps) is to be %e \n",eps);
    printf(" Computational tests pass if scaled residuals are less than 10.\n");

    /*----------------------------------------------------------
     *  TESTING ZTRMM
     */

    /* Initialize A, B, C */
    LAPACKE_zlarnv_work(IONE, ISEED, LDAxM, A);
    LAPACKE_zlarnv_work(IONE, ISEED, LDBxN, B);
    for(i=0;i<max(M,N);i++)
      A[LDA*i+i] = A[LDA*i+i] + 2.0;

    for (s=0; s<2; s++) {
        for (u=0; u<2; u++) {
#if defined(PRECISION_z) || defined(PRECISION_c)
            for (t=0; t<3; t++) {
#else
            for (t=0; t<2; t++) {
#endif
                for (d=0; d<2; d++) {

                    memcpy(Binit,  B, LDBxN*sizeof(CHAMELEON_Complex64_t));
                    memcpy(Bfinal, B, LDBxN*sizeof(CHAMELEON_Complex64_t));

                    /* CHAMELEON ZTRMM */
                    CHAMELEON_ztrmm(side[s], uplo[u], trans[t], diag[d],
                                 M, N, alpha, A, LDA, Bfinal, LDB);

                    /* Check the solution */
                    info_solution = check_solution(side[s], uplo[u], trans[t], diag[d],
                                                   M, N, alpha, A, LDA, Binit, Bfinal, LDB);

                    printf("***************************************************\n");
                    if (info_solution == 0) {
                        printf(" ---- TESTING ZTRMM (%s, %s, %s, %s) ...... PASSED !\n",
                               sidestr[s], uplostr[u], transstr[t], diagstr[d]);
                    }
                    else {
                        printf(" ---- TESTING ZTRMM (%s, %s, %s, %s) ... FAILED !\n",
                               sidestr[s], uplostr[u], transstr[t], diagstr[d]);    hres++;
                    }
                    printf("***************************************************\n");
                }
            }
        }
    }

    free(A); free(B);
    free(Binit); free(Bfinal);

    return hres;
}

/*--------------------------------------------------------------
 * Check the solution
 */
static int check_solution(cham_side_t side, cham_uplo_t uplo, cham_trans_t trans, cham_diag_t diag,
                          int M, int N, CHAMELEON_Complex64_t alpha,
                          CHAMELEON_Complex64_t *A, int LDA,
                          CHAMELEON_Complex64_t *Bref, CHAMELEON_Complex64_t *Bcham, int LDB)
{
    int info_solution;
    double Anorm, Binitnorm, Bchamnorm, Blapacknorm, Rnorm, result;
    double eps;
    CHAMELEON_Complex64_t mzone = (CHAMELEON_Complex64_t)-1.0;

    double *work = (double *)malloc(max(M, N)* sizeof(double));
    int Am, An;

    if (side == ChamLeft) {
        Am = M; An = M;
    } else {
        Am = N; An = N;
    }

    Anorm       = LAPACKE_zlantr_work(LAPACK_COL_MAJOR, 'I', chameleon_lapack_const(uplo), chameleon_lapack_const(diag),
                                Am, An, A, LDA, work);
    Binitnorm   = LAPACKE_zlange_work(LAPACK_COL_MAJOR, 'I', M, N, Bref,    LDB, work);
    Bchamnorm = LAPACKE_zlange_work(LAPACK_COL_MAJOR, 'I', M, N, Bcham, LDB, work);

    cblas_ztrmm(CblasColMajor, (CBLAS_SIDE)side, (CBLAS_UPLO)uplo, (CBLAS_TRANSPOSE)trans,
                (CBLAS_DIAG)diag, M, N, CBLAS_SADDR(alpha), A, LDA, Bref, LDB);

    Blapacknorm = LAPACKE_zlange_work(LAPACK_COL_MAJOR, 'I', M, N, Bref, LDB, work);

    cblas_zaxpy(LDB * N, CBLAS_SADDR(mzone), Bcham, 1, Bref, 1);

    Rnorm = LAPACKE_zlange_work(LAPACK_COL_MAJOR, 'I', M, N, Bref, LDB, work);

    eps = LAPACKE_dlamch_work('e');

    printf("Rnorm %e, Anorm %e, Binitnorm %e, Bchamnorm %e, Blapacknorm %e\n",
           Rnorm, Anorm, Binitnorm, Bchamnorm, Blapacknorm);

    result = Rnorm / ((Anorm + Blapacknorm) * max(M,N) * eps);

    printf("============\n");
    printf("Checking the norm of the difference against reference ZTRMM \n");
    printf("-- ||Ccham - Clapack||_oo/((||A||_oo+||B||_oo).N.eps) = %e \n", result);

    if ( isinf(Blapacknorm) || isinf(Bchamnorm) || isnan(result) || isinf(result) || (result > 10.0) ) {
        printf("-- The solution is suspicious ! \n");
        info_solution = 1;
    }
    else {
        printf("-- The solution is CORRECT ! \n");
        info_solution= 0 ;
    }
    free(work);

    return info_solution;
}
