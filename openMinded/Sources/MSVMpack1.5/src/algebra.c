/*--------------------------------------------------------------------------*/
/*  Name           : algebra.c                                              */
/*  Version        : 1.0                                                    */
/*  Creation       : 12/22/95                                               */
/*  Last update    : 02/04/11                                               */
/*  Subject        : Additional math libraries                              */
/*  Module         : Unit including algebra functions                       */
/*  Author         : Fabien Lauer and Yann Guermeur Yann.Guermeur@loria.fr  */
/*--------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <xmmintrin.h>
#include "algebra.h"

#define true 1
#define false 0
#define sign(a,b) ((b) > 0.0 ? fabs(a) : -fabs(a))

/*
	Allocate matrices with 16-byte aligned rows
*/ 

/* Allocate a DOUBLE aligned matrix */
double **matrix_aligned(long nrow, long ncol) {

	long ind1; 
	double **m;
	m = (double **) malloc((size_t)((nrow+1)*sizeof(double*)));

	if(m == NULL)
	  {
	  printf("\nallocation failure 1 in matrix_aligned()");
	  exit(0);
	  }

	// Complete each row with this amount of empty elements:
	int fill_row_to_align = 2 - (ncol %2);

	// Allocate an aligned memory block
	m[0] = MALLOC(double, nrow*(ncol+1 + fill_row_to_align)); 
	if(m[0] == NULL)
	  {
	  printf("\nallocation failure 2 in matrix_aligned()\n");
	  exit(0);
	  }
	  
	// Align the first element of the first row accessed by m[1][1]
	m[1] = m[0] + ALIGNMENT(double);

	// Assign aligned pointers to rows
	if(fill_row_to_align != 0)
		for(ind1=2;ind1<=nrow;ind1++){
		  m[ind1] = m[ind1-1]+ncol+ fill_row_to_align;	 
		}
	else {
		for(ind1=2;ind1<=nrow;ind1++)
		  m[ind1] = m[ind1-1]+ncol;
	}
	return m;
}

/* Allocate a FLOAT aligned matrix */
float **matrix_f_aligned(long nrow, long ncol) {

	long ind1;
	float **m;
	m = (float **) malloc((size_t)((nrow+1)*sizeof(float*)));
	if(m == NULL)
	  {
	  printf("\nallocation failure 1 in matrix_f_aligned()\n");
	  exit(0);
	  }

	int fill_row_to_align = 4 - (ncol %4);

	m[0] = MALLOC(float, nrow*(ncol+1 + fill_row_to_align)); 
	m[1] = m[0] + ALIGNMENT(float);	// to align m[1][1]

	if(m[0]==NULL)
	  {
	  printf("\nallocation failure 2 in matrix_f_aligned()");
	  exit(0);
	  }

	if(fill_row_to_align != 0)
		for(ind1=2;ind1<=nrow;ind1++){
		  m[ind1] = m[ind1-1] + ncol + fill_row_to_align;	  
		}
	else {
		for(ind1=2;ind1<=nrow;ind1++)
		  m[ind1] = m[ind1-1]+ncol;
	}
	return m;

}

/* Allocate a INT aligned matrix */
int **matrix_i_aligned(long nrow, long ncol) {

	long ind1;
	int **m;
	m = (int **) malloc((size_t)((nrow+1)*sizeof(int*)));

	if(!m)
	  {
	  printf("\nallocation failure 1 in matrix_i_aligned()");
	  exit(0);
	  }

	int fill_row_to_align = 4 - (ncol %4);

	m[0] = MALLOC(int, nrow*(ncol+1 + fill_row_to_align)); 
	m[1] = m[0] + ALIGNMENT(int);	// to align m[1][1]

	if(m[0]==NULL)
	  {
	  printf("\nallocation failure 2 in matrix_i_aligned()");
	  exit(0);
	  }

	if(fill_row_to_align != 0)
		for(ind1=2;ind1<=nrow;ind1++){
		  m[ind1] = m[ind1-1] + ncol + fill_row_to_align;	  
		}
	else {
		for(ind1=2;ind1<=nrow;ind1++)
		  m[ind1] = m[ind1-1]+ncol;
	}

	return m;

}

