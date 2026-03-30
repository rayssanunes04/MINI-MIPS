#include <stdio.h> 
#include <stdlib.h>
#include <string.h>

#define DATA_SIZE 256
#define INSTR_SIZE 16
#define REG_COUNT 32

enum classe_inst {
    tipo_I, tipo_J, tipo_R, tipo_OUTROS
};

struct instrucao {
    enum classe_inst tipo_inst;
    char inst_char[INSTR_SIZE+1];
    int opcode;
    int rs, rt, rd;
    int funct;
    int imm;
    int addr;
};

struct memoria_dados {
    int dados[DATA_SIZE];
};

struct pc {
    int pc;
    int prev_pc;
};

struct ULA {
    int entrada1;
    int entrada2;
    int resultado;
};

struct controle {
    int alu_op;
    int mem_read;
    int mem_write;
    int reg_write;
};

struct simulador {
    struct memoria_dados dmem;
    struct pc pc; // contador do meu pc
    int reg[REG_COUNT];
    struct instrucao *programa;
    int prog_size;

    struct ULA ula;
    struct controle ctrl;
};

// IMPRIMIR MEMÓRIA 
void imprimir_memoria(struct memoria_dados *mem) {
    printf("\n MEMORIA \n");
    for (int i = 0; i < DATA_SIZE; i++) {
        if (mem->dados[i] != 0) {
            printf("Mem[%d] = %d\n", i, mem->dados[i]);
        }
    }
    printf("================\n");
}

// VOLTAR INSTRUÇÃO
void voltar_instrucao(struct simulador *sim) {
    if (sim->pc.prev_pc >= 0) {
        sim->pc.pc = sim->pc.prev_pc;
        printf("\nVoltou para instrucao %d\n", sim->pc.pc);
    } else {
        printf("\nNao ha instrucao anterior\n");
    }
}

//ula
int executar_ula(struct ULA *ula, int operacao) {
    switch(operacao) {

        case 0: // ADD
            ula->resultado = ula->entrada1 + ula->entrada2;
            break;

        case 1: // SUB
            ula->resultado = ula->entrada1 - ula->entrada2;
            break;

        case 2: // AND
            ula->resultado = ula->entrada1 & ula->entrada2;
            break;

        case 3: // OR
            ula->resultado = ula->entrada1 | ula->entrada2;
            break;

        case 4: // SLT
            ula->resultado = (ula->entrada1 < ula->entrada2) ? 1 : 0;
            break;

        default:
            printf("erro\n");
            ula->resultado = 0;
    }

    return ula->resultado;
}

//  UNIDADE DE CONTROLE
void unidade_controle(struct instrucao *inst, struct controle *ctrl) {

    ctrl->alu_op = 0;
    ctrl->mem_read = 0;
    ctrl->mem_write = 0;
    ctrl->reg_write = 0;

    if (inst->tipo_inst == tipo_R) {

        ctrl->reg_write = 1;

        switch(inst->funct) {
            case 0: ctrl->alu_op = 0; break; // ADD
            case 1: ctrl->alu_op = 1; break; // SUB
            case 2: ctrl->alu_op = 2; break; // AND
            case 3: ctrl->alu_op = 3; break; // OR
            case 4: ctrl->alu_op = 4; break; // SLT
            default: ctrl->alu_op = 0;
        }
    }
    else if (inst->tipo_inst == tipo_I) {

        if (inst->opcode == 4) { // LW
            ctrl->mem_read = 1;
            ctrl->reg_write = 1;
        }
        else if (inst->opcode == 5) { // SW
            ctrl->mem_write = 1;
        }
    }
}

