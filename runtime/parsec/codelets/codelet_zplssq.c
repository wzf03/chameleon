/**
 *
 * @copyright (c) 2009-2015 The University of Tennessee and The University
 *                          of Tennessee Research Foundation.
 *                          All rights reserved.
 * @copyright (c) 2012-2015 Inria. All rights reserved.
 * @copyright (c) 2012-2015 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria, Univ. Bordeaux. All rights reserved.
 *
 **/

/**
 * @file codelet_zplssq.c
 *
 *  MORSE codelets kernel
 *  MORSE is a software package provided by Univ. of Tennessee,
 *  Univ. of California Berkeley and Univ. of Colorado Denver
 *
 * @version 2.5.0
 * @author Reazul Hoque
 * @precisions normal z -> c d s
 *
 **/
#include <math.h>
#include "runtime/parsec/include/morse_parsec.h"

/*****************************************************************************
 *
 * @ingroup CORE_MORSE_Complex64_t
 *
 *  MORSE_TASK_zplssq returns: scl * sqrt(ssq)
 *
 * with scl and ssq such that
 *
 *    ( scl**2 )*ssq = sum( A( 2*i )**2 * A( 2*i+1 ) )
 *                      i
 *
 * The values of A(2*i+1) are assumed to be at least unity.
 * The values of A(2*i) are assumed to be non-negative and scl is
 *
 *    scl = max( A( 2*i ) ),
 *           i
 *
 * The routine makes only one pass through the matrix A.
 *
 *******************************************************************************
 *
 *  @param[in] M
 *          The number of couple (scale, sumsq) in the matrix A.
 *
 *  @param[in] A
 *          The 2-by-M matrix.
 *
 *  @param[out] result
 *          On exit, result contains scl * sqrt( ssq )
 *
 */
static int
CORE_zplssq_parsec(dague_execution_unit_t *context, dague_execution_context_t *this_task)
{
    dague_data_copy_t *gSCALESUMSQ;
    dague_data_copy_t *gSCLSSQ;

    dague_dtd_unpack_args(this_task,
                          UNPACK_DATA,  &gSCALESUMSQ,
                          UNPACK_DATA,  &gSCLSSQ
                          );


    double *SCALESUMSQ = (double *) DAGUE_DATA_COPY_GET_PTR((dague_data_copy_t *)gSCALESUMSQ);
    double *SCLSSQ     = (double *) DAGUE_DATA_COPY_GET_PTR((dague_data_copy_t *)gSCLSSQ);

    if( SCLSSQ[0] < SCALESUMSQ[0] ) {
        SCLSSQ[1] = SCALESUMSQ[1] + (SCLSSQ[1]     * (( SCLSSQ[0] / SCALESUMSQ[0] ) * ( SCLSSQ[0] / SCALESUMSQ[0] )));
        SCLSSQ[0] = SCALESUMSQ[0];
    } else {
        SCLSSQ[1] = SCLSSQ[1]     + (SCALESUMSQ[1] * (( SCALESUMSQ[0] / SCLSSQ[0] ) * ( SCALESUMSQ[0] / SCLSSQ[0] )));
    }

    return 0;
}

void MORSE_TASK_zplssq( MORSE_option_t *options,
                        MORSE_desc_t *SCALESUMSQ, int SCALESUMSQm, int SCALESUMSQn,
                        MORSE_desc_t *SCLSSQ,     int SCLSSQm,     int SCLSSQn )
{
    dague_dtd_handle_t* DAGUE_dtd_handle = (dague_dtd_handle_t *)(options->sequence->schedopt);

    insert_task_generic_fptr(DAGUE_dtd_handle,      CORE_zplssq_parsec,               "plssq",
                             PASSED_BY_REF,         RTBLKADDR( SCALESUMSQ, MORSE_Complex64_t, SCALESUMSQm, SCALESUMSQn ),    INPUT | REGION_FULL,
                             PASSED_BY_REF,         RTBLKADDR( SCLSSQ, MORSE_Complex64_t, SCLSSQm, SCLSSQn ),                INOUT | REGION_FULL,
                             0);
}

static int
CORE_zplssq2_parsec(dague_execution_unit_t *context, dague_execution_context_t *this_task)
{
    dague_data_copy_t *gRESULT;

    dague_dtd_unpack_args(this_task,
                          UNPACK_DATA,  &gRESULT
                          );

    double *RESULT = DAGUE_DATA_COPY_GET_PTR((dague_data_copy_t *)gRESULT);

    RESULT[0] = RESULT[0] * sqrt( RESULT[1] );

    return 0;
}

void MORSE_TASK_zplssq2( MORSE_option_t *options,
                         MORSE_desc_t *RESULT, int RESULTm, int RESULTn )
{
    dague_dtd_handle_t* DAGUE_dtd_handle = (dague_dtd_handle_t *)(options->sequence->schedopt);

    insert_task_generic_fptr(DAGUE_dtd_handle,      CORE_zplssq2_parsec,               "plssq2",
                             PASSED_BY_REF,         RTBLKADDR( RESULT, MORSE_Complex64_t, RESULTm, RESULTn ),     INOUT | REGION_FULL,
                             0);
}