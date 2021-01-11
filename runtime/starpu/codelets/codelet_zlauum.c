/**
 *
 * @file starpu/codelet_zlauum.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2020 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon zlauum StarPU codelet
 *
 * @version 1.0.0
 * @comment This file has been automatically generated
 *          from Plasma 2.5.0 for CHAMELEON 0.9.2
 * @author Julien Langou
 * @author Henricus Bouwmeester
 * @author Mathieu Faverge
 * @author Emmanuel Agullo
 * @author Cedric Castagnede
 * @author Lucas Barros de Assis
 * @date 2020-03-03
 * @precisions normal z -> c d s
 *
 */
#include "chameleon_starpu.h"
#include "runtime_codelet_z.h"

#if !defined(CHAMELEON_SIMULATION)
static void cl_zlauum_cpu_func(void *descr[], void *cl_arg)
{
    cham_uplo_t uplo;
    int N;
    CHAM_tile_t *tileA;

    tileA = cti_interface_get(descr[0]);

    starpu_codelet_unpack_args(cl_arg, &uplo, &N);
    TCORE_zlauum(uplo, N, tileA);
}
#endif /* !defined(CHAMELEON_SIMULATION) */

/*
 * Codelet definition
 */
CODELETS_CPU(zlauum, cl_zlauum_cpu_func)

/**
 *
 * @ingroup INSERT_TASK_Complex64_t
 *
 */
void INSERT_TASK_zlauum( const RUNTIME_option_t *options,
                         cham_uplo_t uplo, int n, int nb,
                         const CHAM_desc_t *A, int Am, int An )
{
    (void)nb;
    struct starpu_codelet *codelet = &cl_zlauum;
    void (*callback)(void*) = options->profiling ? cl_zlauum_callback : NULL;
    starpu_option_request_t* schedopt = (starpu_option_request_t *)(options->request->schedopt);
    int workerid = (schedopt == NULL) ? -1 : schedopt->workerid;

    CHAMELEON_BEGIN_ACCESS_DECLARATION;
    CHAMELEON_ACCESS_RW(A, Am, An);
    CHAMELEON_END_ACCESS_DECLARATION;

    rt_starpu_insert_task(
        codelet,
        STARPU_VALUE,    &uplo,              sizeof(int),
        STARPU_VALUE,    &n,                 sizeof(int),
        STARPU_RW,        RTBLKADDR(A, CHAMELEON_Complex64_t, Am, An),
        STARPU_PRIORITY,  options->priority,
        STARPU_CALLBACK,  callback,
        STARPU_EXECUTE_ON_WORKER, workerid,
#if defined(CHAMELEON_CODELETS_HAVE_NAME)
        STARPU_NAME, "zlauum",
#endif
        0);

}
