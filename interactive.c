#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include "fraction.h"

struct FractionStack {
    struct Fraction *value;
    struct FractionStack *next;
};

void pushFractionStack(struct FractionStack **stack, struct Fraction *x) {
    struct FractionStack *temp = malloc(sizeof(struct FractionStack));
    temp->next = *stack;
    temp->value = x;
    *stack = temp;
}

struct Fraction *popFractionStack(struct FractionStack **stack) {
    assert(*stack != NULL);

    struct Fraction *out = (*stack)->value;
    struct FractionStack *temp = (*stack)->next;
    free(*stack);
    *stack = temp;

    return out;
}

struct Fraction *evalExpr(char *expr, struct Fraction *lastResult) {
    char *token = strtok(expr, " ");
    struct FractionStack *stack = NULL;
    struct Fraction *temp1;
    struct Fraction *temp2;

    while (token != NULL) {
        if (strcmp(token, "*") == 0) {
             temp2 = popFractionStack(&stack);
             temp1 = popFractionStack(&stack);
             pushFractionStack(&stack, multiplyFraction(temp1, temp2));
             freeFraction(temp1);
             freeFraction(temp2);
         } else if (strcmp(token, "/") == 0) {
             // Order of pops is important
             temp2 = popFractionStack(&stack);
             temp1 = popFractionStack(&stack);
             pushFractionStack(&stack, divideFraction(temp1, temp2));
             freeFraction(temp1);
             freeFraction(temp2);
         } else if (strcmp(token, "+") == 0) {
             temp2 = popFractionStack(&stack);
             temp1 = popFractionStack(&stack);
             pushFractionStack(&stack, addFraction(temp1, temp2));
             freeFraction(temp1);
             freeFraction(temp2);
         } else if (strcmp(token, "-") == 0) {
             // Order of pops is important
             temp2 = popFractionStack(&stack);
             temp1 = popFractionStack(&stack);
             pushFractionStack(&stack, subtractFraction(temp1, temp2));
             freeFraction(temp1);
             freeFraction(temp2);
        } else if (strcmp(token, "%") == 0) {
            pushFractionStack(&stack, copyFraction(lastResult));
        } else {
            pushFractionStack(&stack, createFromStringFraction(token, "1"));
        }

        token = strtok(NULL, " ");
    }

    temp1 = popFractionStack(&stack);
    assert(stack == NULL);

    printFraction(temp1); printf("\n");
    return temp1;
}

int main (int argc, char** argv) {
    char buff[10000];
    char *token;

    struct Fraction *lastResult = createFromStringFraction("0", "1");

    printf("******************************************************************\n");
    printf("*** Welcome to Sebastian's calculator!\n");
    printf("*** Sample expression: 1 2 * -3 *\n");
    printf("*** Rules:\n");
    printf("*** - enter expressions in postfix (i.e. Reverse Polish Notation)\n");
    printf("*** - all tokens should be separated by a single space\n");
    printf("*** - allowed operators: +, -, *, and /\n");
    printf("*** - only integers are allowed\n");
    printf("*** - use %% in place of an integer to access the result of the\n");
    printf("***   last expression to be evaluated\n");
    printf("******************************************************************\n");

    while (1) {
        printf("> ");
        fgets(buff, 10000, stdin);
        buff[strcspn(buff, "\r\n")] = 0;
        replaceFraction(&lastResult, evalExpr(buff, lastResult));
    }
}
