#include <stdio.h> // fprintf
#include <stdlib.h> // exit
#include <stdarg.h>

// #define PRINT_MSG

extern FILE *fin, *fout, *ferr;
extern unsigned char Dmem[1024];    /* 1K bytes are 1024 bytes. */
extern unsigned int Imem[256];      /* 1K bytes are 256 words. */
extern int reg[32];                 /* There are 32 registers. */
extern int pc;                      /* program counter */
extern int cycle;                   /* record the current cycle */

/* variables for instruction data */
extern int cmd_idx;
extern int s, t, d, shamt, imm;

/* Due to the endian issue, we have to reverse the order of data we read in. */
int reverse(unsigned int num)
{
    int res = 0;
    for (int i=0; i<4; i++) {
        res <<= 8;
        res += num & 0xFF;
        num >>= 8;
    }
    return res;
}

/* I want to control the output using preprocessor directives. */
int print(const char *format, ...) {
    #ifdef PRINT_MSG
        va_list ap;
        va_start(ap, format);
        int ret = vprintf(format, ap);
        va_end(ap);
        return ret;
    #else
        return 0;
    #endif
}

FILE* fopen_safe(const char *pathname, const char *mode) {
    FILE *fptr = fopen(pathname, mode);
    if (!fptr) {
        fprintf(stderr, "The file \"%s\" cannot be opened!!\n", pathname);
        fprintf(stderr, "Press ENTER to terminate the program...\n");
        getchar();
        exit(0);
    }
    return fptr;
}

void process_invalid_testcases(char message[])
{
    fprintf(stderr, "%s\n", message);
    fprintf(stderr, "The test case may be invalid!!\n");
    fprintf(stderr, "Press ENTER to terminate the program...\n");
    getchar();
    exit(0);
}

void print_registers()
{
    /* for the output file */
    fprintf(fout, "cycle %d\n", cycle);
    for (int i=0; i<32; i++)
        fprintf(fout, "$%02d: 0x%08X\n", i, reg[i]);
    fprintf(fout, "PC: 0x%08X\n\n\n", pc);

    /* for the standard output
    print("cycle %d: \n", cycle);
    for (int i=0; i<32; i++)
    {
        print("$%02d: 0x%08X ", i, reg[i]);
        if (i%4 == 3)
            print("\n");
    }
    print(" PC: 0x%08X\n\n", pc);
    */

    cycle++; /* go to the next cycle */
}

void print_instructions()
{
    print("\nindex in C = %d, PC in simulator = %d\n", cmd_idx, pc);
    for (int i=0; i<32; i++) {
        if (((Imem[cmd_idx] >> (31-i)) & 1) == 1)
            print("1");
        else
            print("0");
        if (i==5 || i==10 || i==15 || i==20 || i==25)
            print(" ");
    }
    print(" ===>>> ");
}

void add()
{
    int temp = reg[s] + reg[t];    /* reg[d] = reg[s] + reg[t]; */
    /* The reason why we use temp instead of reg[d] here is because
     * s or t may be equal to d. This will affect the judgment below. */
    print("add $%d,$%d,$%d\n", d, s, t);
    if (reg[s]>0 && reg[t]>0 && temp<=0 || reg[s]<0 && reg[t]<0 && temp>=0) {
        fprintf(ferr, "In cycle %d: Number Overflow\n", cycle);
        print("In cycle %d: Number Overflow\n", cycle);
    }
    reg[d] = temp;
    pc += 4;
}

void sub()
{
    print("sub $%d,$%d,$%d\n", d, s, t);
    int temp = reg[s] + (-reg[t]); /* reg[d] = reg[s] + (-reg[t]); */
    /* The reason why we use temp instead of reg[d] here is because
     * s or t may be equal to d. This will affect the judgment below.
     * Besides, subtraction is replaced with addition with negation
     * due to spec requirement. The difference will occur when the
     * second operand is -2^32 = -2147483648 since -(-2^32) = -2^32
     * under the world of int32!
     * */
    if (reg[s]>0 && -reg[t]>0 && temp<=0 || reg[s]<0 && -reg[t]<0 && temp>=0) {
        fprintf(ferr, "In cycle %d: Number Overflow\n", cycle);
        print("In cycle %d: Number Overflow\n", cycle);
    }
    reg[d] = temp;
    pc += 4;
}

void and()
{
    print("and $%d,$%d,$%d\n", d, s, t);
    reg[d] = reg[s] & reg[t];
    pc += 4;
}

void or()
{
    print("or $%d,$%d,$%d\n", d, s, t);
    reg[d] = reg[s] | reg[t];
    pc += 4;
}

void xor()
{
    print("xor $%d,$%d,$%d\n", d, s, t);
    reg[d] = reg[s] ^ reg[t];
    pc += 4;
}

