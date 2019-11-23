#ifndef BIGINT_HEADER
#define BIGINT_HEADER

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

struct BigInt* createBigInt(uint32_t value);
struct BigInt* createFromStringBigInt(char *str);
struct BigIntPair *createBigIntPair(struct BigInt *x, struct BigInt *y);
struct BigIntDigitPair *createBigIntDigitPair(struct BigInt *x, uint32_t y);

void freeBigInt(struct BigInt *x);
void freeBigIntPair(struct BigIntPair *x);
void freeBigIntDigitPair(struct BigIntDigitPair *x);
void replaceBigInt(struct BigInt **x, struct BigInt *y);

void validateBigInt(struct BigInt *x);
int isZeroBigInt(struct BigInt *x);
void growBigInt(struct BigInt *x);
void useBlocksBigInt(struct BigInt *x, unsigned int numBlocks);
struct BigInt *copyBigInt(struct BigInt *x);
void flipSignBigInt(struct BigInt *x);

int compareAbsoluteBigInt(struct BigInt *x, struct BigInt *y);
int compareBigInt(struct BigInt *x, struct BigInt *y);

struct BigInt *addBigInt(struct BigInt *x, struct BigInt *y);
struct BigInt *subtractBigInt(struct BigInt *x, struct BigInt *y);
struct BigInt *multiplyBigInt(struct BigInt *x, struct BigInt *y);
struct BigInt *shiftRightBigInt(struct BigInt *x, unsigned int places);
struct BigInt *shiftLeftBigInt(struct BigInt *x, unsigned int places);
struct BigIntDigitPair *divideByDigitBigInt(struct BigInt *x, uint32_t y);
struct BigIntPair *divideBigInt(struct BigInt *x, struct BigInt *y);
struct BigInt *gcdBigInt(struct BigInt *x, struct BigInt *y);

void printBigIntDecimal(struct BigInt *x);
void printBigIntDigitPair(struct BigIntDigitPair *pair);
void printBigIntPair(struct BigIntPair *pair);
void printBigInt(struct BigInt *x);

#endif
