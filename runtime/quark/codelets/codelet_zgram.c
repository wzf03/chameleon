/**
 *
 * @file quark/codelet_zgram.c
 *
 * @copyright 2012-2019 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon zgram Quark codelet
 *
 * @version 0.9.2
 * @author Mathieu Faverge
 * @author Florent Pruvost
 * @date 2019-04-16
 * @precisions normal z -> c d s
 *
 */
#include "chameleon_quark.h"
#include "chameleon/tasks_z.h"
#include "coreblas/coreblas_z.h"

void CORE_zgram_quark(Quark *quark)
{
    cham_uplo_t uplo;
    int m, n, mt, nt;
    double *Di;
    int lddi;
    double *Dj;
    int lddj;
    double *D;
    double *A;
    int lda;

    quark_unpack_args_12(quark, uplo, m, n, mt, nt, Di, lddi, Dj, lddj, D, A, lda);
    CORE_zgram( uplo,
                m, n, mt, nt,
                Di, lddi,
                Dj, lddj,
                D,
                A, lda);
}

void INSERT_TASK_zgram( const RUNTIME_option_t *options,
                        cham_uplo_t uplo,
                        int m, int n, int mt, int nt,
                        const CHAM_desc_t *Di, int Dim, int Din, int lddi,
                        const CHAM_desc_t *Dj, int Djm, int Djn, int lddj,
                        const CHAM_desc_t *D, int Dm, int Dn,
                        CHAM_desc_t *A, int Am, int An, int lda)
{
    quark_option_t *opt = (quark_option_t*)(options->schedopt);
    DAG_CORE_GRAM;
    QUARK_Insert_Task(opt->quark, CORE_zgram_quark, (Quark_Task_Flags*)opt,
                      sizeof(int),             &uplo,      VALUE,
                      sizeof(int),             &m,         VALUE,
                      sizeof(int),             &n,         VALUE,
                      sizeof(int),             &mt,        VALUE,
                      sizeof(int),             &nt,        VALUE,
                      sizeof(double)*lddi*mt,  RTBLKADDR(Di, double, Dim, Din), INPUT,
                      sizeof(int),             &lddi,      VALUE,
                      sizeof(double)*lddj*nt,  RTBLKADDR(Dj, double, Djm, Djn), INPUT,
                      sizeof(int),             &lddj,      VALUE,
                      sizeof(double)*2,        RTBLKADDR(D, double, Dm, Dn),  INPUT,
                      sizeof(double)*mt*nt,    RTBLKADDR(A, double, Am, An),  INOUT,
                      sizeof(int),             &lda,       VALUE,
                      0);
}