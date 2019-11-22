#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

struct BigInt {
    int sign;
    unsigned int numBlocks;
    unsigned int numBlocksUsed;
    uint32_t *blocks;
};

struct BigIntPair {
    struct BigInt *x;
    struct BigInt *y;
};

struct BigIntDigitPair {
    struct BigInt *x;
    uint32_t y;
};

void printBigInt(struct BigInt *x);

struct BigInt* createBigInt(uint32_t value) {
    struct BigInt *x = malloc(sizeof(struct BigInt));
    x->sign = 1;
    x->numBlocks = 1;
    x->numBlocksUsed = 1;
    x->blocks = malloc(sizeof(uint32_t));
    *(x->blocks) = value;

    return x;
}

struct BigIntPair *createBigIntPair(struct BigInt *x, struct BigInt *y) {
    struct BigIntPair *out = malloc(sizeof(struct BigIntPair));
    out->x = x;
    out->y = y;
    return out;
}

struct BigIntDigitPair *createBigIntDigitPair(struct BigInt *x, uint32_t y) {
    struct BigIntDigitPair *out = malloc(sizeof(struct BigIntDigitPair));
    out->x = x;
    out->y = y;
    return out;
}

void freeBigInt(struct BigInt *x) {
    free(x->blocks);
    free(x);
}

void freeBigIntPair(struct BigIntPair *x) {
    freeBigInt(x->x);
    freeBigInt(x->y);
    free(x);
}

void freeBigIntDigitPair(struct BigIntDigitPair *x) {
    freeBigInt(x->x);
    free(x);
}

void replaceBigInt(struct BigInt **x, struct BigInt *y) {
    freeBigInt(*x);
    *x = y;
}

void validateBigInt(struct BigInt *x) {
    if (x->sign != -1 && x->sign != 1) {
        printf("BigInt must have sign 1 or -1\n");
        exit(1);
    }

    if (x->numBlocksUsed <= 0) {
        printf("BigInt must use at least one block (even if zero)\n");
        exit(1);
    }

    if (x->numBlocksUsed > 1 && x->blocks[x->numBlocksUsed - 1] == 0) {
        printf("BigInt cannot have trailing zeros (except 0, which has exactly one)\n");
        exit(1);
    }

    if (x->numBlocksUsed == 1 && x->blocks[0] == 0 && x->sign == -1) {
        printf("Zero must have positive sign\n");
        exit(1);
    }
}

int isZeroBigInt(struct BigInt *x) {
    validateBigInt(x);
    return x->numBlocksUsed == 1 && x->blocks[0] == 0;
}

void growBigInt(struct BigInt *x) {
    x->numBlocks *= 2;
    uint32_t *newBlocks = malloc(x->numBlocks * sizeof(uint32_t));
    for (unsigned int i = 0; i < x->numBlocksUsed; i++) {
        newBlocks[i] = x->blocks[i];
    }
    free(x->blocks);
    x->blocks = newBlocks;
}

void useBlocksBigInt(struct BigInt *x, unsigned int numBlocks) {
    if (x->numBlocks < numBlocks) {
        unsigned int newNumBlocks = x->numBlocks;
        while (newNumBlocks < numBlocks) {
            newNumBlocks *= 2;
        }

        uint32_t *newBlocks = malloc(newNumBlocks * sizeof(uint32_t));
        for (unsigned int i = 0; i < x->numBlocksUsed; i++) {
            newBlocks[i] = x->blocks[i];
        }

        free(x->blocks);
        x->blocks = newBlocks;
        x->numBlocks = newNumBlocks;
    }

    while (x->numBlocksUsed < numBlocks) {
        x->blocks[x->numBlocksUsed] = 0;
        x->numBlocksUsed++;
    }
}

struct BigInt *copyBigInt(struct BigInt *x) {
    struct BigInt *out = malloc(sizeof(struct BigInt));
    out->sign = x->sign;
    out->numBlocks = x->numBlocks;
    out->numBlocksUsed = x->numBlocksUsed;
    out->blocks = malloc(x->numBlocks * sizeof(uint32_t));

