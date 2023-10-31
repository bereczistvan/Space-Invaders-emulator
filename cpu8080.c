#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "cpu8080.h"

State8080* Init8080(void)
{
    State8080* state = calloc(1, sizeof(State8080));
    state->memory = malloc(0x10000);
    state->portsin = calloc(1, 0xff);
    state->portsout = calloc(1, 0xff);
    state->portsin[0] |= 0b00001110;    // Set always 1 bits
    state->portsin[1] |= 0b00001000;    // Set always 1 bits
    return state;
}

bool parity(uint16_t res)
{
    bool parity = false;
    while(res)
    {
        parity = !parity;
        res &= (res - 1);
    }
    return parity;
}

void set_flags_8(State8080* state, uint8_t res)
{
    state->rf.z = (res == 0);
    state->rf.s = 0x80 == (res & 0x80);
    state->rf.p = !parity(res);
}

void set_flags_16_cy(State8080* state, uint16_t res)
{
    set_flags_8(state, res & 0xff);
    state->rf.c = ((res & 0xff00) != 0);
}

void set_flags_logic(State8080* state)
{
    state->rf.z = (state->ra == 0);
    state->rf.s = 0x80 == (state->ra & 0x80);
    state->rf.p = !parity(state->ra);
    state->rf.c = 0;
}

void add8(State8080* state, uint8_t a, uint8_t b)
{
    uint16_t res = a + b;
    state->rf.ac = ((((a & 0xf) + (b & 0xf)) & 0xf0) != 0);
    set_flags_16_cy(state, res);
    state->ra = res & 0xff;
}

void sub8(State8080* state, uint8_t b)
{
    uint16_t res = state->ra - b;
    state->rf.ac = ((((state->ra & 0xf) - (b & 0xf)) & 0xf0) != 0);
    set_flags_16_cy(state, res);
    state->ra = res & 0xff;
}

void cmp(State8080* state, uint8_t b)
{
    uint16_t res = state->ra - b;
    state->rf.ac = ((((state->ra & 0xf) - (b & 0xf)) & 0xf0) != 0);
    set_flags_16_cy(state, res);
}

void inr(State8080* state, uint8_t* reg)
{
    uint8_t res = *reg + 1;
    state->rf.ac = ((((*reg & 0xf) + 1) & 0xf0) != 0);
    set_flags_8(state, res);
    *reg = res;
}

void dcr(State8080* state, uint8_t* reg)
{
    uint8_t res = *reg - 1;
    state->rf.ac = ((((*reg & 0xf) - 1) & 0xf0) != 0);
    set_flags_8(state, res);
    *reg = res;
}

void UnimplementedInstruction(State8080* state)
{
//    unsigned char* opcode = &state->memory[state->pc - 1];
//    printf("Error: Unimplemented instruction %x\n", *opcode);
    exit(1);
}

