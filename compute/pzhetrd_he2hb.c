/**
 *
 * @file pzhetrd_he2hb.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2014 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon zhetrd_he2hb parallel algorithm
 *
 * @version 1.0.0
 * @author Hatem Ltaief
 * @author Azzam Haidar
 * @date 2010-11-15
 * @precisions normal z -> s d c
 *
 */
#include "control/common.h"
#include <stdlib.h>

#define A(m, n) A,  m,  n
#define T(m, n) T,  m,  n
#define D(k) D, (k)-1, 0

#define AT(k) AT, k, 0

#if defined(CHAMELEON_COPY_DIAG)
#define E(m, n) E, m, 0
#else
#define E(m, n) A, m, n
#endif

/**
 *  Parallel tile BAND Tridiagonal Reduction - dynamic scheduler
 */
void chameleon_pzhetrd_he2hb(cham_uplo_t uplo,
                         CHAM_desc_t *A, CHAM_desc_t *T, CHAM_desc_t *E,
                         RUNTIME_sequence_t *sequence, RUNTIME_request_t *request)
{
    CHAM_context_t *chamctxt;
    RUNTIME_option_t options;
    CHAM_desc_t *D  = NULL;
    CHAM_desc_t *AT = NULL;
    size_t ws_worker = 0;
    size_t ws_host = 0;

    int k, m, n, i, j;
    int ldak, ldak1, ldam, ldan, ldaj, ldai;
    int tempkm, tempkn, tempmm, tempnn, tempjj;
    int ib;

    chamctxt = chameleon_context_self();
    if (sequence->status != CHAMELEON_SUCCESS)
        return;

    RUNTIME_options_init(&options, chamctxt, sequence, request);
    ib = CHAMELEON_IB;

    /*
     * zgeqrt        = A->nb * (ib+1)
     * zunmqr        = A->nb * ib
     * ztsqrt        = A->nb * (ib+1)
     * ztsmqr        = A->nb * ib
     * zherfb        = A->nb * ib
     * ztsmqr_hetra1 = A->nb * ib
     */
    ws_worker = A->nb * (ib+1);

    /* Allocation of temporary (scratch) working space */
#if defined(CHAMELEON_USE_CUDA)
    /* Worker space
     *
     * zunmqr = A->nb * ib
     * ztsmqr = 2 * A->nb * ib
     * zherfb = A->nb * ib
     */
    ws_worker = chameleon_max( ws_worker, ib * A->nb * 2 );
#endif

    ws_worker *= sizeof(CHAMELEON_Complex64_t);
    ws_host   *= sizeof(CHAMELEON_Complex64_t);

    RUNTIME_options_ws_alloc( &options, ws_worker, ws_host );

    /* Copy of the diagonal tiles to keep the general version of the tile all along the computation */
    D = (CHAM_desc_t*)malloc(sizeof(CHAM_desc_t));
    chameleon_zdesc_alloc_diag(*D, A->mb, A->nb, chameleon_min(A->m, A->n) - A->mb, A->nb, 0, 0, chameleon_min(A->m, A->n) - A->mb, A->nb, A->p, A->q);

    AT = (CHAM_desc_t*)malloc(sizeof(CHAM_desc_t));
    *AT = chameleon_desc_init(
        ChamComplexDouble, A->mb, A->nb, (A->mb*A->nb),
        chameleon_min(A->mt, A->nt) * A->mb, A->nb, 0, 0, chameleon_min(A->mt, A->nt) * A->mb, A->nb, 1, 1);
    chameleon_desc_mat_alloc( AT );
    RUNTIME_desc_create( AT );

    /* Let's extract the diagonal in a temporary copy that contains A and A' */
    for (k = 1; k < A->nt; k++){
        tempkn = k == A->nt-1 ? A->n-k*A->nb : A->nb;
        ldak = BLKLDD(A, k);

        INSERT_TASK_zhe2ge(&options,
                          uplo,
                          tempkn, tempkn, ldak,
                          A(k, k), ldak,
                          D(k),    ldak);
    }

    if (uplo == ChamLower) {
       for (k = 0; k < A->nt-1; k++){
           RUNTIME_iteration_push(chamctxt, k);

           tempkm = k+1 == A->mt-1 ? A->m-(k+1)*A->mb : A->mb;
           tempkn = k   == A->nt-1 ? A->n- k   *A->nb : A->nb;
           ldak1 = BLKLDD(A, k+1);

           INSERT_TASK_zgeqrt(
               &options,
               tempkm, tempkn, ib, A->nb,
               A(k+1, k), ldak1,
               T(k+1, k), T->mb);

#if defined(CHAMELEON_COPY_DIAG)
           INSERT_TASK_zlacpy(
               &options,
               ChamLower, tempkm, tempkn, A->nb,
               A(k+1, k), ldak1,
               E(k+1, k), ldak1 );
#if defined(CHAMELEON_USE_CUDA)
           INSERT_TASK_zlaset(
               &options,
               ChamUpper, tempkm, tempkn,
               0., 1.,
               E(k+1, k), ldak1 );
#endif
#endif

           /* LEFT and RIGHT on the symmetric diagonal block */
           INSERT_TASK_zherfb(
               &options,
               ChamLower,
               tempkm, tempkm, ib, A->nb,
               E(k+1, k), ldak1,
               T(k+1, k), T->mb,
               D(k+1),    ldak1);

           /* RIGHT on the remaining tiles until the bottom */
           for (m = k+2; m < A->mt ; m++) {
               tempmm = m == A->mt-1 ? A->m-m*A->mb : A->mb;
               ldam = BLKLDD(A, m);
               INSERT_TASK_zunmqr(
                   &options,
                   ChamRight, ChamNoTrans,
                   tempmm, A->nb, tempkm, ib, A->nb,
                   E(k+1, k),   ldak1,
                   T(k+1, k),   T->mb,
                   A(m,   k+1), ldam);
           }

           for (m = k+2; m < A->mt; m++) {
               tempmm = m == A->mt-1 ? A->m-m*A->mb : A->mb;
               ldam = BLKLDD(A, m);

               options.priority = 1;
               INSERT_TASK_ztsqrt(
                   &options,
                   tempmm, A->nb, ib, A->nb,
                   A(k+1, k), ldak1,
                   A(m  , k), ldam,
                   T(m  , k), T->mb);
               options.priority = 0;

               /* LEFT */
               for (i = k+2; i < m; i++) {
                   ldai = BLKLDD(A, i);
                   INSERT_TASK_ztsmqr_hetra1(
                       &options,
                       ChamLeft, ChamConjTrans,
                       A->mb, A->nb, tempmm, A->nb, A->nb, ib, A->nb,
                       A(i, k+1), ldai,
                       A(m,   i), ldam,
                       A(m,   k), ldam,
                       T(m,   k), T->mb);
               }

               /* RIGHT */
               for (j = m+1; j < A->mt ; j++) {
                   tempjj = j == A->mt-1 ? A->m-j*A->mb : A->mb;
                   ldaj = BLKLDD(A, j);
                   INSERT_TASK_ztsmqr(
                       &options,
                       ChamRight, ChamNoTrans,
                       tempjj, A->nb, tempjj, tempmm, A->nb, ib, A->nb,
                       A(j, k+1), ldaj,
                       A(j,   m), ldaj,
                       A(m,   k), ldam,
                       T(m,   k), T->mb);
               }

               /* LEFT->RIGHT */
               options.priority = 1;
               /**
                * Compute the update of the following:
                *
                *     A1   A2'
                *     A2   A3
                *
                * with A1 and A3 two diagonal tiles. This is the tsmqr_corner
                * from plasma split in 4 tasks
                */
               /*  Copy the transpose of A2 (m, k+1): AT(k) <- A2' = A2(k+1, m) */
               INSERT_TASK_zlatro(
                   &options,
                   ChamUpperLower, ChamConjTrans,
                   tempmm, A->nb, A->nb,
                   A(m, k+1), ldam,
                   AT(m),  ldak1);

               /*  Left application on |A1| */
               /*                      |A2| */
               INSERT_TASK_ztsmqr(
                   &options,
                   ChamLeft, ChamConjTrans,
                   A->mb, A->nb, tempmm, A->nb, A->nb, ib, A->nb,
                   D(k+1), ldak1,
                   A(m, k+1), ldam,
                   A(m,   k), ldam,
                   T(m,   k), T->mb);

               /*  Left application on | A2'| */
               /*                      | A3 | */
               INSERT_TASK_ztsmqr(
                   &options,
                   ChamLeft, ChamConjTrans,
                   A->mb, tempmm, tempmm, tempmm, A->nb, ib, A->nb,
                   AT(m), ldak1,
                   D(m) , ldam,
                   A(m,  k), ldam,
                   T(m,  k), T->mb);

               /*  Right application on | A1 A2' | */
               INSERT_TASK_ztsmqr(
                   &options,
                   ChamRight, ChamNoTrans,
                   A->mb, A->nb, A->mb, tempmm, A->nb, ib, A->nb,
                   D(k+1), ldak1,
                   AT(m) , ldak1,
                   A(m,   k), ldam,
                   T(m,   k), T->mb);

               /*  Right application on | A2 A3 | */
               INSERT_TASK_ztsmqr(
                   &options,
                   ChamRight, ChamNoTrans,
                   tempmm, A->nb, tempmm, tempmm, A->nb, ib, A->nb,
                   A(m, k+1), ldam,
                   D(m)  , ldam,
                   A(m,   k), ldam,
                   T(m,   k), T->mb);
               options.priority = 0;
           }

           RUNTIME_iteration_pop(chamctxt);
       }
    }
    else {
       for (k = 0; k < A->nt-1; k++){
           RUNTIME_iteration_push(chamctxt, k);

           tempkn = k+1 == A->nt-1 ? A->n-(k+1)*A->nb : A->nb;
           tempkm = k   == A->mt-1 ? A->m- k   *A->mb : A->mb;
           ldak  = BLKLDD(A, k);
           ldak1 = BLKLDD(A, k+1);
           INSERT_TASK_zgelqt(
               &options,
               tempkm, tempkn, ib, A->nb,
               A(k, k+1), ldak,
               T(k, k+1), T->mb);

#if defined(CHAMELEON_COPY_DIAG)
           INSERT_TASK_zlacpy(
               &options,
               ChamUpper, tempkm, tempkn, A->nb,
               A(k, k+1), ldak,
               E(k, k+1), ldak );
#if defined(CHAMELEON_USE_CUDA)
           INSERT_TASK_zlaset(
               &options,
               ChamLower, tempkm, tempkn,
               0., 1.,
               E(k, k+1), ldak );
#endif
#endif

           /* RIGHT and LEFT on the symmetric diagonal block */
           INSERT_TASK_zherfb(
               &options,
               ChamUpper,
               tempkn, tempkn, ib, A->nb,
               E(k, k+1), ldak,
               T(k, k+1), T->mb,
               D(k+1),    ldak1);

           /* LEFT on the remaining tiles until the left side */
           for (n = k+2; n < A->nt ; n++) {
               tempnn = n == A->nt-1 ? A->n-n*A->nb : A->nb;
               INSERT_TASK_zunmlq(
                   &options,
                   ChamLeft, ChamNoTrans,
                   A->mb, tempnn, tempkn, ib, A->nb,
                   E(k,   k+1), ldak,
                   T(k,   k+1), T->mb,
                   A(k+1, n  ), ldak1);
           }

           for (n = k+2; n < A->nt; n++) {
               tempnn = n == A->nt-1 ? A->n-n*A->nb : A->nb;
               ldan = BLKLDD(A, n);
               options.priority = 1;
               INSERT_TASK_ztslqt(
                   &options,
                   A->mb, tempnn, ib, A->nb,
                   A(k, k+1), ldak,
                   A(k, n  ), ldak,
                   T(k, n  ), T->mb);
               options.priority = 0;

               /* RIGHT */
               for (i = k+2; i < n; i++) {
                   ldai = BLKLDD(A, i);
                   INSERT_TASK_ztsmlq_hetra1(
                       &options,
                       ChamRight, ChamConjTrans,
                       A->mb, A->nb, A->nb, tempnn, A->nb, ib, A->nb,
                       A(k+1, i), ldak1,
                       A(i,   n), ldai,
                       A(k,   n), ldak,
                       T(k,   n), T->mb);
               }

               /* LEFT */
               for (j = n+1; j < A->nt ; j++) {
                   tempjj = j == A->nt-1 ? A->n-j*A->nb : A->nb;
                   INSERT_TASK_ztsmlq(
                       &options,
                       ChamLeft, ChamNoTrans,
                       A->nb, tempjj, tempnn, tempjj, A->nb, ib, A->nb,
                       A(k+1, j), ldak1,
                       A(n,   j), ldan,
                       A(k,   n), ldak,
                       T(k,   n), T->mb);
               }

               /* RIGHT->LEFT */
               options.priority = 1;
               /**
                * Compute the update of the following:
                *
                *     A1   A2
                *     A2'  A3
                *
                * with A1 and A3 two diagonal tiles. This is the tsmqr_corner
                * from plasma split in 4 tasks
                */
               /*  Copy the transpose of A2: AT(k) <- A2' */
               INSERT_TASK_zlatro(
                   &options,
                   ChamUpperLower, ChamConjTrans,
                   A->mb, tempnn, A->nb,
                   A(k+1, n), ldak1,
                   AT(n),     A->mb);

               /*  Right application on | A1 A2 | */
               INSERT_TASK_ztsmlq(
                   &options,
                   ChamRight, ChamConjTrans,
                   A->mb, A->nb, A->mb, tempnn, A->nb, ib, A->nb,
                   D(k+1),    ldak1,
                   A(k+1, n), ldak1,
                   A(k,   n), ldak,
                   T(k,   n), T->mb);

               /*  Right application on | A2' A3 | */
               INSERT_TASK_ztsmlq(
                   &options,
                   ChamRight, ChamConjTrans,
                   tempnn, A->nb, tempnn, tempnn, A->nb, ib, A->nb,
                   AT(n),    A->mb,
                   D(n),     ldan,
                   A(k,  n), ldak,
                   T(k,  n), T->mb);

               /*  Left application on |A1 | */
               /*                      |A2'| */
               INSERT_TASK_ztsmlq(
                   &options,
                   ChamLeft, ChamNoTrans,
                   A->mb, A->nb, tempnn, A->nb, A->nb, ib, A->nb,
                   D(k+1),  ldak1,
                   AT(n),   A->mb,
                   A(k, n), ldak,
                   T(k, n), T->mb);

               /*  Left application on | A2 | */
               /*                      | A3 | */
               INSERT_TASK_ztsmlq(
                   &options,
                   ChamLeft, ChamNoTrans,
                   A->mb, tempnn, tempnn, tempnn, A->nb, ib, A->nb,
                   A(k+1, n), ldak1,
                   D(n),      ldan,
                   A(k,   n), ldak,
                   T(k,   n), T->mb);
           }
           options.priority = 0;

           RUNTIME_iteration_pop(chamctxt);
       }
    }

    /* Copy-back into A */
    for (k = 1; k < A->nt; k++){
        tempkn = k == A->nt-1 ? A->n-k*A->nb : A->nb;
        ldak = BLKLDD(A, k);
        INSERT_TASK_zlacpy(&options,
                          uplo,
                          tempkn, tempkn, ldak,
                          D(k), ldak,
                          A(k, k), ldak);
    }


    RUNTIME_options_ws_free(&options);
    RUNTIME_options_finalize(&options, chamctxt);

    CHAMELEON_Sequence_Wait(sequence);
    chameleon_desc_mat_free(D);
    free(D);

    chameleon_desc_mat_free(AT);
    free(AT);

    (void)E;
}