    for (unsigned int i = 0; i < x->numBlocksUsed; i++) {
        out->blocks[i] = x->blocks[i];
    }

    return out;
}

void flipSignBigInt(struct BigInt *x) {
    if (!isZeroBigInt(x)) {
        x->sign *= -1;
    }
}

int compareAbsoluteBigInt(struct BigInt *x, struct BigInt *y) {
    validateBigInt(x);
    validateBigInt(y);

    unsigned int xBlocks = x->numBlocksUsed;
    unsigned int yBlocks = y->numBlocksUsed;
    unsigned int n = xBlocks > yBlocks ? xBlocks : yBlocks;

    int currComp = 0;
    for (unsigned int i = 0; i < n; i++) {
        if (yBlocks <= i || (xBlocks > i && x->blocks[i] > y->blocks[i])) {
            // Most signficant block of x (until now) is bigger than that of y
            currComp = 1;
        } else if (xBlocks <= i || (yBlocks > i && y->blocks[i] > x->blocks[i])) {
            // Same as previous case, but with x and y swapped (i.e. y > x)
            currComp = -1;
        }
    }

    return currComp;
}


int compareBigInt(struct BigInt *x, struct BigInt *y) {
    validateBigInt(x);
    validateBigInt(y);

    if (x->sign > y->sign) {
        return 1;
    }

    if (x->sign < y->sign) {
        return -1;
    }

    // If both x and y are negative, we want to flip comparison of their absolute values
    // (at this point, we know their signs are equal)
    return x->sign * compareAbsoluteBigInt(x, y);
}

struct BigInt *addBigInt(struct BigInt *x, struct BigInt *y) {
    validateBigInt(x);
    validateBigInt(y);

    struct BigInt *out = createBigInt(0);

    // A subtraction problem (opposite signs)
    if (x->sign + y->sign == 0) {
        int cmp = compareAbsoluteBigInt(x, y);
        if (cmp == 0) {
            return out;
        }

        if (cmp < 0) {
            // Ensure that x has larger absolute value
            struct BigInt *temp = x;
            x = y;
            y = temp;
        }

        unsigned int xBlocks = x->numBlocksUsed;
        unsigned int yBlocks = y->numBlocksUsed;
        unsigned int n = xBlocks > yBlocks ? xBlocks : yBlocks;

        uint32_t newBlock;
        unsigned int borrow = 0;
        for (unsigned int i = 0; i < n; i++) {
            if (xBlocks <= i) {
                newBlock = y->blocks[i] - borrow;
                borrow = 0;
            } else if (yBlocks <= i) {
                newBlock = x->blocks[i] - borrow;
                borrow = 0;
            } else {
                newBlock = x->blocks[i] - y->blocks[i] - borrow;
                if (newBlock > x->blocks[i]) {
                    borrow = 1;
                } else {
                    borrow = 0;
                }
            }

            if (newBlock != 0) {
                useBlocksBigInt(out, i + 1);
                out->blocks[i] = newBlock;
            }
        }

        out->sign = x->sign;

        return out;
    }
    
    unsigned int xBlocks = x->numBlocksUsed;
    unsigned int yBlocks = y->numBlocksUsed;
    unsigned int n = xBlocks > yBlocks ? xBlocks : yBlocks;

    unsigned int carry = 0;
    for (unsigned int i = 0; i < n; i++) {
        if (out->numBlocks <= i) {
            growBigInt(out);
        }

        if (i == out->numBlocksUsed) {
            out->numBlocksUsed += 1;
        }

        if (xBlocks <= i) {
            out->blocks[i] = y->blocks[i] + carry;
            carry = 0;
        } else if (yBlocks <= i) {
            out->blocks[i] = x->blocks[i] + carry;
            carry = 0;
        } else {
            out->blocks[i] = x->blocks[i] + y->blocks[i] + carry;
            if (out->blocks[i] < x->blocks[i]) {
                carry = 1;
            } else {
                carry = 0;
            }
        }
    }

