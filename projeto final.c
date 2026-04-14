#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_SIZE 256
#define INSTR_SIZE 16
#define REG_COUNT 8   // ALTERADO PARA 8

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

struct simulador {
    struct memoria_dados dmem;
    struct pc pc;
    int reg[REG_COUNT];
    int reg_backup[REG_COUNT]; // BACK
    struct instrucao *programa;
    int prog_size;
    struct ULA ula;
};

// ---------------- MEMÓRIA ----------------

void imprimir_memoria(struct memoria_dados *mem) {
    int op;
    printf("1 - Mostrar tudo\n2 - Mostrar posicao\n");
    scanf("%d", &op);

    if (op == 1) {
        for (int i = 0; i < DATA_SIZE; i++) {
            printf("Mem[%d] = %d\n", i, mem->dados[i]);
        }
    } else {
        int pos;
        printf("Posicao: ");
        scanf("%d", &pos);
        printf("Mem[%d] = %d\n", pos, mem->dados[pos]);
    }
}

void mostrar_registradores(int reg[]) {
    printf("\n=== REGISTRADORES ===\n");
    for (int i = 0; i < REG_COUNT; i++) {
        printf("R%d = %d\n", i, reg[i]);
    }
}

// ---------------- ULA ----------------

int executar_ula(struct ULA *ula, int op) {
    switch(op) {
        case 0: return ula->entrada1 + ula->entrada2;
        case 1: return ula->entrada1 - ula->entrada2;
        case 2: return ula->entrada1 & ula->entrada2;
        case 3: return ula->entrada1 | ula->entrada2;
        default: return 0;
    }
}

// ---------------- DECODIFICADOR ----------------

void decodificador(struct instrucao *inst) {

    inst->opcode = 0;

    for (int i = 0; i < 4; i++)
        inst->opcode = (inst->opcode << 1) | (inst->inst_char[i] - '0');

    if (inst->opcode == 0) inst->tipo_inst = tipo_R;
    else if (inst->opcode == 2) inst->tipo_inst = tipo_J;
    else inst->tipo_inst = tipo_I;

    if (inst->tipo_inst == tipo_R) {
        inst->rs = inst->rt = inst->rd = inst->funct = 0;

        for (int i = 4; i < 7; i++)
            inst->rs = (inst->rs << 1) | (inst->inst_char[i] - '0');

        for (int i = 7; i < 10; i++)
            inst->rt = (inst->rt << 1) | (inst->inst_char[i] - '0');

        for (int i = 10; i < 13; i++)
            inst->rd = (inst->rd << 1) | (inst->inst_char[i] - '0');

        for (int i = 13; i < 16; i++)
            inst->funct = (inst->funct << 1) | (inst->inst_char[i] - '0');
    }
    else if (inst->tipo_inst == tipo_I) {
        inst->rs = inst->rt = inst->imm = 0;

        for (int i = 4; i < 7; i++)
            inst->rs = (inst->rs << 1) | (inst->inst_char[i] - '0');

        for (int i = 7; i < 10; i++)
            inst->rt = (inst->rt << 1) | (inst->inst_char[i] - '0');

        for (int i = 10; i < 16; i++)
            inst->imm = (inst->imm << 1) | (inst->inst_char[i] - '0');
    }
    else {
        inst->addr = 0;
        for (int i = 4; i < 16; i++)
            inst->addr = (inst->addr << 1) | (inst->inst_char[i] - '0');
    }
}

// ---------------- EXECUÇÃO ----------------

void executar_instrucao(struct simulador *sim, struct instrucao *inst) {

    // BACKUP
    memcpy(sim->reg_backup, sim->reg, sizeof(sim->reg));

    // ADDI
    if (inst->opcode == 4) {
        sim->reg[inst->rt] = sim->reg[inst->rs] + inst->imm;
    }

    // BEQ
    else if (inst->opcode == 9) {
        if (sim->reg[inst->rs] == sim->reg[inst->rt]) {
            sim->pc.pc += inst->imm;
            return;
        }
    }

    // TIPO R
    else if (inst->tipo_inst == tipo_R) {
        sim->ula.entrada1 = sim->reg[inst->rs];
        sim->ula.entrada2 = sim->reg[inst->rt];
        sim->reg[inst->rd] = executar_ula(&sim->ula, inst->funct);
    }

    // LW
    else if (inst->opcode == 4) {
        sim->reg[inst->rt] = sim->dmem.dados[sim->reg[inst->rs] + inst->imm];
    }

    // SW
    else if (inst->opcode == 5) {
        sim->dmem.dados[sim->reg[inst->rs] + inst->imm] = sim->reg[inst->rt];
    }

    // JUMP
    else if (inst->tipo_inst == tipo_J) {
        sim->pc.pc = inst->addr;
    }
}

// ---------------- STEP ----------------

void step_simulation(struct simulador *sim) {

    if (sim->pc.pc >= sim->prog_size) {
        printf("Fim do programa\n");
        return;
    }

    struct instrucao *inst = &sim->programa[sim->pc.pc];

    sim->pc.prev_pc = sim->pc.pc;

    decodificador(inst);

    printf("\nExecutando %d: %s\n", sim->pc.pc, inst->inst_char);
    printf("PC: %d\n", sim->pc.pc);

    // MOSTRAR TIPO
    if (inst->tipo_inst == tipo_R)
        printf("Tipo: R\n");
    else if (inst->opcode == 4)
        printf("Tipo: ADDI\n");
    else if (inst->opcode == 9)
        printf("Tipo: BEQ\n");

    executar_instrucao(sim, inst);

    if (inst->tipo_inst != tipo_J)
        sim->pc.pc++;
}

// ---------------- RUN ----------------

void run_simulation(struct simulador *sim) {
    while (sim->pc.pc < sim->prog_size)
        step_simulation(sim);
}

// ---------------- BACK ----------------

void voltar_instrucao(struct simulador *sim) {

    memcpy(sim->reg, sim->reg_backup, sizeof(sim->reg));
    sim->pc.pc = sim->pc.prev_pc;

    printf("Voltou para %d\n", sim->pc.pc);
}

// ---------------- MENU ----------------

void menu(struct simulador *sim) {

    int op;

    do {
        printf("\n1.Step\n2.Run\n3.Memoria\n4.Registradores\n5.Back\n6.Ir para instrucao\n0.Sair\n");
        scanf("%d", &op);

        switch(op) {
            case 1: step_simulation(sim); break;
            case 2: run_simulation(sim); break;
            case 3: imprimir_memoria(&sim->dmem); break;
            case 4: mostrar_registradores(sim->reg); break;
            case 5: voltar_instrucao(sim); break;

            case 6:
                printf("Instrucao: ");
                int i;
                scanf("%d", &i);
                sim->pc.pc = i;
                step_simulation(sim);
                break;
        }

    } while(op != 0);
}

// ---------------- MAIN ----------------

int main() {

    struct instrucao programa[100];

    struct simulador sim = {
        .dmem = {0},
        .pc = {0, -1},
        .reg = {0},
        .programa = programa,
        .prog_size = 0,
        .ula = {0}
    };

    menu(&sim);

    return 0;
}
