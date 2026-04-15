#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_SIZE 256
#define INSTR_SIZE 8
#define REG_COUNT 4

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

```
struct ULA ula;
struct controle ctrl;
```

};

void mostrar_instrucao(struct instrucao *inst) {

```
if (inst->tipo_inst == tipo_R) {
    if (inst->funct == 0) printf("ADD\n");
    else if (inst->funct == 1) printf("SUB\n");
    else if (inst->funct == 2) printf("AND\n");
    else if (inst->funct == 3) printf("OR\n");
}
else if (inst->tipo_inst == tipo_I) {
    if (inst->opcode == 2) printf("ADDI\n");
    else if (inst->opcode == 3) printf("LW/SW\n");
}
else {
    printf("JUMP\n");
}
```

}

void mostrar_registradores(int reg[]) {

```
int i;

printf("\n=== REGISTRADORES ===\n");

for (i = 0; i < REG_COUNT; i++) {
    printf("R%d = %d\n", i, reg[i]);
}
```

}

void decodificador(struct instrucao *inst) {

```
int i;

inst->inst_char[INSTR_SIZE] = '\0';

inst->opcode = 0;

// bits 0-1
for (i = 0; i < 2; i++) {
    inst->opcode = (inst->opcode << 1) | (inst->inst_char[i] - '0');
}

// 00 -> R
if (inst->opcode == 0)
    inst->tipo_inst = tipo_R;

// 01 -> J
else if (inst->opcode == 1)
    inst->tipo_inst = tipo_J;

// 10 e 11 -> I
else
    inst->tipo_inst = tipo_I;


if (inst->tipo_inst == tipo_R) {

    inst->rs = 0;
    inst->rt = 0;
    inst->funct = 0;

    // bits 2-3
    for (i = 2; i < 4; i++)
        inst->rs = (inst->rs << 1) | (inst->inst_char[i] - '0');

    // bits 4-5
    for (i = 4; i < 6; i++)
        inst->rt = (inst->rt << 1) | (inst->inst_char[i] - '0');

    // bits 6-7
    for (i = 6; i < 8; i++)
        inst->funct = (inst->funct << 1) | (inst->inst_char[i] - '0');

    inst->rd = inst->rt;
}

else if (inst->tipo_inst == tipo_I) {

    inst->rs = 0;
    inst->rt = 0;
    inst->imm = 0;

    // bits 2-3
    for (i = 2; i < 4; i++)
        inst->rs = (inst->rs << 1) | (inst->inst_char[i] - '0');

    // bits 4-5
    for (i = 4; i < 6; i++)
        inst->rt = (inst->rt << 1) | (inst->inst_char[i] - '0');

    // bits 6-7
    for (i = 6; i < 8; i++)
        inst->imm = (inst->imm << 1) | (inst->inst_char[i] - '0');
}

else {

    inst->addr = 0;

    // bits 2-7
    for (i = 2; i < 8; i++)
        inst->addr = (inst->addr << 1) | (inst->inst_char[i] - '0');
}
```

}

int executar_ula(struct ULA *ula, int op) {

```
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
}

return ula->resultado;
```

}

void executar_instrucao(struct simulador *sim, struct instrucao *inst) {

```
if (inst->tipo_inst == tipo_R) {

    sim->ula.entrada1 = sim->reg[inst->rs];
    sim->ula.entrada2 = sim->reg[inst->rt];

    sim->reg[inst->rd] = executar_ula(&sim->ula, inst->funct);
}

else if (inst->tipo_inst == tipo_I) {

    // opcode 10 = ADDI
    if (inst->opcode == 2) {
        sim->reg[inst->rt] = sim->reg[inst->rs] + inst->imm;
    }

    // opcode 11 = SW simples
    else if (inst->opcode == 3) {
        sim->dmem.dados[inst->imm] = sim->reg[inst->rt];
    }
}

else if (inst->tipo_inst == tipo_J) {
    sim->pc.pc = inst->addr;
    return;
}

sim->pc.pc++;
```

}

void step_simulation(struct simulador *sim) {

```
if (sim->pc.pc >= sim->prog_size) {
    printf("Fim do programa\n");
    return;
}

struct instrucao *inst = &sim->programa[sim->pc.pc];

decodificador(inst);

printf("\nInstrucao %d: %s -> ", sim->pc.pc, inst->inst_char);
mostrar_instrucao(inst);

executar_instrucao(sim, inst);

mostrar_registradores(sim->reg);
```

}

int main() {

```
FILE *arq = fopen("teste.mem", "r");

if (!arq) {
    printf("Erro ao abrir teste.mem\n");
    return 1;
}

struct instrucao programa[100];
int i = 0;

while (fscanf(arq, "%8s", programa[i].inst_char) != EOF) {
    i++;
}

fclose(arq);

struct simulador sim = {0};

sim.programa = programa;
sim.prog_size = i;
sim.pc.pc = 0;

while (sim.pc.pc < sim.prog_size) {
    step_simulation(&sim);
}

return 0;
```

}

/*
Exemplo de teste.mem

00010100
00011001
10000111
01000101

*/

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

// ================= FUNÇÕES =================

