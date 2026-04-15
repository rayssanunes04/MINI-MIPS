#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_SIZE 16
#define INSTR_SIZE 8
#define REG_COUNT 8

enum classe_inst { tipo_R, tipo_I, tipo_J };

struct instrucao {
    enum classe_inst tipo_inst;
    char inst_char[INSTR_SIZE + 1];
    int opcode;
    int rs, rt, imm, addr;
};

struct memoria_dados { int dados[DATA_SIZE]; };
struct pc { int pc; int prev_pc; };
struct ULA { int entrada1, entrada2, resultado; };
struct controle { int alu_op, mem_read, mem_write, reg_write; };

struct simulador {
    struct memoria_dados dmem;
    struct pc pc;
    int reg[REG_COUNT];
    struct instrucao *programa;
    int prog_size;
    struct ULA ula;
    struct controle ctrl;
};

void decodificador(struct instrucao *inst) {
    int i;
    inst->inst_char[INSTR_SIZE] = '\0';
    
    // Opcode: bits 0-1
    inst->opcode = 0;
    for (i = 0; i < 2; i++) inst->opcode = (inst->opcode << 1) | (inst->inst_char[i] - '0');

    // RS (Registrador 1): bits 2-4
    inst->rs = 0;
    for (i = 2; i < 5; i++) inst->rs = (inst->rs << 1) | (inst->inst_char[i] - '0');

    // RT / Imm (Registrador 2 ou Valor): bits 5-7
    inst->rt = 0;
    inst->imm = 0;
    for (i = 5; i < 8; i++) {
        int bit = inst->inst_char[i] - '0';
        inst->rt = (inst->rt << 1) | bit;
        inst->imm = (inst->imm << 1) | bit;
    }

    if (inst->opcode == 0) inst->tipo_inst = tipo_R;
    else if (inst->opcode == 3) inst->tipo_inst = tipo_J;
    else inst->tipo_inst = tipo_I;
}

void mostrar_estado(struct simulador *sim) {
    printf("\n--- ESTADO ATUAL ---");
    printf("\nPC: %d | Registradores: ", sim->pc.pc);
    for(int i=0; i<REG_COUNT; i++) printf("R%d:%d ", i, sim->reg[i]);
    printf("\n--------------------\n");
}

void executar_instrucao(struct simulador *sim) {
    if (sim->pc.pc >= sim->prog_size) return;

    struct instrucao *inst = &sim->programa[sim->pc.pc];
    decodificador(inst);
    sim->pc.prev_pc = sim->pc.pc;

    printf("Executando: %s (Op:%d)\n", inst->inst_char, inst->opcode);

    switch(inst->opcode) {
        case 0: // ADD: R[rs] = R[rs] + R[rt]
            sim->reg[inst->rs] += sim->reg[inst->rt];
            sim->pc.pc++;
            break;
        case 1: // ADDI: R[rs] = R[rs] + imm
            sim->reg[inst->rs] += inst->imm;
            sim->pc.pc++;
            break;
        case 2: // SW: Mem[imm] = R[rs]
            sim->dmem.dados[inst->imm % DATA_SIZE] = sim->reg[inst->rs];
            sim->pc.pc++;
            break;
        case 3: // JUMP: PC = imm
            sim->pc.pc = inst->imm;
            break;
    }
}

void menu(struct simulador *sim) {
    int op;
    do {
        printf("\n1. Step | 2. Registradores | 3. Memoria | 4. PC | 0. Sair\nOpcao: ");
        if (scanf("%d", &op) != 1) break;
        switch(op) {
            case 1: executar_instrucao(sim); mostrar_estado(sim); break;
            case 2: for(int i=0; i<REG_COUNT; i++) printf("R%d = %d\n", i, sim->reg[i]); break;
            case 3: for(int i=0; i<DATA_SIZE; i++) if(sim->dmem.dados[i]!=0) printf("M[%d]=%d\n", i, sim->dmem.dados[i]); break;
            case 4: printf("PC: %d\n", sim->pc.pc); break;
        }
    } while(op != 0);
}

int main() {
    FILE *arq = fopen("teste.mem", "r");
    if (!arq) { 
        printf("Erro: Crie o arquivo 'teste.mem' com instrucoes de 8 bits.\n");
        return 1; 
    }

    struct instrucao *programa = malloc(100 * sizeof(struct instrucao));
    int i = 0;
    while (fscanf(arq, "%s", programa[i].inst_char) != EOF) {
        if (strlen(programa[i].inst_char) == 8) i++;
    }
    fclose(arq);

    struct simulador sim = { .dmem={0}, .pc={0,-1}, .reg={0}, .programa=programa, .prog_size=i };
    
    printf("Simulador 8-bits carregado (%d instrucoes).\n", i);
    menu(&sim);

    free(programa);
    return 0;
}
