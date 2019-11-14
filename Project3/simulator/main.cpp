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

#include "prepare.h"
#include "function.h"
#include "body.h" // find_instruction
#include "tlb.h"
#include "pagetable.h"
#include "cache.h"
#include "memory.h"
#include "disk.h"

FILE *fin, *fout, *frpt;
int pc;
int cycle;
int reg[32];

/* variables for instruction data */
int s, t, d, shamt, imm;

/* variables for TLB, page table, cache, memory, and disk */
TLB tlb[2];
PAGETABLE pagetable[2];
CACHE cache[2];
MEMORY memory[2];
DISK disk[2];

int main(int argc, char* argv[])
{
    /** Part 1: argument setting **/
    argument_setting(argc, argv);

    /** Part 2: Read the data image into the D-disk. **/
    read_Dimage_into_Ddisk();

    /** Part 3: Read the instruction image into the I-disk. **/
    read_Iimage_into_Idisk();

    /** Part 4: Execute the instructions. **/
    /* open the files */
    fout = fopen_safe("snapshot.rpt", "w");

    /* start executing */
    print("#5 The following is the tracing of the execution.\n\n");
    while (1) {
        /* PC validity check */
        if (pc<0 || pc>1023 || pc%4!=0)
            process_invalid_testcases("The program counter is not on a valid position.");

        /* cycle overflows interruption */
        if (cycle > 500000)
            process_invalid_testcases("The process has run for too many cycles.");

        /* print registers */
        print_registers();

        /* find instruction */
        int instruction = find_instruction(pc);

        /* register index setting */
        instruction_decomposition(instruction);

        /* print instruction */
        print_instruction(instruction);

        /* mapping from instruction bit to real operation */
        if (choose_instruction_to_execute(instruction))
            break;
    }

    /* report counts of hit and miss */
    write_hit_miss_counts_to_file();
    fclose(fout);
    return 0; 
}