void nor()
{
    print("nor $%d,$%d,$%d\n", d, s, t);
    reg[d] = ~(reg[s] | reg[t]);
    pc += 4;
}

void nand()
{
    print("nand $%d,$%d,$%d\n", d, s, t);
    reg[d] = ~(reg[s] & reg[t]);
    pc += 4;
}

void slt()
{
    print("slt $%d,$%d,$%d\n", d, s, t);
    reg[d] = (reg[s] < reg[t]);
    pc += 4;
}

void sll()
{
    print("sll $%d,$%d,%d\n", d, t, shamt);
    reg[d] = reg[t] << shamt;
    pc += 4;
}

void srl()
{
    print("srl $%d,$%d,%d\n", d, t, shamt);
    reg[d] = (unsigned)reg[t] >> shamt;
    pc += 4;
}

void sra()
{
    print("sra $%d,$%d,%d\n", d, t, shamt);
    reg[d] = reg[t] >> shamt;
    pc += 4;
}

void jr()
{
    print("jr $%d\n", s);
    pc = reg[s];
}

void addi()
{
    int simm = (imm << 16) >> 16; /* signed immediate */
    print("addi $%d,$%d,%d\n", t, s, simm);
    if (t == 0) {
        fprintf(ferr, "In cycle %d: Write $0 Error\n", cycle);
        print("In cycle %d: Write $0 Error\n", cycle);
    }

    int temp = reg[s] + simm; /* reg[t] = reg[s] + simm; */
    /* The reason why we use temp instead of reg[t] here is because
     * s may be equal to t. This will affect the judgment below. */
    if (reg[s]>0 && simm>0 && temp<=0 || reg[s]<0 && simm<0 && temp>=0) {
        fprintf(ferr, "In cycle %d: Number Overflow\n", cycle);
        print("In cycle %d: Number Overflow\n", cycle);
    }
    reg[t] = temp;
    pc += 4;
}

int lw()
{
    char should_halt = 0; /* boolean variable */
    int simm = (imm << 16) >> 16; /* signed immediate */
    print("lw $%d,%d($%d)\n", t, simm, s);

    if (t == 0) {
        fprintf(ferr, "In cycle %d: Write $0 Error\n", cycle);
        print("In cycle %d: Write $0 Error\n", cycle);
    }
    int addr = reg[s] + simm;
    if (reg[s]>0 && simm>0 && addr<=0 || reg[s]<0 && simm<0 && addr>=0) {
        fprintf(ferr, "In cycle %d: Number Overflow\n", cycle);
        print("In cycle %d: Number Overflow\n", cycle);
    }
    if (addr<0 || addr>1020) {
        fprintf(ferr, "In cycle %d: Address Overflow\n", cycle);
        print("In cycle %d: Address Overflow\n", cycle);
        should_halt = 1;
    }
    if (addr%4 != 0) {
        fprintf(ferr, "In cycle %d: Misalignment Error\n", cycle);
        print("In cycle %d: Misalignment Error\n", cycle);
        should_halt = 1;
    }
    if (should_halt == 1)
        return 1;

    reg[t] = Dmem[addr + 0] << 24;
    reg[t] += Dmem[addr + 1] << 16;
    reg[t] += Dmem[addr + 2] << 8;
    reg[t] += Dmem[addr + 3] << 0;
    pc += 4;
    return 0;
}

int lh()
{
    char should_halt = 0; /* boolean variable */
    int simm = (imm << 16) >> 16; /* signed immediate */
    print("lh $%d,%d($%d)\n", t, simm, s);

    if (t == 0) {
        fprintf(ferr, "In cycle %d: Write $0 Error\n", cycle);
        print("In cycle %d: Write $0 Error\n", cycle);
    }
    int addr = reg[s] + simm;
    if (reg[s]>0 && simm>0 && addr<=0 || reg[s]<0 && simm<0 && addr>=0) {
        fprintf(ferr, "In cycle %d: Number Overflow\n", cycle);
        print("In cycle %d: Number Overflow\n", cycle);
    }
    if (addr<0 || addr>1022) {
        fprintf(ferr, "In cycle %d: Address Overflow\n", cycle);
        print("In cycle %d: Address Overflow\n", cycle);
        should_halt = 1;
    }
    if (addr%2 != 0) {
        fprintf(ferr, "In cycle %d: Misalignment Error\n", cycle);
        print("In cycle %d: Misalignment Error\n", cycle);
        should_halt = 1;
    }
    if (should_halt == 1)
        return 1;

    reg[t] = Dmem[addr + 0] << 8;
    reg[t] += Dmem[addr + 1] << 0;
    reg[t] = (reg[t] << 16) >> 16;
    pc += 4;
    return 0;
}

