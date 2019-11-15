/**
 *
 * @file testing_zcheck.h
 *
 * @copyright 2019-2019 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon CHAMELEON_Complex64_t auxiliary testings header
 *
 * @version 0.9.2
 * @author Lucas Barros de Assis
 * @date 2019-07-16
 * @precisions normal z -> c d s
 *
 */

#ifndef _testing_zcheck_h_
#define _testing_zcheck_h_

#include "testings.h"
#include <math.h>

#ifdef WIN32
#include <float.h>
#define isnan _isnan
#endif

#define CHECK_TRMM 3
#define CHECK_TRSM 4

int check_zmatrices     ( run_arg_list_t *args, cham_uplo_t uplo, CHAM_desc_t *descA, CHAM_desc_t *descB );
int check_znorm         ( run_arg_list_t *args, cham_mtxtype_t mtxtype, cham_normtype_t norm_type, cham_uplo_t uplo,
                          cham_diag_t diag, double norm_cham, CHAM_desc_t *descA );
int check_zsum          ( run_arg_list_t *args, cham_uplo_t uplo, cham_trans_t trans, CHAMELEON_Complex64_t alpha, CHAM_desc_t *descA,
                          CHAMELEON_Complex64_t beta, CHAM_desc_t *descBref, CHAM_desc_t *descBcham );
int check_zscale        ( run_arg_list_t *args, cham_uplo_t uplo, CHAMELEON_Complex64_t alpha, CHAM_desc_t *descA1, CHAM_desc_t *descA2 );
int check_zgemm         ( run_arg_list_t *args, cham_trans_t transA, cham_trans_t transB, CHAMELEON_Complex64_t alpha, CHAM_desc_t *descA,
                          CHAM_desc_t *descB, CHAMELEON_Complex64_t beta, CHAM_desc_t *descCref, CHAM_desc_t *descC );
int check_zsymm         ( run_arg_list_t *args, cham_mtxtype_t mtxtype, cham_side_t side, cham_uplo_t uplo, CHAMELEON_Complex64_t alpha, CHAM_desc_t *descA, CHAM_desc_t *descB,
                          CHAMELEON_Complex64_t beta, CHAM_desc_t *descCref, CHAM_desc_t *descC );
int check_zsyrk         ( run_arg_list_t *args, cham_mtxtype_t mtxtype, cham_uplo_t uplo, cham_trans_t trans, CHAMELEON_Complex64_t alpha, CHAM_desc_t *descA,
                          CHAM_desc_t *descB, CHAMELEON_Complex64_t beta, CHAM_desc_t *descCref, CHAM_desc_t *descC );
int check_ztrmm         ( run_arg_list_t *args, int check_func, cham_side_t side, cham_uplo_t uplo, cham_trans_t trans, cham_diag_t diag,
                          CHAMELEON_Complex64_t alpha, CHAM_desc_t *descA, CHAM_desc_t *descB, CHAM_desc_t *descBref );
int check_zlauum        ( run_arg_list_t *args, cham_uplo_t uplo, CHAM_desc_t *descA1, CHAM_desc_t *descA2 );
int check_zxxtrf        ( run_arg_list_t *args, cham_mtxtype_t mtxtype, cham_uplo_t uplo, CHAM_desc_t *descA1, CHAM_desc_t *descA2 );
int check_zsolve        ( run_arg_list_t *args, cham_mtxtype_t mtxtype, cham_trans_t trans, cham_uplo_t uplo,
                          CHAM_desc_t *descA, CHAM_desc_t *descX, CHAM_desc_t *descB );
int check_ztrtri        ( run_arg_list_t *args, cham_mtxtype_t mtxtype, cham_uplo_t uplo, cham_diag_t diag,
                          CHAM_desc_t *descA, CHAM_desc_t *descAi );

/* Using QR factorization */
int check_zortho        ( run_arg_list_t *args, CHAM_desc_t *descQ );
int check_zgeqrf        ( run_arg_list_t *args, CHAM_desc_t *descA, CHAM_desc_t *descAF, CHAM_desc_t *descQ );
int check_zgelqf        ( run_arg_list_t *args, CHAM_desc_t *descA, CHAM_desc_t *descAF, CHAM_desc_t *descQ );
int check_zgels         ( run_arg_list_t *args, cham_trans_t trans, CHAM_desc_t *descA, CHAM_desc_t *descX, CHAM_desc_t *descB );
int check_zgeqrs        ( run_arg_list_t *args, cham_trans_t trans, CHAM_desc_t *descA, double Bnorm, CHAM_desc_t *descR );
int check_zgelqs        ( run_arg_list_t *args, cham_trans_t trans, CHAM_desc_t *descA, double Bnorm, CHAM_desc_t *descR );
int check_zqc           ( run_arg_list_t *args, cham_side_t side, cham_trans_t trans, CHAM_desc_t *descC, CHAM_desc_t *descQ, CHAM_desc_t *descCC );

#endif

