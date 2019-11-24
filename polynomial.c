#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include "fraction.h"

struct Polynomial {
    unsigned int numCoeffs;
    unsigned int numCoeffsAllocated;
    struct Fraction **coeffs;
};

struct Polynomial *createPolynomial() {
    struct Polynomial *p = malloc(sizeof(struct Polynomial));
    p->numCoeffs = 1;
    p->numCoeffsAllocated = 1;
    p->coeffs = malloc(sizeof(struct Fraction*));
    p->coeffs[0] = createFromStringFraction("0", "1");

    return p;
}

void ensureNumCoeffsPolynomial(struct Polynomial *x, unsigned int numCoeffs) {
    if (x->numCoeffsAllocated < numCoeffs) {
        unsigned int newNumCoeffsAllocated = x->numCoeffsAllocated;
        while (newNumCoeffsAllocated < numCoeffs) {
            newNumCoeffsAllocated *= 2;
        }

        struct Fraction **newCoeffs = malloc(newNumCoeffsAllocated * sizeof(struct Fraction*));
        for (unsigned int i = 0; i < x->numCoeffs; i++) {
            newCoeffs[i] = x->coeffs[i];
        }

        free(x->coeffs);
        x->coeffs = newCoeffs;
        x->numCoeffsAllocated = newNumCoeffsAllocated;
    }

    while (x->numCoeffs < numCoeffs) {
        x->coeffs[x->numCoeffs] = createFromStringFraction("0", "1");
        x->numCoeffs++;
    }
}

struct Polynomial *joinPolynomial(
    struct Polynomial *x,
    struct Polynomial *y,
    struct Fraction *(*func)(struct Fraction*, struct Fraction*)
) {
    unsigned int maxCoeffs = x->numCoeffs > y->numCoeffs ? x->numCoeffs : y->numCoeffs;
    struct Polynomial *out = createPolynomial();

    struct Fraction *coeff;
    for (unsigned int i = 0; i < maxCoeffs; i++) {
        if (i >= x->numCoeffs) {
            coeff = copyFraction(y->coeffs[i]);
        } else if (i >= y->numCoeffs) {
            coeff = copyFraction(x->coeffs[i]);
        } else {
            coeff = (*func)(x->coeffs[i], y->coeffs[i]);
        }

        if (!isZeroBigInt(coeff->n)) {
            ensureNumCoeffsPolynomial(out, i + 1);
            out->coeffs[i] = coeff;
        } else {
            freeFraction(coeff);
        }
    }

    return out;
}

struct Polynomial *addPolynomial(struct Polynomial *x, struct Polynomial *y) {
    return joinPolynomial(x, y, &addFraction);
}

struct Polynomial *subtractPolynomial(struct Polynomial *x, struct Polynomial *y) {
    return joinPolynomial(x, y, &subtractFraction);
}

void printPolynomial(struct Polynomial *x) {
    unsigned int j;
    for (unsigned int i = 0; i < x->numCoeffs; i++) {
        j = x->numCoeffs - i - 1;
        if (j == 0) {
            printFraction(x->coeffs[0]);
        } else {
            printFraction(x->coeffs[j]);
            printf(" * x^%u + ", j);
        }
    }
}

int main (int argc, char** argv) {
    struct Polynomial *p = malloc(sizeof(struct Polynomial));
    p->numCoeffs = 1;
    p->numCoeffsAllocated = 1;
    p->coeffs = malloc(sizeof(struct Fraction*));
    p->coeffs[0] = createFromStringFraction("12", "10");
    printPolynomial(addPolynomial(p, p)); printf("\n");
    ensureNumCoeffsPolynomial(p, 4);
    p->coeffs[2] = createFromStringFraction("1", "3");
    printPolynomial(subtractPolynomial(p, p)); printf("\n");
}
