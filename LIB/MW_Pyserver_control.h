/* Copyright 2019 The MathWorks, Inc. */
#ifndef _MW_PYSERVER_CONTROL_H_
#define _MW_PYSERVER_CONTROL_H_
#if ( defined(MATLAB_MEX_FILE) || defined(RSIM_PARAMETER_LOADING) ||  defined(RSIM_WITH_SL_SOLVER) )

#else
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#endif
#include "rtwtypes.h"
#ifdef __cplusplus
extern "C"
{
#endif

#if ( defined(MATLAB_MEX_FILE) || defined(RSIM_PARAMETER_LOADING) ||  defined(RSIM_WITH_SL_SOLVER) )
#define MW_getDefines() 0
#define MW_killPyserver() 0
#define MW_launchPyserver() 0
#define MW_execmd(cmd) 0
#else
//C-code interface
void MW_getDefines();
void MW_killPyserver();
void MW_launchPyserver();
void MW_execmd(char *buff);
#endif 

#ifdef __cplusplus
}
#endif
#endif 