/*--------------------------------------------------------------------------*/
/*  Name           : biblio.c                                               */
/*  Version        : 1.0                                                    */
/*  Creation       : 07/12/96                                               */
/*  Last update    : 01/01/10                                               */
/*  Subject        : Implementation of a classifier                         */
/*  Module         : Measures of the prediction accuracy                    */
/*  Author         : Yann Guermeur Yann.Guermeur@loria.fr                   */
/*--------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "algebra.h"

 
/* Display the recognition rate only */
void display_stats(long classes, long **matrice)
{ 
	long ind1, ind2;

	double nb_ex = 0.0, nb_rec = 0.0;

	for(ind1=1; ind1<=classes; ind1++)
	  for(ind2=1; ind2<=classes; ind2++)
	    {
	    nb_ex += matrice[ind1][ind2];
	    if(ind1 == ind2)
	      nb_rec += matrice[ind1][ind2];
	    }
	 
	printf("\nRecognition rate: %6.2lf \%%\n", (nb_rec*100.0)/nb_ex);
	 
}

/* Display the values of all the measures of accuracy */
void display_full_stats(long classes, long **matrice)
{

	long ind1, ind2, disp_size;

	double *nb, *pred, denominator, nb_ex=0.0, nb_rec=0.0, *C_matt, *p, *o, *n, *u;

	p = (double *) calloc(classes+1, sizeof(double));
	o = (double *) calloc(classes+1, sizeof(double));
	n = (double *) calloc(classes+1, sizeof(double));
	u = (double *) calloc(classes+1, sizeof(double));
	nb = (double *) calloc(classes+1, sizeof(double));
	pred = (double *) calloc(classes+1, sizeof(double));
	C_matt = (double *) calloc(classes+1, sizeof(double));

	for(ind1=1; ind1<=classes; ind1++)
	  for(ind2=1; ind2<=classes; ind2++)
	    {
	    nb_ex += matrice[ind1][ind2];
	    nb[ind1] += matrice[ind1][ind2];
	    pred[ind2] += matrice[ind1][ind2];
	    }

	disp_size = (long) ceil(log10(nb_ex));
	printf("\nConfusion matrix:\n");
	display_mat_l(matrice, classes, classes, disp_size);

	for(ind1=1; ind1<=classes; ind1++)
	  {
	  p[ind1] = matrice[ind1][ind1];
	  nb_rec += p[ind1];
	  }

	for(ind1=1; ind1<=classes; ind1++)
	  {
	  u[ind1] = nb[ind1] - p[ind1];
	  o[ind1] = pred[ind1] - p[ind1];
	  }

	for(ind1=1; ind1<=classes; ind1++)
	  n[ind1] = nb_ex - (pred[ind1] + u[ind1]);

	for(ind1=1; ind1<=classes; ind1++)
	  {
	  denominator = sqrt(n[ind1]+u[ind1]) * sqrt(n[ind1]+o[ind1]) *
		     sqrt(p[ind1]+u[ind1]) * sqrt(p[ind1]+o[ind1]);
	  if(denominator != 0.0)
	    C_matt[ind1] = (p[ind1] * n[ind1] - u[ind1] * o[ind1]) / denominator;
	  else
	    C_matt[ind1] = 0.0;
	  }

	printf("Recognition rate: %6.2lf \%%\n", (nb_rec*100.0)/nb_ex);

	printf("\nMatthews coefficients:\n\n");

	for(ind1=1; ind1<=classes; ind1++)
	  printf("C%ld: %c%4.2lf\n", ind1, (C_matt[ind1] >= 0) ? ' ' : '-',
		 fabs(C_matt[ind1]));

	printf("\n");

	free(p);
	free(o);
	free(n);
	free(u);
	free(nb);
	free(pred);
	free(C_matt);
	
}
