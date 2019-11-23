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
    validateBigInt(n);
    validateBigInt(d);
    assert(!isZeroBigInt(d));

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

struct Fraction *createFromStringFraction(char *nStr, char *dStr) {
    struct BigInt *n = createFromStringBigInt(nStr);
    struct BigInt *d = createFromStringBigInt(dStr);
    struct Fraction *out = createFraction(n, d);
    freeBigInt(n);
    freeBigInt(d);
    return out;
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

struct Fraction *invertFraction(struct Fraction *x) {
    assert(!isZeroBigInt(x->n));

    struct Fraction *out = malloc(sizeof(struct Fraction));
    out->n = copyBigInt(x->d);
    out->d = copyBigInt(x->n);

    out->n->sign = out->d->sign;
    out->d->sign = 1;

    return out;
}

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

    if (isZeroBigInt(a)) {
        a->sign = 1;
    }

    struct Fraction *out = malloc(sizeof(struct Fraction));
    out->n = a;
    out->d = b;

    return out;
}

struct Fraction *divideFraction(struct Fraction *x, struct Fraction *y) {
    struct Fraction *y1 = invertFraction(y);
    struct Fraction *out = multiplyFraction(x, y1);
    freeFraction(y1);

    return out;
}

void printFraction(struct Fraction *f) {
    printBigIntDecimal(f->n);
    printf(" / ");
    printBigIntDecimal(f->d);
}

int main (int argc, char** argv) {
//    struct Fraction *f = createFromStringFraction("-479001600", "1048576");
    struct Fraction *f = createFromStringFraction("12312", "93326215443944152681699238856266700490715968264381621468592963895217599993229915608941463976156518286253697920827223758251185210916864000000000000000000000000");
    struct Fraction *f1 = invertFraction(f);

    printFraction(f); printf("\n");

    replaceFraction(&f, divideFraction(f, f));
    printFraction(f); printf("\n");

    freeFraction(f);
    freeFraction(f1);
}
