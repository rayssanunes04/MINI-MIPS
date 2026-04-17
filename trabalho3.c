#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_SIZE 256
#define INSTR_SIZE 16
#define REG_COUNT 8  

// ================= ESTRUTURAS =================

struct instrucao {
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
    struct instrucao *programa;
    int prog_size;
    struct ULA ula;
};

// ================= INSTRUÇÕES =================

void carregar_memoria(struct simulador *sim) {

    char nome[100];
    FILE *arq;

    printf("Digite o nome do arquivo de instrucoes: ");
    scanf("%s", nome);

    arq = fopen(nome, "r");

    if (!arq) {
        printf("Erro ao abrir arquivo!\n");
        return;
    }

    int i = 0;
    char linha[50];

    while (fscanf(arq, "%s", linha) != EOF && i < 100) {
        if (strlen(linha) == INSTR_SIZE) {
            strcpy(sim->programa[i].inst_char, linha);
            i++;
        }
    }

    sim->prog_size = i;

    fclose(arq);

    printf("Programa carregado com %d instrucoes!\n", i);
}

// ================= MEMÓRIA DE DADOS =================

void carregar_dados(struct memoria_dados *mem) {

    FILE *arq = fopen("dados.dat", "r");

    if (!arq) {
        printf("Erro ao abrir dados.dat!\n");
        return;
    }

    int pos, valor;

    while (fscanf(arq, "%d %d", &pos, &valor) == 2) {

        if (pos >= 0 && pos < DATA_SIZE) {
            mem->dados[pos] = valor;
        }
    }

    fclose(arq);

    printf("Memoria de dados carregada!\n");
}

// ================= IMPRIMIR =================

void imprimir_memoria_instrucao(struct simulador *sim) {

    printf("\n=== MEMORIA DE INSTRUCOES ===\n");

    for (int i = 0; i < sim->prog_size; i++) {
        printf("[%d] = %s\n", i, sim->programa[i].inst_char);
    }
}

void imprimir_memoria_dados(struct memoria_dados *mem) {

    printf("\n=== MEMORIA DE DADOS ===\n");

    for (int i = 0; i < DATA_SIZE; i++) {
        if (mem->dados[i] != 0) {
            printf("Mem[%d] = %d\n", i, mem->dados[i]);
        }
    }
}

// ================= REGISTRADORES =================

void mostrar_registradores(int reg[]) {

    printf("\n=== REGISTRADORES ===\n");

    for (int i = 0; i < REG_COUNT; i++) {
        printf("R%d = %d\n", i, reg[i]);
    }
}

// ================= PC =================

void mostrar_pc(struct simulador *sim) {
    printf("\nPC atual: %d\n", sim->pc.pc);
}

// ================= ULA =================

int executar_ula(struct ULA *ula, int op) {

    switch(op) {

        case 0:
            return ula->entrada1 + ula->entrada2;

        case 2:
            return ula->entrada1 - ula->entrada2;

        case 4:
            return ula->entrada1 & ula->entrada2;

        case 5:
            return ula->entrada1 | ula->entrada2;

        default:
            return 0;
    }
}

// ================= DECODIFICADOR (SEGURO) =================

void decodificador(struct instrucao *inst) {

    inst->rs = inst->rt = inst->rd = 0;
    inst->funct = inst->imm = inst->addr = 0;

    inst->inst_char[INSTR_SIZE] = '\0';

    inst->opcode = 0;
    for (int i = 0; i < 4; i++)
        inst->opcode = (inst->opcode << 1) | (inst->inst_char[i] - '0');

    if (inst->opcode == 0) {

        for (int i = 4; i < 7; i++)
            inst->rs = (inst->rs << 1) | (inst->inst_char[i] - '0');

        for (int i = 7; i < 10; i++)
            inst->rt = (inst->rt << 1) | (inst->inst_char[i] - '0');

        for (int i = 10; i < 13; i++)
            inst->rd = (inst->rd << 1) | (inst->inst_char[i] - '0');

        for (int i = 13; i < 16; i++)
            inst->funct = (inst->funct << 1) | (inst->inst_char[i] - '0');
    }
    else {

        for (int i = 4; i < 7; i++)
            inst->rs = (inst->rs << 1) | (inst->inst_char[i] - '0');

        for (int i = 7; i < 10; i++)
            inst->rt = (inst->rt << 1) | (inst->inst_char[i] - '0');

        for (int i = 10; i < 16; i++)
            inst->imm = (inst->imm << 1) | (inst->inst_char[i] - '0');
    }
}

