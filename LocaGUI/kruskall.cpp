#include "kruskall.h"

/******************************************************************/
/*                                                                */
/* kruskal_wallis : Fonction de calcul du test de Kruskal-Wallis. */
/*                                                                */
/******************************************************************/
int InsermLibrary::kruskal_wallis(float **tab_test, int nb_group, int *nb_sample_per_group, double *H, double *p, int ties_correction)/* Flag de correction (1) ou non (0) en fonction des ex-aequos. */
{
	int nb_sample_tot, i, j, k;
	double *tab_val, *tab_rank, *tab_group, *tie, *tab_sum;
	double tmp_val, tmp_group, tmp_tie;
	int nb_ties = 0;
	double coef_tie;
	double correct_T, n_cube;
	double moy[2];
	int signe = 1;

	/* Calcul des moyennes pour chaque groupe si 2 conditions */
	/* pour determination du signe.                           */
	/*--------------------------------------------------------*/
	if (nb_group == 2)
	{
		for (j = 0; j<2; j++)
		{
			moy[j] = 0.0;
			for (i = 0; i<nb_sample_per_group[j]; i++)
			{
				moy[j] += (double)tab_test[j][i];
			}
			moy[j] /= (double)nb_sample_per_group[j];
		}
		if (moy[0]<moy[1])
		{
			signe = -1;
		} /* Negatif = condition2 > condition 1. */
	}

	/* Calcul du nombre d'evenements total. */
	/*--------------------------------------*/
	nb_sample_tot = 0;
	for (i = 0; i<nb_group; i++)
	{
		nb_sample_tot += nb_sample_per_group[i];
	}

	/* Allocations memoire pour classement des valeurs. */
	/*--------------------------------------------------*/
	tab_val = (double *)calloc(nb_sample_tot, sizeof(double));
	if (tab_val == NULL)
	{
		fprintf(stderr, "Memory allocation error in Kruskal-Wallis test.\n");
		return(-1);
	}
	tab_rank = (double *)calloc(nb_sample_tot, sizeof(double));
	if (tab_rank == NULL)
	{
		fprintf(stderr, "Memory allocation error in Kruskal-Wallis test.\n");
		return(-1);
	}
	tab_group = (double *)calloc(nb_sample_tot, sizeof(double));
	if (tab_group == NULL)
	{
		fprintf(stderr, "Memory allocation error in Kruskal-Wallis test.\n");
		return(-1);
	}
	tie = (double *)calloc(nb_sample_tot, sizeof(double));
	if (tie == NULL)
	{
		fprintf(stderr, "Memory allocation error in Kruskal-Wallis test.\n");
		return(-1);
	}
	tab_sum = (double *)calloc(nb_group, sizeof(double));
	if (tab_sum == NULL)
	{
		fprintf(stderr, "Memory allocation error in Kruskal-Wallis test.\n");
		return(-1);
	}


	/* Initialisation tableaux. */
	/*--------------------------*/
	k = 0;
	for (i = 0; i<nb_group; i++)
	{
		for (j = 0; j<nb_sample_per_group[i]; j++)
		{
			tab_val[k] = (double)tab_test[i][j];
			tab_rank[k] = (double)(k + 1);
			tab_group[k] = (double)i;
			k++;
		}
	}

	/* Classement des valeurs. */
	/*-------------------------*/
	for (i = 1; i<nb_sample_tot; i++)
	{
		j = i - 1;
		if (tab_val[i] <= tab_val[j])
		{
			while ((j>0) && (tab_val[i] <= tab_val[j]))
			{
				j--;
			}

			if (tab_val[i] > tab_val[j]) j++;

			tmp_val = tab_val[i];
			tmp_group = tab_group[i];

			/* Egalite. */
			if (tab_val[i] == tab_val[j])
			{
				tie[i] = tie[j] = (double)1.0;
			}
			tmp_tie = tie[i];

			for (k = i; k>j; k--)
			{
				tab_val[k] = tab_val[k - 1];
				tab_rank[k] = (double)(k + 1);
				tab_group[k] = tab_group[k - 1];
				tie[k] = tie[k - 1];
			}

			tab_val[j] = tmp_val;
			tab_rank[j] = (double)j + 1;
			tab_group[j] = tmp_group;
			tie[j] = tmp_tie;
		}
	}

	/* Gestion des ex-aequo. */
	/*-----------------------*/
	correct_T = (double)0.0;
	for (i = 0; i<nb_sample_tot; i++)
	{
		if (tie[i] == (double)1.0)
		{
			j = i + 1;
			nb_ties = 1;
			while ((j<nb_sample_tot) && (tie[j] == (double)1.0) && (tab_val[j] == tab_val[i]))
			{
				j++;
				nb_ties++;
			}
			coef_tie = (double)0.0;
			for (k = i; k<j; k++)
			{
				coef_tie += tab_rank[k];
			}
			for (k = i; k<j; k++)
			{
				tab_rank[k] = coef_tie / (double)nb_ties;
			}
			i = j - 1;

			/* Correction des ex-aequos. */
			/*---------------------------*/
			correct_T += ((double)nb_ties*(double)nb_ties*(double)nb_ties) - (double)nb_ties;

			if (ties_correction == 0)
			{
				fprintf(stderr, "Warning : %d ties found in Kruskal-Wallis test (no correction).\n", nb_ties);
			}
		}
	}

	/* Calcul de la somme des rangs de chaque groupe. */
	/*------------------------------------------------*/
	for (i = 0; i<nb_sample_tot; i++)
	{
		tab_sum[(int)tab_group[i]] += tab_rank[i];
	}

	/* Calcul de la valeur H. */
	/*------------------------*/
	*H = (double)0.0;
	for (i = 0; i<nb_group; i++)
	{
		*H += (tab_sum[i] * tab_sum[i] / (double)nb_sample_per_group[i]);
	}

	*H *= (double)12.0 / ((double)nb_sample_tot*((double)nb_sample_tot + (double)1.0));
	*H -= (double)3.0*((double)nb_sample_tot + (double)1.0);


	/* Correction en tenant compte des ex-aequos. */
	/*--------------------------------------------*/
	if (ties_correction == 1)
	{
		n_cube = ((double)nb_sample_tot*(double)nb_sample_tot*(double)nb_sample_tot) - (double)nb_sample_tot;
		if (fabs(n_cube - correct_T)>0.0001) /* Pas tous a la meme valeur. */
		{
			*H /= (double)((double)1.0 - (correct_T / (double)n_cube));
		}
		else
		{
			*H = (double)0.0;
		}
	}


	/* Calcul de la valeur p. */
	/*------------------------*/
	if (fabs(*H)<0.0000001) *H = (double)0.0;
	*p = pchisq(*H, (nb_group - 1));
	*p = (double)1.0 - *p;

	/* Ajout du signe dans le cas de 2 groupes. */
	/*------------------------------------------*/
	if (nb_group == 2)
	{
		*H *= (double)signe;
	}

	/* DEBUG affichage des valeurs rangees. */
	/*
	for (i=0; i<nb_sample_tot; i++)
	{
	printf("%f  %f  %f %f\n", tab_val[i], tab_group[i], tab_rank[i], tie[i]);
	}
	printf("H=%f  p=%f\n", *H, *p);
	*/

	/* Desallocations memoire. */
	/*-------------------------*/
	free(tab_val);
	free(tab_rank);
	free(tab_group);
	free(tie);
	free(tab_sum);

	return(1);
}

