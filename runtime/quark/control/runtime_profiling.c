/**
 *
 * @file quark/runtime_profiling.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2019 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon Quark profiling routines
 *
 * @version 0.9.2
 * @author Vijay Joshi
 * @author Cedric Castagnede
 * @date 2015-05-22
 *
 */
#include "chameleon_quark.h"
#include "chameleon/timer.h"

double RUNTIME_get_time(){
    return CHAMELEON_timer();
}

void RUNTIME_start_profiling()
{
    chameleon_warning("RUNTIME_start_profiling()", "FxT profiling is not available with Quark\n");
}

void RUNTIME_stop_profiling()
{
    chameleon_warning("RUNTIME_stop_profiling()", "FxT profiling is not available with Quark\n");
}

void RUNTIME_start_stats()
{
    chameleon_warning("RUNTIME_start_stats()", "pruning stats are not available with Quark\n");
}

void RUNTIME_stop_stats()
{
    chameleon_warning("RUNTIME_stop_stats()", "pruning stats are not available with Quark\n");
}

void RUNTIME_schedprofile_display(void)
{
    chameleon_warning("RUNTIME_schedprofile_display(quark)", "Scheduler profiling is not available with Quark\n");
}

void RUNTIME_kernelprofile_display(void)
{
    chameleon_warning("RUNTIME_kernelprofile_display(quark)", "Kernel profiling is not available with Quark\n");
}

/**
 *  Set iteration numbers for traces
 */
void RUNTIME_iteration_push( CHAM_context_t *chamctxt, unsigned long iteration )
{
    (void)chamctxt; (void)iteration;
    return;
}
void RUNTIME_iteration_pop( CHAM_context_t *chamctxt )
{
    (void)chamctxt;
    return;
}
