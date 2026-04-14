#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_SIZE 256
#define INSTR_SIZE 16
#define REG_COUNT 32

enum classe_inst {
    tipo_I,
    tipo_J,
    tipo_R
};

struct instrucao {
    enum classe_inst tipo_inst;
    char inst_char[INSTR_SIZE + 1];
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
    struct pc pc;
    int reg[REG_COUNT];
    struct instrucao *programa;
    int prog_size;

    struct ULA ula;
    struct controle ctrl;
};

// ================= MEMORIA =================

void imprimir_memoria(struct memoria_dados *mem) {

    int i;

    printf("\n=== MEMORIA ===\n");

    for (i = 0; i < DATA_SIZE; i++) {

        if (mem->dados[i] != 0) {
            printf("Mem[%d] = %d\n", i, mem->dados[i]);
        }
    }
}

void mostrar_registradores(int reg[]) {

    int i;

    printf("\n=== REGISTRADORES ===\n");

    for (i = 0; i < REG_COUNT; i++) {
        printf("R%d = %d\n", i, reg[i]);
    }
}

// ================= ULA =================

int executar_ula(struct ULA *ula, int op) {

    switch(op) {

        case 0:
            ula->resultado = ula->entrada1 + ula->entrada2;
            break;

        case 1:
            ula->resultado = ula->entrada1 - ula->entrada2;
            break;

        case 2:
            ula->resultado = ula->entrada1 & ula->entrada2;
            break;

        case 3:
            ula->resultado = ula->entrada1 | ula->entrada2;
            break;

        default:
            ula->resultado = 0;
            break;
    }

    return ula->resultado;
}

// ================= CONTROLE =================

void unidade_controle(struct instrucao *inst, struct controle *ctrl) {

    ctrl->alu_op = 0;
    ctrl->mem_read = 0;
    ctrl->mem_write = 0;
    ctrl->reg_write = 0;

    if (inst->tipo_inst == tipo_R) {

        ctrl->reg_write = 1;
        ctrl->alu_op = inst->funct;
    }

    else if (inst->tipo_inst == tipo_I) {

        if (inst->opcode == 8) {

            ctrl->reg_write = 1;
            ctrl->alu_op = 0;
        }

        else if (inst->opcode == 4) {

            ctrl->mem_read = 1;
            ctrl->reg_write = 1;
        }

        else if (inst->opcode == 5) {

            ctrl->mem_write = 1;
        }
    }
}

// ================= DECODIFICADOR =================

void decodificador(struct instrucao *inst) {

    int i;

    inst->inst_char[INSTR_SIZE] = '\0';

    inst->opcode = 0;

    for (i = 0; i < 4; i++) {
        inst->opcode = (inst->opcode << 1) | (inst->inst_char[i] - '0');
    }

    if (inst->opcode == 0) {
        inst->tipo_inst = tipo_R;
    }
    else if (inst->opcode == 2) {
        inst->tipo_inst = tipo_J;
    }
    else {
        inst->tipo_inst = tipo_I;
    }

    if (inst->tipo_inst == tipo_R) {

        inst->rs = inst->rt = inst->rd = inst->funct = 0;

        for (i = 4; i < 7; i++)
            inst->rs = (inst->rs << 1) | (inst->inst_char[i] - '0');

        for (i = 7; i < 10; i++)
            inst->rt = (inst->rt << 1) | (inst->inst_char[i] - '0');

        for (i = 10; i < 13; i++)
            inst->rd = (inst->rd << 1) | (inst->inst_char[i] - '0');

        for (i = 13; i < 16; i++)
            inst->funct = (inst->funct << 1) | (inst->inst_char[i] - '0');
    }

    else if (inst->tipo_inst == tipo_I) {

        inst->rs = inst->rt = inst->imm = 0;

        for (i = 4; i < 7; i++)
            inst->rs = (inst->rs << 1) | (inst->inst_char[i] - '0');

        for (i = 7; i < 10; i++)
            inst->rt = (inst->rt << 1) | (inst->inst_char[i] - '0');

        for (i = 10; i < 16; i++)
            inst->imm = (inst->imm << 1) | (inst->inst_char[i] - '0');
    }

    else {

        inst->addr = 0;

        for (i = 4; i < 16; i++)
            inst->addr = (inst->addr << 1) | (inst->inst_char[i] - '0');
    }
}

// ================= EXECUÇÃO =================

void executar_instrucao(struct simulador *sim, struct instrucao *inst) {

    if (inst->tipo_inst == tipo_I && inst->opcode == 8) {

        sim->reg[inst->rt] = sim->reg[inst->rs] + inst->imm;
    }

    else if (inst->tipo_inst == tipo_I && inst->opcode == 9) {

        if (sim->reg[inst->rs] == sim->reg[inst->rt]) {
            sim->pc.pc += inst->imm;
            return;
        }
    }

    else if (inst->tipo_inst == tipo_R && sim->ctrl.reg_write) {

        sim->ula.entrada1 = sim->reg[inst->rs];
        sim->ula.entrada2 = sim->reg[inst->rt];

        sim->reg[inst->rd] =
            executar_ula(&sim->ula, sim->ctrl.alu_op);
    }

    else if (sim->ctrl.mem_read) {

        sim->reg[inst->rt] =
            sim->dmem.dados[sim->reg[inst->rs] + inst->imm];
    }

    else if (sim->ctrl.mem_write) {

        sim->dmem.dados[sim->reg[inst->rs] + inst->imm] =
            sim->reg[inst->rt];
    }

    else if (inst->tipo_inst == tipo_J) {

        sim->pc.pc = inst->addr;
    }
}

// ================= STEP =================

void step_simulation(struct simulador *sim) {

    if (sim->pc.pc >= sim->prog_size) {
        printf("Fim do programa\n");
        return;
    }

    struct instrucao *inst = &sim->programa[sim->pc.pc];

    sim->pc.prev_pc = sim->pc.pc;

    decodificador(inst);
    unidade_controle(inst, &sim->ctrl);

    printf("\nExecutando %d: %s\n", sim->pc.pc, inst->inst_char);

    executar_instrucao(sim, inst);

    if (inst->tipo_inst != tipo_J) {
        sim->pc.pc++;
    }
}

// ================= RUN =================

void run_simulation(struct simulador *sim) {

    while (sim->pc.pc < sim->prog_size) {
        step_simulation(sim);
    }
}

// ================= BACK =================

void voltar_instrucao(struct simulador *sim) {

    if (sim->pc.prev_pc >= 0) {
        sim->pc.pc = sim->pc.prev_pc;
        printf("\nVoltou para instrucao %d\n", sim->pc.pc);
    }
}

// ================= MENU =================

void menu(struct simulador *sim) {

    int op;

    do {
        printf("\nMenu principal:\n");
        printf("1. Imprimir memórias\n");
        printf("2. Imprimir registradores\n");
        printf("3. Executar programa (run)\n");
        printf("4. Executar passo (step)\n");
        printf("5. Voltar instrução (back)\n");
        printf("0. Sair\n");
        printf("Opcao: ");
        scanf("%d", &op);

        switch(op) {

            case 1:
                imprimir_memoria(&sim->dmem);
                break;

            case 2:
                mostrar_registradores(sim->reg);
                break;

            case 3:
                run_simulation(sim);
                break;

            case 4:
                step_simulation(sim);
                break;

            case 5:
                voltar_instrucao(sim);
                break;

            case 0:
                break;

            default:
                printf("Opcao invalida\n");
                break;
        }

    } while(op != 0);
}

// ================= MAIN =================

int main() {

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
