#include <string.h> // memset
#include "function.h" // fetch_bit

extern char error_message[4][50];
extern char hazards[3][5][30];
extern int hazards_size[3];
extern int cycle;

int ALU_control(int op, int funct)
{
    int op_in_I_type_has_add[] = {0x08,0x23,0x21,0x25,0x20,0x24,0x2B,0x29,0x28};
    int funct_in_R_type[] = {0x20,0x22,0x24,0x25,0x26,0x27,0x28,0x2A,0x00,0x02,0x03};

    for (int i=0; i<sizeof(op_in_I_type_has_add)/sizeof(int); i++) /* op has addition in I type */
        if (op == op_in_I_type_has_add[i])
            return 1;
    if (op == 0x00) {
        for (int i=0; i<sizeof(funct_in_R_type)/sizeof(int); i++) /* other operations in R type */
            if (funct == funct_in_R_type[i])
                return i + 1; /* return 1-12 */
        return 0; /* no operation */
    }
    else if (op == 0x0F) /* lui */
        return 12;
    else if (op == 0x0C) /* andi */
        return 13;
    else if (op == 0x0D) /* ori */
        return 14;
    else if (op == 0x0E) /* nori */
        return 15;
    else if (op == 0x0A) /* slti, signed comparison */
        return 8;
    else
        return 0; /* no operation */
}

int ALU(int in1, int in2, int control_bits)
{
    switch (control_bits) {
        case 1:
            if (in1>0 && in2>0 && in1+in2<=0 || in1<0 && in2<0 && in1+in2>=0)
                sprintf(error_message[3], "In cycle %d: Number Overflow", cycle);
            return in1 + in2; /* add */
        case 2:
            if (in1>0 && (-in2)>0 && in1+(-in2)<=0 || in1<0 && (-in2)<0 && in1+(-in2)>=0)
                sprintf(error_message[3], "In cycle %d: Number Overflow", cycle);
            return in1 + (-in2); /* sub */
        case 3: return in1 & in2; /* and */
        case 4: return in1 | in2; /* or */
        case 5: return in1 ^ in2; /* xor */
        case 6: return ~(in1 | in2); /* nor */
        case 7: return ~(in1 & in2); /* nand */
        case 8: return in1 < in2; /* slt */
        case 9: return in1 << fetch_bit(in2,10,6); /* sll */
        case 10: return (unsigned)in1 >> fetch_bit(in2,10,6); /* srl */
        case 11: return in1 >> fetch_bit(in2,10,6); /* sra */
        case 12: return in2 << 16; /* lui */
        case 13: return in1 & (in2 & 0xFFFF); /* andi */
        case 14: return in1 | (in2 & 0xFFFF); /* ori */
        case 15: return ~(in1 | (in2 & 0xFFFF)); /* nori */
        default: return -1; /* no operation */
    }
}

/* output: 0.Reg-Write, 1.Mem-Control, 2.ALU-Control, 3.ALU-Source, 4.Jump, 5.Branch, 6.$rs, 7.$rt */
void control_unit(int op, int funct, int out[])
{
    /* Reg-Write */
    out[0] = (op==0x00) && (funct==0x20 || funct==0x22 || funct==0x24 || funct==0x25 || funct==0x26 || funct==0x27 || funct==0x28 || funct==0x2A || funct==0x00 || funct==0x02 || funct==0x03)
            || op==0x08 || op==0x23 || op==0x21 || op==0x25 || op==0x20 || op==0x24 || op==0x0F || op==0x0C || op==0x0D || op==0x0E || op==0x0A || op==0x03;
    if (out[0])
        out[0] += (op==0x00) + ((op==0x03) << 1);

    /* Mem-Control */
    int Mem_Control_Opcode[] = {0x2B,0x29,0x28, /* 前 3 個是 write */
                                0x23,0x21,0x25,0x20,0x24}; /* 後 5 個是 read */
    out[1] = 0;
    for (int i=0; i<sizeof(Mem_Control_Opcode)/sizeof(int); i++)
        if (op == Mem_Control_Opcode[i])
            out[1] = i + 1;

    /* ALU-Control */
    out[2] = ALU_control(op, funct);

    /* ALU-Source */
    out[3] = ((op == 0) && (funct==0x00 || funct==0x02 || funct==0x03)) << 1;
    out[3] += (op == 0) && (funct==0x20 || funct==0x22 || funct==0x24 || funct==0x25 || funct==0x26 || funct==0x27 || funct==0x28 || funct==0x2A);

    /* want to use $rs */
    out[4] = (((op==0x00) && (funct==0x20 || funct==0x22 || funct==0x24 || funct==0x25 || funct==0x26 || funct==0x27 || funct==0x28 || funct==0x2A)
            || op==0x08 || op==0x23 || op==0x21 || op==0x25 || op==0x20 || op==0x24 || op==0x2B || op==0x29 || op==0x28
            || op==0x0C || op==0x0D || op==0x0E || op==0x0A) << 1) /* only EX stage (ALU input) */
            + (op==0x00 && funct==0x08 || op==0x04 || op==0x05); /* only ID stage (beq, bne, jr) */

    /* want to use $rt */
    out[5] = (((op==0x00) && (funct==0x20 || funct==0x22 || funct==0x24 || funct==0x25 || funct==0x26 || funct==0x27 || funct==0x28 || funct==0x2A || funct==0x00 || funct==0x02 || funct==0x03)
            || op==0x2B || op==0x29 || op==0x28) << 1) /* only EX stage (ALU input) */
            + (op==0x04 || op==0x05); /* only ID stage (beq, bne) */

    /* Jump */
    out[6] = (op==0x02 || op==0x03) + ((op==0x00 && funct==0x08) << 1);

    /* Branch */
    out[7] = (op==0x04) + ((op==0x05) << 1);
}

