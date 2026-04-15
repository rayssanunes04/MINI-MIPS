#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_SIZE 256
#define INSTR_SIZE 16
#define REG_COUNT 8  

enum classe_inst {
    tipo_I,
    tipo_J,
    tipo_R
};

struct instrucao {
    enum classe_inst tipo_inst;
    char inst_char[INSTR_SIZE + 1]; // guardando a inst em binario
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
    int prev_pc; // voltando o back
};

struct ULA {
    int entrada1;
    int entrada2;
    int resultado;
};

struct controle {
    int alu_op;
    int mem_read; //lw 
    int mem_write; // sw
    int reg_write; // escrevndo reg
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



// mostrar PC
void mostrar_pc(struct simulador *sim) {
    printf("\nPC atual: %d\n", sim->pc.pc);
}

// mostrar nome da instrução
void mostrar_instrucao(struct instrucao *inst) {

    printf("Tipo: ");

    if (inst->tipo_inst == tipo_R) {
        if (inst->funct == 0) printf("ADD\n");
        else if (inst->funct == 1) printf("SUB\n");
        else if (inst->funct == 2) printf("AND\n");
        else if (inst->funct == 3) printf("OR\n");
        else printf("R\n");
    }
    else if (inst->tipo_inst == tipo_I) {
        if (inst->opcode == 8) printf("ADDI\n");
        else if (inst->opcode == 4) printf("LW\n");
        else if (inst->opcode == 5) printf("SW\n");
        else if (inst->opcode == 9) printf("BEQ\n");
        else printf("I\n");
    }
    else {
        printf("JUMP\n");
    }
}

// digitar memoria
void digitar_memoria(struct memoria_dados *mem) { // recebendo a memoria

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

        mem->dados[pos] = valor; // armazena na memoria
    }
}

// definir registradores
void definir_registradores(int reg[]) {

    printf("\n=== DEFINIR REGISTRADORES (R0 a R7) ===\n");

    for (int i = 0; i < REG_COUNT; i++) {
        printf("R%d: ", i);
        scanf("%d", &reg[i]);
    }
}

void imprimir_memoria(struct memoria_dados *mem) { // rec ponteiro para a minha memoria
    int i;

    printf("\n=== MEMORIA ===\n");

    for (i = 0; i < DATA_SIZE; i++) {

        if (mem->dados[i] != 0) {
            printf("Mem[%d] = %d\n", i, mem->dados[i]);
        }
    }
}
// mostra posição e valor da memoria
void mostrar_registradores(int reg[]) { 

    int i;

    printf("\n=== REGISTRADORES ===\n");

    for (i = 0; i < REG_COUNT; i++) {
        printf("R%d = %d\n", i, reg[i]);
    }
}
// imprimindo os 8 reg


int executar_ula(struct ULA *ula, int op) {

    switch(op) {
//add
        case 0:
            ula->resultado = ula->entrada1 + ula->entrada2;
            break;
//sub
        case 1:
            ula->resultado = ula->entrada1 - ula->entrada2;
            break;
//and
        case 2:
            ula->resultado = ula->entrada1 & ula->entrada2;
            break;
//or
        case 3:
            ula->resultado = ula->entrada1 | ula->entrada2;
            break;

        default:
            ula->resultado = 0;
            break;
    }

    return ula->resultado;
}


// instrução ja vem decodificada
void unidade_controle(struct instrucao *inst, struct controle *ctrl) {

    ctrl->alu_op = 0;
    ctrl->mem_read = 0;
    ctrl->mem_write = 0;
    ctrl->reg_write = 0;
// tipo R
    if (inst->tipo_inst == tipo_R) {

        ctrl->reg_write = 1; // escrevendo no meu registrador
        ctrl->alu_op = inst->funct; // op da ULA vem do funct AQUI DEFINE
    }
//i
    else if (inst->tipo_inst == tipo_I) {
//addi
        if (inst->opcode == 8) {

            ctrl->reg_write = 1;
            ctrl->alu_op = 0; // usa soma
        }
//lw
        else if (inst->opcode == 4) {

            ctrl->mem_read = 1; // le a memoria
            ctrl->reg_write = 1; // escreve no reg
        }
//sw
        else if (inst->opcode == 5) {

            ctrl->mem_write = 1; // escreve na memoria
        }
    }
}


