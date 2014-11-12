#include <math.h>

// Include user-defined custom kernels
#include "custom_kernels.c"
 
/*
	Common kernel functions
*/
 
#ifdef __SSE4_1__
 
/*
	Vectorized code with SSE4.1 intrinsics

	(only for Intel processors)
*/
#include <smmintrin.h>
/*
	Dot product (linear kernel function)
*/ 
double dot_double(const double *a, const double *b, const long dim) {

  double dot;

  long i;

  __m128d num1, num2, num4;

  num4= _mm_setzero_pd();  //sets sum to zero

  for(i=1; i<=dim-1; i+=2)
  {
    num1 = _mm_load_pd(a+i);   
    num2 = _mm_load_pd(b+i);   
    
    num2 = _mm_dp_pd(num1,num2,0x31); // dot product

    num4 = _mm_add_sd(num4, num2); // vertical addition (global sum)
  }
   
  _mm_store_sd(&dot,num4);

  // Epilog:
  if(dim%2 != 0) {
  	dot += a[dim]*b[dim]; 
  }

  return dot;
}

double dot_float(const float *a, const float *b, const long dim) {

  float dot;
  long i;

  __m128 num1, num2, num4;
  
  num4= _mm_setzero_ps();  //sets sum to zero
  
  for(i=1; i<=dim-3; i+=4)
  {
    num1 = _mm_load_ps(a+i);  
    num2 = _mm_load_ps(b+i);  

    num2 = _mm_dp_ps(num1,num2,0xf1); // dot product

    num4 = _mm_add_ss(num4, num2);  // vertical addition    
  }
  _mm_store_ss(&dot,num4);
  
  int epilog = dim%4;	
  switch (epilog) {	
	case 1:
		dot += (a[dim]*b[dim]);
		break;
	case 2:
		dot += (a[dim]*b[dim]) + (a[dim-1]*b[dim-1]);
		break;
	case 3:
		dot += (a[dim]*b[dim]) + (a[dim-1]*b[dim-1]) + (a[dim-2]*b[dim-2]); 
		break;	
	default:		
		break;
	}
 return (double)dot;
}


/*  
	Gaussian RBF kernel
	k(x,z) = exp(-||x - z||^2 / 2sigma^2)
*/
double rbf_double(const double *a, const double *b, const long dim, const double sigma) {

  double ker_val;

  long i;

  __m128d num1, num2, num3, num4;

  num4= _mm_setzero_pd();  //sets sum to zero

  for(i=1; i<=dim-1; i+=2)
  {
    num1 = _mm_load_pd(a+i);   
    num2 = _mm_load_pd(b+i);   

    num3 = _mm_sub_pd(num1, num2); // substraction

    num3 = _mm_dp_pd(num3,num3,0x31); // dot product

    num4 = _mm_sub_sd(num4, num3);  // vertical addition
  }
   
  _mm_store_sd(&ker_val,num4);

  // Epilog:
  if(dim%2 != 0) {
  	ker_val -= (a[dim]-b[dim])*(a[dim]-b[dim]); 
  }

  return exp(ker_val / (2 * sigma * sigma));
}

double rbf_float(const float *a, const float *b, const long dim, const double sigma) {

  float ker_val;
  long i;

  __m128 num1, num2, num3, num4;
  
  num4= _mm_setzero_ps();  //sets sum to zero
  
  for(i=1; i<=dim-3; i+=4)
  {
    num1 = _mm_load_ps(a+i);   //loads aligned array a into num1  num1= a[3]  a[2]  a[1]  a[0]
    num2 = _mm_load_ps(b+i);   //loads aligned array b into num2  num2= b[3]   b[2]   b[1]  b[0]

    num3 = _mm_sub_ps(num1, num2); // parallel substraction num3 = num1 - num2
                           
    num3 = _mm_dp_ps(num3,num3,0xf1); // dot product: num3[0] = num3[0]^2 + ... + num3[4]^2

    num4 = _mm_sub_ss(num4, num3);  // vertical addition    
  }
  _mm_store_ss(&ker_val,num4);
  
  // Epilog
  int epilog = dim%4;	
  switch (epilog) {
	
	case 1:
		ker_val -= (a[dim]-b[dim])*(a[dim]-b[dim]); 
		break;
	case 2:
		ker_val -= (a[dim]-b[dim])*(a[dim]-b[dim]) + (a[dim-1]-b[dim-1])*(a[dim-1]-b[dim-1]); 
		break;
	case 3:
		ker_val -= (a[dim]-b[dim])*(a[dim]-b[dim]) + (a[dim-1]-b[dim-1])*(a[dim-1]-b[dim-1]) + (a[dim-2]-b[dim-2])*(a[dim-2]-b[dim-2]); 
		break;	
	default:		
		break;
	}
	 
  return exp((double)ker_val / (2 * sigma * sigma));
}