int hazard_detection_unit(int IF_ID_Use_rs, int IF_ID_rs, int IF_ID_Use_rt, int IF_ID_rt,
                          int ID_EX_Use_rs, int ID_EX_rs, int ID_EX_Use_rt, int ID_EX_rt, int ID_EX_isMemRead, int ID_EX_isRegWrite, int ID_EX_RegDst,
                          int EX_MEM_isMemRead, int EX_MEM_isRegWrite, int EX_MEM_RegDst,
                          int MEM_WB_isRegWrite, int MEM_WB_RegDst,
                          int forward[])
{
    /**********************************************************************************************************/
    int stall = 0;
    if (ID_EX_isMemRead && ID_EX_RegDst!=0 && /* ID_EX stage is load instruction, and */
           (IF_ID_Use_rs && ID_EX_RegDst==IF_ID_rs
         || IF_ID_Use_rt && ID_EX_RegDst==IF_ID_rt)) /* IF_ID wants to read registers */
        stall = 1;
    else if ((IF_ID_Use_rs==1 && ID_EX_RegDst==IF_ID_rs
           || IF_ID_Use_rt==1 && ID_EX_RegDst==IF_ID_rt) /* IF_ID stage is beq, bne, jr, and */
           && ID_EX_isRegWrite && ID_EX_RegDst!=0) /* ID_EX stage is ALU instruction */
        stall = 1;
    else if ((IF_ID_Use_rs==1 && EX_MEM_RegDst==IF_ID_rs
           || IF_ID_Use_rt==1 && EX_MEM_RegDst==IF_ID_rt) /* IF_ID stage is beq, bne, jr, and */
           && EX_MEM_isMemRead && EX_MEM_RegDst!=0) /* EX_MEM stage is load instruction */
        stall = 1;
    if (stall) {
        strcpy(hazards[0][hazards_size[0]++], "to_be_stalled");
        strcpy(hazards[1][hazards_size[1]++], "to_be_stalled");
    }
    /**********************************************************************************************************/

    /**********************************************************************************************************/
    memset(forward, 0, 4*sizeof(int));
    if (!stall && EX_MEM_isRegWrite && !EX_MEM_isMemRead && EX_MEM_RegDst!=0) { /* branch forwarding */
        if (IF_ID_Use_rs==1 && EX_MEM_RegDst==IF_ID_rs) { /* rs */
            forward[0] = 1;
            sprintf(hazards[1][hazards_size[1]++], "fwd_EX-DM_rs_$%d", IF_ID_rs);
        }
        if (IF_ID_Use_rt==1 && EX_MEM_RegDst==IF_ID_rt) { /* rt */
            forward[1] = 1;
            sprintf(hazards[1][hazards_size[1]++], "fwd_EX-DM_rt_$%d", IF_ID_rt);
        }
    }
    if (ID_EX_Use_rs == 2) { /* ALU operation forwarding rs */
        if (EX_MEM_isRegWrite && !EX_MEM_isMemRead && EX_MEM_RegDst!=0 && EX_MEM_RegDst==ID_EX_rs) { /* EX_MEM hazard */
            forward[2] = 2;
            sprintf(hazards[2][hazards_size[2]++], "fwd_EX-DM_rs_$%d", ID_EX_rs);
        }
        else if (MEM_WB_isRegWrite && MEM_WB_RegDst!=0 && MEM_WB_RegDst==ID_EX_rs) { /* MEM_WB hazard */
            forward[2] = 1;
            sprintf(hazards[2][hazards_size[2]++], "fwd_DM-WB_rs_$%d", ID_EX_rs);
        }
    }
    if (ID_EX_Use_rt == 2) { /* ALU operation forwarding rt */
        if (EX_MEM_isRegWrite && !EX_MEM_isMemRead && EX_MEM_RegDst!=0 && EX_MEM_RegDst==ID_EX_rt) { /* EX_MEM hazard */
            forward[3] = 2;
            sprintf(hazards[2][hazards_size[2]++], "fwd_EX-DM_rt_$%d", ID_EX_rt);
        }
        else if (MEM_WB_isRegWrite && MEM_WB_RegDst!=0 && MEM_WB_RegDst==ID_EX_rt) { /* MEM_WB hazard */
            forward[3] = 1;
            sprintf(hazards[2][hazards_size[2]++], "fwd_DM-WB_rt_$%d", ID_EX_rt);
        }
    }
    /**********************************************************************************************************/

    return stall;
}
