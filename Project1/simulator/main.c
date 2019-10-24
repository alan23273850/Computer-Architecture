#include "function.h"

FILE *fin, *fout, *ferr;
unsigned char Dmem[1024];    /* 1K bytes are 1024 bytes. */
unsigned int Imem[256];      /* 1K bytes are 256 words. */
int reg[32];                 /* There are 32 registers. */
int pc;                      /* program counter */
int cycle;                   /* record the current cycle */

/* variables for instruction data */
int cmd_idx;
int s, t, d, shamt, imm, funct;

int main(int argc, char* argv[])
{
    /**********************************************************************************************************/
    /***************************** Part 1: Read the data image into the D-memory. *****************************/
    /* read file */
    fin = fopen_safe("dimage.bin", "rb");

    /* one instruction each loop */
    print("#1 The following is the data image in hexadecimal form.\n");
    int line = 0; int size = 2147483640;
    while (!feof(fin)) {
        ++line;
        int word;
        fread(&word, sizeof(int), 1, fin);
        word = reverse(word);
        print("line[%03d]: %08X\n", line, word);
        if (line == 1)
            reg[29] = word;    /* stack pointer */
        else if (line == 2)
            size = word;    /* how many data */
        else {
            if (line-3 >= 256) {
                print("The data image input is too much for the D-memory!!\n");
                print("The test case may be invalid!!\n");
                break;
            }
            for (int i=0; i<4; i++) /* load into the memory */
                Dmem[4*(line-3) + i] = word >> (24-8*i);
        }
        /* If the given input is too much, stop reading. */
        if (line-3 >= size-1)
            break;
    }
    if (line-3 < size-1) {
        print("The real data image input is less than the amount specified!!\n");
        print("The test case may be invalid!!\n");
    }
    print("\n");
    fclose(fin);

    /* print initial data memory */
    print("#2 The following is the initial data memory.\n");
    for (int i=0; i<256; i++)
        print("D-mem[%03d]=%02x%02x%02x%02x\n", i, Dmem[4*i], Dmem[4*i+1], Dmem[4*i+2], Dmem[4*i+3]);
    print("\n");
    /**********************************************************************************************************/
    /**********************************************************************************************************/

    /**********************************************************************************************************/
    /************************** Part 2: Read the instruction image into the I-memory. *************************/
    /* read file */
    fin = fopen_safe("iimage.bin", "rb");

    /* one instruction each loop */
    print("#3 The following is the instruction image in hexadecimal form.\n");
    line = 0; size = 2147483640;
    while (!feof(fin)) {
        ++line;
        int word;
        fread(&word, sizeof(int), 1, fin);
        word = reverse(word);
        print("line[%03d]: %08X\n", line, word);
        if (line == 1) {
            pc = word;    /* program counter initialization */
            if (pc<0 || pc>1023 || pc%4!=0)
                process_invalid_testcases("The program counter is not on a valid position.");
        }
        else if (line == 2) {
            size = word;    /* how many instructions */
            if (size < 0)
                process_invalid_testcases("The negative number of instructions is obviously invalid.");
        }
        else {
            if (pc/4 + (line-3) >= 256) {
                print("The instruction image input is too much for the I-memory!!\n");
                print("The test case may be invalid!!\n");
                break;
            }
            Imem[(pc/4) + (line-3)] = word;
        }
        /* If the given input is too much, stop reading. */
        if (line-3 >= size-1)
            break;
    }
    if (line-3 < size-1) {
        print("The real data image input is less than the amount specified!!\n");
        print("The test case may be invalid!!\n");
    }
    print("\n");
    fclose(fin);

    /* print initial instruction memory */
    print("#4 The following is the initial instruction memory.\n");
    for (int i=0; i<256; i++)
        print("I-mem[%03d]=%08X\n", i, Imem[i]);
    print("\n");
    /**********************************************************************************************************/
    /**********************************************************************************************************/

    /**********************************************************************************************************/
    /************************************ Part 3: Execute the instructions. ***********************************/
    /* open the files */
    fout = fopen_safe("snapshot.rpt", "w");
    ferr = fopen_safe("error_dump.rpt", "w");

    /* start executing */
    print("#5 The following is the tracing of the execution.\n");
    int run = 1;
    while (run) {
        /* PC validity check and instruction index calculation */
        if (pc<0 || pc>1023 || pc%4!=0)
            process_invalid_testcases("The program counter is not on a valid position.");
        cmd_idx = pc/4;

        /* infinite loop check */
        if (cycle > 500000)
            process_invalid_testcases("The testcase contains too many cycles...\n");

        /* print registers */
        print_registers();

        /* register number setting */
        s = (Imem[cmd_idx] << 6) >> 27;
        t = (Imem[cmd_idx] << 11) >> 27;
        d = (Imem[cmd_idx] << 16) >> 27;
        shamt = (Imem[cmd_idx] << 21) >> 27;
        imm = Imem[cmd_idx] & 0xFFFF;

        /* print instructions */
        print_instructions();

        /* decode instructions */
        switch (Imem[cmd_idx] >> 26) {
            /* halt */
            case 0x3F: print("halt\n"); run = 0; break;
            /* I-type */
            case 0x08: addi(); break;
            case 0x23: run = !lw(); break;
            case 0x21: run = !lh(); break;
            case 0x25: run = !lhu(); break;
            case 0x20: run = !lb(); break;
            case 0x24: run = !lbu(); break;
            case 0x2B: run = !sw(); break;
            case 0x29: run = !sh(); break;
            case 0x28: run = !sb(); break;
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
                funct = Imem[cmd_idx] & 63;
                if ((d==0) && (funct!=0x08) && ((Imem[cmd_idx]&0xFC1FFFFF)!=0)) { /* write 0 error occurs */
                    fprintf(ferr, "In cycle %d: Write $0 Error\n", cycle);
                    print("In cycle %d: Write $0 Error\n", cycle);
                }
                switch (funct) {
                    case 0x20: add(); break;
                    case 0x22: sub(); break;
                    case 0x24: and(); break;
                    case 0x25: or(); break;
                    case 0x26: xor(); break;
                    case 0x27: nor(); break;
                    case 0x28: nand(); break;
                    case 0x2A: slt(); break;
                    case 0x00: sll(); break;
                    case 0x02: srl(); break;
                    case 0x03: sra(); break;
                    case 0x08: jr(); break;
                    default: process_invalid_testcases("This is NOT an instruction!!"); break;
                }
                break;
            default: process_invalid_testcases("This is NOT an instruction!!");  break;
        }
        reg[0] = 0;      /* avoid write 0 error */
    }
    /**********************************************************************************************************/
    /**********************************************************************************************************/

    fclose(fout);
    fclose(ferr);
    return 0;
}