void Emulate8080Op(State8080* state)
{
    unsigned char* opcode = &state->memory[state->pc];
    state->pc+=1;
    switch(*opcode)
    {
    case 0x00:                               // NOP
        break;
    case 0x01:                               // LXI B,word
    {
        state->rc = opcode[1];
        state->rb = opcode[2];
        state->pc += 2;
        break;
    }
    case 0x02:                               // STAX B
    {
        state->memory[state->rbc] = state->ra;
        break;
    }
    case 0x03:                               // INX B
    {
        state->rbc += 1;
        break;
    }
    case 0x04:                               // INR B
    {
        inr(state, &state->rb);
        break;
    }
    case 0x05:                               // DCR B
    {
        dcr(state, &state->rb);
        break;
    }
    case 0x06:                               // MVI B,byte
    {
        state->rb = opcode[1];
        state->pc += 1;
        break;
    }
    case 0x07:                               // RLC
    {
        state->rf.c = (0x80 == (state->ra & 0x80));
        state->ra = (state->rf.c | (state->ra << 1));
        break;
    }
    case 0x08:
        UnimplementedInstruction(state);
        break;
    case 0x09:                               // DAD B
    {
        uint32_t res = state->rhl + state ->rbc;
        state->rf.c = ((res & 0xffff0000) != 0);
        state->rhl = res & 0xffff;
        break;
    }
    case 0x0A:                               // LDAX B
    {
        state->ra = state->memory[state->rbc];
        break;
    }
    case 0x0B:                               // DCX B
    {
        state->rbc -= 1;
        break;
    }
    case 0x0C:                               // INR C
    {
        inr(state, &state->rc);
        break;
    }
    case 0x0D:                               // DCR C
    {
        dcr(state, &state->rc);
        break;
    }
    case 0x0E:                               // MVI C,byte
    {
        state->rc = opcode[1];
        state->pc += 1;
        break;
    }
    case 0x0F:                               // RRC
    {
        state->rf.c = (0x1 == (state->ra & 0x1));
        state->ra = (state->rf.c << 7) | (state->ra >> 1);
        break;
    }
    case 0x10:
        UnimplementedInstruction(state);
        break;
    case 0x11:                               // LXI D,word
    {
        state->re = opcode[1];
        state->rd = opcode[2];
        state->pc += 2;
        break;
    }
    case 0x12:                               // STAX D
    {
        state->memory[state->rde] = state->ra;
        break;
    }
    case 0x13:                               // INX D
    {
        state->rde += 1;
        break;
    }
    case 0x14:                               // INR D
    {
        inr(state, &state->rd);
        break;
    }
    case 0x15:                               // DCR D
    {
        dcr(state, &state->rd);
        break;
    }
    case 0x16:                               // MVI D,byte
    {
        state->rd = opcode[1];
        state->pc += 1;
        break;
    }
    case 0x17:                               // RAL
    {
        uint8_t res = (state->rf.c | (state->ra << 1));
        state->rf.c = (0x80 == (state->ra & 0x80));
        state->ra = res;
        break;
    }
    case 0x18:
        UnimplementedInstruction(state);
        break;
    case 0x19:                               // DAD D
    {
        uint32_t res = state->rhl + state ->rde;
        state->rf.c = ((res & 0xffff0000) != 0);
        state->rhl = res & 0xffff;
        break;
    }
    case 0x1A:                               // LDAX D
    {
        state->ra = state->memory[state->rde];
        break;
    }
    case 0x1B:                               // DCX D
    {
        state->rde -= 1;
        break;
    }
    case 0x1C:                               // INR E
    {
        inr(state, &state->re);
        break;
    }
    case 0x1D:                               // DCR E
    {
        dcr(state, &state->re);
        break;
    }
    case 0x1E:                               // MVI E,byte
    {
        state->re = opcode[1];
        state->pc += 1;
        break;
    }
    case 0x1F:                               // RAR
    {
        uint8_t res = ((state->rf.c << 7) | (state->ra >> 1));
        state->rf.c = (0x1 == (state->ra & 0x1));
        state->ra = res;
        break;
    }
    case 0x20:
        UnimplementedInstruction(state);
        break;
    case 0x21:                               // LXI H,word
    {
        state->rl = opcode[1];
        state->rh = opcode[2];
        state->pc += 2;
        break;
    }
    case 0x22:                               // SHLD
    {
        state->memory[(opcode[2] << 8) | opcode[1]] = state->rl;
        state->memory[((opcode[2] << 8) | opcode[1]) + 1] = state->rh;
        state->pc += 2;
        break;
    }
    case 0x23:                               // INX H
    {
        state->rhl += 1;
        break;
    }
    case 0x24:                               // INR H
    {
        inr(state, &state->rh);
        break;
    }
    case 0x25:                               // DCR H
    {
        dcr(state, &state->rh);
        break;
    }
    case 0x26:                               // MVI H,byte
    {
        state->rh = opcode[1];
        state->pc += 1;
        break;
    }
    case 0x27:                               // DAA
    {
        if ((state->ra & 0xf) > 9)
        {
            state->ra += 6;
            state->rf.ac = 1;
        }
        else if (state->rf.ac)
        {
            state->ra += 6;
        }

        if (((state->ra >> 4) & 0xf) > 9)
        {
            state->ra += 0x60;
            state->rf.c = 1;
        }
        else if (state->rf.c)
        {
            state->ra += 0x60;
        }
        break;
    }
    case 0x28:
        UnimplementedInstruction(state);
        break;
    case 0x29:                               // DAD H
    {
        uint32_t res = state->rhl + state ->rhl;
        state->rf.c = ((res & 0xffff0000) != 0);
        state->rhl = res & 0xffff;
        break;
    }
    case 0x2A:                               // LHLD
    {
        state->rl = state->memory[(opcode[2] << 8) | opcode[1]];
        state->rh = state->memory[((opcode[2] << 8) | opcode[1]) + 1];
        state->pc += 2;
        break;
    }
    case 0x2B:                               // DCX H
    {
        state->rhl -= 1;
        break;
    }
    case 0x2C:                               // INR L
    {
        inr(state, &state->rl);
        break;
    }
    case 0x2D:                               // DCR L
    {
        dcr(state, &state->rl);
        break;
    }
    case 0x2E:                               // MVI L,byte
    {
        state->rl = opcode[1];
        state->pc += 1;
        break;
    }
    case 0x2F:                               // CMA
    {
        state->ra = ~state->ra;
        break;
    }
    case 0x30:
        UnimplementedInstruction(state);
        break;
    case 0x31:                               // LXI SP,word
    {
        state->sp = (opcode[2] << 8) | opcode[1];
        state->pc += 2;
        break;
    }
    case 0x32:                               // STA
    {
        state->memory[(opcode[2] << 8) | opcode[1]] = state->ra;
        state->pc += 2;
        break;
    }
    case 0x33:                               // INX SP
    {
        state->sp += 1;
        break;
    }
    case 0x34:                               // INR M
    {
        inr(state, &state->memory[state->rhl]);
        break;
    }
    case 0x35:                               // DCR M
    {
        dcr(state, &state->memory[state->rhl]);
        break;
    }
    case 0x36:                               // MVI M,byte
    {
        state->memory[state->rhl] = opcode[1];
        state->pc += 1;
        break;
    }
    case 0x37:                               // STC
    {
        state->rf.c = 1;
        break;
    }
    case 0x38:
        UnimplementedInstruction(state);
        break;
    case 0x39:                               // DAD SP
    {
        uint32_t res = state->rhl + state ->sp;
        state->rf.c = ((res & 0xffff0000) != 0);
        state->rhl = res & 0xffff;
        break;
    }
    case 0x3A:                               // LDA
    {
        state->ra = state->memory[(opcode[2] << 8) | opcode[1]];
        state->pc += 2;
        break;
    }
    case 0x3B:                               // DCX SP
    {
        state->sp -= 1;
        break;
    }
    case 0x3C:                               // INR A
    {
        inr(state, &state->ra);
        break;
    }
    case 0x3D:                               // DCR A
    {
        dcr(state, &state->ra);
        break;
    }
    case 0x3E:                               // MVI A,byte
    {
        state->ra = opcode[1];
        state->pc += 1;
        break;
    }
    case 0x3F:                               // CMC
    {
        state->rf.c = !state->rf.c;
        break;
    }
    case 0x40:                               // MOV B,B
    {
        state->rb = state->rb;
        break;
    }
    case 0x41:                               // MOV B,C
    {
        state->rb = state->rc;
        break;
    }
    case 0x42:                               // MOV B,D
    {
        state->rb = state->rd;
        break;
    }
    case 0x43:                               // MOV B,E
    {
        state->rb = state->re;
        break;
    }
    case 0x44:                               // MOV B,H
    {
        state->rb = state->rh;
        break;
    }
    case 0x45:                               // MOV B,H
    {
        state->rb = state->rl;
        break;
    }
    case 0x46:                               // MOV B,M
    {
        state->rb = state->memory[state->rhl];
        break;
    }
    case 0x47:                               // MOV B,A
    {
        state->rb = state->ra;
        break;
    }
    case 0x48:                               // MOV C,B
    {
        state->rc = state->rb;
        break;
    }
    case 0x49:                               // MOV C,C
    {
        state->rc = state->rc;
        break;
    }
    case 0x4A:                               // MOV C,D
    {
        state->rc = state->rd;
        break;
    }
    case 0x4B:                               // MOV C,E
    {
        state->rc = state->re;
        break;
    }
    case 0x4C:                               // MOV C,H
    {
        state->rc = state->rh;
        break;
    }
    case 0x4D:                               // MOV C,L
    {
        state->rc = state->rl;
        break;
    }
    case 0x4E:                               // MOV C,M
    {
        state->rc = state->memory[state->rhl];
        break;
    }
    case 0x4F:                               // MOV C,A
    {
        state->rc = state->ra;
        break;
    }
    case 0x50:                               // MOV D,B
    {
        state->rd = state->rb;
        break;
    }
    case 0x51:                               // MOV D,C
    {
        state->rd = state->rc;
        break;
    }
    case 0x52:                               // MOV D,D
    {
        state->rd = state->rd;
        break;
    }
    case 0x53:                               // MOV D,E
    {
        state->rd = state->re;
        break;
    }
    case 0x54:                               // MOV D,H
    {
        state->rd = state->rh;
        break;
    }
    case 0x55:                               // MOV D,L
    {
        state->rd = state->rl;
        break;
    }
    case 0x56:                               // MOV D,M
    {
        state->rd = state->memory[state->rhl];
        break;
    }
    case 0x57:                               // MOV D,A
    {
        state->rd = state->ra;
        break;
    }
    case 0x58:                               // MOV E,B
    {
        state->re = state->rb;
        break;
    }
    case 0x59:                               // MOV E,C
    {
        state->re = state->rc;
        break;
    }
    case 0x5A:                               // MOV E,D
    {
        state->re = state->rd;
        break;
    }
    case 0x5B:                               // MOV E,E
    {
        state->re = state->re;
        break;
    }
    case 0x5C:                               // MOV E,H
    {
        state->re = state->rh;
        break;
    }
    case 0x5D:                               // MOV E,L
    {
        state->re = state->rl;
        break;
    }
    case 0x5E:                               // MOV E,M
    {
        state->re = state->memory[state->rhl];
        break;
    }
    case 0x5F:                               // MOV E,A
    {
        state->re = state->ra;
        break;
    }
    case 0x60:                               // MOV H,B
    {
        state->rh = state->rb;
        break;
    }
    case 0x61:                               // MOV H,C
    {
        state->rh = state->rc;
        break;
    }
    case 0x62:                               // MOV H,D
    {
        state->rh = state->rd;
        break;
    }
    case 0x63:                               // MOV H,E
    {
        state->rh = state->re;
        break;
    }
    case 0x64:                               // MOV H,H
    {
        state->rh = state->rh;
        break;
    }
    case 0x65:                               // MOV H,L
    {
        state->rh = state->rl;
        break;
    }
    case 0x66:                               // MOV H,M
    {
        state->rh = state->memory[state->rhl];
        break;
    }
    case 0x67:                               // MOV H,A
    {
        state->rh = state->ra;
        break;
    }
    case 0x68:                               // MOV L,B
    {
        state->rl = state->rb;
        break;
    }
    case 0x69:                               // MOV L,C
    {
        state->rl = state->rc;
        break;
    }
    case 0x6A:                               // MOV L,D
    {
        state->rl = state->rd;
        break;
    }
    case 0x6B:                               // MOV L,E
    {
        state->rl = state->re;
        break;
    }
    case 0x6C:                               // MOV L,H
    {
        state->rl = state->rh;
        break;
    }
    case 0x6D:                               // MOV L,L
    {
        state->rl = state->rl;
        break;
    }
    case 0x6E:                               // MOV L,M
    {
        state->rl = state->memory[state->rhl];
        break;
    }
    case 0x6F:                               // MOV L,A
    {
        state->rl = state->ra;
        break;
    }
    case 0x70:                               // MOV M,B
    {
        state->memory[state->rhl] = state->rb;
        break;
    }
    case 0x71:                               // MOV M,C
    {
        state->memory[state->rhl] = state->rc;
        break;
    }
    case 0x72:                               // MOV M,D
    {
        state->memory[state->rhl] = state->rd;
        break;
    }
    case 0x73:                               // MOV M,E
    {
        state->memory[state->rhl] = state->re;
        break;
    }
    case 0x74:                               // MOV M,H
    {
        state->memory[state->rhl] = state->rh;
        break;
    }
    case 0x75:                               // MOV M,L
    {
        state->memory[state->rhl] = state->rl;
        break;
    }
    case 0x76:
        UnimplementedInstruction(state);
        break;
    case 0x77:                               // MOV M,A
    {
        state->memory[state->rhl] = state->ra;
        break;
    }
    case 0x78:                               // MOV A,B
    {
        state->ra = state->rb;
        break;
    }
    case 0x79:                               // MOV A,C
    {
        state->ra = state->rc;
        break;
    }
    case 0x7A:                               // MOV A,D
    {
        state->ra = state->rd;
        break;
    }
    case 0x7B:                               // MOV A,E
    {
        state->ra = state->re;
        break;
    }
    case 0x7C:                               // MOV A,H
    {
        state->ra = state->rh;
        break;
    }
    case 0x7D:                               // MOV A,L
    {
        state->ra = state->rl;
        break;
    }
    case 0x7E:                               // MOV A,M
    {
        state->ra = state->memory[state->rhl];
        break;
    }
    case 0x7F:                               // MOV A,A
    {
        state->ra = state->ra;
        break;
    }
    case 0x80:                               // ADD B
    {
        add8(state, state->ra, state->rb);
        break;
    }
    case 0x81:                               // ADD C
    {
        add8(state, state->ra, state->rc);
        break;
    }
    case 0x82:                               // ADD D
    {
        add8(state, state->ra, state->rd);
        break;
    }
    case 0x83:                               // ADD E
    {
        add8(state, state->ra, state->re);
        break;
    }
    case 0x84:                               // ADD H
    {
        add8(state, state->ra, state->rh);
        break;
    }
    case 0x85:                               // ADD L
    {
        add8(state, state->ra, state->rl);
        break;
    }
    case 0x86:                               // ADD M
    {
        add8(state, state->ra, state->memory[state->rhl]);
        break;
    }
    case 0x87:                               // ADD A
    {
        add8(state, state->ra, state->ra);
        break;
    }
    case 0x88:                               // ADC B
    {
        add8(state, state->ra, state->rb + state->rf.c);
        break;
    }
    case 0x89:                               // ADC C
    {
        add8(state, state->ra, state->rc + state->rf.c);
        break;
    }
    case 0x8A:                               // ADC D
    {
        add8(state, state->ra, state->rd + state->rf.c);
        break;
    }
    case 0x8B:                               // ADC E
    {
        add8(state, state->ra, state->re + state->rf.c);
        break;
    }
    case 0x8C:                               // ADC H
    {
        add8(state, state->ra, state->rh + state->rf.c);
        break;
    }
    case 0x8D:                               // ADC L
    {
        add8(state, state->ra, state->rl + state->rf.c);
        break;
    }
    case 0x8E:                               // ADC M
    {
        add8(state, state->ra, state->memory[state->rhl] + state->rf.c);
        break;
    }
    case 0x8F:                               // ADC A
    {
        add8(state, state->ra, state->ra + state->rf.c);
        break;
    }
    case 0x90:                               // SUB B
    {
        sub8(state, state->rb);
        break;
    }
    case 0x91:                               // SUB C
    {
        sub8(state, state->rc);
        break;
    }
    case 0x92:                               // SUB D
    {
        sub8(state, state->rd);
        break;
    }
    case 0x93:                               // SUB E
    {
        sub8(state, state->re);
        break;
    }
    case 0x94:                               // SUB H
    {
        sub8(state, state->rh);
        break;
    }
    case 0x95:                               // SUB L
    {
        sub8(state, state->rl);
        break;
    }
    case 0x96:                               // SUB M
    {
        sub8(state, state->memory[state->rhl]);
        break;
    }
    case 0x97:                               // SUB A
    {
        sub8(state, state->ra);
        break;
    }
    case 0x98:                               // SBB B
    {
        sub8(state, state->rb + state->rf.c);
        break;
    }
    case 0x99:                               // SBB C
    {
        sub8(state, state->rc + state->rf.c);
        break;
    }
    case 0x9A:                               // SBB D
    {
        sub8(state, state->rd + state->rf.c);
        break;
    }
    case 0x9B:                               // SBB E
    {
        sub8(state, state->re + state->rf.c);
        break;
    }
    case 0x9C:                               // SBB H
    {
        sub8(state, state->rh + state->rf.c);
        break;
    }
    case 0x9D:                               // SBB L
    {
        sub8(state, state->rl + state->rf.c);
        break;
    }
    case 0x9E:                               // SBB M
    {
        sub8(state, state->memory[state->rhl] + state->rf.c);
        break;
    }
    case 0x9F:                               // SBB A
    {
        sub8(state, state->ra + state->rf.c);
        break;
    }
    case 0xA0:                               // ANA B
    {
        state->ra = state->ra & state->rb;
        set_flags_logic(state);
        break;
    }
    case 0xA1:                               // ANA C
    {
        state->ra = state->ra & state->rc;
        set_flags_logic(state);
        break;
    }
    case 0xA2:                               // ANA D
    {
        state->ra = state->ra & state->rd;
        set_flags_logic(state);
        break;
    }
    case 0xA3:                               // ANA E
    {
        state->ra = state->ra & state->re;
        set_flags_logic(state);
        break;
    }
    case 0xA4:                               // ANA H
    {
        state->ra = state->ra & state->rh;
        set_flags_logic(state);
        break;
    }
    case 0xA5:                               // ANA L
    {
        state->ra = state->ra & state->rl;
        set_flags_logic(state);
        break;
    }
    case 0xA6:                               // ANA M
    {
        state->ra = state->ra & state->memory[state->rhl];
        set_flags_logic(state);
        break;
    }
    case 0xA7:                               // ANA A
    {
        state->ra = state->ra & state->ra;
        set_flags_logic(state);
        break;
    }
    case 0xA8:                               // XRA B
    {
        state->ra = state->ra ^ state->rb;
        set_flags_logic(state);
        break;
    }
    case 0xA9:                               // XRA C
    {
        state->ra = state->ra ^ state->rc;
        set_flags_logic(state);
        break;
    }
    case 0xAA:                               // XRA D
    {
        state->ra = state->ra ^ state->rd;
        set_flags_logic(state);
        break;
    }
    case 0xAB:                               // XRA E
    {
        state->ra = state->ra ^ state->re;
        set_flags_logic(state);
        break;
    }
    case 0xAC:                               // XRA H
    {
        state->ra = state->ra ^ state->rh;
        set_flags_logic(state);
        break;
    }
    case 0xAD:                               // XRA L
    {
        state->ra = state->ra ^ state->rl;
        set_flags_logic(state);
        break;
    }
    case 0xAE:                               // XRA M
    {
        state->ra = state->ra ^ state->memory[state->rhl];
        set_flags_logic(state);
        break;
    }
    case 0xAF:                               // XRA A
    {
        state->ra = state->ra ^ state->ra;
        set_flags_logic(state);
        break;
    }
    case 0xB0:                               // ORA B
    {
        state->ra = state->ra | state->rb;
        set_flags_logic(state);
        break;
    }
    case 0xB1:                               // ORA C
    {
        state->ra = state->ra | state->rc;
        set_flags_logic(state);
        break;
    }
    case 0xB2:                               // ORA D
    {
        state->ra = state->ra | state->rd;
        set_flags_logic(state);
        break;
    }
    case 0xB3:                               // ORA E
    {
        state->ra = state->ra | state->re;
        set_flags_logic(state);
        break;
    }
    case 0xB4:                               // ORA H
    {
        state->ra = state->ra | state->rh;
        set_flags_logic(state);
        break;
    }
    case 0xB5:                               // ORA L
    {
        state->ra = state->ra | state->rl;
        set_flags_logic(state);
        break;
    }
    case 0xB6:                               // ORA M
    {
        state->ra = state->ra | state->memory[state->rhl];
        set_flags_logic(state);
        break;
    }
    case 0xB7:                               // ORA A
    {
        state->ra = state->ra | state->ra;
        set_flags_logic(state);
        break;
    }
    case 0xB8:                               // CMP B
    {
        cmp(state, state->rb);
        break;
    }
    case 0xB9:                               // CMP C
    {
        cmp(state, state->rc);
        break;
    }
    case 0xBA:                               // CMP D
    {
        cmp(state, state->rd);
        break;
    }
    case 0xBB:                               // CMP E
    {
        cmp(state, state->re);
        break;
    }
    case 0xBC:                               // CMP H
    {
        cmp(state, state->rh);
        break;
    }
    case 0xBD:                               // CMP L
    {
        cmp(state, state->rl);
        break;
    }
    case 0xBE:                               // CMP M
    {
        cmp(state, state->memory[state->rhl]);
        break;
    }
    case 0xBF:                               // CMP A
    {
        cmp(state, state->ra);
        break;
    }
    case 0xC0:                               // RNZ
    {
        if (!state->rf.z)
        {
            state->pc = (state->memory[state->sp + 1] << 8) | state->memory[state->sp];
            state->sp += 2;
        }
        break;
    }
    case 0xC1:                               // POP B
    {
        state->rc = state->memory[state->sp];
        state->rb = state->memory[state->sp + 1];
        state->sp += 2;
        break;
    }
    case 0xC2:                               // JNZ addr
    {
        if (!state->rf.z)
        {
            state->pc = (opcode[2] << 8) | opcode[1];
        }
        else
        {
            state->pc += 2;
        }
        break;
    }
    case 0xC3:                               // JMP addr
    {
        state->pc = (opcode[2] << 8) | opcode[1];
        break;
    }
    case 0xC4:                               // CNZ addr
    {
        if (!state->rf.z)
        {
            uint16_t res = state->pc + 2;
            state->memory[state->sp - 1] = res >> 8;
            state->memory[state->sp - 2] = res & 0xff;
            state->sp = state->sp - 2;
            state->pc = (opcode[2] << 8) | opcode[1];
        }
        else
        {
            state->pc +=2;
        }
        break;
    }
    case 0xC5:                               // PUSH B
    {
        state->memory[state->sp - 1] = state->rb;
        state->memory[state->sp - 2] = state->rc;
        state->sp = state->sp - 2;
        break;
    }
    case 0xC6:                               // ADI byte
    {
        add8(state, state->ra, opcode[1]);
        state->pc +=1;
        break;
    }
    case 0xC7:                               // RST 0
    {
        state->memory[state->sp - 1] = state->pc >> 8;
        state->memory[state->sp - 2] = state->pc & 0xff;
        state->sp = state->sp - 2;
        state->pc = 0x0;
        break;
    }
    case 0xC8:                               // RZ
    {
        if (state->rf.z)
        {
            state->pc = (state->memory[state->sp + 1] << 8) | state->memory[state->sp];
            state->sp += 2;
        }
        break;
    }
    case 0xC9:                               // RET
    {
        state->pc = (state->memory[state->sp + 1] << 8) | state->memory[state->sp];
        state->sp += 2;
        break;
    }
    case 0xCA:                               // JZ addr
    {
        if (state->rf.z)
        {
            state->pc = (opcode[2] << 8) | opcode[1];
        }
        else
        {
            state->pc += 2;
        }
        break;
    }
    case 0xCB:
        UnimplementedInstruction(state);
        break;
    case 0xCC:                               // CZ addr
    {
        if (state->rf.z)
        {
            uint16_t res = state->pc + 2;
            state->memory[state->sp - 1] = res >> 8;
            state->memory[state->sp - 2] = res & 0xff;
            state->sp = state->sp - 2;
            state->pc = (opcode[2] << 8) | opcode[1];
        }
        else
        {
            state->pc +=2;
        }
        break;
    }
    case 0xCD:                               // CALL addr
    {
        uint16_t res = state->pc + 2;
        state->memory[state->sp - 1] = res >> 8;
        state->memory[state->sp - 2] = res & 0xff;
        state->sp = state->sp - 2;
        state->pc = (opcode[2] << 8) | opcode[1];
        break;
    }
    case 0xCE:                               // ACI byte
    {
        add8(state, state->ra, opcode[1] + state->rf.c);
        state->pc += 1;
        break;
    }
    case 0xCF:                               // RST 1
    {
        state->memory[state->sp - 1] = state->pc >> 8;
        state->memory[state->sp - 2] = state->pc & 0xff;
        state->sp = state->sp - 2;
        state->pc = 0x8;
        break;
    }
    case 0xD0:                               // RNC
    {
        if (!state->rf.c)
        {
            state->pc = (state->memory[state->sp + 1] << 8) | state->memory[state->sp];
            state->sp += 2;
        }
        break;
    }
    case 0xD1:                               // POP D
    {
        state->re = state->memory[state->sp];
        state->rd = state->memory[state->sp + 1];
        state->sp += 2;
        break;
    }
    case 0xD2:                               // JNC addr
    {
        if (!state->rf.c)
        {
            state->pc = (opcode[2] << 8) | opcode[1];
        }
        else
        {
            state->pc += 2;
        }
        break;
    }
    case 0xD3:                               // OUT byte // TODO
        {
            if (opcode[1] == 4)
            {
                state->portsout[opcode[1]] = state->ra;
                state->rshift = ((state->ra << 8) | (state->rshift >> 8));
            }
            else if (opcode[1] == 2)
            {
                state->portsout[opcode[1]] = state->ra & 0x7;
            }
            else
            {
                state->portsout[opcode[1]] = state->ra;
            }
            state->pc += 1;
            break;
        }
    case 0xD4:                               // CNC addr
    {
        if (!state->rf.c)
        {
            uint16_t res = state->pc + 2;
            state->memory[state->sp - 1] = res >> 8;
            state->memory[state->sp - 2] = res & 0xff;
            state->sp = state->sp - 2;
            state->pc = (opcode[2] << 8) | opcode[1];
        }
        else
        {
            state->pc +=2;
        }
        break;
    }
    case 0xD5:                               // PUSH D
    {
        state->memory[state->sp - 1] = state->rd;
        state->memory[state->sp - 2] = state->re;
        state->sp = state->sp - 2;
        break;
    }
    case 0xD6:                               // SUI byte
    {
        sub8(state, opcode[1]);
        state->pc += 1;
        break;
    }
    case 0xD7:                               // RST 2
    {
        state->memory[state->sp - 1] = state->pc >> 8;
        state->memory[state->sp - 2] = state->pc & 0xff;
        state->sp = state->sp - 2;
        state->pc = 0x10;
        break;
    }
    case 0xD8:                               // RC
    {
        if (state->rf.c)
        {
            state->pc = (state->memory[state->sp + 1] << 8) | state->memory[state->sp];
            state->sp += 2;
        }
        break;
    }
    case 0xD9:
        UnimplementedInstruction(state);
        break;
    case 0xDA:                               // JC addr
    {
        if (state->rf.c)
        {
            state->pc = (opcode[2] << 8) | opcode[1];
        }
        else
        {
            state->pc += 2;
        }
        break;
    }
    case 0xDB:                               // IN port
    {
        uint8_t port_num = opcode[1];
        if (port_num == 3)
        {
            state->ra = ((state->rshift >> (8 - (state->portsout[2] & 0x07))) & 0xff);
        }
        else
        {
            state->ra = state->portsin[port_num];
        }
        state->pc += 1;
        break;
    }
    case 0xDC:                               // CC addr
    {
        if (state->rf.c)
        {
            uint16_t res = state->pc + 2;
            state->memory[state->sp - 1] = res >> 8;
            state->memory[state->sp - 2] = res & 0xff;
            state->sp = state->sp - 2;
            state->pc = (opcode[2] << 8) | opcode[1];
        }
        else
        {
            state->pc +=2;
        }
        break;
    }
    case 0xDD:
        UnimplementedInstruction(state);
        break;
    case 0xDE:                               // SBI byte
    {
        sub8(state, opcode[1] + state->rf.c);
        state->pc += 1;
        break;
    }
    case 0xDF:                               // RST 3
    {
        state->memory[state->sp - 1] = state->pc >> 8;
        state->memory[state->sp - 2] = state->pc & 0xff;
        state->sp = state->sp - 2;
        state->pc = 0x18;
        break;
    }
    case 0xE0:                               // RPO
    {
        if (!state->rf.p)
        {
            state->pc = (state->memory[state->sp + 1] << 8) | state->memory[state->sp];
            state->sp += 2;
        }
        break;
    }
    case 0xE1:                               // POP H
    {
        state->rl = state->memory[state->sp];
        state->rh = state->memory[state->sp + 1];
        state->sp += 2;
        break;
    }
    case 0xE2:                               // JPO addr
    {
        if (!state->rf.p)
        {
            state->pc = (opcode[2] << 8) | opcode[1];
        }
        else
        {
            state->pc += 2;
        }
        break;
    }
    case 0xE3:                               // XTHL
    {
        uint8_t tmp1 = state->memory[state->sp];
        uint8_t tmp2 = state->memory[state->sp + 1];
        state->memory[state->sp] = state->rl;
        state->memory[state->sp + 1] = state->rh;
        state->rl = tmp1;
        state->rh = tmp2;
        break;
    }
    case 0xE4:                               // CPO addr
    {
        if (!state->rf.p)
        {
            uint16_t res = state->pc + 2;
            state->memory[state->sp - 1] = res >> 8;
            state->memory[state->sp - 2] = res & 0xff;
            state->sp = state->sp - 2;
            state->pc = (opcode[2] << 8) | opcode[1];
        }
        else
        {
            state->pc +=2;
        }
        break;
    }
    case 0xE5:                               // PUSH H
    {
        state->memory[state->sp - 1] = state->rh;
        state->memory[state->sp - 2] = state->rl;
        state->sp -= 2;
        break;
    }
    case 0xE6:                               // ANI byte
    {
        state->ra = state->ra & opcode[1];
        set_flags_logic(state);
        state->pc +=1;
        break;
    }
    case 0xE7:                               // RST 4
    {
        state->memory[state->sp - 1] = state->pc >> 8;
        state->memory[state->sp - 2] = state->pc & 0xff;
        state->sp = state->sp - 2;
        state->pc = 0x20;
        break;
    }
    case 0xE8:                               // RPE
    {
        if (state->rf.p)
        {
            state->pc = (state->memory[state->sp + 1] << 8) | state->memory[state->sp];
            state->sp += 2;
        }
        break;
    }
    case 0xE9:                               // PCHL
    {
        state->pc = state->rhl;
        break;
    }
    case 0xEA:                               // JPE addr
    {
        if (state->rf.p)
        {
            state->pc = (opcode[2] << 8) | opcode[1];
        }
        else
        {
            state->pc += 2;
        }
        break;
    }
    case 0xEB:                               // XCHG
    {
        uint16_t tmp = state->rde;
        state->rde = state->rhl;
        state->rhl = tmp;
        break;
    }
    case 0xEC:                               // CPE
    {
        if (state->rf.p)
        {
            uint16_t res = state->pc + 2;
            state->memory[state->sp - 1] = res >> 8;
            state->memory[state->sp - 2] = res & 0xff;
            state->sp = state->sp - 2;
            state->pc = (opcode[2] << 8) | opcode[1];
        }
        else
        {
            state->pc +=2;
        }
        break;
    }
    case 0xED:
        UnimplementedInstruction(state);
        break;
    case 0xEE:                               // XRI byte
    {
        state->ra = state->ra ^ opcode[1];
        set_flags_logic(state);
        state->pc += 1;
        break;
    }
    case 0xEF:                               // RST 5
    {
        state->memory[state->sp - 1] = state->pc >> 8;
        state->memory[state->sp - 2] = state->pc & 0xff;
        state->sp = state->sp - 2;
        state->pc = 0x28;
        break;
    }
    case 0xF0:                               // RP
    {
        if (!state->rf.s)
        {
            state->pc = (state->memory[state->sp + 1] << 8) | state->memory[state->sp];
            state->sp += 2;
        }
        break;
    }
    case 0xF1:                               // POP PSW
    {
        state->psw = (state->memory[state->sp] & 0xff);
        state->ra = state->memory[state->sp + 1];
        state->sp += 2;
        break;
    }
    case 0xF2:                               // JP addr
    {
        if (!state->rf.s)
        {
            state->pc = (opcode[2] << 8) | opcode[1];
        }
        else
        {
            state->pc += 2;
        }
        break;
    }
    case 0xF3:                               // DI
    {
        state->inte = 0;
        break;
    }
    case 0xF4:                               // CP addr
    {
        if (!state->rf.s)
        {
            uint16_t res = state->pc + 2;
            state->memory[state->sp - 1] = res >> 8;
            state->memory[state->sp - 2] = res & 0xff;
            state->sp = state->sp - 2;
            state->pc = (opcode[2] << 8) | opcode[1];
        }
        else
        {
            state->pc +=2;
        }
        break;
    }
    case 0xF5:                               // PUSH PSW
    {
        state->memory[state->sp - 1] = state->ra;
        state->memory[state->sp - 2] = (state->psw & 0xff);
        state->sp = state->sp - 2;
        break;
    }
    case 0xF6:                               // ORI byte
    {
        state->ra = state->ra | opcode[1];
        set_flags_logic(state);
        state->pc += 1;
        break;
    }
    case 0xF7:                               // RST 6
    {
        state->memory[state->sp - 1] = state->pc >> 8;
        state->memory[state->sp - 2] = state->pc & 0xff;
        state->sp = state->sp - 2;
        state->pc = 0x30;
        break;
    }
    case 0xF8:                               // RM
    {
        if (state->rf.s)
        {
            state->pc = (state->memory[state->sp + 1] << 8) | state->memory[state->sp];
            state->sp += 2;
        }
        break;
    }
    case 0xF9:                               // SPHL
    {
        state->sp = state->rhl;
        break;
    }
    case 0xFA:                               // JM addr
    {
        if (state->rf.s)
        {
            state->pc = (opcode[2] << 8) | opcode[1];
        }
        else
        {
            state->pc += 2;
        }
        break;
    }
    case 0xFB:                               // EI
    {
        state->inte = 1;
        break;
    }
    case 0xFC:                               // CM addr
    {
        if (state->rf.s)
        {
            uint16_t res = state->pc + 2;
            state->memory[state->sp - 1] = res >> 8;
            state->memory[state->sp - 2] = res & 0xff;
            state->sp = state->sp - 2;
            state->pc = (opcode[2] << 8) | opcode[1];
        }
        else
        {
            state->pc +=2;
        }
        break;
    }
    case 0xFD:
        UnimplementedInstruction(state);
        break;
    case 0xFE:                               // CPI addr
    {
        cmp(state, opcode[1]);
        state->pc += 1;
        break;
    }
    case 0xFF:                               // RST 7
    {
        state->memory[state->sp - 1] = state->pc >> 8;
        state->memory[state->sp - 2] = state->pc & 0xff;
        state->sp = state->sp - 2;
        state->pc = 0x38;
        break;
    }
    }
}
