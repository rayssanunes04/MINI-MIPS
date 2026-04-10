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
    int prev_pc; // back
};

struct ULA {
    int entrada1;
    int entrada2;
    int resultado; //  RD ou Memória
};

struct controle {
    int alu_op; 
    int mem_read;
    int mem_write;
    int reg_write;
};

struct simulador {
    struct memoria_dados dmem;
    struct pc pc;
    int reg[REG_COUNT];
    struct instrucao *programa;
    int prog_size;

    struct ULA ula;
    struct controle ctrl;
};

void imprimir_memoria(struct memoria_dados *mem) {
    printf("\nMEMORIA:\n");
    for (int i = 0; i < DATA_SIZE; i++) {
        if (mem->dados[i] != 0) {
            printf("Mem[%d] = %d\n", i, mem->dados[i]);
        }
    }
}

void mostrar_registradores(int reg[]) {
    printf("\nREGISTRADORES:\n");
    for (int i = 0; i < REG_COUNT; i++) {
        printf("R%d = %d\n", i, reg[i]);
    }
}

void voltar_instrucao(struct simulador *sim) {
    if (sim->pc.prev_pc >= 0) {
        sim->pc.pc = sim->pc.prev_pc; // volta pro o pc
        printf("\nVoltou para instrucao %d\n", sim->pc.pc);
    } else {
        printf("\nNao ha instrucao anterior\n");
    }
}


void salvar_memoria_dados(struct memoria_dados *mem, const char *nome) {
    FILE *arq = fopen(nome, "w");
    if (!arq) return;

    for (int i = 0; i < DATA_SIZE; i++) {
        if (mem->dados[i] != 0) {
            fprintf(arq, "%d %d\n", i, mem->dados[i]);
        }
    }

    fclose(arq);
    printf("Memoria salva em %s\n", nome);
}

void salvar_programa(struct simulador *sim, const char *nome) {
    FILE *arq = fopen(nome, "w");
    if (!arq) return;

    for (int i = 0; i < sim->prog_size; i++) {
        fprintf(arq, "%s\n", sim->programa[i].inst_char);
    }

    fclose(arq);
    printf("Programa salvo em %s\n", nome);
}

int executar_ula(struct ULA *ula, int op) {
    switch(op) {
        case 0: ula->resultado = ula->entrada1 + ula->entrada2; break;
        case 1: ula->resultado = ula->entrada1 - ula->entrada2; break;
        case 2: ula->resultado = ula->entrada1 & ula->entrada2; break;
        case 3: ula->resultado = ula->entrada1 | ula->entrada2; break;
        case 4: ula->resultado = (ula->entrada1 < ula->entrada2); break;
        default: ula->resultado = 0;
    }
    return ula->resultado;
}

void unidade_controle(struct instrucao *inst, struct controle *ctrl) {
    ctrl->alu_op = 0;
    ctrl->mem_read = 0;
    ctrl->mem_write = 0;
    ctrl->reg_write = 0;

    if (inst->tipo_inst == tipo_R) {
        ctrl->reg_write = 1; // escrevendo no reg
        ctrl->alu_op = inst->funct; //  funct da inst vai virar alu_op
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

void decodificador(struct instrucao *inst) {

    inst->inst_char[INSTR_SIZE] = '\0'; // val

    char opcode_str[5];
    strncpy(opcode_str, inst->inst_char, 4); // 
    opcode_str[4] = '\0';
    inst->opcode = strtol(opcode_str, NULL, 2); // con

    if (inst->opcode == 0) {
        inst->tipo_inst = tipo_R;

        char rs[4], rt[4], rd[4], funct[4]; // variavel

        strncpy(rs, inst->inst_char + 4, 3); rs[3] = '\0';
        strncpy(rt, inst->inst_char + 7, 3); rt[3] = '\0';
        strncpy(rd, inst->inst_char + 10, 3); rd[3] = '\0';
        strncpy(funct, inst->inst_char + 13, 3); funct[3] = '\0';
// con
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

void executar_instrucao(struct simulador *sim, struct instrucao *inst) {

    // HALT
    if (strcmp(inst->inst_char, "0000000000000000") == 0) { // para
        printf("HALT\n");
        sim->pc.pc = sim->prog_size;~// final
        return;
    }

    // ADDI
    if (inst->tipo_inst == tipo_I && inst->opcode == 8) {
        sim->reg[inst->rt] = sim->reg[inst->rs] + inst->imm;
    }

    // BEQ
    if (inst->tipo_inst == tipo_I && inst->opcode == 9) {
        if (sim->reg[inst->rs] == sim->reg[inst->rt]) { 
            sim->pc.pc += inst->imm; // + im 
            return;
        }
    }

    // R
    if (inst->tipo_inst == tipo_R && sim->ctrl.reg_write) {
        // ula recebe val dos reg
        sim->ula.entrada1 = sim->reg[inst->rs];
        sim->ula.entrada2 = sim->reg[inst->rt];
        sim->reg[inst->rd] = //
            executar_ula(&sim->ula, sim->ctrl.alu_op); // chama a função ULA
    }

    // LW
    if (sim->ctrl.mem_read) { //  cont le mem
        sim->reg[inst->rt] =
            sim->dmem.dados[sim->reg[inst->rs] + inst->imm];
    }

    // SW
    if (sim->ctrl.mem_write) { // es mem
        sim->dmem.dados[sim->reg[inst->rs] + inst->imm] =
            sim->reg[inst->rt];
    }

    // JUMP
    if (inst->tipo_inst == tipo_J) {
        sim->pc.pc = inst->addr; // jump ints
    }
}

void step_simulation(struct simulador *sim) {

    if (sim->pc.pc >= sim->prog_size) {
        printf("Fim do programa\n");
        return;
    }

    struct instrucao *inst = &sim->programa[sim->pc.pc]; // inst ataul pc

    sim->pc.prev_pc = sim->pc.pc; // guarda v pc

    decodificador(inst);
    unidade_controle(inst, &sim->ctrl); // decide 

    printf("\nExecutando %d: %s\n", sim->pc.pc, inst->inst_char);

    executar_instrucao(sim, inst);

    if (inst->tipo_inst != tipo_J) { // atualiza 
        sim->pc.pc++; // prox
    }
}
// 1 
void run_simulation(struct simulador *sim) {
    while (sim->pc.pc < sim->prog_size) {
        step_simulation(sim); // ins, dec, exec, atu
    }
}

// MENU
void menu(struct simulador *sim) {
    int op;

    do {
        printf("\n===== MENU =====\n");
        printf("1. Step\n");
        printf("2. Run\n");
        printf("3. Registradores\n");
        printf("4. Memoria\n");
        printf("5. Back\n");
        printf("6. Salvar .dat\n");
        printf("7. Salvar .asm\n");
        printf("0. Sair\n");
        printf("Opcao: ");
        scanf("%d", &op);

        switch(op) {
            case 1: step_simulation(sim); break;
            case 2: run_simulation(sim); break;
            case 3: mostrar_registradores(sim->reg); break;
            case 4: imprimir_memoria(&sim->dmem); break;
            case 5: voltar_instrucao(sim); break;
            case 6: salvar_memoria_dados(&sim->dmem, "saida.dat"); break;
            case 7: salvar_programa(sim, "saida.asm"); break;
        }

    } while(op != 0);
}

// MAIN
int main(){

    FILE *arq = fopen("teste.mem", "r");

    if (!arq) {
        printf("Erro ao abrir teste.mem\n");
        return 1;
    }

    struct instrucao programa[100];
    int i = 0;

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

    menu(&sim);

    return 0;
}
