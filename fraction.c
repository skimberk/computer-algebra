#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include "fraction.h"

void correctSignFraction(struct Fraction *x) {
    // Keep sign on numerator
    x->n->sign = x->n->sign * x->d->sign;
    x->d->sign = 1;

    if (isZeroBigInt(x->d)) {
        x->d->sign = 1;
    }
}

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

    correctSignFraction(f);

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

struct Fraction *createFromSingleStringFraction(char *str) {
    char *saveptr; // for strtok_r
    char *nStr = strtok_r(str, "/", &saveptr);
    char *dStr = strtok_r(NULL, "/", &saveptr);

    assert(nStr != NULL);
    assert(strtok_r(NULL, "/", &saveptr) == NULL); // No more tokens
    
    if (dStr == NULL) {
        // No denominator
        return createFromStringFraction(nStr, "1");
    }

    return createFromStringFraction(nStr, dStr);
}

struct Fraction *copyFraction(struct Fraction *x) {
    struct Fraction *out = malloc(sizeof(struct Fraction));
    out->n = copyBigInt(x->n);
    out->d = copyBigInt(x->d);

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

    correctSignFraction(out);

    return out;
}

struct Fraction *addFraction(struct Fraction *x, struct Fraction *y) {
    struct BigInt *a = x->n;
    struct BigInt *b = x->d;
    struct BigInt *c = y->n;
    struct BigInt *d = y->d;
    struct BigInt *gcd = gcdBigInt(b, d);

    struct BigIntPair *pair;

    pair = divideBigInt(b, gcd);
    assert(isZeroBigInt(pair->y));
    struct BigInt *b1 = pair->x;
    freeBigInt(pair->y);
    free(pair);

    pair = divideBigInt(d, gcd);
    assert(isZeroBigInt(pair->y));
    struct BigInt *d1 = pair->x;
    freeBigInt(pair->y);
    free(pair);

    a = multiplyBigInt(a, d1);
    c = multiplyBigInt(c, b1);

    struct Fraction *out = malloc(sizeof(struct Fraction));
    out->n = addBigInt(a, c);
    out->d = multiplyBigInt(b1, d);

    replaceBigInt(&gcd, gcdBigInt(out->n, gcd));

    pair = divideBigInt(out->n, gcd);
    assert(isZeroBigInt(pair->y));
    replaceBigInt(&out->n, pair->x);
    freeBigInt(pair->y);
    free(pair);

    pair = divideBigInt(out->d, gcd);
    assert(isZeroBigInt(pair->y));
    replaceBigInt(&out->d, pair->x);
    freeBigInt(pair->y);
    free(pair);

    freeBigInt(a);
    freeBigInt(b1);
    freeBigInt(c);
    freeBigInt(d1);
    freeBigInt(gcd);

    correctSignFraction(out);

    return out;
}

struct Fraction *subtractFraction(struct Fraction *x, struct Fraction *y) {
    struct Fraction *yNeg = copyFraction(y);
    flipSignBigInt(yNeg->n);
    struct Fraction *out = addFraction(x, yNeg);
    freeFraction(yNeg);
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
 
    struct Fraction *out = malloc(sizeof(struct Fraction));
    out->n = a;
    out->d = b;

    correctSignFraction(out);

    return out;
}

struct Fraction *divideFraction(struct Fraction *x, struct Fraction *y) {
    struct Fraction *y1 = invertFraction(y);
    struct Fraction *out = multiplyFraction(x, y1);
    freeFraction(y1);

    return out;
}

struct Fraction *exponentFraction(struct Fraction *x, struct Fraction *y) {
    assert(y->n->sign == 1); // Positive
    assert(y->d->numBlocksUsed == 1 && y->d->blocks[0] == 1); // Denominator is 1

    struct BigInt *n = copyBigInt(y->n);
    struct Fraction *out = createFromStringFraction("1", "1");
    struct Fraction *z = copyFraction(x);

    struct BigIntDigitPair *pair;

    while (!isZeroBigInt(n)) {
        pair = divideByDigitBigInt(n, 2);
        replaceBigInt(&n, pair->x);

        if (pair->y == 1) {
            replaceFraction(&out, multiplyFraction(out, z));
        }

        free(pair);

        if (!isZeroBigInt(n)) {
            replaceFraction(&z, multiplyFraction(z, z));
        }
    }

    freeBigInt(n);
    freeFraction(z);

    return out;
}

struct Fraction *factorialFraction(struct Fraction *x) {
    assert(x->n->sign == 1); // Positive
    assert(x->n->numBlocksUsed == 1); // Can't handle taking factorial of large numbers
    assert(x->d->numBlocksUsed == 1 && x->d->blocks[0] == 1); // Denominator is 1

    struct BigInt *n = createBigInt(1);
    struct BigInt *mult = createBigInt(0);

    for (uint32_t i = 1; i <= x->n->blocks[0]; i++) {
        mult->blocks[0] = i;
        replaceBigInt(&n, multiplyBigInt(n, mult));
    }

    freeBigInt(mult);

    struct Fraction *out = malloc(sizeof(struct Fraction));
    out->n = n;
    out->d = createBigInt(1);

    return out;
}

void printFraction(struct Fraction *f) {
    printBigIntDecimal(f->n);
    printf("/");
    printBigIntDecimal(f->d);
}

/*
int main (int argc, char** argv) {
//    struct Fraction *f = createFromStringFraction("-479001600", "1048576");
//    struct Fraction *f = createFromStringFraction("12312", "93326215443944152681699238856266700490715968264381621468592963895217599993229915608941463976156518286253697920827223758251185210916864000000000000000000000000");
//    struct Fraction *f1 = invertFraction(f);
    struct Fraction *f = createFromStringFraction("3", "5");
    struct Fraction *f1 = createFromStringFraction("-3", "5");

    printFraction(f); printf("\n");

    replaceFraction(&f, subtractFraction(f, f1));
    printFraction(f); printf("\n");

//    struct BigInt *a = createBigInt(1);
//    struct BigInt *b = createBigInt(0);
//    for (int i = 1; i < 10001; i++) {
//        b->blocks[0] = i;
//        replaceBigInt(&a, multiplyBigInt(a, b));
//    }
//    printBigIntDecimal(a); printf("\n");
//    freeBigInt(a);
//    freeBigInt(b);
 
    freeFraction(f);
    freeFraction(f1);
}
*/
