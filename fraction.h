#ifndef FRACTION_HEADER
#define FRACTION_HEADER

#include "bigint.h"

struct Fraction {
    struct BigInt *n;
    struct BigInt *d;
};

struct Fraction *createFraction(struct BigInt *n, struct BigInt *d);
struct Fraction *createFromStringFraction(char *nStr, char *dStr);
struct Fraction *copyFraction(struct Fraction *x);
void freeFraction(struct Fraction *f);
void replaceFraction (struct Fraction **x, struct Fraction *y);

// All operations assume that fractions are in simplest form
void correctSignFraction(struct Fraction *x);
struct Fraction *invertFraction(struct Fraction *x);
struct Fraction *addFraction(struct Fraction *x, struct Fraction *y);
struct Fraction *subtractFraction(struct Fraction *x, struct Fraction *y);
struct Fraction *multiplyFraction(struct Fraction *x, struct Fraction *y);
struct Fraction *divideFraction(struct Fraction *x, struct Fraction *y);
void printFraction(struct Fraction *f);

#endif
