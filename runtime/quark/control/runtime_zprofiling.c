/**
 *
 * @file quark/runtime_zprofiling.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2019 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon Quark CHAMELEON_Complex64_t kernel progiling
 *
 * @version 0.9.2
 * @author Mathieu Faverge
 * @author Cedric Augonnet
 * @author Cedric Castagnede
 * @date 2015-05-22
 * @precisions normal z -> s d c
 *
 */
#include "chameleon_quark.h"

void RUNTIME_zdisplay_allprofile()
{
    chameleon_warning("RUNTIME_zdisplay_allprofile(quark)", "Profiling is not available with Quark");
}

void RUNTIME_zdisplay_oneprofile( cham_tasktype_t kernel )
{
    (void)kernel;
    chameleon_warning("RUNTIME_zdisplay_oneprofile(quark)", "Profiling is not available with Quark\n");
}

