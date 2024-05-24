#ifndef CPU8080_H
#define CPU8080_H

#include <stdbool.h>
#include <stdint.h>


// Status registers
typedef struct Flags
{
    uint8_t c: 1;           // Carry
    uint8_t undefiend1: 1;
    uint8_t p: 1;           // Parity
    uint8_t undefined3: 1;
    uint8_t ac:1;           // Auxiliary carry
    uint8_t undefined5: 1;
    uint8_t z: 1;           // Zero
    uint8_t s: 1;           // Sign
} Flags;

typedef struct State8080
{
    union
    {
        struct
        {
            uint8_t rl;
            uint8_t rh;
            uint8_t re;
            uint8_t rd;
            uint8_t rc;
            uint8_t rb;
            struct Flags rf;
            uint8_t ra;
        };
        struct
        {
            uint16_t rhl;
            uint16_t rde;
            uint16_t rbc;
            uint16_t psw;
        };
    };
    uint16_t sp;            // Stack Pointer
    uint16_t pc;            // Program Counter
    uint8_t *memory;
    uint8_t inte;           // Interrupt Enabled
    uint8_t *portsin;
    uint8_t *portsout;
    uint16_t rshift;
} State8080;

bool parity(uint16_t res);

void set_flags_8(State8080* state, uint8_t res);

void set_flags_16_cy(State8080* state, uint16_t res);

void set_flags_logic(State8080* state);

void add8(State8080* state, uint8_t b);

void sub8(State8080* state, uint8_t b);

void cmp(State8080* state, uint8_t b);

void inr(State8080* state, uint8_t* reg);

void dcr(State8080* state, uint8_t* reg);

void push(State8080* state, uint16_t reg);

void pop(State8080* state, uint16_t* reg);

void UnimplementedInstruction(State8080* state);

void ReadFileIntoMemoryAt(State8080* state, const char* filename, uint32_t offset);

State8080* Init8080(void);

void Emulate8080Op(State8080* state);

#endif
