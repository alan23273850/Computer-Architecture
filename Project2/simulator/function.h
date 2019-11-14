#ifndef FUNCTION_H
#define FUNCTION_H

#include <stdio.h> // FILE

int reverse(int num);
int print(const char *format, ...);
int fetch_bit(unsigned int instruction, int MSB, int LSB);
FILE* fopen_safe(const char *pathname, const char *mode);
void process_invalid_testcases(char message[]);
void set_nop_to_stage(int stage);
void print_registers();
void print_instructions_and_hazards();
int print_error_messages();
int generate_instruction_name_and_check_nop(int instruction, char str[]);

void add();
void sub();
void and();
void or();
void xor();
void nor();
void nand();
void slt();
void sll();
void srl();
void sra();
void jr();
void addi();
int lw();
int lh();
int lhu();
int lb();
int lbu();
int sw();
int sh();
int sb();
void lui();
void andi();
void ori();
void nori();
void slti();
void beq();
void bne();
void j();
void jal();

#endif  // FUNCTION_H