//  DECODIFICADOR 
void decodificador(struct instrucao *inst) {

    inst->inst_char[INSTR_SIZE] = '\0';

    char opcode_str[5];
    strncpy(opcode_str, inst->inst_char, 4);
    opcode_str[4] = '\0';
    inst->opcode = strtol(opcode_str, NULL, 2);
    
    if (inst->opcode == 0) { // Tipo R
        inst->tipo_inst = tipo_R;

        char rs[4], rt[4], rd[4], funct[4];

        strncpy(rs, inst->inst_char + 4, 3); rs[3] = '\0';
        strncpy(rt, inst->inst_char + 7, 3); rt[3] = '\0';
        strncpy(rd, inst->inst_char + 10, 3); rd[3] = '\0';
        strncpy(funct, inst->inst_char + 13, 3); funct[3] = '\0';

        inst->rs = strtol(rs, NULL, 2);
        inst->rt = strtol(rt, NULL, 2);
        inst->rd = strtol(rd, NULL, 2);
        inst->funct = strtol(funct, NULL, 2);
    } 
    else if (inst->opcode == 2 || inst->opcode == 3) {
        inst->tipo_inst = tipo_J;

        char addr[13];
        strncpy(addr, inst->inst_char + 4, 12);
        addr[12] = '\0';

        inst->addr = strtol(addr, NULL, 2);
    } 
    else {
        inst->tipo_inst = tipo_I;

        char rs[4], rt[4], imm[7];

        strncpy(rs, inst->inst_char + 4, 3); rs[3] = '\0';
        strncpy(rt, inst->inst_char + 7, 3); rt[3] = '\0';
        strncpy(imm, inst->inst_char + 10, 6); imm[6] = '\0';

        inst->rs = strtol(rs, NULL, 2);
        inst->rt = strtol(rt, NULL, 2);
        inst->imm = strtol(imm, NULL, 2);
    }
}

// EXECUÇÃO =
void executar_instrucao(struct simulador *sim, struct instrucao *inst) {

    // Tipo R (ULA)
    if (inst->tipo_inst == tipo_R && sim->ctrl.reg_write) {

        sim->ula.entrada1 = sim->reg[inst->rs];
        sim->ula.entrada2 = sim->reg[inst->rt];

        sim->reg[inst->rd] =
            executar_ula(&sim->ula, sim->ctrl.alu_op);

        switch(sim->ctrl.alu_op) {
            case 0: printf("ADD "); break;
            case 1: printf("SUB "); break;
            case 2: printf("AND "); break;
            case 3: printf("OR "); break;
            case 4: printf("SLT "); break;
        }

        printf("R%d, R%d, R%d\n",
               inst->rd, inst->rs, inst->rt);
    }

    // LW
    if (sim->ctrl.mem_read) {
        sim->reg[inst->rt] =
            sim->dmem.dados[sim->reg[inst->rs] + inst->imm];

        printf("LW R%d, %d(R%d)\n",
               inst->rt, inst->imm, inst->rs);
    }

    // SW
    if (sim->ctrl.mem_write) {
        sim->dmem.dados[sim->reg[inst->rs] + inst->imm] =
            sim->reg[inst->rt];

        printf("SW R%d, %d(R%d)\n",
               inst->rt, inst->imm, inst->rs);
    }

    // JUMP
    if (inst->tipo_inst == tipo_J) {
        printf("J %d\n", inst->addr);
        sim->pc.pc = inst->addr;
    }
}

// loop de simulação
void run_simulation(struct simulador *sim) {

    while (sim->pc.pc < sim->prog_size) {

        struct instrucao *inst = &sim->programa[sim->pc.pc];

        sim->pc.prev_pc = sim->pc.pc;

        decodificador(inst);

        unidade_controle(inst, &sim->ctrl);

        executar_instrucao(sim, inst);

        if (inst->tipo_inst != tipo_J) {
            sim->pc.pc++;
        }
    }
}

// ===== MAIN =====
int main(){

    FILE *arq = fopen("teste.mem", "r");

    if (arq == NULL) {
        printf("Erro ao abrir o arquivo teste.mem\n");
        return 1;
    }

    struct instrucao programa[100];
    int i = 0;

    // Lê cada linha do arquivo (.mem)
    while (fscanf(arq, "%s", programa[i].inst_char) != EOF) {
        i++;
    }

    fclose(arq);

    struct simulador sim = {
        .dmem = {0},
        .pc = {0, -1},
        .reg = {0},
        .programa = programa,
        .prog_size = i,
        .ula = {0,0,0},
        .ctrl = {0,0,0,0}
    };

    sim.reg[1] = 0;
    sim.dmem.dados[42] = 1234;

    run_simulation(&sim);

    return 0;
}