#else
#ifdef __SSE3__
/*
	Vectorized code with SSE3 intrinsics 
	(this should be available on most Intel & AMD processors)
*/
#include <pmmintrin.h>
/*
	Dot product (linear kernel function)
*/ 
double dot_double(const double *a, const double *b, const long dim) {

  double dot;

  long i;

  __m128d num1, num2, num3, num4;

  num4= _mm_setzero_pd();  //sets sum to zero

  for(i=1; i<=dim-1; i+=2)
  {
    num1 = _mm_load_pd(a+i);   
    num2 = _mm_load_pd(b+i);   
    
    num3 = _mm_mul_pd(num1,num2); // multiplication
    num3 = _mm_hadd_pd(num3, num3); // horizontal addition

    num4 = _mm_add_sd(num4, num3);  // vertical summation
  }
   
  _mm_store_sd(&dot,num4);

  // Epilog:
  if(dim%2 != 0) {
  	dot += a[dim]*b[dim]; 
  }

  return dot;
}

double dot_float(const float *a, const float *b, const long dim) {

  float dot;
  long i;

  __m128 num1, num2, num3, num4;
  
  num4= _mm_setzero_ps();  //sets sum to zero
  
  for(i=1; i<=dim-3; i+=4)
  {
    num1 = _mm_load_ps(a+i);  
    num2 = _mm_load_ps(b+i);  

    num3 = _mm_mul_ps(num1,num2); // product: num3[0] = num1[0]*num2[0], num3[1]=num1[0]*num2[0]...
    num3 = _mm_hadd_ps(num3, num3); // horizontal addition: num3[0] = num3[0]+num3[1],num3[1]=num3[2]+num3[3]
 
    num4 = _mm_add_ps(num4, num3);  // vertical addition: num4[0] += num3[0], num4[1]+=num3[1]
  }
  num4= _mm_hadd_ps(num4, num4); // last horizontal addition: num4[0] = num4[0]+num4[1]
  _mm_store_ss(&dot,num4);
  
  int epilog = dim%4;	
  switch (epilog) {	
	case 1:
		dot += (a[dim]*b[dim]);
		break;
	case 2:
		dot += (a[dim]*b[dim]) + (a[dim-1]*b[dim-1]);
		break;
	case 3:
		dot += (a[dim]*b[dim]) + (a[dim-1]*b[dim-1]) + (a[dim-2]*b[dim-2]); 
		break;	
	default:		
		break;
	}
 return (double)dot;
}


/*  
	Gaussian RBF kernel
	k(x,z) = exp(-||x - z||^2 / 2sigma^2)
*/
double rbf_double(const double *a, const double *b, const long dim, const double sigma) {

  double ker_val;

  long i;

  __m128d num1, num2, num3, num4;

  num4= _mm_setzero_pd();  //sets sum to zero

  for(i=1; i<=dim-1; i+=2)
  {
    num1 = _mm_load_pd(a+i);   
    num2 = _mm_load_pd(b+i);   

    num3 = _mm_sub_pd(num1, num2); // substraction

    // dot product:
    num3 = _mm_mul_pd(num3,num3);   // multiplication
    num3 = _mm_hadd_pd(num3, num3); // horizontal addition
    
    num4 = _mm_sub_sd(num4, num3);  // vertical addition
  }
   
  _mm_store_sd(&ker_val,num4);

  // Epilog:
  if(dim%2 != 0) {
  	ker_val -= (a[dim]-b[dim])*(a[dim]-b[dim]); 
  }

  return exp(ker_val / (2 * sigma * sigma));
}

