#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "bigint.h"

struct Fraction {
    struct BigInt *n;
    struct BigInt *d;
};

struct Fraction *createFraction(struct BigInt *n, struct BigInt *d) {
    struct Fraction *f = malloc(sizeof(struct Fraction));
    struct BigInt *gcd = gcdBigInt(n, d);

    struct BigIntPair *pair;

    pair = divideBigInt(n, gcd);
    assert(isZeroBigInt(pair->y));
    f->n = pair->x;
    freeBigInt(pair->y);
    free(pair);

    pair = divideBigInt(d, gcd);
    assert(isZeroBigInt(pair->y));
    f->d = pair->x;
    freeBigInt(pair->y);
    free(pair);

    f->n->sign = f->n->sign * f->d->sign;
    f->d->sign = 1;

    freeBigInt(gcd);

    return f;
}

void freeFraction(struct Fraction *f) {
    freeBigInt(f->n);
    freeBigInt(f->d);
    free(f);
}

void replaceFraction (struct Fraction **x, struct Fraction *y) {
    freeFraction(*x);
    *x = y;
}

// All operations assume that fractions are in simplest form

struct Fraction *multiplyFraction(struct Fraction *x, struct Fraction *y) {
    struct BigInt *a = gcdBigInt(x->n, y->d);
    struct BigInt *b = gcdBigInt(x->d, y->n);
    struct BigInt *gcd = multiplyBigInt(a, b);

    replaceBigInt(&a, multiplyBigInt(x->n, y->n)); 
    replaceBigInt(&b, multiplyBigInt(x->d, y->d)); 

    struct BigIntPair *pair;

    pair = divideBigInt(a, gcd);
    assert(isZeroBigInt(pair->y));
    replaceBigInt(&a, pair->x);
    freeBigInt(pair->y);
    free(pair);

    pair = divideBigInt(b, gcd);
    assert(isZeroBigInt(pair->y));
    replaceBigInt(&b, pair->x);
    freeBigInt(pair->y);
    free(pair);

    freeBigInt(gcd);
 
    // Keep sign on numerator
    a->sign = a->sign * b->sign;
    b->sign = 1;

    struct Fraction *out = malloc(sizeof(struct Fraction));
    out->n = a;
    out->d = b;

    return out;
}

void printFraction(struct Fraction *f) {
    printBigIntDecimal(f->n);
    printf(" / ");
    printBigIntDecimal(f->d);
}

int main (int argc, char** argv) {
    struct BigInt *a = createBigInt(479001600);
    struct BigInt *b = createBigInt(1048576);
    a->sign = 1;
    b->sign = -1;
    struct Fraction *f = createFraction(a, b);

    printFraction(f); printf("\n");

    replaceFraction(&f, multiplyFraction(f, f));
    printFraction(f); printf("\n");

    freeBigInt(a);
    freeBigInt(b);
    freeFraction(f);
}