void decodificador(struct instrucao *inst) { // transforma binario 

    int i;

    inst->inst_char[INSTR_SIZE] = '\0'; // garante fim da string

    inst->opcode = 0;
// converte os 4 primeiros bits em inteiro
    for (i = 0; i < 4; i++) {
        inst->opcode = (inst->opcode << 1) | (inst->inst_char[i] - '0');
    }
// opcode 0 → tipo R
    if (inst->opcode == 0) {
        inst->tipo_inst = tipo_R;
    }
//opcode 2 → tipo J
    else if (inst->opcode == 2) {
        inst->tipo_inst = tipo_J;
    }
        // resto → tipo I
    else {
        inst->tipo_inst = tipo_I;
    }
// PARTE R
    if (inst->tipo_inst == tipo_R) {
//zera tudo
        inst->rs = inst->rt = inst->rd = inst->funct = 0;
//bits 4–6 → rs
        for (i = 4; i < 7; i++)
            inst->rs = (inst->rs << 1) | (inst->inst_char[i] - '0');
//bits 7–9 → rt
        for (i = 7; i < 10; i++)
            inst->rt = (inst->rt << 1) | (inst->inst_char[i] - '0');
//bits 10–12 → rd
        for (i = 10; i < 13; i++)
            inst->rd = (inst->rd << 1) | (inst->inst_char[i] - '0');
//bits 13–15 → funct
        for (i = 13; i < 16; i++)
            inst->funct = (inst->funct << 1) | (inst->inst_char[i] - '0');
    }
// PARTE I
    else if (inst->tipo_inst == tipo_I) {

        inst->rs = inst->rt = inst->imm = 0;

        for (i = 4; i < 7; i++)
            inst->rs = (inst->rs << 1) | (inst->inst_char[i] - '0');

        for (i = 7; i < 10; i++)
            inst->rt = (inst->rt << 1) | (inst->inst_char[i] - '0');
//bits finais → imediato
        for (i = 10; i < 16; i++)
            inst->imm = (inst->imm << 1) | (inst->inst_char[i] - '0');
    }
// Parte J
    else {

        inst->addr = 0;
// endereço
        for (i = 4; i < 16; i++)
            inst->addr = (inst->addr << 1) | (inst->inst_char[i] - '0');
    }
}


void executar_instrucao(struct simulador *sim, struct instrucao *inst) { // aqui q vao o valores do meu reg
// se for ADDI
    if (inst->tipo_inst == tipo_I && inst->opcode == 8) {
// soma e salva
        sim->reg[inst->rt] = sim->reg[inst->rs] + inst->imm; // pegando valor do rs, somando com imediato e guardando em rt
    }
//beq
    else if (inst->tipo_inst == tipo_I && inst->opcode == 9) {
//compara
        if (sim->reg[inst->rs] == sim->reg[inst->rt]) {
            sim->pc.pc += inst->imm; // pula e sai da minha função
            return;
        }
    }
// TIPO R
    else if (inst->tipo_inst == tipo_R && sim->ctrl.reg_write) {

        sim->ula.entrada1 = sim->reg[inst->rs];
        sim->ula.entrada2 = sim->reg[inst->rt];

        sim->reg[inst->rd] =
            executar_ula(&sim->ula, sim->ctrl.alu_op); // executa e salva
    }
//lw
    else if (sim->ctrl.mem_read) {

        sim->reg[inst->rt] =
            sim->dmem.dados[sim->reg[inst->rs] + inst->imm]; // le a memoria
    }
//sw
    else if (sim->ctrl.mem_write) {

        sim->dmem.dados[sim->reg[inst->rs] + inst->imm] =
            sim->reg[inst->rt]; //escrevendo na memoria
    }
// jump
    else if (inst->tipo_inst == tipo_J) {

        sim->pc.pc = inst->addr; // muda PC
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

    if (inst->tipo_inst != tipo_J) {
        sim->pc.pc++;
    }
}


void run_simulation(struct simulador *sim) {

    while (sim->pc.pc < sim->prog_size) {
        step_simulation(sim);
    }
}

//back

void voltar_instrucao(struct simulador *sim) {

    if (sim->pc.prev_pc >= 0) {
        sim->pc.pc = sim->pc.prev_pc;
        printf("\nVoltou para instrucao %d\n", sim->pc.pc);

        struct instrucao *inst = &sim->programa[sim->pc.pc];
        decodificador(inst);
        mostrar_instrucao(inst);
    }
}


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

            case 6:
                mostrar_pc(sim);
                break;

            case 7:
                digitar_memoria(&sim->dmem);
                break;

            case 8:
                definir_registradores(sim->reg);
                break;

            case 0:
                break;

            default:
                printf("Opcao invalida\n");
                break;
        }

    } while(op != 0);
}


int main() {

    FILE *arq = fopen("teste.mem", "r");

    if (!arq) {
        printf("Erro ao abrir teste.mem\n");
        return 1;
    }

    struct instrucao programa[100]; // estou guardando minhas instruçoes 
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
