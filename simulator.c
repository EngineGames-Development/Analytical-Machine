#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define STORE_SIZE 1000
#define MAX_CARDS 256

typedef enum {
    OP_NUM, OP_LOAD, OP_STORE,
    OP_ADD, OP_SUB, OP_MUL, OP_DIV,
    OP_JMP_F, OP_JMP_B,
    OP_PRINT, OP_HALT
} OpCode;

typedef struct {
    OpCode op;
    int64_t arg1;
    int64_t arg2;
} Card;

typedef struct {
    int64_t store[STORE_SIZE];
    int64_t ingress1;
    int64_t ingress2;
    int64_t egress;
    int ingress_count;
    bool run_up_lever;
    int pc;
} AnalyticalEngine;

void init_engine(AnalyticalEngine *ae) {
    memset(ae->store, 0, sizeof(ae->store));
    ae->ingress1 = 0;
    ae->ingress2 = 0;
    ae->egress = 0;
    ae->ingress_count = 0;
    ae->run_up_lever = false;
    ae->pc = 0;
}
Card assemble_line(const char *line) {
    Card c = {OP_HALT, 0, 0};
    char mnemonic[16];
    if (sscanf(line, "%15s", mnemonic) != 1) return c;

    if (strcmp(mnemonic, "N") == 0) {
        c.op = OP_NUM;
        sscanf(line, "%*s %lld %lld", &c.arg1, &c.arg2);
    } else if (strcmp(mnemonic, "L") == 0) {
        c.op = OP_LOAD;
        sscanf(line, "%*s %lld", &c.arg1);
    } else if (strcmp(mnemonic, "S") == 0) {
        c.op = OP_STORE;
        sscanf(line, "%*s %lld", &c.arg1);
    } else if (strcmp(mnemonic, "+") == 0) {
        c.op = OP_ADD;
    } else if (strcmp(mnemonic, "-") == 0) {
        c.op = OP_SUB;
    } else if (strcmp(mnemonic, "*") == 0) {
        c.op = OP_MUL;
    } else if (strcmp(mnemonic, "/") == 0) {
        c.op = OP_DIV;
    } else if (strcmp(mnemonic, "CF?") == 0) {
        c.op = OP_JMP_F;
        sscanf(line, "%*s %lld", &c.arg1);
    } else if (strcmp(mnemonic, "CB?") == 0) {
        c.op = OP_JMP_B;
        sscanf(line, "%*s %lld", &c.arg1);
    } else if (strcmp(mnemonic, "P") == 0) {
        c.op = OP_PRINT;
    } else if (strcmp(mnemonic, "H") == 0) {
        c.op = OP_HALT;
    }
    return c;
}

void run_engine(AnalyticalEngine *ae, Card deck[], int deck_size) {
    while (ae->pc >= 0 && ae->pc < deck_size) {
        Card c = deck[ae->pc];
        ae->pc++;

        switch (c.op) {
            case OP_NUM:
                if (c.arg1 >= 0 && c.arg1 < STORE_SIZE)
                    ae->store[c.arg1] = c.arg2;
                break;

            case OP_LOAD:
                if (c.arg1 >= 0 && c.arg1 < STORE_SIZE) {
                    if (ae->ingress_count == 0) {
                        ae->ingress1 = ae->store[c.arg1];
                        ae->ingress_count = 1;
                    } else {
                        ae->ingress2 = ae->store[c.arg1];
                        ae->ingress_count = 0;
                    }
                }
                break;

            case OP_STORE:
                if (c.arg1 >= 0 && c.arg1 < STORE_SIZE)
                    ae->store[c.arg1] = ae->egress;
                break;

            case OP_ADD:
                ae->egress = ae->ingress1 + ae->ingress2;
                ae->run_up_lever = (ae->egress > 0);
                ae->ingress_count = 0;
                break;

            case OP_SUB:
                ae->egress = ae->ingress1 - ae->ingress2;
                ae->run_up_lever = (ae->egress > 0);
                ae->ingress_count = 0;
                break;

            case OP_MUL:
                ae->egress = ae->ingress1 * ae->ingress2;
                ae->run_up_lever = (ae->egress > 0);
                ae->ingress_count = 0;
                break;

            case OP_DIV:
                if (ae->ingress2 == 0) {
                    fprintf(stderr, "BELL RINGS: Division by zero!\n");
                    return;
                }
                ae->egress = ae->ingress1 / ae->ingress2;
                ae->run_up_lever = (ae->egress > 0);
                ae->ingress_count = 0;
                break;

            case OP_JMP_F:
                if (ae->run_up_lever) ae->pc += (int)c.arg1 - 1;
                break;

            case OP_JMP_B:
                if (ae->run_up_lever) ae->pc -= (int)c.arg1 + 1;
                break;

            case OP_PRINT:
                printf("PRINTER OUTPUT: %lld\n", ae->egress);
                break;

            case OP_HALT:
                printf("BELL RINGS: Execution halted.\n");
                return;
        }
    }
}

int main(void) {
    const char *program[] = {
        "N 0 5",
        "N 1 1",
        "N 2 1",
        "N 3 0",

        "L 1",
        "L 0",
        "*",
        "S 1",

        "L 0",
        "L 2",
        "-",
        "S 0",
        "CB? 8",

        "L 1",
        "L 3",
        "+",
        "P",
        "H"
    };

    int num_cards = sizeof(program) / sizeof(program[0]);
    Card deck[MAX_CARDS];

    for (int i = 0; i < num_cards; i++) {
        deck[i] = assemble_line(program[i]);
    }

    AnalyticalEngine ae;
    init_engine(&ae);
    run_engine(&ae, deck, num_cards);

    return 0;
}
