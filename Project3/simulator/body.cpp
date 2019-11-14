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

#include <cstring> // memcpy
#include "global.h" // variables
#include "function.h" // process_invalid_testcases

int access_data_from_address(int rw, int va, int mode, unsigned int data)
{
    int sel = !(rw == 0);
    int vpn = va / memory[sel].sizeP;

    int ppn;
    while ((ppn = tlb[sel].find_ppn(vpn)) == -1) { /* TLB miss */
        while ((ppn = pagetable[sel].find_ppn(vpn)) == -1) { /* page table miss */
            int old_is_valid;
            ppn = memory[sel].read_disk_into_memory(va, old_is_valid);
            if (old_is_valid) { /* to save time, we can omit this step if the selected page is new */
                pagetable[sel].disable(ppn);
                tlb[sel].disable(ppn);
                cache[sel].disable(ppn);
            }
            pagetable[sel].update(vpn, ppn);
        }
        /* should be page table hit here */
        tlb[sel].update(vpn, ppn);
    }
    /* should be TLB hit here */

    /* physical address decomposition in cache */
    int PA = (ppn*memory[sel].sizeP) + (va%memory[sel].sizeP);
    if (sel==0 && PA%4!=0)
        process_invalid_testcases("this instruction is misaligned");
    int tag = PA / cache[sel].sizeE / cache[sel].sizeB;
    int index = (PA / cache[sel].sizeB) % cache[sel].sizeE;
    int offset = PA % cache[sel].sizeB;

    int set;
    while ((set = cache[sel].find_corresponding_set(index, tag)) == -1) { /* cache miss */
        set = cache[sel].find_replace_set(index);

        /* if adopting write-back policy, we should remember to save the content of
         * cache back to memory. however this step can be omitted if write-through
         * policy is adopted. */

        /* load memory to the desired block */
        int base_byte = ((tag * cache[sel].sizeE) + index) * cache[sel].sizeB;
        int ppn = base_byte / memory[sel].sizeP;
        int offset = base_byte % memory[sel].sizeP;
        memcpy(cache[sel].entry[index].set[set].byte, /* to destination cache block */
               memory[sel].page[ppn].byte + offset, /* load from source memory page */
               cache[sel].sizeB * sizeof(char)); /* of one cache block size */
        cache[sel].entry[index].set[set].tag = tag;
        cache[sel].entry[index].set[set].valid = 1;
    }
    /* should be cache hit here */

    /* read data from cache or write data to cache */
    switch (rw) {
        case 0:
            return (cache[sel].entry[index].set[set].byte[offset+0] << 24)
                 | (cache[sel].entry[index].set[set].byte[offset+1] << 16)
                 | (cache[sel].entry[index].set[set].byte[offset+2] << 8)
                 | (cache[sel].entry[index].set[set].byte[offset+3] << 0);
        case 1:
            switch (mode) {
                case 1: /* lw */
                    if (offset%4 != 0)
                        process_invalid_testcases("this data address is misaligned");
                    return (cache[sel].entry[index].set[set].byte[offset+0] << 24)
                         | (cache[sel].entry[index].set[set].byte[offset+1] << 16)
                         | (cache[sel].entry[index].set[set].byte[offset+2] << 8)
                         | (cache[sel].entry[index].set[set].byte[offset+3] << 0);
                case 2: /* lh */
                case 3: /* lhu */ {
                    if (offset%2 != 0)
                        process_invalid_testcases("this data address is misaligned");
                    int answer = (cache[sel].entry[index].set[set].byte[offset+0] << 8)
                               | (cache[sel].entry[index].set[set].byte[offset+1] << 0);
                    if (mode == 2)
                        answer = (answer << 16) >> 16;
                    return answer;}
                case 4: /* lb */
                case 5: /* lbu */ {
                    int answer = cache[sel].entry[index].set[set].byte[offset];
                    if (mode == 4)
                        answer = (answer << 24) >> 24;
                    return answer;}
            }
        case 2:
            switch (mode) {
                case 1: /* sw */
                    if (offset%4 != 0)
                        process_invalid_testcases("this data address is misaligned");
                    cache[sel].entry[index].set[set].byte[offset+0] = (data & 0xFF000000) >> 24;
                    cache[sel].entry[index].set[set].byte[offset+1] = (data & 0x00FF0000) >> 16;
                    cache[sel].entry[index].set[set].byte[offset+2] = (data & 0x0000FF00) >> 8;
                    cache[sel].entry[index].set[set].byte[offset+3] = (data & 0x000000FF) >> 0;
                    break;
                case 2: /* sh */
                    if (offset%2 != 0)
                        process_invalid_testcases("this data address is misaligned");
                    cache[sel].entry[index].set[set].byte[offset+0] = (data & 0x0000FF00) >> 8;
                    cache[sel].entry[index].set[set].byte[offset+1] = (data & 0x000000FF) >> 0;
                    break;
                case 3: /* sb */
                    cache[sel].entry[index].set[set].byte[offset+0] = (data & 0x000000FF) >> 0;
                    break;
            }
            /* adopt write-through policy (write back to memory instantly) so that physical memory can be
             * replaced in the future without having to trace back to its corresponding cache blocks again. */
            int base_byte = ((tag * cache[sel].sizeE) + index) * cache[sel].sizeB;
            int ppn = base_byte / memory[sel].sizeP;
            int offset = base_byte % memory[sel].sizeP;
            memcpy(memory[sel].page[ppn].byte + offset, /* to destination memory page */
                   cache[sel].entry[index].set[set].byte, /* write from source cache block */
                   cache[sel].sizeB * sizeof(char)); /* of one cache block size */
    }
    return -1; /* useless answer just to satisfy the function format */
}

int find_instruction(int va)
{
    return access_data_from_address(0, va, 0, 0);
}

int fetch_data(int va, int mode)
{
    /** mode: 1=lw, 2=lh, 3=lhu, 4=lb, 5=lbu with the fetched data returned **/
    return access_data_from_address(1, va, mode, 0);
}

void write_data(int va, int mode, int data)
{
    /** mode: 1=sw, 2=sh, 3=sb **/
    access_data_from_address(2, va, mode, data);
}
