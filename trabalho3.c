#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_SIZE 256
#define INSTR_SIZE 16
#define REG_COUNT 8  



struct instrucao {
    char inst_char[INSTR_SIZE + 1]; // armazenando inst em binario
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
    int pc; // contador do meu programa (aponta a inst atual)
    int prev_pc; // guarda posição anterior
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
    struct instrucao *programa; // vetor de inst carregadas
    int prog_size;
    struct ULA ula;
};

// ================= INSTRUÇÕES =================

void carregar_memoria(struct simulador *sim) { // função q esta lendo o arquivo e coloca as inst na memoriqa

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
        if (strlen(linha) == INSTR_SIZE) { // garante q a minha inst tenha 16 bits
            strcpy(sim->programa[i].inst_char, linha); // copiando a ints para memoria
            i++;
        }
    }

    sim->prog_size = i; // salvando a quantidade de inst

    fclose(arq);

    printf("Programa carregado com %d instrucoes!\n", i);
}


void carregar_dados(struct memoria_dados *mem) { // carregando meus dados

    FILE *arq = fopen("dados.dat", "r");

    if (!arq) {
        printf("Erro ao abrir dados.dat!\n");
        return;
    }

    int pos, valor;

    while (fscanf(arq, "%d %d", &pos, &valor) == 2) { // lendo posição e o valor

        if (pos >= 0 && pos < DATA_SIZE) {
            mem->dados[pos] = valor; // escrevendo na memoria 
        }
    }

    fclose(arq);

    printf("Memoria de dados carregada!\n"); 
}



void imprimir_memoria_instrucao(struct simulador *sim) { // mostra todas inst carregadas

    printf("\n=== MEMORIA DE INSTRUCOES ===\n");

    for (int i = 0; i < sim->prog_size; i++) { // mostra indice e inst binaria (guardando quantas int foram carregadas na memoria) 
        printf("[%d] = %s\n", i, sim->programa[i].inst_char);
    }
}

void imprimir_memoria_dados(struct memoria_dados *mem) { // mostra valores armazenados na memoria

    printf("\n=== MEMORIA DE DADOS ===\n");

    for (int i = 0; i < DATA_SIZE; i++) {
        if (mem->dados[i] != 0) { // so vai mostrar as posiçoes q foram usadas
            printf("Mem[%d] = %d\n", i, mem->dados[i]);
        }
    }
}


void mostrar_registradores(int reg[]) { // exebir meus reg

    printf("\n=== REGISTRADORES ===\n");

    for (int i = 0; i < REG_COUNT; i++) {
        printf("R%d = %d\n", i, reg[i]);
    }
}



void mostrar_pc(struct simulador *sim) { // exibir o pc
    printf("\nPC atual: %d\n", sim->pc.pc); // mostra apenas a posição atual da inst
}


int executar_ula(struct ULA *ula, int op) {

    switch(op) {

        case 0: //add
            return ula->entrada1 + ula->entrada2;

        case 2: //sub
            return ula->entrada1 - ula->entrada2;

        case 4: // and
            return ula->entrada1 & ula->entrada2;

        case 5: //or
            return ula->entrada1 | ula->entrada2;

        default:
            return 0;
    }
}


void decodificador(struct instrucao *inst) { // transforma string binária em campos (opcode, rs, rt...)

    inst->rs = inst->rt = inst->rd = 0; // inicializa os reg da inst, neste caso estou zerando os reg
    inst->funct = inst->imm = inst->addr = 0;

    inst->inst_char[INSTR_SIZE] = '\0'; // coloca caractere nulo no final da instrução

    inst->opcode = 0; // inicializaçao do op
    for (int i = 0; i < 4; i++) // percorre os 4 primeiros bits da instrução
        inst->opcode = (inst->opcode << 1) | (inst->inst_char[i] - '0'); // conversão binário → inteiro

    if (inst->opcode == 0) { // vendo se é do tipo R
// percorrendoi meus registradores rs e rt e rd, e funct
        for (int i = 4; i < 7; i++)
            inst->rs = (inst->rs << 1) | (inst->inst_char[i] - '0');

        for (int i = 7; i < 10; i++)
            inst->rt = (inst->rt << 1) | (inst->inst_char[i] - '0');

        for (int i = 10; i < 13; i++)
            inst->rd = (inst->rd << 1) | (inst->inst_char[i] - '0');

        for (int i = 13; i < 16; i++)
            inst->funct = (inst->funct << 1) | (inst->inst_char[i] - '0');//campo funct ula
    }
    else {
// tipo I e J
        for (int i = 4; i < 7; i++)
            inst->rs = (inst->rs << 1) | (inst->inst_char[i] - '0'); // pegando valor atual, deslocando , adicionando novo bit

        for (int i = 7; i < 10; i++)
            inst->rt = (inst->rt << 1) | (inst->inst_char[i] - '0');

        for (int i = 10; i < 16; i++)
            inst->imm = (inst->imm << 1) | (inst->inst_char[i] - '0'); // percorrendo imm
    }
}