    if (carry) {
        if (out->numBlocks == out->numBlocksUsed) {
            growBigInt(out);
        }
        out->numBlocksUsed += 1;
        out->blocks[n] = 1;
    }
    
    out->sign = x->sign;
    return out;
}

struct BigInt *subtractBigInt(struct BigInt *x, struct BigInt *y) {
    validateBigInt(x);
    validateBigInt(y);

    flipSignBigInt(y);
    struct BigInt *out = addBigInt(x, y);
    flipSignBigInt(y);

    return out;
}

struct BigInt *multiplyBigInt(struct BigInt *x, struct BigInt *y) {
    validateBigInt(x);
    validateBigInt(y);

    struct BigInt *out = createBigInt(0);

    if (isZeroBigInt(x) || isZeroBigInt(y)) {
        return out;
    }

    unsigned int xBlocks = x->numBlocksUsed;
    unsigned int yBlocks = y->numBlocksUsed;

    uint32_t carry;
    uint64_t result;
    for (unsigned int i = 0; i < xBlocks; i++) {
        carry = 0;
        for (unsigned int j = 0; j < yBlocks; j++) {
            useBlocksBigInt(out, i + j + 1);

            result = (uint64_t)x->blocks[i] * y->blocks[j];
            out->blocks[i + j] += (uint32_t)result + carry;
            carry = result >> 32;

            if (out->blocks[i + j] < (uint32_t)result) {
                // Overflow during addition
                carry++;
            }
        }
        if (carry) {
            if (i + yBlocks == out->numBlocksUsed) {
                if (out->numBlocks == out->numBlocksUsed) {
                    growBigInt(out);
                }
                out->numBlocksUsed++;
            }
            out->blocks[i + yBlocks] = carry;
        }
    }
    
    out->sign = x->sign * y->sign;
    return out;
}

struct BigInt *shiftRightBigInt(struct BigInt *x, unsigned int places) {
    validateBigInt(x);

    unsigned int xBlocks = x->numBlocksUsed;
    assert(xBlocks > places);
    unsigned int newNumBlocks = xBlocks - places;
    
    struct BigInt *out = createBigInt(0);
    useBlocksBigInt(out, newNumBlocks);

    for (unsigned int i = 0; i < newNumBlocks; i++) {
        out->blocks[i] = x->blocks[i + places];
    }

    out->sign = x->sign;
    return out;
}

struct BigInt *shiftLeftBigInt(struct BigInt *x, unsigned int places) {
    validateBigInt(x);

    struct BigInt *out = createBigInt(0);

    if (isZeroBigInt(x)) {
        return out;
    }

    useBlocksBigInt(out, x->numBlocksUsed + places);

    for (unsigned int i = 0; i < x->numBlocksUsed; i++) {
        out->blocks[i + places] = x->blocks[i];
    }

    out->sign = x->sign;
    return out;
}

struct BigIntDigitPair *divideByDigitBigInt(struct BigInt *x, uint32_t y) {
    validateBigInt(x);
    assert(y != 0);

    unsigned int xBlocks = x->numBlocksUsed;
    unsigned int n = 1;
    unsigned int m = xBlocks - 1;

    struct BigInt *q = createBigInt(0);
    struct BigInt *u = shiftRightBigInt(x, m);
    u->sign = 1; // We always divide positive numbers, then set sign later
    struct BigInt *w;
    struct BigInt *r = createBigInt(0);

    struct BigInt *temp = createBigInt(0);
    struct BigInt *yBig = createBigInt(y);

    uint32_t tempBlock;

    uint32_t un;
    uint32_t un1;

