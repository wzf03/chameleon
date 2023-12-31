/**
 *
 * @file openmp/codelet_ipiv_to_perm.c
 *
 * @copyright 2012-2023 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon OpenMP codelets to convert pivot to permutations
 *
 * @version 1.3.0
 * @author Mathieu Faverge
 * @author Matthieu Kuhn
 * @date 2023-08-31
 *
 */
#include "chameleon_openmp.h"
#include "chameleon/tasks.h"
#include "coreblas.h"

void INSERT_TASK_ipiv_to_perm( const RUNTIME_option_t *options,
                               int m0, int m, int k,
                               const CHAM_ipiv_t *ipivdesc, int ipivk )
{
    int *ipiv = NULL; // get pointer from ipivdesc
    int *perm = NULL; // get pointer from ipivdesc
    int *invp = NULL; // get pointer from ipivdesc

#pragma omp task firstprivate( m0, m, k ) depend( in:ipiv[0] ) depend( inout:perm[0] ) depend( inout:invp[0] )
    {
        CORE_ipiv_to_perm( m0, m, k, ipiv, perm, invp );
    }

    (void)options;
    (void)ipivk;
}