double rbf_float(const float *a, const float *b, const long dim, const double sigma) {

  float ker_val;
  long i;

  __m128 num1, num2, num3, num4;
  
  num4= _mm_setzero_ps();  //sets sum to zero
  
  for(i=1; i<=dim-3; i+=4)
  {
    num1 = _mm_load_ps(a+i);   //loads aligned array a into num1  num1= a[3]  a[2]  a[1]  a[0]
    num2 = _mm_load_ps(b+i);   //loads aligned array b into num2  num2= b[3]  b[2]  b[1]  b[0]

    num3 = _mm_sub_ps(num1, num2); // parallel substraction num3 = num1 - num2
                           
    num3 = _mm_mul_ps(num3,num3); // product: num3[0] = num3[0]^2...
    num3 = _mm_hadd_ps(num3, num3); // horizontal addition: num3[0] = num3[0]+num3[1],
    				    // 			     num3[1] = num3[2]+num3[3]
    num4 = _mm_sub_ps(num4, num3);  // vertical addition    
  }
  num4= _mm_hadd_ps(num4, num4); // last horizontal addition
  _mm_store_ss(&ker_val,num4);
  
  // Epilog
  int epilog = dim%4;	
  switch (epilog) {
	
	case 1:
		ker_val -= (a[dim]-b[dim])*(a[dim]-b[dim]); 
		break;
	case 2:
		ker_val -= (a[dim]-b[dim])*(a[dim]-b[dim]) + (a[dim-1]-b[dim-1])*(a[dim-1]-b[dim-1]); 
		break;
	case 3:
		ker_val -= (a[dim]-b[dim])*(a[dim]-b[dim]) + (a[dim-1]-b[dim-1])*(a[dim-1]-b[dim-1]) + (a[dim-2]-b[dim-2])*(a[dim-2]-b[dim-2]); 
		break;	
	default:		
		break;
	}
	 
  return exp((double)ker_val / (2 * sigma * sigma));
}

#else

/*
	If SSE3 is not available:
	
	 Use standard (non-vectorized) operations (no intrinsics)	 
*/

/*
	Dot product (linear kernel function)
*/
double dot_double(const double *a , const double *b , const long dim) {
	long i;
	double result=0.0;

	for(i = 1; i<=dim; i++)
	  result += a[i] * b[i];

	return result;
}

double dot_float(const float *a , const float *b , const long dim) {
	long i;
	float result = 0.0;

	for(i = 1; i<=dim; i++)
	  result += a[i] * b[i];

	return (double)result;
}

/* 
	Gaussian RBF kernel
	k(x,z) = exp(-||x - z||^2 / 2sigma^2)
*/
double rbf_double(const double *a, const double *b, const long dim, const double sigma) {

	long i;
	double result=0.0;

	for(i=1;i<=dim;i++) 
	  result -= (a[i] - b[i]) * (a[i] - b[i]);
	
	result /= (2 * sigma * sigma);

	return exp(result);
}
double rbf_float(const float *a, const float *b, const long dim, const double sigma) {

	long i;
	float result=0.0;

	for(i=1;i<=dim;i++) 
	  result -= (a[i] - b[i]) * (a[i] - b[i]);

	return exp((double)result/(2 * sigma * sigma));
}
#endif
#endif

/*
	Vectorized code with SSE2 intrinsics
	
	(this should be available on all processors released since 2003)
*/
#include <emmintrin.h>

