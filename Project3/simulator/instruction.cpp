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

#include "global.h" // cycle
#include "function.h" // process_invalid_testcases
#include "body.h" // find_instruction, fetch_data, write_data

void add()
{
    print("add $%d,$%d,$%d\n", d, s, t);
    reg[d] = reg[s] + reg[t];
    pc += 4;
}

void sub()
{
    print("sub $%d,$%d,$%d\n", d, s, t);
    reg[d] = reg[s] - reg[t];
    pc += 4;
}

void AND()
{
    print("and $%d,$%d,$%d\n", d, s, t);
    reg[d] = reg[s] & reg[t];
    pc += 4;
}

void OR()
{
    print("or $%d,$%d,$%d\n", d, s, t);
    reg[d] = reg[s] | reg[t];
    pc += 4;
}

void XOR()
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

    reg[t] = reg[s] + simm;
    pc += 4;
}

void lw()
{
    int simm = (imm << 16) >> 16; /* signed immediate */
    print("lw $%d,%d($%d)\n", t, simm, s);

    int addr = reg[s] + simm;
    if (addr<0 || addr>1020)
        process_invalid_testcases("In cycle %d: Address Overflow", cycle);
    if (addr%4 != 0)
        process_invalid_testcases("In cycle %d: Misalignment Error", cycle);

    reg[t] = fetch_data(addr, 1);
    pc += 4;
}

void lh()
{
    int simm = (imm << 16) >> 16; /* signed immediate */
    print("lh $%d,%d($%d)\n", t, simm, s);

    int addr = reg[s] + simm;
    if (addr<0 || addr>1022)
        process_invalid_testcases("In cycle %d: Address Overflow", cycle);
    if (addr%2 != 0)
        process_invalid_testcases("In cycle %d: Misalignment Error", cycle);

    reg[t] = fetch_data(addr, 2);
    pc += 4;
}

void lhu()
{
    int simm = (imm << 16) >> 16; /* signed immediate */
    print("lhu $%d,%d($%d)\n", t, simm, s);

    int addr = reg[s] + simm;
    if (addr<0 || addr>1022)
        process_invalid_testcases("In cycle %d: Address Overflow", cycle);
    if (addr%2 != 0)
        process_invalid_testcases("In cycle %d: Misalignment Error", cycle);

    reg[t] = fetch_data(addr, 3);
    pc += 4;
}

void lb()
{
    int simm = (imm << 16) >> 16; /* signed immediate */
    print("lb $%d,%d($%d)\n", t, simm, s);

    int addr = reg[s] + simm;
    if (addr<0 || addr>1023)
        process_invalid_testcases("In cycle %d: Address Overflow", cycle);

    reg[t] = fetch_data(addr, 4);
    pc += 4;
}

void lbu()
{
    int simm = (imm << 16) >> 16; /* signed immediate */
    print("lb $%d,%d($%d)\n", t, simm, s);

    int addr = reg[s] + simm;
    if (addr<0 || addr>1023)
        process_invalid_testcases("In cycle %d: Address Overflow", cycle);

    reg[t] = fetch_data(addr, 5);
    pc += 4;
}

void sw()
{
    int simm = (imm << 16) >> 16; /* signed immediate */
    print("sw $%d,%d($%d)\n", t, simm, s);

    int addr = reg[s] + simm;
    if (addr<0 || addr>1020)
        process_invalid_testcases("In cycle %d: Address Overflow", cycle);
    if (addr%4 != 0)
        process_invalid_testcases("In cycle %d: Misalignment Error", cycle);

    write_data(addr, 1, reg[t]);
    pc += 4;
}

void sh()
{
    int simm = (imm << 16) >> 16; /* signed immediate */
    print("lh $%d,%d($%d)\n", t, simm, s);

    int addr = reg[s] + simm;
    if (addr<0 || addr>1022)
        process_invalid_testcases("In cycle %d: Address Overflow", cycle);
    if (addr%2 != 0)
        process_invalid_testcases("In cycle %d: Misalignment Error", cycle);

    write_data(addr, 2, reg[t]);
    pc += 4;
}

void sb()
{
    int simm = (imm << 16) >> 16; /* signed immediate */
    print("sb $%d,%d($%d)\n", t, simm, s);

    int addr = reg[s] + simm;
    if (addr<0 || addr>1023)
        process_invalid_testcases("In cycle %d: Address Overflow", cycle);

    write_data(addr, 3, reg[t]);
    pc += 4;
}

void lui()
{
    print("lui $%d,%d\n", t, imm);
    reg[t] = imm << 16;
    pc += 4;
}

void andi()
{
    print("andi $%d,$%d,%d\n", t, s, imm);
    reg[t] = reg[s] & imm;
    pc += 4;
}

void ori()
{
    print("ori $%d,$%d,%d\n", t, s, imm);
    reg[t] = reg[s] | imm;
    pc += 4;
}

void nori()
{
    print("nori $%d,$%d,%d\n", t, s, imm);
    reg[t] = ~(reg[s] | imm);
    pc += 4;
}

void slti()
{
    int simm = (imm << 16) >> 16; /* signed immediate */
    print("slti $%d,$%d,%d\n", t, s, simm);

    reg[t] = (reg[s] < simm);
    pc += 4;
}

void beq()
{
    int simm = (imm << 16) >> 16; /* signed immediate */
    print("beq $%d,$%d,%d\n", s, t, simm);
    pc += 4;
    if (reg[s] == reg[t])
        pc += simm * 4;
}

void bne()
{
    int simm = (imm << 16) >> 16; /* signed immediate */
    print("bne $%d,$%d,%d\n", s, t, simm);
    pc += 4;
    if (reg[s] != reg[t])
        pc += simm * 4;
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