int lhu()
{
    char should_halt = 0; /* boolean variable */
    int simm = (imm << 16) >> 16; /* signed immediate */
    print("lhu $%d,%d($%d)\n", t, simm, s);

    if (t == 0) {
        fprintf(ferr, "In cycle %d: Write $0 Error\n", cycle);
        print("In cycle %d: Write $0 Error\n", cycle);
    }
    int addr = reg[s] + simm;
    if (reg[s]>0 && simm>0 && addr<=0 || reg[s]<0 && simm<0 && addr>=0) {
        fprintf(ferr, "In cycle %d: Number Overflow\n", cycle);
        print("In cycle %d: Number Overflow\n", cycle);
    }
    if (addr<0 || addr>1022) {
        fprintf(ferr, "In cycle %d: Address Overflow\n", cycle);
        print("In cycle %d: Address Overflow\n", cycle);
        should_halt = 1;
    }
    if (addr%2 != 0) {
        fprintf(ferr, "In cycle %d: Misalignment Error\n", cycle);
        print("In cycle %d: Misalignment Error\n", cycle);
        should_halt = 1;
    }
    if (should_halt == 1)
        return 1;

    reg[t] = Dmem[addr + 0] << 8;
    reg[t] += Dmem[addr + 1] << 0;
    pc += 4;
    return 0;
}

int lb()
{
    int simm = (imm << 16) >> 16; /* signed immediate */
    print("lb $%d,%d($%d)\n", t, simm, s);
    if (t == 0) {
        fprintf(ferr, "In cycle %d: Write $0 Error\n", cycle);
        print("In cycle %d: Write $0 Error\n", cycle);
    }
    int addr = reg[s] + simm;
    if (reg[s]>0 && simm>0 && addr<=0 || reg[s]<0 && simm<0 && addr>=0) {
        fprintf(ferr, "In cycle %d: Number Overflow\n", cycle);
        print("In cycle %d: Number Overflow\n", cycle);
    }
    if (addr<0 || addr>1023) {
        fprintf(ferr, "In cycle %d: Address Overflow\n", cycle);
        print("In cycle %d: Address Overflow\n", cycle);
        return 1;
    }
    reg[t] = (Dmem[addr] << 24) >> 24;
    pc += 4;
    return 0;
}

int lbu()
{
    int simm = (imm << 16) >> 16; /* signed immediate */
    print("lbu $%d,%d($%d)\n", t, simm, s);
    if (t == 0) {
        fprintf(ferr, "In cycle %d: Write $0 Error\n", cycle);
        print("In cycle %d: Write $0 Error\n", cycle);
    }
    int addr = reg[s] + simm;
    if (reg[s]>0 && simm>0 && addr<=0 || reg[s]<0 && simm<0 && addr>=0) {
        fprintf(ferr, "In cycle %d: Number Overflow\n", cycle);
        print("In cycle %d: Number Overflow\n", cycle);
    }
    if (addr<0 || addr>1023) {
        fprintf(ferr, "In cycle %d: Address Overflow\n", cycle);
        print("In cycle %d: Address Overflow\n", cycle);
        return 1;
    }
    reg[t] = Dmem[addr];
    pc += 4;
    return 0;
}

int sw()
{
    char should_halt = 0; /* boolean variable */
    int simm = (imm << 16) >> 16; /* signed immediate */
    print("sw $%d,%d($%d)\n", t, simm, s);
    int addr = reg[s] + simm;
    if (reg[s]>0 && simm>0 && addr<=0 || reg[s]<0 && simm<0 && addr>=0) {
        fprintf(ferr, "In cycle %d: Number Overflow\n", cycle);
        print("In cycle %d: Number Overflow\n", cycle);
    }
    if (addr<0 || addr>1020) {
        fprintf(ferr, "In cycle %d: Address Overflow\n", cycle);
        print("In cycle %d: Address Overflow\n", cycle);
        should_halt = 1;
    }
    if (addr%4 != 0) {
        fprintf(ferr, "In cycle %d: Misalignment Error\n", cycle);
        print("In cycle %d: Misalignment Error\n", cycle);
        should_halt = 1;
    }
    if (should_halt == 1)
        return 1;

    Dmem[addr + 0] = reg[t] >> 24;
    Dmem[addr + 1] = reg[t] >> 16;
    Dmem[addr + 2] = reg[t] >> 8;
    Dmem[addr + 3] = reg[t] >> 0;
    pc += 4;
    return 0;
}

