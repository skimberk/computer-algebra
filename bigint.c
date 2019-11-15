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

struct BigInt* createBigInt(uint32_t value) {
    struct BigInt *x = malloc(sizeof(struct BigInt));
    x->sign = 1;
    x->numBlocks = 1;
    x->numBlocksUsed = 1;
    x->blocks = malloc(sizeof(uint32_t));
    *(x->blocks) = value;

    return x;
}

void freeBigInt(struct BigInt *x) {
    free(x->blocks);
    free(x);
}

void replaceBigInt(struct BigInt **x, struct BigInt *y) {
    freeBigInt(*x);
    x = &y;
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
                // TODO: Do a better job with this
                while (out->numBlocks <= i) {
                    growBigInt(out);
                }

                while (out->numBlocksUsed <= i) {
                    out->blocks[out->numBlocksUsed] = 0;
                    out->numBlocksUsed++;
                }
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

struct BigInt *multiplyBigInt(struct BigInt *x, struct BigInt *y) {
    validateBigInt(x);
    validateBigInt(y);

    struct BigInt *out = createBigInt(0);

    unsigned int xBlocks = x->numBlocksUsed;
    unsigned int yBlocks = y->numBlocksUsed;

    uint32_t carry;
    uint64_t result;
    for (unsigned int i = 0; i < xBlocks; i++) {
        carry = 0;
        for (unsigned int j = 0; j < yBlocks; j++) {
            if (i + j == out->numBlocksUsed) {
                if (out->numBlocks == out->numBlocksUsed) {
                    growBigInt(out);
                }
                out->numBlocksUsed++;
            }

            result = (uint64_t)x->blocks[i] * y->blocks[j];
            out->blocks[i + j] = (uint32_t)result + carry;
            carry = result / UINT32_MAX;

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

    while (out->numBlocks <= newNumBlocks) {
        growBigInt(out);
    }

    for (unsigned int i = 0; i < newNumBlocks; i++) {
        out->blocks[i] = x->blocks[i + places];
    }
    out->numBlocksUsed = newNumBlocks;

    return out;
}

struct BigInt *divideBigInt(struct BigInt *x, struct BigInt *y) {
    validateBigInt(x);
    validateBigInt(y);
    assert(!isZeroBigInt(y));

    uint32_t d = 1;
    if (y->blocks[y->numBlocksUsed - 1] < UINT32_MAX / 2 + 1) {
        d = UINT32_MAX / (y->blocks[y->numBlocksUsed - 1] + 1) + 1;
    }
    struct BigInt *temp = createBigInt(d);
    x = multiplyBigInt(x, temp);
    y = multiplyBigInt(y, temp);

    unsigned int xBlocks = x->numBlocksUsed;
    unsigned int yBlocks = y->numBlocksUsed;
    unsigned int n = yBlocks;
    unsigned int m = xBlocks - n;

    struct BigInt *out = createBigInt(0);
    struct BigInt *u = shiftRightBigInt(x, m);
    struct BigInt *w;

    unsigned int j;
    for (unsigned int i = 0; i < m + 1; i++) {
        j = m - j;
        if (u->blocks[n] == y->blocks[n - 1]) {
            q->blocks[j] = UINT32_MAX;
        } else {
            q->blocks[j] = u[n - 1] / y[n - 1];
        }

        temp->blocks[0] = q->blocks[j];
        w = multiplyBigInt(temp, y);

        while (compareAbsoluteBigInt(w, u) == 1) {
            q->blocks[j]--;

        }

        freeBigInt(w);
    }

    freeBigInt(temp);
    freeBigInt(x);
    freeBigInt(y);

    return out;
}

void printBigInt(struct BigInt *x) {
    validateBigInt(x);

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
        a = addBigInt(a, b);
        printBigInt(a); printf("\n");
    }

    b->sign = -1;
	for (int i = 0; i < 40; i++) {
        a = addBigInt(a, b);
        printBigInt(a); printf("\n");
    }
    
    b->sign = 1;
	for (int i = 0; i < 40; i++) {
        a = addBigInt(b, a);
        printBigInt(a); printf("\n");
    }
    
    b->sign = -1;
    for (int i = 0; i < 40; i++) {
        a = addBigInt(b, a);
        printBigInt(a); printf("\n");
    }

    struct BigInt *d = createBigInt(2);
    struct BigInt *e = createBigInt(2);
    e->sign = -1;
    for (int i = 0; i < 100; i++) {
        d = multiplyBigInt(d, e);
        printBigInt(d); printf("\n");
    }

    struct BigInt *f = createBigInt(1);
    struct BigInt *g = createBigInt(0);
    for (int i = 1; i < 101; i++) {
        g->blocks[0] = i;
        f = multiplyBigInt(f, g);
        printBigInt(f); printf("\n");
    }

    for (int i = 0; i < 15; i++) {
        f = shiftRightBigInt(f, 1);
        printBigInt(f); printf("\n");
    }

    b->blocks[0]=65537;
    printBigInt(multiplyBigInt(b, b)); printf("\n");

    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);

    return 0;
}
