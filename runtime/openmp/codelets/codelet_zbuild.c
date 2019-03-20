/**
 *
 * @file openmp/codelet_zbuild.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2019 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon zbuild StarPU codelet
 *
 * @version 0.9.2
 * @comment This file has been automatically generated
 *          from Plasma 2.5.0 for CHAMELEON 0.9.2
 * @author Piotr Luszczek
 * @author Pierre Lemarinier
 * @author Mathieu Faverge
 * @author Emmanuel Agullo
 * @author Cedric Castagnede
 * @author Guillaume Sylvand
 * @author Philippe Virouleau
 * @date 2018-06-15
 * @precisions normal z -> c d s
 *
 */
#include "chameleon_openmp.h"
#include "chameleon/tasks_z.h"
#include "coreblas/coreblas_z.h"

void INSERT_TASK_zbuild( const RUNTIME_option_t *options,
                        const CHAM_desc_t *A, int Am, int An, int lda,
                        void *user_data, void* user_build_callback )
{
  int row_min, row_max, col_min, col_max;
  row_min = Am*A->mb ;
  row_max = Am == A->mt-1 ? A->m-1 : row_min+A->mb-1 ;
  col_min = An*A->nb ;
  col_max = An == A->nt-1 ? A->n-1 : col_min+A->nb-1 ;
  CHAMELEON_Complex64_t *ptrA = RTBLKADDR(A, CHAMELEON_Complex64_t, Am, An);
  void (*callback)(int row_min, int row_max, int col_min, int col_max, void *buffer, int ld, void *user_data) ;
  callback = user_build_callback;

#pragma omp task firstprivate(row_min, row_max, col_min, col_max, ptrA, lda, user_data) depend(inout:ptrA[0])
  callback(row_min, row_max, col_min, col_max, ptrA, lda, user_data);
}
