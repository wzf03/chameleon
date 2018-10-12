/**
 *
 * @file workspace.h
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2018 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon workspace header
 *
 * @version 1.0.0
 * @author Jakub Kurzak
 * @author Cedric Castagnede
 * @date 2010-11-15
 *
 */
#ifndef _CHAMELEON_WORKSPACE_H_
#define _CHAMELEON_WORKSPACE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  Internal routines
 */
int chameleon_alloc_ibnb_tile(int M, int N, cham_tasktype_t func, int type, CHAM_desc_t **desc, int p, int q);
int chameleon_alloc_ipiv(int M, int N, cham_tasktype_t func, int type, CHAM_desc_t **desc, void **IPIV, int p, int q);

#ifdef __cplusplus
}
#endif

#endif