/*
	Linear kernel for SHORT INT data
*/
double dot_short_int(const short int *a, const short int *b, const long dim) {

  int dot;
  long i;

  __m128i num1, num2, num3, num4;
  
  num4= _mm_setzero_si128();  //sets sum to zero
  
  for(i=1; i<=dim-7; i+=8)
  {
    num1 = _mm_load_si128((__m128i *)(a+i));   
    num2 = _mm_load_si128((__m128i *)(b+i));   
	
    num3 = _mm_madd_epi16(num1,num2); // dot product
    num4 = _mm_add_epi32(num4, num3); // vertical addition 
  }
  
  // Final horizontal addition
  num3 = _mm_srli_si128(num4,4);
  num4 = _mm_add_epi32(num3,num4);
  num3 = _mm_srli_si128(num3,4); 
  num4 = _mm_add_epi32(num3,num4);
  num3 = _mm_srli_si128(num3,4);
  num4 = _mm_add_epi32(num3,num4);
  
  dot = _mm_cvtsi128_si32(num4);
  
  int epilog = dim%8;	
  switch (epilog) {	
	case 1:
		dot += (a[dim]*b[dim]);
		break;
	case 2:
		dot += (a[dim]*b[dim]) + (a[dim-1]*b[dim-1]);
		break;
	case 3:
		dot += (a[dim]*b[dim]) + (a[dim-1]*b[dim-1]) + (a[dim-2]*b[dim-2]); 
		break;	
	case 4:
		dot += (a[dim]*b[dim]) + (a[dim-1]*b[dim-1]) + (a[dim-2]*b[dim-2]) + (a[dim-3]*b[dim-3]); 
		break;	
	case 5:
		dot += (a[dim]*b[dim]) + (a[dim-1]*b[dim-1]) + (a[dim-2]*b[dim-2]) + (a[dim-3]*b[dim-3]) + (a[dim-4]*b[dim-4]); 
		break;	
	case 6:
		dot += (a[dim]*b[dim]) + (a[dim-1]*b[dim-1]) + (a[dim-2]*b[dim-2])+ (a[dim-3]*b[dim-3]) + (a[dim-5]*b[dim-5]);  
		break;	
	case 7:
		dot += (a[dim]*b[dim]) + (a[dim-1]*b[dim-1]) + (a[dim-2]*b[dim-2])+ (a[dim-3]*b[dim-3]) + (a[dim-5]*b[dim-5]) + (a[dim-6]*b[dim-6]);
		break;	
	default:		
		break;
	}
 return (double)dot;
}

/*
	Gaussian RBF kernel for SHORT INT data
*/
double rbf_short_int(const short int *a, const short int *b, const long dim, const double sigma) {

  int dot;
  long i;

  __m128i num1, num2, num3, num4;
  
  num4= _mm_setzero_si128();  //sets sum to zero
  
  for(i=1; i<=dim-7; i+=8)
  {
    num1 = _mm_load_si128((__m128i *)(a+i));   
    num2 = _mm_load_si128((__m128i *)(b+i));   

    num3 = _mm_sub_epi16(num1, num2); // substraction

    num3 = _mm_madd_epi16(num3,num3); // dot product
    num4 = _mm_add_epi32(num4, num3); // vertical addition 
  }
  
  // Final horizontal addition
  num3 = _mm_srli_si128(num4,4);
  num4 = _mm_add_epi32(num3,num4);
  num3 = _mm_srli_si128(num3,4); 
  num4 = _mm_add_epi32(num3,num4);
  num3 = _mm_srli_si128(num3,4);
  num4 = _mm_add_epi32(num3,num4);
  
  dot = _mm_cvtsi128_si32(num4);

  int epilog = dim%8;	
  switch (epilog) {	
  case 1:
	dot += (a[dim]-b[dim])*(a[dim]-b[dim]); 
	break;
  case 2:
	dot += (a[dim]-b[dim])*(a[dim]-b[dim]) + (a[dim-1]-b[dim-1])*(a[dim-1]-b[dim-1]); 
	break;
  case 3:
	dot += (a[dim]-b[dim])*(a[dim]-b[dim]) + (a[dim-1]-b[dim-1])*(a[dim-1]-b[dim-1]) + (a[dim-2]-b[dim-2])*(a[dim-2]-b[dim-2]); 
	break;	
  case 4:
	dot += (a[dim]-b[dim])*(a[dim]-b[dim]) + (a[dim-1]-b[dim-1])*(a[dim-1]-b[dim-1]) + (a[dim-2]-b[dim-2])*(a[dim-2]-b[dim-2])+ (a[dim-3]-b[dim-3])*(a[dim-3]-b[dim-3]); 	
	break;	
  case 5:
	dot += (a[dim]-b[dim])*(a[dim]-b[dim]) + (a[dim-1]-b[dim-1])*(a[dim-1]-b[dim-1]) + (a[dim-2]-b[dim-2])*(a[dim-2]-b[dim-2])+ (a[dim-3]-b[dim-3])*(a[dim-3]-b[dim-3])+ (a[dim-4]-b[dim-4])*(a[dim-4]-b[dim-4]);
	break;	
  case 6:
	dot += (a[dim]-b[dim])*(a[dim]-b[dim]) + (a[dim-1]-b[dim-1])*(a[dim-1]-b[dim-1]) + (a[dim-2]-b[dim-2])*(a[dim-2]-b[dim-2])+ (a[dim-3]-b[dim-3])*(a[dim-3]-b[dim-3]) + (a[dim-4]-b[dim-4])*(a[dim-4]-b[dim-4]) + (a[dim-5]-b[dim-5])*(a[dim-5]-b[dim-5]); 
	break;	
  case 7:
	dot += (a[dim]-b[dim])*(a[dim]-b[dim]) + (a[dim-1]-b[dim-1])*(a[dim-1]-b[dim-1]) + (a[dim-2]-b[dim-2])*(a[dim-2]-b[dim-2])+ (a[dim-3]-b[dim-3])*(a[dim-3]-b[dim-3]) + (a[dim-4]-b[dim-4])*(a[dim-4]-b[dim-4]) + (a[dim-5]-b[dim-5])*(a[dim-5]-b[dim-5]) + (a[dim-6]-b[dim-6])*(a[dim-6]-b[dim-6]); 
	break;	

  default:		
	break;
 }
 
  return exp(-(double)dot / (2 * sigma * sigma));
}