/* Allocate a SHORT INT aligned matrix */
short int **matrix_s_aligned(long nrow, long ncol) {

	long ind1;
	short int **m;
	m = (short int **) malloc((size_t)((nrow+1)*sizeof(short int*)));

	if(!m)
	  {
	  printf("\nallocation failure 1 in matrix_s_aligned()");
	  exit(0);
	  }

	int fill_row_to_align = 8 - (ncol % 8);

	m[0] = MALLOC(short int, nrow*(ncol+1 + fill_row_to_align)); 
	m[1] = m[0] + ALIGNMENT(short int);	// to align m[1][1]

	if(m[0]==NULL)
	  {
	  printf("\nallocation failure 2 in matrix_s_aligned()");
	  exit(0);
	  }

	if(fill_row_to_align != 0)
		for(ind1=2;ind1<=nrow;ind1++){
		  m[ind1] = m[ind1-1] + ncol + fill_row_to_align;	  
		}
	else {
		for(ind1=2;ind1<=nrow;ind1++)
		  m[ind1] = m[ind1-1]+ncol;
	}

	return m;

} 

/* Allocate a BYTE (unsigned char) aligned matrix */
unsigned char **matrix_b_aligned(long nrow, long ncol) {

	long ind1;
	unsigned char **m;
	m = (unsigned char **) malloc((size_t)((nrow+1)*sizeof(unsigned char*)));

	if(!m)
	  {
	  printf("\nallocation failure 1 in matrix_b_aligned()");
	  exit(0);
	  }

	int fill_row_to_align = 16 - (ncol % 16);

	m[0] = MALLOC(unsigned char, nrow*(ncol+1 + fill_row_to_align)); 
	m[1] = m[0] + ALIGNMENT(unsigned char);	// to align m[1][1]

	if(m[0]==NULL)
	  {
	  printf("\nallocation failure 2 in matrix_b_aligned()");
	  exit(0);
	  }

	if(fill_row_to_align != 0)
		for(ind1=2;ind1<=nrow;ind1++){
		  m[ind1] = m[ind1-1] + ncol + fill_row_to_align;	  
		}
	else {
		for(ind1=2;ind1<=nrow;ind1++)
		  m[ind1] = m[ind1-1]+ncol;
	}

	return m;

} 

/* Free aligned matrices */
void free_mat_aligned(double **m) {
	FREE(m[0]);
	free(m);
}

void free_mat_f_aligned(float **m) {
	FREE(m[0]);
	free(m);
}
void free_mat_i_aligned(int **m) {
	FREE(m[0]);
	free(m);
}
void free_mat_s_aligned(short int **m) {
	FREE(m[0]);
	free(m);
}
void free_mat_b_aligned(unsigned char **m) {
	FREE(m[0]);
	free(m);
}

/*
	Unaligned alloc function
*/

long **matrix_l(long nrow, long ncol)

/* Allocate a long matrix */

{

long ind1, **m;

m = (long **) malloc((size_t)((nrow+1)*sizeof(long*)));
if(!m)
  {
  printf("\nallocation failure 1 in matrix_l()");
  exit(0);
  }

m[1] = (long *) malloc((size_t)((nrow*(ncol+1))*sizeof(long)));
if(!m[1])
  {
  printf("\nallocation failure 2 in matrix_l()");
  exit(0);
  }

for(ind1=2;ind1<=nrow;ind1++)
  m[ind1]=m[ind1-1]+ncol;

return m;

}


int **matrix_i(long nrow, long ncol)

/* Allocate a long matrix */

{

long ind1;

int **m = (int **) malloc((size_t)((nrow+1)*sizeof(int*)));
if(!m)
  {
  printf("\nallocation failure 1 in matrix_l()");
  exit(0);
  }

m[1] = (int *) malloc((size_t)((nrow*(ncol+1))*sizeof(int)));
if(!m[1])
  {
  printf("\nallocation failure 2 in matrix_l()");
  exit(0);
  }

for(ind1=2;ind1<=nrow;ind1++)
  m[ind1]=m[ind1-1]+ncol;

return m;

}

double **matrix(long nrow, long ncol)

/* Allocate a double matrix */

{

long ind1;

double **m;

m = (double **) malloc((size_t)((nrow+1)*sizeof(double*)));

if(!m)
  {
  printf("\nallocation failure 1 in matrix()");
  exit(0);
  }

m[1] = (double *) malloc((size_t)((nrow*(ncol+1))*sizeof(double)));

if(!m[1])
  {
  printf("\nallocation failure 2 in matrix()");
  exit(0);
  }

for(ind1=2;ind1<=nrow;ind1++)
  m[ind1]=m[ind1-1]+ncol;

return m;

}

float **matrix_f(long nrow, long ncol)

/* Allocate a float matrix */

