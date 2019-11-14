#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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

void growBigInt(struct BigInt *x) {
    x->numBlocks *= 2;
    uint32_t *newBlocks = malloc(x->numBlocks * sizeof(uint32_t));
    for (unsigned int i = 0; i < x->numBlocksUsed; i++) {
        newBlocks[i] = x->blocks[i];
    }
    free(x->blocks);
    x->blocks = newBlocks;
}

int compareBigInt(struct BigInt *x, struct BigInt *y) {
    validateBigInt(x);
    validateBigInt(y);

    unsigned int xBlocks = x->numBlocksUsed;
    unsigned int yBlocks = y->numBlocksUsed;
    unsigned int n = xBlocks > yBlocks ? xBlocks : yBlocks;

    int withinBlocks;
    int currComp = 0;
    int bothZero = 1;
    for (unsigned int i = 0; i < n; i++) {
        withinBlocks = xBlocks > i && yBlocks > i;

        if (bothZero && ((xBlocks > i && x->blocks[i] > 0) || (yBlocks > i && y->blocks[i] > 0))) {
            bothZero = 0;
        }

        if ((yBlocks <= i && x->blocks[i] > 0) || (withinBlocks && x->blocks[i] > y->blocks[i])) {
            // Most signficant block of x (until now) is bigger than that of y
            currComp = 1;
        } else if ((xBlocks <= i && y->blocks[i] > 0) || (withinBlocks && y->blocks[i] > x->blocks[i])) {
            // Same as previous case, but with x and y swapped (i.e. y > x)
            currComp = -1;
        }
    }

    if (bothZero) {
        return 0;
    }

    if (x->sign > y->sign) {
        return 1;
    }

    if (x->sign < y->sign) {
        return -1;
    }

    // If both x and y are negative, we want to flip currComp
    // (at this point, we know their signs are equal)
    return x->sign * currComp;
}

struct BigInt *addBigInt(struct BigInt *x, struct BigInt *y) {
    validateBigInt(x);
    validateBigInt(y);

    struct BigInt *out = createBigInt(0);

    unsigned int xBlocks = x->numBlocksUsed;
    unsigned int yBlocks = y->numBlocksUsed;
    unsigned int n = xBlocks > yBlocks ? xBlocks : yBlocks;

    unsigned int carry = 0;

    // A subtraction problem (opposite signs)
    if (x->sign + y->sign == 0) {
        int cmp = compareBigInt(x, y);
        if (cmp == 0) {
            return out;
        }

        if (cmp < 0) {
            struct BigInt *temp = x;
            x = y;
            y = x;
        }

        for (unsigned int i = 0; i < n; i++) {

        }
        return out;
    }
    
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

    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);

    return 0;
}