    unsigned int j;
    for (unsigned int i = 0; i < m + 1; i++) {
        j = m - i;

        if (u->numBlocksUsed < n) {
            un = 0;
            un1 = 0;
        } else if (u->numBlocksUsed == n) {
            un = 0;
            un1 = u->blocks[n - 1];
        } else {
            un = u->blocks[n];
            un1 = u->blocks[n - 1];
        }

//        printf("un: %u\n", un);
//        printf("un1: %u\n", un1);
//        printf("y: %u\n", y);

        if (un == y) {
            tempBlock = UINT32_MAX;
        } else {
            tempBlock = (((uint64_t)un << 32) + un1) / y;
        }

//        printf("tempBlock: %u\n", tempBlock);
//        printf("yBig: "); printBigInt(yBig); printf("\n");

        temp->blocks[0] = tempBlock;
//        printf("temp: "); printBigInt(temp); printf("\n");
        w = multiplyBigInt(temp, yBig);

//        printf("u: "); printBigInt(u); printf("\n");
//        printf("w: "); printBigInt(w); printf("\n");

        if (tempBlock != 0) {
            useBlocksBigInt(q, j + 1);
            q->blocks[j] = tempBlock;
        }

        replaceBigInt(&r, subtractBigInt(u, w));
//        printf("r: "); printBigInt(r); printf("\n");

        if (j > 0) {
            temp->blocks[0] = x->blocks[j - 1];
            replaceBigInt(&u, shiftLeftBigInt(r, 1));
            replaceBigInt(&u, addBigInt(u, temp));
        }

        freeBigInt(w);
//        printf("\n");
    }

    freeBigInt(yBig);
    freeBigInt(temp);
    freeBigInt(u);

    assert(r->numBlocksUsed == 1);
    uint32_t rDigit = r->blocks[0];
    freeBigInt(r);

    assert(rDigit < y);
    q->sign = x->sign;
    if (x->sign == -1 && rDigit != 0) {
        rDigit = y - rDigit;
    }

    return createBigIntDigitPair(q, rDigit);
}
struct BigIntPair *divideBigInt(struct BigInt *x, struct BigInt *y) {
    validateBigInt(x);
    validateBigInt(y);
    assert(!isZeroBigInt(y));

    uint32_t d = 1;
    if (y->blocks[y->numBlocksUsed - 1] < UINT32_MAX / 2 + 1) {
        d = UINT32_MAX / (y->blocks[y->numBlocksUsed - 1] + 1) + 1;
    }
    struct BigInt *temp = createBigInt(d);
    int sign = x->sign * y->sign;
    x = multiplyBigInt(x, temp);
    y = multiplyBigInt(y, temp);
    x->sign = 1;
    y->sign = 1;

//    printf("d: %u\n", d);
//    printf("x: "); printBigInt(x); printf("\n");
//    printf("y: "); printBigInt(y); printf("\n");

    unsigned int xBlocks = x->numBlocksUsed;
    unsigned int yBlocks = y->numBlocksUsed;
    unsigned int n = yBlocks;
    unsigned int m = xBlocks - n;

    struct BigInt *q = createBigInt(0);
    struct BigInt *u = shiftRightBigInt(x, m);
    struct BigInt *w;
    struct BigInt *r = createBigInt(0);

    uint32_t tempBlock;

    uint32_t un;
    uint32_t un1;

