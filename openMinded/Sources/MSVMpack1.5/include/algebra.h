/*
	Linear Algebra routines -- Header file
*/
#ifndef _ALGEBRA_H
#define _ALGEBRA_H

// Get SSE definitions
#ifdef __SSE__
#include <xmmintrin.h>
#endif

// Number of elements to add to 16-byte align type a
#define ALIGNMENT(a) (128/sizeof(a) - 1) 

// Allocate a 16-byte aligned block of b elements of type a
#define MALLOC(a,b) ((a *)_mm_malloc((b + ALIGNMENT(a)) * sizeof(a),16))
#define FREE(a) _mm_free(a)

double **matrix_aligned(long nrow, long ncol);
float **matrix_f_aligned(long nrow, long ncol);
int **matrix_i_aligned(long nrow, long ncol);
short int **matrix_s_aligned(long nrow, long ncol);
unsigned char **matrix_b_aligned(long nrow, long ncol);

void free_mat_aligned(double **m);
void free_mat_f_aligned(float **m); 
void free_mat_i_aligned(int **m);
void free_mat_s_aligned(short int **m);
void free_mat_b_aligned(unsigned char **m);

double **matrix(long nrow, long ncol);
int **matrix_i(long nrow, long ncol);
float **matrix_f(long nrow, long ncol);
long **matrix_l(long nrow, long ncol);

extern double prod_scal(const double *vect1, const double *vect2, const long dim);
void trans_mat(double **mat1, double **mat2, long dim1, long dim2);
void trans_mat_float(float **mat1, float **mat2, long dim1, long dim2);
void trans_mat_int(int **mat1, int **mat2, long dim1, long dim2);
void add_mat(double **mat1, double **mat2, double **mat3, long dim1, long dim2);
void scal_mat(double coeff, double **mat1, double **mat2, long dim1, long dim2);
void mult_mat(double **mat1, double **mat2, double **mat3,
              long dim1, long dim2, long dim3);
void display_mat_l(long **mat, long dim1, long dim2, int par1);
void display_mat(double **mat, long dim1, long dim2, int par1, int par2);
void copy(double **mat1, double **mat2, long dim1, long dim2);
extern int compare(double **mat1, double **mat2, long dim1, long dim2, 
               double seuil);
void diagonalize(double **mat, long dim, double *diagonal);

#endif /* _ALGEBRA_H */