{

long ind1;

float **m;

m = (float **) malloc((size_t)((nrow+1)*sizeof(float*)));
if(!m)
  {
  printf("\nallocation failure 1 in matrix_f()");
  exit(0);
  }

m[1] = (float *) malloc((size_t)((nrow*(ncol+1))*sizeof(float)));
if(!m[1])
  {
  printf("\nallocation failure 2 in matrix_f()");
  exit(0);
  }

for(ind1=2;ind1<=nrow;ind1++)
  m[ind1]=m[ind1-1]+ncol;

return m;

}

void trans_mat(double **mat1, double **mat2, long dim1, long dim2)
 
/* Transpose a matrix */
 
{
 
long ind1, ind2;

for(ind1=1; ind1<=dim1; ind1++)
  for(ind2=1; ind2<=dim2; ind2++)
    mat2[ind2][ind1] = mat1[ind1][ind2];
 
}

/* Transpose a matrix */
void trans_mat_float(float **mat1, float **mat2, long dim1, long dim2) {
	 
	long ind1, ind2;

	for(ind1=1; ind1<=dim1; ind1++)
	  for(ind2=1; ind2<=dim2; ind2++)
	    mat2[ind2][ind1] = mat1[ind1][ind2];
}

/* Transpose a matrix */
void trans_mat_int(int **mat1, int **mat2, long dim1, long dim2) {
	 
	long ind1, ind2;

	for(ind1=1; ind1<=dim1; ind1++)
	  for(ind2=1; ind2<=dim2; ind2++)
	    mat2[ind2][ind1] = mat1[ind1][ind2];
}

void add_mat(double **mat1, double **mat2, double **mat3, long dim1, long dim2)
 
/* Addition of two matrices */
 
{
 
long ind1, ind2;

for(ind1=1; ind1<=dim1; ind1++)
  for(ind2=1; ind2<=dim2; ind2++)
    mat3[ind1][ind2] = mat1[ind1][ind2] + mat2[ind1][ind2];

}
 
void scal_mat(double coeff, double **mat1, double **mat2, long dim1, long dim2)
 
/* Multiplication of a matrix by a scalar */
 
{

long ind1, ind2;

for(ind1=1; ind1<=dim1; ind1++)
  for(ind2=1; ind2<=dim2; ind2++)
    mat2[ind1][ind2] = coeff * mat1[ind1][ind2];
 
}
double prod_scal(const double *vect1, const double *vect2, const long dim)

/* Dot product of two vectors (of same dimension) */

{

long ind1;

double product = 0.0;

for(ind1=1; ind1<=dim; ind1++)
  product += vect1[ind1] * vect2[ind1];

return product;
  
}

void mult_mat(double **mat1, double **mat2, double **mat3,
              long dim1, long dim2, long dim3)
 
/* Matrix product of any dimensions (assumed to be compatible) */
 
{
 
int ind1, ind2, ind3;

for(ind1=1; ind1<=dim1; ind1++)
  for(ind2=1; ind2<=dim3; ind2++)
    {
    mat3[ind1][ind2] = 0.0;
    for(ind3=1; ind3<=dim2; ind3++)
      mat3[ind1][ind2] += mat1[ind1][ind3] * mat2[ind3][ind2];
    }
 
}


void display_mat_l(long **mat, long dim1, long dim2, int par1)
{

long ind1, ind2;

printf("\n");

for(ind1=1; ind1<=dim1; ind1++)
  {
  for(ind2=1; ind2<=dim2; ind2++)
    printf("%*ld ", par1, mat[ind1][ind2]);
  printf("\n");
  }

printf("\n");

}

void display_mat(double **mat, long dim1, long dim2, int par1, int par2)

{

long ind1, ind2;

printf("\n");

for(ind1=1; ind1<=dim1; ind1++)
  {
  for(ind2=1; ind2<=dim2; ind2++)
    printf("%*.*lf ",par1,par2,mat[ind1][ind2]);
  printf("\n");
  }

printf("\n");

}

void copy(double **mat1, double **mat2, long dim1, long dim2)

{

long ind1, ind2;

for(ind1=1; ind1<=dim1; ind1++)
  for(ind2=1; ind2<=dim2; ind2++)
    mat2[ind1][ind2] = mat1[ind1][ind2];

}

int compare(double **mat1, double **mat2, long dim1, long dim2, double seuil)

{

long ind1, ind2, id = true, end_mat = false;

ind1 = ind2 = 1;

while((id == true) && (end_mat == false))
  {
  if(fabs(mat1[ind1][ind2] - mat2[ind1][ind2]) > seuil)
    id = false;
  else
    {
    if(ind2 < dim2)
      ind2++;
    else
      {
      if(ind1 < dim1)
        {
        ind1++;
        ind2 = 1;
        }
      else
        end_mat = true;
      }
    }
  }

return id;

}