    unsigned int j;
    for (unsigned int i = 0; i < m + 1; i++) {
        j = m - i;

        if (u->numBlocksUsed < n) {
            un = 0;
            un1 = 0;
        } else if (u->numBlocksUsed == n) {
            un = 0;
            un1 = u->blocks[n - 1];
        } else {
            un = u->blocks[n];
            un1 = u->blocks[n - 1];
        }

//        printf("un: %u\n", un);
//        printf("un1: %u\n", un1);
//        printf("yn1: %u\n", y->blocks[n - 1]);

        if (un == y->blocks[n - 1]) {
            tempBlock = UINT32_MAX;
        } else {
            tempBlock = ((uint64_t)un * ((uint64_t)UINT32_MAX + 1) + un1) / y->blocks[n - 1];
        }

//        printf("tempBlock: %u\n", tempBlock);

        temp->blocks[0] = tempBlock;
        w = multiplyBigInt(temp, y);

//        printf("w: "); printBigInt(w); printf("\n");
//        printf("temp: "); printBigInt(temp); printf("\n");

        while (compareAbsoluteBigInt(w, u) == 1) {
            tempBlock--;
            replaceBigInt(&w, subtractBigInt(w, y));
        }

        if (tempBlock != 0) {
            useBlocksBigInt(q, j + 1);
        }

        if (j < q->numBlocksUsed) {
            q->blocks[j] = tempBlock;
        }

        replaceBigInt(&r, subtractBigInt(u, w));

        if (j > 0) {
            temp->blocks[0] = x->blocks[j - 1];
            replaceBigInt(&u, shiftLeftBigInt(r, 1));
            replaceBigInt(&u, addBigInt(u, temp));
        }

        freeBigInt(w);
    }

//    printf("r: "); printBigInt(r); printf("\n");
//    printf("q: "); printBigInt(q); printf("\n");

    q->sign = sign;
    if (sign == -1 && !isZeroBigInt(r)) {
        replaceBigInt(&r, subtractBigInt(y, r));
    }

    struct BigIntDigitPair *pair = divideByDigitBigInt(r, d);
    assert(pair->y == 0);
    replaceBigInt(&r, pair->x);

    free(pair);
    freeBigInt(temp);
    freeBigInt(x);
    freeBigInt(y);
    freeBigInt(u);

    return createBigIntPair(q, r);
}

struct BigInt *gcdBigInt(struct BigInt *x, struct BigInt *y) {
    struct BigInt *u = copyBigInt(x);
    struct BigInt *v = copyBigInt(y);
    u->sign = 1;
    v->sign = 1;

    struct BigIntPair *pair;

    while (!isZeroBigInt(v)) {
        pair = divideBigInt(u, v);
        replaceBigInt(&u, v);
        v = pair->y;

        freeBigInt(pair->x);
        free(pair);
    }

    freeBigInt(v);
    return u;
}

void printBigIntDecimal(struct BigInt *x) {
    validateBigInt(x);

    if (x->sign == -1) {
        printf("-");
    }

    if (isZeroBigInt(x)) {
        printf("0");
        return;
    }

    unsigned int approxDigits = 10 * x->numBlocksUsed;
    char *digits = malloc(approxDigits * sizeof(char));

    struct BigInt *q = copyBigInt(x);
    q->sign = 1; // We already printed the negative sign if negative
    struct BigIntDigitPair *pair;
    unsigned int actualDigits = 0;
    while(!isZeroBigInt(q)) {
        pair = divideByDigitBigInt(q, 10);
        assert(pair->y < 10);

        digits[actualDigits] =  pair->y + '0';

        replaceBigInt(&q, pair->x);
        free(pair);

        actualDigits++;
        assert(actualDigits <= approxDigits);
    }

    freeBigInt(q);

    for (unsigned int i = 0; i < actualDigits; i++) {
        printf("%c", digits[actualDigits - i - 1]);
    }

    free(digits);
}

void printBigIntDigitPair(struct BigIntDigitPair *pair) {
    printf("x: "); printBigIntDecimal(pair->x); printf("\n");
    printf("y: %u\n", pair->y);
}

void printBigIntPair(struct BigIntPair *pair) {
    printf("x: "); printBigIntDecimal(pair->x); printf("\n");
    printf("y: "); printBigIntDecimal(pair->y); printf("\n");
}

void printBigInt(struct BigInt *x) {
//    validateBigInt(x);

    if (x->sign == -1) {
        printf("- ");
    }

    printf("%u", x->blocks[0]);

    for (unsigned int i = 1; i < x->numBlocksUsed; i++) {
        printf(" %u", x->blocks[i]);
    }
}

