#ifndef _KRUSKALL_H
#define _KRUSKALL_H

#include <iostream>	
#include "float.h"

#define MAXITER 10000
#define ETA 1.07e-308  /* Smallest number representable on computer */
#define EPS      1.0e-14                /*      a little margin */
#define a1 0.918938533204673
#define a2 0.000595238095238
#define a3 0.000793650793651
#define a4 0.002777777777778
#define a5 0.083333333333333

namespace InsermLibrary
{
	int kruskal_wallis(float **tab_test, int nb_group, int *nb_sample_per_group, double *H, double *p, int ties_correction);
	double pchisq(double c2, int n);
	double gammap(double x, double a, double tol, int *itmax);
	double gampsr(double x, double a, double tol, int *itmax);
	double gamqcf(double x, double a, double tol, int *itmax);
	double SafeExp(double x);
	double alogam(double x);
	double absf(double x);
}
#endif