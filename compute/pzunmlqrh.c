/**
 *
 * @file pzunmlqrh.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2016 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon zunmlqrh parallel algorithm
 *
 * @version 1.0.0
 * @comment This file has been automatically generated
 *          from Plasma 2.5.0 for CHAMELEON 1.0.0
 * @author Hatem Ltaief
 * @author Jakub Kurzak
 * @author Dulceneia Becker
 * @author Mathieu Faverge
 * @author Emmanuel Agullo
 * @author Cedric Castagnede
 * @date 2010-11-15
 * @precisions normal z -> s d c
 *
 */
#include "control/common.h"

#define A(m,n) A,  (m),  (n)
#define B(m,n) B,  (m),  (n)
#define T(m,n) T,  (m),  (n)
#define T2(m,n) T,  (m),  ((n)+A->nt)
#if defined(CHAMELEON_COPY_DIAG)
#define D(m,n) D, ((n)/BS), 0
#else
#define D(m,n) A, (m), (n)
#endif

/**
 *  Parallel application of Q using tile V - LQ factorization (reduction
 *  Householder) - dynamic scheduling
 */
void chameleon_pzunmlqrh(cham_side_t side, cham_trans_t trans,
                     CHAM_desc_t *A, CHAM_desc_t *B, CHAM_desc_t *T, CHAM_desc_t *D, int BS,
                     RUNTIME_sequence_t *sequence, RUNTIME_request_t *request)
{
    CHAM_context_t *chamctxt;
    RUNTIME_option_t options;
    size_t ws_worker = 0;
    size_t ws_host = 0;

    int k, m, n;
    int K, N, RD, lastRD;
    int ldak, ldbN, ldbm, ldbNRD;
    int tempNn, tempkm, tempnn, tempmm, tempNRDn, tempkmin;
    int ib;

    chamctxt = chameleon_context_self();
    if (sequence->status != CHAMELEON_SUCCESS)
        return;
    RUNTIME_options_init(&options, chamctxt, sequence, request);

    ib = CHAMELEON_IB;

    /*
     * zunmlq = A->nb * ib
     * ztsmlq = A->nb * ib
     * zttmlq = A->nb * ib
     */
    ws_worker = A->nb * ib;

#if defined(CHAMELEON_USE_CUDA)
    /* Worker space
     *
     * zunmlq = A->nb * ib
     * ztsmlq = 2 * A->nb * ib
     */
    ws_worker = chameleon_max( ws_worker, ib * A->nb * 2 );
#endif

    ws_worker *= sizeof(CHAMELEON_Complex64_t);
    ws_host   *= sizeof(CHAMELEON_Complex64_t);

    RUNTIME_options_ws_alloc( &options, ws_worker, ws_host );

    K = chameleon_min(A->mt, A->nt);
    if (side == ChamLeft ) {
        if (trans == ChamNoTrans) {
            /*
             *  ChamLeft / ChamNoTrans
             */
            for (k = 0; k < K; k++) {
                RUNTIME_iteration_push(chamctxt, k);

                tempkm = k == A->mt-1 ? A->m-k*A->mb : A->mb;
                ldak = BLKLDD(A, k);
                for (N = k; N < A->nt; N += BS) {
                    tempNn   = N == A->nt-1 ? A->n-N*A->nb : A->nb;
                    tempkmin = chameleon_min(tempkm,tempNn);
                    ldbN = BLKLDD(B, N);
#if defined(CHAMELEON_COPY_DIAG)
                    INSERT_TASK_zlacpy(
                        &options,
                        ChamUpper, tempkmin, tempNn, A->nb,
                        A(k, N), ldak,
                        D(k, N), ldak );
#if defined(CHAMELEON_USE_CUDA)
                    INSERT_TASK_zlaset(
                        &options,
                        ChamLower, tempkmin, tempNn,
                        0., 1.,
                        D(k, N), ldak );
#endif
#endif
                    for (n = 0; n < B->nt; n++) {
                        tempnn = n == B->nt-1 ? B->n-n*B->nb : B->nb;
                        INSERT_TASK_zunmlq(
                            &options,
                            side, trans,
                            tempNn, tempnn,
                            tempkmin, ib, T->nb,
                            D(k, N), ldak,
                            T(k, N), T->mb,
                            B(N, n), ldbN);
                    }
                    RUNTIME_data_flush( sequence, D(k, N) );
                    RUNTIME_data_flush( sequence, T(k, N) );

                    for (m = N+1; m < chameleon_min(N+BS, A->nt); m++) {
                        tempmm = m == B->mt-1 ? B->m-m*B->mb : B->mb;
                        ldbm = BLKLDD(B, m);
                        for (n = 0; n < B->nt; n++) {
                            tempnn = n == B->nt-1 ? B->n-n*B->nb : B->nb;

                            RUNTIME_data_migrate( sequence, B(N, n),
                                                  B->get_rankof( B, m, n ) );
                            RUNTIME_data_migrate( sequence, B(m, n),
                                                  B->get_rankof( B, m, n ) );

                            /* TS kernel */
                            INSERT_TASK_ztpmlqt(
                                &options, side, trans,
                                tempmm, tempnn, tempkm, 0, ib, T->nb,
                                A(k, m), ldak,
                                T(k, m), T->mb,
                                B(N, n), ldbN,
                                B(m, n), ldbm);
                        }
                        RUNTIME_data_flush( sequence, A(k, m) );
                        RUNTIME_data_flush( sequence, T(k, m) );
                    }
                }
                for (RD = BS; RD < A->nt-k; RD *= 2) {
                    for (N = k; N+RD < A->nt; N += 2*RD) {
                        tempNRDn = N+RD == A->nt-1 ? A->n-(N+RD)*A->nb : A->nb;
                        ldbN   = BLKLDD(B, N   );
                        ldbNRD = BLKLDD(B, N+RD);
                        for (n = 0; n < B->nt; n++) {
                            tempnn = n == B->nt-1 ? B->n-n*B->nb : B->nb;

                            RUNTIME_data_migrate( sequence, B(N, n),
                                                  B->get_rankof( B, N+RD, n ) );
                            RUNTIME_data_migrate( sequence, B(N+RD, n),
                                                  B->get_rankof( B, N+RD, n ) );

                            /* TT kernel */
                            INSERT_TASK_ztpmlqt(
                                &options,
                                side, trans,
                                tempNRDn, tempnn, tempkm, tempnn, ib, T->nb,
                                A (k, N+RD), ldak,
                                T2(k, N+RD), T->mb,
                                B (N,    n), ldbN,
                                B (N+RD, n), ldbNRD);
                        }
                        RUNTIME_data_flush( sequence, A (k, N+RD) );
                        RUNTIME_data_flush( sequence, T2(k, N+RD) );
                    }
                }

                /* Restore the original location of the tiles */
                for (n = 0; n < B->nt; n++) {
                    RUNTIME_data_migrate( sequence, B(k, n),
                                          B->get_rankof( B, k, n ) );
                }

                RUNTIME_iteration_pop(chamctxt);
            }
        } else {
            /*
             *  ChamLeft / ChamConjTrans
             */
            for (k = K-1; k >= 0; k--) {
                RUNTIME_iteration_push(chamctxt, k);

                tempkm = k == A->mt-1 ? A->m-k*A->mb : A->mb;
                ldak = BLKLDD(A, k);
                lastRD = 0;
                for (RD = BS; RD < A->nt-k; RD *= 2)
                    lastRD = RD;
                for (RD = lastRD; RD >= BS; RD /= 2) {
                    for (N = k; N+RD < A->nt; N += 2*RD) {
                        tempNRDn = N+RD == A->nt-1 ? A->n-(N+RD)*A->nb : A->nb;
                        ldbN   = BLKLDD(B, N   );
                        ldbNRD = BLKLDD(B, N+RD);
                        for (n = 0; n < B->nt; n++) {
                            tempnn = n == B->nt-1 ? B->n-n*B->nb : B->nb;

                            RUNTIME_data_migrate( sequence, B(N, n),
                                                  B->get_rankof( B, N+RD, n ) );
                            RUNTIME_data_migrate( sequence, B(N+RD, n),
                                                  B->get_rankof( B, N+RD, n ) );

                            /* TT kernel */
                            INSERT_TASK_ztpmlqt(
                                &options,
                                side, trans,
                                tempNRDn, tempnn, tempkm, tempnn, ib, T->nb,
                                A (k, N+RD), ldak,
                                T2(k, N+RD), T->mb,
                                B (N,    n), ldbN,
                                B (N+RD, n), ldbNRD);
                        }
                        RUNTIME_data_flush( sequence, A (k, N+RD) );
                        RUNTIME_data_flush( sequence, T2(k, N+RD) );
                    }
                }
                for (N = k; N < A->nt; N += BS) {
                    tempNn   = N == A->nt-1 ? A->n-N*A->nb : A->nb;
                    tempkmin = chameleon_min(tempkm,tempNn);
                    ldbN = BLKLDD(B, N);
                    for (m = chameleon_min(N+BS, A->nt)-1; m > N; m--) {
                        tempmm = m == B->mt-1 ? B->m-m*B->mb : B->mb;
                        ldbm = BLKLDD(B, m);
                        for (n = 0; n < B->nt; n++) {
                            tempnn = n == B->nt-1 ? B->n-n*B->nb : B->nb;

                            RUNTIME_data_migrate( sequence, B(N, n),
                                                  B->get_rankof( B, m, n ) );
                            RUNTIME_data_migrate( sequence, B(m, n),
                                                  B->get_rankof( B, m, n ) );

                            /* TS kernel */
                            INSERT_TASK_ztpmlqt(
                                &options,
                                side, trans,
                                tempmm, tempnn, tempkm, 0, ib, T->nb,
                                A(k, m), ldak,
                                T(k, m), T->mb,
                                B(N, n), ldbN,
                                B(m, n), ldbm);
                        }
                        RUNTIME_data_flush( sequence, A(k, m) );
                        RUNTIME_data_flush( sequence, T(k, m) );
                    }
#if defined(CHAMELEON_COPY_DIAG)
                    INSERT_TASK_zlacpy(
                        &options,
                        ChamUpper, tempkmin, tempNn, A->nb,
                        A(k, N), ldak,
                        D(k, N), ldak );
#if defined(CHAMELEON_USE_CUDA)
                    INSERT_TASK_zlaset(
                        &options,
                        ChamLower, tempkmin, tempNn,
                        0., 1.,
                        D(k, N), ldak );
#endif
#endif
                    for (n = 0; n < B->nt; n++) {
                        tempnn = n == B->nt-1 ? B->n-n*B->nb : B->nb;

                        RUNTIME_data_migrate( sequence, B(N, n),
                                              B->get_rankof( B, N, n ) );

                        INSERT_TASK_zunmlq(
                            &options,
                            side, trans,
                            tempNn, tempnn,
                            tempkmin, ib, T->nb,
                            D(k, N), ldak,
                            T(k, N), T->mb,
                            B(N, n), ldbN);
                    }
                    RUNTIME_data_flush( sequence, D(k, N) );
                    RUNTIME_data_flush( sequence, T(k, N) );
                }
                RUNTIME_iteration_pop(chamctxt);
            }
        }
    }
    else {
        if (trans == ChamNoTrans) {
            /*
             *  ChamRight / ChamNoTrans
             */
            for (k = K-1; k >= 0; k--) {
                RUNTIME_iteration_push(chamctxt, k);

                tempkm = k == A->mt-1 ? A->m-k*A->mb : A->mb;
                ldak = BLKLDD(A, k);
                lastRD = 0;
                for (RD = BS; RD < A->nt-k; RD *= 2)
                    lastRD = RD;
                for (RD = lastRD; RD >= BS; RD /= 2) {
                    for (N = k; N+RD < A->nt; N += 2*RD) {
                        tempNRDn = N+RD == A->nt-1 ? A->n-(N+RD)*A->nb : A->nb;
                        for (m = 0; m < B->mt; m++) {
                            ldbm   = BLKLDD(B, m);
                            tempmm = m == B->mt-1 ? B->m-m*B->mb : B->mb;

                            RUNTIME_data_migrate( sequence, B(m, N),
                                                  B->get_rankof( B, m, N+RD ) );
                            RUNTIME_data_migrate( sequence, B(m, N+RD),
                                                  B->get_rankof( B, m, N+RD ) );

                            /* TT kernel */
                            INSERT_TASK_ztpmlqt(
                                &options,
                                side, trans,
                                tempmm, tempNRDn, tempkm, tempNRDn, ib, T->nb,
                                A (k, N+RD), ldak,
                                T2(k, N+RD), T->mb,
                                B (m, N   ), ldbm,
                                B (m, N+RD), ldbm);
                        }
                        RUNTIME_data_flush( sequence, A (k, N+RD) );
                        RUNTIME_data_flush( sequence, T2(k, N+RD) );
                    }
                }
                for (N = k; N < A->nt; N += BS) {
                    tempNn   = N == A->nt-1 ? A->n-N*A->nb : A->nb;
                    tempkmin = chameleon_min(tempkm,tempNn);
                    for (n = chameleon_min(N+BS, A->nt)-1; n > N; n--) {
                        tempnn = n == B->nt-1 ? B->n-n*B->nb : B->nb;
                        for (m = 0; m < B->mt; m++) {
                            tempmm = m == B->mt-1 ? B->m-m*B->mb : B->mb;
                            ldbm = BLKLDD(B, m);

                            RUNTIME_data_migrate( sequence, B(m, N),
                                                  B->get_rankof( B, m, n ) );
                            RUNTIME_data_migrate( sequence, B(m, m),
                                                  B->get_rankof( B, m, n ) );

                            /* TS kernel */
                            INSERT_TASK_ztpmlqt(
                                &options,
                                side, trans,
                                tempmm, tempnn, tempkm, 0, ib, T->nb,
                                A(k, n), ldak,
                                T(k, n), T->mb,
                                B(m, N), ldbm,
                                B(m, n), ldbm);
                        }
                        RUNTIME_data_flush( sequence, A(k, n) );
                        RUNTIME_data_flush( sequence, T(k, n) );
                    }
#if defined(CHAMELEON_COPY_DIAG)
                    INSERT_TASK_zlacpy(
                        &options,
                        ChamUpper, tempkmin, tempNn, A->nb,
                        A(k, N), ldak,
                        D(k, N), ldak );
#if defined(CHAMELEON_USE_CUDA)
                    INSERT_TASK_zlaset(
                        &options,
                        ChamLower, tempkmin, tempNn,
                        0., 1.,
                        D(k, N), ldak );
#endif
#endif
                    for (m = 0; m < B->mt; m++) {
                        tempmm = m == B->mt-1 ? B->m-m*B->mb : B->mb;
                        ldbm = BLKLDD(B, m);

                        RUNTIME_data_migrate( sequence, B(m, N),
                                              B->get_rankof( B, m, N ) );

                        INSERT_TASK_zunmlq(
                            &options,
                            side, trans,
                            tempmm, tempNn,
                            tempkmin, ib, T->nb,
                            D(k, N), ldak,
                            T(k, N), T->mb,
                            B(m, N), ldbm);
                    }
                    RUNTIME_data_flush( sequence, D(k, N) );
                    RUNTIME_data_flush( sequence, T(k, N) );
                }

                RUNTIME_iteration_pop(chamctxt);
            }
        } else {
            /*
             *  ChamRight / ChamConjTrans
             */
            for (k = 0; k < K; k++) {
                RUNTIME_iteration_push(chamctxt, k);

                tempkm = k == A->mt-1 ? A->m-k*A->mb : A->mb;
                ldak = BLKLDD(A, k);
                for (N = k; N < A->nt; N += BS) {
                    tempNn = N == A->nt-1 ? A->n-N*A->nb : A->nb;
                    tempkmin = chameleon_min(tempkm,tempNn);
#if defined(CHAMELEON_COPY_DIAG)
                    INSERT_TASK_zlacpy(
                        &options,
                        ChamUpper, tempkmin, tempNn, A->nb,
                        A(k, N), ldak,
                        D(k, N), ldak );
#if defined(CHAMELEON_USE_CUDA)
                    INSERT_TASK_zlaset(
                        &options,
                        ChamLower, tempkmin, tempNn,
                        0., 1.,
                        D(k, N), ldak );
#endif
#endif
                    for (m = 0; m < B->mt; m++) {
                        ldbm = BLKLDD(B, m);
                        tempmm = m == B->mt-1 ? B->m-m*B->mb : B->mb;
                        INSERT_TASK_zunmlq(
                            &options,
                            side, trans,
                            tempmm, tempNn,
                            tempkmin, ib, T->nb,
                            D(k, N), ldak,
                            T(k, N), T->mb,
                            B(m, N), ldbm);
                    }
                    RUNTIME_data_flush( sequence, D(k, N) );
                    RUNTIME_data_flush( sequence, T(k, N) );

                    for (n = N+1; n < chameleon_min(N+BS, A->nt); n++) {
                        tempnn = n == B->nt-1 ? B->n-n*B->nb : B->nb;
                        for (m = 0; m < B->mt; m++) {
                            tempmm = m == B->mt-1 ? B->m-m*B->mb : B->mb;
                            ldbm = BLKLDD(B, m);

                            RUNTIME_data_migrate( sequence, B(m, N),
                                                  B->get_rankof( B, m, n ) );
                            RUNTIME_data_migrate( sequence, B(m, n),
                                                  B->get_rankof( B, m, n ) );

                            /* TS kernel */
                            INSERT_TASK_ztpmlqt(
                                &options,
                                side, trans,
                                tempmm, tempnn, tempkm, 0, ib, T->nb,
                                A(k, n), ldak,
                                T(k, n), T->mb,
                                B(m, N), ldbm,
                                B(m, n), ldbm);
                        }
                        RUNTIME_data_flush( sequence, A(k, n) );
                        RUNTIME_data_flush( sequence, T(k, n) );
                    }
                }
                for (RD = BS; RD < A->nt-k; RD *= 2) {
                    for (N = k; N+RD < A->nt; N += 2*RD) {
                        tempNRDn = N+RD == A->nt-1 ? A->n-(N+RD)*A->nb : A->nb;
                        for (m = 0; m < B->mt; m++) {
                            tempmm = m == B->mt-1 ? B->m-m*B->mb : B->mb;
                            ldbm   = BLKLDD(B, m);

                            RUNTIME_data_migrate( sequence, B(m, N),
                                                  B->get_rankof( B, m, N+RD ) );
                            RUNTIME_data_migrate( sequence, B(m, N+RD),
                                                  B->get_rankof( B, m, N+RD ) );

                            /* TT kernel */
                            INSERT_TASK_ztpmlqt(
                                &options,
                                side, trans,
                                tempmm, tempNRDn, tempkm, tempNRDn, ib, T->nb,
                                A (k, N+RD), ldak,
                                T2(k, N+RD), T->mb,
                                B (m, N   ), ldbm,
                                B (m, N+RD), ldbm);
                        }
                        RUNTIME_data_flush( sequence, A (k, N+RD) );
                        RUNTIME_data_flush( sequence, T2(k, N+RD) );
                    }
                }

                /* Restore the original location of the tiles */
                for (m = 0; m < B->mt; m++) {
                    RUNTIME_data_migrate( sequence, B(m, k),
                                          B->get_rankof( B, m, k ) );
                }

                RUNTIME_iteration_pop(chamctxt);
            }
        }
    }

    RUNTIME_options_ws_free(&options);
    RUNTIME_options_finalize(&options, chamctxt);
    (void)D;
}