int main (int argc, char** argv) {
    uint32_t x = 4294967295;
    uint32_t y = 4294967295;
    uint64_t result = (uint64_t)x * y;
    uint32_t hi = result / UINT32_MAX;
    uint32_t lo = result;
    printf("max uint32: %u\n", UINT32_MAX);
    printf("result: %lu\n", result);
    printf("hi: %u\n", hi);
    printf("lo: %u\n", lo);

    struct BigInt *a = createBigInt(0);
    struct BigInt *b = createBigInt(4294967295);
    struct BigInt *c = createBigInt(0);

    printf("comparison of a and b: %d\n", compareBigInt(a, b));
    printf("comparison of b and a: %d\n", compareBigInt(b, a));
    printf("comparison of a and a: %d\n", compareBigInt(a, a));
    printf("comparison of b and b: %d\n", compareBigInt(b, b));

    printf("comparison of a and c: %d\n", compareBigInt(a, c));
    printf("comparison of c and a: %d\n", compareBigInt(c, a));
    growBigInt(c);
    printf("comparison of a and c: %d\n", compareBigInt(a, c));
    printf("comparison of c and a: %d\n", compareBigInt(c, a));

    printBigInt(a); printf("\n");
    printBigInt(b); printf("\n");

    for (int i = 0; i < 20; i++) {
        replaceBigInt(&a, addBigInt(a, b));
        printBigInt(a); printf("\n");
    }

    b->sign = -1;
	for (int i = 0; i < 40; i++) {
        replaceBigInt(&a, addBigInt(a, b));
        printBigInt(a); printf("\n");
    }
    
    b->sign = 1;
	for (int i = 0; i < 40; i++) {
        replaceBigInt(&a, addBigInt(b, a));
        printBigInt(a); printf("\n");
    }
    
    b->sign = -1;
    for (int i = 0; i < 40; i++) {
        replaceBigInt(&a, addBigInt(b, a));
        printBigInt(a); printf("\n");
    }

    for (int i = 0; i < 40; i++) {
        replaceBigInt(&a, subtractBigInt(a, b));
        printBigInt(a); printf("\n");
    }

    struct BigInt *d = createBigInt(2);
    struct BigInt *e = createBigInt(2);
    e->sign = -1;
    for (int i = 0; i < 100; i++) {
        replaceBigInt(&d, multiplyBigInt(d, e));
        printBigInt(d); printf("\n");
    }

    d->sign = 1;
    e->sign = 1;

    printBigIntDecimal(d); printf("\n");
    printBigIntDecimal(e); printf("\n");

//    for (int i = 0; i < 100; i++) {
//        replaceBigInt(&d, divideByDigitBigInt(d, 2));
//        printBigInt(d); printf("\n");
//    }

//    replaceBigInt(&e, multiplyBigInt(d, e));
//    replaceBigInt(&e, divideBigInt(e, d));

    struct BigInt *f = createBigInt(1);
    struct BigInt *g = createBigInt(0);
    for (int i = 1; i < 101; i++) {
        g->blocks[0] = i;
        replaceBigInt(&f, multiplyBigInt(f, g));
    }

    struct BigIntPair *pair;
    struct BigIntDigitPair *digitPair;

    printBigIntDecimal(f); printf("\n");
    printBigIntDecimal(d); printf("\n");
    pair = divideBigInt(f, d);
    printBigIntPair(pair);
    freeBigIntPair(pair);

    replaceBigInt(&c, gcdBigInt(f, d));
    printf("gcd: "); printBigIntDecimal(c); printf("\n");

//    for (int i = 0; i < 15; i++) {
//        replaceBigInt(&f, shiftRightBigInt(f, 1));
//        printBigInt(f); printf("\n");
//    }

    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
    freeBigInt(d);
    freeBigInt(e);
    freeBigInt(f);
    freeBigInt(g);

    a = createBigInt(15);
    a->sign = 1;
    b = createBigInt(5);
    b->sign = 1;
    c = gcdBigInt(a, b);
    printf("gcd: "); printBigIntDecimal(c); printf("\n");
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);

    return 0;
}