// ================= MOSTRAR INSTRUÇÃO =================

void mostrar_instrucao(struct instrucao *inst) {

    switch(inst->opcode) {

        case 0:
            if (inst->funct == 0) printf("ADD\n");
            else if (inst->funct == 2) printf("SUB\n");
            else if (inst->funct == 4) printf("AND\n");
            else if (inst->funct == 5) printf("OR\n");
            else printf("R-type\n");
            break;

        case 4:
            printf("ADDI\n");
            break;

        case 11:
            printf("LW\n");
            break;

        case 15:
            printf("SW\n");
            break;

        case 8:
            printf("BEQ\n");
            break;

        case 2:
            printf("JUMP\n");
            break;

        default:
            printf("DESCONHECIDA\n");
            break;
    }
}

// ================= EXECUÇÃO (CORRIGIDA FINAL) =================

void executar_instrucao(struct simulador *sim, struct instrucao *inst) {

    switch(inst->opcode) {

        case 4:
            sim->reg[inst->rt] = sim->reg[inst->rs] + inst->imm;
            break;

        case 8:
            if (sim->reg[inst->rs] == sim->reg[inst->rt]) {
                sim->pc.pc += inst->imm;
            }
            break;

        case 11:
            sim->reg[inst->rt] =
                sim->dmem.dados[sim->reg[inst->rs] + inst->imm];
            break;

        case 15:
            sim->dmem.dados[sim->reg[inst->rs] + inst->imm] =
                sim->reg[inst->rt];
            break;

        case 0: {
            sim->ula.entrada1 = sim->reg[inst->rs];
            sim->ula.entrada2 = sim->reg[inst->rt];

            int result = executar_ula(&sim->ula, inst->funct);

            sim->reg[inst->rd] = result;

            printf("DEBUG: R%d = %d\n", inst->rd, result);
            break;
        }

        case 2:
            sim->pc.pc = inst->addr;
            break;
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

    printf("\nExecutando %d: %s\n", sim->pc.pc, inst->inst_char);

    mostrar_instrucao(inst);

    executar_instrucao(sim, inst);

    if (inst->opcode != 2)
        sim->pc.pc++;
}

// ================= RUN =================

void run_simulation(struct simulador *sim) {

    while (sim->pc.pc < sim->prog_size)
        step_simulation(sim);
}

// ================= BACK =================

void voltar_instrucao(struct simulador *sim) {

    if (sim->pc.prev_pc >= 0) {
        sim->pc.pc = sim->pc.prev_pc;
        printf("Voltou para instrucao %d\n", sim->pc.pc);
    }
}

// ================= RESET =================

void reset_pc(struct simulador *sim) {
    sim->pc.pc = 0;
    printf("PC reiniciado!\n");
}

// ================= MENU =================

void menu(struct simulador *sim) {

    int op;

    do {
        printf("\nMENU:\n");
        printf("1. Carregar instrucoes\n");
        printf("2. Carregar memoria de dados\n");
        printf("3. Imprimir memorias\n");
        printf("4. Registradores\n");
        printf("5. Estado completo\n");
        printf("8. Run\n");
        printf("9. Step\n");
        printf("10. Back\n");
        printf("11. Reset PC\n");
        printf("12. Sair\n");
        printf("Opcao: ");
        scanf("%d", &op);

        switch(op) {

            case 1:
                carregar_memoria(sim);
                break;

            case 2:
                carregar_dados(&sim->dmem);
                break;

            case 3:
                imprimir_memoria_instrucao(sim);
                imprimir_memoria_dados(&sim->dmem);
                break;

            case 4:
                mostrar_registradores(sim->reg);
                break;

            case 5:
                mostrar_pc(sim);
                mostrar_registradores(sim->reg);
                imprimir_memoria_instrucao(sim);
                imprimir_memoria_dados(&sim->dmem);
                break;

            case 8:
                run_simulation(sim);
                break;

            case 9:
                step_simulation(sim);
                break;

            case 10:
                voltar_instrucao(sim);
                break;

            case 11:
                reset_pc(sim);
                break;

            case 12:
                printf("Saindo...\n");
                break;
        }

    } while(op != 12);
}

// ================= MAIN =================

int main() {

    struct instrucao programa[100];

    struct simulador sim = {
        .dmem = {0},
        .pc = {0, -1},
        .reg = {0},
        .programa = programa,
        .prog_size = 0,
        .ula = {0,0,0}
    };

    menu(&sim);

    return 0;
}