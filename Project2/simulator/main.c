#include <string.h> // memcpy
#include "function.h"
#include "unit.h"

FILE *fin, *fout, *ferr;
unsigned char Dmem[1024];    /* 1K bytes are 1024 bytes. */
int Imem[256];               /* 1K bytes are 256 words. */
int reg[32];                 /* There are 32 registers. */
int pc;                      /* program counter */
int cycle;                   /* record the current cycle */
int stall_at_ID_stage;       /* flags to let IF and ID stages stall for one cycle */

/* variables for instruction data */
int s, t, d, shamt, imm, funct;

/* variables for middle registers between stages */
int IF_ID[3], ID_EX[14], EX_MEM[6], MEM_WB[6];
int ctrl_out[12], fwd_out[4], pc_before[3];
int jump, branch, rs_for_branch, rt_for_branch;
int ALU_in1, ALU_in2, write_back_data;

/* instruction and hazard messages */
char instruction_name[4][30], instruction_saved[5][30];
char error_message[4][50];
char hazards[3][5][30];
int hazards_size[3];

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
            reg[29] = word; /* stack pointer */
        else if (line == 2)
            size = word; /* how many data */
        else {
            if (line-3 >= 256) {
                fprintf(stderr, "The data image input is too much for the D-memory!!\n");
                fprintf(stderr, "The test case may be invalid!!\n");
                break;
            }
            for (int i=0; i<4; i++) /* load into the memory */
                Dmem[4*(line-3) + i] = word >> (24-8*i);
        }
        /* If the given input is too much, stop reading. */
        if (line-3 >= size-1)
            break;
    }
    if (line-3 < size-1)
    {
        fprintf(stderr, "The real data image input is less than the amount specified!!\n");
        fprintf(stderr, "The test case may be invalid!!\n");
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
            pc = word; /* program size initialization */
            if (pc<0 || pc>1023 || pc%4!=0)
                process_invalid_testcases("The program size is not on a valid position.");
        }
        else if (line == 2) {
            size = word; /* how many instructions */
            if (size<0)
                process_invalid_testcases("The negative number of instructions is obviously invalid.");
        }
        else {
            if (pc/4 + line-3 >= 256) {
                fprintf(stderr, "The instruction image input is too much for the I-memory!!\n");
                fprintf(stderr, "The test case may be invalid!!\n");
                break;
            }
            Imem[pc/4 + line-3] = word;
        }
        /* If the given input is too much, stop reading. */
        if (line-3 >= size-1)
            break;
    }
    if (line-3 < size-1) {
        fprintf(stderr, "The real data image input is less than the amount specified!!\n");
        fprintf(stderr, "The test case may be invalid!!\n");
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
    print("#5 The following is the tracing of the execution.\n\n");
    for (int i=0; i<4; i++) /* initialize all stages with nop instructions */
        set_nop_to_stage(i);
    int run = 1;
    while (run) {
        /* PC validity check */
        if (pc<0 || pc>1023 || pc%4!=0)
            process_invalid_testcases("The program size is not on a valid position.");

        /* infinite loop check */
        if (cycle > 500000)
            process_invalid_testcases("The executed cycle is too much.");

        /* print registers */
        print_registers();

        /* hazard and error dump buffer reset */
        memset(&hazards_size, 0, sizeof(hazards_size));
        for (int i=0; i<4; i++)
            strcpy(error_message[i], "");

        /* save instruction names before executing all cycles for printing */
        sprintf(instruction_saved[0], "0x%08X", Imem[pc/4]);
        for (int i=0; i<4; i++)
            strcpy(instruction_saved[i+1], instruction_name[i]);

        /* 5 halt detection */
        if (fetch_bit(Imem[pc/4],31,26)==0x3F && strcmp(instruction_saved[1],"HALT")==0 && strcmp(instruction_saved[2],"HALT")==0 && strcmp(instruction_saved[3],"HALT")==0 && strcmp(instruction_saved[4],"HALT")==0)
            run = 0;

        /* hazard detection */
        int ID_EX_RegDst;
        switch (ID_EX[1]) {
            case 1: ID_EX_RegDst = ID_EX[10]; break;
            case 2: ID_EX_RegDst = ID_EX[11]; break;
            case 3: ID_EX_RegDst = 31; break; /* jal */
        }
        control_unit(fetch_bit(IF_ID[1],31,26), fetch_bit(IF_ID[1],5,0), ctrl_out);
        stall_at_ID_stage = hazard_detection_unit( /* include stall and forwarding */
            ctrl_out[4], fetch_bit(IF_ID[1],25,21), ctrl_out[5], fetch_bit(IF_ID[1],20,16), /* IF_ID */
            ID_EX[5], ID_EX[9], ID_EX[6], ID_EX[10], ID_EX[2]>=4, ID_EX[1]>0, ID_EX_RegDst, /* ID_EX */
            EX_MEM[2]>=4, EX_MEM[1]>0, EX_MEM[5], /* EX_MEM */
            MEM_WB[1]>0, MEM_WB[5], /* MEM_WB */
            fwd_out);

        /**********************************************************************************************************/
        /************************************* WB Stage (MEM_WB to REGISTER)  *************************************/
        int write_back_data;
        if (MEM_WB[2] >= 4) /* data multiplexer */
            write_back_data = MEM_WB[3];
        else
            write_back_data = MEM_WB[4];
        if (MEM_WB[1] > 0) { /* Reg-Write enabled */
            if (MEM_WB[5]==0 && !MEM_WB[0]) /* destination is the $0 register but not an NOP instruction */
                sprintf(error_message[0], "In cycle %d: Write $0 Error", cycle);
            else
                reg[MEM_WB[5]] = write_back_data; /* write data back to register */
        }
        /* EX stage (non-branch) forwarding */
        if (fwd_out[2] == 1)
            ALU_in1 = write_back_data;
        if (fwd_out[3] == 1)
            ALU_in2 = write_back_data;
        /**********************************************************************************************************/
        /**********************************************************************************************************/

        /**********************************************************************************************************/
        /************************************** MEM Stage (EX_MEM to MEM_WB) **************************************/
        strcpy(instruction_name[3], instruction_name[2]);
        memcpy(MEM_WB, EX_MEM, 6*sizeof(int)); /* MEM_WB[0:5] = EX_MEM[0:5] except MEM_WB[3] */
        switch (EX_MEM[2]) { /* Mem-Read */
            case 4:
                if (EX_MEM[4]%4 != 0)
                    sprintf(error_message[2], "In cycle %d: Misalignment Error", cycle);
                if (EX_MEM[4]<0 || EX_MEM[4]>1020) {
                    sprintf(error_message[1], "In cycle %d: Address Overflow", cycle);
                    break;
                }
                MEM_WB[3] = (Dmem[EX_MEM[4]] << 24) + (Dmem[EX_MEM[4]+1] << 16) + (Dmem[EX_MEM[4]+2] << 8) + (Dmem[EX_MEM[4]+3] << 0);
                break;
            case 5:
                if (EX_MEM[4]%2 != 0)
                    sprintf(error_message[2], "In cycle %d: Misalignment Error", cycle);
                if (EX_MEM[4]<0 || EX_MEM[4]>1022) {
                    sprintf(error_message[1], "In cycle %d: Address Overflow", cycle);
                    break;
                }
                MEM_WB[3] = (((Dmem[EX_MEM[4]] << 8) + (Dmem[EX_MEM[4]+1] << 0)) << 16) >> 16;
                break;
            case 6:
                if (EX_MEM[4]%2 != 0)
                    sprintf(error_message[2], "In cycle %d: Misalignment Error", cycle);
                if (EX_MEM[4]<0 || EX_MEM[4]>1022) {
                    sprintf(error_message[1], "In cycle %d: Address Overflow", cycle);
                    break;
                }
                MEM_WB[3] = (Dmem[EX_MEM[4]] << 8) + (Dmem[EX_MEM[4]+1] << 0);
                break;
            case 7:
                if (EX_MEM[4]<0 || EX_MEM[4]>1023) {
                    sprintf(error_message[1], "In cycle %d: Address Overflow", cycle);
                    break;
                }
                MEM_WB[3] = (Dmem[EX_MEM[4]] << 24) >> 24;
                break;
            case 8:
                if (EX_MEM[4]<0 || EX_MEM[4]>1023) {
                    sprintf(error_message[1], "In cycle %d: Address Overflow", cycle);
                    break;
                }
                MEM_WB[3] = Dmem[EX_MEM[4]];
        }
        switch (EX_MEM[2]) { /* Mem-Write */
            case 1:
                if (EX_MEM[4]%4 != 0)
                    sprintf(error_message[2], "In cycle %d: Misalignment Error", cycle);
                if (EX_MEM[4]<0 || EX_MEM[4]>1020) {
                    sprintf(error_message[1], "In cycle %d: Address Overflow", cycle);
                    break;
                }
                Dmem[EX_MEM[4]] = (EX_MEM[3] & 0xFF000000) >> 24;
                Dmem[EX_MEM[4] + 1] = (EX_MEM[3] & 0x00FF0000) >> 16;
                Dmem[EX_MEM[4] + 2] = (EX_MEM[3] & 0x0000FF00) >> 8;
                Dmem[EX_MEM[4] + 3] = (EX_MEM[3] & 0x000000FF) >> 0;
                break;
            case 2:
                if (EX_MEM[4]%2 != 0)
                    sprintf(error_message[2], "In cycle %d: Misalignment Error", cycle);
                if (EX_MEM[4]<0 || EX_MEM[4]>1022) {
                    sprintf(error_message[1], "In cycle %d: Address Overflow", cycle);
                    break;
                }
                Dmem[EX_MEM[4]] = (EX_MEM[3] & 0x0000FF00) >> 8;
                Dmem[EX_MEM[4]+1] = (EX_MEM[3] & 0x000000FF) >> 0;
                break;
            case 3:
                if (EX_MEM[4]<0 || EX_MEM[4]>1023) {
                    sprintf(error_message[1], "In cycle %d: Address Overflow", cycle);
                    break;
                }
                Dmem[EX_MEM[4]] = (EX_MEM[3] & 0x000000FF) >> 0;
                break;
        }
        /* ID stage (branch) forwarding */
        if (fwd_out[0] == 1)
            rs_for_branch = EX_MEM[4];
        if (fwd_out[1] == 1)
            rt_for_branch = EX_MEM[4];
        /* EX stage (non-branch) forwarding */
        if (fwd_out[2] == 2)
            ALU_in1 = EX_MEM[4];
        if (fwd_out[3] == 2)
            ALU_in2 = EX_MEM[4];
        /**********************************************************************************************************/
        /**********************************************************************************************************/

        /**********************************************************************************************************/
        /*************************************** EX Stage (ID_EX to EX_MEM) ***************************************/
        strcpy(instruction_name[2], instruction_name[1]);
        memcpy(EX_MEM, ID_EX, 3*sizeof(int)); /* EX_MEM[0:2] = ID_EX[0:2] */
        /* forwarding has been done by future stages above, so we only consider the non-forwarding case here */
        if (fwd_out[2] == 0)
            ALU_in1 = ID_EX[7];
        if (fwd_out[3] == 0)
            ALU_in2 = ID_EX[8];
        EX_MEM[3] = ALU_in2;
        if ((ID_EX[4] >> 1) == 1) /* MSB */
            ALU_in1 = ALU_in2;
        if ((ID_EX[4] & 1) == 0) /* LSB */
            ALU_in2 = ID_EX[12];
        /*********************************************/
        if (ID_EX[1] == 3)
            EX_MEM[4] = ID_EX[13];
        else
            EX_MEM[4] = ALU(ALU_in1, ALU_in2, ID_EX[3]); /* ALU_out */
        EX_MEM[5] = ID_EX_RegDst;
        /**********************************************************************************************************/
        /**********************************************************************************************************/

        /**********************************************************************************************************/
        /**************************************** ID Stage (IF_ID to ID_EX) ***************************************/
        if (stall_at_ID_stage) /* flush ID_EX registers */
            set_nop_to_stage(1);
        else {
            strcpy(instruction_name[1], instruction_name[0]);
            ID_EX[0] = IF_ID[0];
            memcpy(ID_EX+1, ctrl_out, 6*sizeof(int)); /* ID_EX[1:6] = ctrl_out[0:5] */
            ID_EX[9] = fetch_bit(IF_ID[1],25,21);
            ID_EX[7] = reg[ID_EX[9]]; /* note ID_EX[9] must be updated at the previous line first */
            ID_EX[10] = fetch_bit(IF_ID[1],20,16);
            ID_EX[8] = reg[ID_EX[10]]; /* note ID_EX[10] must be updated at the previous line first */
            ID_EX[11] = fetch_bit(IF_ID[1],15,11);
            ID_EX[12] = (fetch_bit(IF_ID[1],15,0) << 16) >> 16;
            ID_EX[13] = IF_ID[2];
            jump = ctrl_out[6];
            branch = ctrl_out[7];

            /* forwarding has been done by future stages above, so we only consider the non-forwarding case here */
            if (fwd_out[0] == 0)
                rs_for_branch = ID_EX[7]; /* note ID_EX[7] must be updated at the previous line first */
            if (fwd_out[1] == 0)
                rt_for_branch = ID_EX[8]; /* note ID_EX[8] must be updated at the previous line first */

            /* "branch" becomes 1 if we encounter beq and bne */
            branch = (branch==1 && rs_for_branch==rt_for_branch) /* beq */
                  || (branch==2 && rs_for_branch!=rt_for_branch); /* bne */

            /* pc updated at ID stage due to "branch" */
            pc_before[0] = (ID_EX[12] << 2) + IF_ID[2]; /* note ID_EX[12] must be updated at the previous line first */
            pc_before[1] = (fetch_bit(IF_ID[2],31,28) << 28) | (fetch_bit(IF_ID[1],25,0) << 2);
            pc_before[2] = rs_for_branch;
        }
        /**********************************************************************************************************/
        /**********************************************************************************************************/

        /**********************************************************************************************************/
        /*********************************** IF Stage (PC to IF_ID) + PC Update ***********************************/
        if (!stall_at_ID_stage) { /** After all execution, PC should be updated. **/
            if (jump==0 && branch==0) { /* without branch */
                IF_ID[1] = Imem[pc/4];
                IF_ID[0] = generate_instruction_name_and_check_nop(IF_ID[1], instruction_name[0]); /* note IF_ID[1] must be updated at the previous line first */
                IF_ID[2] = pc + 4;
                pc = IF_ID[2]; /* note IF_ID[2] must be updated at the previous line first */
            }
            else { /* with branch */
                strcpy(hazards[0][hazards_size[0]++], "to_be_flushed");
                set_nop_to_stage(0); /* flush pre-stored IF_ID registers */
                if (branch == 1) /* beq or bne */
                    pc = pc_before[0];
                else if (jump == 1) /* j or jal */
                    pc = pc_before[1];
                else if (jump == 2) /* jr */
                    pc = pc_before[2];
            }
        }
        /**********************************************************************************************************/
        /**********************************************************************************************************/

        print_instructions_and_hazards();
        if (print_error_messages())
            run = 0;
    }
    /**********************************************************************************************************/
    /**********************************************************************************************************/

    fclose(fout);
    fclose(ferr);
    return 0;
}