/*
	---- Non-vectorized part of code ----
*/

/*
	Linear kernel
*/
double dot_int(const int *a , const int *b , const long dim) {
	long i;
	int result = 0;

	for(i = 1; i<=dim; i++)
	  result += a[i] * b[i];

	return (double)result;
}
double dot_byte(const unsigned char *a , const unsigned char *b , const long dim) {
	long i;
	int result = 0;

	for(i = 1; i<=dim; i++)
	  result += a[i] * b[i];

	return (double)result;
}

/*
	Gaussian RBF kernel
*/
double rbf_int(const int *a, const int *b, const long dim, const double sigma) {

	long i;
	int result=0;

	for(i=1;i<=dim;i++) 
	  result -= (a[i] - b[i]) * (a[i] - b[i]);

	return exp((double)result/(2 * sigma * sigma));
}
double rbf_byte(const unsigned char *a, const unsigned char *b, const long dim, const double sigma) {

	long i;
	unsigned int result=0;

	for(i=1;i<=dim;i++) 
	  result += (a[i] - b[i]) * (a[i] - b[i]);

	return exp(- (double)result/(2 * sigma * sigma));
}


/* 
	Homogeneous Polynomial kernel
	k(x,z) = (x^T z)^deg
*/
double polynomial_homo_double(const double *a , const double *b , const long dim, const int deg) {
	int i;
	double value = dot_double(a,b,dim);
	double result = value;

	for(i = 1; i < deg; i++)
		result *= value;

	return result;
}

double polynomial_homo_float(const float *a , const float *b , const long dim, const int deg) {
	int i;

	// The dot product is computed with single precision floats
	double value = dot_float(a,b,dim);
	double result = value;
	
	// But the power is computed with double precision
	for(i = 1; i < deg; i++)
		result *= value;

	return result;
}
double polynomial_homo_int(const int *a , const int *b , const long dim, const int deg) {
	int i;
	long value = 0;
	long result;

	for(i = 1; i<=dim; i++)
	  value += a[i] * b[i];

	result = value;
	for(i = 1; i < deg; i++)
		result *= value;

	return (double)result;
}
double polynomial_homo_short_int(const short int *a , const short int *b , const long dim, const int deg) {
	int i;
	int value = 0;
	int result;

	for(i = 1; i<=dim; i++)	
	  value += a[i] * b[i];	// could be vectorized in SSE2

	result = value;
	for(i = 1; i < deg; i++)
		result *= value;

	return (double)result;
}
double polynomial_homo_byte(const unsigned char *a , const unsigned char *b , const long dim, const int deg) {
	int i;
	unsigned int value = 0;
	unsigned int result;

	for(i = 1; i<=dim; i++)
	  value += a[i] * b[i];

	result = value;
	for(i = 1; i < deg; i++)
		result *= value;

	return (double)result;
}