int sh()
{
    char should_halt = 0; /* boolean variable */
    int simm = (imm << 16) >> 16; /* signed immediate */
    print("sh $%d,%d($%d)\n", t, simm, s);
    int addr = reg[s] + simm;
    if (reg[s]>0 && simm>0 && addr<=0 || reg[s]<0 && simm<0 && addr>=0) {
        fprintf(ferr, "In cycle %d: Number Overflow\n", cycle);
        print("In cycle %d: Number Overflow\n", cycle);
    }
    if (addr<0 || addr>1022) {
        fprintf(ferr, "In cycle %d: Address Overflow\n", cycle);
        print("In cycle %d: Address Overflow\n", cycle);
        should_halt = 1;
    }
    if (addr%2 != 0) {
        fprintf(ferr, "In cycle %d: Misalignment Error\n", cycle);
        print("In cycle %d: Misalignment Error\n", cycle);
        should_halt = 1;
    }
    if (should_halt == 1)
        return 1;

    Dmem[addr + 0] = (reg[t] & 0x0000FF00) >> 8;
    Dmem[addr + 1] = (reg[t] & 0x000000FF) >> 0;
    pc += 4;
    return 0;
}

int sb()
{
    int simm = (imm << 16) >> 16; /* signed immediate */
    print("sb $%d,%d($%d)\n", t, simm, s);
    int addr = reg[s] + simm;
    if (reg[s]>0 && simm>0 && addr<=0 || reg[s]<0 && simm<0 && addr>=0) {
        fprintf(ferr, "In cycle %d: Number Overflow\n", cycle);
        print("In cycle %d: Number Overflow\n", cycle);
    }
    if (addr<0 || addr>1023) {
        fprintf(ferr, "In cycle %d: Address Overflow\n", cycle);
        print("In cycle %d: Address Overflow\n", cycle);
        return 1;
    }
    Dmem[addr] = reg[t] & 0x000000FF;
    pc += 4;
    return 0;
}

void lui()
{
    if (t == 0) {
        fprintf(ferr, "In cycle %d: Write $0 Error\n", cycle);
        print("In cycle %d: Write $0 Error\n", cycle);
    }
    else
        reg[t] = imm << 16;
    pc += 4;
    print("lui $%d,%d\n", t, imm);
}

void andi()
{
    if (t == 0) {
        fprintf(ferr, "In cycle %d: Write $0 Error\n", cycle);
        print("In cycle %d: Write $0 Error\n", cycle);
    }
    else
        reg[t] = reg[s] & imm;
    pc += 4;
    print("andi $%d,$%d,%d\n", t, s, imm);
}

void ori()
{
    if (t == 0) {
        fprintf(ferr, "In cycle %d: Write $0 Error\n", cycle);
        print("In cycle %d: Write $0 Error\n", cycle);
    }
    else
        reg[t] = reg[s] | imm;
    pc += 4;
    print("ori $%d,$%d,%d\n", t, s, imm);
}

void nori()
{
    if (t == 0) {
        fprintf(ferr, "In cycle %d: Write $0 Error\n", cycle);
        print("In cycle %d: Write $0 Error\n", cycle);
    }
    else
        reg[t] = ~(reg[s] | imm);
    pc += 4;
    print("nori $%d,$%d,%d\n", t, s, imm);
}

void slti()
{
    int simm = imm; /* signed immediate */
    if (t == 0) {
        fprintf(ferr, "In cycle %d: Write $0 Error\n", cycle);
        print("In cycle %d: Write $0 Error\n", cycle);
    }
    else {
        simm = (simm << 16) >> 16;
        reg[t] = (reg[s] < simm);
        print("slti $%d,$%d,%d\n", t, s, simm);
    }
    pc += 4;
}

void beq()
{
    int simm = (imm << 16) >> 16; /* signed immediate */
    print("beq $%d,$%d,%d\n", s, t, simm);
    simm <<= 2;
    pc += 4;
    if (reg[s] == reg[t]) {
        int pc_after = pc + simm;
        if (pc>0 && simm>0 && pc_after<=0 || pc<0 && simm<0 && pc_after>=0) {
            fprintf(ferr, "In cycle %d: Number Overflow\n", cycle);
            print("In cycle %d: Number Overflow\n", cycle);
        }
        pc = pc_after;
    }
}

void bne()
{
    int simm = (imm << 16) >> 16; /* signed immediate */
    print("bne $%d,$%d,%d\n", s, t, simm);
    simm <<= 2;
    pc += 4;
    if (reg[s] != reg[t]) {
        int pc_after = pc + simm;
        if (pc>0 && simm>0 && pc_after<=0 || pc<0 && simm<0 && pc_after>=0) {
            print("In cycle %d: Number Overflow\n", cycle);
            fprintf(ferr, "In cycle %d: Number Overflow\n", cycle);
        }
        pc = pc_after;
    }
}

void j()
{
    print("j %d\n", imm);
    pc = ((pc+4) & 0xF0000000) | (imm << 2);
}

void jal()
{
    print("jal %d\n", imm);
    reg[31] = pc + 4;
    pc = ((pc+4) & 0xF0000000) | (imm << 2);
}
