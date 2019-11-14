#ifndef UNIT_H
#define UNIT_H

int ALU_control(int op, int funct);
int ALU(int in1, int in2, int control_bits);
void control_unit(int op, int funct, int out[]);
int hazard_detection_unit(int IF_ID_Use_rs, int IF_ID_rs, int IF_ID_Use_rt, int IF_ID_rt,
                          int ID_EX_Use_rs, int ID_EX_rs, int ID_EX_Use_rt, int ID_EX_rt, int ID_EX_isMemRead, int ID_EX_isRegWrite, int ID_EX_RegDst,
                          int EX_MEM_isMemRead, int EX_MEM_isRegWrite, int EX_MEM_RegDst,
                          int MEM_WB_isRegWrite, int MEM_WB_RegDst,
                          int forward[]);

#endif  // UNIT_H
