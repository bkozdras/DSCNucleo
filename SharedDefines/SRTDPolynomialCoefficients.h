#ifndef _S_RTD_POLYNOMIAL_COEFFICIENTS_H_

#define _S_RTD_POLYNOMIAL_COEFFICIENTS_H_

#define RTD_POLYNOMIAL_DEGREE 5

typedef struct _SRTDPolynomialCoefficients
{
    double values [RTD_POLYNOMIAL_DEGREE + 1];
} SRTDPolynomialCoefficients;

#endif
