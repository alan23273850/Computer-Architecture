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

#ifndef GLOBAL_H
#define GLOBAL_H

#include <cstdio>
#include "tlb.h"
#include "pagetable.h"
#include "cache.h"
#include "memory.h"
#include "disk.h"

extern FILE *fin, *fout, *frpt;
extern int pc;
extern int cycle;
extern int reg[32];

/* variables for instruction data */
extern int s, t, d, shamt, imm;

/* variables for TLB, page table, cache, memory, and disk */
extern TLB tlb[2];
extern PAGETABLE pagetable[2];
extern CACHE cache[2];
extern MEMORY memory[2];
extern DISK disk[2];

#endif // GLOBAL_H
