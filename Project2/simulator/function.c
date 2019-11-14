#include <stdlib.h> // exit
#include <string.h> // memset
#include "function.h" // so many instructions

// #define PRINT_MSG

extern FILE *fin, *fout, *ferr;
extern int reg[32];               /* There are 32 registers. */
extern int pc;                    /* program counter */
extern int cycle;                 /* record the current cycle */

/* variables for instruction data */
extern int s, t, d, shamt, imm, funct;

/* variables for middle registers between stages */
extern int IF_ID[3], ID_EX[14], EX_MEM[6], MEM_WB[6];

/* instruction and hazard messages */
extern char instruction_name[4][30], instruction_saved[5][30];
extern char error_message[4][50];
extern char hazards[3][5][30];
extern int hazards_size[3];

/* Due to the endian issue, we have to reverse the order of data we read in. */
int reverse(int num)
{
    return (fetch_bit(num, 7, 0) << 24)
         | (fetch_bit(num,15, 8) << 16)
         | (fetch_bit(num,23,16) <<  8)
         | (fetch_bit(num,31,24) <<  0);
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

int fetch_bit(unsigned int instruction, int MSB, int LSB)
{
    if (MSB < LSB)
        return 0;
    else
    {
        instruction <<= (31-MSB);
        instruction >>= (31-MSB + LSB);
        return instruction;
    }
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

void set_nop_to_stage(int stage) {
    switch (stage) {
        case 0: /* IF_ID */
            strcpy(instruction_name[0], "NOP");
            memset(&IF_ID, 0, sizeof(IF_ID));
            IF_ID[0] = 2; /* indicate NOP to avoid write $0 error */
            break;
        case 1: /* ID_EX */
            strcpy(instruction_name[1], "NOP");
            memset(&ID_EX, 0, sizeof(ID_EX));
            ID_EX[0] = 2; /* indicate NOP to avoid write $0 error */
            break;
        case 2: /* EX_MEM */
            strcpy(instruction_name[2], "NOP");
            memset(&EX_MEM, 0, sizeof(EX_MEM));
            EX_MEM[0] = 2; /* indicate NOP to avoid write $0 error */
            break;
        case 3: /* MEM_WB */
            strcpy(instruction_name[3], "NOP");
            memset(&MEM_WB, 0, sizeof(MEM_WB));
            MEM_WB[0] = 2; /* indicate NOP to avoid write $0 error */
            break;
    }
}

void print_registers()
{
    /* for the output file */
    fprintf(fout, "cycle %d\n", cycle);
    for (int i=0; i<32; i++)
        fprintf(fout, "$%02d: 0x%08X\n", i, reg[i]);
    fprintf(fout, "PC: 0x%08X\n", pc);

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

void print_instructions_and_hazards()
{
    /* for file output */
    fprintf(fout, "IF: %s", instruction_saved[0]);
    for (int i=0; i<hazards_size[0]; i++)
        fprintf(fout, " %s", hazards[0][i]);
    fprintf(fout, "\n");
    fprintf(fout, "ID: %s", instruction_saved[1]);
    for (int i=0; i<hazards_size[1]; i++)
        fprintf(fout, " %s", hazards[1][i]);
    fprintf(fout, "\n");
    fprintf(fout, "EX: %s", instruction_saved[2]);
    for (int i=0; i<hazards_size[2]; i++)
        fprintf(fout, " %s", hazards[2][i]);
    fprintf(fout, "\n");
    fprintf(fout, "DM: %s\n", instruction_saved[3]);
    fprintf(fout, "WB: %s\n\n\n", instruction_saved[4]);

    /* for standard output
    print("IF: %s", instruction_saved[0]);
    for (int i=0; i<hazards_size[0]; i++)
        print(" %s", hazards[0][i]);
    print("\n");
    print("ID: %s", instruction_saved[1]);
    for (int i=0; i<hazards_size[1]; i++)
        print(" %s", hazards[1][i]);
    print("\n");
    print("EX: %s", instruction_saved[2]);
    for (int i=0; i<hazards_size[2]; i++)
        print(" %s", hazards[2][i]);
    print("\n");
    print("DM: %s\n", instruction_saved[3]);
    print("WB: %s\n\n\n", instruction_saved[4]);
    */
}

int print_error_messages() {
    int halt = 0;
    for (int i=0; i<4; i++)
        if (strlen(error_message[i]) > 0) {
            fprintf(ferr, "%s\n", error_message[i]);
            if (i==1 || i==2) /* memory address overflow or data misaligned */
                halt = 1;
        }
    return halt;
}

/**
    This function is used in debugging mode. It prints the instruction fetched in IF stage
    and also check whether the fetched bits is indeed an instruction or not.
    Its return value shows if it is NOP instruction or not.
**/
int generate_instruction_name_and_check_nop(int instruction, char str[])
{
    s = fetch_bit(instruction,25,21);
    t = fetch_bit(instruction,20,16);
    d = fetch_bit(instruction,15,11);
    shamt = fetch_bit(instruction,10,6);
    imm = fetch_bit(instruction,15,0);
    funct = fetch_bit(instruction,5,0);

    switch (fetch_bit(instruction,31,26)) {
        /* halt */
        case 0x3F: strcpy(str,"HALT"); return 0;
        /* I-type */
        case 0x08: addi(); strcpy(str,"ADDI"); return 0;
        case 0x23: lw(); strcpy(str,"LW"); return 0;
        case 0x21: lh(); strcpy(str,"LH"); return 0;
        case 0x25: lhu(); strcpy(str,"LHU"); return 0;
        case 0x20: lb(); strcpy(str,"LB"); return 0;
        case 0x24: lbu(); strcpy(str,"LBU"); return 0;
        case 0x2B: sw(); strcpy(str,"SW"); return 0;
        case 0x29: sh(); strcpy(str,"SH"); return 0;
        case 0x28: sb(); strcpy(str,"SB"); return 0;
        case 0x0F: lui(); strcpy(str,"LUI"); return 0;
        case 0x0C: andi(); strcpy(str,"ANDI"); return 0;
        case 0x0D: ori(); strcpy(str,"ORI"); return 0;
        case 0x0E: nori(); strcpy(str,"NORI"); return 0;
        case 0x0A: slti(); strcpy(str,"SLTI"); return 0;
        case 0x04: beq(); strcpy(str,"BEQ"); return 0;
        case 0x05: bne(); strcpy(str,"BNE"); return 0;
        /* J-type */
        case 0x02: j(); strcpy(str,"J"); return 0;
        case 0x03: jal(); strcpy(str,"JAL"); return 0;
        /* R-type */
        case 0x00:
            switch (funct) {
                case 0x20: add(); strcpy(str,"ADD"); return 0;
                case 0x22: sub(); strcpy(str,"SUB"); return 0;
                case 0x24: and(); strcpy(str,"AND"); return 0;
                case 0x25: or(); strcpy(str,"OR"); return 0;
                case 0x26: xor(); strcpy(str,"XOR"); return 0;
                case 0x27: nor(); strcpy(str,"NOR"); return 0;
                case 0x28: nand(); strcpy(str,"NAND"); return 0;
                case 0x2A: slt(); strcpy(str,"SLT"); return 0;
                case 0x00:
                    if (fetch_bit(instruction,20,6) == 0) { /* precedence of == is higher than &, so the parentheses is important!! */
                        strcpy(str,"NOP");
                        return 1;
                    }
                    else {
                        sll();
                        strcpy(str,"SLL");
                        return 0;
                    }
                case 0x02: srl(); strcpy(str,"SRL"); return 0;
                case 0x03: sra(); strcpy(str,"SRA"); return 0;
                case 0x08: jr(); strcpy(str,"JR"); return 0;
                default: process_invalid_testcases("This is NOT an instruction!!"); return 0;
            }
        default: process_invalid_testcases("This is NOT an instruction!!"); return 0;
    }
}

/** The following functions are used to translate the fetched instruction into assembly language. **/
void add()
{
    print("add $%d,$%d,$%d\n", d, s, t);
}

void sub()
{
    print("sub $%d,$%d,$%d\n", d, s, t);
}

void and()
{
    print("and $%d,$%d,$%d\n", d, s, t);
}

void or()
{
    print("or $%d,$%d,$%d\n", d, s, t);
}

void xor()
{
    print("xor $%d,$%d,$%d\n", d, s, t);
}

void nor()
{
    print("nor $%d,$%d,$%d\n", d, s, t);
}

void nand()
{
    print("nand $%d,$%d,$%d\n", d, s, t);
}

void slt()
{
    print("slt $%d,$%d,$%d\n", d, s, t);
}

void sll()
{
    print("sll $%d,$%d,%d\n", d, t, shamt);
}

void srl()
{
    print("srl $%d,$%d,%d\n", d, t, shamt);
}

void sra()
{
    print("sra $%d,$%d,%d\n", d, t, shamt);
}

void jr()
{
    print("jr $%d\n", s);
}

void addi()
{
    print("addi $%d,$%d,%d\n", t, s, (imm << 16) >> 16);
}

int lw()
{
    print("lw $%d,%d($%d)\n", t, (imm << 16) >> 16, s);
    return 0;
}

int lh()
{
    print("lh $%d,%d($%d)\n", t, (imm << 16) >> 16, s);
    return 0;
}

int lhu()
{
    print("lhu $%d,%d($%d)\n", t, (imm << 16) >> 16, s);
    return 0;
}

int lb()
{
    print("lb $%d,%d($%d)\n", t, (imm << 16) >> 16, s);
    return 0;
}

int lbu()
{
    print("lbu $%d,%d($%d)\n", t, (imm << 16) >> 16, s);
    return 0;
}

int sw()
{
    print("sw $%d,%d($%d)\n", t, (imm << 16) >> 16, s);
    return 0;
}

int sh()
{
    print("sh $%d,%d($%d)\n", t, (imm << 16) >> 16, s);
    return 0;
}

int sb()
{
    print("sb $%d,%d($%d)\n", t, (imm << 16) >> 16, s);
    return 0;
}

void lui()
{
    print("lui $%d,%d\n", t, imm);
}

void andi()
{
    print("andi $%d,$%d,%d\n", t, s, imm);
}

void ori()
{
    print("ori $%d,$%d,%d\n", t, s, imm);
}

void nori()
{
    print("nori $%d,$%d,%d\n", t, s, imm);
}

void slti()
{
    print("slti $%d,$%d,%d\n", t, s, (imm << 16) >> 16);
}

void beq()
{
    print("beq $%d,$%d,%d\n", s, t, (imm << 16) >> 16);
}

void bne()
{
    print("bne $%d,$%d,%d\n", s, t, (imm << 16) >> 16);
}

void j()
{
    print("j %d\n", imm);
}

void jal()
{
    print("jal %d\n", imm);
}