/* returns probability */
double InsermLibrary::pchisq(double c2, int n)/* Chi-Square value */ /* degrees of freedom */
{
	//extern SysConst CSLmath;
	//char *CSLmsg = "pchisq (st091)";
	double p, a; int iter;

	iter = MAXITER;
	a = 0.5 * (double)(n);
	c2 *= 0.5;
	p = gammap(c2, a, ETA, &iter);
	if (iter >= MAXITER - 1)
	{
		//CSLmath.CSLcode = -4; /* this should not happen */
		//if (CSLmath.LogFlag > 0) CSLlog(CSLmsg);
		p = -1;
	}
	return(p);
}

/*incomplete gamma, using series and continued fraction*/
double InsermLibrary::gammap(double x, double a, double tol, int *itmax)
{
	double gp;
	if (x < 0.0 || a <= 0.0) return(-1.0);
	if (x <= EPS) return(0.0);
	if (x < a + 1.0) gp = gampsr(x, a, tol, itmax);
	else {
		gp = gamqcf(x, a, tol, itmax);
		gp = 1.0 - gp;
	}
	return(gp);
}

/*incomplete gamma, using series*/
double InsermLibrary::gampsr(double x, double a, double tol, int *itmax)
{
	int i, apogee;
	double gln, ap, sum, del, t1, t2;
	if (x < 0.0) return(-1.0);
	if (x < EPS) return(0.0);
	gln = alogam(a);
	ap = a;
	sum = 1.0 / a;
	del = sum;
	apogee = *itmax;
	for (i = 1; i <= apogee; i++) {
		ap += 1.0;
		del *= x / ap;
		sum += del;
		t1 = absf(del); t2 = absf(sum);
		if (t1 < t2*tol) break;
	}
	*itmax = i;
	t1 = a*log(x);
	t1 -= (x + gln);
	t2 = SafeExp(t1);
	return(sum*t2);
}

