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

#include <cstdlib> // atoi
#include "global.h" // storage elements
#include "function.h" // process_invalid_testcases

void argument_setting(int argc, char* argv[])
{
    int memory_total_size[2];
    int cache_total_size[2];

    if (argc == 1) {
        memory_total_size[0] = 64;
        memory_total_size[1] = 32;
        memory[0].sizeP = 8;
        memory[1].sizeP = 16;
        cache_total_size[0] = 16;
        cache[0].sizeB = 4;
        cache[0].sizeS = 4;
        cache_total_size[1] = 16;
        cache[1].sizeB = 4;
        cache[1].sizeS = 1;
    }
    else if (argc == 11) {
        memory_total_size[0] = atoi(argv[1]);
        memory_total_size[1] = atoi(argv[2]);
        memory[0].sizeP = atoi(argv[3]);
        memory[1].sizeP = atoi(argv[4]);
        cache_total_size[0] = atoi(argv[5]);
        cache[0].sizeB = atoi(argv[6]);
        cache[0].sizeS = atoi(argv[7]);
        cache_total_size[1] = atoi(argv[8]);
        cache[1].sizeB = atoi(argv[9]);
        cache[1].sizeS = atoi(argv[10]);
    }
    else
        process_invalid_testcases("Input arguments are invalid.\n");

    disk[0].sizeP = memory[0].sizeP;
    disk[1].sizeP = memory[1].sizeP;
    pagetable[0].sizeE = 1024 / disk[0].sizeP;
    pagetable[1].sizeE = 1024 / disk[1].sizeP;
    tlb[0].sizeE = pagetable[0].sizeE / 4;
    tlb[1].sizeE = pagetable[1].sizeE / 4;
    cache[0].sizeE = cache_total_size[0] / cache[0].sizeS / cache[0].sizeB;
    cache[1].sizeE = cache_total_size[1] / cache[1].sizeS / cache[1].sizeB;
    memory[0].sizeE = memory_total_size[0] / memory[0].sizeP;
    memory[1].sizeE = memory_total_size[1] / memory[1].sizeP;

    pagetable[0].m = &(memory[0]);
    pagetable[1].m = &(memory[1]);
    cache[0].m = &(memory[0]);
    cache[1].m = &(memory[1]);
    memory[0].pt = &(pagetable[0]);
    memory[1].pt = &(pagetable[1]);
    memory[0].d = &(disk[0]);
    memory[1].d = &(disk[1]);
}

void read_Dimage_into_Ddisk()
{
    /* read file */
    fin = fopen_safe("dimage.bin", "rb");

    /* one instruction each loop */
    print("#1 The following is the data image in hexadecimal form.\n");
    int line = 0; int size = 2147483647;
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
                break;
            }
            for (int i=0; i<4; i++) /* load into the memory */
                disk[1].byte(4*(line-3) + i) = word >> (24-8*i);
        }
        /* If the given input is too much, stop reading. */
        if (line-3 >= size-1)
            break;
    }
    if (line-3 < size-1)
        process_invalid_testcases("The real data image input is less than the amount specified!!");
    print("\n");
    fclose(fin);

    /* print initial data disk */
    print("#2 The following is the initial data disk.\n");
    for (int i=0; i<256; i++)
        print("D-disk[%03d]=%02x%02x%02x%02x\n", i, disk[1].byte(4*i), disk[1].byte(4*i+1), disk[1].byte(4*i+2), disk[1].byte(4*i+3));
    print("\n");
}

void read_Iimage_into_Idisk()
{
    /* read file */
    fin = fopen_safe("iimage.bin", "rb");

    /* one instruction each loop */
    print("#3 The following is the instruction image in hexadecimal form.\n");
    int line = 0; int size = 2147483647;
    while (!feof(fin)) {
        ++line;
        int word;
        fread(&word, sizeof(int), 1, fin);
        word = reverse(word);
        print("line[%03d]: %08X\n", line, word);
        if (line == 1) {
            pc = word;    /* program counter initialization */
            if (pc<0 || pc>1023 || pc%4!=0)
                process_invalid_testcases("The initial program counter is not on a valid position.");
        }
        else if (line == 2) {
            size = word;    /* how many instructions */
            if (size <= 0)
                process_invalid_testcases("The nonpositive number of instructions is obviously invalid.");
        }
        else {
            if (pc/4 + (line-3) >= 256) {
                print("The instruction image input is too much for the I-memory!!\n");
                break;
            }
            for (int i=0; i<4; i++) /* load into the memory */
                disk[0].byte(pc+4*(line-3) + i) = word >> (24-8*i);
        }
        /* If the given input is too much, stop reading. */
        if (line-3 >= size-1)
            break;
    }
    if (line-3 < size-1)
        process_invalid_testcases("The real data image input is less than the amount specified!!");
    print("\n");
    fclose(fin);

    /* print instruction disk */
    print("#4 The following is the instruction disk.\n");
    for (int i=0; i<256; i++)
        print("I-disk[%03d]=%02x%02x%02x%02x\n", i, disk[0].byte(4*i), disk[0].byte(4*i+1), disk[0].byte(4*i+2), disk[0].byte(4*i+3));
    print("\n");
}
