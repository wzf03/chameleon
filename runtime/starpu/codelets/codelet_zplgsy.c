/**
 *
 * @file starpu/codelet_zplgsy.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2019 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon zplgsy StarPU codelet
 *
 * @version 0.9.2
 * @comment This file has been automatically generated
 *          from Plasma 2.5.0 for CHAMELEON 0.9.2
 * @author Piotr Luszczek
 * @author Pierre Lemarinier
 * @author Mathieu Faverge
 * @author Emmanuel Agullo
 * @author Cedric Castagnede
 * @author Lucas Barros de Assis
 * @date 2014-11-16
 * @precisions normal z -> c d s
 *
 */
#include "chameleon_starpu.h"
#include "runtime_codelet_z.h"

/*   cl_zplgsy_cpu_func - Generate a tile for random symmetric (positive definite if 'bump' is large enough) matrix. */

#if !defined(CHAMELEON_SIMULATION)
static void cl_zplgsy_cpu_func(void *descr[], void *cl_arg)
{
    CHAMELEON_Complex64_t bump;
    int m;
    int n;
    CHAMELEON_Complex64_t *A;
    int ldA;
    int bigM;
    int m0;
    int n0;
    unsigned long long int seed;

    A = (CHAMELEON_Complex64_t *)STARPU_MATRIX_GET_PTR(descr[0]);
    ldA = STARPU_MATRIX_GET_LD( descr[0] );

    starpu_codelet_unpack_args(cl_arg, &bump, &m, &n, &bigM, &m0, &n0, &seed );
    CORE_zplgsy( bump, m, n, A, ldA, bigM, m0, n0, seed );
}
#endif /* !defined(CHAMELEON_SIMULATION) */

/*
 * Codelet definition
 */
CODELETS_CPU(zplgsy, 1, cl_zplgsy_cpu_func)

void INSERT_TASK_zplgsy( const RUNTIME_option_t *options,
                        CHAMELEON_Complex64_t bump, int m, int n, const CHAM_desc_t *A, int Am, int An, int ldA,
                        int bigM, int m0, int n0, unsigned long long int seed )
{

    struct starpu_codelet *codelet = &cl_zplgsy;
    void (*callback)(void*) = options->profiling ? cl_zplgsy_callback : NULL;

    CHAMELEON_BEGIN_ACCESS_DECLARATION;
    CHAMELEON_ACCESS_W(A, Am, An);
    CHAMELEON_END_ACCESS_DECLARATION;

    starpu_insert_task(
        starpu_mpi_codelet(codelet),
        STARPU_VALUE, &bump,       sizeof(CHAMELEON_Complex64_t),
        STARPU_VALUE,    &m,                      sizeof(int),
        STARPU_VALUE,    &n,                      sizeof(int),
        STARPU_W,         RTBLKADDR(A, CHAMELEON_Complex64_t, Am, An),
        STARPU_VALUE, &bigM,                      sizeof(int),
        STARPU_VALUE,   &m0,                      sizeof(int),
        STARPU_VALUE,   &n0,                      sizeof(int),
        STARPU_VALUE, &seed,   sizeof(unsigned long long int),
        STARPU_PRIORITY,    options->priority,
        STARPU_CALLBACK,    callback,
#if defined(CHAMELEON_CODELETS_HAVE_NAME)
        STARPU_NAME, "zplgsy",
#endif
        0);
    (void)ldA;
}