/*compliment of incomplete gamma, continued fraction*/
double InsermLibrary::gamqcf(double x, double a, double tol, int *itmax)
{
	int i, apogee;
	double glog, q, r0, r1, t0, t1, u, di, ima, q0, au, temp;
	q0 = 0.0;
	if (x < 0.0) return(-1.0);
	if (x <= EPS) return(0.0);
	glog = alogam(a);
	q = 0.0;
	r0 = 1.0;
	r1 = x;
	t0 = 0.0;
	t1 = 1.0;
	u = 1.0;
	apogee = *itmax;
	for (i = 1; i <= apogee; i++) {
		di = (double)((double)(i));
		ima = di - a;
		r0 = (r1 + r0*ima)*u;
		t0 = (t1 + t0*ima)*u;
		au = di*u;
		r1 *= au;
		r1 += x*r0;
		t1 *= au;
		t1 += x*t0;
		if (r1 != 0.0) {
			u = 1.0 / r1;
			q0 = t1*u;
			temp = (q0 - q) / q0;
			temp = absf(temp);
			if (temp < tol) break;
			q = q0;
		}
	}
	*itmax = i;
	temp = log(x);
	temp *= a;
	temp -= (x + glog);
	q = SafeExp(temp);
	return(q0*q);
}

/*exp checking for over or underflow*/
double InsermLibrary::SafeExp(double x) /* returns exp(x) */
{
	double y;

	y = exp(x);
	if (y <= 0.0 || y >= DBL_MAX) /* underflow or overflow */
	{
		y = 0.0;
	}
	return(y); /* returns 0.0 on underflow and HUGE_VAL or overflow */
}

/*return natural logarithm of gamma*/
double InsermLibrary::alogam(double x)
{
	double f, y, z;
	if (x <= 0.0)
	{
		x = 0.0;
	}
	y = x; f = 0.0; if (y >= 7.0) goto L30; f = y;
L10:
	y += 1.0; if (y >= 7.0) goto L20; f *= y; goto L10;
L20:
	f = -log(f);
L30:
	z = 1.0 / (y*y); f += (((a3 - a2*z)*z - a4)*z + a5) / y;
	f += (y - 0.5)*log(y); f += a1 - y; return(f);
}

/*absolute value of double precision value*/
double InsermLibrary::absf(double x)
{
	return(((x)<0.0) ? -(x) : (x));
}