#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_SIZE 16   // Ajustado para 8 bits
#define INSTR_SIZE 8   // Ajustado para 8 bits
#define REG_COUNT 8    

enum classe_inst { tipo_I, tipo_J, tipo_R };

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

// --- SUAS FUNÇÕES DE EXIBIÇÃO E INTERFACE ---

void mostrar_pc(struct simulador *sim) {
    printf("\nPC atual: %d\n", sim->pc.pc);
}

void mostrar_instrucao(struct instrucao *inst) {
    printf("Tipo: ");
    if (inst->tipo_inst == tipo_R) {
        if (inst->funct == 0) printf("ADD\n");
        else if (inst->funct == 1) printf("SUB\n");
        else printf("R\n");
    } else if (inst->tipo_inst == tipo_I) {
        if (inst->opcode == 1) printf("ADDI\n"); // Ajustado opcode para 8 bits
        else if (inst->opcode == 2) printf("LW\n");
        else printf("I\n");
    } else {
        printf("JUMP\n");
    }
}

void digitar_memoria(struct memoria_dados *mem) {
    int n, pos, valor;
    printf("\n=== MEMORIA DE DADOS ===\n");
    printf("Quantas posicoes deseja preencher? ");
    scanf("%d", &n);
    for (int i = 0; i < n; i++) {
        do {
            printf("Endereco (0 a %d): ", DATA_SIZE-1);
            scanf("%d", &pos);
        } while (pos < 0 || pos >= DATA_SIZE);
        printf("Valor: ");
        scanf("%d", &valor);
        mem->dados[pos] = valor;
    }
}

void definir_registradores(int reg[]) {
    printf("\n=== DEFINIR REGISTRADORES (R0 a R7) ===\n");
    for (int i = 0; i < REG_COUNT; i++) {
        printf("R%d: ", i);
        scanf("%d", &reg[i]);
    }
}

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

// --- LOGICA DE EXECUÇÃO ---

int executar_ula(struct ULA *ula, int op) {
    switch(op) {
        case 0: ula->resultado = ula->entrada1 + ula->entrada2; break;
        case 1: ula->resultado = ula->entrada1 - ula->entrada2; break;
        default: ula->resultado = 0; break;
    }
    return ula->resultado;
}

void unidade_controle(struct instrucao *inst, struct controle *ctrl) {
    ctrl->alu_op = 0; ctrl->mem_read = 0; ctrl->mem_write = 0; ctrl->reg_write = 0;
    if (inst->tipo_inst == tipo_R) {
        ctrl->reg_write = 1;
        ctrl->alu_op = inst->funct;
    } else if (inst->tipo_inst == tipo_I) {
        if (inst->opcode == 1) { ctrl->reg_write = 1; ctrl->alu_op = 0; }
        else if (inst->opcode == 2) { ctrl->mem_read = 1; ctrl->reg_write = 1; }
    }
}

void decodificador(struct instrucao *inst) {
    int i;
    inst->inst_char[INSTR_SIZE] = '\0';
    inst->opcode = 0;
    // Opcode: 2 bits (0 e 1)
    for (i = 0; i < 2; i++) inst->opcode = (inst->opcode << 1) | (inst->inst_char[i] - '0');

    if (inst->opcode == 0) inst->tipo_inst = tipo_R;
    else if (inst->opcode == 3) inst->tipo_inst = tipo_J;
    else inst->tipo_inst = tipo_I;

    // Divisão para 8 bits: [OP:2] [RS:2] [RT/RD:2] [Funct/Imm:2]
    inst->rs = 0; for (i = 2; i < 4; i++) inst->rs = (inst->rs << 1) | (inst->inst_char[i] - '0');
    inst->rt = 0; for (i = 4; i < 6; i++) inst->rt = (inst->rt << 1) | (inst->inst_char[i] - '0');
    inst->rd = inst->rt; 
    
    inst->imm = 0;
    inst->funct = 0;
    for (i = 6; i < 8; i++) {
        int bit = inst->inst_char[i] - '0';
        inst->imm = (inst->imm << 1) | bit;
        inst->funct = (inst->funct << 1) | bit;
    }
    
    if (inst->tipo_inst == tipo_J) {
        inst->addr = 0;
        for (i = 2; i < 8; i++) inst->addr = (inst->addr << 1) | (inst->inst_char[i] - '0');
    }
}

void executar_instrucao(struct simulador *sim, struct instrucao *inst) {
    if (inst->tipo_inst == tipo_I && inst->opcode == 1) { // ADDI
        sim->reg[inst->rt] = sim->reg[inst->rs] + inst->imm;
    } 
    else if (inst->tipo_inst == tipo_R && sim->ctrl.reg_write) {
        sim->ula.entrada1 = sim->reg[inst->rs];
        sim->ula.entrada2 = sim->reg[inst->rt];
        sim->reg[inst->rd] = executar_ula(&sim->ula, sim->ctrl.alu_op);
    }
    else if (sim->ctrl.mem_read) {
        sim->reg[inst->rt] = sim->dmem.dados[inst->imm % DATA_SIZE];
    }
    else if (inst->tipo_inst == tipo_J) {
        sim->pc.pc = inst->addr;
    }
}

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
    mostrar_instrucao(inst);
    executar_instrucao(sim, inst);
    if (inst->tipo_inst != tipo_J) sim->pc.pc++;
}

void run_simulation(struct simulador *sim) {
    while (sim->pc.pc < sim->prog_size) step_simulation(sim);
}

void voltar_instrucao(struct simulador *sim) {
    if (sim->pc.prev_pc >= 0) {
        sim->pc.pc = sim->pc.prev_pc;
        printf("\nVoltou para instrucao %d\n", sim->pc.pc);
    }
}

// --- SEU MENU ORIGINAL ---

void menu(struct simulador *sim) {
    int op;
    do {
        printf("\nMenu principal:\n");
        printf("1. Imprimir memórias\n");
        printf("2. Imprimir registradores\n");
        printf("3. Executar programa (run)\n");
        printf("4. Executar passo (step)\n");
        printf("5. Voltar instrução (back)\n");
        printf("6. Ver valor do PC\n");
        printf("7. Digitar memoria de dados\n");
        printf("8. Definir valores dos registradores\n");
        printf("0. Sair\n");
        printf("Opcao: ");
        scanf("%d", &op);
        switch(op) {
            case 1: imprimir_memoria(&sim->dmem); break;
            case 2: mostrar_registradores(sim->reg); break;
            case 3: run_simulation(sim); break;
            case 4: step_simulation(sim); break;
            case 5: voltar_instrucao(sim); break;
            case 6: mostrar_pc(sim); break;
            case 7: digitar_memoria(&sim->dmem); break;
            case 8: definir_registradores(sim->reg); break;
            case 0: break;
            default: printf("Opcao invalida\n"); break;
        }
    } while(op != 0);
}

int main() {
    FILE *arq = fopen("teste.mem", "r");
    if (!arq) { printf("Erro ao abrir teste.mem\n"); return 1; }
    struct instrucao *programa = malloc(100 * sizeof(struct instrucao));
    int i = 0;
    while (fscanf(arq, "%s", programa[i].inst_char) != EOF) i++;
    fclose(arq);

    struct simulador sim = { .dmem={0}, .pc={0, -1}, .reg={0}, .programa=programa, .prog_size=i };
    menu(&sim);
    free(programa);
    return 0;
}