void mostrar_instrucao(struct instrucao *inst) { // transformando meu binario para texto

    switch(inst->opcode) { // ecolhendo o tipo de inst

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


void executar_instrucao(struct simulador *sim, struct instrucao *inst) { // interpreta a instrução já decodificada e executa a operação

    switch(inst->opcode) { // qual op vai executar

        case 4:
            //addi
            sim->reg[inst->rt] = sim->reg[inst->rs] + inst->imm;// soma registrador rs com valor imediato e salva em rt
            break;

        case 8:
            //beq
            if (sim->reg[inst->rs] == sim->reg[inst->rt]) { //verifica se dois registradores são iguais
                sim->pc.pc += inst->imm; // pula instruções (branch)
            }
            break;

        case 11:
            //lw
            sim->reg[inst->rt] = // destino do valor carregado
                sim->dmem.dados[sim->reg[inst->rs] + inst->imm]; // pega valor da memória usando endereço calculado
            break;

        case 15:
            //sw
            sim->dmem.dados[sim->reg[inst->rs] + inst->imm] = // endereço onde será escrito
                sim->reg[inst->rt]; // valor que será armazenado ( pega no reg e salva na memoria)
            break;

        case 0: {
            // tipo R
            sim->ula.entrada1 = sim->reg[inst->rs]; // coloca rs na ULA
            sim->ula.entrada2 = sim->reg[inst->rt]; // coloca rt na ULA

            int result = executar_ula(&sim->ula, inst->funct); // chamando a ula, executando R e guaradndo resultado, ULA ESTA SENDO EXECUTA AQUI

            sim->reg[inst->rd] = result;//salva resul no reg destino

            printf("DEBUG: R%d = %d\n", inst->rd, result);
            break;
        }

        case 2:
            sim->pc.pc = inst->addr; // aletera o pc (pula para o end especifico 0
            break;
    }
}


void step_simulation(struct simulador *sim) {

    if (sim->pc.pc >= sim->prog_size) { // ve se acabou
        printf("Fim do programa\n");
        return;
    }

    struct instrucao *inst = &sim->programa[sim->pc.pc]; // pegando a minha inst atuaç e usando Pc como indice

    sim->pc.prev_pc = sim->pc.pc; // salvando posiçao anterior para o back

    decodificador(inst); // transformando binario em campos

    printf("\nExecutando %d: %s\n", sim->pc.pc, inst->inst_char); //imprime número de ints(PC) e int em binario 

    mostrar_instrucao(inst); // traduzao para nossa linguagem

    executar_instrucao(sim, inst); // executa

    if (inst->opcode != 2) // se n for um jump
        sim->pc.pc++; // avança para a proxima inst
}

void run_simulation(struct simulador *sim) {

    while (sim->pc.pc < sim->prog_size)
        step_simulation(sim); // executa 1 inst por vez
}

// ================= BACK =================

void voltar_instrucao(struct simulador *sim) {

    if (sim->pc.prev_pc >= 0) { // garante que existe posição anterior válida
        sim->pc.pc = sim->pc.prev_pc; // volta para instrução anterior ( altera o PC)
        printf("Voltou para instrucao %d\n", sim->pc.pc);
    }
}

// ================= RESET =================

void reset_pc(struct simulador *sim) {
    sim->pc.pc = 0; // reset ( estou colocando o PC no inicio )
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
                carregar_memoria(sim); // carrega instruções do arquivo
                break;

            case 2:
                carregar_dados(&sim->dmem); // carrega memória de dados
                break;

            case 3:
                imprimir_memoria_instrucao(sim);
                imprimir_memoria_dados(&sim->dmem);
                break;

            case 4:
                mostrar_registradores(sim->reg);
                break;

            case 5: // mostra estado completo do sistema
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

    struct instrucao programa[100]; // cria memória de instruções (100 posições)

    struct simulador sim = {
        .dmem = {0}, // zera memória de dados
        .pc = {0, -1}, // PC começa em 0, sem anterior
        .reg = {0}, // zera registradores
        .programa = programa, // liga memória de instruções
        .prog_size = 0, // programa começa vazio
        .ula = {0,0,0} // zera ula
    };

    menu(&sim);

    return 0;
}
