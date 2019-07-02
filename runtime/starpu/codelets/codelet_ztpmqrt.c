/**
 *
 * @file starpu/codelet_ztpmqrt.c
 *
 * @copyright 2009-2016 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2019 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 * @brief Chameleon ztpmqrt StarPU codelet
 *
 * @version 0.9.2
 * @author Mathieu Faverge
 * @author Lucas Barros de Assis
 * @date 2016-12-16
 * @precisions normal z -> s d c
 *
 */
#include "chameleon_starpu.h"
#include "runtime_codelet_z.h"

#if !defined(CHAMELEON_SIMULATION)
static void cl_ztpmqrt_cpu_func(void *descr[], void *cl_arg)
{
    cham_side_t side;
    cham_trans_t trans;
    int M;
    int N;
    int K;
    int L;
    int ib;
    const CHAMELEON_Complex64_t *V;
    int ldV;
    const CHAMELEON_Complex64_t *T;
    int ldT;
    CHAMELEON_Complex64_t *A;
    int ldA;
    CHAMELEON_Complex64_t *B;
    int ldB;
    CHAMELEON_Complex64_t *WORK;
    size_t lwork;

    V    = (const CHAMELEON_Complex64_t *)STARPU_MATRIX_GET_PTR(descr[0]);
    T    = (const CHAMELEON_Complex64_t *)STARPU_MATRIX_GET_PTR(descr[1]);
    A    = (CHAMELEON_Complex64_t *)STARPU_MATRIX_GET_PTR(descr[2]);
    B    = (CHAMELEON_Complex64_t *)STARPU_MATRIX_GET_PTR(descr[3]);
    WORK = (CHAMELEON_Complex64_t *)STARPU_MATRIX_GET_PTR(descr[4]); /* ib * nb */
    ldV = STARPU_MATRIX_GET_LD( descr[0] );
    ldT = STARPU_MATRIX_GET_LD( descr[1] );
    ldA = STARPU_MATRIX_GET_LD( descr[2] );
    ldB = STARPU_MATRIX_GET_LD( descr[3] );
    starpu_codelet_unpack_args( cl_arg, &side, &trans, &M, &N, &K, &L, &ib, &lwork );

    CORE_ztpmqrt( side, trans, M, N, K, L, ib,
                  V, ldV, T, ldT, A, ldA, B, ldB, WORK );

    (void)lwork;
}


#if defined(CHAMELEON_USE_CUDA)
static void cl_ztpmqrt_cuda_func(void *descr[], void *cl_arg)
{
    cham_side_t side;
    cham_trans_t trans;
    int M;
    int N;
    int K;
    int L;
    int ib;
    const cuDoubleComplex *V;
    int ldV;
    const cuDoubleComplex *T;
    int ldT;
    cuDoubleComplex *A;
    int ldA;
    cuDoubleComplex *B;
    int ldB;
    cuDoubleComplex *W;
    size_t lwork;

    V = (const cuDoubleComplex *)STARPU_MATRIX_GET_PTR(descr[0]);
    T = (const cuDoubleComplex *)STARPU_MATRIX_GET_PTR(descr[1]);
    A = (cuDoubleComplex *)STARPU_MATRIX_GET_PTR(descr[2]);
    B = (cuDoubleComplex *)STARPU_MATRIX_GET_PTR(descr[3]);
    W = (cuDoubleComplex *)STARPU_MATRIX_GET_PTR(descr[4]); /* 3*ib*nb */
    ldV = STARPU_MATRIX_GET_LD( descr[0] );
    ldT = STARPU_MATRIX_GET_LD( descr[1] );
    ldA = STARPU_MATRIX_GET_LD( descr[2] );
    ldB = STARPU_MATRIX_GET_LD( descr[3] );
    starpu_codelet_unpack_args( cl_arg, &side, &trans, &M, &N, &K, &L, &ib, &lwork );

    RUNTIME_getStream(stream);

    CUDA_ztpmqrt(
            side, trans, M, N, K, L, ib,
            V, ldV, T, ldT, A, ldA, B, ldB,
            W, lwork, stream );

#ifndef STARPU_CUDA_ASYNC
    cudaStreamSynchronize( stream );
#endif
}
#endif /* defined(CHAMELEON_USE_CUDA) */
#endif /* !defined(CHAMELEON_SIMULATION) */

/*
 * Codelet definition
 */
CODELETS(ztpmqrt, 5, cl_ztpmqrt_cpu_func, cl_ztpmqrt_cuda_func, STARPU_CUDA_ASYNC)

void INSERT_TASK_ztpmqrt( const RUNTIME_option_t *options,
                          cham_side_t side, cham_trans_t trans,
                          int M, int N, int K, int L, int ib, int nb,
                          const CHAM_desc_t *V, int Vm, int Vn, int ldV,
                          const CHAM_desc_t *T, int Tm, int Tn, int ldT,
                          const CHAM_desc_t *A, int Am, int An, int ldA,
                          const CHAM_desc_t *B, int Bm, int Bn, int ldB )
{
    struct starpu_codelet *codelet = &cl_ztpmqrt;
    void (*callback)(void*) = options->profiling ? cl_ztpmqrt_callback : NULL;

    CHAMELEON_BEGIN_ACCESS_DECLARATION;
    CHAMELEON_ACCESS_R(V, Vm, Vn);
    CHAMELEON_ACCESS_R(T, Tm, Tn);
    CHAMELEON_ACCESS_RW(A, Am, An);
    CHAMELEON_ACCESS_RW(B, Bm, Bn);
    CHAMELEON_END_ACCESS_DECLARATION;

    starpu_insert_task(
        starpu_mpi_codelet(codelet),
        STARPU_VALUE, &side,  sizeof(int),
        STARPU_VALUE, &trans, sizeof(int),
        STARPU_VALUE, &M,     sizeof(int),
        STARPU_VALUE, &N,     sizeof(int),
        STARPU_VALUE, &K,     sizeof(int),
        STARPU_VALUE, &L,     sizeof(int),
        STARPU_VALUE, &ib,     sizeof(int),
        STARPU_R,      RTBLKADDR(V, CHAMELEON_Complex64_t, Vm, Vn),
        STARPU_R,      RTBLKADDR(T, CHAMELEON_Complex64_t, Tm, Tn),
        STARPU_RW,     RTBLKADDR(A, CHAMELEON_Complex64_t, Am, An),
        STARPU_RW,     RTBLKADDR(B, CHAMELEON_Complex64_t, Bm, Bn),
        STARPU_VALUE, &(options->ws_wsize), sizeof(size_t),
        /* Other options */
        STARPU_SCRATCH,   options->ws_worker,
        STARPU_PRIORITY,  options->priority,
        STARPU_CALLBACK,  callback,
#if defined(CHAMELEON_USE_MPI)
        STARPU_EXECUTE_ON_NODE, B->get_rankof(B, Bm, Bn),
#endif
#if defined(CHAMELEON_CODELETS_HAVE_NAME)
        STARPU_NAME, (( L == 0 ) ? "ztsmqr" : "ztpmqrt"),
#endif
        0);
    (void)ldA;
    (void)ldT;
    (void)ldV;

    (void)ib; (void)nb;
}
