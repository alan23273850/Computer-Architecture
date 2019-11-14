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

#ifndef FUNCTION_H
#define FUNCTION_H

#include <cstdio> // FILE

int print(const char *format, ...);
int fetch_bit(unsigned int instruction, int MSB, int LSB);
int reverse(int num);
FILE* fopen_safe(const char *pathname, const char *mode);
void process_invalid_testcases(const char *format, ...);
void print_registers();
void print_instruction(int instruction);
void write_hit_miss_counts_to_file();
void instruction_decomposition(int instruction);
int choose_instruction_to_execute(int instruction);

#endif // FUNCTION_H