void diagonalize(double **mat, long dim, double *diagonal)

/* Diagonalize a symmetric matrix */

{

long i, j, k, l, m, iter;

double scale, s, r, p, g, f, h, hh, dd, c, b, *off_diagonal;

off_diagonal = (double *) calloc(dim+1, sizeof(double));

for(i=dim;i>=2;i--)
  {
  l=i-1;
  h=scale=0.0;
  if(l > 1)
    {
    for(k=1;k<=l;k++)
      scale += fabs(mat[i][k]);
    if(scale == 0.0)
      off_diagonal[i] = mat[i][l];
    else
      {
      for(k=1;k<=l;k++)
        {
        mat[i][k] /= scale;
        h += mat[i][k] * mat[i][k];
        }
      f = mat[i][l];
      g = (f > 0.0 ? -sqrt(h) : sqrt(h));
      off_diagonal[i] = scale * g;
      h -= f*g;
      mat[i][l] = f-g;
      f = 0.0;
      for(j=1;j<=l;j++)
        {
        mat[j][i] = mat[i][j]/h;
        g = 0.0;
        for(k=1;k<=j;k++)
          g += mat[j][k] * mat[i][k];
        for(k=j+1;k<=l;k++)
          g += mat[k][j] * mat[i][k];
        off_diagonal[j] = g/h;
        f += off_diagonal[j] * mat[i][j];
        }
      hh = f / (h+h);
      for(j=1;j<=l;j++)
        {
        f = mat[i][j];
        off_diagonal[j] = g = off_diagonal[j] - hh * f;
        for(k=1;k<=j;k++)
          mat[j][k] -= (f*off_diagonal[k]+g*mat[i][k]);
        }
      }
    }
  else
    off_diagonal[i] = mat[i][l];
  diagonal[i] = h;
  }

diagonal[1] = 0.0;
off_diagonal[1] = 0.0;
      
for(i=1;i<=dim;i++)
  {
  l=i-1;
  if(diagonal[i])
    {
    for(j=1;j<=l;j++)
      {
      g = 0.0;
      for(k=1;k<=l;k++)
        g += mat[i][k] * mat[k][j];
      for(k=1;k<=l;k++)
        mat[k][j] -= g * mat[k][i];
      }
    }
  diagonal[i] = mat[i][i];
  mat[i][i] = 1.0;
  for(j=1;j<=l;j++)
    mat[j][i] = mat[i][j] = 0.0; 
  }

for(i=2;i<=dim;i++)
  off_diagonal[i-1] = off_diagonal[i];

off_diagonal[dim] = 0.0;

for(l=1;l<=dim;l++)
  {
  iter=0;
  do
    {
    for(m=l;m<=dim-1;m++)
      {
      dd = fabs(diagonal[m]) + fabs(diagonal[m+1]);
      if((fabs(off_diagonal[m])+dd) == dd)
        break;
      }
    if(m != l)
      {
      if(iter++ == 1000)
        {
        printf("\nToo many iterations in tqli...\n\n");
        exit(0);
        }
      g = (diagonal[l+1]-diagonal[l]) / (2.0*off_diagonal[l]);
      r = sqrt(g*g + 1.0);
      g = diagonal[m]-diagonal[l]+off_diagonal[l] / (g+sign(r,g));
      s = c = 1.0;
      p = 0.0;
      for(i=m-1;i>=l;i--)
        {
        f = s*off_diagonal[i];
        b = c*off_diagonal[i];
        off_diagonal[i+1] = (r = sqrt(f*f + g*g));
        if(r == 0.0)
          {
          diagonal[i+1] -= p;
          off_diagonal[m] = 0.0;
          break;
          }
        s = f/r;
        c = g/r;
        g = diagonal[i+1]-p;
        r = (diagonal[i]-g)*s+2.0*c*b;
        diagonal[i+1] = g+(p=s*r);
        g = c*r-b;
        for(k=1;k<=dim;k++)
          {
          f=mat[k][i+1];
          mat[k][i+1] = s*mat[k][i]+c*f;
          mat[k][i] = c*mat[k][i]-s*f;
          }
        }
      if(r == 0.0 && i >= 1)
        continue;
      diagonal[l] -= p;
      off_diagonal[l] = g;
      off_diagonal[m] = 0.0;
      }
    }
    while(m != l);
  }

free(off_diagonal);

} 
