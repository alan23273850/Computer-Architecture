/*
//
//                       _oo0oo_
//                      o8888888o
//                      88" . "88
//                      (| -_- |)
//                      0\  =  /0
//                    ___/`---'\___
//                  .' \\|     |// '.
//                 / \\|||  :  |||// \
//                / _||||| -:- |||||- \
//               |   | \\\  -  /// |   |
//               | \_|  ''\---/''  |_/ |
//               \  .-\__  '-'  ___/-. /
//             ___'. .'  /--.--\  `. .'___
//          ."" '<  `.___\_<|>_/___.' >' "".
//         | | :  `- \`.;`\ _ /`;.`/ - ` : | |
//         \  \ `_.   \_ __\ /__ _/   .-` /  /
//     =====`-.____`.___ \_____/___.-`___.-'=====
//                       `=---='
//
//
//     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//               佛祖保佑         永無BUG
//
//
//
*/

#include <cstdio> // fprintf
#include <cstdlib> // exit
#include <cstdarg>
#include "global.h" // cycle
#include "instruction.h" // instructions

// #define PRINT_MSG

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
    else {
        instruction <<= (31-MSB);
        instruction >>= (31-MSB + LSB);
        return instruction;
    }
}

/* Due to the endian encoding, we have to reverse the order of data we read in. */
int reverse(int num)
{
    return (fetch_bit(num, 7, 0) << 24)
         | (fetch_bit(num,15, 8) << 16)
         | (fetch_bit(num,23,16) <<  8)
         | (fetch_bit(num,31,24) <<  0);
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

void process_invalid_testcases(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);
    puts("\nThe testcase may be invalid!!");
    puts("Press ENTER to terminate the program...");
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
    for (int i=0; i<32; i++) {
        print("$%02d: 0x%08X ", i, reg[i]);
        if (i%4 == 3)
            printf("\n");
    }
    print(" PC: 0x%08X\n\n", pc);
    */
    cycle++;
}

void print_instruction(int instruction)
{
    print("index in C = %d, PC in simulator = %d\n", pc/4, pc);
    for (int i=31; i>=0; i--) {
        if (fetch_bit(instruction,i,i) == 1)
            print("1");
        else
            print("0");
        if (i==5 || i==10 || i==15 || i==20 || i==25)
            print(" ");
    }
    print(" ===>>> ");
}

void write_hit_miss_counts_to_file()
{
    frpt = fopen("report.rpt", "w");
    fprintf(frpt, "ICache :\n");
    fprintf(frpt, "# hits: %d\n", cache[0].hit - cache[0].miss);
    fprintf(frpt, "# misses: %u\n\n", cache[0].miss);
    fprintf(frpt, "DCache :\n");
    fprintf(frpt, "# hits: %d\n", cache[1].hit - cache[1].miss);
    fprintf(frpt, "# misses: %u\n\n", cache[1].miss);
    fprintf(frpt, "ITLB :\n");
    fprintf(frpt, "# hits: %d\n", tlb[0].hit - tlb[0].miss);
    fprintf(frpt, "# misses: %u\n\n", tlb[0].miss);
    fprintf(frpt, "DTLB :\n");
    fprintf(frpt, "# hits: %d\n", tlb[1].hit - tlb[1].miss);
    fprintf(frpt, "# misses: %u\n\n", tlb[1].miss);
    fprintf(frpt, "IPageTable :\n");
    fprintf(frpt, "# hits: %d\n", pagetable[0].hit - pagetable[0].miss);
    fprintf(frpt, "# misses: %u\n\n", pagetable[0].miss);
    fprintf(frpt, "DPageTable :\n");
    fprintf(frpt, "# hits: %d\n", pagetable[1].hit - pagetable[1].miss);
    fprintf(frpt, "# misses: %u\n\n", pagetable[1].miss);
    fclose(frpt);
}

void instruction_decomposition(int instruction)
{
    s = fetch_bit(instruction,25,21);
    t = fetch_bit(instruction,20,16);
    d = fetch_bit(instruction,15,11);
    shamt = fetch_bit(instruction,10,6);
    imm = fetch_bit(instruction,15,0);
}

int choose_instruction_to_execute(int instruction)
{
    /** return 1 if should be interrupted, return 0 otherwise. **/

    switch (fetch_bit(instruction,31,26)) {
        /* halt */
        case 0x3F: print("halt\n"); return 1;
        /* I-type */
        case 0x08: addi(); break;
        case 0x23: lw(); break;
        case 0x21: lh(); break;
        case 0x25: lhu(); break;
        case 0x20: lb(); break;
        case 0x24: lbu(); break;
        case 0x2B: sw(); break;
        case 0x29: sh(); break;
        case 0x28: sb(); break;
        case 0x0F: lui(); break;
        case 0x0C: andi(); break;
        case 0x0D: ori(); break;
        case 0x0E: nori(); break;
        case 0x0A: slti(); break;
        case 0x04: beq(); break;
        case 0x05: bne(); break;
        /* J-type */
        case 0x02: j(); break;
        case 0x03: jal(); break;
        /* R-type */
        case 0x00:
            switch (fetch_bit(instruction,5,0)) {
                case 0x20: add(); break;
                case 0x22: sub(); break;
                case 0x24: AND(); break;
                case 0x25: OR(); break;
                case 0x26: XOR(); break;
                case 0x27: nor(); break;
                case 0x28: nand(); break;
                case 0x2A: slt(); break;
                case 0x00: sll(); break;
                case 0x02: srl(); break;
                case 0x03: sra(); break;
                case 0x08: jr(); break;
                default: process_invalid_testcases("This is NOT an instruction!!");
            }
            break;
        default: process_invalid_testcases("This is NOT an instruction!!");
    }

    /* avoid write 0 error */
    reg[0] = 0;
    return 0;
}
