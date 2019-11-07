/**
 *
 * @file pzher2k.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2019 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon zher2k parallel algorithm
 *
 * @version 0.9.2
 * @comment This file has been automatically generated
 *          from Plasma 2.5.0 for CHAMELEON 0.9.2
 * @author Mathieu Faverge
 * @author Emmanuel Agullo
 * @author Cedric Castagnede
 * @date 2014-11-16
 * @precisions normal z -> c
 *
 */
#include "control/common.h"

#define A(m,n) A,  m,  n
#define B(m,n) B,  m,  n
#define C(m,n) C,  m,  n
/**
 *  Parallel tile Hermitian rank-k update - dynamic scheduling
 */
void chameleon_pzher2k( cham_uplo_t uplo, cham_trans_t trans,
                        CHAMELEON_Complex64_t alpha, CHAM_desc_t *A, CHAM_desc_t *B,
                        double beta,  CHAM_desc_t *C,
                        RUNTIME_sequence_t *sequence, RUNTIME_request_t *request)
{
    CHAM_context_t *chamctxt;
    RUNTIME_option_t options;

    int m, n, k, mmin, mmax;
    int ldak, ldam, ldan, ldcm, ldcn;
    int ldbk, ldbm, ldbn;
    int tempnn, tempmm, tempkn, tempkm;

    CHAMELEON_Complex64_t zone   = (CHAMELEON_Complex64_t)1.0;
    CHAMELEON_Complex64_t zbeta;
    double dbeta;

    chamctxt = chameleon_context_self();
    if (sequence->status != CHAMELEON_SUCCESS) {
        return;
    }
    RUNTIME_options_init(&options, chamctxt, sequence, request);

    for (n = 0; n < C->nt; n++) {
        tempnn = n == C->nt-1 ? C->n-n*C->nb : C->nb;
        ldan = BLKLDD(A, n);
        ldbn = BLKLDD(B, n);
        ldcn = BLKLDD(C, n);

        if (uplo == ChamLower) {
            mmin = n+1;
            mmax = C->mt;
        }
        else {
            mmin = 0;
            mmax = n;
        }

        /*
         *  ChamNoTrans
         */
        if (trans == ChamNoTrans) {
            for (k = 0; k < A->nt; k++) {
                tempkn = k == A->nt-1 ? A->n-k*A->nb : A->nb;
                dbeta = k == 0 ? beta : 1.0;
                INSERT_TASK_zher2k(
                    &options,
                    uplo, trans,
                    tempnn, tempkn, A->mb,
                    alpha, A(n, k), ldan, /* ldan * K */
                           B(n, k), ldbn,
                    dbeta, C(n, n), ldcn); /* ldc  * N */
            }
            for (m = mmin; m < mmax; m++) {
                tempmm = m == C->mt-1 ? C->m-m*C->mb : C->mb;
                ldam = BLKLDD(A, m);
                ldbm = BLKLDD(B, m);
                ldcm = BLKLDD(C, m);
                for (k = 0; k < A->nt; k++) {
                    tempkn = k == A->nt-1 ? A->n-k*A->nb : A->nb;
                    zbeta = k == 0 ? (CHAMELEON_Complex64_t)beta : zone;
                    INSERT_TASK_zgemm(
                        &options,
                        ChamNoTrans, ChamConjTrans,
                        tempmm, tempnn, tempkn, A->mb,
                        alpha, A(m, k), ldam,
                               B(n, k), ldbn,
                        zbeta, C(m, n), ldcm);

                    INSERT_TASK_zgemm(
                        &options,
                        ChamNoTrans, ChamConjTrans,
                        tempmm, tempnn, tempkn, A->mb,
                        conj(alpha), B(m, k), ldbm,
                                     A(n, k), ldan,
                        zone,        C(m, n), ldcm);
                }
            }
        }
        /*
         *  ChamConjTrans
         */
        else {
            for (k = 0; k < A->mt; k++) {
                tempkm = k == A->mt-1 ? A->m-k*A->mb : A->mb;
                ldak = BLKLDD(A, k);
                ldbk = BLKLDD(B, k);
                dbeta = k == 0 ? beta : 1.0;
                INSERT_TASK_zher2k(
                    &options,
                    uplo, trans,
                    tempnn, tempkm, A->mb,
                    alpha, A(k, n), ldak,  /* lda * N */
                           B(k, n), ldbk,
                    dbeta, C(n, n), ldcn); /* ldc * N */
            }
            for (m = mmin; m < mmax; m++) {
                tempmm = m == C->mt-1 ? C->m-m*C->mb : C->mb;
                ldcm = BLKLDD(C, m);
                for (k = 0; k < A->mt; k++) {
                    tempkm = k == A->mt-1 ? A->m-k*A->mb : A->mb;
                    ldak = BLKLDD(A, k);
                    ldbk = BLKLDD(B, k);
                    zbeta = k == 0 ? (CHAMELEON_Complex64_t)beta : zone;
                    INSERT_TASK_zgemm(
                        &options,
                        ChamConjTrans, ChamNoTrans,
                        tempmm, tempnn, tempkm, A->mb,
                        alpha, A(k, m), ldak,
                               B(k, n), ldbk,
                        zbeta, C(m, n), ldcm);

                    INSERT_TASK_zgemm(
                        &options,
                        ChamConjTrans, ChamNoTrans,
                        tempmm, tempnn, tempkm, A->mb,
                        conj(alpha), B(k, m), ldbk,
                                     A(k, n), ldak,
                        zone,        C(m, n), ldcm );
                }
            }
        }
    }
    RUNTIME_options_finalize(&options, chamctxt);
}