// IMPRIMIR MEMÓRIA
void imprimir_memoria(struct memoria_dados *mem) {
    printf("\nMEMORIA:\n");
    for (int i = 0; i < DATA_SIZE; i++) {
        if (mem->dados[i] != 0) {
            printf("Mem[%d] = %d\n", i, mem->dados[i]);
        }
    }
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

// SALVAR .dat
void salvar_memoria_dados(struct memoria_dados *mem, const char *nome_arquivo) {
    FILE *arq = fopen(nome_arquivo, "w");

    if (!arq) {
        printf("Erro ao salvar arquivo\n");
        return;
    }

    for (int i = 0; i < DATA_SIZE; i++) {
        if (mem->dados[i] != 0) {
            fprintf(arq, "%d %d\n", i, mem->dados[i]);
        }
    }

    fclose(arq);
    printf("Memoria salva em %s\n", nome_arquivo);
}

// SALVAR .asm
void salvar_programa(struct simulador *sim, const char *nome_arquivo) {
    FILE *arq = fopen(nome_arquivo, "w");

    if (!arq) {
        printf("Erro ao salvar arquivo\n");
        return;
    }

    for (int i = 0; i < sim->prog_size; i++) {
        fprintf(arq, "%s\n", sim->programa[i].inst_char);
    }

    fclose(arq);
    printf("Programa salvo em %s\n", nome_arquivo);
}

// ULA
int executar_ula(struct ULA *ula, int operacao) {
    switch(operacao) {
        case 0: ula->resultado = ula->entrada1 + ula->entrada2; break;
        case 1: ula->resultado = ula->entrada1 - ula->entrada2; break;
        case 2: ula->resultado = ula->entrada1 & ula->entrada2; break;
        case 3: ula->resultado = ula->entrada1 | ula->entrada2; break;
        case 4: ula->resultado = (ula->entrada1 < ula->entrada2) ? 1 : 0; break;
        default: ula->resultado = 0;
    }
    return ula->resultado;
}

// CONTROLE
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
        if (inst->opcode == 4) {
            ctrl->mem_read = 1;
            ctrl->reg_write = 1;
        }
        else if (inst->opcode == 5) {
            ctrl->mem_write = 1;
        }
    }
}

// DECODIFICADOR
void decodificador(struct instrucao *inst) {

    inst->inst_char[INSTR_SIZE] = '\0';

    char opcode_str[5];
    strncpy(opcode_str, inst->inst_char, 4);
    opcode_str[4] = '\0';
    inst->opcode = strtol(opcode_str, NULL, 2);

    if (inst->opcode == 0) {
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

// EXECUÇÃO
void executar_instrucao(struct simulador *sim, struct instrucao *inst) {

    if (inst->tipo_inst == tipo_R && sim->ctrl.reg_write) {

        sim->ula.entrada1 = sim->reg[inst->rs];
        sim->ula.entrada2 = sim->reg[inst->rt];

        sim->reg[inst->rd] =
            executar_ula(&sim->ula, sim->ctrl.alu_op);

        printf("R%d = %d\n", inst->rd, sim->reg[inst->rd]);
    }

    if (sim->ctrl.mem_read) {
        sim->reg[inst->rt] =
            sim->dmem.dados[sim->reg[inst->rs] + inst->imm];
    }

    if (sim->ctrl.mem_write) {
        sim->dmem.dados[sim->reg[inst->rs] + inst->imm] =
            sim->reg[inst->rt];
    }

    if (inst->tipo_inst == tipo_J) {
        sim->pc.pc = inst->addr;
    }
}

// RUN
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

// STEP
void step_simulation(struct simulador *sim) {

    if (sim->pc.pc >= sim->prog_size) {
        printf("Fim do programa\n");
        return;
    }

    struct instrucao *inst = &sim->programa[sim->pc.pc];

    sim->pc.prev_pc = sim->pc.pc;

    decodificador(inst);
    unidade_controle(inst, &sim->ctrl);
    executar_instrucao(sim, inst);

    if (inst->tipo_inst != tipo_J) {
        sim->pc.pc++;
    }
}

// MENU
void menu(struct simulador *sim) {
    int op;

    do {
        printf("\n===== MENU =====\n");
        printf("1. Imprimir memoria\n");
        printf("2. Imprimir registradores\n");
        printf("3. Run\n");
        printf("4. Step\n");
        printf("5. Back\n");
        printf("6. Salvar .dat\n");
        printf("7. Salvar .asm\n");
        printf("0. Sair\n");
        printf("Opcao: ");
        scanf("%d", &op);

        switch(op) {

            case 1:
                imprimir_memoria(&sim->dmem);
                break;

            case 2:
                for (int i = 0; i < REG_COUNT; i++) {
                    printf("R%d = %d\n", i, sim->reg[i]);
                }
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

            case 6:
                salvar_memoria_dados(&sim->dmem, "saida.dat");
                break;

            case 7:
                salvar_programa(sim, "saida.asm");
                break;

        }

    } while (op != 0);
}

// ===== MAIN =====
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
        .prog_size = i
    };

    sim.reg[1] = 0;
    sim.dmem.dados[42] = 1234;

    menu(&sim);

    return 0;
}