/* 
	Non-homogeneous Polynomial kernel
	k(x,z) = (x^T z + 1)^deg
*/
double polynomial_double(const double *a , const double *b , const long dim, const int deg) {
	int i;
	double value = 1.0 + dot_double(a,b,dim);
	double result = value;
	
	for(i = 1; i < deg; i++)
		result *= value;

	return result;
}

double polynomial_float(const float *a , const float *b , const long dim, const int deg) {
	int i;

	// The dot product is computed with single precision floats
	double value = 1.0 + dot_float(a,b,dim);
	double result = value;
	
	// But the power is computed with double precision
	for(i = 1; i < deg; i++)
		result *= value;
		
	return result;
}

double polynomial_int(const int *a , const int *b , const long dim, const int deg) {
	int i;
	long value = 1;
	long result;

	for(i = 1; i<=dim; i++)
	  value += a[i] * b[i];

	result = value;
	for(i = 1; i < deg; i++)
		result *= value;

	return (double)result;
}
double polynomial_short_int(const short int *a , const short int *b , const long dim, const int deg) {
	int i;
	int value = 1;
	int result;

	for(i = 1; i<=dim; i++)	
	  value += a[i] * b[i];	// could be vectorized in SSE2

	result = value;
	for(i = 1; i < deg; i++)
		result *= value;

	return (double)result;
}

double polynomial_byte(const unsigned char *a , const unsigned char *b , const long dim, const int deg) {
	int i;
	int value = 1;
	int result;

	for(i = 1; i<=dim; i++)
	  value += a[i] * b[i];

	result = value;
	for(i = 1; i < deg; i++)
		result *= value;

	return (double)result;
}

/*
	Linear kernel for BIT data
	k(x,z) = x^T z = sum (x[i] == z[i])
*/
double dot_bit(const int *a, const int *b, const long dim) {
	long i=1;
	unsigned int res;
	int num_bits = 0;
	long true_dim = dim/32;
	
	
	if(dim%32 != 0)
		true_dim++;
	
	for(i=1;i<=true_dim ; i++) {
	    res = a[i] & b[i];		      
	    num_bits += __builtin_popcount(res);  // fast popcount on SSE4.2/SSE4a processors
	}
	
	return (double)num_bits;
}
/*
	Gaussian RBF kernel for BIT data with Hamming distance
	k(x,z) = exp(- (sum (x[i] != z[i]))^2 / 2 sigma^2 )
*/
double rbf_bit(const int *a, const int *b, const long dim, const double sigma) {
	long i=1;
	unsigned int res;
	int dist = 0;
	long true_dim = dim/32;
	
	
	if(dim%32 != 0)
		true_dim++;
	
	for(i=1;i<=true_dim ; i++) {
	    res = ~(a[i] & b[i]);		      
	    dist += __builtin_popcount(res);  
	}
	
	return exp( (double)( -dist * dist) / (2 * sigma * sigma));
}

/*
	Homogeneous polynomial kernel for BIT data
	k(x,z) = (x^T z)^deg = (sum (x[i] == z[i]) )^deg 
*/
double polynomial_homo_bit(const int *a, const int *b, const long dim, const int deg) {
	long i=1;
	unsigned int res;
	int num_bits = 0;
	long true_dim = dim/32;
	
	
	if(dim%32 != 0)
		true_dim++;
	
	for(i=1;i<=true_dim ; i++) {
	    res = a[i] & b[i];		      
	    num_bits += __builtin_popcount(res);  
	}

	int result = num_bits;
	for(i = 1; i < deg; i++)
		result *= num_bits;
		
	return (double)result;
}

/*
	Non-homogeneous polynomial kernel for BIT data
*/
double polynomial_bit(const int *a, const int *b, const long dim, const int deg) {
	long i=1;
	unsigned int res;
	int num_bits = 1;
	long true_dim = dim/32;
	
	
	if(dim%32 != 0)
		true_dim++;
	
	for(i=1;i<=true_dim ; i++) {
	    res = a[i] & b[i];		      
	    num_bits += __builtin_popcount(res);  
	}

	int result = num_bits;
	for(i = 1; i < deg; i++)
		result *= num_bits;
		
	return (double)result;
}
